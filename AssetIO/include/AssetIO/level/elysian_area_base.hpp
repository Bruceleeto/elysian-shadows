/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_AREA_BASE_HPP
#define ELYSIAN_AREA_BASE_HPP

#include <libGyro/gyro_video_api.h>
#include <vector>
#include <stdint.h>
#include <AssetIO/utility/elysian_lua_object.hpp>
#include <AssetIO/primitive/elysian_instance_transform.hpp>
#include <AssetIO/crtp/elysian_component.hpp>

#define ELYSIAN_MAP_BOUNDARY_CHECKS                 false //PLEASE, GOD, TAKE ME OUT BEFORE RELEASE!!!!!!
#define ELYSIAN_AREA_USE_INIT_LUA                   true
#define ELYSIAN_AREA_GLAL_STRIP_INDEX_ARRAY_SIZE	(200*200*3*8)
#define ELYSIAN_AREA_ARRAY_VBO_TYPE                 GL_STATIC_DRAW
#define ELYSIAN_AREA_ELEMENT_ARRAY_VBO_TYPE         GL_STATIC_DRAW

#define ELYSIAN_AREA_BGM_TRACKS     3
#define ELYSIAN_AREA_AUX_TRACKS     2
#define ELYSIAN_AREA_MAP_LAYERS		3
#define ELYSIAN_AREA_MAP_WIDTH		200
#define ELYSIAN_AREA_MAP_HEIGHT		200
#define ELYSIAN_AREA_NAME_SIZE		50
#define ELYSIAN_AREA_STRING_SIZE	50
#define ELYSIAN_AREA_PATH_SIZE		200
#define ELYSIAN_AREA_MAP_PATH		"/map.bin"
#define ELYSIAN_AREA_ENTITY_PATH	"/entity.txt"
#define ELYSIAN_AREA_TABLE_PATH     "/area_table.lua"
#define ELYSIAN_AREA_INIT_LUA_PATH	"/area_init.lua"
#define ELYSIAN_TILE_ENTRY_ELEV_MAX	31
#define ELYSIAN_AREA_LAYER_DEPTH_EPSILON    0.001f

#include <AssetIO/level/elysian_tile_entry.hpp>

//#define ELYSIAN_TILE_ENTRY_TO_VBO_INDEX(l, x, y) ((l*height*width + y*E + x)*4)

#ifdef ELYSIAN_MAP_BOUNDARY_CHECKS
#	define ELYSIAN_AREA_PROTECTED_MAP_ACCESS(t) ((t < tileEntryCount)? tiles[t] : tiles[0])
#	define ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(l, t) ((t < layerEntryCount)? l[t] : l[0])
#else
#	define ELYSIAN_AREA_PROTECTED_MAP_ACCESS(t) tiles[t]
#	define ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(l, t) l[t]
#endif

ELYSIAN_ASSET_TYPEDEF(Area)

using namespace std;

namespace elysian {

    class AreaBase:
        public LuaObject<AreaBase, Area> {
    public: using LuaObjectType = LuaObject<AreaBase, Area>;

    public:

        public:

        //Entity data
        vector<Entity*>             entities;
        vector<Entity*>             inactiveEntities;

        //Component data
        vector<Renderable*>         renderables;
        vector<Collidable*>         collidables;
        vector<RigidBody*>			rigidBodies;
        vector<Character*>			characters;
        vector<Warp*>				warps;
        vector<Examinable*>         examinables;
        vector<AudioSource*>		audioSources;
        vector<LightSource*>		lightSources;
        vector<ParticleEmitter*>	particleEmitters;
        vector<Combat*>             combats;
        vector<Skeleton*>           skeletons;
        vector<Behavior*>           behaviors;

        char                        dir[ELYSIAN_AREA_NAME_SIZE];		//name of area directory
        char                        path[ELYSIAN_AREA_PATH_SIZE];
        char                        string[ELYSIAN_AREA_STRING_SIZE];	//in-game area name

        //map - tile layers
        //pointer to entire map (contiguous block of memory)
        TileEntry*                  tiles			= nullptr;
        //pointer to individual tile layers (faster indexing)
        TileEntry*                  tileLayer2		= nullptr;
        TileEntry*                  tileLayer3		= nullptr;

        Level*                      level			= nullptr;

        unsigned short int          width			= 0;
        unsigned short int          height			= 0;
        unsigned                    tileEntryCount	= 0;
        unsigned                    layerEntryCount	= 0;

        uint32_t                    vertexVbo       = 0;
        uint32_t                    elementVbo[2]   = {0} ;
        uint32_t                    vao[2]          = {0};
        uint32_t                    mapVao          = 0;    //every map tile as one static triangle strip
        uint32_t                    mapIbo          = 0;
        uint32_t                    mapIboCount     = 0;
        bool                        vboPopulated    = false;
        bool                        activated       = false;

        uint32_t                    elementsArray[2][ELYSIAN_AREA_GLAL_STRIP_INDEX_ARRAY_SIZE] __attribute__((aligned(32)));
        int                         elementsArraySize[2] = {0};
        int                         lastElemArraySize[2] = {0};
        int                         lastOffset[2] = {0};
        int                         minIndex[2] = {INT_MAX};
        int                         maxIndex[2] = {0};
        GYVector4                   _ambientColor   = { 0.5f, 0.5f, 0.5f, 1.0f };
        bool                        _useSun         = true;

        char                        _bgmOggTracks[3][ELYSIAN_AREA_PATH_SIZE] = {{ 0 }};
        char                        _auxOggTracks[2][ELYSIAN_AREA_PATH_SIZE] = {{ 0 }};
        float                       _oggGain = 1.0f;

        //HDR Parameters
        unsigned char               luminanceKey    = 53;
        unsigned char               luminanceMax    = 100;
        unsigned char               luminanceMin    = 10;

                                    AreaBase(void);
                                    ~AreaBase(void);

        unsigned short              getMapWidth(void) const;
        unsigned short              getMapHeight(void) const;
        void                        setMapWidth(const unsigned short w);
        void                        setMapHeight(const unsigned short h);

        const char*                 getDirName(void) const;
        const char*                 getPath(void) const;
        const char*                 getNameString(void) const;
        void                        setDirName(const char* name);
        void                        setNameString(const char *str);

        bool                        getUseSun(void) const;
        void                        setUseSun(const bool val);
        Vector4                     getAmbientColor(void);
        void                        setAmbientColor(const Vector4& color);

        const char*                 getBgmOggByTrack(const unsigned index) const;
        void                        setBgmOggByTrack(const unsigned index, const char* path);
        unsigned                    getBgmOggTrackCount(void) const;
        const char*                 getAuxOggByTrack(const unsigned index) const;
        void                        setAuxOggByTrack(const unsigned index, const char* path);
        unsigned                    getAuxOggTrackCount(void) const;
        float                       getOggGain(void) const;
        void                        setOggGain(float gain);

        //Slower than layer-specific accessors, but loop-friendly
        TileEntry                   getTileEntry(const unsigned l, const unsigned x, const unsigned y) const;
        void                        setTileEntry(const unsigned l, const unsigned x, const unsigned y, const TileEntry e);
        bool                        isValidTileEntry(const unsigned x, const unsigned y) const;
        //Faster lookups
#if 0
        TileEntry                   getL1TileEntry(const unsigned x, const unsigned y) const;
        void                        setL1TileEntry(const unsigned x, const unsigned y, const TileEntry e);
        TileEntry                   getL2TileEntry(const unsigned x, const unsigned y) const;
        void                        setL2TileEntry(const unsigned x, const unsigned y, const TileEntry e);
        TileEntry                   getL3TileEntry(const unsigned x, const unsigned y) const;
        void                        setL3TileEntry(const unsigned x, const unsigned y, const TileEntry e);
#endif
        //generic entity management
        void                        addEntity(Entity *entity);
        void                        addInactiveEntity(Entity* entity);
        void                        removeEntity(Entity *entity);
        void                        validateEntityName(Entity* entity);
        Entity*                     findEntity(const char* name);

        //active entity management
        Entity*                     getActiveEntityByIndex(const unsigned index) const;
        void                        setActiveEntityByIndex(const unsigned index, Entity*const entity);
        unsigned                    getActiveEntityCount(void) const;

        //inactive entity management
        Entity*                     getInactiveEntityByIndex(const unsigned index) const;
        void                        setInactiveEntityByIndex(const unsigned index, Entity*const entity);
        unsigned                    getInactiveEntityCount(void) const;

        template<typename T>
        std::vector<T*>*            getContainer(void);

        void                        allocMap(void); //width + height better be set first!
        void                        freeMap(void);
        void                        resizeMap(const unsigned w, const unsigned h);

        void                        setLevel(Level*const lev);
        Level*                      getLevel(void) const;
        bool                        load(const char *const dir);
        bool                        save(const char *const dir);

        bool                        loadMap(const char *const filename);
        bool                        saveMap(const char *const filename);
        void                        behaviorsOnLoad(void);
        void                        behaviorsOnUnload(void);
        void                        activate(void);
        void                        deactivate(void);

        int                         loadOggTracks(void);
        int                         unloadOggTracks(void);

        void                        populateVbo(void);
        void                        freeVbo(void);
        unsigned                    mapLocationToVBOIndex(unsigned x, unsigned y, unsigned l);
        void                        vboEntryFromTileEntry(GYVertex*const vboEntry, const TileEntry *const tileEntry, const unsigned x, const unsigned y, const unsigned layer) const;
        void                        animateVboTileEntry(const unsigned l, const unsigned x, const unsigned y, const uint16_t index);


    };










    // ==== INLINEZ ====
    inline TileEntry AreaBase::getTileEntry(const unsigned l, const unsigned x, const unsigned y) const {
        return ELYSIAN_AREA_PROTECTED_MAP_ACCESS(l*height*width + y*width + x);
    }


    inline void AreaBase::animateVboTileEntry(const unsigned l, const unsigned x, const unsigned y, const uint16_t index) {
        TileEntry e = getTileEntry(l, x, y);
        const uint16_t oldIndex = e.getIndex();
        e.setIndex(index);
        setTileEntry(l, x, y, e);
        ELYSIAN_AREA_PROTECTED_MAP_ACCESS(l*height*width + y*width + x).setIndex(oldIndex);
    }



#if 0
    inline TileEntry AreaBase::getL1TileEntry(const unsigned x, const unsigned y) const {
        return ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(tiles, y*width + x);
    }
    inline void AreaBase::setL1TileEntry(const unsigned x, const unsigned y, const TileEntry e) {
        ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(tiles, y*width + x) = e;
    }
    inline TileEntry AreaBase::getL2TileEntry(const unsigned x, const unsigned y) const {
        return ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(tileLayer2, y*width + x);
    }
    inline void AreaBase::setL2TileEntry(const unsigned x, const unsigned y, const TileEntry e) {
        ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(tileLayer2, y*width + x) = e;
    }
    inline TileEntry AreaBase::getL3TileEntry(const unsigned x, const unsigned y) const {
        return ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(tileLayer3, y*width + x);
    }
    inline void AreaBase::setL3TileEntry(const unsigned x, const unsigned y, const TileEntry e) {
        ELYSIAN_AREA_PROTECTED_MAP_LAYER_ACCESS(tileLayer3, y*width + x) = e;
    }
#endif

    inline unsigned short  AreaBase::getMapWidth(void) const { return width; }
    inline unsigned short  AreaBase::getMapHeight(void) const { return height; }

    inline const char*     AreaBase::getDirName(void) const { return dir; }
    inline const char*     AreaBase::getPath(void) const { return path; }
    inline const char*     AreaBase::getNameString(void) const { return string; }
    inline void            AreaBase::setDirName(const char* name) { strcpy(dir, name); }
    inline void            AreaBase::setNameString(const char* name) { strcpy(string, name); }

    inline void AreaBase::setLevel(Level * const lev) { level = lev; }

    inline Level* AreaBase::getLevel(void) const { return level; }

     inline bool AreaBase::isValidTileEntry(const unsigned x, const unsigned y) const { return (x < width && y < height); }

     inline unsigned AreaBase::getActiveEntityCount(void) const { return entities.size(); }

     inline void AreaBase::deactivate(void) {
        activated = false;
     }

     inline unsigned AreaBase::getInactiveEntityCount(void) const {
         return inactiveEntities.size();
     }

     template<typename T>
     inline std::vector<T*>* AreaBase::getContainer(void) {
        switch(T::containerType) {
        case Component::RENDERABLE: return &renderables;
        case Component::COLLIDABLE: return &collidables;
        case Component::RIGID_BODY: return &rigidBodies;
        case Component::CHARACTER: return &characters;
        case Component::WARP:       return &warps;
        case Component::EXAMINABLE: return &examinables;
        case Component::AUDIO_SOURCE: return &audioSources;
        case Component::LIGHT_SOURCE: return &lightSources;
        case Component::PARTICLE_EMITTER: return &particleEmitters;
        case Component::COMBAT: return &combats;
        case Component::SKELETON: return &skeletons;
        case Component::BEHAVIOR: return &behaviors;
        default: return nullptr;

        }
     }


     inline bool AreaBase::getUseSun(void) const { return _useSun; }
     inline void AreaBase::setUseSun(const bool val) { _useSun = val; }
     inline Vector4 AreaBase::getAmbientColor(void) { return Vector4(&_ambientColor); }
     inline void AreaBase::setAmbientColor(const Vector4& color) { _ambientColor = color; }
     inline unsigned AreaBase::mapLocationToVBOIndex(unsigned x, unsigned y, unsigned l) {
        return (y*width*ELYSIAN_AREA_MAP_LAYERS + x*ELYSIAN_AREA_MAP_LAYERS + l)*4;
     }
     inline const char* AreaBase::getBgmOggByTrack(const unsigned index) const { return _bgmOggTracks[index]; }
     inline void AreaBase::setBgmOggByTrack(const unsigned index, const char* path) { strcpy(_bgmOggTracks[index], path); }
     inline const char* AreaBase::getAuxOggByTrack(const unsigned index) const { return _auxOggTracks[index]; }
     inline void AreaBase::setAuxOggByTrack(const unsigned index, const char* path) { strcpy(_auxOggTracks[index], path); }
     inline float AreaBase::getOggGain(void) const { return _oggGain; }
     inline void AreaBase::setOggGain(float gain) { _oggGain = gain; }

}

#endif
