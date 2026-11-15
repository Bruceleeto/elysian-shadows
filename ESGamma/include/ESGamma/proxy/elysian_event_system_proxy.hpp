/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_EVENT_SYSTEM_PROXY
#define ELYSIAN_EVENT_SYSTEM_PROXY

#include <AssetIO/level/elysian_event.hpp>
#include <ESGamma/system/elysian_event_system.hpp>
#include <AssetIO/proxy/elysian_event_proxy.hpp>
#include <AssetIO/utility/elysian_lua_binding.hpp>






namespace elysian::lua {
void registerEventSystem(sol::state_view state);
}

#endif
