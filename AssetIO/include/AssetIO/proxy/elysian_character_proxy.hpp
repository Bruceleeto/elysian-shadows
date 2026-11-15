/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_CHARACTER_PROXY_HPP
#define ELYSIAN_CHARACTER_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_character.hpp>
#include <AssetIO/proxy/elysian_character_proxy.hpp>
#include <AssetIO/proxy/elysian_component_proxy.hpp>








namespace elysian::lua {
void registerDirection(sol::state_view state);
void registerCharacter(sol::state_view state);
}

#endif // ELYSIAN_CHARACTER_PROXY_HPP

