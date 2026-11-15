/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_DIALOG_PROXY_HPP
#define ELYSIAN_DIALOG_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_dialog.hpp>
#include <AssetIO/crtp/elysian_entity.hpp>














namespace elysian::lua {
void registerDialogScreen(sol::state_view state);
void registerDialogText(sol::state_view state);
void registerDialogOption(sol::state_view state);
void registerDialogPrompt(sol::state_view state);
void registerDialog(sol::state_view state);
}

#endif // ELYSIAN_DIALOG_PROXY_HPP

