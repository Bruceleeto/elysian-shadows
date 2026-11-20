/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <libGyro/gyro_matrix_api.h>
#include <cmath>
#include <vector>

namespace elysian {
struct PartyTrailPoint {
    GYVector3 position{};
    float verticalVelocity=0;
};

inline bool recordPartyTrail(std::vector<PartyTrailPoint>& points,const PartyTrailPoint& point) {
    if(points.empty()) return false;
    const auto& last=points.front().position;
    if(std::abs(last.x-point.position.x)<1 && std::abs(last.y-point.position.y)<1 &&
       std::abs(last.z-point.position.z)<1) return false;
    for(size_t i=points.size()-1;i>0;--i) points[i]=points[i-1];
    points.front()=point;
    return true;
}
}
