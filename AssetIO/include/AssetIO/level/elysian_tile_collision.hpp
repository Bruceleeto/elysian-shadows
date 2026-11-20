/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_TILE_COLLISION_HPP
#define ELYSIAN_TILE_COLLISION_HPP
#include <AssetIO/level/elysian_tile_entry.hpp>
#include <AssetIO/level/elysian_tile.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace elysian {
// Painted squares are independent of tile faces and elevation slices. Project
// the character footprint into the overlay's map coordinates (world Y - feet Z),
// then lift each marked square back to that exact Z, including its fractional part.
// This needs one layer-zero bit test per overlapping square, at any height.
template<class Read, class Visit>
void visitBlockedTiles(const GYVector4 (&vertices)[4], int width, int height,
                       bool character, Read read, Visit visit) {
    if(!character || width<=0 || height<=0) return;
    const float feetZ=vertices[3].z;
    float left=vertices[0].x, right=left;
    float nearY=vertices[0].y-feetZ, farY=nearY;
    for(const auto& v:vertices) {
        left=std::min(left,v.x); right=std::max(right,v.x);
        nearY=std::min(nearY,v.y-feetZ); farY=std::max(farY,v.y-feetZ);
    }
    // Tiles are centred at multiples of 32, extending 16 units each way.
    if(!std::isfinite(left) || !std::isfinite(right) || !std::isfinite(nearY) || !std::isfinite(farY) ||
       right < -16 || left >= width*32.0f-16 || farY < -16 || nearY >= height*32.0f-16) return;
    const int firstX=static_cast<int>(std::floor((std::max(left,-16.0f)+16)/32));
    const int lastX=std::min(width-1,static_cast<int>(std::floor((std::min(right,width*32.0f-16)+16)/32)));
    const int firstY=static_cast<int>(std::floor((std::max(nearY,-16.0f)+16)/32));
    const int lastY=std::min(height-1,static_cast<int>(std::floor((std::min(farY,height*32.0f-16)+16)/32)));
    for(int y=firstY;y<=lastY;++y) {
        for(int x=firstX;x<=lastX;++x) {
            if(read(x,y).getBlocked()) visit(x*32.0f,y*32.0f+feetZ);
        }
    }
}

struct BlockedTileContact {
    GYVector2 normal{};
    float depth=0;
};

// Resolve the union of marked squares, not each square independently. On each
// axis find the first translation where the entire footprint is clear. A shared
// tile edge is never an exit: if the next tile overlaps, continue to its boundary.
// The four SAT axes retain the actual footprint of rotated box characters.
// No map-wide mesh, heap allocations or changes to ordinary tile contacts.
template<class Read>
BlockedTileContact blockedTileContact(const GYVector4 (&vertices)[4], int width, int height,
                                      bool character, Read read) {
    if(!character) return {};
    constexpr float tolerance=0.001f;
    const GYVector2 directions[]={{-1,0},{1,0},{0,-1},{0,1}};
    const GYVector2 axes[]={{1,0},{0,1},
        {vertices[1].y-vertices[0].y,vertices[0].x-vertices[1].x},
        {vertices[3].y-vertices[0].y,vertices[0].x-vertices[3].x}};
    float low[4],high[4],radius[4],epsilon[4];
    for(unsigned a=0;a<4;++a) {
        low[a]=high[a]=vertices[0].x*axes[a].x+vertices[0].y*axes[a].y;
        for(const auto& v:vertices) {
            const float value=v.x*axes[a].x+v.y*axes[a].y;
            low[a]=std::min(low[a],value); high[a]=std::max(high[a],value);
        }
        radius[a]=16*(std::abs(axes[a].x)+std::abs(axes[a].y));
        epsilon[a]=tolerance*std::hypot(axes[a].x,axes[a].y);
    }
    const auto exits=[&](GYVector2 direction,float distance,float (&result)[4]) {
        GYVector4 shifted[4];
        for(unsigned v=0;v<4;++v) {
            shifted[v]=vertices[v];
            shifted[v].x+=direction.x*distance; shifted[v].y+=direction.y*distance;
        }
        bool overlaps=false;
        visitBlockedTiles(shifted,width,height,true,read,[&](float x,float y) {
            float tileLow[4],tileHigh[4];
            for(unsigned a=0;a<4;++a) {
                const float center=x*axes[a].x+y*axes[a].y;
                tileLow[a]=center-radius[a]; tileHigh[a]=center+radius[a];
                const float shift=(direction.x*axes[a].x+direction.y*axes[a].y)*distance;
                if(radius[a]>0 && (high[a]+shift<=tileLow[a]+epsilon[a] ||
                                  low[a]+shift>=tileHigh[a]-epsilon[a])) return;
            }
            overlaps=true;
            for(unsigned d=0;d<4;++d) {
                float exit=std::numeric_limits<float>::infinity();
                for(unsigned a=0;a<4;++a) {
                    const float speed=directions[d].x*axes[a].x+directions[d].y*axes[a].y;
                    if(speed>0) exit=std::min(exit,(tileHigh[a]-low[a])/speed);
                    else if(speed<0) exit=std::min(exit,(tileLow[a]-high[a])/speed);
                }
                result[d]=std::max(result[d],exit);
            }
        });
        return overlaps;
    };
    float initial[4]={};
    if(!exits({},0,initial)) return {};
    BlockedTileContact best{{},std::numeric_limits<float>::infinity()};
    for(unsigned d=0;d<4;++d) {
        float distance=initial[d];
        // Each probe leaves at least one intersected tile behind.
        // Leaving the finite map is always a clear candidate, even for a body
        // spawned inside a completely painted region.
        for(uint64_t step=0;step<=uint64_t(width)*uint64_t(height) && distance<best.depth;++step) {
            float next[4]={};
            if(!exits(directions[d],distance,next)) { best={directions[d],distance}; break; }
            distance=std::max(next[d],distance+tolerance);
        }
    }
    return std::isfinite(best.depth)?best:BlockedTileContact{};
}

}
#endif
