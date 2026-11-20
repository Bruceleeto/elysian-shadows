/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#ifndef TK_AREA_PREVIEW_H
#define TK_AREA_PREVIEW_H

#include <ESToolkit/ui/widget/tk_area_face.h>
#include <ESToolkit/core/tk_tile_placement.h>
#include <QImage>
#include <QSet>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QVector2D>
#include <QVector3D>
#include <memory>
#include <optional>
#include <array>
#include <functional>
#include <vector>

struct AreaPreviewVertex {
    QVector3D position;
    QVector2D uv;
    QVector3D normal;
};

// Returns the nearest visible six-vertex face; transparent atlas pixels are not hits.
int pickAreaPreviewFace(const std::vector<AreaPreviewVertex>& mesh, const QImage& atlas,
                        const QMatrix4x4& matrix, const QPointF& mouse, const QSize& viewport);

struct AreaPaintTarget {
    AreaPreviewFace face;
    int existingIndex=-1;
    QString error;
    bool replacesTile=false;
    std::array<AreaPreviewVertex,6> vertices;
};
using AreaPlacementCheck=std::function<bool(AreaPreviewFace&)>;
std::optional<AreaPaintTarget> pickAreaPaintTarget(const std::vector<AreaPreviewVertex>& mesh,
    const std::vector<AreaPreviewFace>& faces,const QImage& atlas,const QMatrix4x4& matrix,
    QPointF mouse,QSize viewport,bool extend,const AreaPlacementCheck& canPlace,int preferredPlane=-1,
    const AreaPaintTarget* anchor=nullptr);

// Intersects a chosen surface's tile grid, independent of nearby surfaces.
// Invalid placements retain their geometry and explain why they cannot be painted.
std::optional<AreaPaintTarget> pickAreaSurfaceTarget(const AreaPaintTarget& surface,
    const std::vector<AreaPreviewVertex>& mesh,const std::vector<AreaPreviewFace>& faces,
    const QImage& atlas,const QMatrix4x4& matrix,QPointF mouse,QSize viewport,
    const AreaPlacementCheck& canPlace,bool erase=false);

// Builds an upright face along the nearest edge of a flat surface. Existing
// upright faces can still be painted or extended with the Wall tool.
std::optional<AreaPaintTarget> pickAreaWallTarget(const std::vector<AreaPreviewVertex>& mesh,
    const std::vector<AreaPreviewFace>& faces,const QImage& atlas,const QMatrix4x4& matrix,
    QPointF mouse,QSize viewport,const AreaPlacementCheck& canPlace);

// Input positions remain in engine world coordinates (Y down, Z up).
struct AreaPreviewCamera {
    QVector3D target;
    float distance = 256;
    float yaw = -65;
    float pitch = 45;
    void frame(const QVector3D& minimum, const QVector3D& maximum, float aspect);
    void orbit(const QPointF& delta);
    void zoom(float steps);
    void pan(const QPointF& delta, float viewportHeight);
    QMatrix4x4 matrix(float aspect) const;
};

// Only uses its own GL objects in the current preview context. No libGyro
// globals, cameras, framebuffers, simulation updates or engine render calls.
class AreaPreviewRenderer : protected QOpenGLFunctions_3_3_Core {
public:
    bool initialize();
    void release();
    void upload(const std::vector<AreaPreviewVertex>& mesh, const QImage& atlas);
    void draw(const QSize& pixels, const QMatrix4x4& matrix, int highlighted=-1, bool standing=false,
              const std::vector<AreaPreviewVertex>& ghost = {}, bool invalidGhost=false, bool replacesTile=false);
    QString error() const { return _error; }
private:
    std::unique_ptr<QOpenGLShaderProgram> _program;
    std::unique_ptr<QOpenGLTexture> _texture;
    QOpenGLVertexArrayObject _vao;
    QOpenGLBuffer _vbo{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer _edges{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject _ghostVao;
    QOpenGLBuffer _ghostVbo{QOpenGLBuffer::VertexBuffer};
    int _count = 0;
    qint64 _atlasKey = 0;
    QString _error;
};

class AreaPreview : public QOpenGLWidget {
public:
    enum class Tool { Select, Paint, Wall, Erase };
    explicit AreaPreview(QWidget* parent = nullptr);
    void setTool(Tool tool);
    Tool tool() const { return _tool; }
    std::function<void(Tool)> onToolChanged;
    void setPlacementPlane(int plane);
    void setLayerCount(int count) { _layerCount=count; }
    void setPaintLayer(int layer);
    int paintLayer() const { return _paintLayer; }
    std::function<std::array<TilePlacementSlot,4>(int,int)> placementSlots;
    int placementPlane() const { return _placementPlane; }
    std::function<void(int)> onPlacementPlaneChanged;
    void finishStroke();
    void pickSurface();
    void clearSurface();
    bool pickingSurface() const { return _pickingSurface; }
    const AreaPreviewFace* lockedSurface() const { return _surface ? &_surface->face : nullptr; }
    std::function<void()> onSurfaceChanged;
    std::function<void(const QString&)> onPlacementHint;
    std::function<bool(bool)> onStrokeBegin;
    std::function<bool(const AreaPreviewFace&)> onPaint;
    std::function<bool(const AreaPreviewFace&)> onSampleTile;
    std::function<QString(const AreaPreviewFace&)> onPlacementError;
    std::function<void()> onStrokeEnd;
    AreaPlacementCheck canPlace;
    std::function<std::vector<AreaPreviewVertex>(const AreaPreviewFace&)> onBrushPreview;
    std::function<std::vector<AreaPreviewVertex>(const AreaPreviewFace&)> onFacePreview;
    ~AreaPreview() override;
    void setScene(std::vector<AreaPreviewVertex> mesh, QImage atlas, bool frameArea,
                  std::vector<AreaPreviewFace> faces = {});
    void frameArea();
    void focusWorldPoint(const QVector3D& point, float radius = 192);
    const std::vector<AreaPreviewVertex>& mesh() const { return _mesh; }
    const AreaPreviewFace* hoveredFace() const;
    std::function<void(const AreaPreviewFace*)> onHover;
    void clearHover();
protected:
    bool event(QEvent* event) override;
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeGL(int width, int height) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    void cleanup();
    void pickAt(const QPointF& point);
    void paintAt(const QPointF& point);
    std::optional<AreaPaintTarget> targetAt(const QPointF& point,bool stroke) const;
    void chooseSurface(const QPointF& point);
    void applyPlacementPlane(AreaPaintTarget& target) const;
    void resolveDestination(AreaPaintTarget& target,bool stroke) const;
    int _layerCount=3;
    int _paintLayer=-1;
    int _placementPlane=-1; // Match the pointed surface unless explicitly chosen.
    std::optional<AreaPaintTarget> _surface, _strokeSurface;
    bool _pickingSurface=false;
    Tool _tool=Tool::Select;
    Qt::MouseButton _strokeButton=Qt::NoButton;
    enum class Navigation { None, Orbit, Pan };
    Navigation _navigation=Navigation::None;
    std::vector<AreaPreviewVertex> _strokeMesh;
    std::vector<AreaPreviewFace> _strokeFaces;
    QImage _strokeAtlas;
    QMatrix4x4 _strokeMatrix;
    QSet<quint64> _paintedFaces;
    bool _erasing=false;
    std::optional<AreaPaintTarget> _target, _paintAnchor;
    std::vector<AreaPreviewVertex> _ghost;
    AreaPreviewRenderer _renderer;
    AreaPreviewCamera _camera;
    std::vector<AreaPreviewVertex> _mesh;
    std::vector<AreaPreviewFace> _faces;
    int _hovered = -1;
    QImage _atlas;
    bool _dirty = true;
    bool _ready = false;
    QPointF _lastMouse;
};
#endif
