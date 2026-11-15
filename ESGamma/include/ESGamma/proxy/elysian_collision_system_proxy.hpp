/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_COLLISION_SYSTEM_PROXY_HPP
#define ELYSIAN_COLLISION_SYSTEM_PROXY_HPP

#include <ESGamma/system/elysian_collision_system.hpp>
#include <AssetIO/utility/elysian_lua_binding.hpp>






namespace elysian::lua {
void registerCollisionSystem(sol::state_view state);
}

#endif // ELYSIAN_COLLISION_SYSTEM_PROXY_HPP
