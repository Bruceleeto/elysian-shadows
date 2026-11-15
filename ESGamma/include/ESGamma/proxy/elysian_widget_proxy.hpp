/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_WIDGET_PROXY_HPP
#define ELYSIAN_WIDGET_PROXY_HPP

#include <ESGamma/gui/elysian_widget.hpp>
#include <ESGamma/gui/elysian_entity_widget.hpp>
#include <ESGamma/gui/elysian_label_widget.hpp>
#include <AssetIO/utility/elysian_lua_binding.hpp>












namespace elysian::lua {
void registerWidget(sol::state_view state);
void registerGroupWidget(sol::state_view state);
void registerEntityWidget(sol::state_view state);
void registerLabelWidget(sol::state_view state);
}

#endif // ELYSIAN_WIDGET_PROXY_HPP
