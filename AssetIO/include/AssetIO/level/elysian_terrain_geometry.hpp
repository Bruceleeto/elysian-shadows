/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_TERRAIN_GEOMETRY_HPP
#define ELYSIAN_TERRAIN_GEOMETRY_HPP
#include <AssetIO/level/elysian_tile_entry.hpp>
#include <common/gyro_tile_geometry.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <optional>

namespace elysian::terrain {
constexpr float tolerance=.001f;
struct Vec {
    float x=0,y=0,z=0;
    Vec operator+(Vec b) const {return {x+b.x,y+b.y,z+b.z};}
    Vec operator-(Vec b) const {return {x-b.x,y-b.y,z-b.z};}
    Vec operator*(float s) const {return {x*s,y*s,z*s};}
};
inline float dot(Vec a,Vec b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline Vec cross(Vec a,Vec b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline float length(Vec v){return std::sqrt(dot(v,v));}
inline Vec abs(Vec v){return {std::abs(v.x),std::abs(v.y),std::abs(v.z)};}
inline bool finite(Vec v){return std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);}
using Quad=std::array<Vec,4>;
inline Quad geometry(TileEntry entry,int x,int y) {
    Quad result{};
    if(entry.getPlane()>=GY_RECT_PLANE_MAX)return result;
    for(unsigned i=0;i<4;++i) {
        const auto& v=gyTilePlaneVerts[entry.getPlane()][i];
        result[i]={(x+v.x)*32,(y+entry.getElev()+v.y)*32,(entry.getElev()+v.z)*32};
    }
    return result;
}
// A vertical prism with the body's real rotated footprint. No GL objects or
// dynamic mesh allocation: quads are reconstructed only near the swept body.
struct Body {
    Vec center,halfX,halfY;
    float halfZ=0;
    float radius(Vec axis) const {return std::abs(dot(halfX,axis))+std::abs(dot(halfY,axis))+halfZ*std::abs(axis.z);}
    Vec extent() const {return abs(halfX)+abs(halfY)+Vec{0,0,halfZ};}
};
// The runtime box stores four XY corners, with the top Z on the first
// edge and the bottom Z on the opposite edge. Reconstruct its vertical prism.
inline Body bodyFromVertices(const GYVector4 (&v)[4]) {
    Vec center{};float low=v[0].z,high=v[0].z;
    for(const auto& p:v){center=center+Vec{p.x*.25f,p.y*.25f,0};low=std::min(low,p.z);high=std::max(high,p.z);}
    center.z=(low+high)*.5f;
    return {center,{(v[1].x-v[0].x)*.5f,(v[1].y-v[0].y)*.5f,0},
        {(v[3].x-v[0].x)*.5f,(v[3].y-v[0].y)*.5f,0},(high-low)*.5f};
}
struct Hit {float time=1,depth=0;Vec normal;bool contact=false;};

// Continuous SAT for a translating box and a zero-thickness convex quad.
// Face normals plus edge cross-products cover floors, walls, diagonal walls
// and ramps. Touching while moving tangentially is not a collision: adjacent
// coplanar tiles must not introduce seams or invisible side walls.
inline Hit sweep(const Body& body,Vec movement,const Quad& quad) {
    const Vec u=quad[1]-quad[0],v=quad[2]-quad[0];
    const Vec bx=body.halfX,by=body.halfY,bz{0,0,1};
    const std::array<Vec,10> axes={cross(bx,by),cross(by,bz),cross(bz,bx),cross(u,v),
        cross(bx,u),cross(bx,v),cross(by,u),cross(by,v),cross(bz,u),cross(bz,v)};
    float entry=-std::numeric_limits<float>::infinity(),exit=1,depth=std::numeric_limits<float>::infinity();
    Vec entryNormal{},pushNormal{};
    bool penetrating=true;
    for(auto axis:axes) {
        const float size=length(axis);if(size<1e-6f)continue;axis=axis*(1/size);
        // Work relative to a nearby vertex to retain precision far from origin.
        float low=0,high=0;
        for(const auto& p:quad){const float q=dot(p-quad[0],axis);low=std::min(low,q);high=std::max(high,q);}
        const float center=dot(body.center-quad[0],axis),radius=body.radius(axis),speed=dot(movement,axis);
        const float down=center+radius-low,up=high-center+radius;
        if(down<=tolerance || up<=tolerance)penetrating=false;
        const float overlap=std::min(down,up);
        if(overlap<depth){depth=overlap;pushNormal=axis*(down<up?-1.f:1.f);}
        if(std::abs(speed)<1e-7f) {
            if(down<=tolerance || up<=tolerance)return {};
            continue;
        }
        float near=(low-radius-center)/speed,far=(high+radius-center)/speed;
        Vec normal=axis*(speed>0?-1.f:1.f);
        if(near>far)std::swap(near,far);
        if(near>entry){entry=near;entryNormal=normal;}
        exit=std::min(exit,far);
        if(entry>exit)return {};
    }
    if(penetrating && depth>tolerance && std::isfinite(depth))return {0,depth,pushNormal,true};
    // A body resting on a floor can start a few ulps inside it at large world coordinates.
    // Judge that tolerance in world units, not as a fraction of this frame's movement.
    if(entry<0) {
        if(depth>=-tolerance && depth<=tolerance && dot(movement,pushNormal)<0)
            return {0,std::max(0.f,depth),pushNormal,true};
        return {};
    }
    if(entry>1 || exit<0 || dot(movement,entryNormal)>=0)return {};
    return {std::max(0.f,entry),0,entryNormal,true};
}

template<class Read,class Visit>
void visit(int width,int height,unsigned layers,const Body& body,Vec movement,Read read,Visit callback) {
    const auto e=body.extent(),end=body.center+movement;
    const Vec lo{std::min(body.center.x,end.x)-e.x,std::min(body.center.y,end.y)-e.y,std::min(body.center.z,end.z)-e.z};
    const Vec hi{std::max(body.center.x,end.x)+e.x,std::max(body.center.y,end.y)+e.y,std::max(body.center.z,end.z)+e.z};
    if(width<=0 || height<=0 || !finite(lo) || !finite(hi))return;
    if(hi.x < -16 || lo.x>(width-.5f)*32 || hi.y < -16 || lo.y>(height+30.5f)*32 || hi.z<0 || lo.z>1024)return;
    const int firstX=std::clamp(int(std::floor((std::clamp(lo.x,-32.f,width*32.f)-16)/32)),0,width-1);
    const int lastX=std::clamp(int(std::floor((std::clamp(hi.x,-32.f,width*32.f)+16)/32)),0,width-1);
    const int firstZ=std::clamp(int(std::floor(std::clamp(lo.z,-32.f,1024.f)/32))-1,0,31);
    const int lastZ=std::clamp(int(std::floor(std::clamp(hi.z,-32.f,1024.f)/32)),0,31);
    for(int elev=firstZ;elev<=lastZ;++elev) {
        const int firstY=std::clamp(int(std::floor((std::clamp(lo.y,-32.f,(height+32)*32.f)-16)/32))-elev,0,height-1);
        const int lastY=std::clamp(int(std::floor((std::clamp(hi.y,-32.f,(height+32)*32.f)+16)/32))-elev,0,height-1);
        for(int y=firstY;y<=lastY;++y)for(int x=firstX;x<=lastX;++x)for(unsigned l=0;l<layers;++l) {
            const auto tile=read(l,x,y);
            if(tile.getSolid() && tile.getElev()==elev && tile.getPlane()<GY_RECT_PLANE_MAX)
                callback(geometry(tile,x,y));
        }
    }
}
struct Motion {Vec center,velocity;bool grounded=false;unsigned contacts=0;float verticalImpact=0;};
template<class Read>
Motion move(int width,int height,unsigned layers,Read read,Body body,Vec movement,Vec velocity,float restitution=0,bool walking=false,float restSpeed=0) {
    Motion result{body.center,velocity};
    if(!finite(body.center)||!finite(movement)||!finite(velocity))return result;
    for(unsigned iteration=0;iteration<12;++iteration) {
        Hit nearest;
        visit(width,height,layers,body,movement,read,[&](const Quad& q){
            const auto hit=sweep(body,movement,q);
            if(hit.contact && (!nearest.contact || hit.time<nearest.time ||
               (hit.time==nearest.time && hit.depth>nearest.depth)))nearest=hit;
        });
        if(!nearest.contact){body.center=body.center+movement;break;}
        ++result.contacts;
        if(nearest.normal.z>.5f)result.verticalImpact=std::max(result.verticalImpact,-result.velocity.z);
        body.center=body.center+movement*nearest.time+nearest.normal*(nearest.depth+tolerance);
        movement=movement*(1-nearest.time);
        if(walking && nearest.normal.z>.5f && result.velocity.z<=0) {
            // A grounded character follows walkable slopes without gravity
            // accelerating it downhill. Preserve horizontal walking input;
            // the geometric contact determines the required vertical rise.
            movement.z=-(nearest.normal.x*movement.x+nearest.normal.y*movement.y)/nearest.normal.z;
            result.velocity.z=0;
        } else {
            const float into=dot(movement,nearest.normal);
            if(into<0)movement=movement-nearest.normal*into;
            const float speed=dot(result.velocity,nearest.normal);
            if(speed<0)result.velocity=result.velocity-nearest.normal*((1+restitution)*speed);
        }
        if(nearest.normal.z>.5f && result.velocity.z>0 && result.velocity.z<restSpeed)result.velocity.z=0;
        // Keep testing after a contact, even with no remaining motion: spawn
        // penetration or another wall at a corner can require another normal.
    }
    result.grounded=false;
    if(result.velocity.z<=0) {
        visit(width,height,layers,body,{0,0,-.01f},read,[&](const Quad& q){
            const auto contact=sweep(body,{0,0,-.01f},q);
            if(contact.contact && contact.normal.z>.5f)result.grounded=true;
        });
    }
    result.center=body.center;
    return result;
}
// Exact vertical intersection for editor height display. Upright/diagonal
// walls have no walkable horizontal area and never manufacture a ledge.
inline std::optional<float> surfaceHeight(const Quad& q,float x,float y) {
    const auto u=q[1]-q[0],v=q[2]-q[0];
    const float det=u.x*v.y-u.y*v.x;if(std::abs(det)<1e-6f)return {};
    const float dx=x-q[0].x,dy=y-q[0].y;
    const float a=(dx*v.y-dy*v.x)/det,b=(u.x*dy-u.y*dx)/det;
    if(a<0 || a>1 || b<0 || b>1)return {};
    return q[0].z+a*u.z+b*v.z;
}
struct Support {float top;int layer;};
// Vertical point probe for authoring and diagnostics. Runtime bodies use swept
// volumes above; this does not snap a body's feet or invent a step allowance.
template<class Read>
std::optional<Support> support(int width,int height,unsigned layers,Read read,
                              float x,float y,float z,float distance=.02f) {
    if(!finite({x,y,z}) || distance<0)return {};
    const float col=std::floor((x+16)/32);
    if(col<0 || col>=width)return {};
    std::optional<Support> result;
    float nearest=distance;
    for(unsigned elev=0;elev<=31;++elev) {
        const float row=std::floor((y+16)/32)-elev;
        if(row<0 || row>=height)continue;
        for(unsigned l=0;l<layers;++l) {
            const auto tile=read(l,int(col),int(row));
            if(!tile.getSolid() || tile.getElev()!=elev)continue;
            const auto top=surfaceHeight(geometry(tile,int(col),int(row)),x,y);
            if(top && std::abs(*top-z)<=nearest) {nearest=std::abs(*top-z);result=Support{*top,int(l)};}
        }
    }
    return result;
}

}
#endif
