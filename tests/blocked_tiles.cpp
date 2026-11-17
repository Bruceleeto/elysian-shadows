/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#include <AssetIO/level/elysian_tile_collision.hpp>
#include <ESToolkit/core/tk_blocked_paint_stroke.h>
#include <libGyro-Kinetic/gyro_kinetic_collision.h>
#include <libGyro-Kinetic/gyro_kinetic_contact.h>
#include <libGyro-Kinetic/gyro_kinetic_kinetic.h>
#include <libGyro-Kinetic/gyro_kinetic_physics.h>
#include <QUndoStack>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <type_traits>

using elysian::TileEntry;
using elysian::Tile;
static void check(bool ok, const char* message) {
    if(!ok) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
struct Map {
    static constexpr int width=8,height=12;
    std::array<TileEntry,width*height*3> tiles{};
    TileEntry& at(unsigned layer,int x,int y) { return tiles[(layer*height+y)*width+x]; }
    TileEntry getTileEntry(unsigned layer,int x,int y) const { return tiles[(layer*height+y)*width+x]; }
    void setBlocked(QPoint p,bool blocked) { at(0,p.x(),p.y()).setBlocked(blocked); }
    void beginMapUpdate() {}
    void endMapUpdate() {}
    void paint(unsigned layer,int x,int y,TileEntry entry) { at(layer,x,y)=elysian::paintTileEntry(entry,at(layer,x,y),layer); }
};
static Tile types[2];
struct Hit { bool blocked=false; float depth=0; GYVector2 correction{}; };
// Feed an independently specified map-space footprint into the runtime query,
// SAT and penetration solver. In particular, never position the test body from
// the collider rectangle we are testing: that hid the fractional-height error.
static Hit projectedContact(const Map& map,float x,float mapY,float z,bool character,
                            bool tileMask=false,bool legacyBarriers=false) {
    const float y=mapY+z;
    GYBoxCollider body{}; body.collider.type=GY_COLLIDER_AABB;
    gyMatLoadIdentity(); gyMatTranslateScale(x,y,z,12.8f,20,20); gyCalcBoxWorldVert(&body);
    body.collider.worldPos={x,y,z};
    GYStatic stat{}; stat.inverseMass=.002f;
    gyColliderCurrent(&body.collider,&stat);
    Hit hit;
    const auto box=[&](float cx,float cy) {
        GYBoxCollider tile{}; tile.collider.type=GY_COLLIDER_AABB;
        gyMatLoadIdentity(); gyMatTranslateScale(cx,cy,5,32,32,32); gyCalcBoxWorldVert(&tile);
        GYContact result{}; int swapped=0;
        if(!gyCollision(&tile.collider,nullptr,&result,&swapped) || result.depth<=hit.depth) return;
        GYVector2 displacement[2]{}, rotation[2]{}; float angle[2]{};
        gyResolvePenetration(&result,displacement,rotation,angle);
        hit={true,result.depth,displacement[0]};
    };
    if(!legacyBarriers) {
        const auto blocked=elysian::blockedTileContact(body.worldVert,Map::width,Map::height,character,
            [&](int col,int row) {
                check(col>=0 && col<Map::width && row>=0 && row<Map::height,"barrier lookup stays in map bounds");
                return map.getTileEntry(0,col,row);
            });
        if(blocked.depth>0) {
            GYContact result{};
            result.normal=blocked.normal; result.depth=blocked.depth;
            result.collider[0]=&body.collider; result.staticAttrib[0]=&stat;
            result.worldPos={x,y};
            const GYVector2 center{x,y};
            gyContactCalculatePenetrationInternals(&result,&center,nullptr);
            GYVector2 displacement[2]{}, rotation[2]{}; float angle[2]{};
            gyResolvePenetration(&result,displacement,rotation,angle);
            hit={true,result.depth,displacement[0]};
        }
    }
    // Ordinary tile pass, and the old painted-barrier placement kept only as a
    // regression reference showing why the former test failed to find the bug.
    const int startX=static_cast<int>(body.worldVert[0].x),endX=static_cast<int>(body.worldVert[1].x);
    const int startY=static_cast<int>(body.worldVert[0].y),endY=static_cast<int>(body.worldVert[3].y);
    if((tileMask || legacyBarriers) && startX>=0 && startY>=0 && endX<=Map::width*32 && endY<=Map::height*32) {
        const int first=std::clamp(static_cast<int>(z/32),0,31);
        const int last=std::clamp(static_cast<int>((z+20)/32),0,31);
        for(int elev=first;elev<=last;++elev) {
            for(int row=startY/32-(elev-first)-1;row<=endY/32-(elev-first)+1;++row) {
                for(int col=startX/32;col<=endX/32+1;++col) {
                    const int mapRow=row-elev;
                    if(col<0 || col>=Map::width || mapRow<0 || mapRow>=Map::height) continue;
                    TileEntry entries[3];
                    for(unsigned l=0;l<3;++l) entries[l]=map.getTileEntry(l,col,mapRow);
                    auto shape=tileMask ? elysian::tileCollider(entries,types,elev) : Tile::COLLIDER_NONE;
                    if(legacyBarriers && character && entries[0].getBlocked()) shape=Tile::COLLIDER_FULL;
                    if(shape==Tile::COLLIDER_NONE) continue;
                    check(shape==Tile::COLLIDER_FULL,"fixture uses full-square colliders");
                    box(col*32.0f,row*32.0f-(z-first*32));
                }
            }
        }
    }
    return hit;
}
static bool contact(const Map& map,int col,int row,float z,bool character,bool tileMask=true) {
    const auto hit=projectedContact(map,col*32.0f+18,row*32.0f,z,character,tileMask);
    if(hit.blocked) check(col*32.0f+18+hit.correction.x-6.4f >= col*32.0f+16-.001f,
        "runtime solver pushes character outside the marked square");
    return hit.blocked;
}
static void projectedBarrierRegression() {
    Map map; map.setBlocked({3,4},true);
    // Bottom edge: feet overlap the painted tile at z=31. The previous elevation
    // slices put its collider elsewhere, despite the same mark working at z=0.
    check(!projectedContact(map,96,140,31,true,false,true).blocked,
          "reproduce original fractional-height gap");
    for(float z : {-10.0f,0.0f,15.5f,31.0f,31.99f,32.0f,63.9f,96.0f,101.5f,1200.0f}) {
        for(const auto offset : {GYVector2{-18,0},GYVector2{18,0},GYVector2{0,-20},GYVector2{0,20}}) {
            const auto hit=projectedContact(map,96+offset.x,128+offset.y,z,true);
            check(hit.blocked,"every edge blocks at integer and fractional heights");
            const float x=offset.x+hit.correction.x,y=offset.y+hit.correction.y;
            check(std::abs(x)>=22.4f-.001f || std::abs(y)>=26.0f-.001f,"resolved footprint stays outside painted square");
            check(!projectedContact(map,96+offset.x,128+offset.y,z,false).blocked,"non-character ignores painted square");
        }
    }
    GYKinetic body{}; GYStatic stat{};
    stat.inverseMass=.002f; stat.damping={.001f,.001f,.5f}; stat.angDamping=1;
    GYVector3 position{96,190,0}; float angle=0,apex=0; unsigned contacts=0;
    constexpr float dt=1.0f/60;
    for(int frame=0;frame<150;++frame) {
        body.forceAccum={0,-1000000,-2000/stat.inverseMass+(frame==0?330000/dt:0)};
        gyIntegrate(&body,&stat,dt,&position,&angle);
        apex=std::max(apex,position.z);
        if(position.z<0) { position.z=0; body.vel.z=0; }
        const auto hit=projectedContact(map,position.x,position.y-position.z,position.z,true);
        if(hit.blocked) { position.x+=hit.correction.x; position.y+=hit.correction.y; ++contacts; }
        check(position.y-position.z>=154-.002f,"walking/jumping cannot cross bottom painted edge");
    }
    check(apex>70 && contacts>20,"regression actually jumps and repeatedly reaches the barrier");
}

// The actual two simultaneous contacts from frame 1071 of the user's recording,
// translated by (-576,-96) to fit this small map. Use the engine's deepest-first
// iteration and update ALL contact depths after each real gyResolvePenetration.
static void recordedSeamRegression() {
    Map map; map.setBlocked({3,4},true); map.setBlocked({4,4},true);
    constexpr float x=692.8182f-576, y=225.1247f+5-96, z=17.0448f;
    GYBoxCollider body{}; body.collider.type=GY_COLLIDER_AABB;
    gyMatLoadIdentity(); gyMatTranslateScale(x,y,z,12.8f,20,20); gyCalcBoxWorldVert(&body);
    body.collider.worldPos={x,y,z};
    GYStatic stat{}; stat.inverseMass=.002f;
    gyColliderCurrent(&body.collider,&stat);
    GYBoxCollider boxes[2]{}; GYContact contacts[2]{};
    for(unsigned i=0;i<2;++i) {
        boxes[i].collider.type=GY_COLLIDER_AABB;
        gyMatLoadIdentity(); gyMatTranslateScale((3+i)*32.0f,128+z,5,32,32,32); gyCalcBoxWorldVert(&boxes[i]);
        int swapped=0;
        check(gyCollision(&boxes[i].collider,nullptr,&contacts[i],&swapped),"recorded body overlaps both marked tiles");
        const GYVector2 center{x,y};
        gyContactCalculatePenetrationInternals(&contacts[i],&center,nullptr);
    }
    check(contacts[0].normal.x>.99f && contacts[1].normal.x<-.99f,"reproduce opposing normals at the shared edge");
    unsigned iterations=0;
    for(;iterations<1000;++iterations) {
        auto& deepest=contacts[contacts[0].depth>contacts[1].depth?0:1];
        if(deepest.depth<=.8f) break;
        GYVector2 delta[2]{}, rotation[2]{}; float angle[2]{};
        gyResolvePenetration(&deepest,delta,rotation,angle);
        for(auto& c:contacts) c.depth-=gyVec2DotProd(&delta[0],&c.normal);
    }
    check(iterations==1000,"old independent boxes exhaust the real solver's iteration limit");
    const auto hit=projectedContact(map,x,y-z,z,true);
    check(hit.blocked && std::abs(hit.correction.x)<.001f && hit.correction.y<0,
          "connected region pushes towards the outside, not into its neighbour");
    check(!projectedContact(map,x+hit.correction.x,y-z+hit.correction.y,z,true).blocked,
          "one solver correction clears both recorded tiles");
    std::puts("recorded seam: old contacts hit 1000 iterations; union contact clears both tiles in one");
}

// Independent SAT check over EVERY marked cell, including cells the query did
// not visit. This catches a correction that simply pushes into a different tile.
static bool overlapsMarked(const Map& map,GYBoxCollider& body,GYStatic& stat) {
    gyColliderCurrent(&body.collider,&stat);
    for(int y=0;y<Map::height;++y) for(int x=0;x<Map::width;++x) {
        if(!map.getTileEntry(0,x,y).getBlocked()) continue;
        GYBoxCollider tile{}; tile.collider.type=GY_COLLIDER_AABB;
        gyMatLoadIdentity(); gyMatTranslateScale(x*32.0f,y*32.0f+body.worldVert[3].z,5,32,32,32);
        gyCalcBoxWorldVert(&tile);
        GYContact contact{}; int swapped=0;
        if(gyCollision(&tile.collider,nullptr,&contact,&swapped) && contact.depth>.002f) return true;
    }
    return false;
}
static void regionShapesRegression() {
    unsigned checked=0;
    for(unsigned shape=0;shape<7;++shape) {
        Map map;
        for(int y=2;y<=7;++y) for(int x=1;x<=6;++x) {
            const bool marked=shape==0 ? y==4 : shape==1 ? x==3 : shape==2 ? (x==3 || y==4) :
                shape==3 ? (x==3 || (y==4 && x>3)) : shape==4 ? (x+y)%2==0 :
                shape==5 ? (x==1 || x==6 || y==2 || y==7) : true;
            if(marked) map.setBlocked({x,y},true);
        }
        for(float z:{0.0f,17.0448f,96.0f}) for(float angle:{0.0f,.3f,.785398f}) {
            for(float y=40;y<=248;y+=13) for(float x=16;x<=216;x+=10) {
                GYBoxCollider body{}; body.collider.type=GY_COLLIDER_OBB;
                gyMatLoadIdentity(); gyMatTranslate(x,y+z,z); gyMatRotateZ(angle); gyMatScale(12.8f,20,20);
                gyCalcBoxWorldVert(&body); body.collider.worldPos={x,y+z,z};
                GYStatic stat{}; stat.inverseMass=.002f;
                const bool before=overlapsMarked(map,body,stat);
                const auto result=elysian::blockedTileContact(body.worldVert,Map::width,Map::height,true,
                    [&](int col,int row){return map.getTileEntry(0,col,row);});
                check(!before || result.depth>0,"SAT overlap always produces a region contact");
                if(result.depth<=0) continue;
                GYContact contact{};
                contact.depth=result.depth; contact.normal=result.normal;
                contact.collider[0]=&body.collider; contact.staticAttrib[0]=&stat;
                contact.worldPos={x,y+z}; const GYVector2 center{x,y+z};
                gyContactCalculatePenetrationInternals(&contact,&center,nullptr);
                GYVector2 delta[2]{},rotation[2]{}; float angles[2]{};
                gyResolvePenetration(&contact,delta,rotation,angles);
                for(auto& v:body.worldVert) {v.x+=delta[0].x; v.y+=delta[0].y;}
                body.collider.worldPos.x+=delta[0].x; body.collider.worldPos.y+=delta[0].y;
                check(!overlapsMarked(map,body,stat),"solver correction clears whole region including corners and rotated footprints");
                ++checked;
            }
        }
    }
    std::printf("blocked regions: %u overlapping positions resolved across rows, columns, corners, holes and rotated bodies\n",checked);
}

int main() {
    recordedSeamRegression();
    regionShapesRegression();
    projectedBarrierRegression();
    static_assert(sizeof(TileEntry)==sizeof(uint32_t));
    static_assert(std::is_trivially_copyable_v<TileEntry>);
    TileEntry legacy(0x1fffff);
    check(!legacy.getBlocked(),"all old attributes leave bit 21 clear");
    legacy.setBlocked(true);
    check(legacy.getValue()==0x3fffff,"mark occupies only bit 21");
    legacy.setBlocked(false);
    check(legacy.getValue()==0x1fffff,"clearing preserves all old attributes");
    Map map;
    map.setBlocked({3,4},true);
    map.setBlocked({0,0},true);
    for(float z:{0.0f,96.0f,101.5f,992.0f}) {
        check(contact(map,3,4,z,true),"blocked square contacts character at any height");
        check(contact(map,3,4,z,true,false),"mark cannot be bypassed by character tile mask");
        check(!contact(map,4,4,z,true),"unblocked neighbour stays open");
        check(!contact(map,3,4,z,false),"projectile/object ignores mark");
    }
    types[1].colliderType=Tile::COLLIDER_FULL;
    map.at(1,4,4).setIndex(1); map.at(1,4,4).setElev(3);
    check(contact(map,4,4,96,false),"ordinary collider still blocks objects at matching elevation");
    check(!contact(map,4,4,0,false),"ordinary collider retains elevation check");
    check(!contact(map,4,4,96,true,false),"normal tile mask remains respected away from marks");
    map.at(1,4,4).setBlocked(true);
    check(!contact(map,4,4,0,true),"upper layers do not store square barriers");

    TileEntry art; art.setIndex(1); art.setElev(3); art.setPlane(GY_RECT_PLANE_FRONT);
    map.paint(0,3,4,art);
    check(map.at(0,3,4).getBlocked() && map.at(0,3,4).getElev()==3,"painting retains destination mark");
    map.paint(0,3,4,TileEntry{});
    check(map.at(0,3,4).getBlocked() && !map.at(0,3,4).getIndex(),"erasing retains destination mark");
    map.paint(0,4,4,map.at(0,3,4));
    check(!map.at(0,4,4).getBlocked(),"pasting cannot copy a mark into its destination");
    map.paint(2,3,4,map.at(0,3,4));
    check(!map.at(2,3,4).getBlocked() && map.at(0,3,4).getBlocked(),"other layers preserve layer zero mark");

    QUndoStack history;
    auto* stroke=new BlockedPaintStroke<Map>(&map,true);
    stroke->paint({1,2}); stroke->paint({6,2});
    history.push(stroke);
    for(int x=1;x<=6;++x) check(map.at(0,x,2).getBlocked(),"fast drag fills intermediate squares");
    history.undo();
    for(int x=1;x<=6;++x) check(!map.at(0,x,2).getBlocked(),"one undo removes whole drag");
    history.redo();
    auto* clear=new BlockedPaintStroke<Map>(&map,false);
    clear->paint({1,2}); clear->paint({6,2}); history.push(clear);
    map.paint(0,2,2,art);
    history.undo();
    check(map.at(0,2,2).getBlocked() && map.at(0,2,2).getIndex()==1,"undo clearing restores only the mark");
    history.redo(); check(!map.at(0,2,2).getBlocked(),"redo clears mark");

    // AreaBase::{saveMap,loadMap} stores two 50-byte strings, two uint16 sizes,
    // then the TileEntry array verbatim. Exercise that unchanged on-disk layout.
    FILE* file=std::tmpfile(); check(file!=nullptr,"temporary map file");
    const char names[100]={}; const uint16_t dimensions[]={Map::width,Map::height};
    check(std::fwrite(names,sizeof names,1,file)==1,"write map strings");
    check(std::fwrite(dimensions,sizeof dimensions,1,file)==1,"write map dimensions");
    check(std::fwrite(map.tiles.data(),sizeof(map.tiles),1,file)==1,"save map entries");
    check(std::ftell(file)==104+static_cast<long>(sizeof(map.tiles)),"unchanged map byte count");
    check(std::fseek(file,104,SEEK_SET)==0,"seek entries");
    Map loaded;
    check(std::fread(loaded.tiles.data(),sizeof(loaded.tiles),1,file)==1,"load map entries");
    std::fclose(file);
    check(loaded.tiles==map.tiles,"all map words survive save/load");
    check(loaded.at(0,3,4).getBlocked() && loaded.at(0,0,0).getBlocked(),"blocked empty squares survive save/load");
    std::puts("blocked_tiles: collision, painting, undo/redo and map format passed");
}
