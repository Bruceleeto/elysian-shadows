/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef TK_3D_TILE_BRUSH_H
#define TK_3D_TILE_BRUSH_H
#include <ESToolkit/core/tk_tile_placement.h>
#include <AssetIO/level/elysian_tile_entry.hpp>
#include <map>
#include <vector>

struct TileBrushCell3D {
    int column=0,row=0;
    elysian::TileEntry entry;
};
struct TileBrush3D {
    int width=0,height=0;
    int anchorColumn=0,anchorRow=0;
    std::vector<TileBrushCell3D> cells;
};
struct TileBrushPlacement3D {
    AreaPreviewFace face;
    elysian::TileEntry entry;
};
struct TileBrushPlan3D {
    enum class Failure { None, Empty, Outside, Full };
    Failure failure=Failure::None;
    AreaPreviewFace failedFace;
    std::vector<TileBrushPlacement3D> tiles;
    bool valid() const { return failure==Failure::None && !tiles.empty(); }
};

// Column/row steps of gyTilePlaneVerts, converted from world coordinates using
// mapY=(worldY-worldZ)/32. Stamp rows follow the painted face downwards.
inline AreaPreviewFace tileBrushFace3D(AreaPreviewFace anchor,int column,int row) {
    switch(anchor.plane) {
        case 0: anchor.x+=column; anchor.y+=row; break;
        case 1: anchor.x+=column; anchor.y+=row; anchor.elevation-=row; break;
        case 2: case 3: anchor.y+=column+row; anchor.elevation-=row; break;
        case 4: anchor.x+=column; anchor.y+=2*row; anchor.elevation-=row; break;
        case 5: anchor.x+=column; anchor.y+=row-column; anchor.elevation-=row; break;
        case 6: anchor.x+=column; anchor.y+=row+column; anchor.elevation-=row; break;
        default: anchor.plane=-1; break;
    }
    return anchor;
}

// Resolve the whole brush before writing anything. Reservations matter when
// several upright tiles require different slots in the same saved map square.
template<class Read>
TileBrushPlan3D planTileBrush3D(const TileBrush3D& brush,AreaPreviewFace anchor,
                              int width,int height,int preferredLayer,Read read,unsigned layerCount=3) {
    TileBrushPlan3D plan;
    if(brush.cells.empty()) { plan.failure=TileBrushPlan3D::Failure::Empty; return plan; }
    std::map<std::pair<int,int>,std::array<TilePlacementSlot,4>> reserved;
    for(const auto& cell:brush.cells) {
        if(!cell.entry.getIndex()) continue;
        auto face=tileBrushFace3D(anchor,cell.column,cell.row);
        const auto fail=[&](TileBrushPlan3D::Failure reason) {
            if(plan.failure==TileBrushPlan3D::Failure::None) { plan.failure=reason; plan.failedFace=face; }
        };
        if(face.x<0 || face.y<0 || face.x>=width || face.y>=height || face.elevation<0 ||
           face.elevation>31 || face.plane<0 || face.plane>6) {
            fail(TileBrushPlan3D::Failure::Outside);
        } else {
            auto [it,inserted]=reserved.try_emplace({face.x,face.y});
            auto& slots=it->second;
            if(inserted) for(unsigned layer=0;layer<layerCount;++layer) {
                const auto entry=read(layer,face.x,face.y);
                slots[layer]={entry.getIndex()!=0,int(entry.getElev()),int(entry.getPlane())};
            }
            const auto destination=resolveTilePlacement(face,slots,preferredLayer,false,int(layerCount));
            if(destination.action==TilePlacementAction::Reject) fail(TileBrushPlan3D::Failure::Full);
            else {
                face=destination.face;
                slots[face.layer]={true,face.elevation,face.plane};
            }
        }
        plan.tiles.push_back({face,cell.entry});
    }
    if(plan.tiles.empty()) plan.failure=TileBrushPlan3D::Failure::Empty;
    return plan;
}
#endif
