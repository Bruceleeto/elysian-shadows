/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_COMBAT_PROXY_HPP
#define ELYSIAN_COMBAT_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_combat.hpp>
#include <AssetIO/proxy/elysian_component_proxy.hpp>

















namespace elysian::lua {
void registerCombatAttrib(sol::state_view state);
void registerCombatStatus(sol::state_view state);
void registerCombatStatusModifier(sol::state_view state);
void registerAggroManager(sol::state_view state);
void registerCombat(sol::state_view state);
}

#endif // ELYSIAN_COMBAT_PROXY_HPP

