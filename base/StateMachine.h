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
#include "Entity.h"
#include <base/SacDefs.h>
#include "util/MurmurHash.h"

class AssetAPI;

template<typename T>
class StateHandler {
    public:
        StateHandler(const char* pName) : name(pName) {}

        // Virtual destructor
        virtual ~StateHandler() {}

        // Setup internal var, states, ...
        virtual void setup(AssetAPI*) {}

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

        const char* name;
};

void initStateEntities(AssetAPI* asset, const char* stateName, std::map<hash_t, Entity>& entities);

template<typename T>
class StateMachine {
#ifdef SAC_INGAME_EDITORS
    friend class RecursiveRunnerDebugConsole;
#endif
    public:
        StateMachine();

        StateMachine(T initState);

        ~StateMachine();

        void setup(AssetAPI* asset);

        void start(T initState);

        void registerState(T id, StateHandler<T>* hdl);

        void unregisterAllStates();

        void update(float dt);

        void forceNewState(T state);

        T getCurrentState() const;

        unsigned getStateCount() const { return state2handler.size(); }

        const std::map<T, StateHandler<T>*>& getHandlers() const;

        StateHandler<T>* getCurrentHandler() { return state2handler[currentState]; }

        // const char* getCurrentStateName() const { return getCurrentHandler->name; }

        int serialize(uint8_t** out) const;
        int deserialize(const uint8_t* in, int size);

    private:
        T currentState, overrideNextState, previousState;
        std::map<T, StateHandler<T>*> state2handler;

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


// #include "StateMachine.inl"
