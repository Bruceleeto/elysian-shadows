/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_LIGHT_SOURCE_BASE_HPP
#define ELYSIAN_LIGHT_SOURCE_BASE_HPP

#include <libGyro/gyro_shader_api.h>
#include <libGyro/gyro_video_api.h>
#include <AssetIO/crtp/elysian_component.hpp>
#include <ESGamma/crtp/elysian_camera_system.hpp>
#include <AssetIO/utility/elysian_lua_object.hpp>

#define ELYSIAN_LIGHT_SOURCE_ATTENUATION_EPSILON	0.1f

namespace elysian {

    class LightAttrib:
            private GYLightProperties,
            public LuaObject<LightAttrib>
    {
    public: using LuaObjectType = LuaObject<LightAttrib>;

		friend class LightSystem;
        friend class LightSourceBase;
        friend LightSource;

    public:

        public:

        LightAttrib(void):
            GYLightProperties(defaultProperties) {}

        static GYLightProperties defaultProperties;
        static LightAttrib       defaultAttrib;

        void            setAmbientColor(const GYVector4& color);
        void            setDiffuseColor(const GYVector4& color);
        void            setSpecularColor(const GYVector4& color);
        void            setAttenuationFactors(const GYVector3& vec);
        void            setSpotExponent(const float exp);
        void            setComputeDistanceAttenuation(const bool val);

        void            getAmbientColor(GYVector4& color) const;
        void            getDiffuseColor(GYVector4& color) const;
        void            getSpecularColor(GYVector4& color) const;
        void            getAttenuationFactors(GYVector3& vec) const;
        float           getSpotExponent(void) const;

        Vector4         getAmbient(void);
        Vector4         getDiffuse(void);
        Vector4         getSpecular(void);
        Vector3         getAttenuation(void);

        void            setAmbient(const Vector4& color);
        void            setDiffuse(const Vector4& color);
        void            setSpecular(const Vector4& color);
        void            setAttenuation(const Vector3& vec);

        float           getRadius(void);

        void            linearInterpolate(const LightAttrib& startKeyFrame, const LightAttrib& endKeyFrame,
                               const float interval, const float elapsed);

    private:
        static float    _calculateRadius(const GYVector3& attenuation);

        GYVector3       _prevAttenuation = { 0.0f, 0.0f, 0.0f };
    };


    class LightSourceBase:
            public Component,
            private GYLightSource,
            public LuaObject<LightSourceBase, LightSource>
    {
    public: using LuaObjectType = LuaObject<LightSourceBase, LightSource>;

        friend class    LightSystem;
        friend class    Engine;
        friend          LightSource;

        Camera                          _camState;
        std::shared_ptr<LightAttrib>    _attrib;
        GYVector3                       _relPos         = { 0.0f, 0.0f, 0.0f };
        bool                            _shadowCaster   = false;
    public:
        public:

                            LightSourceBase(void) = default;
                            LightSourceBase(Entity& entity, LightSourceBase*const prefab=nullptr);
                            ~LightSourceBase(void);

         LightSourceBase&   operator=(LightSourceBase& rhs);

		//reimplemented from component
        static constexpr const Component::TYPE componentType = Component::LIGHT_SOURCE;

        void                setLightType(const GY_LIGHT_TYPE _type);
        void                setDirection(const GYVector3& dir);
        void                setSpotCutoffAngle(const float angle);
        void                setAttrib(std::shared_ptr<LightAttrib> attrib);
        void                setShadowCaster(const bool val);
        void                setRelativePos(const float x, const float y, const float z);


        GY_LIGHT_TYPE       getLightType(void) const;
        bool                isShadowCaster(void) const;
        const GYVector4&    getPosition(void) const;
        auto                getAttrib(void) const -> std::shared_ptr<LightAttrib>;
        void                getDirection(GYVector3& dir) const;
        float               getSpotCutoffAngle(void) const;
        void                getRelativePos(GYVector3& pos) const;
        Vector3             getRelPos(void);
        void                setRelPos(const Vector3& pos);
        Vector3             getDir(void);

        void                updateBoundingVolume(const Camera*const cam, const GY_CUBE_MAP_FACE face);
        Camera*             getCameraState(void);
        void                update(void);

    };








    // ========== INLINEZ ===============

    inline void LightAttrib::setAmbientColor(const GYVector4& color) {
        ambientColor = color;
    }
    inline void LightAttrib::setDiffuseColor(const GYVector4& color) {
        diffuseColor = color;
    }
    inline void LightAttrib::setSpecularColor(const GYVector4& color) {
        specularColor = color;
    }

    inline void LightAttrib::setSpotExponent(const float exp) {
        spotExponent = exp;
    }

    inline void LightAttrib::getAmbientColor(GYVector4& color) const { color = ambientColor; }
    inline void LightAttrib::getDiffuseColor(GYVector4& color) const { color = diffuseColor; }
    inline void LightAttrib::getSpecularColor(GYVector4& color) const { color = specularColor; }
    inline void LightAttrib::getAttenuationFactors(GYVector3& vec) const { vec = attenuationFactors; }
    inline float LightAttrib::getSpotExponent(void) const { return spotExponent; }
    inline void LightAttrib::setAmbient(const Vector4& color) { ambientColor = color; }
    inline void LightAttrib::setDiffuse(const Vector4& color) { diffuseColor = color; }
    inline void LightAttrib::setSpecular(const Vector4& color) { specularColor = color; }
    inline void LightAttrib::setAttenuation(const Vector3& vec) { attenuationFactors = vec; }


    inline Vector4 LightAttrib::getAmbient(void) { return Vector4(&ambientColor); }
    inline Vector4 LightAttrib::getDiffuse(void) { return Vector4(&diffuseColor); }
    inline Vector4 LightAttrib::getSpecular(void) { return Vector4(&specularColor); }
    inline Vector3 LightAttrib::getAttenuation(void) { return Vector3(&attenuationFactors); }

    inline LightSourceBase::LightSourceBase(Entity& entity, LightSourceBase*const prefab):
		Component(entity),
		GYLightSource({
			{ 0.0f, 0.0f, 0.0f, 0.0f }, //pos
            { 0.0f, 0.0f, -1.0f },		//dir
			360.0f,						//cutoff
			GY_LIGHT_POINT,				//type
            nullptr,
			&_camState,
            nullptr
        })
    {
        if(!prefab) {
            gyVec3Normalize(&direction);
            setLightType(GY_LIGHT_POINT);
        } else {
            *this = *prefab;              //absorb the other fucker's config and attrib
        }
    }
    inline LightSourceBase& LightSourceBase::operator=(LightSourceBase& rhs) {
        _camState       = rhs._camState;
        _relPos         = rhs._relPos;
        _shadowCaster   = rhs._shadowCaster;
        position        = rhs.position;
        direction       = rhs.direction;
        spotCutoffAngle = rhs.spotCutoffAngle;
        properties      = rhs.properties;
        _attrib         = rhs._attrib;
        setLightType(rhs.type);
        return *this;
    }
    inline void LightSourceBase::setAttrib(std::shared_ptr<LightAttrib> attrib) {
        _attrib = attrib;
        properties = (GYLightProperties*)(attrib.get());
    }
    inline void LightSourceBase::setDirection(const GYVector3& dir) { direction = dir; gyVec3Normalize(&direction); }
    inline void LightSourceBase::setSpotCutoffAngle(const float angle) { spotCutoffAngle = (type == GY_LIGHT_SPOT)? angle : 360.0f; }
    inline auto LightSourceBase::getLightType(void) const -> GY_LIGHT_TYPE { return type; }
    inline const GYVector4& LightSourceBase::getPosition(void) const { return position; }
    inline std::shared_ptr<LightAttrib> LightSourceBase::getAttrib(void) const { return _attrib; }
    inline void LightSourceBase::setShadowCaster(const bool val) { _shadowCaster = val; }
    inline bool LightSourceBase::isShadowCaster(void) const { return _shadowCaster; }
    inline void LightSourceBase::setRelativePos(const float x, const float y, const float z) { _relPos.x = x; _relPos.y = y; _relPos.z = z; }
    inline void LightSourceBase::setRelPos(const Vector3 &pos) { _relPos = pos; }
    inline void LightSourceBase::getDirection(GYVector3& dir) const { dir = direction; }
    inline float LightSourceBase::getSpotCutoffAngle(void) const { return spotCutoffAngle; }
    inline void LightSourceBase::getRelativePos(GYVector3 &pos) const { pos = _relPos; }
    inline Vector3 LightSourceBase::getRelPos(void) { return Vector3(&_relPos); }
    inline Vector3 LightSourceBase::getDir(void) { return Vector3(&direction); }

    inline Camera* LightSourceBase::getCameraState(void) {
		return &_camState;
	}

    inline void LightAttrib::linearInterpolate(const LightAttrib& startKeyFrame, const LightAttrib& endKeyFrame,
                           const float interval, const float elapsed)
    {
        //So fucking type unsafe, but so fucking awesome...
        gyLinearInterp((float*)&ambientColor.x, 16,
                       (float*)&startKeyFrame.ambientColor.x, (float*)&endKeyFrame.ambientColor.x,
                       interval, elapsed);
    }

}


#endif // ELYSIAN_LIGHT_SOURCE_HPP
