/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESGamma/crtp/elysian_camera_system.hpp>
#include <libGyro/gyro_video_api.h>
#include <ESToolkit/ui/widget/tk_engine_gl_scene_widget.h>
#include <gl/gyro_video_gl.h>
#include <ESGamma/system/elysian_engine.hpp>
#include <ESGamma/crtp/elysian_timer_system.hpp>
#include <ESToolkit/ui/widget/tk_map_view.h>
#include <QOpenGLContext>
#include <QMessageBox>

EngineGLSceneWidget* EngineGLSceneWidget::_self = nullptr;

void EngineGLSceneWidget::makeCurrentContext(void) {
    if(_self && _self->context() && QOpenGLContext::currentContext() != _self->context()) {
        _self->makeCurrent();
    }
}

void EngineGLSceneWidget::doneCurrentContext(void) {
    if(_self && _self->context() && QOpenGLContext::currentContext() == _self->context()) {
        _self->doneCurrent();
    }
}

EngineGLSceneWidget::EngineGLSceneWidget(QWidget *parent):
    QOpenGLWidget(parent),
    _engineTask("ESGamma")
{
    assert(!_self);
    _self = this;

    connect(this, &EngineGLSceneWidget::engineUpdated, getAssets(), &Assets::engineUpdated);
    connect(&_engineUpdateTimer, &QTimer::timeout, this, &EngineGLSceneWidget::_onUpdateTimer);

    //Render at ~60fps. A 0ms timer re-renders back-to-back on the UI thread and
    //starves input, leaving the editor unresponsive.
    _engineUpdateTimer.setInterval(16);
}

EngineGLSceneWidget::~EngineGLSceneWidget(void) {
    if(_self == this) _self = nullptr;
}

bool EngineGLSceneWidget::ensureContext(void) {
    //QOpenGLWidget only creates its context when it is first painted or grabbed.
    //The engine needs it before then (GLEW, shaders, textures), so grab once to force it.
    if(!context()) grabFramebuffer();
    return context() && context()->isValid();
}

void EngineGLSceneWidget::initializeGL(void) {
    DbgEvent event("EngineGLSceneWidget::initializeGL()");
    if(getAssets()->getEngine()) getAssets()->getEngine()->getCameraSys()->initializeProjMatrix();
}

void EngineGLSceneWidget::startRenderTimer(void) {
    makeCurrent();
    _resizeIfNeeded();
    _initialized = true;
    _engineUpdateTimer.start();
    update();
}

void EngineGLSceneWidget::_onUpdateTimer(void) {
    //Schedules a repaint of the map view, which renders the engine (see MapView::paintEvent()).
    update();
}

void EngineGLSceneWidget::_resizeIfNeeded(void) {
    if(!_engineValid) return;

    const int w = width();
    const int h = height();
    if(w == _width && h == _height) return;

    const qreal dpr = devicePixelRatioF();
    _engineValid = glalVidOglInit((int)(w*dpr), (int)(h*dpr), w, h);
    _width  = w;
    _height = h;

    if(!_engineValid) {
        QMessageBox::critical(nullptr, "Error!", "The libGyro Video API back-end (OpenGL) failed to initialize properly! Something is FUNDAMENTALLY fucked. Don't blame me, blame OpenGL.");
        return;
    }

    if(getAssets()->getEngine()) getAssets()->getEngine()->getCameraSys()->initializeProjMatrix();
}

void EngineGLSceneWidget::paintGL(void) {
    //Only reached when the widget is painted on its own (e.g. grabFramebuffer());
    //as the MapView's viewport it is painted through MapView::paintEvent().
    renderFrame();
}

void EngineGLSceneWidget::renderFrame(void) {
    if(!getCurrentArea() || !_initialized || !_engineValid) return;

    _resizeIfNeeded();
    if(!_engineValid) return;

    //Qt's painter draws the view's items with this same context between frames.
    glalVidRestoreState();

    auto* engine = getAssets()->getEngine();
    engine->getTimerSys()->pushTaskMonitor(&_engineTask);

    _monitor.start();
    engine->processInput();
    engine->update();
    engine->render();
    _monitor.stop();

    if(_advanceFrame) {
        _advanceFrame = false;
        engine->setEngineState(elysian::System::ENGINE_ESTK_STOP);
    }

    auto* camera = engine->getCameraSys();
    camera->setFollowEnabled(!_cameraManual);
    camera->setDynamic(!_cameraManual);
    camera->setBoundByMap(!_cameraManual);

    engine->getTimerSys()->popTaskMonitor(&_engineTask);

    Q_EMIT engineUpdated();
}

void EngineGLSceneWidget::stop(void) {
    getAssets()->getEngine()->setEngineState(elysian::Engine::ENGINE_ESTK_STOP);
    _cameraManual = true;
}

void EngineGLSceneWidget::play(void) {
    getAssets()->getEngine()->setEngineState(elysian::Engine::ENGINE_NORMAL);
    _cameraManual = _cameraManualOnPlay;
}

void EngineGLSceneWidget::advanceFrame(void) {
    _advanceFrame = true;
    getAssets()->getEngine()->setEngineState(elysian::Engine::ENGINE_NORMAL);
}

bool EngineGLSceneWidget::isPlaying(void) const {
    return getAssets()->getEngine()->getEngineState() != elysian::System::ENGINE_ESTK_STOP;
}

void EngineGLSceneWidget::setCameraManualOnPlay(const bool val) {
    _cameraManualOnPlay = val;

    if(getAssets()->getEngine()->getEngineState() != elysian::Engine::ENGINE_ESTK_STOP) {
        _cameraManual = val;
    }
}

void EngineGLSceneWidget::dumpPerfMonitor(void) {
    static bool once = false;
    if(!once) {
        _monitor.addChild(&getAssets()->getEngine()->updateMonitor);
        _monitor.addChild(&getAssets()->getEngine()->renderMonitor);
        _monitor.addChild(&getAssets()->getEngine()->processInputMonitor);
        once = true;
    }
    elysian::PerformanceMonitor::print(&_monitor);
}
