/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_LIGHT_SOURCE_PROXY_HPP
#define ELYSIAN_LIGHT_SOURCE_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_light_source.hpp>









namespace elysian::lua {
void registerLightAttrib(sol::state_view state);
void registerLightSource(sol::state_view state);
}

#endif // ELYSIAN_LIGHT_SOURCE_PROXY_HPP

