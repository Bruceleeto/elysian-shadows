/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESToolkit/ESGamma/tk_lua_manager.h>
#include <AssetIO/utility/elysian_lua_values.hpp>

QStringList TkLuaManager::_udTypeNames;
sol::object TkLuaManager::_serializeFunc;
unsigned TkLuaManager::_serializeDepth = 0;


bool TkLuaManager::initialize(void) {
    bool success = true;
    success &= LuaManagerBase::initialize();

    success &= runScriptSandbox(TK_LUA_MANAGER_METATABLE_SCRIPT_PATH);

    pushGlobalField("serialize");
    _serializeFunc = elysian::lua::fromStack();
    lua_register(_mainState, "serialize", &_serializeCFunc);

    //Put the bitches in alphabetical order!
    _udTypeNames.sort();

    return success;
}

bool TkLuaManager::uninitialize(void) {
    _serializeFunc = sol::object{};
    _udTypeNames.clear();
    _serializeDepth = 0;
    return elysian::LuaManagerBase::uninitialize();
}

sol::object TkLuaManager::getParent(const sol::object &t) {
    return callGlobalFunction<sol::object>("getparenttable", nullptr, t);
}

void TkLuaManager::setParent(const sol::object& t, const sol::object& p) {
    callGlobalFunction<void>("setparenttable", nullptr, t, p);
}

int TkLuaManager::_serializeCFunc(lua_State */*vm*/) {
    if(++_serializeDepth == 1) setSerializing(true);

    //rPrintByIndex(-3, 1, "Serialize[-3]");
    //rPrintByIndex(-2, 1, "Serialize[-2]");
    //rPrintByIndex(-1, 1, "Serialize[-1]");

//    dbgprintf("Stack size - %d", lua_gettop(_mainState));
    _serializeFunc.push();

    lua_insert(_mainState, -2);

  //  rPrintByIndex(-3, 1, "Serialize[-3]");
   // rPrintByIndex(-2, 1, "Serialize[-2]");
   // rPrintByIndex(-1, 1, "Serialize[-1]");

     //   dbgprintf("Stack size - %d", lua_gettop(_mainState));

    ELYSIAN_LUA_MANAGER_PROTECTED_CALL(1, 1, nullptr);

    if(--_serializeDepth == 0) setSerializing(false);

    return 1;
}
