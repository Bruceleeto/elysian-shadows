/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef TK_3D_PAINT_STROKE_H
#define TK_3D_PAINT_STROKE_H
#include <AssetIO/level/elysian_tile_entry.hpp>
#include <ESToolkit/ui/widget/tk_area_face.h>
#include <QUndoCommand>
#include <QPoint>
#include <QSet>
#include <array>
#include <functional>
#include <memory>

// Own the normal per-layer tile commands as one drag in the area's undo stack.
// The runtime supplies MapViewPlaceTileGroupUndoItem and TkArea's usual writer.
template<class Command>
class TilePaint3DStroke final : public QUndoCommand {
public:
    using Entry=elysian::TileEntry;
    using Factory=std::function<std::unique_ptr<Command>(int)>;
    using Read=std::function<Entry(int,QPoint)>;
    using Write=std::function<void(int,QPoint,Entry)>;
    TilePaint3DStroke(bool erase, Factory factory, Read read, Write write):
        _factory(std::move(factory)),_read(std::move(read)),_write(std::move(write)),_erase(erase) {
        setText(erase ? QObject::tr("Erase tiles in 3D") : QObject::tr("Paint tiles in 3D"));
    }
    bool paint(const AreaPreviewFace& face,Entry brush,bool repaintVisited=false) {
        if(face.layer<0 || face.layer>=4 || face.x<0 || face.y<0 ||
           face.elevation<0 || face.elevation>31 || face.plane<0 || face.plane>6) return false;
        const quint64 key=(quint64(face.layer)<<32)|(quint64(face.y)<<16)|quint64(face.x);
        if(!repaintVisited && _visited.contains(key)) return false;
        const QPoint point(face.x,face.y);
        const Entry before=_read(face.layer,point);
        // Enforce the placement contract again at the write boundary. A stale
        // preview or future tool must not overwrite unrelated geometry.
        if(before.getIndex() && (int(before.getElev())!=face.elevation || int(before.getPlane())!=face.plane)) return false;
        if(_erase && !before.getIndex()) return false;
        Entry after;
        if(!_erase) { after=brush; after.setElev(face.elevation); after.setPlane(GY_RECT_PLANE_TYPE(face.plane)); }
        after=elysian::paintTileEntry(after,before,unsigned(face.layer));
        if(after==before) return false;
        auto& command=_commands[size_t(face.layer)];
        if(!command) { command=_factory(face.layer); command->setCombine(false); }
        if(!command->addTile(point,after)) return false;
        _write(face.layer,point,after);
        _visited.insert(key);
        ++_changes;
        return true;
    }
    bool empty() const { return !_changes; }
    void undo() override { for(auto i=_commands.rbegin();i!=_commands.rend();++i) if(*i) (*i)->undo(); }
    void redo() override { for(auto& command:_commands) if(command) command->redo(); }
private:
    std::array<std::unique_ptr<Command>,4> _commands;
    QSet<quint64> _visited;
    Factory _factory;
    Read _read;
    Write _write;
    bool _erase;
    unsigned _changes=0;
};
#endif
