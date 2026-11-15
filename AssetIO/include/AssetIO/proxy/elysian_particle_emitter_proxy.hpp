/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_PARTICLE_EMITTER_PROXY_HPP
#define ELYSIAN_PARTICLE_EMITTER_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_particle_emitter.hpp>
#include <AssetIO/crtp/elysian_sprite_attrib.hpp>
#include <AssetIO/crtp/elysian_collidable.hpp>
#include <AssetIO/primitive/elysian_color_base.hpp>








namespace elysian::lua {
void registerParticleEmitterAttrib(sol::state_view state);
void registerParticleEmitter(sol::state_view state);
}

#endif // ELYSIAN_PARTICLE_EMITTER_PROXY_HPP

