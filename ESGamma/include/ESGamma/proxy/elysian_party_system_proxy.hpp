/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_PARTY_SYSTEM_PROXY_LUA
#define ELYSIAN_PARTY_SYSTEM_PROXY_LUA

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/system/elysian_party_system.hpp>






namespace elysian::lua {
void registerPartySystem(sol::state_view state);
}

#endif // ELYSIAN_PARTY_SYSTEM_PROXY_LUA

