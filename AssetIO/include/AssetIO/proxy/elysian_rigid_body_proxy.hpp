/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_RIGID_BODY_PROXY_HPP
#define ELYSIAN_RIGID_BODY_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_rigid_body.hpp>











namespace elysian::lua {
void registerRigidBodyAttrib(sol::state_view state);
void registerForceTarget(sol::state_view state);
void registerRigidBody(sol::state_view state);
}

#endif // ELYSIAN_RIGID_BODY_PROXY_HPP

