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
#include <QKeyEvent>
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
    map->hide(); QApplication::processEvents();
    require(threeD->width()==splitter->width(),"hiding optional 2D side view gives 3D the full width");
    map->show();
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
static void snappingChecks() {
    QImage white(2,2,QImage::Format_RGBA8888); white.fill(Qt::white);
    const QSize size(600,600);
    const auto flat=[](QVector3D center) {
        auto vertices=quad();
        for(auto& v:vertices) v.position=v.position*20+center;
        return vertices;
    };
    AreaPreviewCamera camera; camera.frame({240,340,0},{440,500,192},1);
    const auto matrix=camera.matrix(1);
    const auto screen=[&](QVector3D p) {
        const auto q=matrix*QVector4D(p.x(),-p.y(),p.z(),1);
        return QPointF((q.x()/q.w()+1)*300,(1-q.y()/q.w())*300);
    };
    const AreaPlacementCheck allow=[](AreaPreviewFace&){return true;};
    auto mesh=flat({320,416,96});
    const std::vector<AreaPreviewFace> faces{{10,10,2,3,0}};
    auto target=pickAreaPaintTarget(mesh,faces,white,matrix,screen({352,416,96}),size,true,allow);
    require(target && target->existingIndex==-1 && target->face.x==11 && target->face.y==10 &&
            target->face.elevation==3 && target->face.plane==0,"empty roof square snaps to adjacent roof height");
    require(!pickAreaPaintTarget(mesh,faces,white,matrix,screen({384,416,96}),size,true,allow),
            "unattached empty space does not invent a painting plane");
    require(!pickAreaPaintTarget(mesh,faces,white,matrix,screen({352,416,96}),size,false,allow),
            "erase never extends a neighbouring surface");
    require(!pickAreaPaintTarget(mesh,faces,white,matrix,screen({352,416,96}),size,true,
            [](AreaPreviewFace&){return false;}),"occupied or out-of-map slots reject extension");
    target=pickAreaPaintTarget(mesh,faces,white,matrix,screen({352,416,96}),size,true,
            [](AreaPreviewFace& f){f.layer=1;return true;});
    require(target && target->face.layer==1 && std::abs(target->vertices[0].position.z()-95.999f)<.0001f,
            "free alternative layer keeps the engine's depth offset");
    auto covered=flat({352,416,96}); mesh.insert(mesh.end(),covered.begin(),covered.end());
    target=pickAreaPaintTarget(mesh,{{10,10,2,3,0},{11,10,1,3,0}},white,matrix,screen({352,416,96}),size,true,allow);
    require(target && target->existingIndex==1,"existing face wins over a coplanar imagined extension");

    // FRONT: top edge at (elev+1)*32, bottom at elev*32.
    auto wall=quad();
    for(auto& v:wall) { const auto p=v.position; v.position={320+p.x()*20,432,112-p.y()*20}; }
    target=pickAreaPaintTarget(wall,{{10,10,1,3,1}},white,matrix,screen({320,432,80}),size,true,allow);
    require(target && target->face.x==10 && target->face.y==11 && target->face.elevation==2 && target->face.plane==1,
            "wall gap snaps down one elevation and forward one map row without changing wall position");
    require(std::abs(target->vertices[0].position.y()-432)<.001f &&
            std::abs(target->vertices[0].position.z()-96)<.001f,"snapped wall geometry meets the source edge exactly");
    target=pickAreaPaintTarget(wall,{{10,10,1,0,1}},white,matrix,screen({320,432,80}),size,true,allow);
    require(!target,"cannot extend below the map elevation range");
    auto slope=wall;
    for(auto& v:slope) v.position.setY(432+112-v.position.z());
    target=pickAreaPaintTarget(slope,{{10,10,1,3,4}},white,matrix,screen({320,464,80}),size,true,allow);
    require(target && target->face.y==12 && target->face.elevation==2 && target->face.plane==4,
            "slope continuation uses both world Y and Z displacement");
    std::puts("PASS: roof/wall/slope gap snapping, inherited height, occupied slots and erase isolation");
}
static void wallChecks() {
    auto floor=quad();
    for(auto& v:floor) v.position=v.position*20+QVector3D(320,416,96);
    QImage white(2,2,QImage::Format_RGBA8888); white.fill(Qt::white);
    AreaPreviewCamera camera; camera.frame({240,340,0},{440,500,192},1);
    const auto matrix=camera.matrix(1);
    const auto screen=[&](QVector3D p) {
        const auto q=matrix*QVector4D(p.x(),-p.y(),p.z(),1);
        return QPointF((q.x()/q.w()+1)*300,(1-q.y()/q.w())*300);
    };
    const std::vector<AreaPreviewFace> faces{{10,10,0,3,0}};
    const AreaPlacementCheck spareLayer=[](AreaPreviewFace& f){f.layer=1;return true;};
    const QVector3D points[]={{320,402,96},{320,430,96},{306,416,96},{334,416,96}};
    const int expectedPlane[]={1,1,2,3};
    for(int edge=0;edge<4;++edge) {
        const auto wall=pickAreaWallTarget(floor,faces,white,matrix,screen(points[edge]),{600,600},spareLayer);
        require(wall && wall->existingIndex==-1 && wall->face.layer==1 && wall->face.elevation==3 &&
                wall->face.x==10 && wall->face.y==(edge==0?9:10) && wall->face.plane==expectedPlane[edge],
                "Wall tool chooses the nearest floor edge and preserves its height in a spare layer");
        for(const auto& vertex:wall->vertices) {
            require(std::abs(vertex.position.z()-96.001f)<.001f || std::abs(vertex.position.z()-128.001f)<.001f,
                    "new wall extends exactly one tile upward from the floor");
            const float expected=edge==0 ? 400.001f : edge==1 ? 432.001f : edge==2 ? 304.f : 336.f;
            require(std::abs((edge<2 ? vertex.position.y() : vertex.position.x())-expected)<.001f,
                    "new wall lies exactly on the chosen floor edge using engine row/elevation coordinates");
        }
    }
    require(!pickAreaWallTarget(floor,faces,white,matrix,screen(points[1]),{600,600},
            [](AreaPreviewFace&){return false;}),"full layer slots cannot overwrite the ground to build a wall");
    require(!pickAreaWallTarget(floor,{{10,0,0,3,0}},white,matrix,screen(points[0]),{600,600},spareLayer),
            "north edge cannot write outside the map");
    require(!pickAreaWallTarget(floor,{{10,10,0,3,4}},white,matrix,screen(points[1]),{600,600},spareLayer),
            "Wall tool does not invent a level base on a sloped tile");
    auto wall=pickAreaWallTarget(floor,faces,white,matrix,screen(points[1]),{600,600},spareLayer);
    std::vector<AreaPreviewVertex> mesh(wall->vertices.begin(),wall->vertices.end());
    const auto repeated=pickAreaWallTarget(mesh,{wall->face},white,matrix,screen({320,432.001f,112.001f}),{600,600},spareLayer);
    require(repeated && repeated->existingIndex==0 && repeated->face.plane==1,
            "pointing at an existing wall paints it without creating another tile");
    const auto higher=pickAreaWallTarget(mesh,{wall->face},white,matrix,screen({320,432.001f,144.001f}),{600,600},spareLayer);
    require(higher && higher->face.elevation==4 && higher->face.y==9 && higher->face.plane==1,
            "pointing above a wall extends it upright without pressing plus");
    std::puts("PASS: upright walls on four floor edges, spare layer, exact height, bounds and upward extension");
}
// A missing section can have another floor visible through it. Choosing the
// wall must win over that floor and remain stable across palette/camera changes.
static void surfaceChecks() {
    QImage white(2,2,QImage::Format_RGBA8888); white.fill(Qt::white);
    const QSize size(600,600);
    auto wall=quad();
    for(auto& v:wall) { const auto p=v.position; v.position={320+p.x()*20,432,112-p.y()*20}; }
    AreaPaintTarget surface; surface.face={10,10,0,3,1};
    std::copy(wall.begin(),wall.end(),surface.vertices.begin());
    AreaPreviewCamera camera; camera.frame({240,340,0},{480,500,192},1);
    const auto pixel=[&](QVector3D point,const QMatrix4x4& matrix) {
        const auto p=matrix*QVector4D(point.x(),-point.y(),point.z(),1);
        return QPointF((p.x()/p.w()+1)*300,(1-p.y()/p.w())*300);
    };
    const AreaPlacementCheck empty=[](AreaPreviewFace& f){return f.x<150 && f.y<150;};
    auto matrix=camera.matrix(1);
    const auto hole=QVector3D(384,432,112);
    auto hit=pickAreaSurfaceTarget(surface,wall,{surface.face},white,matrix,pixel(hole,matrix),size,empty);
    require(hit && hit->error.isEmpty() && hit->face.x==12 && hit->face.y==10 && hit->face.elevation==3 && hit->face.plane==1,
            "locked wall fills gaps beyond the adjacent tile without inventing a horizontal face");
    // Large floor behind the opening catches the same ray, but must not change
    // the chosen plane. It isn't stored in the target wall's map square.
    auto ground=quad(); for(auto& v:ground) v.position=v.position*150+QVector3D(384,400,0);
    auto scene=wall; scene.insert(scene.end(),ground.begin(),ground.end());
    std::vector<AreaPreviewFace> faces{surface.face,{12,12,0,0,0}};
    hit=pickAreaSurfaceTarget(surface,scene,faces,white,matrix,pixel(hole,matrix),size,empty);
    require(hit && hit->error.isEmpty() && hit->face.plane==1 && hit->face.elevation==3,
            "visible floor through a wall gap cannot redirect a chosen wall surface");
    camera.orbit({80,-20}); matrix=camera.matrix(1);
    hit=pickAreaSurfaceTarget(surface,wall,{surface.face},white,matrix,pixel(hole,matrix),size,empty);
    require(hit && hit->face.x==12 && hit->face.y==10 && hit->face.plane==1,"camera orbit preserves the locked world grid");
    hit=pickAreaSurfaceTarget(surface,wall,{surface.face},white,matrix,pixel(hole,matrix),size,
        [](AreaPreviewFace&){return false;});
    require(hit && !hit->error.isEmpty() && hit->face.plane==1,"full square returns an invalid wall preview instead of switching to a floor");
    require(!pickAreaSurfaceTarget(surface,wall,{surface.face},white,matrix,pixel(hole,matrix),size,empty,true),
            "erasing an empty square on a chosen wall cannot erase a floor behind it");
    // A transparent texel on an existing wall is still paintable on its chosen
    // grid: repaint that same slot, not another layer or a face behind it.
    QImage transparent(2,2,QImage::Format_RGBA8888); transparent.fill(Qt::transparent);
    hit=pickAreaSurfaceTarget(surface,wall,{surface.face},transparent,matrix,pixel({320,432,112},matrix),size,empty);
    require(hit && hit->error.isEmpty() && hit->existingIndex==0,"locked surface repaints transparent parts of existing artwork");
    hit=pickAreaSurfaceTarget(surface,{}, {},white,matrix,pixel(hole,matrix),size,empty);
    require(hit && hit->face.x==12 && hit->error.isEmpty(),"chosen grid survives deletion of its original reference tile");
    auto flat=quad(); for(auto& v:flat) v.position=v.position*20+QVector3D(320,416,96);
    AreaPaintTarget floor; floor.face={10,10,0,3,0}; std::copy(flat.begin(),flat.end(),floor.vertices.begin());
    hit=pickAreaSurfaceTarget(floor,flat,{floor.face},white,matrix,pixel({384,416,96},matrix),size,empty);
    require(hit && hit->face.x==12 && hit->face.elevation==3 && hit->face.plane==0,"same surface model fills raised floor gaps");
    // A foreground wall must block painting through it, with a visible reason.
    auto blocker=quad(); for(auto& v:blocker) v.position=v.position*100+QVector3D(320,416,128);
    hit=pickAreaSurfaceTarget(floor,blocker,{{10,9,0,4,0}},white,matrix,pixel({320,416,96},matrix),size,empty);
    require(hit && !hit->error.isEmpty(),"occluded locked target cannot paint through other geometry");
    auto slope=wall;
    for(auto& v:slope) v.position.setY(432+112-v.position.z());
    surface.face.plane=4; std::copy(slope.begin(),slope.end(),surface.vertices.begin());
    hit=pickAreaSurfaceTarget(surface,slope,{surface.face},white,matrix,pixel({320,464,80},matrix),size,empty);
    require(hit && hit->face.y==12 && hit->face.elevation==2 && hit->face.plane==4,"locked slopes keep their own lattice and height progression");
    std::puts("PASS: locked wall/floor/slope grids, multi-tile gaps, orbit, transparent tiles, rejection and occlusion");
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

    auto ghost=quad(.1f);
    for(auto& vertex:ghost) vertex.uv={.25f,.75f};
    renderer.draw({128,128},projection,-1,false,ghost);
    const auto ghostImage=previewFbo->toImage();
    require(ghostImage.pixelColor(32,32).blue()>70 && ghostImage.pixelColor(32,32).red()>40,
            "textured brush preview blends with the existing tile before painting");
    renderer.draw({128,128},projection,-1,false,ghost,true);
    const auto invalid=previewFbo->toImage();
    unsigned redEdges=0;
    for(int y=0;y<invalid.height();++y) for(int x=0;x<invalid.width();++x) {
        const auto c=invalid.pixelColor(x,y);
        if(c.red()>245 && c.green()>25 && c.green()<50 && c.blue()>15 && c.blue()<40) ++redEdges;
    }
    require(redEdges>200,"invalid placement uses a red outline");
    renderer.draw({128,128},projection,-1,false,ghost,false,true);
    const auto replacement=previewFbo->toImage();
    unsigned amberEdges=0;
    for(int y=0;y<replacement.height();++y) for(int x=0;x<replacement.width();++x) {
        const auto c=replacement.pixelColor(x,y);
        if(c.red()>245 && c.green()>150 && c.green()<180 && c.blue()>15 && c.blue()<40) ++amberEdges;
    }
    require(amberEdges>200,"replacement preview is amber, distinct from adding or invalid placement");
    renderer.draw({128,128},projection);
    require(previewFbo->toImage()==image,"removing the brush preview leaves the map rendering unchanged");

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
    QMouseEvent release(QEvent::MouseButtonRelease,{170,130},{170,130},Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(preview,&release);
    preview->setScene(quad(),atlas(),true,{{5,6,1,2,1}});
    preview->setTool(AreaPreview::Tool::Paint);
    unsigned begins=0,painted=0,ends=0; bool erasing=false;
    preview->onStrokeBegin=[&](bool erase) { ++begins; erasing=erase; return true; };
    preview->onStrokeEnd=[&] { ++ends; };
    preview->onPaint=[&](const AreaPreviewFace& face) {
        require(face.x==5 && face.y==6 && face.layer==1,"paint callback receives the picked face");
        ++painted;
        // An erased wall reveals a different layer during the drag. Picking must
        // stay on the original visible faces rather than drilling through it.
        preview->setScene(quad(),atlas(),false,{{5,6,0,0,0}});
    return true; };
    const QPointF paintPoint=preview->rect().center();
    preview->setTool(AreaPreview::Tool::Erase);
    QMouseEvent erasePress(QEvent::MouseButtonPress,paintPoint,paintPoint,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent eraseMove(QEvent::MouseMove,paintPoint+QPointF(1,0),paintPoint+QPointF(1,0),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent eraseRelease(QEvent::MouseButtonRelease,paintPoint,paintPoint,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(preview,&erasePress); QApplication::sendEvent(preview,&eraseMove); QApplication::sendEvent(preview,&eraseRelease);
    require(begins==1 && erasing && painted==1 && ends==1,"Erase tool left drag erases the pointed layer once despite live mesh changes");
    preview->setTool(AreaPreview::Tool::Paint);
    preview->onPaint=[&](const AreaPreviewFace&) { ++painted; return true; };
    QMouseEvent paintPress(QEvent::MouseButtonPress,paintPoint,paintPoint,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent paintRelease(QEvent::MouseButtonRelease,paintPoint,paintPoint,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(preview,&paintPress); QApplication::sendEvent(preview,&paintRelease);
    require(begins==2 && !erasing && painted==2 && ends==2,"left click paints and completes one stroke");
    QMouseEvent orbitPress(QEvent::MouseButtonPress,paintPoint,paintPoint,Qt::LeftButton,Qt::LeftButton,Qt::AltModifier);
    QApplication::sendEvent(preview,&orbitPress); QApplication::sendEvent(preview,&move); QApplication::sendEvent(preview,&release);
    require(begins==2 && painted==2,"Alt-left navigation never paints");
    QApplication::sendEvent(preview,&paintPress);
    QKeyEvent undo(QEvent::ShortcutOverride,Qt::Key_Z,Qt::ControlModifier);
    QApplication::sendEvent(preview,&undo);
    require(ends==3,"Ctrl+Z finishes the live stroke before the undo shortcut runs");
    preview->onPaint={}; preview->onStrokeBegin={}; preview->onStrokeEnd={};
    std::puts("PASS: 3D paint/erase gestures, live-mesh stroke isolation, navigation and undo shortcut");
    auto roof=quad();
    for(auto& v:roof) v.position=v.position*20+QVector3D(320,416,96);
    // focusWorldPoint sets this deterministic camera without changing yaw/pitch;
    // a fresh widget avoids inheriting the orbit exercised above.
    AreaPreview painter;
    painter.setAttribute(Qt::WA_DontShowOnScreen); painter.resize(600,600);
    painter.setScene(roof,atlas(),true,{{10,10,2,3,0}}); painter.show(); QApplication::processEvents();
    painter.focusWorldPoint({352,416,96},96); painter.setTool(AreaPreview::Tool::Paint);
    AreaPreviewCamera camera; camera.frame({256,320,48},{448,512,144},1);
    const auto matrix=camera.matrix(1);
    const auto pixel=[&](float x) {
        const auto q=matrix*QVector4D(x,-416,96,1);
        return QPointF((q.x()/q.w()+1)*300,(1-q.y()/q.w())*300);
    };
    unsigned added=0,completed=0;
    painter.canPlace=[](AreaPreviewFace& f){return f.x>=10 && f.x<=12 && f.y==10;};
    painter.onStrokeBegin=[](bool erase){return !erase;};
    painter.onStrokeEnd=[&]{++completed;};
    painter.onPaint=[&](const AreaPreviewFace& f) {
        require(f.elevation==3 && f.plane==0 && f.layer==2,"dragged extension keeps roof geometry");
        ++added;
    return true; };
    const auto start=pixel(352),end=pixel(384);
    QMouseEvent gapPress(QEvent::MouseButtonPress,start,start,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent gapMove(QEvent::MouseMove,end,end,Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent gapRelease(QEvent::MouseButtonRelease,end,end,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(&painter,&gapPress); QApplication::sendEvent(&painter,&gapMove); QApplication::sendEvent(&painter,&gapRelease);
    require(added==2 && completed==1,"one drag grows two adjacent roof tiles across empty space");
    painter.setTool(AreaPreview::Tool::Wall);
    bool upright=false;
    painter.onPaint=[&](const AreaPreviewFace& face) {
        upright=face.x==10 && face.y==10 && face.elevation==3 && face.plane==1;
    return true; };
    const auto wallClip=matrix*QVector4D(320,-430,96,1);
    const QPointF wallPoint((wallClip.x()/wallClip.w()+1)*300,(1-wallClip.y()/wallClip.w())*300);
    QMouseEvent wallHover(QEvent::MouseMove,wallPoint,wallPoint,Qt::NoButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(&painter,&wallHover);
    require(painter.hoveredFace() && painter.hoveredFace()->plane==1,"Wall tool hover reports upright preview before clicking");
    QMouseEvent wallPress(QEvent::MouseButtonPress,wallPoint,wallPoint,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent wallRelease(QEvent::MouseButtonRelease,wallPoint,wallPoint,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(&painter,&wallPress); QApplication::sendEvent(&painter,&wallRelease);
    require(upright && completed==2,"Wall tool click paints the upright preview in its own undo stroke");
    painter.onPaint={}; painter.onStrokeEnd={}; painter.hide();
    std::puts("PASS: mouse drag extends a roof across consecutive empty squares in one stroke");
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
static void surfaceWidgetChecks() {
    AreaPreview view;
    view.setAttribute(Qt::WA_DontShowOnScreen); view.resize(600,600);
    auto wall=quad();
    for(auto& v:wall) { const auto p=v.position; v.position={320+p.x()*20,432,112-p.y()*20}; }
    QImage white(2,2,QImage::Format_RGBA8888); white.fill(Qt::white);
    view.setScene(wall,white,true,{{10,10,0,3,1}}); view.show(); QApplication::processEvents();
    view.focusWorldPoint({352,432,112},96); view.setTool(AreaPreview::Tool::Paint);
    AreaPreviewCamera camera; camera.frame({256,336,64},{448,528,160},1);
    const auto matrix=camera.matrix(1);
    const auto pixel=[&](float x) {
        const auto p=matrix*QVector4D(x,-432,112,1);
        return QPointF((p.x()/p.w()+1)*300,(1-p.y()/p.w())*300);
    };
    const auto click=[&](QPointF point,Qt::KeyboardModifiers modifiers=Qt::NoModifier,Qt::MouseButton button=Qt::LeftButton) {
        QMouseEvent press(QEvent::MouseButtonPress,point,point,button,button,modifiers);
        QMouseEvent release(QEvent::MouseButtonRelease,point,point,button,Qt::NoButton,modifiers);
        QApplication::sendEvent(&view,&press); QApplication::sendEvent(&view,&release);
    };
    unsigned begins=0,paints=0,ends=0;
    QString reason;
    view.canPlace=[](AreaPreviewFace& f){return f.x>=10 && f.x<=12 && f.y==10;};
    view.onStrokeBegin=[&](bool){++begins;return true;}; view.onStrokeEnd=[&]{++ends;};
    view.onPaint=[&](const AreaPreviewFace& f){require(f.plane==1 && f.elevation==3,"surface painting never turns a wall into a floor");++paints;return true; };
    view.onPlacementHint=[&](const QString& message){reason=message;};
    click(pixel(320),Qt::ShiftModifier);
    require(view.lockedSurface() && view.lockedSurface()->plane==1 && begins==0 && paints==0,"Shift-click chooses the real wall without painting or an undo entry");
    click(pixel(384));
    require(paints==1 && ends==1,"locked surface paints a gap two tiles away");
    view.setTool(AreaPreview::Tool::Paint); view.setScene(wall,white,false,{{10,10,0,3,1}});
    require(view.lockedSurface()!=nullptr,"palette selection and live mesh refresh retain the chosen surface");
    click(pixel(384),Qt::NoModifier,Qt::RightButton);
    require(paints==1,"right-click on a hole in the locked wall does not erase other geometry");
    click(pixel(416));
    require(paints==1 && !reason.isEmpty(),"invalid locked target reports why and creates no map writes");
    QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier); QApplication::sendEvent(&view,&escape);
    require(!view.lockedSurface() && !view.pickingSurface(),"Escape releases the plane lock");
    view.pickSurface(); require(view.pickingSurface(),"Pick surface button arms the sampler");
    click(pixel(320)); require(view.lockedSurface() && !view.pickingSurface() && paints==1,"armed sampler consumes the click without painting");
    view.setScene(wall,white,true,{{10,10,0,3,1}});
    require(!view.lockedSurface(),"changing areas cannot carry over a stale surface");
    view.focusWorldPoint({352,432,112},96);
    auto ground=quad(); for(auto& v:ground) v.position=v.position*150+QVector3D(384,400,0);
    auto scene=wall; scene.insert(scene.end(),ground.begin(),ground.end());
    view.setScene(scene,white,false,{{10,10,0,3,1},{12,12,0,0,0}});
    const auto start=pixel(320),end=pixel(384);
    QMouseEvent press(QEvent::MouseButtonPress,start,start,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove,end,end,Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease,end,end,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    QApplication::sendEvent(&view,&press); QApplication::sendEvent(&view,&move); QApplication::sendEvent(&view,&release);
    require(paints==4 && !view.lockedSurface(),"automatic drag stays on the first wall across a gap with a floor behind it");
    view.onPaint={}; view.onStrokeBegin={}; view.onStrokeEnd={}; view.onPlacementHint={}; view.hide();
    std::puts("PASS: surface sampler, no sampling edits, locked gap painting, rejection, Escape, area reset and stable automatic drags");
}
static void directionWidgetChecks() {
    AreaPreview view; view.setAttribute(Qt::WA_DontShowOnScreen); view.resize(600,600);
    auto floor=quad(); for(auto& vertex:floor) vertex.position=vertex.position*20+QVector3D(320,416,96);
    view.setScene(floor,atlas(),true,{{10,10,1,3,0}}); view.show(); QApplication::processEvents();
    view.setTool(AreaPreview::Tool::Paint);
    unsigned painted=0,ended=0; int expected=0,previewPlane=-1,reportedPlane=-2; bool erase=false;
    view.placementSlots=[](int,int){return std::array<TilePlacementSlot,4>{};};
    view.onPlacementPlaneChanged=[&](int plane){reportedPlane=plane;};
    view.onStrokeBegin=[&](bool erasing){erase=erasing;return true;};
    view.onStrokeEnd=[&]{++ended;};
    view.onBrushPreview=[&](const AreaPreviewFace& face){previewPlane=face.plane;return floor;};
    view.onPaint=[&](const AreaPreviewFace& face) {
        require(face.x==10 && face.y==10 && face.layer==1 && face.elevation==3,"explicit tile direction keeps the pointed map square, layer and height");
        require(face.plane==(erase?0:expected),"placement obeys the selected direction; erasing still targets the original face");
        ++painted;
    return true; };
    const QPointF point=view.rect().center();
    const auto hover=[&] { QMouseEvent event(QEvent::MouseMove,point,point,Qt::NoButton,Qt::NoButton,Qt::NoModifier); QApplication::sendEvent(&view,&event); };
    const auto click=[&](Qt::MouseButton button=Qt::LeftButton) {
        QMouseEvent press(QEvent::MouseButtonPress,point,point,button,button,Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease,point,point,button,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(&view,&press); QApplication::sendEvent(&view,&release);
    };
    for(int plane=0;plane<7;++plane) {
        expected=plane; view.setPlacementPlane(plane); hover();
        require(reportedPlane==plane && view.hoveredFace() && view.hoveredFace()->plane==plane && previewPlane==plane,
                "direction selector, hovered tile and brush preview agree for every plane");
        click();
    }
    require(painted==7 && ended==7,"seven direction choices create seven complete strokes");
    const Qt::Key keys[]={Qt::Key_Down,Qt::Key_Up,Qt::Key_Left,Qt::Key_Right};
    for(int plane=0;plane<4;++plane) {
        QKeyEvent key(QEvent::KeyPress,keys[plane],Qt::NoModifier); QApplication::sendEvent(&view,&key);
        require(view.placementPlane()==plane && reportedPlane==plane,"arrow keys select flat/front/left/right and notify the selector");
    }
    view.setTool(AreaPreview::Tool::Paint);
    require(view.placementPlane()==3,"choosing new artwork does not discard explicit tile direction");
    view.placementSlots=[](int,int){return std::array<TilePlacementSlot,4>{{{}, {true,3,0}, {}}};};
    view.setTool(AreaPreview::Tool::Erase); click(); view.setTool(AreaPreview::Tool::Paint); require(painted==8,"erase works independently of direction override");
    view.setPlacementPlane(-1); hover(); require(view.hoveredFace()->plane==0,"Match surface restores automatic orientation");
    view.setPlacementPlane(6);
    QKeyEvent cycle(QEvent::KeyPress,Qt::Key_N,Qt::NoModifier); QApplication::sendEvent(&view,&cycle);
    require(view.placementPlane()==0,"N cycles all planes and wraps to flat");
    view.setTool(AreaPreview::Tool::Wall); require(view.placementPlane()==-1,"Wall creation tool clears the override instead of fighting it");
    view.onPaint={}; view.onStrokeEnd={}; view.onBrushPreview={}; view.onPlacementPlaneChanged={}; view.hide();
    std::puts("PASS: explicit direction matches preview and placement, arrow shortcuts, automatic reset and erase isolation");
}
static void placementWidgetChecks() {
    AreaPreview view; view.setAttribute(Qt::WA_DontShowOnScreen); view.resize(600,600);
    auto floor=quad(); for(auto& v:floor) v.position=v.position*20+QVector3D(320,416,320);
    // Same saved square, distinct world heights: water below a roof. The roof
    // is deliberately outside the mouse ray, just as in the reported image.
    auto roof=quad(); for(auto& v:roof) v.position=v.position*20+QVector3D(320,544,448);
    auto mesh=floor; mesh.insert(mesh.end(),roof.begin(),roof.end());
    std::vector<AreaPreviewFace> faces{{10,3,0,10,0},{10,3,1,14,0}};
    view.setScene(mesh,atlas(),true,faces); view.show(); QApplication::processEvents();
    view.focusWorldPoint({320,416,320},96);
    view.setTool(AreaPreview::Tool::Paint); view.setPaintLayer(1);
    std::array<TilePlacementSlot,4> slots{{{true,10,0},{true,14,0},{false,0,0}}};
    int writes[4]={},ends=0; bool erasing=false; QString hint;
    view.placementSlots=[&](int x,int y) {
        require(x==10 && y==3,"placement keeps the hovered square"); return slots;
    };
    view.onPlacementHint=[&](const QString& text){hint=text;};
    view.onStrokeBegin=[&](bool erase){erasing=erase;return true;}; view.onStrokeEnd=[&]{++ends;};
    view.onBrushPreview=[&](const AreaPreviewFace&){return floor;};
    view.onPaint=[&](const AreaPreviewFace& face) {
        require(face.layer!=1,"water and wall placement can never write into the roof slot");
        const auto& old=slots[face.layer];
        require(!old.occupied || (old.elevation==face.elevation && old.plane==face.plane),"write never repurposes unrelated geometry");
        ++writes[face.layer]; slots[face.layer]={!erasing,face.elevation,face.plane}; return true;
    };
    const QPointF point=view.rect().center();
    const auto hover=[&] { QMouseEvent event(QEvent::MouseMove,point,point,Qt::NoButton,Qt::NoButton,Qt::NoModifier); QApplication::sendEvent(&view,&event); };
    const auto click=[&](Qt::MouseButton button=Qt::LeftButton) {
        QMouseEvent press(QEvent::MouseButtonPress,point,point,button,button,Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease,point,point,button,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(&view,&press); QApplication::sendEvent(&view,&release);
    };
    hover();
    require(view.hoveredFace() && view.hoveredFace()->layer==0 && view.hoveredFace()->elevation==10 && hint.startsWith("REPAINT"),
            "pointing at water targets water, not the roof in the preferred layer");
    click(); require(writes[0]==1 && writes[1]==0,"repainting water preserves the roof");
    view.setPlacementPlane(1); hover();
    require(view.hoveredFace()->layer==2 && view.hoveredFace()->plane==1 && hint.startsWith("ADD"),
            "direction override is resolved before storage: wall gets the empty third slot");
    click(); require(writes[0]==1 && writes[1]==0 && writes[2]==1 && slots[0].plane==0 && slots[1].elevation==14,
            "adding an upright face leaves both water and roof intact");
    view.setPlacementPlane(2); hover();
    require(hint.contains("all 3 slots"),"a fourth distinct face is rejected with the real capacity reason");
    click(); require(writes[0]==1 && writes[1]==0 && writes[2]==1,"full storage never deletes another surface");
    view.setPlacementPlane(-1); view.setTool(AreaPreview::Tool::Erase); click(); view.setTool(AreaPreview::Tool::Paint);
    require(writes[0]==2 && !slots[0].occupied && slots[1].occupied && slots[2].occupied,"erase removes only the visible pointed surface");
    // Rejected writes must not become phantom geometry during a drag.
    view.onPaint=[](const AreaPreviewFace&){return false;};
    const auto before=view.mesh(); click();
    require(view.mesh().size()==before.size(),"failed writes do not mutate the rendered scene");
    require(ends==5,"each attempted gesture completes its stroke");
    // The identical saved-square conflict becomes paintable after upgrading
    // the map, without repainting any of its original three surfaces.
    slots[0]={true,10,0};
    view.setScene(mesh,atlas(),false,faces); view.setLayerCount(4);
    view.focusWorldPoint({320,416,320},96); view.setPlacementPlane(2);
    view.onPaint=[&](const AreaPreviewFace& face) {
        require(face.layer==3 && face.elevation==10 && face.plane==2,"upgraded preview targets fourth slot");
        ++writes[3]; slots[3]={true,face.elevation,face.plane}; return true;
    };
    hover(); require(hint.startsWith("ADD"),"fourth-layer map previews a valid addition");
    click(); require(writes[3]==1 && slots[0].plane==0 && slots[1].elevation==14 && slots[2].plane==1,
        "Qt mouse paint fills fourth slot and preserves all three other surfaces");
    view.onPaint={}; view.onStrokeEnd={}; view.onBrushPreview={}; view.onPlacementHint={}; view.placementSlots={}; view.hide();
    std::puts("PASS: real slot conflict through Qt preview/click: water repaint, wall addition, roof preservation, capacity rejection and erase");
}
// Saved Loren layout around (71,83). Nearby roofs share projected screen
// space with a missing LEFT wall. A direction override used to be applied only
// after picking: it extended the roof into full (71,82), then refused the wall.
static void savedWallGapChecks(bool widgets) {
    const QVector3D flat[4]={{-.5f,-.5f,0},{.5f,-.5f,0},{-.5f,.5f,0},{.5f,.5f,0}};
    const QVector3D left[4]={{-.5f,-.5f,1},{-.5f,.5f,1},{-.5f,-.5f,0},{-.5f,.5f,0}};
    std::vector<AreaPreviewFace> faces;
    for(int y=81;y<=84;++y) {
        if(y<=83) faces.push_back({70,y,0,13,0});
        faces.push_back({71,y,0,11,0});
        faces.push_back({71,y,1,11,2});
        if(y<83) faces.push_back({71,y,2,12,2});
        faces.push_back({72,y,0,11,0});
    }
    std::vector<AreaPreviewVertex> mesh;
    for(const auto& f:faces) for(int i:{0,1,2,2,1,3}) {
        const auto local=f.plane==2 ? left[i] : flat[i];
        mesh.push_back({local*32+QVector3D(f.x*32,(f.y+f.elevation)*32+f.layer*.001f,
            f.elevation*32+f.layer*.001f),{.5f,.5f},f.plane==2 ? QVector3D(-1,0,0) : QVector3D(0,0,1)});
    }
    QImage white(1,1,QImage::Format_RGBA8888); white.fill(Qt::white);
    const auto slotsAt=[&](int x,int y) {
        std::array<TilePlacementSlot,4> result{};
        for(const auto& f:faces) if(f.x==x && f.y==y) result[f.layer]={true,f.elevation,f.plane};
        return result;
    };
    const AreaPlacementCheck bounds=[](AreaPreviewFace& f){return f.x>=0 && f.y>=0 && f.x<150 && f.y<150;};
    const QVector3D gap(71*32-16,(83+12)*32,12*32+16);
    const QSize size(600,600);
    for(float yaw:{-65.f,-45.f,0.f,45.f}) for(float pitch:{30.f,60.f}) {
        AreaPreviewCamera camera; camera.target={gap.x(),-gap.y(),gap.z()};
        camera.distance=250; camera.yaw=yaw; camera.pitch=pitch;
        const auto target=pickAreaPaintTarget(mesh,faces,white,camera.matrix(1),{300,300},size,true,bounds,2);
        require(target && target->face.x==71 && target->face.y==83 && target->face.elevation==12 && target->face.plane==2,
            "Left direction picks the missing wall at 71,83 instead of a neighbouring roof extension");
        const auto destination=resolveTilePlacement(target->face,slotsAt(71,83),1);
        require(destination.action==TilePlacementAction::Add && destination.face.layer==2,
            "saved wall gap uses empty Layer 3 without a false capacity rejection");
        // Looking through a gap should still hit a solid foreground surface,
        // rather than allowing explicit direction to paint through that surface.
        camera.yaw=-135;
        const auto occluded=pickAreaPaintTarget(mesh,faces,white,camera.matrix(1),{300,300},size,true,bounds,2);
        require(occluded && occluded->existingIndex>=0 && occluded->face.plane==0,
            "roof occlusion prevents painting a wall hidden behind it");
    }
    if(widgets) {
        AreaPreview view; view.setAttribute(Qt::WA_DontShowOnScreen); view.resize(size);
        view.setScene(mesh,white,true,faces); view.show(); QApplication::processEvents();
        view.focusWorldPoint(gap,96); view.setTool(AreaPreview::Tool::Paint); view.setPaintLayer(1); view.setPlacementPlane(2);
        view.canPlace=bounds; view.placementSlots=slotsAt;
        unsigned writes=0; QString hint;
        view.onPlacementHint=[&](const QString& value){hint=value;};
        view.onStrokeBegin=[](bool erase){return !erase;};
        view.onPaint=[&](const AreaPreviewFace& f) {
            require(f.x==71 && f.y==83 && f.elevation==12 && f.plane==2 && f.layer==2,
                "actual gap click writes the same coordinate, height, direction and layer as its preview");
            ++writes; return true;
        };
        const QPointF point=view.rect().center();
        QMouseEvent hover(QEvent::MouseMove,point,point,Qt::NoButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(&view,&hover);
        require(view.hoveredFace() && view.hoveredFace()->x==71 && view.hoveredFace()->y==83 &&
                view.hoveredFace()->elevation==12 && view.hoveredFace()->layer==2 && hint.startsWith("ADD"),
            "mouse preview on the saved gap says ADD to Layer 3, not all slots full");
        QMouseEvent press(QEvent::MouseButtonPress,point,point,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease,point,point,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(&view,&press); QApplication::sendEvent(&view,&release);
        require(writes==1,"one click fills the saved wall gap");
        view.onPaint={}; view.onPlacementHint={}; view.placementSlots={}; view.hide();
    }
    std::puts("PASS: saved (71,83) wall gap, direction-guided ray target, eight camera angles, storage and foreground occlusion");
}

// Saved terrain around 108,74: the missing flat at 108,73 / height 11
// has water one step below it. Test the entire square, not a lucky centre pixel.
static void terrainGapChecks(bool widgets) {
    const std::vector<AreaPreviewFace> faces{
        {105,71,0,11,0},
        {106,71,1,11,0},
        {107,71,0,10,0},
        {107,71,1,10,2},
        {108,71,0,10,0},
        {109,71,1,12,0},
        {109,71,2,10,2},
        {109,71,3,11,2},
        {110,71,0,12,0},
        {105,72,0,11,0},
        {106,72,0,11,0},
        {107,72,0,10,0},
        {107,72,1,10,2},
        {108,72,0,10,0},
        {109,72,0,11,2},
        {109,72,1,11,1},
        {109,72,3,10,2},
        {110,72,0,12,0},
        {105,73,0,11,0},
        {106,73,0,11,0},
        {107,73,1,10,0},
        {107,73,3,10,2},
        {108,73,0,10,0},
        {109,73,1,10,1},
        {109,73,3,10,2},
        {110,73,1,12,0},
        {110,73,3,11,2},
        {105,74,0,11,0},
        {106,74,0,11,0},
        {107,74,0,11,0},
        {107,74,1,10,2},
        {107,74,3,10,1},
        {108,74,0,10,0},
        {108,74,1,11,0},
        {108,74,3,10,1},
        {109,74,0,10,0},
        {110,74,0,10,2},
        {110,74,1,11,1},
        {110,74,3,11,2},
        {105,75,0,11,0},
        {106,75,0,11,0},
        {107,75,0,11,0},
        {108,75,1,11,0},
        {109,75,0,10,0},
        {109,75,1,10,2},
        {110,75,1,10,1},
        {110,75,3,10,2},
        {105,76,0,11,0},
        {106,76,0,11,0},
        {107,76,0,11,0},
        {108,76,1,11,0},
        {109,76,0,10,0},
        {109,76,1,10,2},
        {110,76,0,10,0},
        {105,77,0,11,0},
        {106,77,0,11,0},
        {107,77,0,11,0},
        {108,77,1,11,0},
        {109,77,0,10,0},
        {109,77,1,10,2},
        {110,77,0,10,0},
    };
    const QVector3D planes[4][4]={
        {{-.5f,-.5f,0},{.5f,-.5f,0},{-.5f,.5f,0},{.5f,.5f,0}},
        {{-.5f,.5f,1},{.5f,.5f,1},{-.5f,.5f,0},{.5f,.5f,0}},
        {{-.5f,-.5f,1},{-.5f,.5f,1},{-.5f,-.5f,0},{-.5f,.5f,0}},
        {{.5f,.5f,1},{.5f,-.5f,1},{.5f,.5f,0},{.5f,-.5f,0}}
    };
    std::vector<AreaPreviewVertex> mesh;
    for(const auto& f:faces) for(int i:{0,1,2,2,1,3})
        mesh.push_back({planes[f.plane][i]*32+QVector3D(f.x*32,(f.y+f.elevation)*32+f.layer*.001f,
            f.elevation*32+f.layer*.001f),{.5f,.5f},{0,0,1}});
    QImage white(1,1,QImage::Format_RGBA8888); white.fill(Qt::white);
    const AreaPlacementCheck bounds=[](AreaPreviewFace& f){return f.x>=0 && f.y>=0 && f.x<150 && f.y<150;};
    const QVector3D gap(108*32,(73+11)*32,11*32+.001f);
    AreaPaintTarget anchor; anchor.face={108,74,1,11,0};
    const QSize size(600,600);
    for(float yaw:{-65.f,-45.f}) for(float pitch:{45.f,60.f}) for(int plane:{-1,0}) {
        AreaPreviewCamera camera; camera.target={gap.x(),-gap.y(),gap.z()};
        camera.distance=250; camera.yaw=yaw; camera.pitch=pitch;
        const auto matrix=camera.matrix(1);
        for(float dx:{-12.f,-6.f,0.f,6.f,12.f}) for(float dy:{-12.f,-6.f,0.f,6.f,12.f}) {
            const auto p=matrix*QVector4D(gap.x()+dx,-gap.y()-dy,gap.z(),1);
            const QPointF mouse((p.x()/p.w()+1)*300,(1-p.y()/p.w())*300);
            auto target=pickAreaPaintTarget(mesh,faces,white,matrix,mouse,size,true,bounds,plane,&anchor);
            const bool correct=target && target->existingIndex<0 && target->face.x==108 && target->face.y==73 &&
                               target->face.elevation==11 && target->face.plane==0;
            if(!correct) {
                std::fprintf(stderr,"Gap pick yaw=%g pitch=%g plane=%d offset=(%g,%g): ",yaw,pitch,plane,dx,dy);
                if(target) std::fprintf(stderr,"(%d,%d) height=%d plane=%d existing=%d\n",target->face.x,target->face.y,
                    target->face.elevation,target->face.plane,target->existingIndex);
                else std::fprintf(stderr,"no target\n");
            }
            require(correct,"terrain gap fills at surrounding ground height across the square, despite water below");
        }
    }
    // Continuity is local and may never bypass actual foreground geometry.
    AreaPreviewCamera camera; camera.target={gap.x(),-gap.y(),gap.z()}; camera.distance=250;
    const auto matrix=camera.matrix(1);
    const auto original=pickAreaPreviewFace(mesh,white,matrix,{300,300},size);
    require(original>=0,"terrain gap has a real background surface");
    auto erase=pickAreaPaintTarget(mesh,faces,white,matrix,{300,300},size,false,bounds,0,&anchor);
    require(erase && erase->existingIndex==original,"erase ignores the remembered surface and targets visible geometry");
    auto deleted=anchor; deleted.face.layer=2;
    auto stale=pickAreaPaintTarget(mesh,faces,white,matrix,{300,300},size,true,bounds,0,&deleted);
    require(stale && stale->existingIndex==original,"removed source tile cannot leave an invisible snapping plane");
    auto coveredMesh=mesh; auto coveredFaces=faces;
    for(int i:{0,1,2,2,1,3})
        coveredMesh.push_back({planes[0][i]*32+gap,{.5f,.5f},{0,0,1}});
    coveredFaces.push_back({108,73,2,11,0});
    auto filled=pickAreaPaintTarget(coveredMesh,coveredFaces,white,matrix,{300,300},size,true,bounds,0,&anchor);
    require(filled && filled->existingIndex==int(faces.size()),"filling a gap again repaints the real tile rather than adding a duplicate");
    // A foreground roof physically covering the pointer remains the target.
    auto roof=quad();
    for(auto& v:roof) { v.position=v.position*80+gap+QVector3D(0,0,32); coveredMesh.push_back(v); }
    coveredFaces.push_back({108,72,2,12,0});
    auto occluded=pickAreaPaintTarget(coveredMesh,coveredFaces,white,matrix,{300,300},size,true,bounds,0,&anchor);
    require(occluded && occluded->existingIndex==int(faces.size()+1),"remembered surface never paints through a foreground roof");
    if(widgets) {
        AreaPreview view; view.setAttribute(Qt::WA_DontShowOnScreen); view.resize(size);
        view.setScene(mesh,white,true,faces); view.show(); QApplication::processEvents();
        view.focusWorldPoint(gap,96); view.setTool(AreaPreview::Tool::Paint); view.setPaintLayer(1);
        view.setLayerCount(4); view.canPlace=bounds;
        view.placementSlots=[&](int x,int y) {
            std::array<TilePlacementSlot,4> slots{};
            for(const auto& f:faces) if(f.x==x && f.y==y) slots[f.layer]={true,f.elevation,f.plane};
            return slots;
        };
        unsigned writes=0;
        view.onStrokeBegin=[&](bool erase){
            // The real adapter refreshes the mesh before every stroke.
            view.setScene(mesh,white,false,faces); return !erase;
        };
        view.onPaint=[&](const AreaPreviewFace& f) {
            require(f.x==108 && f.y==73 && f.elevation==11 && f.plane==0 && f.layer==1,
                    "gap click adds ground to the free slot without repainting water");
            ++writes; return true;
        };
        // Hover the actual southern edge first, then move onto the gap.
        view.focusWorldPoint(gap+QVector3D(0,32,0),96);
        const QPointF point=view.rect().center();
        QMouseEvent hover(QEvent::MouseMove,point,point,Qt::NoButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(&view,&hover);
        // Shift the cursor using the camera's projection without changing it.
        AreaPreviewCamera widgetCamera;
        const auto source=gap+QVector3D(0,32,0);
        widgetCamera.frame(source-QVector3D(96,96,48),source+QVector3D(96,96,48),1);
        const auto p=widgetCamera.matrix(1)*QVector4D(gap.x(),-gap.y(),gap.z(),1);
        const QPointF hole((p.x()/p.w()+1)*300,(1-p.y()/p.w())*300);
        QMouseEvent move(QEvent::MouseMove,hole,hole,Qt::NoButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(&view,&move);
        QMouseEvent press(QEvent::MouseButtonPress,hole,hole,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease,hole,hole,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
        QApplication::sendEvent(&view,&press); QApplication::sendEvent(&view,&release);
        require(writes==1,"one ordinary click fills the terrain gap");
        view.onPaint={}; view.placementSlots={}; view.hide();
    }
    std::puts("PASS: saved terrain gap near 108,74, background water and 200 picks");
}

static void sampleWidgetChecks() {
    AreaPreview view; view.setAttribute(Qt::WA_DontShowOnScreen); view.resize(600,600);
    auto floor=quad(); for(auto& v:floor) v.position=v.position*20+QVector3D(320,416,96);
    view.setScene(floor,atlas(),true,{{10,10,1,3,0}}); view.show(); QApplication::processEvents();
    view.setTool(AreaPreview::Tool::Paint); view.setPaintLayer(2); view.setPlacementPlane(3);
    unsigned samples=0,begins=0,writes=0,ends=0;
    auto displayedTool=AreaPreview::Tool::Paint;
    view.onToolChanged=[&](AreaPreview::Tool tool){displayedTool=tool;};
    view.onSampleTile=[&](const AreaPreviewFace& f) {
        require(f.x==10 && f.y==10 && f.layer==1 && f.elevation==3 && f.plane==0,
                "right-click samples real visible geometry, ignoring selected layer and direction override");
        ++samples; view.setPlacementPlane(f.plane); return true;
    };
    view.onStrokeBegin=[&](bool){++begins; return true;};
    view.onPaint=[&](const AreaPreviewFace&){++writes; return true;};
    view.onStrokeEnd=[&]{++ends;};
    const QPointF point=view.rect().center();
    const auto click=[&](QPointF p,Qt::MouseButton button,Qt::KeyboardModifiers mods=Qt::NoModifier) {
        QMouseEvent press(QEvent::MouseButtonPress,p,p,button,button,mods);
        QMouseEvent release(QEvent::MouseButtonRelease,p,p,button,Qt::NoButton,mods);
        QApplication::sendEvent(&view,&press); QApplication::sendEvent(&view,&release);
    };
    click(point,Qt::LeftButton,Qt::ShiftModifier);
    require(view.lockedSurface(),"sampling fixture locks a surface");
    view.setTool(AreaPreview::Tool::Erase);
    require(displayedTool==AreaPreview::Tool::Erase,"eraser activation updates the brush panel state");
    click(point,Qt::RightButton);
    require(displayedTool==AreaPreview::Tool::Paint,"sampling restores the paint brush panel state");
    require(samples==1 && !begins && !writes && !ends && !view.lockedSurface(),
            "sampling clears old lock, selects Paint and never begins a map edit or Undo command");
    require(view.placementPlane()==0,"sampled facing becomes the paint direction");
    click({0,0},Qt::RightButton);
    require(samples==1 && !writes,"right-click empty space keeps brush and map unchanged");
    click(point,Qt::RightButton,Qt::AltModifier);
    require(samples==1 && !begins,"Alt-right pan does not sample or paint");
    click(point,Qt::LeftButton);
    require(begins==1 && writes==1 && ends==1,"left-click after sampling paints normally");
    view.onFacePreview=[&](const AreaPreviewFace&){return floor;};
    view.onBrushPreview=[&](const AreaPreviewFace&) {
        auto stamp=floor; auto next=floor; for(auto& v:next) v.position.setX(v.position.x()+32);
        stamp.insert(stamp.end(),next.begin(),next.end()); return stamp;
    };
    QString hint;
    view.onPlacementHint=[&](const QString& value){hint=value;};
    view.onPlacementError=[](const AreaPreviewFace&){return QString("Brush cannot fit: second square full");};
    click(point,Qt::LeftButton);
    require(writes==1 && hint.contains("second square"),"whole-brush conflict prevents even the anchor write and reports its reason");
    require(!view.grabFramebuffer().isNull(),"multi-tile rejected brush ghost renders safely");
    view.onPlacementError={}; click(point,Qt::LeftButton);
    require(writes==2,"full brush preview allows normal placement when every square fits");
    view.onToolChanged={}; view.onSampleTile={}; view.onPaint={}; view.onStrokeBegin={}; view.onStrokeEnd={};
    view.onFacePreview={}; view.onBrushPreview={}; view.onPlacementHint={}; view.hide();
    std::puts("PASS: right-click sampling, no edit/Undo, lock clearing, empty clicks, pan, sampled facing and full-brush preflight");
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
    snappingChecks();
    terrainGapChecks(app.arguments().contains("--widgets"));
    wallChecks();
    surfaceChecks();
    savedWallGapChecks(app.arguments().contains("--widgets"));
    if(app.arguments().contains("--widgets")) { sampleWidgetChecks(); surfaceWidgetChecks(); directionWidgetChecks(); placementWidgetChecks(); return widgetChecks(); }
    return renderChecks(format);
}
