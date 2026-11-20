/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef TK_TILE_PLACEMENT_H
#define TK_TILE_PLACEMENT_H
#include <ESToolkit/ui/widget/tk_area_face.h>
#include <array>

struct TilePlacementSlot {
    bool occupied=false;
    int elevation=0,plane=0;
};
enum class TilePlacementAction { Add, Repaint, Erase, Reject };
enum class TilePlacementFailure { None, Full, Missing, Invalid };
struct TilePlacement {
    AreaPreviewFace face;
    TilePlacementAction action=TilePlacementAction::Reject;
    TilePlacementFailure failure=TilePlacementFailure::None;
};
// A layer is storage, not another world coordinate. An occupied slot can only
// be repainted when it already represents the requested surface. New geometry
// must use empty storage; it must never move/delete an unrelated surface.
template<std::size_t N>
inline TilePlacement resolveTilePlacement(AreaPreviewFace face,
    const std::array<TilePlacementSlot,N>& slots,int preferredLayer,bool erase=false,int layerCount=N) {
    TilePlacement result{face};
    if(face.x<0 || face.y<0 || face.elevation<0 || face.elevation>31 || face.plane<0 || face.plane>6 ||
       preferredLayer<0 || preferredLayer>=layerCount || layerCount<1 || layerCount>int(N)) {
        result.failure=TilePlacementFailure::Invalid; return result;
    }
    const auto matches=[&](int layer) {
        return layer>=0 && layer<layerCount && slots[layer].occupied &&
            slots[layer].elevation==face.elevation && slots[layer].plane==face.plane;
    };
    if(erase) {
        if(matches(face.layer)) result.action=TilePlacementAction::Erase;
        else result.failure=TilePlacementFailure::Missing;
        return result;
    }
    // Preserve the clicked face when overlapping artwork uses multiple layers.
    // Otherwise reuse an existing matching surface before allocating storage.
    for(int layer:{face.layer,preferredLayer,3,2,1,0}) if(matches(layer)) {
        result.face.layer=layer; result.action=TilePlacementAction::Repaint; return result;
    }
    for(int offset=0;offset<layerCount;++offset) {
        const int layer=(preferredLayer+offset)%layerCount;
        if(!slots[layer].occupied) {
            result.face.layer=layer; result.action=TilePlacementAction::Add; return result;
        }
    }
    result.failure=TilePlacementFailure::Full;
    return result;
}
#endif
