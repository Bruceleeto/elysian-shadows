#include <ESToolkit/core/tk_walk_height.h>
#include <cstdio>
#include <cstdlib>
static void check(bool ok,const char* why){if(!ok){std::fprintf(stderr,"FAIL: %s\n",why);std::exit(1);}}
int main(){
    elysian::TileEntry tile;tile.setIndex(1);
    for(unsigned elev=0;elev<=31;++elev) {
        tile.setElev(elev);tile.setPlane(GY_RECT_PLANE_BOTTOM);
        auto floor=WalkHeight::surface(tile,5,7);
        check(floor && floor->top==elev*32 && floor->nearY==(7+elev)*32-16,"flat display height and footprint follow geometry");
        for(auto plane:{GY_RECT_PLANE_FRONT,GY_RECT_PLANE_LEFT,GY_RECT_PLANE_RIGHT,GY_RECT_PLANE_LEFT_SLOPE_45,GY_RECT_PLANE_RIGHT_SLOPE_45}) {
            tile.setPlane(plane);check(!WalkHeight::surface(tile,5,7),"upright faces do not invent walkable ledges");
        }
        tile.setPlane(GY_RECT_PLANE_FRONT_SLOPE_45);floor=WalkHeight::surface(tile,5,7);
        check(floor && floor->ramp && floor->top==elev*32+16,"ramps display a height range around the actual midpoint");
        tile.setSolid(false);check(!WalkHeight::surface(tile,5,7),"collision-off tile has no displayed support");tile.setSolid(true);
    }
    tile.setIndex(0);check(!WalkHeight::surface(tile,5,7),"empty tile is never a floor");
    std::puts("PASS: Walk Height uses solid geometry for flat floors, ramps and walls");
}
