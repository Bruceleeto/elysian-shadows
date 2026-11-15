/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_INPUT_SYSTEM_PROXY_HPP
#define ELYSIAN_INPUT_SYSTEM_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/crtp/elysian_input_system.hpp>
#include <AssetIO/proxy/elysian_instance_transform_proxy.hpp>
#include <ESGamma/device/elysian_rumble_device.hpp>
#include <ESGamma/device/elysian_controller_device.hpp>
#include <ESGamma/device/elysian_visual_memory_device.hpp>
#include <ESGamma/device/elysian_keyboard_device.hpp>
#include <ESGamma/device/elysian_mouse_device.hpp>
#include <ESGamma/device/elysian_touch_device.hpp>
#include <ESGamma/device/elysian_power_supply_device.hpp>
#ifdef ELYSIAN_INPUT_SYSTEM_SENSORS_ENABLED
#   include<ESGamma/device/elysian_sensor_device.hpp>
#endif





















































#ifdef ELYSIAN_INPUT_SYSTEM_SENSORS_ENABLED













#endif





namespace elysian::lua {
void registerPlayerIODevice(sol::state_view state);
void registerKeyboard(sol::state_view state);
void registerMouse(sol::state_view state);
void registerVirtualDPad(sol::state_view state);
void registerController(sol::state_view state);
void registerTouchDevice(sol::state_view state);
void registerPowerSupply(sol::state_view state);
void registerInputEvent(sol::state_view state);
void registerControllerEvent(sol::state_view state);
void registerControllerButtonEvent(sol::state_view state);
void registerControllerAnalogEvent(sol::state_view state);
void registerControllerTriggerEvent(sol::state_view state);
void registerKeyEvent(sol::state_view state);
void registerMouseBaseEvent(sol::state_view state);
void registerMouseEvent(sol::state_view state);
void registerMouseWheelEvent(sol::state_view state);
void registerRumbleEffect(sol::state_view state);
void registerLeftRightRumbleEffect(sol::state_view state);
void registerDirectionalRumbleEffect(sol::state_view state);
void registerConstantRumbleEffect(sol::state_view state);
void registerRampRumbleEffect(sol::state_view state);
void registerPeriodicRumbleEffect(sol::state_view state);
void registerRumbleDevice(sol::state_view state);
void registerVisualMemoryUnit(sol::state_view state);
void registerAccelerometer(sol::state_view state);
void registerGyroscope(sol::state_view state);
void registerMagnetometer(sol::state_view state);
void registerAltimeter(sol::state_view state);
void registerPedometer(sol::state_view state);
void registerScale(sol::state_view state);
void registerInputSystem(sol::state_view state);
}

#endif // EYSIAN_INPUT_SYSTEM_PROXY_HPP

