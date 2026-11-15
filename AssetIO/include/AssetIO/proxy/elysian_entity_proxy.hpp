/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_ENTITY_PROXY_HPP
#define ELYSIAN_ENTITY_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_entity.hpp>
#include <AssetIO/proxy/elysian_instance_transform_proxy.hpp>
#include <AssetIO/proxy/elysian_primitive_proxy.hpp>
#include <AssetIO/proxy/elysian_collidable_proxy.hpp>
#include <ESGamma/proxy/elysian_event_monitor_proxy.hpp>
#include <AssetIO/proxy/elysian_renderable_proxy.hpp>
#include <AssetIO/proxy/elysian_warp_proxy.hpp>
#include <AssetIO/proxy/elysian_rigid_body_proxy.hpp>
#include <AssetIO/proxy/elysian_character_proxy.hpp>
#include <AssetIO/proxy/elysian_audio_source_proxy.hpp>
#include <AssetIO/proxy/elysian_light_source_proxy.hpp>
#include <AssetIO/proxy/elysian_examinable_proxy.hpp>
#include <AssetIO/proxy/elysian_particle_emitter_proxy.hpp>
#include <AssetIO/proxy/elysian_combat_proxy.hpp>
#include <AssetIO/proxy/elysian_skeleton_proxy.hpp>
#include <AssetIO/proxy/elysian_lua_behavior_proxy.hpp>





/*

*/








namespace elysian::lua {
void registerRenderableObject(sol::state_view state);
void registerEntity(sol::state_view state);
}

#endif // ELYSIAN_ENTITY_PROXY_HPP

