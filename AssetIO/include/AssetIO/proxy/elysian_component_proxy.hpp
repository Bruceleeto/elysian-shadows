/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_COMPONENT_PROXY_HPP
#define ELYSIAN_COMPONENT_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <AssetIO/crtp/elysian_component.hpp>
#include <AssetIO/crtp/elysian_entity.hpp>







namespace elysian::lua {
void registerComponent(sol::state_view state);
}

#endif // ELYSIAN_COMPONENT_PROXY_HPP

