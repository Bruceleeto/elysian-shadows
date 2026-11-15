/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_STATE_MACHINE_BEHAVIOR_HPP
#define ELYSIAN_STATE_MACHINE_BEHAVIOR_HPP

#include <AssetIO/crtp/elysian_behavior.hpp>
#include <AssetIO/utility/elysian_lua_values.hpp>
#include <AssetIO/utility/elysian_cpp_lua_callback.hpp>

#define ELYSIAN_STATE_MACHINE_DEBUG         1
#define ELYSIAN_STATE_MACHINE_NAME_SIZE     100
#define ELYSIAN_STATE_MACHINE_BEHAVIOR_NAME "StateMachine"
#define ELYSIAN_STATE_MACHINE_DEFAULT_NAME  "default"

namespace elysian {
#if 0
enum TRIGGER_INDEX {
    ON_SCRIPT_CREATE_INDEX,
    ON_SCRIPT_DESTROY_INDEX,
    ON_CREATE_INDEX,
    ON_DESTROY_INDEX,
    ON_LOAD_INDEX,
    ON_UNLOAD_INDEX,
    ON_ENTER_INDEX,
    ON_EXIT_INDEX,
    ON_UPDATE_INDEX,
    ON_RENDER_INDEX,
    ON_COLLIDE_INDEX,
    ON_COLLISION_ENTER_INDEX,
    ON_COLLISION_EXIT_INDEX,
    ON_EXAMINE_INDEX,
    ON_DIALOG_INDEX,
    ON_COMBAT_ENTER_INDEX,
    ON_COMBAT_EXIT_INDEX,
    ON_COMBAT_DEATH_INDEX,
    ON_COMBAT_RECEIVE_STATUS_MODIFIER_INDEX,
    ON_COMBAT_TARGET_COLLISION_ENTER_INDEX,
    ON_SHATTER_INDEX,
    ON_BUTTON_PRESS_INDEX,
    ON_BUTTON_RELEASE_INDEX,
    TRIGGER_ENGINE_MAX_INDEX,
    //ESTk-only trigers
    ON_TK_SELECT_INDEX=61,
    ON_TK_UNSELECT_INDEX=62,
    TRIGGER_MAX_INDEX=TRIGGER_ENGINE_MAX_INDEX
};
#endif

/*
 * Need to figure out how the fuck Lua is going to handle this...
 *
 * NEED TO ADD FLAGS
 */

    namespace game {

        class State {
            friend class StateMachineBehavior;
        public:

            const char* getName(void) const;
            void        setName(const char* name);

            void        setOnStateEnter(sol::object v);
            void        setOnStateExit(sol::object v);
            void        setOnUpdate(sol::object v);
            void        setOnCombatEnter(sol::object v);
            void        setOnCombatExit(sol::object v);
            void        setOnCombatDeath(sol::object v);


        private:
    #if 0
            CppLuaFunction<void, void> _onStateEnter;
            CppLuaFunction<void, void> _onStateExit;
            CppLuaFunction<void, void> _onUpdate;
            CppLuaFunction<void, void> _onCombatEnter;
            CppLuaFunction<void, void> _onCombatExit;
            CppLuaFunction<void, void> _onCombatDeath;
    #endif
            char        _name[ELYSIAN_STATE_MACHINE_NAME_SIZE] = { 0 };
        };

        class StateMachineBehavior: public Behavior {
        public:

            const char*         getName(void) const;
            void                setName(const char* str);
            State*              getCurrentState(void) const;
            void                setCurrentState(State* state);
            void                setCurrentStateByName(const char* name);
            const char*         getCurrentStateName(void) const;
            sol::object          getStateTable(void) const;
            void                setStateTable(sol::object t);

            virtual const char* getBehavName(void) const override;
            virtual void		onUpdate(void) override;
            //virtual void		onCombatEnter(void) override;
            //virtual void		onCombatExit(void) override;

    #if 0
            virtual void		onRender(void) override;
            virtual void		onCreate(void) override;
            virtual void		onDestroy(void) override;
            virtual void		onLoad(void) override;
            virtual void		onUnload(void) override;
            virtual void		onEnter(void) override;
            virtual void		onExit(void) override;
            virtual void		onCollide(CollisionContact*const contact, Collidable* const collidable) override;
            virtual void		onCollisionEnter(CollisionContact*const contact, Collidable* const collidable) override;
            virtual void		onCollisionExit(Collidable* const collidable) override;
            virtual void        onShatter(CollisionContact*const contact) override;
            virtual void		onExamine(Entity* entity) override;
            virtual void		onDialog(const unsigned screen, const int option) override;
            virtual void		onCombatDeath(void) override;
            virtual void		onCombatReceiveStatusModifier(CombatStatus*const modifier) override;
            virtual void		onCombatTargetCollisionEnter(CollisionContact*const contact, Collidable*const collidable, CombatStatus*const modifier) override;
    #endif


        private:
            char                _name[ELYSIAN_STATE_MACHINE_NAME_SIZE] = { 0 };
            sol::object          _stateTable;
            State*              _curState   = nullptr;
        };








        //======INLINEZ======
        const char* StateMachineBehavior::getName(void) const {
            return _name;
        }

        void StateMachineBehavior::setName(const char* str) {
            strcpy(_name, str);
        }

        State* StateMachineBehavior::getCurrentState(void) const {
            return _curState;
        }

        sol::object StateMachineBehavior::getStateTable(void) const {
            return _stateTable;
        }

        void StateMachineBehavior::setCurrentStateByName(const char *name) {
            State* state = nullptr;//_stateTable[name]->getValue<State*>();
#if ELYSIAN_STATE_MACHINE_DEBUG
            if(name && !state && strcmp(name, ELYSIAN_STATE_MACHINE_DEFAULT_NAME) == 0) {
               // errprintf("StateMachineBehavior::setCurrentStateByName(%s): State not found for Entity[%s]!", name, getEntity()->getName());
            }
#endif
            setCurrentState(state);
        }

        const char* StateMachineBehavior::getCurrentStateName(void) const {
            return _curState? _curState->_name : nullptr;
        }

    }

}

#endif // ELYSIAN_STATE_MACHINE_BEHAVIOR_HPP
