/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef TK_BLOCKED_PAINT_STROKE_H
#define TK_BLOCKED_PAINT_STROKE_H
#include <QUndoCommand>
#include <QHash>
#include <QPoint>
#include <cstdlib>

// One drag is one undo item. Only the flag is restored, never the tile artwork.
template<class AreaType>
class BlockedPaintStroke final : public QUndoCommand {
    AreaType* _area;
    bool _blocked;
    QHash<QPoint, bool> _before;
    QPoint _last;
    bool _started = false;
public:
    BlockedPaintStroke(AreaType* area, bool blocked) : _area(area), _blocked(blocked) {
        setText(blocked ? QStringLiteral("Paint blocked squares") : QStringLiteral("Clear blocked squares"));
    }
    bool empty() const { return _before.isEmpty(); }
    void paint(QPoint point) {
        QPoint from = _started ? _last : point;
        _last = point; _started = true;
        const int dx = std::abs(point.x()-from.x()), dy = -std::abs(point.y()-from.y());
        const int sx = from.x()<point.x() ? 1 : -1, sy = from.y()<point.y() ? 1 : -1;
        int error = dx+dy;
        _area->beginMapUpdate();
        for(;;) {
            if(from.x()>=0 && from.y()>=0 && from.x()<_area->width && from.y()<_area->height) {
                const bool old = _area->getTileEntry(0,from.x(),from.y()).getBlocked();
                if(old != _blocked) {
                    if(!_before.contains(from)) _before.insert(from,old);
                    _area->setBlocked(from,_blocked);
                }
            }
            if(from == point) break;
            const int twice = 2*error;
            if(twice>=dy) { error+=dy; from.rx()+=sx; }
            if(twice<=dx) { error+=dx; from.ry()+=sy; }
        }
        _area->endMapUpdate();
    }
    void undo() override {
        _area->beginMapUpdate();
        for(auto it=_before.cbegin(); it!=_before.cend(); ++it) _area->setBlocked(it.key(),it.value());
        _area->endMapUpdate();
    }
    void redo() override {
        _area->beginMapUpdate();
        for(auto it=_before.cbegin(); it!=_before.cend(); ++it) _area->setBlocked(it.key(),_blocked);
        _area->endMapUpdate();
    }
};
#endif
