#include "StateMachine.h"

template<typename T>
StateMachine<T>::StateMachine() : override(false), transitionning(false) {

}


template<typename T>
StateMachine<T>::StateMachine(T initState) : currentState(initState), override(false), transitionning(false) {

}

template<typename T>
StateMachine<T>::~StateMachine() {
    for(auto it=state2handler.begin(); it!=state2handler.end(); ++it) {
        delete it->second;
    }
}

template<typename T>
void StateMachine<T>::setup(T initState) {
    for(auto it=state2handler.begin(); it!=state2handler.end(); ++it) {
        it->second->setup();
    }
    currentState = initState;
}

template<typename T>
void StateMachine<T>::registerState(T id, StateHandler<T>* hdl, const std::string&
    #ifdef SAC_ENABLE_LOG
    stateDebugName
    #else
    #endif
    ) {
    LOGW_IF(state2handler.find(id) != state2handler.end(), "State id #" << id << " already registered")
	state2handler.insert(std::make_pair(id, hdl));
    #ifdef SAC_ENABLE_LOG
    state2Name.insert(std::make_pair(id, stateDebugName));
    #endif
}

template<typename T>
void StateMachine<T>::update(float dt) {
	// Override next state if requested
	if (override) {
        if (overrideNextState == currentState)
            LOGW("overrideNextState == currentState == " << currentState)
        changeState(currentState, overrideNextState);
        override = transitionning = false;
    }

    // Update active state
    if (!transitionning) {
    	LOGF_IF(state2handler.find(currentState) == state2handler.end(), "Current state #" << currentState << " has no handler")

        // Update state
        T newState = state2handler[currentState]->update(dt);

        // New state requested ?
        if (newState != currentState) {
            LOGV(2, "Transition begins: " << state2Name[newState] << " -> " << state2Name[currentState])
        	// init transition
        	transition.fromState = currentState;
        	transition.toState = newState;
        	transition.readyExit = transition.readyEnter = false;
            state2handler[currentState]->onPreExit(newState);
            state2handler[newState]->onPreEnter(currentState);

            transitionning = true;
        }
    } else {
    	if (!transition.readyExit)
    		transition.readyExit = state2handler[transition.fromState]->updatePreExit(transition.toState, dt);
    	if (!transition.readyEnter)
    		transition.readyEnter = state2handler[transition.toState]->updatePreEnter(transition.fromState, dt);

    	// If both states are ready, change state
    	if (transition.readyExit && transition.readyEnter) {
            LOGV(2, "Transition complete. New state: " << state2Name[transition.toState])
            changeState(transition.fromState, transition.toState);
            transitionning = false;
    	}
    }
}

template<typename T>
void StateMachine<T>::forceNewState(T state) {
    LOGV(1, "Force new state: " << state)
    overrideNextState = state;
    override = true;
}

template<typename T>
T StateMachine<T>::getCurrentState() const {
    return currentState;
}

template<typename T>
void StateMachine<T>::changeState(T oldState, T newState) {
    if (oldState == newState) {
        LOGW("Cannot change state : old state = new state ( = " << oldState << ")")
        return;
    }
    state2handler[oldState]->onExit(newState);
    state2handler[newState]->onEnter(oldState);
    currentState = newState;
}

template<typename T>
void StateMachine<T>::reEnterCurrentState() {
    state2handler[currentState]->onPreEnter(currentState);
    state2handler[currentState]->onEnter(currentState);
}
