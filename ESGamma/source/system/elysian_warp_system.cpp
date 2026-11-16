/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <ESGamma/system/elysian_warp_system.hpp>
#include <ESGamma/crtp/elysian_camera_system.hpp>
#include <AssetIO/crtp/elysian_warp.hpp>
#include <ESGamma/system/elysian_party_system.hpp>
#include <ESGamma/crtp/elysian_debug.hpp>
#include <AssetIO/crtp/elysian_collidable.hpp>
#include <ESGamma/crtp/elysian_timer_system.hpp>
#include <ESGamma/crtp/elysian_level_system.hpp>
#include <AssetIO/primitive/elysian_instance_transform.hpp>
#include <AssetIO/crtp/elysian_entity.hpp>
#include <ESGamma/system/elysian_collision_system.hpp>
#include <ESGamma/crtp/elysian_gui_system.hpp>
#include <AssetIO/level/elysian_zcoords.hpp>
#include <AssetIO/crtp/elysian_area.hpp>

namespace elysian {

WarpSystem::WarpSystem(void): 
	_currentWarp()
{
    _screenRect._transform._pos.z = ZCoordConstants::warpOverlay;
	_screenRect.setColor(0, 0, 0, 0);
    _screenRect.setBlendMode(BlendMode(GY_BLEND_SRC_ALPHA, GY_BLEND_ONE_MINUS_SRC_ALPHA));
}

bool WarpSystem::warpTo(const char* levelPath, const char* areaPath) {
    if(!_enabled || _state != CHECK_WARP || !levelPath || !areaPath ||
       !*levelPath || !*areaPath ||
       strlen(levelPath) >= sizeof(_currentWarp._levelPath) ||
       strlen(areaPath) >= sizeof(_currentWarp._areaPath)) return false;
#ifndef ELYSIAN_ENGINE
    // The editor creates all levels and areas when it opens the project.
    auto* level = getLevelSys()->findLevel(levelPath);
    auto* area = getLevelSys()->findArea(areaPath);
    if(!level || !area || area->getLevel() != level) return false;
#endif
    _currentWarp.setLevelPath(levelPath);
    _currentWarp.setAreaPath(areaPath);
    _currentWarp.setChangeLevel(true);
    _currentWarp.setChangeArea(true);
    _currentWarp.setObliqueTransition(false);
    _currentWarp.setCameraBoundaryRect(Recti(-1)); // Use the destination area's bounds.
    _useAreaStartPosition = true;
    _timer = 0.0f;
    _state = FADE_OUT;
    setEngineState(System::ENGINE_WARPING);
    return true;
}

void WarpSystem::initialize(void) {
	getCollisionSys()->registerContactEventMonitor(ELYSIAN_COLLISION_CONTACT_EVENT_MONITOR_WARP_MASK, this);

}

void WarpSystem::render(void) {
    if(_timer > 0.0f) _screenRect.render();

}

void WarpSystem::update(void) {
    unsigned width, height;

	//Update overlay size to match the current screen resolution
	getGuiSys()->getResolution(width, height);
    _screenRect._transform.setPos(width/2.0f, height/2.0f);
    _screenRect._transform.setSize(width, height);
	
	switch(_state) {
		case CHECK_WARP:
			UpdateCheckWarp();
			break;
		case FADE_OUT:
			UpdateFadeOut();
			break;
        case WARP: {
                if(_currentWarp._changeLevel) getLevelSys()->switchLevel(_currentWarp._levelPath);
                if(_currentWarp._changeArea) getLevelSys()->switchArea(_currentWarp._areaPath);
                _state = FADE_IN;
                auto&rect = _currentWarp.getCameraBoundaryRect();
                getCameraSys()->setBoundaries(rect.top, rect.bottom, rect.left, rect.right);
                if(_currentWarp._obliqueTran) {
                    CameraKeyFrame fr = getCameraSys()->getKeyFrame();
                    fr._orient.x = 0.0f;
                    fr.setScale(1.0f, 1.0f, 1.0f);
                    getCameraSys()->moveToKeyFrame(fr, 1.0f);
                }
            }
			break;
		case FADE_IN:
			UpdateFadeIn();
			break;
		case PAUSE:
			UpdatePause();
			break;
		default:
			break;
	}
}

void WarpSystem::UpdateFadeOut(void) {
	if(_timer < 1.0f) {
		_timer += getTimerSys()->getDeltaTime();
		if(_timer > 1.0f) _timer = 1.0f;
		_screenRect.setColor(0.0f, 0.0f, 0.0f, _timer*255);
	}
	else {
		_timer = 1.0f;
		_state = WARP;
	}
}

void WarpSystem::UpdateFadeIn(void) {
	if(_timer > 0.0f) {
		_timer -= 1.0f*getTimerSys()->getDeltaTime();
		if(_timer < 0.0f) _timer = 0.0f;
		_screenRect.setColor(0.0f, 0.0f, 0.0f, _timer*255);
	}
	else {
		_timer = 0.0f;
		_state = PAUSE;
	}
}

void WarpSystem::UpdatePause(void) {
    setEngineState(System::ENGINE_NORMAL);
    auto* leader = getPartySys()->getPartyLeader();
    auto* playerCollider = leader ? leader->getCollidable() : nullptr;
    if(!playerCollider || !_curArea || !getPartySys()->isPartyVisible()) {
        _state = CHECK_WARP;
        return;
    }
	for(unsigned int i = 0; i < _curArea->warps.size(); ++i) {
        auto* entity = _curArea->warps[i]->getEntity();
        auto* collider = entity ? entity->getCollidable() : nullptr;
        if(collider && getCollisionSys()->checkCollision(*playerCollider, *collider)) {
			return;
		}
	}      
	//Debug::printf(Debug::LOG_VERBOSE, "CHANGING STATE!");
	_state = CHECK_WARP;

}

void WarpSystem::UpdateCheckWarp(void) {
	//Check for warp
	//Debug::printf(Debug::LOG_VERBOSE, "WARP CHECK");
    if(!_enabled || !_curArea || !getPartySys()->isPartyVisible()) return;
    auto* leader = getPartySys()->getPartyLeader();
    auto* playerCollider = leader ? leader->getCollidable() : nullptr;
    if(!playerCollider) return;

	for(unsigned int i = 0; i < _curArea->warps.size(); ++i) {
        auto* entity = _curArea->warps[i]->getEntity();
        auto* collider = entity ? entity->getCollidable() : nullptr;
        if(collider && getCollisionSys()->checkCollision(*playerCollider, *collider)) {
			//Debug::printf(Debug::LOG_VERBOSE, "Warp Collision! %s", _curArea->warps[i]->getEntity()->getName());
			_currentWarp = *_curArea->warps[i];
            _useAreaStartPosition = false;
            _timer = 0.0f;
			_state = FADE_OUT;
			setEngineState(System::ENGINE_WARPING);
			if(_currentWarp._obliqueTran) {
				CameraKeyFrame fr = getCameraSys()->getKeyFrame();
                fr._orient.x =-0.25*M_PI;
                fr.setScale(2.0f, 2.0f, 2.0f);
				getCameraSys()->moveToKeyFrame(fr, 1.0f);
			}
		} 
	}
}

void WarpSystem::onCollisionEnter(Collidable*const /*bodyA*/, Collidable*const /*bodyB*/, CollisionContact* /*contact*/) {
#if 0
	if(_state != CHECK_WARP) return;

	if(getPartySys()->getPartyLeader() && getPartySys()->getPartyLeader() == bodyB->getEntity()) {
		_currentWarp = *bodyA->getEntity()->getWarp();
		_state = FADE_OUT;
		setEngineState(System::ENGINE_WARPING);
	}
#endif
}

void WarpSystem::onCollide(Collidable*const /*bodyA*/, Collidable*const /*bodyB*/, CollisionContact* /*contact*/) {

}

void WarpSystem::onCollisionExit(Collidable*const /*bodyA*/, Collidable*const /*bodyB*/) {

}

void WarpSystem::setRenderWarps(const bool flag) { 
	_renderWarps = flag;
}

bool WarpSystem::getRenderWarps(void) const {
	return _renderWarps;
}

void WarpSystem::Disable(void) { _enabled = false; }
void WarpSystem::Enable(void) { _enabled = true; }

}
