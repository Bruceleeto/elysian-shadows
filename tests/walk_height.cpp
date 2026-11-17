/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#include <ESToolkit/core/tk_walk_height.h>
#include <libGyro-Kinetic/gyro_kinetic_physics.h>
#include <libGyro-Kinetic/gyro_kinetic_kinetic.h>
#include <array>
#include <random>
#include <vector>
#include <fstream>
#include <chrono>
#include <libGyro-Kinetic/gyro_kinetic_collision.h>
#include <libGyro-Kinetic/gyro_kinetic_contact.h>
#include <cstdio>
#include <cstdlib>

static void check(bool ok,const char* message) {
    if(!ok) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
using WalkHeight::Tile;
struct Map {
    static constexpr int width=12,height=16;
    std::array<Tile,width*height*3> tiles{};
    Tile& at(unsigned layer,int x,int y) { return tiles[layer*width*height+y*width+x]; }
    Tile operator()(unsigned layer,int x,int y) const { return tiles[layer*width*height+y*width+x]; }
};
static float jump(float dt) {
    GYKinetic body{};
    GYStatic stat{};
    stat.inverseMass=.002f; stat.damping={.001f,.001f,.5f}; stat.angDamping=1;
    GYVector3 position{};
    float angle=0, apex=0;
    for(int frame=0;frame<2000;++frame) {
        body.forceAccum.z=-2000/stat.inverseMass+(frame==0?330000/dt:0);
        gyIntegrate(&body,&stat,dt,&position,&angle);
        apex=std::max(apex,position.z);
        if(body.vel.z<=0) break;
    }
    return apex;
}
// Differential reference: construct the actual libGyro boxes with the engine's
// transform/decomposition, then use its SAT narrow phase, independent of the
// analysis helper's rectangle overlap implementation.
static bool referenceWalls(const Map& map,float x,float y,float z) {
    GYBoxCollider body{};
    body.collider.type=GY_COLLIDER_AABB;
    gyMatLoadIdentity(); gyMatTranslateScale(x,y,z,12.8f,20,20); gyCalcBoxWorldVert(&body);
    body.collider.worldPos={x,y,z};
    GYStatic stat{}; stat.inverseMass=.002f;
    gyColliderCurrent(&body.collider,&stat);
    const int startX=static_cast<int>(body.worldVert[0].x),endX=static_cast<int>(body.worldVert[1].x);
    const int startY=static_cast<int>(body.worldVert[0].y),endY=static_cast<int>(body.worldVert[3].y);
    if(startX<0 || startY<0 || endX>Map::width*32 || endY>Map::height*32) return false;
    const int first=std::clamp(static_cast<int>(body.worldVert[3].z/32),0,31);
    const int last=std::clamp(static_cast<int>(body.worldVert[0].z/32),0,31);
    const auto boxHit=[&](int col,int row,int part) {
        // _applyTileColliderTransform: x/y translation, x/y scale.
        static const float transform[10][4]={
            {0,0,0,0},{0,0,1,1},{.25f,0,.5f,1},{0,-.25f,1,.5f},
            {-.25f,0,.5f,1},{0,.25f,1,.5f},{.25f,.25f,.5f,.5f},
            {.25f,-.25f,.5f,.5f},{-.25f,-.25f,.5f,.5f},{-.25f,.25f,.5f,.5f}};
        GYBoxCollider tile{}; tile.collider.type=GY_COLLIDER_AABB;
        gyMatLoadIdentity(); gyMatTranslateScale(col*32.0f,row*32.0f-(z-first*32),5,32,32,32);
        gyMatTranslateScale(transform[part][0],transform[part][1],0,transform[part][2],transform[part][3],1);
        gyCalcBoxWorldVert(&tile);
        GYContact contact{}; int swapped=0;
        return gyCollision(&tile.collider,nullptr,&contact,&swapped)!=0;
    };
    for(int elev=first;elev<=last;++elev)
        for(int row=startY/32-(elev-first)-1;row<=endY/32-(elev-first)+1;++row)
            for(int col=startX/32;col<=endX/32+1;++col) {
                if(col<0 || col>=Map::width || row-elev<0 || row-elev>=Map::height) continue;
                auto type=elysian::Tile::COLLIDER_NONE;
                for(unsigned l=0;l<3;++l) {
                    auto t=map(l,col,row-elev);
                    if(static_cast<int>(t.elevation)==elev)
                        type=elysian::Tile::combineColliders(type,elysian::Tile::reorientCollider(
                            static_cast<elysian::Tile::COLLIDER_TYPE>(t.collider),static_cast<GY_RECT_TEX_ORIENT>(t.orientation)));
                }
                static const int parts[6][2]={{5,7},{3,6},{3,9},{5,8},{6,8},{9,7}};
                if(type>=1 && type<=9 && boxHit(col,row,type)) return true;
                if(type>=10 && type<=15 && (boxHit(col,row,parts[type-10][0]) || boxHit(col,row,parts[type-10][1]))) return true;
            }
    return false;
}
static void differentialCheck() {
    std::mt19937 random(87123);
    Map map;
    const auto number=[&](unsigned limit) { return std::uniform_int_distribution<unsigned>(0,limit-1)(random); };
    for(auto& tile:map.tiles) tile={1,number(5),number(7),false,number(16),number(8)};
    for(unsigned i=0;i<4000;++i) {
        const float x=32+static_cast<float>(random()%30000)/100;
        const float y=64+static_cast<float>(random()%36000)/100;
        const float z=static_cast<float>(random()%14500)/100;
        const auto read=[&](unsigned l,int col,int row){return map(l,col,row);};
        check(WalkHeight::wallsBlock(Map::width,Map::height,read,x,y,z)==referenceWalls(map,x,y,z),"wall port agrees with real transformed boxes/SAT");
    }
    std::puts("PASS: 4000 wall queries match libGyro box transforms and SAT");
}
// Optional read-only performance check: walk_height <map.bin> <tileset.txt>.
static void benchmark(const char* mapPath,const char* tilePath) {
    std::ifstream file(mapPath,std::ios::binary),types(tilePath);
    check(bool(file) && bool(types),"benchmark files exist");
    file.seekg(100); uint16_t width=0,height=0;
    file.read(reinterpret_cast<char*>(&width),2); file.read(reinterpret_cast<char*>(&height),2);
    std::string name; unsigned amount; types>>name>>amount;
    std::vector<Tile> tileTypes(amount);
    for(auto& tile:tileTypes) {
        unsigned animated,next,stand,transparency; float animTime,damping;
        types>>tile.collider>>animated>>animTime>>damping>>next>>stand>>transparency;
        tile.standOn=stand!=0;
    }
    std::vector<Tile> map(static_cast<size_t>(width)*height*3);
    for(auto& tile:map) {
        uint32_t value=0; file.read(reinterpret_cast<char*>(&value),4);
        check((value&511u)<amount,"valid tileset index");
        const auto& type=tileTypes[value&511u]; tile=WalkHeight::decode(value,type.standOn,type.collider);
    }
    check(bool(file) && bool(types),"benchmark data read completely");
    const auto read=[&](unsigned l,int x,int y){return map[(l*height+y)*width+x];};
    unsigned counts[3]={};
    const auto begin=std::chrono::steady_clock::now();
    for(unsigned l=0;l<3;++l) for(int y=0;y<height;++y) for(int x=0;x<width;++x) {
        const auto s=WalkHeight::surface(read(l,x,y),x,y);
        if(!s) continue;
        for(auto side:{WalkHeight::Left,WalkHeight::Right,WalkHeight::Near,WalkHeight::Far})
            ++counts[static_cast<unsigned>(WalkHeight::edge(width,height,read,*s,side))];
    }
    const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
    std::printf("%s: %.3fs for entire map; no neighbour %u, reachable %u, blocked %u\n",mapPath,seconds,counts[0],counts[1],counts[2]);
}
int main(int argc,char** argv) {
    differentialCheck();

    const float apex=jump(1.0f/60);
    check(std::abs(apex-WalkHeight::JulienJumpApex)<.001f,"reference apex measured with real gyIntegrate");
    check(jump(1.0f/30)<apex && jump(1.0f/120)>apex,"jump is frame-rate dependent");
    std::printf("Julien apex: %.5f units (%.3f tiles) at 60 Hz; landing reach %.5f\n",apex,apex/32,WalkHeight::JulienLandingReach);
    Map map;
    const auto read=[&](unsigned l,int x,int y){return map(l,x,y);};
    const Tile grass={1,0,0,true,0};
    for(int y=0;y<Map::height;++y) for(int x=0;x<Map::width;++x) map.at(0,x,y)=grass;
    // Flat house art on the second layer is still a zero-height floor.
    for(int y=3;y<=5;++y) for(int x=3;x<=5;++x) map.at(1,x,y)={2,0,0,true,0};
    auto flat=WalkHeight::surface(map(1,3,4),3,4);
    check(flat && flat->top==0,"flat house remains height zero");
    check(WalkHeight::edge(Map::width,Map::height,read,*flat,WalkHeight::Left)==WalkHeight::Barrier::Reachable,"grass to flat house has no red edge");
    Map narrow;
    for(int y=0;y<Map::height;++y) for(int x=0;x<Map::width;++x) narrow.at(0,x,y)=grass;
    narrow.at(1,3,4)={2,0,0,true,1};
    const auto narrowRead=[&](unsigned l,int x,int y){return narrow(l,x,y);};
    const auto narrowRoof=WalkHeight::surface(narrow(1,3,4),3,4);
    check(WalkHeight::edge(Map::width,Map::height,narrowRead,*narrowRoof,WalkHeight::Left)==WalkHeight::Barrier::Blocked,"jumping past a solid flat roof is not landing on it");
    // Real collision query: body volume, same elevation, fractional-Z row offset.
    Map walls;
    const auto wallRead=[&](unsigned l,int x,int y){return walls(l,x,y);};
    walls.at(0,5,5)={1,0,1,false,1};
    check(WalkHeight::wallsBlock(Map::width,Map::height,wallRead,160,160,0),"full solid blocks at its elevation without standOn");
    check(!WalkHeight::wallsBlock(Map::width,Map::height,wallRead,160,160,32),"lower elevation does not block body above it");
    walls.at(0,5,5).elevation=1;
    check(WalkHeight::wallsBlock(Map::width,Map::height,wallRead,160,184,20),"body head reaches next elevation slice");
    check(!WalkHeight::wallsBlock(Map::width,Map::height,wallRead,160,192,0),"wall above entire body is ignored");
    walls.at(0,5,5)={1,0,0,false,2}; // east half
    check(!WalkHeight::wallsBlock(Map::width,Map::height,wallRead,150,160,0),"empty half of partial collider remains open");
    check(WalkHeight::wallsBlock(Map::width,Map::height,wallRead,170,160,0),"east half blocks");
    walls.at(0,5,5).orientation=1; // horizontal flip
    check(WalkHeight::wallsBlock(Map::width,Map::height,wallRead,150,160,0),"flipped east half becomes west");
    walls.at(1,5,5)={1,0,0,false,2};
    check(WalkHeight::combinedCollider(Map::width,Map::height,wallRead,5,5,0)==elysian::Tile::COLLIDER_FULL,"opposite halves combine across layers");
    walls.at(1,5,5).elevation=1;
    check(WalkHeight::combinedCollider(Map::width,Map::height,wallRead,5,5,0)==elysian::Tile::COLLIDER_HALF_W,"only matching elevations combine");
    walls.at(0,5,5)={1,0,1,false,16};
    walls.at(1,5,5)={};
    check(!WalkHeight::wallsBlock(Map::width,Map::height,wallRead,160,160,0),"runtime fine pass ignores unsupported triangle colliders");
    // House with a three-high FRONT wall and a roof deep enough for a stable
    // landing. The front is world row 7; the roof occupies world rows 5..7.
    Map house;
    const auto houseRead=[&](unsigned l,int x,int y){return house(l,x,y);};
    for(int y=0;y<Map::height;++y) for(int x=0;x<Map::width;++x) house.at(0,x,y)=grass;
    for(int y=2;y<=4;++y) for(int x=7;x<=10;++x) house.at(2,x,y)={4,3,0,true,0};
    for(unsigned h=0;h<3;++h) for(int x=7;x<=10;++x) house.at(1,x,7-static_cast<int>(h))={3,h,1,false,1};
    const auto houseRoof=WalkHeight::surface(house(2,8,4),8,4);
    check(WalkHeight::edge(Map::width,Map::height,houseRead,*houseRoof,WalkHeight::Far)==WalkHeight::Barrier::Blocked,"solid FRONT wall 0/1/2 blocks roof at 3");
    for(unsigned h=0;h<3;++h) for(int x=7;x<=10;++x) house.at(1,x,7-static_cast<int>(h)).collider=0;
    check(WalkHeight::edge(Map::width,Map::height,houseRead,*houseRoof,WalkHeight::Far)==WalkHeight::Barrier::Reachable,"same wall made non-solid allows roof at 3");
    // Stacked FRONT wall faces; an actual roof is height 3, not a front's base.
    for(unsigned h=0;h<3;++h) for(int x=7;x<=10;++x) map.at(1,x,7-static_cast<int>(h))={3,h,1,true,1};
    map.at(2,8,4)={4,3,0,true,0};
    for(int h=0;h<3;++h) {
        auto wall=WalkHeight::surface(map(1,7,7-h),7,7-h);
        check(wall && wall->top==(h+1)*32,"FRONT top adds one tile");
        auto firstHalf=WalkHeight::support(Map::width,Map::height,read,7*32,wall->nearY+8,wall->top);
        auto secondHalf=WalkHeight::support(Map::width,Map::height,read,7*32,wall->nearY+20,wall->top);
        check(firstHalf && firstHalf->top==wall->top,"FRONT upper half supports body");
        check(secondHalf && secondHalf->top==0,"FRONT lower half does not support body");
    }
    auto roof=WalkHeight::surface(map(2,8,4),8,4);
    check(roof && roof->top==96,"roof height three");
    // Candidate 96-unit surface can be caught at z=80, but not at z=79.9.
    auto catchRoof=WalkHeight::support(Map::width,Map::height,read,8*32,(4+2)*32+8,80);
    auto missRoof=WalkHeight::support(Map::width,Map::height,read,8*32,(4+2)*32+8,79.9f);
    check(catchRoof && catchRoof->top==96,"inclusive 16-unit landing tolerance");
    check(missRoof && missRoof->top==0,"outside landing tolerance");
    check(!WalkHeight::support(Map::width,Map::height,read,8*32,8*32,96,1),"rising body skips floor search");
    check(!WalkHeight::support(Map::width,Map::height,read,-50,0,0),"out-of-map query rejected");
    // Keep floor footprints adjacent in world space: raised map rows are shifted.
    map.at(2,4,6)={5,4,0,true,0}; // world row 10, height 128
    auto high=WalkHeight::surface(map(2,4,6),4,6);
    check(WalkHeight::edge(Map::width,Map::height,read,*high,WalkHeight::Left)==WalkHeight::Barrier::Blocked,"four-tile rise is red");
    map.at(2,3,10)={5,4,0,true,0}; // adjacent in picture, NOT adjacent in world
    check(WalkHeight::edge(Map::width,Map::height,read,*high,WalkHeight::Left)==WalkHeight::Barrier::Blocked,"painted neighbours do not hide a world-space barrier");
    map.at(2,3,6)={5,4,0,true,0};
    check(WalkHeight::edge(Map::width,Map::height,read,*high,WalkHeight::Left)==WalkHeight::Barrier::Blocked,"isolated equal-height tiles lack support after gravity changes lookup row");
    map.at(2,3,7)={5,4,0,true,0}; map.at(2,4,7)={5,4,0,true,0};
    check(WalkHeight::edge(Map::width,Map::height,read,*high,WalkHeight::Left)==WalkHeight::Barrier::Reachable,"stable same-height floor removes red edge live");
    map.at(2,3,7)={}; map.at(2,4,7)={};
    map.at(2,3,6)={};
    map.at(2,4,6).elevation=3;
    high=WalkHeight::surface(map(2,4,6),4,6);
    check(WalkHeight::edge(Map::width,Map::height,read,*high,WalkHeight::Left)==WalkHeight::Barrier::Blocked,"apex alone does not prove stable landing on isolated roof");
    map.at(2,4,6).standOn=false;
    check(!WalkHeight::surface(map(2,4,6),4,6),"non-standable tile is not tinted");
    const auto slope=WalkHeight::decode(1u|(2u<<12)|(4u<<17),true,0);
    check(slope.top()==64,"runtime treats non-FRONT planes as flat for standing");
    std::puts("PASS: flat house, solid FRONT wall blocks roof 3, non-solid wall permits roof 3, partial/rotated colliders, stable landing and live edits");
    if(argc==3) benchmark(argv[1],argv[2]);
}
