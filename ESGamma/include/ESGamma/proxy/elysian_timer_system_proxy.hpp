/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_TIMER_SYSTEM_PROXY_HPP
#define ELYSIAN_TIMER_SYSTEM_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/crtp/elysian_timer_system.hpp>






namespace elysian::lua {
void registerTimerSystem(sol::state_view state);
}

#endif // ELYSIAN_TIMER_SYSTEM_PROXY_HPP

