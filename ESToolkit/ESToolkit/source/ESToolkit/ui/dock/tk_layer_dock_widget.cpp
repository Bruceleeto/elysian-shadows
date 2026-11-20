#include <ESToolkit/AssetIO/tk_area.h>
#include <ESToolkit/ui/widget/tk_engine_gl_scene_widget.h>
#include <QPushButton>
#include <ESGamma/system/elysian_engine.hpp>
#include <QVBoxLayout>
#include <QPointer>
/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <QTreeWidgetItem>
#include <QCloseEvent>
#include <ESToolkit/ui/dock/tk_layer_dock_widget.h>
#include "ui_tk_layer_dock_widget.h"
#include <ESToolkit/core/tk_assets.h>

LayerDockWidget::LayerDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::LayerDockWidget)
{
    ui->setupUi(this);
    auto* fourth=new QTreeWidgetItem;
    fourth->setText(0,tr("Tile Layer 4")); fourth->setWhatsThis(0,"13");
    fourth->setFlags(Qt::ItemIsSelectable|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
    fourth->setCheckState(0,Qt::Checked);
    ui->m_layerTreeWidget->insertTopLevelItem(1,fourth);
    auto* add=new QPushButton(tr("Add fourth tile layer"),this);
    add->setFocusPolicy(Qt::NoFocus);
    widget()->layout()->addWidget(add);
    const auto refresh=[this,fourth,add] {
        auto* area=getAssets()->getCurrentArea();
        const bool hasFourth=area && area->getLayerCount()==4;
        if(fourth->isDisabled()==hasFourth) fourth->setDisabled(!hasFourth);
        const auto* engine=getAssets()->getEngine();
        add->setEnabled(area && !hasFourth && engine && engine->getEngineState()==elysian::System::ENGINE_ESTK_STOP);
        if(!hasFourth && getAssets()->getCurrentLayer()==Assets::LAYER_TILE4)
            getAssets()->setCurrentLayer(Assets::LAYER_TILE1);
    };
    connect(getAssets(),&Assets::areaChanged,this,[this,refresh](Area* area) {
        disconnect(_layerUndoConnection);
        if(area) _layerUndoConnection=connect(&area->getUndoStack(),&QUndoStack::indexChanged,this,refresh);
        refresh();
    });
    connect(getAssets(),&Assets::engineUpdated,this,refresh);
    connect(add,&QPushButton::clicked,this,[this,refresh] {
        auto* area=getAssets()->getCurrentArea();
        if(!area || area->getLayerCount()!=3 || !getAssets()->getEngine() ||
           getAssets()->getEngine()->getEngineState()!=elysian::System::ENGINE_ESTK_STOP) return;
        class AddLayer final : public QUndoCommand {
            QPointer<TkArea> _area;
            void apply(unsigned count) {
                if(!_area) return;
                EngineGLSceneWidget::makeCurrentContext();
                if(!_area->setLayerCount(count)) { setObsolete(true); return; }
                _area->updateScene();
                Q_EMIT _area->mapChanged(_area,{});
            }
        public:
            explicit AddLayer(TkArea* area):_area(area) { setText(QObject::tr("Add fourth tile layer")); }
            void redo() override { apply(4); }
            void undo() override { apply(3); }
        };
        area->getUndoStack().push(new AddLayer(area));
        refresh();
        getAssets()->setCurrentLayer(Assets::LAYER_TILE4);
    });
    refresh();
	//ui->m_layerTreeWidget->expandAll();
	ui->m_layerTreeWidget->setCurrentItem(ui->m_layerTreeWidget->topLevelItem(4));
    _Q_EMITActiveLayerChanged(4);

    connect(getAssets(), SIGNAL(layerChanged(LAYER_TYPE)), this, SLOT(setCurrentLayer(LAYER_TYPE)));
}

LayerDockWidget::~LayerDockWidget()
{
    delete ui;
}

void LayerDockWidget::ToggleDefaultItem(bool selected)
{
    if(selected) {
        ui->m_layerTreeWidget->clearSelection();
		//ui->m_layerTreeWidget->setCurrentItem(ui->m_layerTreeWidget->itemBelow(ui->m_layerTreeWidget->topLevelItem(0)));
		ui->m_layerTreeWidget->setCurrentItem(ui->m_layerTreeWidget->topLevelItem(4));
        _Q_EMITActiveLayerChanged(4);
    }

    else {
        ui->m_layerTreeWidget->clearSelection();
    }
}

void LayerDockWidget::setBlockedPaintAction(QAction* action) {
    ui->blockedPaintButton->setDefaultAction(action);
}

void LayerDockWidget::showWalkHeight() {
    auto* decorations = ui->m_layerTreeWidget->topLevelItem(5);
    for(int i=0; i<decorations->childCount(); ++i) {
        auto* item = decorations->child(i);
        if(item->whatsThis(0).toInt()==12) item->setCheckState(0,Qt::Checked);
    }
}

void LayerDockWidget::on_m_layerTreeWidget_itemChanged(QTreeWidgetItem *item, int /*column*/)
{
#if 0 //Ain't got no parent layers anymore!
    if(item->whatsThis(0).toInt() == 0 || item->whatsThis(0).toInt() == 3) {

        if(item->checkState(0) == Qt::Unchecked) {
            QTreeWidgetItem *subLayer = item->child(0);
            subLayer->setCheckState(0, Qt::Unchecked);
            subLayer = item->child(1);
            subLayer->setCheckState(0, Qt::Unchecked);
        }

        else {
            QTreeWidgetItem *subLayer = item->child(0);
            subLayer->setCheckState(0, Qt::Checked);
            subLayer = item->child(1);
            subLayer->setCheckState(0, Qt::Checked);
        }
    }
#endif

    const int id=item->whatsThis(0).toInt();
    const bool checked=item->checkState(0)==Qt::Checked;
    const auto isDecoration=[](int value) { return value==7 || value==8 || value==10 || value==11 || value==12; };
    if(checked && isDecoration(id)) {
        auto* group=item->parent();
        for(int i=0;group && i<group->childCount();++i) {
            auto* other=group->child(i);
            if(other!=item && isDecoration(other->whatsThis(0).toInt())) other->setCheckState(0,Qt::Unchecked);
        }
    }
    // Emit the newly enabled mode last, after disabled siblings have finished.
    Q_EMIT LayerVisibilityChanged(id,checked);
	//if(item->whatsThis(0).toInt() == 7) ui->m_layerTreeWidget->setCurrentItem(_lastItem);
}

void LayerDockWidget::on_m_layerTreeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem */*previous*/)
{
	if(!current) return;
#if 0 //Don't think we need any of this shit
	if(previous) previous->setSelected(false);
    if(current->whatsThis(0).toInt() == 0 || current->whatsThis(0).toInt() == 3) {
        ui->m_layerTreeWidget->setCurrentItem(current->child(0));
	} else if (current->whatsThis(0).toInt() == 7) {
		//ui->m_layerTreeWidget->clearSelection();
		//current->setSelected(false);
		//ui->m_layerTreeWidget->setCurrentItem(ui->m_layerTreeWidget->topLevelItem(0)->child(1));
	}
#endif
    _Q_EMITActiveLayerChanged(current->whatsThis(0).toInt());

}

void LayerDockWidget::closeEvent(QCloseEvent *event)
{
    Q_EMIT UncheckMe(2);
    event->accept();
}

int LayerDockWidget::GetCurrentLayerIndex()
{
    return ui->m_layerTreeWidget->currentItem()->whatsThis(0).toInt();
}

void LayerDockWidget::setCurrentLayer(const LAYER_TYPE layer) {
    if(_causedChange) return;

	int index;
	switch(layer) {
	case Assets::LAYER_TILE1:
		index = 4;
		break;
	case Assets::LAYER_TILE2:
		index = 3;
		//ui->m_layerTreeWidget->clearSelection();
		//ui->m_layerTreeWidget->topLevelItem(0)->child(1)->setSelected(true);
		break;
	case Assets::LAYER_TILE3:
		index = 2;
		break;
    case Assets::LAYER_TILE4:
        index = 1;
        break;
    case Assets::LAYER_ENTITY:
	case Assets::LAYER_LIGHT:
		index = 0;
		break;
	default:
		index = 0;
//		Q_ASSERT(false);
	}
    ui->m_layerTreeWidget->setCurrentItem(nullptr);
	ui->m_layerTreeWidget->setCurrentItem(ui->m_layerTreeWidget->topLevelItem(index));
/*
	for(int i = 0; i < ui->m_layerTreeWidget->topLevelItemCount(); ++i) {
		QTreeWidgetItem *parent = ui->m_layerTreeWidget->topLevelItem(i);
		if(parent->whatsThis(0).toInt() == index) {
			ui->m_layerTreeWidget->clearSelection();
			ui->m_layerTreeWidget->setCurrentItem(parent);
			return;
		}
		for(int j = 0; j < parent->childCount(); ++j) {
			QTreeWidgetItem *child = parent->child(j);
			if(child->whatsThis(0).toInt() == index) {
				ui->m_layerTreeWidget->clearSelection();
				ui->m_layerTreeWidget->setCurrentItem(child);
				return;
			}
		}
	}*/

}

void LayerDockWidget::setTopLevelItemCheckState(const unsigned index, const bool value) {
	ui->m_layerTreeWidget->topLevelItem(index>=1 ? index+1 : index)->setCheckState(0, (value)? Qt::Checked : Qt::Unchecked);
}
