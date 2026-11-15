/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_EXAMINABLE_SYSTEM_PROXY_HPP
#define ELYSIAN_EXAMINABLE_SYSTEM_PROXY_HPP

#include <ESGamma/system/elysian_examinable_system.hpp>
#include <AssetIO/utility/elysian_lua_binding.hpp>






namespace elysian::lua {
void registerExaminableSystem(sol::state_view state);
}

#endif // ELYSIAN_EXAMINABLE_SYSTEM_PROXY_HPP
