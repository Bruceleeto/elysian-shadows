/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#include <ESToolkit/ui/widget/tk_area_preview.h>
#include <QApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QDockWidget>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

static void require(bool condition, const char* message) {
    if(!condition) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
static std::vector<AreaPreviewVertex> quad(float z=0) {
    return {{{-.8f,-.8f,z},{0,0},{0,0,1}}, {{.8f,-.8f,z},{1,0},{0,0,1}},
            {{-.8f,.8f,z},{0,1},{0,0,1}}, {{-.8f,.8f,z},{0,1},{0,0,1}},
            {{.8f,-.8f,z},{1,0},{0,0,1}}, {{.8f,.8f,z},{1,1},{0,0,1}}};
}
static QImage atlas() {
    QImage image(2,2,QImage::Format_RGBA8888);
    image.setPixelColor(0,0,Qt::red); image.setPixelColor(1,0,Qt::green);
    image.setPixelColor(0,1,Qt::blue); image.setPixelColor(1,1,Qt::white);
    return image;
}
static void cameraChecks() {
    AreaPreviewCamera camera;
    camera.frame({0,0,0},{320,640,96},1);
    require(camera.target==QVector3D(160,-320,48),"frame uses engine Y down, Z up");
    const auto matrix=camera.matrix(1);
    for(float x:{0.0f,320.0f}) for(float y:{0.0f,640.0f}) for(float z:{0.0f,96.0f}) {
        const auto p=matrix*QVector4D(x,-y,z,1);
        require(p.w()>0 && std::abs(p.x()/p.w())<1 && std::abs(p.y()/p.w())<1,"frame includes all corners");
    }
    const float distance=camera.distance;
    camera.zoom(1); require(camera.distance<distance,"wheel zooms in");
    camera.orbit({50,10000}); require(camera.pitch==85 && camera.matrix(1)!=matrix,"orbit is bounded and changes view");
    camera.orbit({0,-20000}); require(camera.pitch==-85,"orbit cannot hit lookAt singularity");
    const auto target=camera.target;
    camera.pan({30,10},240); require(camera.target!=target,"pan changes target");
    camera.zoom(10000); require(camera.distance==16,"zoom minimum");
    camera.zoom(-10000); require(camera.distance==100000,"zoom maximum");
}
static int renderChecks(const QSurfaceFormat& format) {
    QOpenGLContext mainContext;
    mainContext.setFormat(format);
    if(!mainContext.create()) { std::puts("SKIP: no offscreen OpenGL context"); return 77; }
    QOffscreenSurface surface;
    surface.setFormat(mainContext.format()); surface.create();
    if(!mainContext.makeCurrent(&surface)) return 77;
    QOpenGLFunctions_3_3_Core gl;
    require(gl.initializeOpenGLFunctions(),"OpenGL functions available");
    QOpenGLFramebufferObjectFormat fbFormat;
    fbFormat.setAttachment(QOpenGLFramebufferObject::Depth);
    auto mainFbo=std::make_unique<QOpenGLFramebufferObject>(128,128,fbFormat);
    require(mainFbo->isValid() && mainFbo->bind(),"main framebuffer");
    gl.glClearColor(.7f,.04f,.6f,1); gl.glClear(GL_COLOR_BUFFER_BIT);
    const auto mainImage=mainFbo->toImage();
    mainFbo->bind(); gl.glViewport(3,4,25,26); gl.glDisable(GL_DEPTH_TEST);

    // The offscreen QPA plugin resets the viewport on makeCurrent; record that
    // platform behavior separately from anything the preview renderer does.
    mainContext.doneCurrent(); mainContext.makeCurrent(&surface);
    GLint expectedViewport[4]; gl.glGetIntegerv(GL_VIEWPORT,expectedViewport);

    QOpenGLContext previewContext;
    previewContext.setFormat(format); previewContext.setShareContext(&mainContext);
    require(previewContext.create() && previewContext.makeCurrent(&surface),"shared preview context");
    auto previewFbo=std::make_unique<QOpenGLFramebufferObject>(128,128,fbFormat);
    require(previewFbo->isValid() && previewFbo->bind(),"preview framebuffer");
    AreaPreviewRenderer renderer;
    require(renderer.initialize(),qPrintable(renderer.error()));
    QMatrix4x4 projection; projection.ortho(-1,1,-1,1,-10,10);
    renderer.upload(quad(),atlas()); renderer.draw({128,128},projection);
    auto image=previewFbo->toImage();
    require(image.pixelColor(32,32).red()>150 && image.pixelColor(32,32).blue()<5,"top-left texel orientation");
    require(image.pixelColor(96,32).green()>150,"top-right texel orientation");
    require(image.pixelColor(32,96).blue()>150,"bottom-left texel orientation");

    auto front=quad(1), behind=quad();
    for(auto& vertex:front) vertex.uv={.25f,.75f}; // blue, closer to the camera
    front.insert(front.end(),behind.begin(),behind.end()); // farther quad drawn last
    renderer.upload(front,atlas()); renderer.draw({128,128},projection);
    image=previewFbo->toImage();
    require(image.pixelColor(32,32).blue()>150 && image.pixelColor(32,32).red()<5,"depth test hides geometry behind a raised face");
    QImage transparent(2,2,QImage::Format_RGBA8888); transparent.fill(Qt::transparent);
    renderer.upload(front,transparent); renderer.draw({128,128},projection);
    image=previewFbo->toImage();
    require(image.pixelColor(32,32)==image.pixelColor(0,0),"atlas replacement and transparent tile pixels");
    renderer.upload({},atlas()); renderer.draw({128,128},projection);
    require(previewFbo->toImage().pixelColor(64,64)==image.pixelColor(0,0),"clearing area removes old geometry");
    require(gl.glGetError()==GL_NO_ERROR,"preview OpenGL calls have no errors");

    require(mainContext.makeCurrent(&surface),"return to main context");
    GLint viewport[4], framebuffer;
    gl.glGetIntegerv(GL_VIEWPORT,viewport); gl.glGetIntegerv(GL_FRAMEBUFFER_BINDING,&framebuffer);
    require(std::equal(viewport,viewport+4,expectedViewport),"main viewport unchanged");
    require(static_cast<GLuint>(framebuffer)==mainFbo->handle(),"main framebuffer binding unchanged");
    require(!gl.glIsEnabled(GL_DEPTH_TEST),"main depth state unchanged");
    require(mainFbo->toImage()==mainImage,"preview never overwrites main framebuffer");
    require(gl.glGetError()==GL_NO_ERROR,"main context has no errors");
    mainFbo.reset();
    previewContext.makeCurrent(&surface); renderer.release(); previewFbo.reset();
    std::puts("PASS: camera, texture orientation, depth, live replacement, alpha, clearing and context/framebuffer isolation");
    return 0;
}

// Optional hidden widget integration run on a window-system platform. This
// exercises Qt's actual QOpenGLWidget callbacks without starting the editor.
static int widgetChecks() {
    QMainWindow window;
    window.setAttribute(Qt::WA_DontShowOnScreen);
    auto* central=new QWidget(&window); window.setCentralWidget(central);
    auto* layout=new QVBoxLayout(central);
    auto* input=new QLineEdit(&window); layout->addWidget(input);
    auto* mainView=new AreaPreview(&window); layout->addWidget(mainView);
    auto* dock=new QDockWidget("3D Preview",&window);
    dock->setAttribute(Qt::WA_DontShowOnScreen);
    auto* preview=new AreaPreview(dock); dock->setWidget(preview);
    window.addDockWidget(Qt::RightDockWidgetArea,dock);
    window.resize(400,650);
    mainView->setScene(quad(),atlas(),true);
    preview->setScene(quad(),atlas(),true);
    window.show(); QApplication::processEvents();
    require(mainView->isValid() && preview->isValid(),"hidden Qt widgets have contexts");
    require(mainView->context()!=preview->context(),"Qt creates separate view contexts");
    const auto mainImage=mainView->grabFramebuffer();
    const auto first=preview->grabFramebuffer();
    QImage blue(2,2,QImage::Format_RGBA8888); blue.fill(Qt::blue);
    preview->setScene(quad(3),blue,true);
    mainView->makeCurrent();
    preview->repaint(); QApplication::sendPostedEvents(preview,QEvent::UpdateRequest);
    require(QOpenGLContext::currentContext()==mainView->context(),"preview repaint restores the main editing context");
    const auto second=preview->grabFramebuffer();
    require(first!=second,"widget receives live scene updates");
    require(mainView->grabFramebuffer()==mainImage,"main widget survives preview updates");
    require(preview->focusPolicy()==Qt::NoFocus,"preview does not acquire keyboard focus");
    const auto beforeOrbit=preview->grabFramebuffer();
    QMouseEvent press(QEvent::MouseButtonPress,{100,100},{100,100},Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove,{170,130},{170,130},Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
    QApplication::sendEvent(preview,&press); QApplication::sendEvent(preview,&move);
    require(preview->grabFramebuffer()!=beforeOrbit,"mouse drag orbits the actual preview widget");
    preview->hide(); preview->show(); QApplication::processEvents();
    require(!preview->grabFramebuffer().isNull(),"preview survives hide/show");
    dock->setFloating(true); QApplication::processEvents();
    require(preview->isValid() && !preview->grabFramebuffer().isNull(),"floating dock recreates preview resources");
    dock->setFloating(false); QApplication::processEvents();
    require(preview->isValid() && !preview->grabFramebuffer().isNull(),"redocking recreates preview resources");
    mainView->makeCurrent(); preview->repaint();
    require(QOpenGLContext::currentContext()==mainView->context(),"redocked preview restores editing context");
    std::puts("PASS: hidden Qt widgets, live updates, context restoration, independent framebuffers, orbit, hide/show and floating/redocking");
    return 0;
}
int main(int argc,char** argv) {
    QSurfaceFormat format;
    format.setVersion(3,3); format.setProfile(QSurfaceFormat::CoreProfile); format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    cameraChecks();
    return app.arguments().contains("--widgets") ? widgetChecks() : renderChecks(format);
}
