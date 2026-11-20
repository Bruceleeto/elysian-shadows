/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#include <AssetIO/level/elysian_terrain_geometry.hpp>
#include <AssetIO/level/elysian_map_format.hpp>
#include <AssetIO/level/elysian_tile_collision.hpp>
#include <ESGamma/utility/elysian_party_trail.hpp>
#include <libGyro-Kinetic/gyro_kinetic_physics.h>
#include <libGyro-Kinetic/gyro_kinetic_kinetic.h>
#include <libGyro-Kinetic/gyro_kinetic_collision.h>
#include <libGyro-Kinetic/gyro_kinetic_contact.h>
#include <gl/gyro_video_gl.h>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>
using elysian::TileEntry;
static void check(bool ok,const char* message) {
    if(!ok) {std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}
}
struct Map {
    uint16_t width=0,height=0;
    unsigned layers=3;
    std::vector<TileEntry> entries;
    std::vector<elysian::Tile> types;
    TileEntry at(unsigned layer,int x,int y) const {
        check(x>=0 && y>=0 && x<width && y<height,"map lookup bounded");
        return entries[(layer*height+y)*width+x];
    }
    auto support(float x,float y,float z) const {
        return elysian::terrain::support(width,height,layers,[&](unsigned l,int col,int row){return at(l,col,row);},x,y,z,1.01f);
    }
};
static unsigned advance(const Map& map,GYKinetic& body,const GYStatic& stat,float dt,GYVector3& position,float offset=5,float depth=20,float height=20) {
    using namespace elysian::terrain;
    const Vec before{position.x,position.y,position.z};float angle=0;
    // GravityForceGenerator skips resting bodies in the actual engine.
    if(body.resting)body.forceAccum.z+=2000/stat.inverseMass;
    gyIntegrate(&body,&stat,dt,&position,&angle);
    const Vec movement=Vec{position.x,position.y,position.z}-before;
    const Body box{before+Vec{0,offset,height*.5f},{6.4f,0,0},{0,depth*.5f,0},height*.5f};
    const auto result=move(map.width,map.height,map.layers,[&](unsigned l,int x,int y){return map.at(l,x,y);},
        box,movement,{body.vel.x,body.vel.y,body.vel.z},0,true);
    position={result.center.x,result.center.y-offset,result.center.z-height*.5f};
    body.vel={result.velocity.x,result.velocity.y,result.velocity.z};body.resting=result.grounded;
    return result.contacts;
}
struct Contact {float depth=0;};
static Contact wallContact(const Map& map,GYVector3 center,GYStatic&,float depth=20,float height=20) {
    using namespace elysian::terrain;
    const Body box{{center.x,center.y,center.z+height*.5f},{6.4f,0,0},{0,depth*.5f,0},height*.5f};
    Contact result;
    visit(map.width,map.height,map.layers,box,{},[&](unsigned l,int x,int y){return map.at(l,x,y);},[&](const Quad& q){
        const auto hit=sweep(box,{},q);
        if(hit.contact)result.depth=std::max(result.depth,hit.depth);
    });
    return result;
}
static void approach(const Map& map,int bankRow,int low,int high,float dt,float dx=0) {
    GYStatic stat{};stat.inverseMass=.002f;stat.damping={.001f,.001f,.5f};stat.angDamping=1;
    GYKinetic body{};GYVector3 position{24*32.f,(bankRow+3)*32.f-5,low*32.f};
    unsigned contacts=0;
    for(int frame=0;frame<240;++frame) {
        body.forceAccum={dx*1000000,-1000000,-2000/stat.inverseMass};
        contacts+=advance(map,body,stat,dt,position);
        if(position.z<low*32.f-.01f) {
            std::fprintf(stderr,"bank %d: frame %d dt %.5f centre=(%.3f,%.3f) feet=%.3f expected=%d contacts=%u\n",
                         bankRow,frame,dt,position.x,position.y+5,position.z,low*32,contacts);
            check(false,"walking into a raised bank must not lose the floor beneath the body");
        }
        check(position.z<high*32.f,"walking cannot climb a full-tile rise");
    }
    check(contacts>0,"approach reaches the bank and exercises real wall contact");
    const float edge=(bankRow+.5f)*32;
    check(position.y+5-10>=edge-.02f,"bank stops the body at the rendered wall face");
}
static void renderedFloorChecks(const Map& map) {
    unsigned probes=0;
    for(unsigned layer=0;layer<map.layers;++layer)for(int y=0;y<map.height;++y)for(int x=0;x<map.width;++x) {
        const auto tile=map.at(layer,x,y);
        if(!tile.getSolid() || tile.getPlane()!=GY_RECT_PLANE_BOTTOM)continue;
        // The original terrain stairs retain lower ground beneath the bottom
        // ramp. Its visible surface is checked by renderedStairChecks instead.
        bool coveredByRamp=false;
        for(unsigned l=0;l<map.layers;++l) {
            const auto above=map.at(l,x,y);
            if(above.getIndex() && above.getElev()==tile.getElev() &&
               above.getPlane()==GY_RECT_PLANE_FRONT_SLOPE_45)coveredByRamp=true;
        }
        if(coveredByRamp)continue;
        // Positions come from the renderer's vertex table, not from the floor
        // helper's inverse formula. Sample both edges and the centre of each quad.
        const auto& vertices=gyTilePlaneVerts[GY_RECT_PLANE_BOTTOM];
        for(float u:{.01f,.5f,.99f})for(float v:{.01f,.5f,.99f})for(float dz:{0.f}) {
            const float wx=(x+vertices[0].x+(vertices[1].x-vertices[0].x)*u)*32;
            const float wy=(y+tile.getElev()+vertices[0].y+(vertices[2].y-vertices[0].y)*v)*32;
            const auto floor=map.support(wx,wy,tile.getElev()*32+dz);
            check(floor && floor->top==tile.getElev()*32,"support covers the actual rendered flat quad, including both edges");
            ++probes;
        }
    }
    std::printf("%u rendered floor probes passed\n",probes);
}
static void renderedStairChecks(const Map& map) {
    unsigned stairs=0,probes=0;
    const auto& verts=gyTilePlaneVerts[GY_RECT_PLANE_FRONT_SLOPE_45];
    for(unsigned layer=0;layer<map.layers;++layer)for(int y=0;y<map.height;++y)for(int x=0;x<map.width;++x) {
        const auto tile=map.at(layer,x,y);
        if(!tile.getIndex() || tile.getPlane()!=GY_RECT_PLANE_FRONT_SLOPE_45)continue;
        ++stairs;
        for(float u:{.01f,.5f,.99f})for(float v:{.01f,.25f,.5f,.75f,.99f})for(float dz:{0.f}) {
            const float wx=(x+verts[0].x+(verts[1].x-verts[0].x)*u)*32;
            const float wy=(y+tile.getElev()+verts[0].y+(verts[2].y-verts[0].y)*v)*32;
            const float top=(tile.getElev()+verts[0].z+(verts[2].z-verts[0].z)*v)*32;
            const auto floor=map.support(wx,wy,top+dz);
            check(floor && std::abs(floor->top-top)<.001f,"stair support follows the renderer's slope vertices");
            ++probes;
        }
    }
    check(stairs==14,"all four two-wide terrain stairways are present");
    std::printf("%u rendered stair probes passed\n",probes);
}
static void walkStairs(const Map& map,int bankRow,int low,int high,int column,float dt,bool uphill) {
    GYStatic stat{};stat.inverseMass=.002f;stat.damping={.001f,.001f,.5f};stat.angDamping=1;
    const float upperEdge=(bankRow+.5f)*32,lowerEdge=upperEdge+(high-low)*32;
    const float destination=uphill?upperEdge-40:lowerEdge+40;
    GYKinetic body{};GYVector3 position{column*32.f,(uphill?lowerEdge+40:upperEdge-40)-5,(uphill?low:high)*32.f};
    bool arrived=false;
    unsigned settling=0;
    for(int frame=0;frame<int(4/dt);++frame) {
        body.forceAccum={0,arrived?0.f:(uphill?-1000000.f:1000000.f),-2000/stat.inverseMass};
        advance(map,body,stat,dt,position);
        const float renderedTop=high*32-std::clamp(position.y+5-upperEdge,0.f,(high-low)*32.f);
        check(position.z>=renderedTop-.001f,"walking never falls beneath the rendered stairs");
        if(uphill?position.y+5<=destination:position.y+5>=destination)arrived=true;
        if(arrived && ++settling>unsigned(1/dt))break;
    }
    check(arrived,"walking without jumping reaches the other end of the stairway");
    check(std::abs(position.z-(uphill?high:low)*32)<.02f,"stairs finish on the correct terrace");
}
static void traverse(const Map& map,int bankRow,int low,int high,float dt,bool jump) {
    GYStatic stat{};stat.inverseMass=.002f;stat.damping={.001f,.001f,.5f};stat.angDamping=1;
    const float edge=(bankRow+.5f)*32;
    GYKinetic body{};GYVector3 position{24*32.f,edge+(jump?25.f:-40.f)-5,(jump?low:high)*32.f};
    const int frames=int(std::ceil(1.2f/dt));
    for(int frame=0;frame<frames;++frame) {
        body.forceAccum={0,jump?-1000000.f:1000000.f,-2000/stat.inverseMass};
        if(jump && frame==0)body.forceAccum.z+=330000/dt;
        advance(map,body,stat,dt,position);
        if(position.z<low*32.f-.01f)std::fprintf(stderr,"traverse bank=%d low=%d high=%d dt=%.5f jump=%d frame=%d centreY=%.3f z=%.3f velZ=%.3f\n",bankRow,low,high,dt,jump,frame,position.y+5,position.z,body.vel.z);
        check(position.z>=low*32.f-.01f,"jump/drop never falls below the lower terrace");
    }
    if(!jump)check(std::abs(position.z-low*32.f)<.02f,"intentional drop lands on the lower terrace");
    else if(high-low<=2)check(std::abs(position.z-high*32.f)<.02f,"jump lands on a reachable raised terrace");
    else check(std::abs(position.z-low*32.f)<.02f,"too-high bank blocks the jump and body lands safely below");
}
static bool partyRoute(const Map& map,float dt,bool stairs) {
    GYStatic stat{};stat.inverseMass=.002f;stat.damping={.001f,.001f,.5f};stat.angDamping=1;
    const int row=stairs?31:39,low=stairs?12:10,high=stairs?15:12;
    const float edge=(row+.5f)*32;
    const GYVector3 spawn{(stairs?10:24)*32.f,edge+(stairs?(high-low)*32+40.f:25.f)-5,low*32.f};
    GYVector3 positions[4]={spawn,spawn,spawn,spawn};
    GYKinetic bodies[4]{};
    std::vector<elysian::PartyTrailPoint> trail(121,{spawn,0});
    bool arrived[4]{};
    for(int frame=0;frame<int(5/dt);++frame) {
        const bool advanced=elysian::recordPartyTrail(trail,{positions[0],bodies[0].vel.z});
        if(advanced)for(unsigned i=1;i<4;++i) {
            const auto& point=trail[1+i*29];
            positions[i]=point.position;bodies[i].vel.z=point.verticalVelocity;
        }
        for(unsigned i=0;i<4;++i) {
            const float yOffset=i?0.f:5.f; // Actual party prefab collider offsets.
            const bool turn=positions[0].y+5<edge-64;
            bodies[i].forceAccum={i?0.f:(turn?1000000.f:0.f),i?0.f:(turn?0.f:-1000000.f),-2000/stat.inverseMass};
            if(!i && !stairs && frame==0)bodies[i].forceAccum.z+=330000/dt;
            advance(map,bodies[i],stat,dt,positions[i],yOffset,i?25.6f:20.f,i?25.6f:20.f);
            // A body on the upper terrace must never be underneath its floor.
            if((positions[i].y+yOffset<edge-1 && positions[i].z<high*32-.01f) || positions[i].z<low*32-.01f) {
                std::fprintf(stderr,"party route stairs=%d dt=%.5f frame=%d member=%u pos=(%.3f,%.3f,%.3f)\n",stairs,dt,frame,i,positions[i].x,positions[i].y,positions[i].z);
                return false;
            }
            if(positions[i].y+yOffset<edge-32 && std::abs(positions[i].z-high*32)<.01f)arrived[i]=true;
        }
    }
    for(unsigned i=0;i<4;++i)if(!arrived[i]) {
        std::fprintf(stderr,"party arrival stairs=%d dt=%.5f member=%u pos=(%.3f,%.3f,%.3f)\n",stairs,dt,i,positions[i].x,positions[i].y,positions[i].z);
        return false;
    }
    return true;
}
static void partyChecks(const Map& map) {
    // All four spawn on the same raised terrace. Check their distinct collider
    // offsets over repeated real gravity updates before anyone starts walking.
    for(float offset:{0.f,5.f}) {
        GYStatic stat{};stat.inverseMass=.002f;stat.damping={.001f,.001f,.5f};stat.angDamping=1;
        GYKinetic body{};GYVector3 position{768,2016,256};
        for(int i=0;i<240;++i) {
            body.forceAccum.z=-2000/stat.inverseMass;
            advance(map,body,stat,1.f/60,position,offset);
            check(std::abs(position.z-256)<.02f,"leader and followers retain their spawn floor before movement");
        }
    }
    for(float dt:{1.f/30,1.f/60,1.f/120})for(bool stairs:{false,true})
        check(partyRoute(map,dt,stairs),"all three followers reach the upper terrace via the recorded jump/stairs");
    std::vector<elysian::PartyTrailPoint> trail(121,{{768,2016,256},0});
    check(elysian::recordPartyTrail(trail,{{768,2016,260},200}),"jumping in place records vertical motion");
    check(trail.front().position.z==260 && trail.front().verticalVelocity==200,"trail retains height and vertical velocity");
    std::puts("PASS: raised party spawn, full-height trail following, and three followers on jumps/stairs at 30/60/120 Hz");
}
static int prototypeChecks() {
    const std::string root="assets/level/rewrite/terrain_test";
    auto* file=std::fopen((root+"/area/terrain/map.bin").c_str(),"rb");
    if(!file){std::puts("SKIP: editable prototype assets unavailable");return 77;}
    const auto read=[&](void* data,std::size_t size){return std::fread(data,1,size,file)==size;};
    elysian::mapformat::Header header;
    check(elysian::mapformat::readHeader(read,header),"prototype map header");
    Map map; map.width=header.width;map.height=header.height;map.layers=header.layers;
    map.entries.resize(map.width*map.height*map.layers);
    check(elysian::mapformat::readEntries(read,map.entries.size(),[&](std::size_t n,uint32_t v){map.entries[n]=TileEntry(v);}),"prototype entries");
    std::fclose(file);
    std::ifstream tiles(root+"/tile/tileset.txt");std::string name;unsigned count;
    tiles>>name>>count;check(bool(tiles) && count<=512,"prototype tileset header");map.types.resize(count);
    for(auto& t:map.types){unsigned collider,animated,next,stand,transparent;float anim,damping;
        tiles>>collider>>animated>>anim>>damping>>next>>stand>>transparent;
        t.colliderType=elysian::Tile::COLLIDER_TYPE(collider);t.standOn=stand;
    }
    check(bool(tiles),"prototype settings read");
    unsigned floors=0,walls=0,ramps=0,disabled=0;
    for(const auto& e:map.entries) {
        if(!e.getIndex())continue;
        if(!e.getSolid()){++disabled;continue;}
        if(e.getPlane()==GY_RECT_PLANE_BOTTOM)++floors;
        else if(e.getPlane()==GY_RECT_PLANE_FRONT_SLOPE_45)++ramps;
        else ++walls;
    }
    renderedFloorChecks(map);
    std::printf("PASS: saved prototype geometric probes: %u floors, %u walls, %u ramps, %u collision-off placements\n",floors,walls,ramps,disabled);
    return 0;
}
int main(int argc,char** argv) {
    if(argc>1 && std::string(argv[1])=="--prototype")return prototypeChecks();
    const std::string root="tests/fixtures/stepped_terrain";
    std::ifstream file(root+"/area/terrain/map.bin",std::ios::binary);
    if(!file){std::puts("SKIP: prototype terrain assets unavailable");return 77;}
    Map map;file.seekg(100);file.read(reinterpret_cast<char*>(&map.width),2);file.read(reinterpret_cast<char*>(&map.height),2);
    check(map.width==48 && map.height==72,"expected prototype terrain dimensions");
    map.entries.resize(map.width*map.height*3);file.read(reinterpret_cast<char*>(map.entries.data()),map.entries.size()*sizeof(TileEntry));
    check(bool(file),"complete prototype map read");
    std::ifstream tiles(root+"/tile/tileset.txt");std::string name;std::getline(tiles,name);unsigned count;tiles>>count;
    check(bool(tiles) && count<=512,"valid tileset count");map.types.resize(count);
    for(auto& t:map.types){unsigned collider,animated,next,stand,transparent;float anim,damping;
        tiles>>collider>>animated>>anim>>damping>>next>>stand>>transparent;
        t.colliderType=elysian::Tile::COLLIDER_TYPE(collider);t.standOn=stand;
    }
    check(bool(tiles),"prototype tileset parsed");
    renderedFloorChecks(map);
    renderedStairChecks(map);
    partyChecks(map);
    // A bank face has a fixed world Y at every height through a jump. Probe
    // against its rendered FRONT edge, independently of the collision transform.
    GYStatic probeStat{};probeStat.inverseMass=.002f;
    const float bankFace=(55+gyTilePlaneVerts[GY_RECT_PLANE_FRONT][0].y)*32;
    for(float dz:{0.f,.1f,8.f,16.f,31.9f}) {
        check(wallContact(map,{768,bankFace+9.75f,256+dz},probeStat).depth>0,
              "bank collision stays aligned to its rendered face at fractional jump heights");
        check(wallContact(map,{768,bankFace+10.25f,256+dz},probeStat).depth==0,
              "bank collision does not extend beyond its rendered face");
    }
    for(float dt:{1.f/30,1.f/60,1.f/120})for(float diagonal:{0.f,-.2f,.2f}) {
        approach(map,55,8,9,dt,diagonal);
        approach(map,47,9,10,dt,diagonal);
        approach(map,39,10,12,dt,diagonal);
        approach(map,31,12,15,dt,diagonal);
    }
    for(float dt:{1.f/30,1.f/60,1.f/120})for(bool jump:{false,true}) {
        traverse(map,55,8,9,dt,jump);traverse(map,47,9,10,dt,jump);
        traverse(map,39,10,12,dt,jump);traverse(map,31,12,15,dt,jump);
    }
    for(float dt:{1.f/30,1.f/60,1.f/120})for(bool uphill:{false,true})for(int column=10;column<=11;++column) {
        walkStairs(map,55,8,9,column,dt,uphill);walkStairs(map,47,9,10,column,dt,uphill);
        walkStairs(map,39,10,12,column,dt,uphill);walkStairs(map,31,12,15,column,dt,uphill);
    }
    std::puts("PASS: 48 stair walks up/down every lane at 30/60/120 Hz, without jumping");
    std::puts("PASS: saved prototype map, 36 bank approaches and 24 jumps/drops with real gravity, SAT and contact correction");
}
