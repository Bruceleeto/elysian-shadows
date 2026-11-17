/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <array>
#include <cassert>
#include <AssetIO/level/elysian_tile.hpp>
#include <libGyro-Kinetic/gyro_kinetic_physics.h>
#include <libGyro-Kinetic/gyro_kinetic_kinetic.h>

// Editor analysis only. Shared by the decoration renderer and offline tests;
// deliberately does not replace or change the runtime physics.
namespace WalkHeight {
constexpr float TileSize = 32;
constexpr float LandingTolerance = 16;
// Measured by tests/walk_height.cpp using the real gyIntegrate at 60 Hz:
// CharacterBase::jump: impulse 330000; Julien invMass .002, damping.z .5;
// GravityForceGenerator: -2000. gyIntegrate damps velocity twice each step.
constexpr float JulienJumpApex = 80.32757f;
constexpr float JulienLandingReach = JulienJumpApex + LandingTolerance;

struct Tile {
    unsigned index=0, elevation=0, plane=0;
    bool standOn=false;
    unsigned collider=0, orientation=0;
    bool operator==(const Tile& b) const {
        return index==b.index && elevation==b.elevation && plane==b.plane &&
            standOn==b.standOn && collider==b.collider && orientation==b.orientation;
    }
    bool front() const { return plane==1; } // TileEntry::getNormal, not plane!=0
    float top() const { return (elevation+(front()?1:0))*TileSize; }
};
inline Tile decode(uint32_t value, bool standOn, unsigned collider) {
    return {value&511u,(value>>12)&31u,(value>>17)&15u,standOn,collider,(value>>9)&7u};
}
struct Support {
    float top=0; // integrate3D always retains an implicit ground plane at zero.
    int layer=-1;
};

// Literal floor-search arithmetic from RigidBodySystem::integrate3D. Read(layer,
// column,row) supplies a Tile. Reject invalid coordinates instead of reproducing
// out-of-map flattened-array access. No collider test: integrate3D has none.
template<class Read>
std::optional<Support> support(int width, int height, Read read,
                             float worldX, float worldY, float bodyZ, float velocityZ=0) {
    if(velocityZ>0 || bodyZ<=-32 || !std::isfinite(bodyZ)) return std::nullopt;
    const unsigned elev=static_cast<unsigned>(bodyZ/32);
    worldY+=32;
    const float col=(worldX+16)/32, row=worldY/32-elev-1;
    if(col<0 || row<0 || col>=width || row>=height) return std::nullopt;
    Support result;
    for(unsigned layer=0;layer<3;++layer) {
        const Tile entry=read(layer,static_cast<int>(col),static_cast<int>(row));
        if(!entry.standOn) continue;
        if(entry.front() && (static_cast<int>(worldY)+16)/32>static_cast<int>(worldY)/32) continue;
        const float newTop=entry.top();
        if(newTop>result.top && std::abs(newTop-bodyZ)<=16) result={newTop,static_cast<int>(layer)};
    }
    return result;
}

struct Surface {
    float top, left, right, nearY, farY;
};
inline std::optional<Surface> surface(const Tile& tile, int x, int y) {
    if(!tile.index || !tile.standOn || tile.plane>6) return std::nullopt;
    const float top=tile.top();
    // Invert the runtime lookup at z=top. FRONT supports only the first half
    // of this interval. Render position and support position are NOT identical.
    const float nearY=y*TileSize+top;
    return Surface{top,x*TileSize-16,x*TileSize+16,nearY,nearY+(tile.front()?16:32)};
}
// The default Julien collider is axis-aligned: size 32x50, relative size .4;
// EntityBase::pushViewTransform replaces size.z with size.y for FRONT entities.
// Inputs below are collider-centre coordinates (entity y + 5), as in integrate3D.
constexpr float JulienHalfWidth=6.4f, JulienHalfDepth=10, JulienBodyHeight=20;
struct Rect { float left,right,nearY,farY; };
struct Rects { std::array<Rect,2> boxes{}; unsigned count=0; };
using Collider=elysian::Tile::COLLIDER_TYPE;
inline Collider orientedCollider(const Tile& tile) {
    if(tile.collider>=elysian::Tile::COLLIDER_MAX || tile.orientation>=8) return elysian::Tile::COLLIDER_NONE;
    return elysian::Tile::reorientCollider(static_cast<Collider>(tile.collider),
                                         static_cast<GY_RECT_TEX_ORIENT>(tile.orientation));
}
// Exact rectangles from CollisionSystem::_applyTileColliderTransform and the
// fine pass's decomposition. Its default branch ignores 45-degree triangles.
inline Rects colliderRects(Collider type,float x=0,float y=0) {
    using T=elysian::Tile;
    Rects result;
    const auto add=[&](float l,float r,float n,float f) { result.boxes[result.count++]={x+l,x+r,y+n,y+f}; };
    switch(type) {
    case T::COLLIDER_FULL: add(-16,16,-16,16); break;
    case T::COLLIDER_HALF_E: add(0,16,-16,16); break;
    case T::COLLIDER_HALF_N: add(-16,16,-16,0); break;
    case T::COLLIDER_HALF_W: add(-16,0,-16,16); break;
    case T::COLLIDER_HALF_S: add(-16,16,0,16); break;
    case T::COLLIDER_QUAD_SE: add(0,16,0,16); break;
    case T::COLLIDER_QUAD_NE: add(0,16,-16,0); break;
    case T::COLLIDER_QUAD_NW: add(-16,0,-16,0); break;
    case T::COLLIDER_QUAD_SW: add(-16,0,0,16); break;
    case T::COLLIDER_L_NE: add(-16,16,-16,0); add(0,16,0,16); break;
    case T::COLLIDER_L_NW: add(-16,16,-16,0); add(-16,0,0,16); break;
    case T::COLLIDER_L_SW: add(-16,16,0,16); add(-16,0,-16,0); break;
    case T::COLLIDER_L_SE: add(-16,16,0,16); add(0,16,-16,0); break;
    case T::COLLIDER_QUADS_SE_NW: add(0,16,0,16); add(-16,0,-16,0); break;
    case T::COLLIDER_QUADS_SW_NE: add(-16,0,0,16); add(0,16,-16,0); break;
    default: break;
    }
    return result;
}
inline bool overlaps(const Rect& a,const Rect& b) {
    // gyCollisionOBBvOBB's SAT for axis-aligned rectangles: touching is not contact.
    return a.left<b.right && a.right>b.left && a.nearY<b.farY && a.farY>b.nearY;
}
template<class Read>
Collider combinedCollider(int width,int height,Read read,int x,int row,int elevation) {
    using T=elysian::Tile;
    auto result=T::COLLIDER_NONE;
    if(x<0 || x>=width || row<0 || row>=height) return result;
    for(unsigned layer=0;layer<3;++layer) {
        const auto tile=read(layer,x,row);
        if(static_cast<int>(tile.elevation)!=elevation) continue;
        result=T::combineColliders(result,orientedCollider(tile));
    }
    return result;
}
template<class Read>
bool wallsBlock(int width,int height,Read read,float x,float y,float z) {
    const Rect body={x-JulienHalfWidth,x+JulienHalfWidth,y-JulienHalfDepth,y+JulienHalfDepth};
    // Coarse pass truncates the bounding coordinates and checks every occupied
    // elevation, not just the feet. Preserve its row adjustment and fractional Z.
    const int startX=static_cast<int>(body.left), endX=static_cast<int>(body.right);
    const int startY=static_cast<int>(body.nearY), endY=static_cast<int>(body.farY);
    // ELYSIAN_COLLISION_SYSTEM_PROTECTED is enabled in the engine header.
    if(startX<0 || startY<0 || endX>width*32 || endY>height*32) return false;
    const int startElev=std::clamp(static_cast<int>(z/32),0,31);
    const int endElev=std::clamp(static_cast<int>((z+JulienBodyHeight)/32),0,31);
    const float zOffset=z-startElev*32;
    for(int elev=startElev;elev<=endElev;++elev) {
        for(int row=startY/32-(elev-startElev)-1;row<=endY/32-(elev-startElev)+1;++row) {
            if(row<0) continue;
            for(int col=startX/32;col<=endX/32+1;++col) {
                const auto type=combinedCollider(width,height,read,col,row-elev,elev);
                const auto shapes=colliderRects(type,col*32.0f,row*32.0f-zOffset);
                for(unsigned i=0;i<shapes.count;++i) if(overlaps(body,shapes.boxes[i])) return true;
            }
        }
    }
    return false;
}

enum Edge { Left, Right, Near, Far };
enum class Barrier { NoSurface, Reachable, Blocked };
struct Point { float x,y; };
inline bool inside(const Surface& s,float x,float y) {
    return x>=s.left && x<s.right && y>=s.nearY && y<s.farY;
}

// One normal, non-dashing crossing, checked with actual gyIntegrate at 60 Hz.
// A successful trace must land on the higher side, stay supported at the NEW
// height (not merely snap for one frame), and avoid tile contacts throughout.
// Collision response/sliding is not approximated: a contacting trace is rejected.
template<class Read>
bool trace(int width,int height,Read read,Point start,Point direction,float low,float high,
           float boundary,bool xAxis,const Surface& destination,bool jumping,bool running,int delay) {
    constexpr float dt=1.0f/60;
    GYStatic stat{};
    stat.inverseMass=.002f; stat.damping={.001f,.001f,.5f}; stat.angDamping=1;
    GYKinetic body{};
    const float drag=std::pow(.001f,2*dt);
    const float runVelocity=2000*dt*drag/(1-drag);
    if(running) body.vel={direction.x*runVelocity,direction.y*runVelocity,0};
    GYVector3 position={start.x,start.y,low};
    float angle=0;
    unsigned settled=0;
    const auto initial=support(width,height,read,start.x,start.y,low);
    if(!initial || initial->top!=low || wallsBlock(width,height,read,start.x,start.y,low)) return false;
    for(int frame=0;frame<60;++frame) {
        const float force=frame>=delay && !settled?1000000.0f:0.0f;
        body.forceAccum={direction.x*force,direction.y*force,-2000/stat.inverseMass};
        if(jumping && frame==0) body.forceAccum.z+=330000/dt;
        gyIntegrate(&body,&stat,dt,&position,&angle);
        auto floor=support(width,height,read,position.x,position.y,position.z,body.vel.z);
        bool landed=false;
        if(floor && position.z<=floor->top) {
            position.z=floor->top; body.vel.z=0; landed=true;
        }
        // Engine order: integrate -> floor snap/update collider -> wall detection.
        if(wallsBlock(width,height,read,position.x,position.y,position.z)) return false;
        const float along=xAxis?position.x:position.y;
        const float sign=xAxis?direction.x:direction.y;
        const bool crossed=(along-boundary)*sign>0;
        const auto stable=support(width,height,read,position.x,position.y,position.z);
        bool onDestination=inside(destination,position.x,position.y);
        // A jump onto a raised roof can land one cell inward. Require continuous
        // support at the destination height back to the tested boundary, so a
        // landing on an unrelated surface beyond a gap is not counted.
        if(high>low && crossed && stable && stable->top==high) {
            onDestination=true;
            for(float p=boundary+sign*.25f;(along-p)*sign>0;p+=sign*4) {
                const auto step=support(width,height,read,xAxis?p:position.x,xAxis?position.y:p,high);
                if(!step || step->top!=high) { onDestination=false; break; }
            }
        }
        if(landed && crossed && onDestination && position.z==high && stable && stable->top==high) {
            if(++settled==3) return true;
        } else settled=0;
        if(position.z<low-16 || (along-boundary)*sign>96) return false;
        if(jumping && frame>20 && landed && position.z<high) return false;
    }
    return false;
}

template<class Read>
bool crossing(int width,int height,Read read,const Surface& a,const Surface& b,Edge side) {
    const bool forward=a.top<=b.top;
    const Surface& low=forward?a:b;
    const Surface& destination=forward?b:a;
    const float high=forward?b.top:a.top;
    if(high-low.top>JulienLandingReach) return false;
    const bool xAxis=side==Left || side==Right;
    float sign=(side==Left || side==Near)?-1.0f:1.0f;
    if(!forward) sign=-sign;
    const Point direction=xAxis?Point{sign,0}:Point{0,sign};
    const float boundary=xAxis?(sign>0?low.right:low.left):(sign>0?low.farY:low.nearY);
    const float tangentLo=xAxis?std::max(a.nearY,b.nearY):std::max(a.left,b.left);
    const float tangentHi=xAxis?std::min(a.farY,b.farY):std::min(a.right,b.right);
    if(tangentHi<=tangentLo) return false;
    for(float fraction:{.5f,.25f,.75f}) for(float setback:{.25f,8.0f}) {
        const float tangent=tangentLo+(tangentHi-tangentLo)*fraction;
        const Point start=xAxis?Point{boundary-sign*setback,tangent}:Point{tangent,boundary-sign*setback};
        if(!inside(low,start.x,start.y)) continue;
        if(high==low.top && trace(width,height,read,start,direction,low.top,high,boundary,xAxis,destination,false,false,0)) return true;
        for(bool running:{true,false}) for(int delay:{0,4,8})
            if(trace(width,height,read,start,direction,low.top,high,boundary,xAxis,destination,true,running,delay)) return true;
    }
    return false;
}

// Neighbours are adjacent in world XY, not in the oblique picture. For layered
// terrain test the nearest-height neighbour(s); the edge always tests low -> high.
template<class Read>
Barrier edge(int width,int height,Read read,const Surface& from,Edge side) {
    float x=(from.left+from.right)*.5f, y=(from.nearY+from.farY)*.5f;
    switch(side) {
    case Left: x=from.left-.25f; break;
    case Right: x=from.right+.25f; break;
    case Near: y=from.nearY-.25f; break;
    case Far: y=from.farY+.25f; break;
    }
    const int column=static_cast<int>(std::floor((x+16)/32));
    if(column<0 || column>=width || y<0) return Barrier::NoSurface;
    float closest=std::numeric_limits<float>::infinity();
    std::array<Surface,99> candidates{};
    unsigned count=0;
    for(int h=0;h<=32;++h) {
        const int row=static_cast<int>(y/32)-h;
        if(row<0 || row>=height) continue;
        for(unsigned layer=0;layer<3;++layer) {
            const auto candidate=surface(read(layer,column,row),column,row);
            if(!candidate || y<candidate->nearY || y>=candidate->farY) continue;
            const auto floor=support(width,height,read,x,y,candidate->top);
            if(!floor || floor->top!=candidate->top) continue;
            const float diff=std::abs(from.top-candidate->top);
            if(diff<closest) { count=0; closest=diff; }
            if(diff==closest) candidates[count++]=*candidate;
        }
    }
    if(!count) return Barrier::NoSurface;
    for(unsigned i=0;i<count;++i) if(crossing(width,height,read,from,candidates[i],side)) return Barrier::Reachable;
    return Barrier::Blocked;
}
}
