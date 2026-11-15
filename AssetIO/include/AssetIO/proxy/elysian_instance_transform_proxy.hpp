/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_INSTANCE_TRANSFORM_PROXY_HPP
#define ELYSIAN_INSTANCE_TRANSFORM_PROXY_HPP

#include <AssetIO/utility/elysian_lua_binding.hpp>

#include <AssetIO/primitive/elysian_instance_transform.hpp>
#include <libGyro/gyro_matrix_api.h>






















namespace elysian::lua {
void registerGYVector2(sol::state_view state);
void registerVector2(sol::state_view state);
void registerGYVector3(sol::state_view state);
void registerVector3(sol::state_view state);
void registerGYVector4(sol::state_view state);
void registerVector4(sol::state_view state);
void registerQuaternion(sol::state_view state);
void registerInstanceTransform(sol::state_view state);
}

#endif // ELYSIAN_INSTANCE_TRANSFORM_PROXY_HPP

