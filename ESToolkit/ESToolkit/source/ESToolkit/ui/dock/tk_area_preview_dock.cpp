/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#include <ESToolkit/ui/dock/tk_area_preview_dock.h>
#include <ESToolkit/ui/widget/tk_area_preview.h>
#include <ESToolkit/core/tk_assets.h>
#include <ESToolkit/core/tk_file_utilities.h>
#include <ESToolkit/AssetIO/tk_area.h>
#include <ESToolkit/AssetIO/tk_level.h>
#include <ESGamma/system/elysian_engine.hpp>
#include <ESGamma/crtp/elysian_camera_system.hpp>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <limits>
#include <algorithm>

namespace {
// The adapter only reads the current tile table and requests the engine's CPU
// vertex construction. GPU resources and the camera belong to AreaPreview.
class PreviewSource final : public QObject {
public:
    PreviewSource(QDockWidget* dock, Assets* assets, AreaPreview* view, QLabel* status):
        QObject(dock), _dock(dock), _assets(assets), _view(view), _status(status) {
        _refresh.setSingleShot(true);
        connect(&_refresh,&QTimer::timeout,this,[this]{refresh();});
        connect(assets,&Assets::mapChanged,this,[this](Area* area,const QVector<MapChange>&) {
            // Some edit paths emit before storing the new tile. Read next turn.
            if(area==_assets->getCurrentArea()) _refresh.start(0);
        });
        connect(assets,&Assets::areaChanged,this,[this](Area*){_refresh.start(0);});
        connect(assets,&Assets::levelChanged,this,[this](Level*){_atlasDirty=true;_refresh.start(0);});
        connect(assets,&Assets::projectRemoved,this,[this](Project*){clear();});
        connect(dock,&QDockWidget::visibilityChanged,this,[this](bool visible){
            if(visible) { _poll.start(); _refresh.start(0); } else _poll.stop();
        });
        // Selection previews and some property edits suppress mapChanged. A
        // read-only comparison catches those too; rebuild only when data differs.
        _poll.setInterval(100);
        connect(&_poll,&QTimer::timeout,this,[this]{refresh();});
        if(dock->isVisible()) _poll.start();
    }
    void focusMapView() {
        refresh();
        if(!_assets->getEngine() || _view->mesh().empty()) return;
        const auto* camera=_assets->getEngine()->getCameraSys()->getCurrentState();
        float closest=std::numeric_limits<float>::max();
        QVector3D target;
        for(const auto& v:_view->mesh()) {
            const float dx=v.position.x()-camera->_pos.x;
            const float dy=v.position.y()-v.position.z()-camera->_pos.y;
            const float d=dx*dx+dy*dy;
            if(d<closest) { closest=d; target=v.position; }
        }
        _view->focusWorldPoint(target);
    }
private:
    void clear() {
        disconnect(_levelConnection);
        _entries.clear(); _area=nullptr; _level=nullptr; _atlas=QImage(); _atlasDirty=true;
        _view->setScene({},QImage(),false); _status->setText(tr("Open an area to preview its tiles."));
    }
    void refresh() {
        if(!_dock->isVisible()) return;
        auto* area=_assets->getCurrentArea();
        auto* level=area? area->getLevel() : nullptr;
        if(!area || !area->tiles || !level || !level->tileSheet || !level->tileSheet->_coord) {
            if(_area || !_entries.empty()) clear();
            return;
        }
        const bool reset=(area!=_area || _width!=area->width || _height!=area->height);
        if(level!=_level) {
            disconnect(_levelConnection);
            _level=level; _atlasDirty=true;
            _levelConnection=connect(level,&TkLevel::validChanged,this,[this](bool){_atlasDirty=true;_refresh.start(0);});
        }
        const unsigned count=area->width*area->height*ELYSIAN_AREA_MAP_LAYERS;
        std::vector<uint32_t> entries(count);
        for(unsigned i=0;i<count;++i) entries[i]=area->tiles[i].getValue();
        if(!reset && !_atlasDirty && entries==_entries) return;
        _area=area; _width=area->width; _height=area->height;
        _entries=std::move(entries);
        if(_atlasDirty) {
            _atlas.load(FileUtilities::getAbsoluteAssetPath(level->getNodePath())+ELYSIAN_TILESHEET_PATH);
            _atlasDirty=false;
        }
        std::vector<AreaPreviewVertex> mesh;
        unsigned skipped=0, tiles=0;
        const auto countPresent=std::count_if(_entries.begin(),_entries.end(),[](uint32_t value){return (value&511)!=0;});
        mesh.reserve(static_cast<size_t>(countPresent)*6);
        for(unsigned l=0;l<ELYSIAN_AREA_MAP_LAYERS;++l) for(unsigned y=0;y<area->height;++y) for(unsigned x=0;x<area->width;++x) {
            const auto tile=area->getTileEntry(l,x,y);
            if(!tile.getIndex()) continue;
            if(tile.getPlane()>=GY_RECT_PLANE_MAX || tile.getIndex()>=level->tileAmount) { ++skipped; continue; }
            GYVertex quad[4];
            area->vboEntryFromTileEntry(quad,&tile,x,y,l);
            const auto position=[&](unsigned i){return QVector3D(quad[i].pos.x,quad[i].pos.y,quad[i].pos.z);};
            const auto normal=QVector3D::crossProduct(position(1)-position(0),position(2)-position(0)).normalized();
            for(unsigned i:{0u,1u,2u,2u,1u,3u})
                mesh.push_back({position(i),{quad[i].texCoord.u,quad[i].texCoord.v},normal});
            ++tiles;
        }
        _view->setScene(std::move(mesh),_atlas,reset);
        if(_atlas.isNull()) _status->setText(tr("Tilesheet unavailable; showing untextured geometry."));
        else if(skipped) _status->setText(tr("%1 tiles · %2 invalid tile entries skipped").arg(tiles).arg(skipped));
        else _status->setText(tr("%1 tiles · Live, view only").arg(tiles));
    }
    QDockWidget* _dock;
    Assets* _assets;
    AreaPreview* _view;
    QLabel* _status;
    QTimer _refresh, _poll;
    QPointer<TkArea> _area;
    QPointer<TkLevel> _level;
    QMetaObject::Connection _levelConnection;
    std::vector<uint32_t> _entries;
    unsigned _width=0, _height=0;
    QImage _atlas;
    bool _atlasDirty=true;
};
}

QDockWidget* createAreaPreviewDock(QMainWindow* window, Assets* assets) {
    auto* dock=new QDockWidget(QObject::tr("3D Preview"),window);
    dock->setObjectName("area3DPreviewDock");
    auto* content=new QWidget(dock);
    auto* layout=new QVBoxLayout(content);
    layout->setContentsMargins(4,4,4,4);
    auto* buttons=new QHBoxLayout;
    auto* fit=new QPushButton(QObject::tr("Frame area"),content);
    auto* focus=new QPushButton(QObject::tr("Focus 2D view"),content);
    fit->setFocusPolicy(Qt::NoFocus); focus->setFocusPolicy(Qt::NoFocus);
    buttons->addWidget(fit); buttons->addWidget(focus); buttons->addStretch();
    layout->addLayout(buttons);
    auto* view=new AreaPreview(content);
    layout->addWidget(view,1);
    auto* help=new QLabel(QObject::tr("Drag: orbit · Middle/right drag: pan · Wheel: zoom"),content);
    help->setWordWrap(true); layout->addWidget(help);
    auto* status=new QLabel(QObject::tr("Open an area to preview its tiles."),content);
    status->setWordWrap(true); layout->addWidget(status);
    dock->setWidget(content);
    auto* source=new PreviewSource(dock,assets,view,status);
    QObject::connect(fit,&QPushButton::clicked,view,&AreaPreview::frameArea);
    QObject::connect(focus,&QPushButton::clicked,source,[source]{source->focusMapView();});
    return dock;
}
