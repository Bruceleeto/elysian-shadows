/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_RENDERABLE_PROXY_HPP
#define ELYSIAN_RENDERABLE_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_renderable.hpp>
#include <AssetIO/proxy/elysian_component_proxy.hpp>






namespace elysian::lua {
void registerRenderable(sol::state_view state);
}

#endif // ELYSIAN_RENDERABLE_PROXY_HPP

