/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#include <libGyro/gyro_video_api.h>
#include <ESGamma/utility/elysian_camera_framing.hpp>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

static void check(bool condition,const char* message) {
    if(!condition) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
static bool near(float a,float b) { return std::abs(a-b)<.00001f; }

static void framingChecks() {
    // Match the actual renderer's translation and Z shear with libGyro matrix
    // operations. Party fitting must measure these visible distances, not raw Y.
    for(int elevation=0;elevation<32;++elevation)for(float relativeHeight:{-32.f,0.f,32.f}) {
        const float height=elevation*32.f;
        const GYVector3 camera{700,1500,1};
        const GYVector3 member{740,1500+height+20,height+relativeHeight};
        GYVector4 world{member.x,member.y,member.z,1},projected{};
        gyMatLoadIdentity();
        gyMatTranslate(-camera.x,-camera.y,0);
        gyMatShearZ(0,-1);
        gyMatMultVec(&world,&projected,1);
        const auto distance=elysian::cameraEntityDistance(member,camera);
        check(near(distance.x,std::abs(projected.x)) && near(distance.y,std::abs(projected.y)),
              "party fitting agrees with the rendered 2D position at every terrain height");
        // A nearby party on any terrace fits inside the base zoom. Returning
        // from debug view must not produce a smaller target just due to height.
        const float halfWidth=640*.8f/2,halfHeight=480*.8f/2;
        const float target=std::min(halfWidth/std::max(halfWidth,distance.x),
                                    halfHeight/std::max(halfHeight,distance.y));
        check(near(target,1),"nearby party does not trigger zoom-out on raised terrain");
    }
    const GYVector3 camera{700,1500,1};
    const auto far=elysian::cameraEntityDistance({700,1500+480+400,480},camera);
    check(near(far.y,400),"genuine visible party separation still expands the frame");
    std::puts("camera framing: rendered party distances and stable zoom across all terrain heights passed");
}

int main() {
    framingChecks();
    // A camera-facing square must have equal pixel scales on both axes. The
    // former extra division by aspect passed square viewports but failed here.
    const std::array<std::array<unsigned,2>,5> sizes{{
        {600,600},{1050,890},{1600,900},{600,900},{640,480}
    }};
    GYMatrix4 square;
    gyVidPerspectiveMat(square,45.0f,1.0f,.5f,900.0f);
    const float verticalScale=1.0f/std::tan(float(M_PI)/8.0f);
    check(near(square[0][0],verticalScale) && near(square[1][1],verticalScale),
          "square viewport retains the original 45-degree field of view");
    for(const auto& size:sizes) {
        const float width=float(size[0]),height=float(size[1]),aspect=width/height;
        GYMatrix4 projection;
        gyVidPerspectiveMat(projection,45.0f,aspect,.5f,900.0f);
        check(near(projection[0][0]*aspect,verticalScale),"horizontal frustum uses aspect exactly once");
        check(near(projection[1][1],verticalScale),"resizing does not change vertical field of view");
        const float xPixels=projection[0][0]*width;
        const float yPixels=projection[1][1]*height;
        check(near(xPixels/yPixels,1.0f),"equal world lengths retain equal pixel lengths in a rectangular viewport");
        for(unsigned col=0;col<4;++col) for(unsigned row=0;row<4;++row)
            if(col!=0 || row!=0) check(near(projection[col][row],square[col][row]),
                "aspect correction changes only horizontal projection, preserving depth and vertical scale");
        GYMatrix4 ortho;
        gyVidOrthoProjMat(ortho,0,width,height,0,-1500,1500);
        check(near(std::abs(ortho[0][0])*width/2,1) && near(std::abs(ortho[1][1])*height/2,1),
            "normal orthographic view remains one pixel per world unit");
    }
    // Changing dock widths at a fixed viewport height must reveal more/less
    // world horizontally, not stretch objects or alter their apparent height.
    GYMatrix4 narrow,wide;
    gyVidPerspectiveMat(narrow,45,.75f,.5f,900);
    gyVidPerspectiveMat(wide,45,2.0f,.5f,900);
    check(near(narrow[0][0]*600,wide[0][0]*1600) && near(narrow[1][1],wide[1][1]),
          "dock-width changes preserve pixel scale at fixed viewport height");
    std::puts("camera_projection: rectangular viewports, resize scales, unchanged FOV/depth and orthographic projection passed");
}
