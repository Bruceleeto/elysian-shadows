/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESToolkit/ESGamma/tk_camera_system.h>
#include <ESToolkit/core/tk_assets.h>

TkCameraSystem::TkCameraSystem(void) {

    //Make sure the rest of the stack is initialized, don't give a shit when we're aren't using EStk.
    for(unsigned i = 1; i < ELYSIAN_CAMERA_SYSTEM_STACK_SIZE; ++i) {
        _states[i] = _states[0];
        _states[i]->updateViewVolumeBoundingRegion();
    }

    connect(getAssets(), SIGNAL(areaChanging(Area*const)), this, SLOT(onAreaChanging(Area*const)));

}

void TkCameraSystem::setCurrentIndex(const unsigned index) {

    elysian::CameraSystemBase::setCurrentIndex(index);
    Q_EMIT cameraIndexChanged(index);
}

void TkCameraSystem::onAreaChanging(Area *const area) {
    // Area keyframes belong to the normal 2D camera. The debug camera is
    // temporarily pushed above it; loading a default area frame into that
    // camera replaces its viewing direction with zero and collapses the view.
    auto* areaCamera = isDebug() && _depth > 0 ? _states[_depth-1] : _curState;
    auto& frame = *static_cast<elysian::CameraKeyFrame*>(areaCamera);
    if(_curArea) {
        _curArea->setCameraKeyFrame(frame);
    }
    if(area) {
        frame = area->getCameraKeyFrame();
    }
}
