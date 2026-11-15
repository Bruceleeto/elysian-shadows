/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_CAMERA_SYSTEM_PROXY_HPP
#define ELYSIAN_CAMERA_SYSTEM_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <ESGamma/crtp/elysian_camera_system.hpp>
#include <AssetIO/proxy/elysian_instance_transform_proxy.hpp>











namespace elysian::lua {
void registerCameraKeyFrame(sol::state_view state);
void registerCameraSystem(sol::state_view state);
}

#endif // ELYSIAN_CAMERA_SYSTEM_PROXY_HPP

