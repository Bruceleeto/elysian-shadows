/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#include <AssetIO/level/elysian_tile_collision.hpp>
#include <ESToolkit/core/tk_blocked_paint_stroke.h>
#include <ESToolkit/core/tk_3d_paint_stroke.h>
#include <ESToolkit/core/tk_tile_placement.h>
#include <ESToolkit/core/tk_3d_tile_brush.h>
#include <gl/gyro_video_gl.h>
#include <map>
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
// Ordinary tile colliders retain their existing elevation/orientation rules.
template<std::size_t N>
inline Tile::COLLIDER_TYPE legacyTileCollider(const TileEntry (&entries)[N], const Tile* types, unsigned elevation, unsigned count=N) {
    auto result = Tile::COLLIDER_NONE;
    for(unsigned layer=0;layer<count;++layer) {
        const auto entry=entries[layer];
        if(entry.getElev() != elevation) continue;
        const auto shape = Tile::reorientCollider(types[entry.getIndex()].colliderType, entry.getOrient());
        result = Tile::combineColliders(result, shape);
    }
    return result;
}

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
                    auto shape=tileMask ? legacyTileCollider(entries,types,elev) : Tile::COLLIDER_NONE;
                    if(legacyBarriers && character && entries[0].getBlocked()) shape=Tile::COLLIDER_FULL;
                    if(shape==Tile::COLLIDER_NONE) continue;
                    check(shape==Tile::COLLIDER_FULL,"fixture uses full-square colliders");
                    box(col*32.0f,row*32.0f-(legacyBarriers ? z-first*32 : 0.0f));
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

// The stroke owns normal per-layer commands. This in-memory adapter exercises
// its grouping/live writes without constructing an editor engine or loading assets.
class MemoryTileCommand {
public:
    MemoryTileCommand(Map& map,int layer):_map(map),_layer(layer) {}
    void setCombine(bool) {}
    bool addTile(QPoint point,TileEntry after) {
        if(point.x()<0 || point.y()<0 || point.x()>=Map::width || point.y()>=Map::height) return false;
        const auto key=std::make_pair(point.x(),point.y());
        auto [it,inserted]=_changes.emplace(key,std::make_pair(_map.getTileEntry(_layer,point.x(),point.y()),after));
        if(!inserted) it->second.second=after;
        return true;
    }
    void undo() { for(const auto& [p,data]:_changes) _map.paint(_layer,p.first,p.second,data.first); }
    void redo() { for(const auto& [p,data]:_changes) _map.paint(_layer,p.first,p.second,data.second); }
private:
    Map& _map;
    int _layer;
    std::map<std::pair<int,int>,std::pair<TileEntry,TileEntry>> _changes;
};
static void paint3DChecks() {
    Map map; QUndoStack history;
    TileEntry floor; floor.setIndex(1); floor.setElev(10); floor.setBlocked(true);
    TileEntry roof; roof.setIndex(2); roof.setElev(14);
    map.at(0,2,3)=floor; map.at(1,2,3)=roof;
    const auto baseline=map.tiles;
    using Stroke=TilePaint3DStroke<MemoryTileCommand>;
    const auto stroke=[&](bool erase) {
        return std::make_unique<Stroke>(erase,
            [&](int layer){return std::make_unique<MemoryTileCommand>(map,layer);},
            [&](int layer,QPoint p){return map.getTileEntry(layer,p.x(),p.y());},
            [&](int layer,QPoint p,TileEntry value){map.paint(layer,p.x(),p.y(),value);});
    };
    const auto resolve=[&](AreaPreviewFace face,bool erase=false) {
        std::array<TilePlacementSlot,3> slots;
        for(unsigned l=0;l<3;++l) { const auto entry=map.getTileEntry(l,face.x,face.y); slots[l]={entry.getIndex()!=0,int(entry.getElev()),int(entry.getPlane())}; }
        return resolveTilePlacement(face,slots,1,erase);
    };
    TileEntry brush; brush.setIndex(9); brush.setElev(19); brush.setPlane(GY_RECT_PLANE_FRONT); brush.setOrient(GY_RECT_TEX_ORIENT_FLIP_HORIZ);
    auto destination=resolve({2,3,0,10,0});
    check(destination.action==TilePlacementAction::Repaint && destination.face.layer==0,"water repaint ignores the unrelated roof in the preferred layer");
    auto paint=stroke(false); check(paint->paint(destination.face,brush),"repaint existing surface");
    check(!paint->paint(destination.face,brush),"same drag does not repaint the slot repeatedly");
    history.push(paint.release());
    check(map.at(0,2,3).getElev()==10 && map.at(0,2,3).getPlane()==0 && map.at(0,2,3).getBlocked() && map.at(1,2,3)==roof,
          "water artwork changes but its geometry, blocked mark and roof remain intact");
    const auto afterWater=map.tiles;
    destination=resolve({2,3,0,10,1});
    check(destination.action==TilePlacementAction::Add && destination.face.layer==2,"new wall is assigned empty storage after choosing its direction");
    auto wall=stroke(false); check(wall->paint(destination.face,brush),"write allocated wall"); history.push(wall.release());
    check(map.at(0,2,3)==afterWater[(3*Map::width)+2] && map.at(1,2,3)==roof && map.at(2,2,3).getPlane()==1,
          "wall addition preserves water and roof byte for byte");
    const auto afterWall=map.tiles;
    history.undo(); check(map.tiles==afterWater,"undo wall restores complete map");
    history.redo(); check(map.tiles==afterWall,"redo wall restores exact allocation");
    destination=resolve({2,3,0,10,2});
    check(destination.action==TilePlacementAction::Reject && destination.failure==TilePlacementFailure::Full,"fourth distinct face cannot overwrite occupied storage");
    auto unsafe=stroke(false);
    check(!unsafe->paint({2,3,1,10,0},brush) && unsafe->empty() && map.tiles==afterWall,
          "write boundary independently rejects moving the roof into the water even if the resolver is bypassed");
    auto unsafeErase=stroke(true);
    check(!unsafeErase->paint({2,3,1,10,0},brush) && unsafeErase->empty() && map.tiles==afterWall,
          "stale erase cannot delete a different surface");
    destination=resolve({2,3,2,10,1},true);
    auto erase=stroke(true); check(erase->paint(destination.face,brush),"erase allocated wall"); history.push(erase.release());
    check(map.tiles==afterWater,"erasing wall leaves water and roof untouched");
    history.undo(); check(map.tiles==afterWall,"undo erase restores all entries");
    history.undo(); history.undo(); check(map.tiles==baseline,"whole edit sequence undoes to the original map");
    history.redo(); history.redo(); check(map.tiles==afterWall,"whole edit sequence redoes exactly");
    // Use the unchanged map.bin layout for a complete round trip after editing.
    FILE* file=std::tmpfile(); check(file!=nullptr,"temporary map round trip");
    const char names[100]={}; const uint16_t size[]={Map::width,Map::height};
    check(std::fwrite(names,sizeof names,1,file)==1 && std::fwrite(size,sizeof size,1,file)==1 &&
          std::fwrite(map.tiles.data(),sizeof(map.tiles),1,file)==1,"save edited map with original format");
    check(std::fseek(file,104,SEEK_SET)==0,"seek saved tiles"); Map loaded;
    check(std::fread(loaded.tiles.data(),sizeof(loaded.tiles),1,file)==1 && loaded.tiles==afterWall,"save/load preserves every surface, plane and mark");
    std::fclose(file);
    std::puts("3D placement: water/roof regression, safe wall allocation, writer guards, complete undo/redo and save/load passed");
}
static void placementPolicyChecks() {
    // Captured from the user's saved Loren map, independent of private assets:
    // (50,80) has two roof entries at 14, and an empty third slot. Filling water
    // at 10 must use that empty slot even when Layer 2 is selected.
    const std::array<TilePlacementSlot,3> roofSlots{{{true,14,0},{true,14,0},{}}};
    const auto water=resolveTilePlacement({50,80,0,10,0},roofSlots,1);
    check(water.action==TilePlacementAction::Add && water.face.layer==2 && water.face.elevation==10,
          "saved water gap allocates empty storage beneath both existing roof entries");
    const std::array<TilePlacementSlot,3> fullSlots{{{true,13,0},{true,13,1},{true,13,1}}};
    check(resolveTilePlacement({66,78,0,10,0},fullSlots,1).failure==TilePlacementFailure::Full,
          "saved full square cannot silently lose roof or wall geometry");
    constexpr std::array<TilePlacementSlot,8> cases{{{false,0,0},{true,0,0},{true,3,0},{true,0,1},{true,3,1},{true,3,2},{true,3,3},{true,3,4}}};
    unsigned count=0;
    for(const auto& a:cases) for(const auto& b:cases) for(const auto& c:cases) {
        const std::array<TilePlacementSlot,3> slots{a,b,c};
        for(int height:{0,3,14}) for(int plane=0;plane<7;++plane) for(int preferred=0;preferred<3;++preferred) for(int source=0;source<3;++source) {
            const AreaPreviewFace face{2,3,source,height,plane};
            const auto placement=resolveTilePlacement(face,slots,preferred);
            bool matching=false,free=false;
            for(const auto& slot:slots) {free|=!slot.occupied;matching|=slot.occupied && slot.elevation==height && slot.plane==plane;}
            if(placement.action==TilePlacementAction::Repaint) {
                const auto& slot=slots[placement.face.layer];
                check(slot.occupied && slot.elevation==height && slot.plane==plane,"repaint preserves geometry in every allocation combination");
            } else if(placement.action==TilePlacementAction::Add) check(!slots[placement.face.layer].occupied && !matching,"new geometry uses only empty storage and never duplicates an existing face");
            else check(!matching && !free && placement.failure==TilePlacementFailure::Full,"rejection occurs only at actual capacity");
            const auto erase=resolveTilePlacement(face,slots,preferred,true);
            const bool exists=slots[source].occupied && slots[source].elevation==height && slots[source].plane==plane;
            check((erase.action==TilePlacementAction::Erase)==exists,"erase targets exactly the requested existing surface");
            ++count;
        }
    }
    std::printf("placement policy: %u allocation/geometry cases passed\n",count);
}

static void brushChecks() {
    // Check stamp adjacency against the renderer's actual vertices, including
    // left/right walls where multiple faces need slots in the same saved square.
    for(int plane=0;plane<7;++plane) {
        const AreaPreviewFace anchor{3,5,0,8,plane};
        const auto world=[](const AreaPreviewFace& f,int corner) {
            const auto v=gyTilePlaneVerts[f.plane][corner];
            return std::array<float,3>{f.x+v.x,f.y+f.elevation+v.y,f.elevation+v.z};
        };
        check(world(tileBrushFace3D(anchor,1,0),0)==world(anchor,1),"brush columns join real rendered face edges on every plane");
        check(world(tileBrushFace3D(anchor,0,1),0)==world(anchor,2),"brush rows join real rendered face edges on every plane");
    }
    Map map; QUndoStack history;
    TileEntry roof; roof.setIndex(99); roof.setElev(14); map.at(1,2,3)=roof;
    map.setBlocked({2,3},true);
    const auto original=map.tiles;
    TileBrush3D brush; brush.width=2; brush.height=2;
    for(int y=0;y<2;++y) for(int x=0;x<2;++x) {
        TileEntry e; e.setIndex(10+y*2+x); e.setOrient(GY_RECT_TEX_ORIENT_FLIP_HORIZ);
        brush.cells.push_back({x,y,e});
    }
    using Stroke=TilePaint3DStroke<MemoryTileCommand>;
    auto stroke=std::make_unique<Stroke>(false,
        [&](int layer){return std::make_unique<MemoryTileCommand>(map,layer);},
        [&](int layer,QPoint p){return map.getTileEntry(layer,p.x(),p.y());},
        [&](int layer,QPoint p,TileEntry entry){map.paint(layer,p.x(),p.y(),entry);});
    const auto plan=[&](const TileBrush3D& b,AreaPreviewFace f) {
        return planTileBrush3D(b,f,Map::width,Map::height,0,[&](int layer,int x,int y){return map.getTileEntry(layer,x,y);});
    };
    const auto apply=[&](const TileBrushPlan3D& p) {
        if(!p.valid()) return;
        for(const auto& tile:p.tiles) stroke->paint(tile.face,tile.entry,true);
    };
    auto p=plan(brush,{2,3,0,3,0}); check(p.valid() && p.tiles.size()==4,"four-tile floor brush fits around existing roof"); apply(p);
    for(int y=0;y<2;++y) for(int x=0;x<2;++x)
        check(map.at(0,2+x,3+y).getIndex()==unsigned(10+2*y+x) && map.at(0,2+x,3+y).getOrient()==GY_RECT_TEX_ORIENT_FLIP_HORIZ,
              "stamp preserves artwork order and texture orientation");
    check(map.at(0,2,3).getBlocked() && map.at(1,2,3)==roof,"stamp preserves blocked marks and other surfaces");
    apply(plan(brush,{3,3,0,3,0}));
    check(map.at(0,3,3).getIndex()==10,"overlapping stamps update visited cells to the latest brush artwork");
    history.push(stroke.release()); const auto painted=map.tiles;
    history.undo(); check(map.tiles==original,"one Undo restores entire overlapping multi-tile stroke");
    history.redo(); check(map.tiles==painted,"Redo restores all stamp tiles and preserves unrelated entries");
    // Last cell blocked by capacity: earlier cells must not be written either.
    map=Map{};
    for(int l=0;l<3;++l) {map.at(l,3,4).setIndex(99); map.at(l,3,4).setElev(10+l);}
    const auto full=map.tiles;
    p=plan(brush,{2,3,0,3,0});
    check(!p.valid() && p.failure==TileBrushPlan3D::Failure::Full && p.failedFace.x==3 && p.failedFace.y==4,
          "whole stamp refuses when its last cell is full");
    check(map.tiles==full,"brush preflight does not write a partial stamp");
    check(!plan(brush,{7,11,0,3,0}).valid(),"whole stamp refuses crossing the map edge");
    check(!plan(brush,{2,3,0,0,1}).valid(),"upright stamp refuses extending below supported height");
    map=Map{};
    p=plan(brush,{2,3,0,3,1}); check(p.valid(),"front wall stamp fits");
    check(p.tiles[2].face.x==2 && p.tiles[2].face.y==4 && p.tiles[2].face.elevation==2,"wall stamp rows descend along the wall");
    p=plan(brush,{2,3,0,3,2}); check(p.valid(),"left wall stamp reserves separate slots for shared map squares");
    check(p.tiles[1].face.y==p.tiles[2].face.y && p.tiles[1].face.layer!=p.tiles[2].face.layer,
          "different wall elevations at the same saved square never overwrite one another");
    TileBrush3D diagonal; diagonal.width=4; diagonal.height=4;
    for(int i=0;i<4;++i) diagonal.cells.push_back({i,3-i,brush.cells[0].entry});
    check(!plan(diagonal,{2,3,0,5,2}).valid(),"four wall faces requiring one saved square are rejected before painting");
    brush.cells[1].entry.setIndex(0);
    p=plan(brush,{2,3,0,3,0}); check(p.valid() && p.tiles.size()==3,"empty brush cells leave existing content alone");
    std::puts("3D brush: renderer adjacency, whole-stamp preflight, overlap, undo/redo and storage reservations passed");
}

int main() {
    brushChecks();
    paint3DChecks();
    placementPolicyChecks();
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
