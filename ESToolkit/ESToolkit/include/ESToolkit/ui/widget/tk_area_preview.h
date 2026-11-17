/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#ifndef TK_AREA_PREVIEW_H
#define TK_AREA_PREVIEW_H

#include <QImage>
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
#include <vector>

struct AreaPreviewVertex {
    QVector3D position;
    QVector2D uv;
    QVector3D normal;
};

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
    void draw(const QSize& pixels, const QMatrix4x4& matrix);
    QString error() const { return _error; }
private:
    std::unique_ptr<QOpenGLShaderProgram> _program;
    std::unique_ptr<QOpenGLTexture> _texture;
    QOpenGLVertexArrayObject _vao;
    QOpenGLBuffer _vbo{QOpenGLBuffer::VertexBuffer};
    int _count = 0;
    qint64 _atlasKey = 0;
    QString _error;
};

class AreaPreview : public QOpenGLWidget {
public:
    explicit AreaPreview(QWidget* parent = nullptr);
    ~AreaPreview() override;
    void setScene(std::vector<AreaPreviewVertex> mesh, QImage atlas, bool frameArea);
    void frameArea();
    void focusWorldPoint(const QVector3D& point, float radius = 192);
    const std::vector<AreaPreviewVertex>& mesh() const { return _mesh; }
protected:
    bool event(QEvent* event) override;
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
private:
    void cleanup();
    AreaPreviewRenderer _renderer;
    AreaPreviewCamera _camera;
    std::vector<AreaPreviewVertex> _mesh;
    QImage _atlas;
    bool _dirty = true;
    bool _ready = false;
    QPointF _lastMouse;
};
#endif
