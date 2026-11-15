/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_LAYOUT_PROXY_HPP
#define ELYSIAN_LAYOUT_PROXY_HPP

#include <ESGamma/gui/elysian_layout.hpp>
#include <AssetIO/utility/elysian_lua_binding.hpp>














namespace elysian::lua {
void registerTableLayout(sol::state_view state);
void registerSingleAxisLayout(sol::state_view state);
void registerHorizontalLayout(sol::state_view state);
void registerVerticalLayout(sol::state_view state);
}

#endif // ELYSIAN_LAYOUT_PROXY_HPP
