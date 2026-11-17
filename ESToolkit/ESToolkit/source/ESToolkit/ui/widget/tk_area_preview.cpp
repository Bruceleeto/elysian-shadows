/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#include <ESToolkit/ui/widget/tk_area_preview.h>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QPainter>
#include <QPointer>
#include <QWheelEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace {
// Existing editor actions upload tile buffers between paint events. Preserve the
// context they were using when Qt paints, resizes or reparents this second view.
class RestoreContext {
public:
    RestoreContext(): _context(QOpenGLContext::currentContext()),
        _surface(_context ? _context->surface() : nullptr) {}
    ~RestoreContext() {
        if(QOpenGLContext::currentContext()==_context) return;
        if(_context && _surface && _context->isValid()) _context->makeCurrent(_surface);
        else if(auto* current=QOpenGLContext::currentContext()) current->doneCurrent();
    }
private:
    QPointer<QOpenGLContext> _context;
    QSurface* _surface;
};
QVector3D toView(const QVector3D& world) { return {world.x(), -world.y(), world.z()}; }
QVector3D eyeOffset(float yaw, float pitch) {
    const float y = qDegreesToRadians(yaw), p = qDegreesToRadians(pitch);
    return {std::cos(p)*std::cos(y), std::cos(p)*std::sin(y), std::sin(p)};
}
}

void AreaPreviewCamera::frame(const QVector3D& minimum, const QVector3D& maximum, float aspect) {
    target = toView((minimum+maximum)*0.5f);
    const float radius = std::max(32.0f, (maximum-minimum).length()*0.5f);
    const float halfFov = std::atan(std::tan(qDegreesToRadians(22.5f))*std::min(1.0f,aspect));
    distance = std::clamp(radius/std::sin(halfFov)*1.15f, 16.0f, 100000.0f);
}
void AreaPreviewCamera::orbit(const QPointF& delta) {
    yaw = std::remainder(yaw-static_cast<float>(delta.x())*0.4f,360.0f);
    pitch = std::clamp(pitch+static_cast<float>(delta.y())*0.4f,-85.0f,85.0f);
}
void AreaPreviewCamera::zoom(float steps) {
    distance = std::clamp(distance*std::pow(0.85f,steps),16.0f,100000.0f);
}
void AreaPreviewCamera::pan(const QPointF& delta, float viewportHeight) {
    const auto forward = -eyeOffset(yaw,pitch);
    const auto right = QVector3D::crossProduct(forward,{0,0,1}).normalized();
    const auto up = QVector3D::crossProduct(right,forward).normalized();
    const float units = 2*distance*std::tan(qDegreesToRadians(22.5f))/std::max(1.0f,viewportHeight);
    target += (-right*static_cast<float>(delta.x())+up*static_cast<float>(delta.y()))*units;
}
QMatrix4x4 AreaPreviewCamera::matrix(float aspect) const {
    QMatrix4x4 projection, view;
    projection.perspective(45,std::max(0.01f,aspect),0.5f,std::max(20000.0f,distance*4));
    view.lookAt(target+eyeOffset(yaw,pitch)*distance,target,{0,0,1});
    return projection*view;
}

bool AreaPreviewRenderer::initialize() {
    if(!initializeOpenGLFunctions()) { _error = "OpenGL 3.3 is unavailable."; return false; }
    _error.clear();
    _program = std::make_unique<QOpenGLShaderProgram>();
    const char* vertex = R"(#version 330 core
layout(location=0) in vec3 position;
layout(location=1) in vec2 texCoord;
layout(location=2) in vec3 normal;
uniform mat4 matrix;
out vec2 uv;
out vec3 n;
void main() {
    gl_Position = matrix*vec4(position.x,-position.y,position.z,1);
    uv=texCoord; n=vec3(normal.x,-normal.y,normal.z);
})";
    const char* fragment = R"(#version 330 core
in vec2 uv;
in vec3 n;
uniform sampler2D atlas;
out vec4 color;
void main() {
    vec4 tex=texture(atlas,uv);
    if(tex.a<0.1) discard;
    float shade=0.65+0.35*abs(dot(normalize(n),normalize(vec3(0.3,-0.4,1))));
    color=vec4(tex.rgb*shade,1);
})";
    if(!_program->addShaderFromSourceCode(QOpenGLShader::Vertex,vertex) ||
       !_program->addShaderFromSourceCode(QOpenGLShader::Fragment,fragment) || !_program->link()) {
        _error=_program->log(); return false;
    }
    if(!_vao.create() || !_vbo.create()) { _error="Could not allocate preview geometry."; return false; }
    QOpenGLVertexArrayObject::Binder vao(&_vao);
    _vbo.bind();
    _program->bind();
    _program->enableAttributeArray(0); _program->enableAttributeArray(1); _program->enableAttributeArray(2);
    _program->setAttributeBuffer(0,GL_FLOAT,offsetof(AreaPreviewVertex,position),3,sizeof(AreaPreviewVertex));
    _program->setAttributeBuffer(1,GL_FLOAT,offsetof(AreaPreviewVertex,uv),2,sizeof(AreaPreviewVertex));
    _program->setAttributeBuffer(2,GL_FLOAT,offsetof(AreaPreviewVertex,normal),3,sizeof(AreaPreviewVertex));
    _vbo.release(); _program->release();
    return true;
}
void AreaPreviewRenderer::release() {
    _texture.reset(); _program.reset(); _vbo.destroy(); _vao.destroy(); _count=0; _atlasKey=0;
}
void AreaPreviewRenderer::upload(const std::vector<AreaPreviewVertex>& mesh, const QImage& atlas) {
    const auto bytes=mesh.size()*sizeof(AreaPreviewVertex);
    if(bytes>static_cast<size_t>(std::numeric_limits<int>::max())) { _count=0; return; }
    _vbo.bind(); _vbo.allocate(mesh.data(),static_cast<int>(bytes)); _vbo.release();
    _count=static_cast<int>(mesh.size());
    if(!_texture || _atlasKey!=atlas.cacheKey()) {
        // Top-row-first upload, matching libGyro. UVs already include orientation.
        _texture=std::make_unique<QOpenGLTexture>(atlas.convertToFormat(QImage::Format_RGBA8888),QOpenGLTexture::DontGenerateMipMaps);
        _texture->setMinMagFilters(QOpenGLTexture::Nearest,QOpenGLTexture::Nearest);
        _texture->setWrapMode(QOpenGLTexture::ClampToEdge);
        _atlasKey=atlas.cacheKey();
    }
}
void AreaPreviewRenderer::draw(const QSize& pixels, const QMatrix4x4& matrix) {
    glViewport(0,0,pixels.width(),pixels.height());
    glDisable(GL_SCISSOR_TEST); glDisable(GL_STENCIL_TEST); glDisable(GL_BLEND); glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL);
    glClearColor(0.08f,0.09f,0.11f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    if(!_program || !_texture || !_count) return;
    _program->bind(); _program->setUniformValue("matrix",matrix); _program->setUniformValue("atlas",0);
    _texture->bind(0);
    QOpenGLVertexArrayObject::Binder vao(&_vao);
    glDrawArrays(GL_TRIANGLES,0,_count);
    _texture->release(0); _program->release();
}

AreaPreview::AreaPreview(QWidget* parent):QOpenGLWidget(parent) {
    setMinimumSize(240,200);
    setFocusPolicy(Qt::NoFocus); // Mouse navigation must not steal game keyboard input.
    setToolTip(tr("Left drag: orbit · Middle/right drag: pan · Wheel: zoom"));
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
}
AreaPreview::~AreaPreview() {
    // The base widget destroys its context after our renderer members. Disconnect
    // first so aboutToBeDestroyed cannot call cleanup on destroyed members.
    if(context()) disconnect(context(),nullptr,this,nullptr);
    cleanup();
}
bool AreaPreview::event(QEvent* event) {
    const RestoreContext restore;
    return QOpenGLWidget::event(event);
}
void AreaPreview::cleanup() {
    const RestoreContext restore;
    if(context()) { makeCurrent(); _renderer.release(); doneCurrent(); }
    _ready=false; _dirty=true;
}
void AreaPreview::initializeGL() {
    _ready=_renderer.initialize(); _dirty=true;
    connect(context(),&QOpenGLContext::aboutToBeDestroyed,this,[this]{cleanup();});
}
void AreaPreview::setScene(std::vector<AreaPreviewVertex> mesh, QImage atlas, bool resetCamera) {
    _mesh=std::move(mesh); _atlas=std::move(atlas); _dirty=true;
    if(resetCamera) frameArea();
    update();
}
void AreaPreview::frameArea() {
    if(_mesh.empty()) return;
    auto lo=_mesh.front().position, hi=lo;
    for(const auto& vertex:_mesh) for(int axis=0;axis<3;++axis) {
        lo[axis]=std::min(lo[axis],vertex.position[axis]); hi[axis]=std::max(hi[axis],vertex.position[axis]);
    }
    _camera.frame(lo,hi,static_cast<float>(width())/std::max(1,height())); update();
}
void AreaPreview::focusWorldPoint(const QVector3D& point, float radius) {
    const QVector3D extent(radius,radius,radius*0.5f);
    _camera.frame(point-extent,point+extent,static_cast<float>(width())/std::max(1,height())); update();
}
void AreaPreview::paintGL() {
    if(!_ready) {
        QPainter painter(this); painter.fillRect(rect(),palette().window());
        painter.drawText(rect().adjusted(12,12,-12,-12),Qt::AlignCenter|Qt::TextWordWrap,
                         tr("3D preview unavailable: %1").arg(_renderer.error())); return;
    }
    if(_dirty) {
        if(_atlas.isNull()) { _atlas=QImage(1,1,QImage::Format_RGBA8888); _atlas.fill(Qt::white); }
        _renderer.upload(_mesh,_atlas); _dirty=false;
    }
    const qreal dpr=devicePixelRatioF();
    _renderer.draw(QSize(qRound(width()*dpr),qRound(height()*dpr)),
        _camera.matrix(static_cast<float>(width())/std::max(1,height())));
}
void AreaPreview::mousePressEvent(QMouseEvent* event) {
    _lastMouse=event->position(); event->accept();
}
void AreaPreview::mouseMoveEvent(QMouseEvent* event) {
    const QPointF delta=event->position()-_lastMouse; _lastMouse=event->position();
    if(event->buttons() & (Qt::MiddleButton|Qt::RightButton)) _camera.pan(delta,height());
    else if(event->buttons() & Qt::LeftButton) _camera.orbit(delta);
    update(); event->accept();
}
void AreaPreview::mouseReleaseEvent(QMouseEvent* event) { event->accept(); }
void AreaPreview::wheelEvent(QWheelEvent* event) {
    const float steps=event->pixelDelta().isNull()? event->angleDelta().y()/120.0f : event->pixelDelta().y()/40.0f;
    _camera.zoom(steps); update(); event->accept();
}
