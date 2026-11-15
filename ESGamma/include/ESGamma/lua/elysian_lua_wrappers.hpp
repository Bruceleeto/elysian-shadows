/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_LUA_WRAPPERS_HPP
#define ELYSIAN_LUA_WRAPPERS_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/utility/elysian_event_monitor.hpp>
#include <ESGamma/crtp/elysian_gui_system.hpp>
#include <ESGamma/lua/elysian_lua_singleton.hpp>
#include <AssetIO/crtp/elysian_collidable.hpp>
#include <AssetIO/crtp/elysian_entity.hpp>
#include <ESGamma/crtp/elysian_debug.hpp>
#include <AssetIO/level/elysian_tile.hpp>
#include <AssetIO/proxy/elysian_entity_proxy.hpp>

/* IT'S NOT SAFE TO GO ALONE! TAKE THIS HEADER FILE. IT WILL HELP
 * YOU ON YOUR JOURNEY!
 *
 * I have taken special care to create a SINGLE header file serving
 * as a registry for all Gamma/libGyro functionality exposed to Lua.
 * I have tried to keep this as clean as possible so that it can be
 * used as an index without scaring scripters away. This is everything
 * you need. It is organized as follows:
 * 1) C and C++ Class and Struct Wrappers
 * 2) C and C++ Constant and Enumeration Wrappers
 * 3) C Global and C++ Static Member Function Wrappers
 *
 *
 * Now go forth unto the world with the power of Lua, my children.
 *				--Falco
 */


// ====== C/++ CLASS/STRUCT WRAPPERS ======








//dummy class for encapsulating libGyro within its own namespace
class gyro {};




// ======= C GLOBAL AND C++ STATIC FUNCTION WRAPPERS ======

static inline int dbgPush(lua_State* const /*l*/) {
    elysian::Debug::push();
    return 0;
}

static inline int dbgPop(lua_State* const /*l*/) {
    int count = 1;
    sol::object arg = elysian::lua::fromStack(-1);

    if(elysian::lua::type(arg) == LUA_TNUMBER) {
        const auto val = arg.as<int>();
        if(val >= 0) {
            count = val;
        }
    }

    elysian::Debug::pop(count);
    return 0;
}

static inline int print(lua_State* const /*l*/) {
    elysian::LuaManager::luaPrint(elysian::LuaManager::getVariadicStringFromStack());
    return 0;
}

static inline int warnprint(lua_State* const /*l*/) {
    elysian::Debug::luaPrint(elysian::LuaManager::getVariadicStringFromStack(), elysian::Debug::LOG_WARNING);
    return 0;
}

static inline int errprint(lua_State* const /*l*/) {
    elysian::Debug::luaPrint(elysian::LuaManager::getVariadicStringFromStack(), elysian::Debug::LOG_ERROR);
    return 0;
}

static inline int printAdvLog(lua_State* const /*l*/) {
    elysian::Singletons::getGuiSys()->adventureLog.addLine(elysian::AdventureLog::TEXT_NORMAL, elysian::LuaManager::getVariadicStringFromStack());
    return 0;
}

static inline int Singletons_getLevelSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getLevelSys());
}

static inline int Singletons_getCurArea(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getCurrentArea());
}

static inline int Singletons_getCurLevel(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getCurrentLevel());
}

static inline int Singletons_setProcessInput(lua_State* const vm) {
	(void)vm; elysian::Singletons::setProcessInput(sol::stack::get<bool>(vm, 1)); return 0;
}

static inline int Singletons_getPartySystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getPartySys());
}

static inline int Singletons_getInputSystem(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getInputSys());
}

static inline int Singletons_getAudioSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getAudioSys());
}

static inline int Singletons_getLightSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getLightSys());
}

static inline int Singletons_getParticleSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getPartSys());
}

static inline int Singletons_getDaySystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getDaySys());
}

static inline int Singletons_getDialogSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getDialogSys());
}

static inline int Singletons_getGuiSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getGuiSys());
}

static inline int Singletons_getTimerSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getTimerSys());
}

static inline int Singletons_getCollisionSystem(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getCollisionSys());
}

static inline int Singletons_getRigidBodySystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getRigidBodySys());
}

static inline int Singletons_getCombatSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getCombatSys());
}

static inline int Singletons_getCameraSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getCameraSys());
}

static inline int Singletons_getCharacterSystem(lua_State* const vm) {
	return sol::stack::push(vm, elysian::Singletons::getCharSys());
}

static inline int Singletons_getExaminableSystem(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getExamSys());
}

static inline int Singletons_getEventSystem(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getEventSys());
}

static inline int Singletons_getThreadSystem(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getThreadSystem());
}

static inline int Singletons_getRenderSystem(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Singletons::getRenderSystem());
}

static inline int InputSystem_importControllerButtonMappings(lua_State* const vm) {
    return sol::stack::push(vm, elysian::InputSystem::importControllerButtonMappings(sol::stack::get<const char*>(vm, 1)));;
}

static inline int Direction_fromVector2(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Direction::fromVector2(sol::stack::get<const GYVector2&>(vm, 1)));
}

static inline int Direction_toVector2(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Direction::toVector2(sol::stack::get<elysian::Direction::DIR4>(vm, 1)));
}

static inline int Direction_toVector3(lua_State* const vm) {
    return sol::stack::push(vm, elysian::Direction::toVector3(sol::stack::get<elysian::Direction::DIR4>(vm, 1), sol::stack::get<float>(vm, 2)));
}

static inline int EventMonitor_null(lua_State *const vm) {
	return sol::stack::push(vm, elysian::EventMonitor::null());
}

static inline int gyroWavCreate(lua_State *const vm) {
	return sol::stack::push(vm, gyAudWavCreate(sol::stack::get<const char*>(vm, 1)));
}

static inline int gyroWavDestroy(lua_State *const vm) {
	return sol::stack::push(vm, gyAudWavDestroy(sol::stack::get<GYWav*>(vm, 1)));
}

static inline int gyroMatPush(lua_State *const vm) {
    (void)vm; gyMatPush(); return 0;
}

//===== GYRO MATRIX STACK ====

static inline int gyroMatPop(lua_State*const vm) {
    (void)vm; gyMatPop(); return 0;
}

static inline int gyroMatLoadIdentity(lua_State*const vm) {
    (void)vm; gyMatLoadIdentity(); return 0;
}

static inline int gyroMatTranslate(lua_State*const vm) {
    (void)vm; gyMatTranslate(sol::stack::get<const float>(vm, 1), sol::stack::get<const float>(vm, 2), sol::stack::get<const float>(vm, 3)); return 0;
}

static inline int gyroMatRotateZ(lua_State*const vm) {
    (void)vm; gyMatRotateZ(sol::stack::get<const float>(vm, 1)); return 0;
}

static inline int gyroMatScale(lua_State*const vm) {
    (void)vm; gyMatScale(sol::stack::get<const float>(vm, 1), sol::stack::get<const float>(vm, 2), sol::stack::get<const float>(vm, 3)); return 0;
}

static inline int gyroMatTranslateScale(lua_State*const vm) {
    (void)vm; gyMatTranslateScale(sol::stack::get<const float>(vm, 1), sol::stack::get<const float>(vm, 2), sol::stack::get<const float>(vm, 3), sol::stack::get<const float>(vm, 4), sol::stack::get<const float>(vm, 5), sol::stack::get<const float>(vm, 6)); return 0;
}

/*
static inline int gyroMatInstance(lua_State*const vm) {
    (void)vm; gyMatInstance(sol::stack::get<const GYInstMat *const>(vm, 1)); return 0;
}
*/

static inline int gyroMatRotate(lua_State*const vm) {
    (void)vm; gyMatRotate(sol::stack::get<const float>(vm, 1), sol::stack::get<const float>(vm, 2), sol::stack::get<const float>(vm, 3), sol::stack::get<const float>(vm, 4)); return 0;
}

static inline int gyroMatShearZ(lua_State*const vm) {
    (void)vm; gyMatShearZ(sol::stack::get<const float>(vm, 1), sol::stack::get<const float>(vm, 2)); return 0;
}

//==== GYRO VECTOR MATH ====
/*
static inline int gyroMatMultMat(lua_State*const vm) {
    (void)vm; gyMatMultMat(sol::stack::get<GYMatrix4>(vm, 1), sol::stack::get<const GYMatrix4>(vm, 2), sol::stack::get<const GYMatrix4>(vm, 3)); return 0;
}*/
/*
static inline int gyroMatMultVec(lua_State*const vm) {
    (void)vm; gyMatMultVec(sol::stack::get<const GYVector4*const>(vm, 1), sol::stack::get<GYVector4*>(vm, 2), sol::stack::get<const unsigned>(vm, 3)); return 0;
}

static inline int gyroVec4Add(lua_State*const vm) {
    (void)vm; gyVec4Add(sol::stack::get<void*>(vm, 1), sol::stack::get<const void* const>(vm, 2), sol::stack::get<const void* const>(vm, 3)); return 0;
}*/
/*
static inline int gyroVec4Mult(lua_State*const vm) {
    (void)vm; gyVec4Mult(sol::stack::get<const void*const>(vm, 1), sol::stack::get<const void* const>(vm, 2)); return 0;
}

static inline int gyroVec4Add(lua_State*const vm) {
    (void)vm; gyVec4Add(sol::stack::get<void*>(vm, 1), sol::stack::get<const void* const>(vm, 2), sol::stack::get<const void* const>(vm, 3)); return 0;
}*/


namespace elysian::lua {
void registerEngine(sol::state_view state);
void registergyro(sol::state_view state);
}

#endif
