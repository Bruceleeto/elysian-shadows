/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <libGyro/gyro_matrix_api.h>
#include <cmath>

namespace elysian {
// Camera position is already in the orthographic view's height-sheared space.
inline GYVector2 cameraEntityDistance(const GYVector3& worldPosition,const GYVector3& cameraPosition) {
    return {std::abs(worldPosition.x-cameraPosition.x),
            std::abs((worldPosition.y-worldPosition.z)-cameraPosition.y)};
}
}
