/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_LUA_MANAGER_BASE_HPP
#define ELYSIAN_LUA_MANAGER_BASE_HPP

#include <map>
#include <AssetIO/utility/elysian_lua_values.hpp>
#include <functional>
#include <type_traits>
#include <AssetIO/utility/elysian_stl_algorithm.hpp>
#include <ESGamma/lua/elysian_lua_script.hpp>
#include <AssetIO/utility/elysian_functional.hpp>
#include <ESGamma/crtp/elysian_debug.hpp>
#include <libGyro/gyro_defines.h>
#include <ESGamma/crtp/elysian_task_monitor.hpp>

#if GYRO_PLATFORM == GYRO_PLATFORM_x86
#	define ELYSIAN_LUA_DEBUG                            true
#   define ELYSIAN_LUA_STACK_BALANCE_RECURSIVE_DEPTH     100
#else
#	define ELYSIAN_LUA_DEBUG                            true
#   define ELYSIAN_LUA_TASK_MONITOR_NAME                 "LuaVM"
#   define ELYSIAN_LUA_STACK_BALANCE_RECURSIVE_DEPTH     100
#endif

#define ELYSIAN_LUA_MANAGER_SERIALIZE_SCRIPT_PATH       "/assets/scripts/utility/serialize.lua"
#define ELYSIAN_LUA_MANAGER_TABLE_MANIP_SCRIPT_PATH     "/assets/scripts/utility/table_manip.lua"
#define ELYSIAN_LUA_PREFAB_PATH                         "/assets/scripts/prefab"

#define ELYSIAN_LUA_PROXY_TYPE_METAFIELD                "__typename"
#define ELYSIAN_LUA_PROXY_DOWN_CAST_METAFIELD           "__downcast"


#define ELYSIAN_LUA_MANAGER_PROTECTED_CALL_WITH_SCRIPT(nargs, nresults, script) \
    (elysian::LuaManager::_protectedCall(nargs, nresults, script, __FILE__, __LINE__, __func__))
#define ELYSIAN_LUA_MANAGER_PROTECTED_CALL_WITHOUT_SCRIPT(nargs, nresults) \
    (ELYSIAN_LUA_MANAGER_PROTECTED_CALL_WITH_SCRIPT(nargs, nresults, nullptr))

#define ELYSIAN_LUA_MANAGER_PROTECTED_CALL_MACRO(_1, _2, _3, MACRO, ...) MACRO
#define ELYSIAN_LUA_MANAGER_PROTECTED_CALL(...) \
    ELYSIAN_LUA_MANAGER_PROTECTED_CALL_MACRO(__VA_ARGS__, \
                                            ELYSIAN_LUA_MANAGER_PROTECTED_CALL_WITH_SCRIPT, \
                                            ELYSIAN_LUA_MANAGER_PROTECTED_CALL_WITHOUT_SCRIPT)(__VA_ARGS__)
ELYSIAN_ASSET_TYPEDEF(LuaManager)

namespace elysian {
	
    class LuaManagerBase {
    protected:
		
		static std::map<const char *, LuaScript *, ltstr> _scripts;
        static sol::state           _mainState;
        static int                  _errorCode;
        static int                  _printSeverity;
#ifdef ELYSIAN_LUA_DEBUG
        static int                  _stackBalanceTop[ELYSIAN_LUA_STACK_BALANCE_RECURSIVE_DEPTH];
        static int                  _stackBalanceRecursiveDepth;
        static TaskMonitor          _taskMonitor;
#endif
        static const char           _rPrint[];
        static int                  _traceBackDbgPushes;
        static bool                 _serializing;

        static int                  _changePrintSeverity(const int sev);
        static void                 _errorprintf(const char *const string, ...);
        static void                 _registerWrappers(void);

		//Assumes actual function is already at the top of the stack!
                                    template<typename RType, typename... PTypes>
        static RType                _callFunction(LuaScript* script, PTypes&&... parameters);

    protected:
                                    LuaManagerBase(void) = default;
		static void					_showError(void);
		static int					_getErrorCode(void);
		
        static bool					_protectedCall(const int nargs, const int nresults, LuaScript *script, const char* file, int line, const char* func);
		static LuaScript*			_createScriptFromExisting(const char* const file);

        static int					_traceBack(lua_State *L);
		static unsigned				_envDump(LuaScript* scr);
		static void					_cStackDump(void);
		static void					_runtimeStackDump(void);

	public:

		static bool					initialize(void);
		static bool					uninitialize(void);
        static void                 updateFrameStats(void);
		
        static sol::object           exec(const char*const str, ...);
		static bool					runString(const char *const string);
		static LuaScript*			loadScript(const char *const file);
		static LuaBehaviorScript*	loadBehaviorScript(const char *const file);
		static bool					unloadBehaviorScript(LuaBehaviorScript &script);
		static bool					unloadScript(LuaScript &script);
        static bool					runScriptSandbox(const char *const file, LuaScript* const script=nullptr, const unsigned retCount = 0);
                                    template<typename RType, typename... PTypes>
        static RType                callGlobalFunction(const char*const funcName, LuaScript* script, PTypes&&... parameters);

		static void					runGarbageCollector(void);
		static void					dumpStats(void);
        static int                  stackDepth(void);
        static void                 stackBalanceBegin(void);
        static void                 stackBalanceEnd(void);

		static unsigned				getScriptCount(void);
		static unsigned				getScriptInstances(void);
		static unsigned				getKbytesUsed(void);
        static bool                 isSerializing(void);
        static void                 setSerializing(const bool v);

        static void					rPrintByIndex(const int index, const unsigned depth, const char* name);
        static void					rPrintByRef(const int ref, const unsigned depth, const char* name);
        static void                 luaPrint(const char*const string, const int severity = Debug::LOG_OFF);
        static bool                 genericIterate(const int index, std::function<void(void)> lambda);

        static const char*          getVariadicStringFromStack(void);
        static bool                 getMetatable(const int index);
        static void                 pushGlobalField(const char*const key);
        static sol::object           getGlobalsTable(void);
        static sol::object           getPrefab(const char*const name);
        static const char*          getPrefabName(const sol::object& v);

        static bool                 saveTable(const sol::object& v, const char*const filename);

        template<class... A> static void push(A&&... args) {
            sol::stack::multi_push(_mainState, std::forward<A>(args)...);
        }
        template<class T> static T pull() {
            if constexpr (!std::is_void_v<T>) return sol::stack::pop<T>(_mainState);
        }

	};











    // ========= INLINEZ ========

    //Assumes actual function is already at the top of the stack!
    template<typename RType, typename... PTypes>
    inline RType LuaManagerBase::_callFunction(LuaScript* script, PTypes&&... parameters) {
        auto function = sol::stack::pop<sol::protected_function>(_mainState);
        auto result = function(std::forward<PTypes>(parameters)...);
        if (!result.valid()) {
            const sol::error error = result;
            luaPrint(error.what(), Debug::LOG_ERROR);
            if (script) script->setError();
            return RType();
        }
        if constexpr (std::is_void_v<RType>) return;
        else if constexpr (std::is_same_v<RType, const char*>) {
            static std::string buffer;
            auto value = result.template get<sol::object>();
            if (!value.valid()) return nullptr;
            buffer = lua::string(value);
            return buffer.c_str();
        } else return result.template get<RType>();
    }

    template<typename RType, typename... PTypes>
    inline RType LuaManagerBase::callGlobalFunction(const char* name, LuaScript* script, PTypes&&... parameters) {
        sol::object function;
        if (script) {
            sol::table environment(_mainState, sol::ref_index(script->getEnvironRef()));
            function = environment[name];
        } else function = _mainState[name];
        if (function.get_type() != sol::type::function) {
            errprintf("Lua function '%s' was not found", name);
            return RType();
        }
        function.push();
        return _callFunction<RType>(script, std::forward<PTypes>(parameters)...);
    }

inline bool LuaManagerBase::_protectedCall(const int nargs, const int nresults, LuaScript *script, const char* file, int line, const char* func) {
    (void)(file);
            (void)(line);
          (void)(func);
#ifdef ELYSIAN_LUA_DEBUG
        bool success = true;
        if(script) {
            lua_pushcfunction(_mainState, &LuaManagerBase::_traceBack);
            lua_insert(_mainState, 1);
        }
        _taskMonitor.resume();
        if((_errorCode = lua_pcall(_mainState, nargs, nresults, script? 1 : 0)) != 0) {
            DbgEvent event("Lua Exception");
            if(script) {
                lua_remove(_mainState, 1);
                script->setError();
                _cStackDump();
                lua_remove(_mainState, lua_gettop(_mainState));		//clean up stack after reesting
                //_envDump(script);
                dumpStats();
                if(file && func) {
                    DbgEvent event("C/++ Caller Info");
                    dbgprintf("%s:%d (function: %s)", file, line, func);
                }
                //if(_traceBackDbgPushes) {
                    //Debug::pop();
                //    --_traceBackDbgPushes;
               // }
            } else {
                _showError();
                if(file && func) {
                    DbgEvent event("C/++ Caller Info");
                    dbgprintf("%s:%d (function: %s)", file, line, func);
                }
                //lua_remove(_mainState, 1);
                //_cStackDump();
                //dumpStats();
            }
            success = false;
        } else {
            if(script)
                lua_remove(_mainState, 1); //remove c traceback function from bottom of stack
        }

#else	//ain't NOBODY got time for debug overhead!
        lua_call(_mainState, nargs, nresults);
#endif
        _taskMonitor.pause();
        return success; // shit was just fine
	}


    inline int LuaManagerBase::stackDepth(void) {
        return lua_gettop(_mainState);
    }

    inline void LuaManagerBase::stackBalanceBegin(void) {
        GY_ASSERT(_stackBalanceRecursiveDepth < ELYSIAN_LUA_STACK_BALANCE_RECURSIVE_DEPTH-1);
        _stackBalanceTop[_stackBalanceRecursiveDepth++] = stackDepth();
    }

    inline void LuaManagerBase::stackBalanceEnd(void) {
        if(_stackBalanceTop[--_stackBalanceRecursiveDepth] != stackDepth()) {
            _cStackDump();
            _errorprintf("Start Depth - %d, End Depth - %d", _stackBalanceTop[_stackBalanceRecursiveDepth], stackDepth());
            GY_ASSERT(false);
        }
    }

    inline bool LuaManagerBase::getMetatable(const int index) {
        return (bool)lua_getmetatable(_mainState, index);
    }

    inline void LuaManagerBase::pushGlobalField(const char * const key) {
        lua_getglobal(_mainState, key);
        GY_ASSERT(!lua_isnil(_mainState, -1));
    }


    inline int LuaManagerBase::_changePrintSeverity(const int sev) {
        int current = _printSeverity;
        _printSeverity = (Debug::SEVERITY)sev;
        return current;
    }

    inline bool LuaManagerBase::isSerializing(void) { return _serializing; }
    inline void LuaManagerBase::setSerializing(const bool v) { _serializing = v; }









    template<> inline const char* LuaManagerBase::pull<const char*>() {
        static std::string buffer;
        auto value = sol::stack::pop<sol::object>(_mainState);
        if (!value.valid()) return nullptr;
        buffer = lua::string(value);
        return buffer.c_str();
    }
}
#endif
