/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_BIT_MASK_PROXY_HPP
#define ELYSIAN_BIT_MASK_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/utility/elysian_bit_mask.hpp>



//Proxy over arithmetic operators as bitwise operators (* and +)






namespace elysian::lua {
void registerBitMask32(sol::state_view state);
void registerBitMask64(sol::state_view state);
}

#endif // ELYSIAN_BIT_MASK_PROXY_HPP

