/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <AssetIO/level/elysian_terrain_geometry.hpp>

// The view describes placed geometry, independent of artwork metadata.
namespace WalkHeight {
using Tile=elysian::TileEntry;
inline Tile decode(uint32_t value) {return Tile(value);}
struct Surface {float top,left,right,nearY,farY;bool ramp=false;};
inline std::optional<Surface> surface(Tile tile,int x,int y) {
    if(!tile.getSolid())return {};
    const auto q=elysian::terrain::geometry(tile,x,y);
    const float cx=x*32.f,cy=(y+tile.getElev())*32.f;
    const auto top=elysian::terrain::surfaceHeight(q,cx,cy);
    if(!top)return {};
    return Surface{*top,cx-16,cx+16,cy-16,cy+16,tile.getPlane()==GY_RECT_PLANE_FRONT_SLOPE_45};
}
}
