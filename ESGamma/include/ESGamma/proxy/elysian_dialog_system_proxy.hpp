/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_DIALOG_SYSTEM_PROXY_HPP
#define ELYSIAN_DIALOG_SYSTEM_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/system/elysian_dialog_system.hpp>








namespace elysian::lua {
void registerDialogSystem(sol::state_view state);
}

#endif // ELYSIAN_DIALOG_SYSTEM_PROXY_HPP

