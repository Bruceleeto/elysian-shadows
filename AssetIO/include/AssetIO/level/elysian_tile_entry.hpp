/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ELYSIAN_TILE_ENTRY_HPP
#define ELYSIAN_TILE_ENTRY_HPP
#include <libGyro/gyro_video_api.h>
#include <stdint.h>

//TileEntry properties
#define ELYSIAN_TILE_ENTRY_ORIENT_BIT_POS	9
#define ELYSIAN_TILE_ENTRY_ORIENT_BIT_MASK	0x0007
#define ELYSIAN_TILE_ENTRY_ELEV_BIT_POS		12
#define ELYSIAN_TILE_ENTRY_ELEV_BIT_MASK	0x001f
#define ELYSIAN_TILE_ENTRY_INDEX_BIT_POS	0
#define ELYSIAN_TILE_ENTRY_INDEX_BIT_MASK	0x01ff
#define ELYSIAN_TILE_ENTRY_PLANE_BIT_POS	17
#define ELYSIAN_TILE_ENTRY_PLANE_BIT_MASK	0x000f

#define ELYSIAN_TILE_ENTRY_GET_ATTRIB(t ,a)		((value >> ELYSIAN_##t##_ENTRY_##a##_BIT_POS) & ELYSIAN_##t##_ENTRY_##a##_BIT_MASK)
#define ELYSIAN_TILE_ENTRY_SET_ATTRIB(t, a, v)	do { \
                                                    value &= ~(ELYSIAN_##t##_ENTRY_##a##_BIT_MASK << ELYSIAN_##t##_ENTRY_##a##_BIT_POS); \
                                                    (value) |= ((unsigned)(v) << ELYSIAN_##t##_ENTRY_##a##_BIT_POS); \
                                                } while(0)

// Per-square character barrier, stored on layer zero only.
#define ELYSIAN_TILE_ENTRY_BLOCKED_BIT_POS 21
#define ELYSIAN_TILE_ENTRY_BLOCKED_BIT_MASK 1u

namespace elysian {
    struct TileEntry {
        TileEntry(void);
        TileEntry(const uint32_t val);

        bool                    getBlocked(void) const;
        void                    setBlocked(bool blocked);

        uint32_t                value = 0;

        uint32_t                getValue(void) const;
        void                    setValue(const uint32_t val);
        GY_RECT_TEX_ORIENT      getOrient(void) const;
        unsigned char           getElev(void) const;
        void                    setOrient(const GY_RECT_TEX_ORIENT o);
        void                    setElev(const unsigned char e);
        void					setIndex(const uint16_t i);
        uint16_t				getIndex(void) const;
        void					setPlane(const GY_RECT_PLANE_TYPE n);
        GY_RECT_PLANE_TYPE		getPlane(void) const;
        bool					getNormal(void) const;
        void					setNormal(const bool val);
    };

    inline bool operator== (const TileEntry& t1, const TileEntry& t2) {
        return (t1.value == t2.value);
    }
    inline TileEntry::TileEntry(void): value(0) {}
    inline TileEntry::TileEntry(const uint32_t val): value(val) {}
    inline uint32_t TileEntry::getValue(void) const { return value; }
    inline void TileEntry::setValue(const uint32_t val) { value = val; }
    inline GY_RECT_TEX_ORIENT TileEntry::getOrient(void) const { return (GY_RECT_TEX_ORIENT)ELYSIAN_TILE_ENTRY_GET_ATTRIB(TILE, ORIENT); }
    inline void TileEntry::setOrient(const GY_RECT_TEX_ORIENT o) { ELYSIAN_TILE_ENTRY_SET_ATTRIB(TILE, ORIENT, o); }
    inline unsigned char TileEntry::getElev(void) const { return ELYSIAN_TILE_ENTRY_GET_ATTRIB(TILE, ELEV); }
    inline void TileEntry::setElev(const unsigned char e) { ELYSIAN_TILE_ENTRY_SET_ATTRIB(TILE, ELEV, e); }
    inline uint16_t TileEntry::getIndex(void) const { return ELYSIAN_TILE_ENTRY_GET_ATTRIB(TILE, INDEX); }
    inline void TileEntry::setIndex(const uint16_t i) { ELYSIAN_TILE_ENTRY_SET_ATTRIB(TILE, INDEX, i); }
    inline GY_RECT_PLANE_TYPE TileEntry::getPlane(void) const { return (GY_RECT_PLANE_TYPE)ELYSIAN_TILE_ENTRY_GET_ATTRIB(TILE, PLANE); }
    inline void TileEntry::setPlane(const GY_RECT_PLANE_TYPE n) { ELYSIAN_TILE_ENTRY_SET_ATTRIB(TILE, PLANE, n); }
    inline bool TileEntry::getNormal(void) const {
        GY_RECT_PLANE_TYPE plane = (GY_RECT_PLANE_TYPE)ELYSIAN_TILE_ENTRY_GET_ATTRIB(TILE, PLANE);

        switch(plane) {
            case GY_RECT_PLANE_FRONT: return true;
            default: return false;
        }

    }
    inline void TileEntry::setNormal(const bool v) { ELYSIAN_TILE_ENTRY_SET_ATTRIB(TILE, PLANE, v); }

    inline bool TileEntry::getBlocked(void) const { return ELYSIAN_TILE_ENTRY_GET_ATTRIB(TILE, BLOCKED) != 0; }
    inline void TileEntry::setBlocked(bool blocked) { ELYSIAN_TILE_ENTRY_SET_ATTRIB(TILE, BLOCKED, blocked); }

    // Artwork edits keep the destination square's mark; copies cannot carry it.
    inline TileEntry paintTileEntry(TileEntry incoming, TileEntry destination, unsigned layer) {
        incoming.setBlocked(layer == 0 && destination.getBlocked());
        return incoming;
    }
}
#endif
