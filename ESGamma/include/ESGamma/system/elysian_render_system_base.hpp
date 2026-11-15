/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_RENDER_SYSTEM_BASE_HPP
#define ELYSIAN_RENDER_SYSTEM_BASE_HPP

#include <ESGamma/system/elysian_system.hpp>
#include <ESGamma/system/elysian_thread_system.hpp>
#include <ESGamma/system/elysian_event_system.hpp>
#include <libGyro/gyro_matrix_api.h>
#include <libGyro/gyro_video_api.h>


namespace elysian {

    enum BLEND_MODE {
        BLEND_MODE_NONE,
        BLEND_MODE_NORMAL,
        BLEND_MODE_ADDITIVE,
        //BLEND_MODE_MULTIPLY,
        //BLEND_MODE_SCREEN,
        //BLEND_MODE_DARKEN,
        //BLEND_MODE_LIGHTEN,
        //BLEND_MODE_DIFFERENCE,
        //BLEND_MODE_NEGATION,
        //BLEND_MODE_EXCLUSION,
        //BLEND_MODE_DODGE,
        //BLEND_MODE_BURN,
        BLEND_MODE_CUSTOM,
        BLEND_MODE_COUNT
    };

    class RenderableObjectPipelineAttrib {
    private:
        struct BlendModeFactors {
            GY_BLEND_FACTOR src;
            GY_BLEND_FACTOR dst;
        };

        const static constexpr BlendModeFactors _blendFactors[BLEND_MODE_COUNT] = {
            { GY_BLEND_NONE,            GY_BLEND_NONE },                 //BLEND_MODE_NONE
            { GY_BLEND_SRC_ALPHA,       GY_BLEND_ONE_MINUS_SRC_ALPHA},
            { GY_BLEND_SRC_ALPHA,       GY_BLEND_ONE },
            { GY_BLEND_ONE,             GY_BLEND_ONE }                      //BLEND_MODE_CUSTOM
        };

        uint16_t            _blendSrcFactor = GY_BLEND_NONE;
        uint16_t            _blendDstFactor = GY_BLEND_NONE;
        GYTexture*          _texture        = nullptr;
        //Shader*             _shader         = nullptr;


    public:
                            RenderableObjectPipelineAttrib(void);

        bool                hasAlpha(void) const;
        bool                isOpaque(void) const;

        void                setBlendFactors(GY_BLEND_FACTOR src, GY_BLEND_FACTOR dst);
        void                setBlendMode(BLEND_MODE mode);
        BLEND_MODE          getBlendMode(void) const;
        GY_BLEND_FACTOR     getSrcBlendFactor(void) const;
        GY_BLEND_FACTOR     getDstBlendFactor(void) const;

        GYTexture*          getTexture(void) const;
        void                setTexture(GYTexture* tex);


    };


    class Thread;

    class RendererThread: public Thread {

    };

    class RenderSystemBase: public System {
    private:

    #ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
        struct SceneStats {
            size_t totalSceneObjs 		= 0;
            size_t opaqueSceneObjs 		= 0;
            size_t transparentSceneObjs	= 0;
            size_t culledSceneObjs      = 0;
            size_t batchCount           = 0;
            size_t avgBatchSize         = 0;
            size_t renderCalls          = 0;
            size_t sceneRenderPasses	= 0;
        } _prevSceneStats, _curSceneStats;
    #endif

        Thread*                 _rendererThread                 = nullptr;

        bool                    _transparentGeometryEnabled     = true;


        bool                    _depthTest                      = true;
        GY_VID_DEPTH_MODE       _depthMode                      = GY_VID_DEPTH_LESS;
        GYVidPipelineState      _pipelineState;


    public:


                                RenderSystemBase(void);


        bool                    initialize(void);
        bool                    uninitialize(void);
        void                    render(void);
        bool                    renderParticles(void);

        //newfangled

        void                    beginScene(void);
        void                    endScene(void);
        void                    enableTransparentGeometry(bool val=true);
        bool                    isTransparentGeometryEnabled(void) const;
        void                    renderScene(void);

        void                    setPipelineStateFromAttrib(const RenderableObjectPipelineAttrib& attrib);
        auto                    getPipelineState(void) const -> const GYVidPipelineState&;

        void                    setDepthMode(bool enabled=true, GY_VID_DEPTH_MODE mode=GY_VID_DEPTH_LESS);


#ifdef GY_VIDEO_DEBUG
        bool                    dumpStats(void) const;
#endif

        Thread*                 getRendererThread(void) const;

    public:


#ifdef ELYSIAN_RENDER_SYSTEM_DEBUG
        const SceneStats& getSceneStats(void) const;
#endif


    };





    //========== INLINEZ ===========
    inline Thread* RenderSystemBase::getRendererThread(void) const { return _rendererThread; }




    //=== RenderPipelineState =====
    inline RenderableObjectPipelineAttrib::RenderableObjectPipelineAttrib(void):
       _blendSrcFactor(GY_BLEND_NONE),
       _blendDstFactor(GY_BLEND_NONE)
    {}

    inline bool RenderableObjectPipelineAttrib::hasAlpha(void) const {
        return (_blendSrcFactor != GY_BLEND_NONE && _blendDstFactor != GY_BLEND_NONE);
    }
    inline bool RenderableObjectPipelineAttrib::isOpaque(void) const {
        return (_blendSrcFactor == GY_BLEND_NONE || _blendDstFactor == GY_BLEND_NONE);
    }

    inline void RenderableObjectPipelineAttrib::setBlendFactors(GY_BLEND_FACTOR src, GY_BLEND_FACTOR dst) {
        _blendSrcFactor = src;
        _blendDstFactor = dst;
    }

    inline BLEND_MODE RenderableObjectPipelineAttrib::getBlendMode(void) const {
        for(unsigned i = 0; i < BLEND_MODE_COUNT; ++i) {

            if(_blendFactors[i].src == _blendSrcFactor &&
                _blendFactors[i].dst == _blendDstFactor)
            {
                return static_cast<BLEND_MODE>(i);
            }
        }
        return BLEND_MODE_CUSTOM;
    }

    inline void RenderableObjectPipelineAttrib::setBlendMode(BLEND_MODE mode) {
        assert(mode >= 0 && mode < BLEND_MODE_COUNT);
        _blendSrcFactor = _blendFactors[mode].src;
        _blendSrcFactor = _blendFactors[mode].dst;
    }

    inline GY_BLEND_FACTOR RenderableObjectPipelineAttrib::getSrcBlendFactor(void) const {
        return static_cast<GY_BLEND_FACTOR>(_blendSrcFactor);
    }

    inline GY_BLEND_FACTOR RenderableObjectPipelineAttrib::getDstBlendFactor(void) const {
        return static_cast<GY_BLEND_FACTOR>(_blendDstFactor);
    }

    inline GYTexture* RenderableObjectPipelineAttrib::getTexture(void) const {
        return _texture;
    }

    inline void RenderableObjectPipelineAttrib::setTexture(GYTexture* tex) {
        _texture = tex;
    }



    inline void RenderSystemBase::setPipelineStateFromAttrib(const RenderableObjectPipelineAttrib& attrib) {
        if(attrib.hasAlpha()) {
            _pipelineState.alphaLevel       = GY_ALPHA_FULL;
            _pipelineState.srcBlendFactor   = attrib.getSrcBlendFactor();
            _pipelineState.dstBlendFactor   = attrib.getDstBlendFactor();
            _pipelineState.depthTestEnabled = false;
        } else {
            _pipelineState.alphaLevel       = GY_ALPHA_NONE;
            _pipelineState.depthTestEnabled = _depthTest;
            _pipelineState.depthTestMode    = _depthMode;
        }

        _pipelineState.texture = attrib.getTexture();

        gyVidPipelineState(&_pipelineState);

    }

    inline RenderSystemBase::RenderSystemBase(void):
        _pipelineState({
            0,
            nullptr,
            nullptr,
            nullptr,
            GY_ALPHA_NONE,
            GY_BLEND_NONE,
            GY_BLEND_NONE,
            1,
            GY_VID_DEPTH_LEQUAL,
            {0, 0, 0, 0}})
    {}


    inline auto RenderSystemBase::getPipelineState(void) const -> const GYVidPipelineState& {
        return _pipelineState;
    }

    inline void RenderSystemBase::setDepthMode(bool enabled, GY_VID_DEPTH_MODE mode) {
        _depthTest = enabled;
        _depthMode =  mode;
    }

}





#endif // ELYSIAN_RENDER_SYSTEM_BASE_HPP
