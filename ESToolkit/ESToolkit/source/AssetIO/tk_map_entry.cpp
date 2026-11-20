/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <QPainter>
#include <common/gyro_tile_geometry.h>
#include <ESToolkit/AssetIO/tk_map_entry.h>
#include <ESToolkit/AssetIO/tk_level.h>

bool TileEntry::renderIndeces	= false;
bool TileEntry::renderFlipRot	= false;
bool TileEntry::renderElev		= false;
bool TileEntry::renderColliders = false;
bool TileEntry::renderZTran		= false;
bool TileEntry::renderPlane    = false;

void TileEntry::renderCollider(QPainter *const painter) const {
    if(!getSolid() || getPlane()>=GY_RECT_PLANE_MAX)return;
    QPointF points[4];
    const unsigned order[4]={0,1,3,2};
    for(unsigned i=0;i<4;++i) {
        const auto& v=gyTilePlaneVerts[getPlane()][order[i]];
        points[i]=QPointF(v.x*32,(v.y-v.z)*32);
    }
    painter->save();painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0,0,255,80));painter->drawConvexPolygon(points,4);painter->restore();
}

//====NONSTATIC====

void TileEntry::renderDecoration(QPainter *const painter, const unsigned /*x*/, const unsigned y) const {
    /*static*/ const QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    /*static*/ const QRectF rect(-16.0f, -16.0f, 32.99f, 32.99f);

	painter->setPen(pen);
	//Tile attrib
    if(renderColliders) renderCollider(painter);

	//TileEntry attrib
#if 0
	if(renderZTran && getZTran()) {
		QBrush brush(Qt::DiagCrossPattern);
		brush.setColor(Qt::yellow);
		painter->fillRect(QRect(-16, -16, 32, 32), brush);
	}
#endif

    if(renderPlane) {
        GY_RECT_PLANE_TYPE plane = getPlane();

        if(plane != GY_RECT_PLANE_BOTTOM) {
            QString str;

            switch(plane) {
                case GY_RECT_PLANE_FRONT:
                    str = QString("F");
                    break;
                case GY_RECT_PLANE_LEFT:
                    str = QString("L");
                    break;
                case GY_RECT_PLANE_RIGHT:
                    str = QString("R");
                    break;
                case GY_RECT_PLANE_FRONT_SLOPE_45:
                    str = QString("F45");
                    break;
                case GY_RECT_PLANE_LEFT_SLOPE_45:
                    str = QString("L45");
                    break;
                case GY_RECT_PLANE_RIGHT_SLOPE_45:
                    str = QString("R45");
                    break;
                default:
                    Q_ASSERT(false);
            }
            painter->drawText(rect, Qt::AlignCenter, str);

            QColor color(Qt::white);
            color.setAlpha(100);
            painter->fillRect(rect, color);
        }
    }

	//MapEntry attrib
	if(renderIndeces) {
		unsigned index = getIndex();
		if(!index) return;
        QPen pen;
        pen.setColor(Qt::white);
        painter->setPen(pen);
        painter->drawText(rect, Qt::AlignCenter, QString::number(index));

	} else if(renderFlipRot) {
		GY_RECT_TEX_ORIENT orient = getOrient();
		QString str;

		switch(orient) {
			case GY_RECT_TEX_ORIENT_NORMAL: return;
			case GY_RECT_TEX_ORIENT_FLIP_HORIZ:
				str = QString("H");
				break;
			case GY_RECT_TEX_ORIENT_FLIP_VERT:
				str = QString("V");
				break;
			case GY_RECT_TEX_ORIENT_ROT_90:
				str = QString("90");
				break;
			case GY_RECT_TEX_ORIENT_ROT_180:
				str = QString("180");
				break;
			case GY_RECT_TEX_ORIENT_ROT_270:
				str = QString("270");
				break;
			case GY_RECT_TEX_ORIENT_FLIP_HORIZ_ROT_90:
				str = QString("H90");
				break;
			case GY_RECT_TEX_ORIENT_FLIP_HORIZ_ROT_270:
				str = QString("H270");
				break;
			default:
				Q_ASSERT(false);
		}
		painter->drawText(rect, Qt::AlignCenter, str);

	} else if(renderElev) {
		unsigned elev = getElev();
		if(!elev) return;
		unsigned yBase = y + elev;
		unsigned colorIndex = (yBase%16)+2;						//Use Qt's built in global colors, because I'm too lazy to make my own LUT.
		if(colorIndex > 3 && colorIndex < 7) colorIndex += 4;  //Bump it off the lame ass, repetitive color region.
		QColor color((Qt::GlobalColor)colorIndex);
		/* Quick hack to fix conflict between collision and elevation layer colours. @todo Make this more efficent. */
		if(color.blue() > 90) {
			color.setBlue(90);
		}
		color.setAlpha(100);
		painter->fillRect(rect, color);
		painter->drawText(rect, Qt::AlignCenter, QString::number(elev));

    }
}
