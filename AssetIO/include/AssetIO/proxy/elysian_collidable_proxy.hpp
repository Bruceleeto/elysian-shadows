/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_COLLIDABLE_PROXY_HPP
#define ELYSIAN_COLLIDABLE_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/proxy/elysian_component_proxy.hpp>
#include <AssetIO/crtp/elysian_collidable.hpp>
#include <AssetIO/proxy/elysian_bit_mask_proxy.hpp>

















namespace elysian::lua {
void registerCollisionFlags(sol::state_view state);
void registerCollidableStaticAttrib(sol::state_view state);
void registerCollidableAttrib(sol::state_view state);
void registerCollisionContact(sol::state_view state);
void registerCollidable(sol::state_view state);
}

#endif // ELYSIAN_COLLIDABLE_PROXY_HPP

