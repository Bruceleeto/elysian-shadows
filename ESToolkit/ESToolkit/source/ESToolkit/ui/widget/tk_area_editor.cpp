/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#include <ESToolkit/ui/widget/tk_area_editor.h>
#include <ESToolkit/ui/widget/tk_area_editor_layout.h>
#include <ESToolkit/ui/widget/tk_map_view.h>
#include <ESToolkit/ui/widget/tk_area_preview.h>
#include <ESToolkit/core/tk_assets.h>
#include <ESToolkit/core/tk_file_utilities.h>
#include <ESToolkit/AssetIO/tk_area.h>
#include <ESToolkit/AssetIO/tk_level.h>
#include <ESGamma/system/elysian_engine.hpp>
#include <ESGamma/crtp/elysian_camera_system.hpp>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QToolBar>
#include <QMenu>
#include <QStatusBar>
#include <QAction>
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
    PreviewSource(QWidget* pane, Assets* assets, AreaPreview* view, QLabel* status):
        QObject(pane), _assets(assets), _view(view), _status(status) {
        _refresh.setSingleShot(true);
        connect(&_refresh,&QTimer::timeout,this,[this]{refresh();});
        connect(assets,&Assets::mapChanged,this,[this](Area* area,const QVector<MapChange>&) {
            // Some edit paths emit before storing the new tile. Read next turn.
            if(area==_assets->getCurrentArea()) _refresh.start(0);
        });
        connect(assets,&Assets::areaChanged,this,[this](Area*){_refresh.start(0);});
        connect(assets,&Assets::levelChanged,this,[this](Level*){_atlasDirty=true;_refresh.start(0);});
        connect(assets,&Assets::projectRemoved,this,[this](Project*){clear();});
        // Selection previews and some property edits suppress mapChanged. A
        // read-only comparison catches those too; rebuild only when data differs.
        _poll.setInterval(100);
        connect(&_poll,&QTimer::timeout,this,[this]{refresh();});
    }
    void setActive(bool active) {
        _active=active;
        if(active) { _poll.start(); refresh(); } else { _poll.stop(); _view->clearHover(); }
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
        if(!_active || !_view->isVisible()) return;
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
        std::vector<AreaPreviewFace> faces;
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
            faces.push_back({int(x),int(y),int(l),int(tile.getElev()),int(tile.getPlane())});
            ++tiles;
        }
        _view->setScene(std::move(mesh),_atlas,reset,std::move(faces));
        if(_atlas.isNull()) _status->setText(tr("Tilesheet unavailable; showing untextured geometry."));
        else if(skipped) _status->setText(tr("%1 tiles · %2 invalid tile entries skipped").arg(tiles).arg(skipped));
        else _status->setText(tr("%1 tiles · Hover to inspect").arg(tiles));
    }
    bool _active=false;
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

void installAreaEditor(QMainWindow* window, Assets* assets, MapView* map,
                       QToolBar* toolbar, QMenu* menu, QAction* play, QDockWidget* palette) {
    auto* grid=qobject_cast<QGridLayout*>(window->centralWidget()->layout());
    auto* splitter=createAreaEditorSplitter(grid,map);

    auto* pane=new QWidget(splitter);
    pane->setObjectName("map3DEditor");
    auto* layout=new QVBoxLayout(pane);
    layout->setContentsMargins(0,0,0,0);
    auto* buttons=new QHBoxLayout;
    auto* select=new QLabel(QObject::tr("Select · 3D"),pane);
    auto* fit=new QPushButton(QObject::tr("Frame area"),pane);
    auto* focus=new QPushButton(QObject::tr("Focus 2D view"),pane);
    fit->setFocusPolicy(Qt::NoFocus); focus->setFocusPolicy(Qt::NoFocus);
    buttons->addWidget(select); buttons->addStretch(); buttons->addWidget(fit); buttons->addWidget(focus);
    layout->addLayout(buttons);
    auto* view=new AreaPreview(pane);
    view->setObjectName("map3DView");
    layout->addWidget(view,1);
    auto* help=new QLabel(QObject::tr("Drag: orbit · Middle/right drag: pan · Wheel: zoom · Hover: inspect tile"),pane);
    help->setWordWrap(true); layout->addWidget(help);
    auto* status=new QLabel(pane);
    layout->addWidget(status);
    splitter->addWidget(pane);
    splitter->addWidget(map);
    splitter->setStretchFactor(0,3); splitter->setStretchFactor(1,1);
    pane->hide();
    auto* source=new PreviewSource(pane,assets,view,status);
    QObject::connect(fit,&QPushButton::clicked,view,&AreaPreview::frameArea);
    QObject::connect(focus,&QPushButton::clicked,source,[source]{source->focusMapView();});

    auto* hoverStatus=new QLabel(window);
    hoverStatus->setObjectName("map3DHoverStatus");
    window->statusBar()->addWidget(hoverStatus,1);
    view->onHover=[map,hoverStatus](const AreaPreviewFace* face) {
        if(!face) { map->set3DHoverTile({-1,-1},false); hoverStatus->clear(); return; }
        const QString plane=face->plane==0 ? QObject::tr("flat") :
            face->standing() ? QObject::tr("standing") : QObject::tr("slope");
        hoverStatus->setText(QObject::tr("Tile (%1, %2) · Layer %3 · Height %4 · %5")
            .arg(face->x).arg(face->y).arg(face->layer).arg(face->elevation).arg(plane));
        map->set3DHoverTile({face->x,face->y},face->standing());
    };

    auto* toggle=new QAction(QObject::tr("3D mode"),window);
    toggle->setObjectName("action3DMode");
    toggle->setCheckable(true); toggle->setShortcut(QKeySequence("Ctrl+3"));
    toggle->setToolTip(QObject::tr("Switch the main map between 2D and 3D (Ctrl+3)"));
    toggle->setEnabled(false);
    toolbar->addSeparator(); toolbar->addAction(toggle); menu->addAction(toggle);
    auto previousPaletteArea=std::make_shared<Qt::DockWidgetArea>(Qt::BottomDockWidgetArea);
    auto previousPaletteFloating=std::make_shared<bool>(false);
    QObject::connect(toggle,&QAction::toggled,window,[=](bool enabled) {
        pane->setVisible(enabled);
        hoverStatus->setVisible(enabled);
        if(enabled) {
            *previousPaletteArea=window->dockWidgetArea(palette);
            *previousPaletteFloating=palette->isFloating();
            palette->setFloating(false); window->addDockWidget(Qt::LeftDockWidgetArea,palette); palette->show();
            const int width=std::max(640,splitter->width());
            splitter->setSizes({width*3/4,width/4});
        } else {
            window->addDockWidget(*previousPaletteArea,palette);
            palette->setFloating(*previousPaletteFloating);
            map->setFocus();
        }
        source->setActive(enabled);
    });
    hoverStatus->hide();
    const auto availability=[=] {
        const bool available=assets->getCurrentArea() && !play->isChecked();
        if(!available) toggle->setChecked(false);
        toggle->setEnabled(available);
    };
    QObject::connect(assets,&Assets::areaChanged,window,[=](Area*) { view->clearHover(); availability(); });
    QObject::connect(assets,&Assets::projectRemoved,window,[=](Project*) { toggle->setChecked(false); toggle->setEnabled(false); });
    QObject::connect(play,&QAction::toggled,window,[=](bool) { availability(); });
}
