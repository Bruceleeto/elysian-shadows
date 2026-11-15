/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_AREA_PROXY_HPP
#define ELYSIAN_AREA_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_area.hpp>
#include <AssetIO/proxy/elysian_instance_transform_proxy.hpp>
#include <AssetIO/utility/elysian_lua_values.hpp>









namespace elysian::lua {
void registerTileEntry(sol::state_view state);
void registerArea(sol::state_view state);
}

#endif // ELYSIAN_AREA_PROXY_HPP

