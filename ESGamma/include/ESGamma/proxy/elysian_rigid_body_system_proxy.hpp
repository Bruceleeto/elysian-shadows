/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_RIGID_BODY_SYSTEM_PROXY_HPP
#define ELYSIAN_RIGID_BODY_SYSTEM_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/system/elysian_rigid_body_system.hpp>
#include <ESGamma/utility/elysian_force_generator.hpp>


















namespace elysian::lua {
void registerForceGenerator(sol::state_view state);
void registerGravityForceGenerator(sol::state_view state);
void registerImplosionForceGenerator(sol::state_view state);
void registerExplosionForceGenerator(sol::state_view state);
void registerRigidBodySystem(sol::state_view state);
}

#endif // ELYSIAN_RIGID_BODY_SYSTEM_HPP

