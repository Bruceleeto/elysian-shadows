/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_SKELETON_PROXY_HPP
#define ELYSIAN_SKELETON_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/crtp/elysian_skeleton.hpp>
#include <AssetIO/utility/elysian_lua_values.hpp>

using namespace std;

















namespace elysian::lua {
void registerAnimationTrack(sol::state_view state);
void registerAnimation(sol::state_view state);
void registerSkeletonAttrib(sol::state_view state);
void registerSkeleton(sol::state_view state);
}

#endif // ELYSIAN_SKELETON_PROXY_HPP

