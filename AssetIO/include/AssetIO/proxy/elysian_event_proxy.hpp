/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_EVENT_PROXY_HPP
#define ELYSIAN_EVENT_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <AssetIO/level/elysian_event.hpp>

















namespace elysian::lua {
void registerEventTarget(sol::state_view state);
void registerEventObserver(sol::state_view state);
void registerEvent(sol::state_view state);
void registerLuaEvent(sol::state_view state);
}

#endif // ELYSIAN_EVENT_PROXY_HPP
