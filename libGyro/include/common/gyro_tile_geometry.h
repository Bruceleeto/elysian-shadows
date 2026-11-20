/* Shared tile geometry: rendering and terrain collision use these vertices. */
#ifndef GYRO_TILE_GEOMETRY_H
#define GYRO_TILE_GEOMETRY_H
#include <libGyro/gyro_video_api.h>
static const GYVector4 gyTilePlaneVerts[GY_RECT_PLANE_MAX][4] = {
    {{-.5f,-.5f,0,1},{.5f,-.5f,0,1},{-.5f,.5f,0,1},{.5f,.5f,0,1}},
    {{-.5f,.5f,1,1},{.5f,.5f,1,1},{-.5f,.5f,0,1},{.5f,.5f,0,1}},
    {{-.5f,-.5f,1,1},{-.5f,.5f,1,1},{-.5f,-.5f,0,1},{-.5f,.5f,0,1}},
    {{.5f,-.5f,1,1},{.5f,.5f,1,1},{.5f,-.5f,0,1},{.5f,.5f,0,1}},
    {{-.5f,-.5f,1,1},{.5f,-.5f,1,1},{-.5f,.5f,0,1},{.5f,.5f,0,1}},
    {{-.5f,.5f,1,1},{.5f,-.5f,1,1},{-.5f,.5f,0,1},{.5f,-.5f,0,1}},
    {{-.5f,-.5f,1,1},{.5f,.5f,1,1},{-.5f,-.5f,0,1},{.5f,.5f,0,1}}
};
#endif
