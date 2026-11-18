/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */
#include <ESToolkit/ui/widget/tk_area_preview.h>
#include <ESToolkit/ui/widget/tk_area_editor_layout.h>
#include <QGraphicsView>
#include <QTabBar>
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
static void layoutChecks() {
    QWidget central;
    central.setAttribute(Qt::WA_DontShowOnScreen);
    central.resize(1220,610);
    auto* grid=new QGridLayout(&central);
    grid->setContentsMargins(0,0,0,0);
    auto* tabs=new QTabBar(&central); tabs->addTab("loren_town");
    grid->addWidget(tabs,0,0,Qt::AlignLeft|Qt::AlignCenter);
    auto* map=new QGraphicsView(&central); grid->addWidget(map,1,0);
    grid->addItem(new QSpacerItem(0,0),2,0,Qt::AlignBottom);
    auto* splitter=createAreaEditorSplitter(grid,map);
    auto* threeD=new QWidget(splitter); threeD->setMinimumSize(240,200);
    splitter->addWidget(threeD); splitter->addWidget(map); threeD->hide();
    // ReadSettings calls adjustSize after creating the central map layout.
    map->adjustSize(); central.show(); QApplication::processEvents();
    require(splitter->height()>central.height()-70 && map->height()==splitter->height(),
            "2D map fills the central area instead of collapsing to a 192-pixel strip");
    threeD->show(); splitter->setSizes({900,300}); QApplication::processEvents();
    require(threeD->height()==splitter->height() && map->height()==splitter->height(),
            "both 3D and 2D panes fill the central height");
    require(threeD->width()>map->width(),"3D is the main view with 2D alongside");
    threeD->hide(); central.resize(1000,800); QApplication::processEvents();
    require(map->height()>730 && map->width()==splitter->width(),"returning to 2D restores the whole map area after resize");
    std::puts("PASS: central 2D/3D splitter layout, toggle and resize");
}
static void pickingChecks() {
    QMatrix4x4 projection; projection.ortho(-1,1,-1,1,-10,10);
    const QSize size(200,200);
    const auto pick=[&](const std::vector<AreaPreviewVertex>& mesh,const QImage& image,QPointF point) {
        return pickAreaPreviewFace(mesh,image,projection,point,size);
    };
    require(pick(quad(),atlas(),{100,100})==0,"ray picks a flat face");
    require(pick(quad(),atlas(),{0,0})==-1,"empty space has no tile");
    require(pick(quad(),atlas(),{-1,100})==-1,"outside viewport has no tile");
    auto mesh=quad(2); const auto ground=quad();
    mesh.insert(mesh.end(),ground.begin(),ground.end());
    require(pick(mesh,atlas(),{100,100})==0,"near raised face wins over later ground");
    auto sameDepth=quad(); sameDepth.insert(sameDepth.end(),ground.begin(),ground.end());
    require(pick(sameDepth,atlas(),{100,100})==1,"equal-depth picking matches last rendered layer");
    QImage cutout(2,1,QImage::Format_RGBA8888); cutout.setPixelColor(0,0,Qt::transparent); cutout.setPixelColor(1,0,Qt::white);
    for(size_t i=0;i<mesh.size();++i) mesh[i].uv={i<6?.25f:.75f,.5f};
    require(pick(mesh,cutout,{100,100})==1,"transparent front texels reveal the tile behind");
    QMatrix4x4 invalid; invalid.fill(0);
    require(pickAreaPreviewFace(mesh,cutout,invalid,{100,100},size)==-1,"singular camera cannot produce a pick");
    auto wall=quad();
    for(auto& v:wall) {const float y=v.position.y(); v.position.setY(0); v.position.setZ(y);}
    AreaPreviewCamera camera; camera.frame({-1,-1,-1},{1,1,1},1);
    const auto projected=camera.matrix(1)*QVector4D(0,0,0,1);
    const QPointF center((projected.x()/projected.w()+1)*100,(1-projected.y()/projected.w())*100);
    require(pickAreaPreviewFace(wall,atlas(),camera.matrix(1),center,size)==0,"perspective ray picks a standing wall");
    require(AreaPreviewFace{31,13,2,0,0}.standing()==false && AreaPreviewFace{31,7,1,2,1}.standing(),
            "flat and standing face metadata are distinct");
    std::puts("PASS: flat/wall picking, nearest layer, transparent pixels, misses and camera transforms");
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

    const auto outlineCount=[&](bool standing) {
        renderer.draw({128,128},projection,0,standing);
        const auto outlined=previewFbo->toImage();
        unsigned count=0;
        for(int y=0;y<outlined.height();++y) for(int x=0;x<outlined.width();++x) {
            const auto pixel=outlined.pixelColor(x,y);
            if(pixel.green()>245 && (standing ? (pixel.blue()>245 && pixel.red()<5) :
                                               (pixel.red()>245 && pixel.blue()<5))) ++count;
        }
        return count;
    };
    require(outlineCount(false)>200,"flat tile gets a yellow perimeter");
    require(outlineCount(true)>200,"standing tile gets a cyan perimeter");

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
    preview->setScene(quad(),atlas(),true,{{31,13,2,0,0}});
    bool hovered=false;
    preview->onHover=[&](const AreaPreviewFace* face) {
        hovered=face && face->x==31 && face->y==13 && face->layer==2 && face->elevation==0;
    };
    const QPointF center=preview->rect().center();
    QMouseEvent hover(QEvent::MouseMove,center,center,Qt::NoButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(preview,&hover);
    require(hovered,"mouse hover reports the actual tile coordinates and layer");
    QEvent leave(QEvent::Leave); QApplication::sendEvent(preview,&leave);
    require(!hovered && !preview->hoveredFace(),"leaving the view clears shared hover");
    preview->onHover={};
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
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3,3); format.setProfile(QSurfaceFormat::CoreProfile); format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);
    cameraChecks();
    pickingChecks();
    layoutChecks();
    return app.arguments().contains("--widgets") ? widgetChecks() : renderChecks(format);
}
