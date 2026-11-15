/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESGamma/crtp/elysian_render_system.hpp>
#include <ESGamma/crtp/elysian_debug.hpp>
#include <libGyro/gyro_video_api.h>
#include <AssetIO/crtp/elysian_area.hpp>
#include <AssetIO/crtp/elysian_particle_emitter.hpp>
#include <ESGamma/crtp/elysian_camera_system.hpp>
#include <AssetIO/crtp/elysian_level.hpp>
#include <gl/gyro_video_gl.h>
#include <gl/gyro_gl.h>
#include <gl/gyro_shaders_gl.h>
#include <gl/gyro_gl_utils.h>

namespace elysian {

const constexpr RenderableObjectPipelineAttrib::BlendModeFactors RenderableObjectPipelineAttrib::_blendFactors[BLEND_MODE_COUNT];

bool RenderSystemBase::initialize(void) {
    dbgprintf("RenderSystem::initialize()");
    return true;
}

bool RenderSystemBase::uninitialize(void) {
    return true;
}

bool RenderSystemBase::renderParticles(void) {
    if(gyVidShadowPass()) return true;

    for(unsigned i = 0; i < getCurrentArea()->particleEmitters.size(); ++i) {
        getCurrentArea()->particleEmitters[i]->render();
    }
    return true;
}

void RenderSystemBase::beginScene(void) {
#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
    memcpy(_prevSceneStats, _curSceneStats, sizeof(SceneStats));
    memset(_curSceneStats, 0, sizeof(SceneStats));
    _curSceneStats.totalSceneObjs = _sceneObjs.size();
    for(auto& obj : _sceneObjs) {
        obj.setSceneObjTransformDirty(true);
    }
#endif
}

void RenderSystemBase::endScene(void) {
#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
    _curSceneStats.avgBatchSize /= _curSceneStats.batchCount;
#endif
}

void RenderSystemBase::enableTransparentGeometry(bool val) {
    _transparentGeometryEnabled = val;
}

bool RenderSystemBase::isTransparentGeometryEnabled(void) const {
    return _transparentGeometryEnabled;
}

//Draws every tile of the current area in one call, using the static index buffer
//the area builds when it is activated (see AreaBase::populateVbo()).
void RenderSystemBase::renderScene(void) {
    if(gyVidShadowPass()) return;
    if(!_curArea || !_curArea->mapVao || !_curArea->mapIboCount) return;

    gyVidVertexBatchFlush();
    gyVidVertexBatchEnd();

    GYVidPipelineState state = {
        0,
        getCurrentLevel()->getTileSheet(),
        getCurrentLevel()->getNormalSheet(),
        getCurrentLevel()->getEmissiveSheet(),
        GY_ALPHA_NONE,
        GY_BLEND_ONE,
        GY_BLEND_ONE,
        true,
        GY_VID_DEPTH_LEQUAL,
        { 0, 0, 0, 0 }
    };

    gyVidPipelineState(&state);
    gyVidMaterialBind(nullptr);

    gyVidVertexArrayBind(_curArea->mapVao);
    glDrawElements(GL_TRIANGLE_STRIP, static_cast<GLsizei>(_curArea->mapIboCount), GL_UNSIGNED_INT, nullptr);
    GY_GL_CHECK_ERROR();
    gyVidVertexArrayBind(0);

#if 0
#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
    _curSceneStats.totalSceneObjs += _sceneObjs.size();
    ++_curSceneStats.sceneRenderPasses;
#endif

    _opaqueSceneObjs.clear();
    _alphaSceneObjs.clear();

    //Look at every object in the scene (PARALLELIZE ME!!!)
    for(auto& obj : _sceneObjs) {
        //skip transparency if alpha is disabled this pass
        if(obj.isTransparent() && !_transparentGeometryEnabled) {
            continue;
        }

        //update object's internals (if necessary)
        obj.updateBoundingVolume(_curCamera->viewProjMat);

        //world-space frustum culling
        if(cullSceneObj(obj)) {
            obj.setVisible(false);
#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
            ++_curSceneStats.culledSceneObjs;
#endif
            continue;
        } else {
            obj.setVisible(true);

            //update priority values
            obj.updateRenderPriority();

            //write object into respective queue
            if(obj.isTransparent())
                _alphaSceneObjs.push_back(obj);
            else
                _opaqueSceneObjs.push_back(obj);
        }

    }

    //do both in parallel
    quicksort(&_opaqueSceneObjs); //ascending Z depth
    quicksort(&_alphaSceneObjs);  //descending Z depth

#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
    _curSceneStats.opaqueSceneObjs 	+= _opaqueSceneObjs.size();
    _curSceneStats.alphaSceneObjs 	+= _alphaSceneObjs.size();
#endif

    std::vector<SceneObject*>& objLists[2] = {
        _curSceneStats.opaqueSceneObjs,
        _curSceneStats.alphaSceneObjs
    };

    //Pass 0: opaque geometry, Pass 1: transparent geometry (optional)
    for(unsigned p = 0; p < _transparentGeometryEnabled? 2 : 1; ++p) {

        for(size_t i = 0; i < objLists[p].size(); ++i) {
            const SceneObject*const obj = _objLists[p][i];

            //Determine number of objects within current batch
            const uint32_t batchId = (obj->getRenderPriority()&0xffffffff);
            size_t end = i;
            for(size_t e = i+1; e < objLists[i].size(); ++e) {
                if(objLists[e]->getRenderPriority()&0xffffffff == batchId) {
                    end = e;
                } else break;
            }

#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
            ++_curSceneStats.batchCount;
            _curSceneStats.avgBatchSize += e-i + 1;
#endif

            GYVidPipelineState pipeState = {
                obj->getTextureId(),
                obj->getBlendMode(),
                obj->shaderId()
            };

            gyVidPipelineState(&pipeState);
            for(size_t o = i; o <= end; ++o) {
                obj->render();
#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
                ++_curSceneStats.renderCalls;
#endif
            }

            i = end;
        }

    }
#endif
}


#ifdef GY_VIDEO_DEBUG

bool RenderSystemBase::dumpStats(void) const {
    return gyVidDebugStatsDump();
}

#endif

}
