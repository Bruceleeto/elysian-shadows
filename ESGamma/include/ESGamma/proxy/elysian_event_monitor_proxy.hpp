/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_EVENT_MONITOR_PROXY_HPP
#define ELYSIAN_EVENT_MONITOR_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/utility/elysian_event_monitor.hpp>













namespace elysian::lua {
void registerEventMonitor(sol::state_view state);
void registerLuaEventMonitor(sol::state_view state);
void registerLuaEventCallback(sol::state_view state);
}

#endif // ELYSIAN_EVENT_MONITOR_PROXY_HPP

