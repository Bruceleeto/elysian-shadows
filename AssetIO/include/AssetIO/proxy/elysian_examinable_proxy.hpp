/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_EXAMINABLE_PROXY_HPP
#define ELYSIAN_EXAMINABLE_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/proxy/elysian_component_proxy.hpp>
#include <AssetIO/proxy/elysian_dialog_proxy.hpp>
#include <AssetIO/crtp/elysian_examinable.hpp>







namespace elysian::lua {
void registerExaminable(sol::state_view state);
}

#endif // ELYSIAN_EXAMINABLE_PROXY_HPP

