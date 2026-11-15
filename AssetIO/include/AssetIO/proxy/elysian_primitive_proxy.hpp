/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_PRIMITIVE_PROXY_HPP
#define ELYSIAN_PRIMITIVE_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/primitive/elysian_color_base.hpp>
#include <AssetIO/primitive/elysian_material.hpp>
#include <AssetIO/crtp/elysian_sprite_attrib.hpp>
#include <AssetIO/primitive/elysian_quad_sprite_attrib.hpp>
#include <AssetIO/proxy/elysian_instance_transform_proxy.hpp>
#include <AssetIO/primitive/elysian_rect.hpp>
#include <AssetIO/crtp/elysian_blend_mode.hpp>


























namespace elysian::lua {
void registerGYColor4(sol::state_view state);
void registerColor4(sol::state_view state);
void registerMaterial(sol::state_view state);
void registerBlendMode(sol::state_view state);
void registerPrimAttrib(sol::state_view state);
void registerSpriteAttrib(sol::state_view state);
void registerQuadSpriteAttrib(sol::state_view state);
void registerRecti(sol::state_view state);
}

#endif // ELYSIAN_PRIMITIVE_PROXY_HPP

