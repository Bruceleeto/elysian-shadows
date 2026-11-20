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
#include <QActionGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QSignalBlocker>
#include <QToolButton>
#include <ESToolkit/core/tk_3d_paint_stroke.h>
#include <ESToolkit/core/tk_3d_tile_brush.h>
#include <ESToolkit/core/tk_tile_image.h>
#include <QPainter>
#include <ESToolkit/core/tk_map_view_place_tile_group_undo_item.h>
#include <ESToolkit/ui/widget/tk_engine_gl_scene_widget.h>
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
    void refreshNow() { refresh(); }
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
        _view->setLayerCount(int(area->getLayerCount()));
        const unsigned count=area->width*area->height*area->getLayerCount();
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
        for(unsigned l=0;l<area->getLayerCount();++l) for(unsigned y=0;y<area->height;++y) for(unsigned x=0;x<area->width;++x) {
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

class PaintSource final : public QObject {
public:
    PaintSource(QWidget* parent,Assets* assets,MapView* map,AreaPreview* view,PreviewSource* source,
                QLabel* help,QLabel* brushImage,QLabel* brushCaption):
        QObject(parent),_assets(assets),_map(map),_view(view),_source(source),_help(help),
        _brushImage(brushImage),_brushCaption(brushCaption) {
        view->onStrokeBegin=[this](bool erase) { return begin(erase); };
        view->onPaint=[this](const AreaPreviewFace& face) { return paint(face); };
        view->onStrokeEnd=[this] { finish(); };
        view->onSampleTile=[this](const AreaPreviewFace& face) { return sample(face); };
        view->setPaintLayer(int(assets->getCurrentLayer()));
        connect(assets,&Assets::layerChanged,view,[view](Assets::LAYER_TYPE layer) { view->setPaintLayer(int(layer)); });
        view->canPlace=[this](AreaPreviewFace& face) {
            auto* area=_assets->getCurrentArea();
            const int layer=int(_assets->getCurrentLayer());
            return area && layer>=0 && layer<int(area->getLayerCount()) && face.x>=0 && face.y>=0 &&
                face.x<area->width && face.y<area->height;
        };
        view->placementSlots=[this](int x,int y) {
            std::array<TilePlacementSlot,4> slots{};
            auto* area=_assets->getCurrentArea();
            if(area && x>=0 && y>=0 && x<area->width && y<area->height) for(unsigned layer=0;layer<area->getLayerCount();++layer) {
                const auto entry=area->getTileEntry(layer,x,y);
                slots[layer]={entry.getIndex()!=0,int(entry.getElev()),int(entry.getPlane())};
            }
            return slots;
        };
        view->onFacePreview=[this](const AreaPreviewFace& face) {
            TileBrush3D brush;
            if(!currentBrush(brush)) return std::vector<AreaPreviewVertex>{};
            return geometry(face,brush.cells.front().entry);
        };
        view->onBrushPreview=[this](const AreaPreviewFace& face) {
            std::vector<AreaPreviewVertex> vertices;
            TileBrush3D brush;
            if(!currentBrush(brush)) return vertices;
            const auto plan=planBrush(brush,face);
            for(const auto& tile:plan.tiles) {
                const auto quad=geometry(tile.face,tile.entry);
                vertices.insert(vertices.end(),quad.begin(),quad.end());
            }
            return vertices;
        };
        view->onPlacementError=[this](const AreaPreviewFace& face) {
            TileBrush3D brush;
            if(!currentBrush(brush)) return tr("Choose tiles in Sheet View or right-click a tile in the world.");
            return planError(planBrush(brush,face));
        };
        connect(assets,&Assets::areaChanging,view,[view](Area*) { view->finishStroke(); });
        connect(assets,&Assets::projectRemoved,view,[view](Project*) { view->finishStroke(); });
        connect(assets,&Assets::selectionChanged,this,[this](Selectable::TYPE,Selectable*) {
            _view->finishStroke(); updateBrushDisplay();
        });
        connect(assets,&Assets::areaChanged,this,[this](Area*) { updateBrushDisplay(); });
        updateBrushDisplay();
    }
private:
    bool readBrush(TileBrush3D& brush) const {
        brush={};
        auto* selection=_assets->getCurrentSelectionType()==Selectable::TILE ?
            static_cast<TileSelectable*>(_assets->getCurrentSelection()) : nullptr;
        unsigned width=0,height=0;
        if(selection) selection->getSelectionArea(width,height);
        if(!selection || !width || !height) return false;
        brush.width=int(width); brush.height=int(height);
        for(unsigned y=0;y<height;++y) for(unsigned x=0;x<width;++x) {
            if(!selection->isWithinSelection(int(x),int(y))) continue;
            const auto entry=selection->getEntry(x,y);
            if(entry.getIndex()) brush.cells.push_back({int(x),int(y),entry});
        }
        if(brush.cells.empty()) return false;
        // The cursor anchors the first painted tile, including sparse selections.
        brush.anchorColumn=brush.cells.front().column; brush.anchorRow=brush.cells.front().row;
        for(auto& cell:brush.cells) { cell.column-=brush.anchorColumn; cell.row-=brush.anchorRow; }
        return true;
    }
    bool currentBrush(TileBrush3D& brush) const {
        if(_stroke && !_erase) { brush=_brush; return !brush.cells.empty(); }
        return readBrush(brush);
    }
    TileBrushPlan3D planBrush(const TileBrush3D& brush,const AreaPreviewFace& face) const {
        auto* area=_assets->getCurrentArea();
        if(!area) return {};
        return planTileBrush3D(brush,face,area->width,area->height,int(_assets->getCurrentLayer()),
            [area](int layer,int x,int y) { return area->getTileEntry(unsigned(layer),x,y); },area->getLayerCount());
    }
    QString planError(const TileBrushPlan3D& plan) const {
        using Failure=TileBrushPlan3D::Failure;
        if(plan.valid()) return {};
        if(plan.failure==Failure::Full)
            return tr("Brush cannot fit at (%1, %2): all available slots hold other surfaces. Nothing will be painted.")
                .arg(plan.failedFace.x).arg(plan.failedFace.y);
        if(plan.failure==Failure::Outside)
            return tr("Brush extends outside the map or height range at (%1, %2). Nothing will be painted.")
                .arg(plan.failedFace.x).arg(plan.failedFace.y);
        return tr("Choose tiles in Sheet View or right-click a tile in the world.");
    }
    std::vector<AreaPreviewVertex> geometry(const AreaPreviewFace& face,elysian::TileEntry brush) const {
        std::vector<AreaPreviewVertex> vertices;
        auto* area=_assets->getCurrentArea();
        auto* level=area ? area->getLevel() : nullptr;
        if(!level || !level->tileSheet || !level->tileSheet->_coord || brush.getIndex()>=level->tileAmount ||
           face.x<0 || face.y<0 || face.elevation<0 || face.elevation>31 || face.plane<0 || face.plane>=GY_RECT_PLANE_MAX) return vertices;
        brush.setElev(face.elevation); brush.setPlane(GY_RECT_PLANE_TYPE(face.plane));
        GYVertex quad[4];
        area->vboEntryFromTileEntry(quad,&brush,face.x,face.y,face.layer);
        const auto position=[&](unsigned i){return QVector3D(quad[i].pos.x,quad[i].pos.y,quad[i].pos.z);};
        const auto normal=QVector3D::crossProduct(position(1)-position(0),position(2)-position(0)).normalized();
        for(unsigned i:{0u,1u,2u,2u,1u,3u})
            vertices.push_back({position(i),{quad[i].texCoord.u,quad[i].texCoord.v},normal});
        return vertices;
    }
    bool sample(const AreaPreviewFace& face) {
        auto* area=_assets->getCurrentArea();
        if(!area || !_assets->getEngine() || _assets->getEngine()->getEngineState()!=elysian::System::ENGINE_ESTK_STOP ||
           face.x<0 || face.y<0 || face.x>=area->width || face.y>=area->height || face.layer<0 || face.layer>=int(area->getLayerCount())) return false;
        _map->set3DPaintActive(true);
        const auto entry=area->getTileEntry(unsigned(face.layer),face.x,face.y);
        if(!entry.getIndex() || int(entry.getElev())!=face.elevation || int(entry.getPlane())!=face.plane) {
            _map->set3DPaintActive(false); _source->refreshNow(); return false;
        }
        auto* selection=new TileSelectable(QRect(face.x,face.y,1,1),nullptr,area,area->getLevel(),Assets::LAYER_TYPE(face.layer));
        _assets->setCurrentLayer(Assets::LAYER_TYPE(face.layer));
        _assets->setCurrentSelection(selection);
        _view->setPlacementPlane(face.plane);
        _map->set3DPaintActive(false);
        return true;
    }
public:
    void updateBrushDisplay() {
        if(_view->tool()==AreaPreview::Tool::Erase) {
            QPixmap icon(160,160); icon.fill(Qt::transparent);
            QPainter painter(&icon); painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(QPen(QColor(235,65,65),12,Qt::SolidLine,Qt::RoundCap));
            painter.drawLine(40,40,120,120); painter.drawLine(120,40,40,120);
            painter.end();
            _brushImage->setPixmap(icon); _brushCaption->setText(tr("Erase\nLeft-click or drag to remove tiles"));
            return;
        }
        TileBrush3D brush;
        auto* level=_assets->getCurrentLevel();
        if(!level || !readBrush(brush)) {
            _brushImage->clear(); _brushCaption->setText(tr("No brush\nRight-click a tile")); return;
        }
        QPixmap image(brush.width*32,brush.height*32); image.fill(Qt::transparent);
        QPainter painter(&image);
        for(const auto& cell:brush.cells) {
            const auto* tile=level->getTileImage(cell.entry.getIndex());
            if(!tile) continue;
            painter.save(); painter.translate((cell.column+brush.anchorColumn)*32+16,(cell.row+brush.anchorRow)*32+16);
            // Same texture orientation as the sheet/map pixmap renderer.
            switch(cell.entry.getOrient()) {
                case GY_RECT_TEX_ORIENT_FLIP_HORIZ: painter.scale(-1,1); break;
                case GY_RECT_TEX_ORIENT_FLIP_VERT: painter.scale(1,-1); break;
                case GY_RECT_TEX_ORIENT_ROT_90: painter.rotate(-90); break;
                case GY_RECT_TEX_ORIENT_ROT_180: painter.rotate(180); break;
                case GY_RECT_TEX_ORIENT_ROT_270: painter.rotate(-270); break;
                case GY_RECT_TEX_ORIENT_FLIP_HORIZ_ROT_90: painter.scale(-1,1); painter.rotate(-90); break;
                case GY_RECT_TEX_ORIENT_FLIP_HORIZ_ROT_270: painter.scale(-1,1); painter.rotate(-270); break;
                default: break;
            }
            painter.drawPixmap(-16,-16,32,32,tile->getPixmap()); painter.restore();
        }
        painter.end();
        _brushImage->setPixmap(image.scaled(160,160,Qt::KeepAspectRatio,Qt::FastTransformation));
        _brushCaption->setText(brush.cells.size()==1 ? tr("Brush: tile %1\n1 × 1").arg(brush.cells.front().entry.getIndex()) :
            tr("Brush: %1 tiles\n%2 × %3").arg(qulonglong(brush.cells.size())).arg(brush.width).arg(brush.height));
        unsigned solidCount=0;
        for(const auto& cell:brush.cells) if(cell.entry.getSolid())++solidCount;
        const auto solidity=solidCount==brush.cells.size()?tr("Solid"):
            solidCount==0?tr("Pass-through"):tr("Mixed solidity");
        _brushCaption->setText(_brushCaption->text()+"\n"+solidity);
    }
private:
    bool begin(bool erase) {
        auto* area=_assets->getCurrentArea();
        if(!area || !_assets->getEngine() || _assets->getEngine()->getEngineState()!=elysian::System::ENGINE_ESTK_STOP) return false;
        const int layer=int(_assets->getCurrentLayer());
        if(layer<0 || layer>=int(area->getLayerCount())) { _help->setText(tr("Select a tile layer in Layer View to paint.")); return false; }
        if(!erase && !readBrush(_brush)) {
            _help->setText(tr("Choose tiles in Sheet View or right-click a tile in the world.")); return false;
        }
        _map->set3DPaintActive(true); _source->refreshNow(); _area=area; _erase=erase;
        _stroke=std::make_unique<Stroke>(erase,
            [area](int layer) { return std::make_unique<MapViewPlaceTileGroupUndoItem>(area,Assets::LAYER_TYPE(layer)); },
            [area](int layer,QPoint point) { return area->getTileEntry(point,Assets::LAYER_TYPE(layer)); },
            [area](int layer,QPoint point,elysian::TileEntry entry) {
                EngineGLSceneWidget::makeCurrentContext(); area->setTileEntry(point,Assets::LAYER_TYPE(layer),TileEntry(entry));
            });
        _help->setText(erase ? tr("Erasing · Release to finish one Undo step") : tr("Painting brush · Release to finish one Undo step"));
        return true;
    }
    bool paint(const AreaPreviewFace& face) {
        if(!_stroke || !_area || face.x<0 || face.y<0 || face.x>=_area->width || face.y>=_area->height) return false;
        if(_erase) return _stroke->paint(face,{});
        const auto plan=planBrush(_brush,face);
        if(!plan.valid()) { _help->setText(planError(plan)); return false; }
        _area->beginMapUpdate();
        bool changed=false;
        for(const auto& tile:plan.tiles) changed=_stroke->paint(tile.face,tile.entry,true) || changed;
        _area->endMapUpdate();
        return changed;
    }
    void finish() {
        if(_stroke && _area && !_stroke->empty()) {
            EngineGLSceneWidget::makeCurrentContext(); _area->getUndoStack().push(_stroke.release());
        }
        _stroke.reset(); _area=nullptr; _brush={};
        _map->set3DPaintActive(false); _help->setText(_help->property("navigationHelp").toString());
    }
    using Stroke=TilePaint3DStroke<MapViewPlaceTileGroupUndoItem>;
    Assets* _assets;
    MapView* _map;
    AreaPreview* _view;
    PreviewSource* _source;
    QLabel* _help;
    QLabel* _brushImage;
    QLabel* _brushCaption;
    QPointer<TkArea> _area;
    TileBrush3D _brush;
    bool _erase=false;
    std::unique_ptr<Stroke> _stroke;
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
    auto* tools=new QToolBar(pane);
    tools->setToolButtonStyle(Qt::ToolButtonTextOnly);
    auto* fit=new QPushButton(QObject::tr("Frame area"),pane);
    auto* focus=new QPushButton(QObject::tr("Focus 2D view"),pane);
    fit->setFocusPolicy(Qt::NoFocus); focus->setFocusPolicy(Qt::NoFocus);
    auto* sideView=new QCheckBox(QObject::tr("2D side view"),pane);
    sideView->setFocusPolicy(Qt::NoFocus); focus->hide();
    buttons->addWidget(sideView); buttons->addWidget(tools); buttons->addStretch(); buttons->addWidget(fit); buttons->addWidget(focus);
    layout->addLayout(buttons);
    auto* view=new AreaPreview(pane);
    view->setObjectName("map3DView");
    view->setFocusPolicy(Qt::ClickFocus);

    auto* solid=new QAction(QObject::tr("Solid"),window);
    solid->setObjectName("actionPaintSolid");
    solid->setCheckable(true); solid->setChecked(true);
    solid->setToolTip(QObject::tr("Solid tiles stop bodies at their visible surface. Uncheck, then paint to make a surface pass-through. Right-click samples this setting too."));
    toolbar->addAction(solid); tools->addAction(solid); tools->addSeparator();
    const auto syncSolid=[=]() {
        auto* selection=assets->getCurrentSelectionType()==Selectable::TILE ?
            static_cast<TileSelectable*>(assets->getCurrentSelection()) : nullptr;
        unsigned w=0,h=0; if(selection) selection->getSelectionArea(w,h);
        bool allSolid=true,any=false;
        for(unsigned y=0;y<h;++y) for(unsigned x=0;x<w;++x) {
            if(!selection->isWithinSelection(x,y)) continue;
            const auto tile=selection->getEntry(x,y);
            if(tile.getIndex()) { any=true; allSolid=allSolid && tile.getSolid(); }
        }
        solid->blockSignals(true); solid->setChecked(allSolid); solid->setEnabled(any); solid->blockSignals(false);
    };
    QObject::connect(assets,&Assets::selectionChanged,solid,[=](Selectable::TYPE,Selectable*) { syncSolid(); });
    QObject::connect(solid,&QAction::toggled,assets,[=](bool enabled) {
        if(assets->getCurrentSelectionType()!=Selectable::TILE) return;
        auto* selection=new TileSelectable(*static_cast<TileSelectable*>(assets->getCurrentSelection()));
        unsigned w=0,h=0; selection->getSelectionArea(w,h);
        for(unsigned y=0;y<h;++y) for(unsigned x=0;x<w;++x) {
            auto tile=selection->getEntry(x,y); tile.setSolid(enabled); selection->setEntry(x,y,std::move(tile));
        }
        assets->setCurrentSelection(selection);
    });
    syncSolid();

    auto* toolGroup=new QActionGroup(tools);
    const auto addTool=[&](const QString& label,AreaPreview::Tool tool) {
        auto* action=tools->addAction(label); action->setCheckable(true); action->setData(int(tool)); toolGroup->addAction(action);
        QObject::connect(action,&QAction::triggered,view,[view,tool]{ view->setTool(tool); });
        return action;
    };
    addTool(QObject::tr("Select"),AreaPreview::Tool::Select);
    auto* paintAction=addTool(QObject::tr("Paint"),AreaPreview::Tool::Paint); paintAction->setChecked(true);
    auto* wallAction=addTool(QObject::tr("Wall"),AreaPreview::Tool::Wall);
    wallAction->setToolTip(QObject::tr("Build upright tiles: point at a floor edge, then left-drag. Height snaps automatically."));
    addTool(QObject::tr("Erase"),AreaPreview::Tool::Erase);
    view->setTool(AreaPreview::Tool::Paint);
    for(auto* button:tools->findChildren<QToolButton*>()) button->setFocusPolicy(Qt::NoFocus);
    auto* directionRow=new QHBoxLayout;
    auto* direction=new QComboBox(pane);
    direction->setObjectName("placementPlane3D");
    direction->setFocusPolicy(Qt::NoFocus);
    direction->addItem(QObject::tr("Match surface"),-1);
    direction->addItem(QObject::tr("Flat (Down)"),0);
    direction->addItem(QObject::tr("Upright / front (Up)"),1);
    direction->addItem(QObject::tr("Upright / left (Left)"),2);
    direction->addItem(QObject::tr("Upright / right (Right)"),3);
    direction->addItem(QObject::tr("Front slope"),4);
    direction->addItem(QObject::tr("Diagonal wall / left"),5);
    direction->addItem(QObject::tr("Diagonal wall / right"),6);
    direction->setToolTip(QObject::tr("Sets the direction of the tile you paint. Arrow keys choose flat/front/left/right; N cycles all directions."));
    directionRow->addWidget(new QLabel(QObject::tr("Tile direction:"),pane));
    directionRow->addWidget(direction);
    auto* directionHint=new QLabel(QObject::tr("Choose artwork and direction, then click. Green adds a surface; amber repaints that same surface."),pane);
    directionHint->setWordWrap(true); directionRow->addWidget(directionHint,1);
    layout->addLayout(directionRow);
    QObject::connect(direction,&QComboBox::currentIndexChanged,view,[=](int index) { view->setPlacementPlane(direction->itemData(index).toInt()); });

    auto* alignment=new QToolButton(pane);
    alignment->setText(QObject::tr("Alignment…")); alignment->setCheckable(true); alignment->setFocusPolicy(Qt::NoFocus);
    directionRow->addWidget(alignment);
    auto* surfacePanel=new QWidget(pane);
    auto* surfaceControls=new QHBoxLayout(surfacePanel); surfaceControls->setContentsMargins(0,0,0,0);
    auto* chooseSurface=new QPushButton(QObject::tr("Pick surface"),pane);
    chooseSurface->setCheckable(true);
    chooseSurface->setToolTip(QObject::tr("Click this, then an existing wall or floor. Paint stays on that surface across gaps. Shortcut: Shift-click a tile."));
    auto* autoSurface=new QPushButton(QObject::tr("Auto"),pane);
    autoSurface->setToolTip(QObject::tr("Release the chosen surface (Escape). Each new drag follows its first tile."));
    auto* surfaceLabel=new QLabel(QObject::tr("Surface: automatic · Each drag stays on its starting surface"),pane);
    surfaceLabel->setWordWrap(true);
    chooseSurface->setFocusPolicy(Qt::NoFocus); autoSurface->setFocusPolicy(Qt::NoFocus);
    surfaceControls->addWidget(chooseSurface); surfaceControls->addWidget(autoSurface); surfaceControls->addWidget(surfaceLabel,1);
    layout->addWidget(surfacePanel); surfacePanel->hide();
    QObject::connect(alignment,&QToolButton::toggled,surfacePanel,&QWidget::setVisible);
    tools->removeAction(wallAction);
    auto* wallEdge=new QToolButton(surfacePanel); wallEdge->setDefaultAction(wallAction);
    wallAction->setText(QObject::tr("Build wall edge")); wallEdge->setFocusPolicy(Qt::NoFocus);
    surfaceControls->addWidget(wallEdge);
    QObject::connect(chooseSurface,&QPushButton::clicked,view,[=](bool checked) {
        if(checked) view->pickSurface(); else view->clearSurface();
    });
    QObject::connect(autoSurface,&QPushButton::clicked,view,&AreaPreview::clearSurface);

    layout->addWidget(view,1);
    auto* help=new QLabel(QObject::tr("Paint preview: left drag · Pick tile: right click · Delete: Erase tool · Orbit: Alt-left drag · Pan: middle drag · Wheel: zoom"),pane);
    auto* footer=new QHBoxLayout;
    auto* notices=new QVBoxLayout;
    help->setWordWrap(true); notices->addWidget(help);
    footer->addLayout(notices,1);
    layout->addLayout(footer);
    auto* brushDock=new QDockWidget(QObject::tr("Current tile"),window);
    brushDock->setObjectName("currentTile3DDock");
    brushDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto* brushPanel=new QWidget(brushDock);
    auto* brushLayout=new QVBoxLayout(brushPanel);
    auto* brushImage=new QLabel(brushPanel); brushImage->setObjectName("brushImage3D");
    brushImage->setMinimumSize(176,176); brushImage->setAlignment(Qt::AlignCenter);
    auto* brushCaption=new QLabel(brushPanel); brushCaption->setObjectName("brushCaption3D");
    brushCaption->setAlignment(Qt::AlignCenter); brushCaption->setWordWrap(true);
    brushLayout->addWidget(brushImage,1); brushLayout->addWidget(brushCaption);
    brushDock->setWidget(brushPanel);
    window->addDockWidget(Qt::RightDockWidgetArea,brushDock);
    if(auto* selectionDock=window->findChild<QDockWidget*>("SelectionDockWidget"))
        window->splitDockWidget(selectionDock,brushDock,Qt::Vertical);
    brushDock->hide();
    help->setProperty("navigationHelp",help->text());
    const auto updateToolHelp=[help](AreaPreview::Tool tool) {
        const QString hint=tool==AreaPreview::Tool::Select ?
            QObject::tr("Select: hover to inspect · Left drag: orbit · Right click: pick tile · Middle drag: pan · Wheel: zoom") :
            tool==AreaPreview::Tool::Wall ?
            QObject::tr("Wall: hover near a floor edge, left-drag to build upright · Extend walls by pointing beside or above them · Right click: pick tile") :
            tool==AreaPreview::Tool::Erase ?
            QObject::tr("Erase: left drag · Right-click: pick tile · Shift-click: choose surface · Alt-left: orbit · Middle: pan · Undo: Ctrl+Z") :
            QObject::tr("Paint preview: left drag · Pick tile: right click · Delete: Erase tool · Shift-click: choose surface · Alt-left: orbit · Middle: pan · Undo: Ctrl+Z");
        help->setProperty("navigationHelp",hint); help->setText(hint);
    };
    view->onPlacementPlaneChanged=[=](int plane) {
        const QSignalBlocker blocker(direction);
        direction->setCurrentIndex(direction->findData(plane));
        if(plane>=0) paintAction->setChecked(true);
        const QString hint=plane>=0 ? QObject::tr("%1 · Left-click/drag: place · Arrow keys: change direction · Right-click: pick tile").arg(direction->currentText()) :
            QObject::tr("Match surface · Paint follows the pointed tile · Choose Tile direction to override flat/upright");
        help->setProperty("navigationHelp",hint); help->setText(hint);
    };
    view->onSurfaceChanged=[=] {
        chooseSurface->setChecked(view->pickingSurface());
        if(view->pickingSurface() || view->lockedSurface()) alignment->setChecked(true);
        if(view->pickingSurface()) surfaceLabel->setText(QObject::tr("Click an existing wall or floor. This chooses a surface without painting."));
        else if(const auto* face=view->lockedSurface()) {
            paintAction->setChecked(true);
            const QString hint=QObject::tr("Paint chosen surface: left drag · Erase tool: left drag · Shift-click: choose another surface · Escape: automatic");
            help->setProperty("navigationHelp",hint); help->setText(hint);
            const QString direction=face->plane==0 ? QObject::tr("flat") : face->standing() ? QObject::tr("upright wall") : QObject::tr("slope");
            surfaceLabel->setText(QObject::tr("Surface locked: %1 at (%2, %3) · Paint across gaps · Auto / Escape releases")
                .arg(direction).arg(face->x).arg(face->y));
        } else {
            surfaceLabel->setText(QObject::tr("Surface: automatic · Each drag stays on its starting surface"));
            const QString hint=QObject::tr("Choose artwork, then Paint a surface or use Wall at a floor edge · Shift-click: choose a surface · Alt-left: orbit");
            help->setProperty("navigationHelp",hint); help->setText(hint);
        }
    };
    auto* placementHint=new QLabel(pane);
    placementHint->setWordWrap(true);
    notices->addWidget(placementHint);
    view->onPlacementHint=[placementHint](const QString& hint) { placementHint->setText(hint); };
    auto* status=new QLabel(pane);
    notices->addWidget(status);
    splitter->addWidget(pane);
    splitter->addWidget(map);
    splitter->setStretchFactor(0,3); splitter->setStretchFactor(1,1);
    pane->hide();
    QObject::connect(sideView,&QCheckBox::toggled,pane,[=](bool visible) {
        map->setVisible(visible); focus->setVisible(visible);
        if(visible) { const int width=splitter->width(); splitter->setSizes({width*3/4,width/4}); }
    });
    const auto layerCaption=[=](Assets::LAYER_TYPE layer) {
        const int value=int(layer);
        directionHint->setText(value<4 ? QObject::tr("New surfaces prefer Layer %1 · Green: add · Amber: repaint existing surface").arg(value+1) :
            QObject::tr("Select a tile layer in Layer View to paint."));
    };
    layerCaption(assets->getCurrentLayer());
    QObject::connect(assets,&Assets::layerChanged,directionHint,layerCaption);
    auto* source=new PreviewSource(pane,assets,view,status);
    auto* painter=new PaintSource(pane,assets,map,view,source,help,brushImage,brushCaption);
    view->onToolChanged=[=](AreaPreview::Tool tool) {
        for(auto* action:toolGroup->actions()) action->setChecked(action->data().toInt()==int(tool));
        updateToolHelp(tool); painter->updateBrushDisplay();
        brushDock->setWindowTitle(tool==AreaPreview::Tool::Erase ? QObject::tr("Eraser") : QObject::tr("Current tile"));
    };
    QObject::connect(assets,&Assets::selectionChanged,view,[=](Selectable::TYPE type,Selectable* selection) {
        if(!pane->isVisible() || type!=Selectable::TILE || !selection) return;
        unsigned width=0,height=0;
        static_cast<TileSelectable*>(selection)->getSelectionArea(width,height);
        if(width && height && !wallAction->isChecked()) paintAction->trigger();
    });
    QObject::connect(fit,&QPushButton::clicked,view,&AreaPreview::frameArea);
    QObject::connect(focus,&QPushButton::clicked,source,[source]{source->focusMapView();});

    auto* hoverStatus=new QLabel(window);
    hoverStatus->setObjectName("map3DHoverStatus");
    window->statusBar()->addWidget(hoverStatus,1);
    view->onHover=[map,hoverStatus,assets](const AreaPreviewFace* face) {
        if(!face) { map->set3DHoverTile({-1,-1},false); hoverStatus->clear(); return; }
        const QString plane=face->plane==0 ? QObject::tr("flat") :
            face->plane==1 ? QObject::tr("front wall") :
            face->plane==2 ? QObject::tr("left wall") :
            face->plane==3 ? QObject::tr("right wall") : face->plane==4 ? QObject::tr("ramp") : QObject::tr("diagonal wall");
        hoverStatus->setText(QObject::tr("Tile (%1, %2) · Layer %3 · Height %4 · %5")
            .arg(face->x).arg(face->y).arg(face->layer+1).arg(face->elevation).arg(plane));
        if(auto* area=assets->getCurrentArea()) {
            const auto tile=area->getTileEntry(unsigned(face->layer),face->x,face->y);
            hoverStatus->setText(hoverStatus->text()+" · "+(tile.getSolid()?QObject::tr("Solid"):QObject::tr("Pass-through")));
        }
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
        view->finishStroke();
        if(enabled) { map->set3DPaintActive(true); map->set3DPaintActive(false); }
        pane->setVisible(enabled);
        brushDock->setVisible(enabled);
        if(enabled) window->resizeDocks({brushDock},{230},Qt::Vertical);
        map->setVisible(!enabled || sideView->isChecked());
        for(const char* name:{"elevSpinBox","planeComboBox"}) {
            if(auto* control=palette->findChild<QWidget*>(QString::fromLatin1(name))) {
                control->setEnabled(!enabled);
                control->setToolTip(enabled ? QObject::tr("In 3D, use Tile direction above the map. Height follows the pointed surface.") : QString());
            }
        }
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
