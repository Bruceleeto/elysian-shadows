#include <AssetIO/level/elysian_terrain_geometry.hpp>
#include <libGyro-Kinetic/gyro_kinetic_physics.h>
#include <libGyro-Kinetic/gyro_kinetic_kinetic.h>
#include <libGyro-Kinetic/gyro_kinetic_collision.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace elysian;
using namespace elysian::terrain;
static void check(bool ok,const char* why){if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
static bool near(float a,float b){return std::abs(a-b)<.02f;}
struct Map {
    static constexpr int width=24,height=24;static constexpr unsigned layers=4;
    std::vector<TileEntry> entries=std::vector<TileEntry>(width*height*layers);
    TileEntry& at(unsigned l,int x,int y){return entries[(l*height+y)*width+x];}
    void tile(int x,int worldRow,unsigned elev,GY_RECT_PLANE_TYPE plane=GY_RECT_PLANE_BOTTOM,unsigned l=0) {
        auto& e=at(l,x,worldRow-int(elev));e.setIndex(1);e.setElev(elev);e.setPlane(plane);
    }
    Motion moveBody(Body b,Vec d,Vec v={}) {return move(width,height,layers,[&](unsigned l,int x,int y){return at(l,x,y);},b,d,v,0,true);}
};
static Body body(Vec feet){return {feet+Vec{0,0,10},{6.4f,0,0},{0,10,0},10};}
static void floorChecks(){
    Map map;map.tile(5,8,2);
    auto r=map.moveBody(body({160,256,300}),{0,0,-400},{0,0,-1000});
    check(near(r.center.z-10,64)&&r.grounded&&near(r.velocity.z,0),"fast fall lands on the visible floor without tunnelling");
    r=map.moveBody(body({160,256,0}),{0,0,100},{0,0,300});
    check(near(r.center.z+10,64)&&!r.grounded&&near(r.velocity.z,0),"underside of a floor blocks an upward jump");
    map.at(0,5,6).setSolid(false);
    r=map.moveBody(body({160,256,300}),{0,0,-400});
    check(near(r.center.z-10,-100)&&!r.grounded,"collision off disables both support and blocking");
    map.at(0,5,6).setSolid(true);map.tile(6,8,2);
    r=map.moveBody(body({160,256,64}),{32,0,0},{100,0,0});
    check(near(r.center.x,192)&&near(r.velocity.x,100),"flat neighbouring tiles have no blocking seam");
    r=map.moveBody(body({160,256,63}),{0,0,-1});
    check(near(r.center.z-10,64),"small initial floor penetration is recovered upward");
    r=map.moveBody(body({160,256,64}),{0,0,12},{0,0,300});
    check(near(r.center.z-10,76) && !r.grounded && near(r.velocity.z,300),"jumping away from a touching floor is never blocked");
    // Layer four has the same geometry, and old collider/standOn metadata is irrelevant.
    map.entries.assign(map.entries.size(),{});map.tile(5,8,2,GY_RECT_PLANE_BOTTOM,3);
    r=map.moveBody(body({160,256,150}),{0,0,-200});check(near(r.center.z-10,64),"fourth layer is solid by default");
}
static void wallChecks(){
    for(auto plane:{GY_RECT_PLANE_FRONT,GY_RECT_PLANE_LEFT,GY_RECT_PLANE_RIGHT}){
        Map map;map.tile(5,8,2,plane);
        const bool front=plane==GY_RECT_PLANE_FRONT;
        const float sign=plane==GY_RECT_PLANE_LEFT?-1:1;
        const Vec normal=front?Vec{0,1,0}:Vec{sign,0,0};
        const Vec wall=Vec{160,256,64}+normal*16;
        for(float dz:{0.f,8.f,20.f})for(float direction:{-1.f,1.f}) {
            const auto start=wall+normal*(direction*80)+Vec{0,0,dz};
            const auto r=map.moveBody(body(start),normal*(-direction*160),normal*(-direction*100));
            const float distance=dot(r.center-wall,normal)*direction;
            check(near(distance,front?10:6.4f),"walls stop both approaches at their rendered face at each fractional height");
            check(near(dot(r.velocity,normal),0),"wall removes only inward velocity");
        }
        auto r=map.moveBody(body(wall+normal*80+Vec{0,0,32.1f}),normal*-160);
        check(near(dot(r.center-wall,normal),-80),"jumping above a wall can cross its top");
        // Tile orientation rotates artwork, never the actual wall geometry.
        for(unsigned orientation=0;orientation<8;++orientation){
            map.at(0,5,6).setOrient(static_cast<GY_RECT_TEX_ORIENT>(orientation));
            r=map.moveBody(body(wall+normal*80),normal*-160);
            check(near(dot(r.center-wall,normal),front?10:6.4f),"UV orientation does not rotate wall collision");
        }
    }
    for(auto plane:{GY_RECT_PLANE_LEFT_SLOPE_45,GY_RECT_PLANE_RIGHT_SLOPE_45}) {
        Map map;map.tile(5,8,2,plane);
        const auto q=geometry(map.at(0,5,6),5,6);auto n=cross(q[1]-q[0],q[2]-q[0]);n=n*(1/length(n));
        const auto start=Vec{160,256,64}+n*80;
        const auto r=map.moveBody(body(start),n*-160);
        check(r.contacts && dot(r.center-Vec{160,256,64},n)>0,"diagonal upright faces block rather than becoming invisible floors");
    }
}
static void rampChecks(){
    Map map;
    for(int x=3;x<=8;++x){
        for(int row=5;row<=7;++row)map.tile(x,row,2);
        map.tile(x,8,1,GY_RECT_PLANE_FRONT_SLOPE_45);
        for(int row=9;row<=12;++row)map.tile(x,row,1);
    }
    for(float dt:{1.f/30,1.f/60,1.f/120})for(bool uphill:{true,false}) {
        GYStatic stat{};stat.inverseMass=.002f;stat.damping={.001f,.001f,.5f};stat.angDamping=1;
        GYKinetic kinetic{};GYVector3 pos{160,uphill?320.f:192.f,uphill?32.f:64.f};float angle=0;
        unsigned contacts=0;
        for(int frame=0;frame<int(2/dt);++frame){
            const Vec old{pos.x,pos.y,pos.z};
            const bool arrived=uphill?pos.y<210:pos.y>302;
            kinetic.forceAccum={0,arrived?0.f:(uphill?-1000000.f:1000000.f),-2000/stat.inverseMass};
            gyIntegrate(&kinetic,&stat,dt,&pos,&angle);
            const Vec delta=Vec{pos.x,pos.y,pos.z}-old;
            const auto r=map.moveBody(body(old),delta,{kinetic.vel.x,kinetic.vel.y,kinetic.vel.z});
            pos={r.center.x,r.center.y,r.center.z-10};kinetic.vel={r.velocity.x,r.velocity.y,r.velocity.z};contacts+=r.contacts;
            check(pos.z>=31.99f,"ramp walk never falls below the lower floor");
        }
        if(!near(pos.z,uphill?64:32))std::fprintf(stderr,"ramp dt=%g up=%d y=%g z=%g\n",dt,uphill,pos.y,pos.z);
        check(contacts>0 && near(pos.z,uphill?64:32) && (uphill?pos.y<224:pos.y>288),"real integration walks up/down the rendered ramp at 30/60/120 Hz");
    }
}
static void contactsAndBounds(){
    Map map;
    for(int y=5;y<12;++y)for(int x=3;x<10;++x)map.tile(x,y,2);
    // Two perpendicular walls meeting at (176,272), with continuous floor.
    map.tile(5,8,2,GY_RECT_PLANE_FRONT,1);
    map.tile(5,8,2,GY_RECT_PLANE_RIGHT,2);
    auto r=map.moveBody(body({160,256,64}),{80,80,-1},{500,500,-20});
    check(r.center.x<=169.62f && r.center.y<=262.02f && near(r.center.z-10,64),"diagonal movement cannot escape a closed wall corner");
    r=map.moveBody(body({160,256,64}),{-20,80,-1},{-100,500,-20});
    check(near(r.center.x,140) && r.center.y<=262.02f,"sliding along a wall retains tangential movement");
    // Build the body's real transformed vertices rather than duplicating the
    // runtime's extraction formula in the test.
    GYBoxCollider box{};box.collider.type=GY_COLLIDER_OBB;
    gyMatLoadIdentity();gyMatTranslate(160,256,64);gyMatRotate(.3f,0,0,1);gyMatScale(12.8f,20,20);gyCalcBoxWorldVert(&box);
    auto rotated=bodyFromVertices(box.worldVert);
    check(near(rotated.center.z-rotated.halfZ,64),"runtime vertex conversion retains feet height");
    r=map.moveBody(rotated,{100,0,0},{500,0,0});
    check(near(r.center.x+rotated.radius({1,0,0}),176),"rotated body stops at its real footprint");
    // Large coordinates outside the map must never become overflowing indices.
    for(auto point:{Vec{-1e20f,256,64},Vec{1e20f,256,64},Vec{160,1e20f,64},Vec{160,256,-1e20f}}){
        r=map.moveBody(body(point),{0,0,-100});check(!r.contacts,"out-of-map body safely remains unsupported");
    }
    Map empty;r=empty.moveBody(body({160,256,0}),{0,0,-100},{0,0,-100});
    check(near(r.center.z-10,-100) && !r.grounded,"empty space has no implicit invisible ground plane");
    const auto read=[&](unsigned l,int x,int y){return map.at(l,x,y);};
    r=move(Map::width,Map::height,Map::layers,read,body({128,224,100}),{0,0,-50},{0,0,-300},.5f,false,100);
    check(r.verticalImpact==300 && near(r.velocity.z,150) && !r.grounded,"thrown body bounces and records landing impact");
    r=move(Map::width,Map::height,Map::layers,read,body({128,224,65}),{0,0,-2},{0,0,-30},.5f,false,100);
    check(r.grounded && near(r.velocity.z,0),"small bounces settle instead of vibrating forever");
}
int main(){floorChecks();wallChecks();rampChecks();contactsAndBounds();std::puts("PASS: geometry, fast sweeps, floors/walls/ramps/ceilings, corners, rotated bodies, jumping, bounce, collision off and bounds");}
