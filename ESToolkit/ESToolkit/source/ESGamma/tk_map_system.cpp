/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESToolkit/ESGamma/tk_map_system.h>
#include <ESToolkit/core/tk_walk_height.h>
#include <cstdio>
#include <vector>
#include <ESGamma/crtp/elysian_camera_system.hpp>

elysian::Material* TkMapSystem::_bgMat  = nullptr;
elysian::Material* TkMapSystem::_selMat = nullptr;
bool TkMapSystem::_renderMapBackground  = false;

void TkMapSystem::staticInitialize(void) {
    MapSystemBase::staticInitialize();

    _bgMat = new elysian::Material;
    _bgMat->setEmissiveColor(GYVector4{0.0f, 0.0f, 0.0f, 0.0f});

    _selMat = new elysian::Material[DRAW_MAX];
    _selMat[DRAW_SELECTING].setEmissiveColor(GYVector4{0.0f, 0.0f, 0.0f, 0.0f});
    _selMat[DRAW_SELECTION].setEmissiveColor(GYVector4{0.0f, 0.0f, 0.0f, 0.0f});
    _selMat[DRAW_FILLING].setEmissiveColor(GYVector4{0.0f, 0.0f, 0.0f, 0.0f});

}

void TkMapSystem::staticUninitialize(void) {
    MapSystemBase::staticUninitialize();

    delete      _bgMat;
    delete[]    _selMat;
}

void TkMapSystem::_renderMapBorders(void) const {
    if(!_renderMapBackground || getLightSys()->isShadowPass()) return;

    GYColor4 color = { 255, 255, 255, 100 };
    float xPos  =  (_scrStartX + _scrEndX)*16.0f;
    float yPos  =  (_scrStartY + _scrEndY)*16.0f;
    float xSize =  (_scrEndX - _scrStartX)*32.0f + 32.0f;
    float ySize =  (_scrEndY - _scrStartY)*32.0f + 32.0f;

    //if(xSize <= 0.0f || ySize <= 0.0f) return;

    GYVidPipelineState state = {
        0,
        nullptr,
        nullptr,
        nullptr,
        GY_ALPHA_FULL,
        GY_BLEND_SRC_ALPHA,
        GY_BLEND_ONE,
        0,
        GY_VID_DEPTH_ALWAYS,
        { 0, 0, 0, 0 }
    };

    gyVidMaterialBind(_bgMat);
    gyVidPipelineState(&state);

    gyVidRectTexOrient(GY_RECT_TEX_ORIENT_NORMAL);
    gyVidQuadListBegin(&color);

    gyVidRectPlaneType(GY_RECT_PLANE_BOTTOM);
    gyMatPush();
    gyMatLoadIdentity();
    gyMatTranslateScale(xPos, yPos, 1.0f, xSize, ySize, ySize);
    gyVidRect(0);
    //gyVidRect(0);
    gyMatPop();

    gyVidQuadListEnd();
    //gyVidDepthTest(false);
}

void TkMapSystem::_renderWalkHeight(unsigned count, const TkMapDepthBuffEntryImpl array[]) const {
    if(getLightSys()->isShadowPass()) return;
    const auto* area=elysian::System::getCurrentArea();
    const auto* level=elysian::System::getCurrentLevel();
    if(!area || !level) return;
    gyVidVertexBatchFlush(); gyVidVertexBatchEnd();
    const auto* camera=getCameraSys()->getCurrentState();
    const bool labels=32*std::abs(camera->getScale().x*camera->staticScale)>=24;
    const GYColor4 raised={100,175,255,65}, blocked={255,25,35,155};
    const GYVidPipelineState state={0,nullptr,nullptr,nullptr,GY_ALPHA_FULL,
        GY_BLEND_SRC_ALPHA,GY_BLEND_ONE_MINUS_SRC_ALPHA,0,GY_VID_DEPTH_ALWAYS,{0,0,0,0}};
    const auto tint=[&](int x,int y,float z,const GYColor4& color) {
        gyVidMaterialBind(_bgMat);
        gyVidPipelineState(&state);
        gyVidRectPlaneType(GY_RECT_PLANE_BOTTOM);
        gyVidRectTexOrient(GY_RECT_TEX_ORIENT_NORMAL);
        gyVidQuadListBegin(&color);
        gyMatPush(); gyMatLoadIdentity();
        gyMatTranslateScale(x*32.0f,y*32.0f+z,z,32,32,1);
        gyVidRect(-1);
        gyMatPop(); gyVidQuadListEnd();
    };
    auto& font=getGuiSys()->pragmata18;
    font._transform.setSize(12,24,24);
    for(unsigned i=0;i<count;++i) {
        const auto& entry=array[i];
        if(entry.x>=area->width || entry.y>=area->height) continue;
        if(area->getTileEntry(0,entry.x,entry.y).getBlocked()) continue;
        if(!getAssets()->getLayerVisible(static_cast<Assets::LAYER_TYPE>(entry.layer))) continue;
        if(!entry.mapEntry.getIndex() || entry.mapEntry.getIndex()>=level->tileAmount) continue;
        const auto tile=WalkHeight::decode(entry.mapEntry.getValue());
        const auto surface=WalkHeight::surface(tile,entry.x,entry.y);
        if(!surface || surface->top<=0) continue;
        const float z=entry.mapEntry.getElev()*32.0f+entry.layer*ELYSIAN_AREA_LAYER_DEPTH_EPSILON;
        tint(entry.x,entry.y,z,raised);
        if(labels) {
            char text[16];
            if(surface->ramp) std::snprintf(text,sizeof(text),"%u-%u",tile.getElev(),tile.getElev()+1);
            else std::snprintf(text,sizeof(text),"%u",tile.getElev());
            font._transform.setPos(entry.x*32.0f+1,entry.y*32.0f+z+1,z);
            font.setColor(0,0,0,255); font.drawString(elysian::Font::C_ALIGN,text);
            font._transform.setPos(entry.x*32.0f,entry.y*32.0f+z,z);
            font.setColor(255,255,255,255); font.drawString(elysian::Font::C_ALIGN,text);
        }
    }
    // Marks belong to the square, not the artwork: show even empty/hidden layer 0.
    for(unsigned i=0;i<count;++i) {
        const auto& entry=array[i];
        if(entry.layer!=0 || entry.x>=area->width || entry.y>=area->height) continue;
        if(area->getTileEntry(0,entry.x,entry.y).getBlocked()) tint(entry.x,entry.y,0,blocked);
    }
    gyVidVertexBatchFlush(); gyVidVertexBatchEnd();
    gyVidRectPlaneType(GY_RECT_PLANE_BOTTOM);
}
