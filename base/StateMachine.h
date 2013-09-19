/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#include "Log.h"
#include <string>
#include <map>

template<typename T>
class StateHandler {
	public:
        // Virtual destructor
        virtual ~StateHandler() {}

		// Setup internal var, states, ...
		virtual void setup() = 0;

        // Called once, before calling updatePreEnter
        virtual void onPreEnter(T ) {}

		// Called repeteadly, before proper entering.
		// Must return true when handler is ready to enter
		virtual bool updatePreEnter(T , float ) {return true;}

		// Called when entering the state (and old state has exited)
		virtual void onEnter(T ) {}

		// Update method. Return value is next state
		virtual T update(float dt) = 0;

        virtual void onPreExit(T ) {}

		// Called repeteadly, before proper exiting.
		// Must return true when handler is ready to exit
		virtual bool updatePreExit(T , float ) {return true;}

		// Called when exiting the state (and old state has exited)
		virtual void onExit(T ) {};
};

template<typename T>
class StateMachine {
#ifdef SAC_INGAME_EDITORS
    friend class RecursiveRunnerDebugConsole;
#endif
	public:
        StateMachine();

		StateMachine(T initState);

        ~StateMachine();

        void setup(T initState);

        void reEnterCurrentState();

		void registerState(T id, StateHandler<T>* hdl, const std::string& stateDebugName);

        void unregisterAllStates();

		void update(float dt);

        void forceNewState(T state);

        T getCurrentState() const;

        unsigned getStateCount() const { return state2handler.size(); }

        const std::map<T, StateHandler<T>*>& getHandlers() const;

        StateHandler<T>* getCurrentHandler() { return state2handler[currentState]; }

	private:
		T currentState, overrideNextState;
		std::map<T, StateHandler<T>*> state2handler;
        #if SAC_ENABLE_LOG || SAC_ENABLE_PROFILING
        std::map<T, std::string> state2Name;
        #endif
		struct {
			T fromState, toState;
			bool readyExit, readyEnter;
            bool dumbFrom;
		} transition;
		bool override, transitionning;

    private:
        void transitionTo(T oldState, T newState);
        void changeState(T oldState, T newState, bool ignoreFromState);
};


#include "StateMachine.inl"
