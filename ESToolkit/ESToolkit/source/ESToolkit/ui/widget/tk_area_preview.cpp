/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#include <ESToolkit/ui/widget/tk_area_preview.h>
#include <QMouseEvent>
#include <QKeyEvent>
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

int pickAreaPreviewFace(const std::vector<AreaPreviewVertex>& mesh, const QImage& atlas,
                        const QMatrix4x4& matrix, const QPointF& mouse, const QSize& viewport) {
    if(viewport.width()<=0 || viewport.height()<=0 || mouse.x()<0 || mouse.y()<0 ||
       mouse.x()>=viewport.width() || mouse.y()>=viewport.height()) return -1;
    bool invertible=false;
    const auto inverse=matrix.inverted(&invertible);
    if(!invertible) return -1;
    const float x=2*float(mouse.x())/viewport.width()-1, y=1-2*float(mouse.y())/viewport.height();
    const auto origin=toView((inverse*QVector4D(x,y,-1,1)).toVector3DAffine());
    const auto far=toView((inverse*QVector4D(x,y,1,1)).toVector3DAffine());
    const auto direction=(far-origin).normalized();
    float nearest=(far-origin).length();
    int hit=-1;
    for(size_t i=0;i+2<mesh.size();i+=3) {
        const auto& a=mesh[i]; const auto& b=mesh[i+1]; const auto& c=mesh[i+2];
        const auto edge1=b.position-a.position, edge2=c.position-a.position;
        const auto cross=QVector3D::crossProduct(direction,edge2);
        const float determinant=QVector3D::dotProduct(edge1,cross);
        if(std::abs(determinant)<1e-7f) continue;
        const auto offset=origin-a.position;
        const float u=QVector3D::dotProduct(offset,cross)/determinant;
        if(u<0 || u>1) continue;
        const auto q=QVector3D::crossProduct(offset,edge1);
        const float v=QVector3D::dotProduct(direction,q)/determinant;
        if(v<0 || u+v>1) continue;
        const float distance=QVector3D::dotProduct(edge2,q)/determinant;
        if(distance<0 || distance>nearest+0.0001f) continue;
        if(!atlas.isNull()) {
            const auto uv=a.uv*(1-u-v)+b.uv*u+c.uv*v;
            const int tx=std::clamp(int(std::floor(uv.x()*atlas.width())),0,atlas.width()-1);
            const int ty=std::clamp(int(std::floor(uv.y()*atlas.height())),0,atlas.height()-1);
            if(atlas.pixelColor(tx,ty).alphaF()<0.1f) continue;
        }
        nearest=distance; hit=int(i/6);
    }
    return hit;
}

namespace {
struct SurfaceGridHit { QVector3D shift; float distance; int column,row; };
// Both automatic extension and chosen surfaces use this one grid intersection.
std::optional<SurfaceGridHit> intersectSurfaceGrid(const AreaPreviewVertex* vertices,
    const QVector3D& origin,const QVector3D& ray,float farDistance) {
    const auto a=vertices[0].position,u=vertices[1].position-a,v=vertices[2].position-a;
    const auto normal=QVector3D::crossProduct(u,v);
    const float denominator=QVector3D::dotProduct(normal,ray);
    if(std::abs(denominator)<1e-6f*normal.length()) return {};
    const float distance=QVector3D::dotProduct(normal,a-origin)/denominator;
    if(distance<0 || distance>farDistance) return {};
    const auto offset=origin+ray*distance-a;
    const float uu=QVector3D::dotProduct(u,u),vv=QVector3D::dotProduct(v,v),uv=QVector3D::dotProduct(u,v);
    const float determinant=uu*vv-uv*uv;
    if(determinant<1e-7f) return {};
    const float du=QVector3D::dotProduct(offset,u),dv=QVector3D::dotProduct(offset,v);
    const float column=std::floor((du*vv-dv*uv)/determinant),row=std::floor((dv*uu-du*uv)/determinant);
    if(std::abs(column)>65535 || std::abs(row)>65535) return {};
    return SurfaceGridHit{u*column+v*row,distance,int(column),int(row)};
}
std::optional<AreaPaintTarget> translateSurfaceTile(AreaPaintTarget target,const QVector3D& shift) {
    // Engine mapping: world X=32*x, world Y=32*(y+elev), world Z=32*elev.
    const QVector3D step(shift.x()/32,(shift.y()-shift.z())/32,shift.z()/32);
    const int dx=qRound(step.x()),dy=qRound(step.y()),dz=qRound(step.z());
    if((step-QVector3D(dx,dy,dz)).length()>.01f) return {};
    target.existingIndex=-1; target.error.clear();
    target.face.x+=dx; target.face.y+=dy; target.face.elevation+=dz;
    for(auto& vertex:target.vertices) vertex.position+=shift;
    if(target.face.x<0 || target.face.y<0 || target.face.elevation<0 || target.face.elevation>31)
        target.error=QStringLiteral("Outside the map or supported height range.");
    return target;
}
}

std::optional<AreaPaintTarget> pickAreaPaintTarget(const std::vector<AreaPreviewVertex>& mesh,
    const std::vector<AreaPreviewFace>& faces,const QImage& atlas,const QMatrix4x4& matrix,
    QPointF mouse,QSize viewport,bool extend,const AreaPlacementCheck& canPlace,int preferredPlane,
    const AreaPaintTarget* anchor) {
    if(viewport.width()<=0 || viewport.height()<=0 || mouse.x()<0 || mouse.y()<0 ||
       mouse.x()>=viewport.width() || mouse.y()>=viewport.height()) return {};
    bool invertible=false; const auto inverse=matrix.inverted(&invertible);
    if(!invertible) return {};
    const float x=2*float(mouse.x())/viewport.width()-1,y=1-2*float(mouse.y())/viewport.height();
    const auto origin=toView((inverse*QVector4D(x,y,-1,1)).toVector3DAffine());
    const auto far=toView((inverse*QVector4D(x,y,1,1)).toVector3DAffine());
    const auto ray=(far-origin).normalized();
    const int hit=pickAreaPreviewFace(mesh,atlas,matrix,mouse,viewport);
    std::optional<AreaPaintTarget> result;
    float nearest=(far-origin).length();
    const auto makeTarget=[&](size_t i) {
        AreaPaintTarget target; target.face=faces[i]; target.existingIndex=int(i);
        std::copy_n(mesh.begin()+i*6,6,target.vertices.begin()); return target;
    };
    if(hit>=0 && size_t(hit)<faces.size()) {
        result=makeTarget(size_t(hit));
        const auto& a=mesh[size_t(hit)*6];
        const auto normal=QVector3D::crossProduct(mesh[size_t(hit)*6+1].position-a.position,
                                                mesh[size_t(hit)*6+2].position-a.position);
        const float denominator=QVector3D::dotProduct(normal,ray);
        if(std::abs(denominator)>1e-7f) nearest=QVector3D::dotProduct(normal,a.position-origin)/denominator;
    }
    if(!extend || !canPlace) return result;
    // Continue from the real surface the brush just left. Several different
    // heights can project onto a gap; choosing a fresh nearest extension at
    // every pixel makes the brush jump between them. Limit continuity to one
    // neighbouring square, and always respect actual foreground geometry.
    if(anchor && (preferredPlane<0 || anchor->face.plane==preferredPlane)) {
        for(size_t i=0;i<faces.size() && i*6+5<mesh.size();++i) {
            const auto& f=faces[i]; const auto& a=anchor->face;
            if(f.x!=a.x || f.y!=a.y || f.layer!=a.layer || f.elevation!=a.elevation || f.plane!=a.plane) continue;
            const auto grid=intersectSurfaceGrid(&mesh[i*6],origin,ray,nearest-.01f);
            if(!grid || std::abs(grid->column)+std::abs(grid->row)!=1) break;
            auto candidate=translateSurfaceTile(makeTarget(i),grid->shift);
            if(candidate && candidate->error.isEmpty() && canPlace(candidate->face)) {
                const float depth=(candidate->face.layer-f.layer)*.001f;
                for(auto& vertex:candidate->vertices) vertex.position+=QVector3D(0,depth,depth);
                return candidate;
            }
            break;
        }
    }
    // Actual matching faces win when there is no adjacent reference to follow.
    // Explicit facing still guides wall/flat extension through a visible gap.
    if(result && (preferredPlane<0 || result->face.plane==preferredPlane)) return result;
    // Extend a real face by exactly one of its own tile edges. The mesh comes
    // from vboEntryFromTileEntry, so slopes and standing faces retain their true
    // orientation. No guessed horizontal plane or user-entered height is used.
    for(size_t i=0;i<faces.size() && i*6+5<mesh.size();++i) {
        if(preferredPlane>=0 && faces[i].plane!=preferredPlane) continue;
        const auto hit=intersectSurfaceGrid(&mesh[i*6],origin,ray,nearest-.001f);
        if(!hit || std::abs(hit->column)+std::abs(hit->row)!=1) continue;
        auto candidate=translateSurfaceTile(makeTarget(i),hit->shift);
        if(!candidate || !candidate->error.isEmpty() || !canPlace(candidate->face)) continue;
        const float layerOffset=(candidate->face.layer-faces[i].layer)*.001f;
        for(auto& vertex:candidate->vertices) vertex.position+=QVector3D(0,layerOffset,layerOffset);
        nearest=hit->distance; result=std::move(candidate);
    }
    return result;
}

std::optional<AreaPaintTarget> pickAreaSurfaceTarget(const AreaPaintTarget& surface,
    const std::vector<AreaPreviewVertex>& mesh,const std::vector<AreaPreviewFace>& faces,
    const QImage& atlas,const QMatrix4x4& matrix,QPointF mouse,QSize viewport,
    const AreaPlacementCheck& canPlace,bool erase) {
    if(viewport.isEmpty() || mouse.x()<0 || mouse.y()<0 || mouse.x()>=viewport.width() || mouse.y()>=viewport.height()) return {};
    bool invertible=false; const auto inverse=matrix.inverted(&invertible);
    if(!invertible) return {};
    const float x=2*float(mouse.x())/viewport.width()-1,y=1-2*float(mouse.y())/viewport.height();
    const auto origin=toView((inverse*QVector4D(x,y,-1,1)).toVector3DAffine());
    const auto far=toView((inverse*QVector4D(x,y,1,1)).toVector3DAffine());
    const auto ray=(far-origin).normalized();
    const auto gridHit=intersectSurfaceGrid(surface.vertices.data(),origin,ray,(far-origin).length());
    if(!gridHit) return {};
    auto projected=translateSurfaceTile(surface,gridHit->shift);
    if(!projected || !projected->error.isEmpty()) return projected;
    auto target=*projected;
    const float distance=gridHit->distance;
    // Repaint the existing face on this plane, never an unrelated floor behind it.
    for(size_t i=0;i<faces.size() && i*6+5<mesh.size();++i) {
        const auto& face=faces[i];
        if(face.x==target.face.x && face.y==target.face.y && face.elevation==target.face.elevation && face.plane==target.face.plane) {
            target.face=face; target.existingIndex=int(i);
            std::copy_n(mesh.begin()+i*6,6,target.vertices.begin());
        }
    }
    if(erase && target.existingIndex<0) return {};
    if(target.existingIndex<0) {
        const int oldLayer=target.face.layer;
        if(!canPlace || !canPlace(target.face)) target.error=QStringLiteral("Outside the editable map. Select a tile layer and a square inside the map.");
        const float depth=(target.face.layer-oldLayer)*.001f;
        for(auto& vertex:target.vertices) vertex.position+=QVector3D(0,depth,depth);
    }
    // A locked plane is not permission to paint through foreground geometry.
    const int hit=pickAreaPreviewFace(mesh,atlas,matrix,mouse,viewport);
    if(hit>=0 && hit!=target.existingIndex) {
        const auto p=mesh[size_t(hit)*6].position;
        const auto n=QVector3D::crossProduct(mesh[size_t(hit)*6+1].position-p,mesh[size_t(hit)*6+2].position-p);
        const float d=QVector3D::dotProduct(n,ray);
        if(std::abs(d)>1e-7f && QVector3D::dotProduct(n,p-origin)/d<distance-.01f)
            target.error=QStringLiteral("Chosen surface is hidden behind another tile. Orbit to see it.");
    }
    return target;
}

std::optional<AreaPaintTarget> pickAreaWallTarget(const std::vector<AreaPreviewVertex>& mesh,
    const std::vector<AreaPreviewFace>& faces,const QImage& atlas,const QMatrix4x4& matrix,
    QPointF mouse,QSize viewport,const AreaPlacementCheck& canPlace) {
    auto target=pickAreaPaintTarget(mesh,faces,atlas,matrix,mouse,viewport,true,canPlace);
    if(!target || target->face.standing()) return target;
    if(target->face.plane!=0 || !canPlace) return {};
    // Intersect the flat surface, then choose its nearest edge in world space.
    // Plane FRONT is the south edge; the north edge uses FRONT in the previous
    // map row. LEFT/RIGHT describe the west/east edges. No new map format.
    const auto inverse=matrix.inverted();
    const float x=2*float(mouse.x())/viewport.width()-1,y=1-2*float(mouse.y())/viewport.height();
    const auto origin=toView((inverse*QVector4D(x,y,-1,1)).toVector3DAffine());
    const auto far=toView((inverse*QVector4D(x,y,1,1)).toVector3DAffine());
    const auto ray=far-origin;
    if(std::abs(ray.z())<1e-7f) return {};
    const auto& quad=target->vertices;
    const auto point=origin+ray*((quad[0].position.z()-origin.z())/ray.z());
    const float distances[]={std::abs(point.y()-quad[0].position.y()),
                             std::abs(point.y()-quad[2].position.y()),
                             std::abs(point.x()-quad[0].position.x()),
                             std::abs(point.x()-quad[1].position.x())};
    const int edge=int(std::min_element(distances,distances+4)-distances);
    const unsigned first[]={0,2,0,1},second[]={1,5,2,5};
    auto a=quad[first[edge]],b=quad[second[edge]];
    const int oldLayer=target->face.layer;
    target->face.plane=edge<2 ? 1 : edge==2 ? 2 : 3;
    if(edge==0) --target->face.y;
    if(target->face.y<0 || !canPlace(target->face)) return {};
    const float depth=(target->face.layer-oldLayer)*.001f;
    a.position+=QVector3D(0,depth,depth); b.position+=QVector3D(0,depth,depth);
    auto topA=a,topB=b;
    topA.position.setZ(topA.position.z()+32); topB.position.setZ(topB.position.z()+32);
    const auto normal=QVector3D::crossProduct(topB.position-topA.position,a.position-topA.position).normalized();
    target->vertices={topA,topB,a,a,topB,b};
    for(auto& vertex:target->vertices) vertex.normal=normal;
    target->existingIndex=-1;
    return target;
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
uniform bool outline;
uniform bool ghost;
uniform vec4 outlineColor;
out vec4 color;
void main() {
    if(outline) { color=outlineColor; return; }
    vec4 tex=texture(atlas,uv);
    if(tex.a<0.1) discard;
    float shade=0.65+0.35*abs(dot(normalize(n),normalize(vec3(0.3,-0.4,1))));
    color=vec4(tex.rgb*shade,ghost ? 0.55 : 1.0);
})";
    if(!_program->addShaderFromSourceCode(QOpenGLShader::Vertex,vertex) ||
       !_program->addShaderFromSourceCode(QOpenGLShader::Fragment,fragment) || !_program->link()) {
        _error=_program->log(); return false;
    }
    if(!_vao.create() || !_vbo.create() || !_edges.create() || !_ghostVao.create() || !_ghostVbo.create()) { _error="Could not allocate preview geometry."; return false; }
    QOpenGLVertexArrayObject::Binder vao(&_vao);
    _edges.bind();
    const GLuint edges[]={0,1,5,2};
    _edges.allocate(edges,sizeof edges);
    _vbo.bind();
    _program->bind();
    _program->enableAttributeArray(0); _program->enableAttributeArray(1); _program->enableAttributeArray(2);
    _program->setAttributeBuffer(0,GL_FLOAT,offsetof(AreaPreviewVertex,position),3,sizeof(AreaPreviewVertex));
    _program->setAttributeBuffer(1,GL_FLOAT,offsetof(AreaPreviewVertex,uv),2,sizeof(AreaPreviewVertex));
    _program->setAttributeBuffer(2,GL_FLOAT,offsetof(AreaPreviewVertex,normal),3,sizeof(AreaPreviewVertex));
    _vbo.release();
    {
        QOpenGLVertexArrayObject::Binder ghostVao(&_ghostVao);
        _edges.bind(); _ghostVbo.bind();
        _ghostVbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        for(int attribute=0;attribute<3;++attribute) _program->enableAttributeArray(attribute);
        _program->setAttributeBuffer(0,GL_FLOAT,offsetof(AreaPreviewVertex,position),3,sizeof(AreaPreviewVertex));
        _program->setAttributeBuffer(1,GL_FLOAT,offsetof(AreaPreviewVertex,uv),2,sizeof(AreaPreviewVertex));
        _program->setAttributeBuffer(2,GL_FLOAT,offsetof(AreaPreviewVertex,normal),3,sizeof(AreaPreviewVertex));
        _ghostVbo.release();
    }
    _program->release();
    return true;
}
void AreaPreviewRenderer::release() {
    _texture.reset(); _program.reset(); _vbo.destroy(); _edges.destroy(); _vao.destroy(); _ghostVbo.destroy(); _ghostVao.destroy(); _count=0; _atlasKey=0;
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
void AreaPreviewRenderer::draw(const QSize& pixels, const QMatrix4x4& matrix, int highlighted, bool standing, const std::vector<AreaPreviewVertex>& ghost, bool invalidGhost, bool replacesTile) {
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
    _program->setUniformValue("outline",false);
    _program->setUniformValue("ghost",false);
    glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(1,1);
    glDrawArrays(GL_TRIANGLES,0,_count);
    glDisable(GL_POLYGON_OFFSET_FILL);
    if(highlighted>=0 && highlighted<_count/6) {
        _program->setUniformValue("outline",true);
        _program->setUniformValue("outlineColor",standing ? QVector4D(0,1,1,1) : QVector4D(1,1,0,1));
        glLineWidth(1);
        glDrawElementsBaseVertex(GL_LINE_LOOP,4,GL_UNSIGNED_INT,nullptr,highlighted*6);
    }
    if(ghost.size()==6) {
        QOpenGLVertexArrayObject::Binder ghostVao(&_ghostVao);
        _ghostVbo.bind(); _ghostVbo.allocate(ghost.data(),int(ghost.size()*sizeof(AreaPreviewVertex))); _ghostVbo.release();
        _program->setUniformValue("outline",false); _program->setUniformValue("ghost",true);
        glDepthMask(GL_FALSE); glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(-1,-1);
        glDrawArrays(GL_TRIANGLES,0,6);
        glDisable(GL_POLYGON_OFFSET_FILL); glDisable(GL_BLEND);
        _program->setUniformValue("outline",true);
        _program->setUniformValue("outlineColor",invalidGhost ? QVector4D(1,.15f,.1f,1) : replacesTile ? QVector4D(1,.65f,.1f,1) : QVector4D(0.3f,1,0.3f,1));
        glDrawElementsBaseVertex(GL_LINE_LOOP,4,GL_UNSIGNED_INT,nullptr,0);
        glDepthMask(GL_TRUE);
    }
    _texture->release(0); _program->release();
}

AreaPreview::AreaPreview(QWidget* parent):QOpenGLWidget(parent) {
    setMinimumSize(240,200);
    setMouseTracking(true);
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
    if(event->type()==QEvent::Hide || event->type()==QEvent::WindowDeactivate || event->type()==QEvent::UngrabMouse)
        finishStroke();
    if(event->type()==QEvent::ShortcutOverride) {
        const auto* key=static_cast<QKeyEvent*>(event);
        if(key->modifiers() & Qt::ControlModifier) finishStroke();
    }
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
void AreaPreview::setScene(std::vector<AreaPreviewVertex> mesh, QImage atlas, bool resetCamera, std::vector<AreaPreviewFace> faces) {
    if(resetCamera) clearSurface();
    const auto anchor=_paintAnchor;
    clearHover(); _paintAnchor=anchor; _faces=std::move(faces);
    _mesh=std::move(mesh); _atlas=std::move(atlas); _dirty=true;
    if(resetCamera) frameArea();
    if(underMouse()) pickAt(mapFromGlobal(QCursor::pos()));
    update();
}
void AreaPreview::frameArea() {
    _paintAnchor.reset();
    if(_mesh.empty()) return;
    auto lo=_mesh.front().position, hi=lo;
    for(const auto& vertex:_mesh) for(int axis=0;axis<3;++axis) {
        lo[axis]=std::min(lo[axis],vertex.position[axis]); hi[axis]=std::max(hi[axis],vertex.position[axis]);
    }
    _camera.frame(lo,hi,static_cast<float>(width())/std::max(1,height())); update();
}
void AreaPreview::focusWorldPoint(const QVector3D& point, float radius) {
    _paintAnchor.reset();
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
        _camera.matrix(static_cast<float>(width())/std::max(1,height())),_hovered,
        hoveredFace() && hoveredFace()->standing(),_ghost,_target && !_target->error.isEmpty(),_target && _target->replacesTile);
}
void AreaPreview::setTool(Tool tool) {
    finishStroke();
    if(tool==Tool::Wall) { clearSurface(); setPlacementPlane(-1); }
    _tool=tool; clearHover();
    if(onToolChanged) onToolChanged(tool);
}
void AreaPreview::setPaintLayer(int layer) {
    finishStroke(); _paintLayer=(layer>=0 && layer<4) ? layer : -2;
    if(underMouse()) pickAt(mapFromGlobal(QCursor::pos())); else clearHover();
}
void AreaPreview::resolveDestination(AreaPaintTarget& target,bool stroke) const {
    if(_pickingSurface || _tool==Tool::Select || !target.error.isEmpty()) return;
    if(_paintLayer==-2) { target.error=tr("Select a tile layer in Layer View to paint."); return; }
    const auto& faces=stroke ? _strokeFaces : _faces;
    std::array<TilePlacementSlot,4> slots{};
    if(placementSlots) slots=placementSlots(target.face.x,target.face.y);
    else for(const auto& face:faces) {
        if(face.x==target.face.x && face.y==target.face.y && face.layer>=0 && face.layer<4)
            slots[face.layer]={true,face.elevation,face.plane};
    }
    const bool erase=_erasing || _tool==Tool::Erase;
    const auto destination=resolveTilePlacement(target.face,slots,_paintLayer>=0 ? _paintLayer : target.face.layer,erase,_layerCount);
    if(destination.action==TilePlacementAction::Reject) {
        target.existingIndex=-1;
        switch(destination.failure) {
            case TilePlacementFailure::Full:
                target.error=tr("Cannot add this surface at (%1, %2): all %3 slots hold other surfaces. Nothing will be replaced.").arg(target.face.x).arg(target.face.y).arg(_layerCount); break;
            case TilePlacementFailure::Missing: target.error=tr("That surface no longer exists. Nothing will be erased."); break;
            default: target.error=tr("Outside the map or supported tile range."); break;
        }
        return;
    }
    const int oldLayer=target.face.layer;
    target.face=destination.face;
    target.replacesTile=destination.action==TilePlacementAction::Repaint;
    target.existingIndex=-1;
    if(destination.action!=TilePlacementAction::Add) for(size_t i=0;i<faces.size();++i) {
        const auto& face=faces[i];
        if(face.x==target.face.x && face.y==target.face.y && face.layer==target.face.layer &&
           face.elevation==target.face.elevation && face.plane==target.face.plane) target.existingIndex=int(i);
    }
    const float depth=(target.face.layer-oldLayer)*.001f;
    for(auto& vertex:target.vertices) vertex.position+=QVector3D(0,depth,depth);
    if(!erase && onPlacementError) target.error=onPlacementError(target.face);
}
void AreaPreview::setPlacementPlane(int plane) {
    if(plane < -1 || plane > 6) return;
    finishStroke(); _placementPlane=plane;
    if(plane>=0) { _tool=Tool::Paint; if(onToolChanged) onToolChanged(_tool); }
    if(onPlacementPlaneChanged) onPlacementPlaneChanged(plane);
    if(underMouse()) pickAt(mapFromGlobal(QCursor::pos())); else clearHover();
}
void AreaPreview::applyPlacementPlane(AreaPaintTarget& target) const {
    if(_placementPlane<0 || _erasing || _tool!=Tool::Paint || _pickingSurface || !target.error.isEmpty()) return;
    // Explicit orientation changes the tile being placed, not the picking grid.
    // Determine the requested surface before resolving its storage slot.
    target.face.plane=_placementPlane;
    if(onFacePreview || onBrushPreview) {
        const auto geometry=onFacePreview ? onFacePreview(target.face) : onBrushPreview(target.face);
        if(geometry.size()>=target.vertices.size()) std::copy_n(geometry.begin(),target.vertices.size(),target.vertices.begin());
    }
}
void AreaPreview::pickSurface() {
    finishStroke(); _pickingSurface=true; clearHover(); setFocus(Qt::OtherFocusReason);
    if(onSurfaceChanged) onSurfaceChanged();
}
void AreaPreview::clearSurface() {
    finishStroke(); _surface.reset(); _pickingSurface=false; clearHover();
    if(onSurfaceChanged) onSurfaceChanged();
}
void AreaPreview::chooseSurface(const QPointF& point) {
    const int hit=pickAreaPreviewFace(_mesh,_atlas,_camera.matrix(float(width())/std::max(1,height())),point,size());
    if(hit<0 || size_t(hit)>=_faces.size()) {
        if(onPlacementHint) onPlacementHint(tr("Click an existing floor or wall to choose its surface."));
        return;
    }
    AreaPaintTarget surface; surface.face=_faces[size_t(hit)]; surface.existingIndex=hit;
    std::copy_n(_mesh.begin()+size_t(hit)*6,6,surface.vertices.begin());
    _surface=surface; _pickingSurface=false; setTool(Tool::Paint); setFocus(Qt::MouseFocusReason);
    if(onSurfaceChanged) onSurfaceChanged();
    pickAt(point);
}
void AreaPreview::keyPressEvent(QKeyEvent* event) {
    if(event->key()==Qt::Key_Escape) { clearSurface(); event->accept(); return; }
    if(event->modifiers()==Qt::NoModifier) {
        int plane=-2;
        switch(event->key()) {
            case Qt::Key_Up: plane=1; break;
            case Qt::Key_Down: plane=0; break;
            case Qt::Key_Left: plane=2; break;
            case Qt::Key_Right: plane=3; break;
            case Qt::Key_N: plane=(_placementPlane+1)%7; break;
            default: break;
        }
        if(plane>=0) { setPlacementPlane(plane); event->accept(); return; }
    }
    QOpenGLWidget::keyPressEvent(event);
}
std::optional<AreaPaintTarget> AreaPreview::targetAt(const QPointF& point,bool stroke) const {
    const auto& mesh=stroke ? _strokeMesh : _mesh;
    const auto& faces=stroke ? _strokeFaces : _faces;
    const auto& atlas=stroke ? _strokeAtlas : _atlas;
    const auto matrix=stroke ? _strokeMatrix : _camera.matrix(float(width())/std::max(1,height()));
    const auto& surface=stroke ? _strokeSurface : _surface;
    if(_pickingSurface) return pickAreaPaintTarget(mesh,faces,atlas,matrix,point,size(),false,{});
    const bool erase=_erasing || _tool==Tool::Erase;
    if(surface && _tool!=Tool::Select) return pickAreaSurfaceTarget(*surface,mesh,faces,atlas,matrix,point,size(),canPlace,erase);
    return (_tool==Tool::Wall && !erase) ? pickAreaWallTarget(mesh,faces,atlas,matrix,point,size(),canPlace) :
        pickAreaPaintTarget(mesh,faces,atlas,matrix,point,size(),_tool!=Tool::Select && !erase,canPlace,
            _tool==Tool::Paint && !erase ? _placementPlane : -1,
            _tool==Tool::Paint && !erase && _paintAnchor ? &*_paintAnchor : nullptr);
}
void AreaPreview::finishStroke() {
    if(_strokeButton==Qt::NoButton) return;
    _strokeButton=Qt::NoButton; _erasing=false; _strokeSurface.reset();
    _strokeMesh.clear(); _strokeFaces.clear(); _strokeAtlas={}; _paintedFaces.clear();
    if(onStrokeEnd) onStrokeEnd();
}
void AreaPreview::paintAt(const QPointF& point) {
    auto target=targetAt(point,true);
    if(!target || !target->error.isEmpty()) return;
    if(!_erasing && !_strokeSurface) _strokeSurface=*target;
    applyPlacementPlane(*target);
    resolveDestination(*target,true);
    if(!target->error.isEmpty()) return;
    const auto face=target->face;
    const quint64 key=(quint64(face.layer)<<32)|(quint64(face.y)<<16)|quint64(face.x);
    if(_paintedFaces.contains(key)) return;
    if(!onPaint || !onPaint(face)) return;
    _paintedFaces.insert(key);
    // Newly placed faces extend the brush surface during the same drag. Erasing
    // retains the original snapshot and never drills through to a lower layer.
    if(target->existingIndex<0) {
        _strokeMesh.insert(_strokeMesh.end(),target->vertices.begin(),target->vertices.end());
        _strokeFaces.push_back(face);
    }
}

void AreaPreview::mousePressEvent(QMouseEvent* event) {
    finishStroke();
    _lastMouse=event->position(); _navigation=Navigation::None;
    const bool alt=event->modifiers() & Qt::AltModifier;
    if(event->button()==Qt::RightButton && !alt) {
        // Sample the actual visible tile, independent of placement direction,
        // surface locks and hypothetical extensions. Sampling never starts Undo.
        const int hit=pickAreaPreviewFace(_mesh,_atlas,_camera.matrix(float(width())/std::max(1,height())),event->position(),size());
        if(hit>=0 && size_t(hit)<_faces.size() && onSampleTile) {
            const auto face=_faces[size_t(hit)];
            if(onSampleTile(face)) {
                clearSurface(); setTool(Tool::Paint); setFocus(Qt::MouseFocusReason);
                pickAt(event->position());
            }
        } else if(onPlacementHint) onPlacementHint(tr("No tile here to pick. Right-click visible tile artwork."));
        event->accept(); return;
    }
    if(event->button()==Qt::LeftButton && !alt && (_pickingSurface || (event->modifiers() & Qt::ShiftModifier))) {
        chooseSurface(event->position()); event->accept(); return;
    }
    if(event->button()==Qt::MiddleButton || (alt && event->button()==Qt::RightButton)) _navigation=Navigation::Pan;
    else if(event->button()==Qt::LeftButton && (alt || _tool==Tool::Select)) _navigation=Navigation::Orbit;
    else if(event->button()==Qt::LeftButton) {
        const bool erase=_tool==Tool::Erase;
        // Begin first: the adapter removes any temporary 2D tile preview and
        // refreshes the mesh before we snapshot the faces that this drag edits.
        if(onStrokeBegin && onStrokeBegin(erase)) {
            _strokeMesh=_mesh; _strokeFaces=_faces; _strokeAtlas=_atlas;
            _strokeMatrix=_camera.matrix(float(width())/std::max(1,height()));
            _strokeButton=event->button(); _erasing=erase; _strokeSurface=_surface;
            setFocus(Qt::MouseFocusReason);
            paintAt(event->position());
        }
    }
    pickAt(event->position()); event->accept();
}
void AreaPreview::mouseMoveEvent(QMouseEvent* event) {
    const QPointF from=_lastMouse, delta=event->position()-from; _lastMouse=event->position();
    if(_strokeButton!=Qt::NoButton) {
        if(event->buttons() & _strokeButton) {
            const int steps=std::max(1,int(std::ceil(std::hypot(delta.x(),delta.y())/2)));
            for(int i=1;i<=steps;++i) paintAt(from+delta*(qreal(i)/steps));
        } else finishStroke();
    } else if(event->buttons()!=Qt::NoButton) {
        if(_navigation==Navigation::Pan) { _paintAnchor.reset(); _camera.pan(delta,height()); }
        else if(_navigation==Navigation::Orbit) { _paintAnchor.reset(); _camera.orbit(delta); }
    }
    pickAt(event->position()); update(); event->accept();
}
void AreaPreview::mouseReleaseEvent(QMouseEvent* event) {
    if(event->button()==_strokeButton) finishStroke();
    _navigation=Navigation::None;
    pickAt(event->position()); event->accept();
}
void AreaPreview::wheelEvent(QWheelEvent* event) {
    finishStroke();
    const float steps=event->pixelDelta().isNull()? event->angleDelta().y()/120.0f : event->pixelDelta().y()/40.0f;
    _paintAnchor.reset(); _camera.zoom(steps); pickAt(event->position()); update(); event->accept();
}

const AreaPreviewFace* AreaPreview::hoveredFace() const { return _target ? &_target->face : nullptr; }
void AreaPreview::pickAt(const QPointF& point) {
    _target=targetAt(point,_strokeButton!=Qt::NoButton);
    // Remember only real geometry, never a speculative extension. The source
    // is revalidated against the live mesh before it can guide another pick.
    if(_target && _target->existingIndex>=0 && _tool==Tool::Paint && !_pickingSurface && !_erasing)
        _paintAnchor=_target;
    if(_target) { applyPlacementPlane(*_target); resolveDestination(*_target,_strokeButton!=Qt::NoButton); }
    _hovered=_target ? _target->existingIndex : -1;
    _ghost.clear();
    if(_target && !_pickingSurface && (_tool==Tool::Paint || _tool==Tool::Wall) && !_erasing) {
        if(onBrushPreview) _ghost=onBrushPreview(_target->face);
        if(_ghost.empty() && !_target->error.isEmpty()) _ghost.assign(_target->vertices.begin(),_target->vertices.end());
    }
    if(onHover) onHover(hoveredFace());
    if(onPlacementHint) {
        QString hint=_target ? _target->error : QString();
        if(!_target && _surface) hint=tr("No target on the chosen surface. Orbit to face it.");
        if(_target && hint.isEmpty() && _paintLayer>=0 && !_pickingSurface && _tool!=Tool::Select) {
            hint=(_erasing || _tool==Tool::Erase) ? tr("Erase pointed surface on Layer %1").arg(_target->face.layer+1) :
                _target->replacesTile ? tr("REPAINT this surface on Layer %1 · Position and direction stay unchanged").arg(_target->face.layer+1) :
                tr("ADD surface on Layer %1 · Existing surfaces stay untouched").arg(_target->face.layer+1);
        }
        onPlacementHint(hint);
    }
    update();
}

void AreaPreview::clearHover() {
    _hovered=-1; _target.reset(); _paintAnchor.reset(); _ghost.clear();
    if(onHover) onHover(nullptr);
    if(onPlacementHint) onPlacementHint({});
    update();
}
void AreaPreview::leaveEvent(QEvent* event) { clearHover(); QOpenGLWidget::leaveEvent(event); }
void AreaPreview::resizeGL(int, int) { finishStroke(); clearHover(); }
