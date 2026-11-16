/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef ENGINE_GL_SCENE_WIDGET_H
#define ENGINE_GL_SCENE_WIDGET_H

#include <ESToolkit/core/tk_asset_manager.h>
#include <QOpenGLWidget>
#include <QTimer>
#include <ESGamma/utility/elysian_performance_monitor.hpp>
#include <ESToolkit/ESGamma/tk_task_monitor.h>

class MapView;

/* The engine's OpenGL surface. It is installed as the MapView's viewport, so the
 * MapView paints it: MapView::paintEvent() calls renderFrame() inside
 * native painting. */
class EngineGLSceneWidget:
        public QOpenGLWidget,
        public AssetManager
{
    Q_OBJECT
public:
    explicit    EngineGLSceneWidget(QWidget* parent = nullptr);
    virtual     ~EngineGLSceneWidget(void) override;

    //Creates the GL context if it doesn't exist yet. Returns false if that failed.
    bool        ensureContext(void);
    void        startRenderTimer(void);
    //Updates and renders one engine frame into this widget's framebuffer.
    void        renderFrame(void);

    void        advanceFrame(void);
    void        play(void);
    void        stop(void);
    bool        isPlaying(void) const;
    bool        isCameraManual(void) const;
    void        setCameraManual(const bool val);
    bool        isCameraManualOnPlay(void) const;
    void        setCameraManualOnPlay(const bool val);
    void        setMapView(MapView* view);
    void        dumpPerfMonitor(void);
    bool        isEngineValid(void) const;
    void        setEngineValid(bool val);

    static void makeCurrentContext(void);
    static void doneCurrentContext(void);

protected:
    virtual void    initializeGL(void) override;
    virtual void    paintGL(void) override;

private Q_SLOTS:
    void            _onUpdateTimer(void);

Q_SIGNALS:
    void            engineUpdated(void);

private:
    void            _resizeIfNeeded(void);
    void            _forceOpaque(void);

    int                     _width              = 0;
    int                     _height             = 0;
    bool                    _advanceFrame       = false;
    bool                    _cameraManual       = true;
    bool                    _cameraManualOnPlay = false;
    MapView*                _mapView            = nullptr;
    bool                    _initialized        = false;
    bool                    _engineValid        = true;
    TaskMonitor             _engineTask;
    elysian::PerformanceMonitor _monitor = elysian::PerformanceMonitor("Engine Frame");
    QTimer                  _engineUpdateTimer;

    static EngineGLSceneWidget* _self;
};


// ===== INLINEZ =====

inline bool EngineGLSceneWidget::isCameraManual(void) const { return _cameraManual; }

inline void EngineGLSceneWidget::setCameraManual(const bool val) { _cameraManual = val; }

inline bool EngineGLSceneWidget::isCameraManualOnPlay(void) const { return _cameraManualOnPlay; }

inline void EngineGLSceneWidget::setMapView(MapView *view) { _mapView = view; }

inline bool EngineGLSceneWidget::isEngineValid(void) const { return _engineValid; }
inline void EngineGLSceneWidget::setEngineValid(bool val) {
    if(_engineValid != val) {
        setEnabled(val);
        _engineValid = val;
    }
}

#endif // ENGINE_GL_SCENE_WIDGET_H
