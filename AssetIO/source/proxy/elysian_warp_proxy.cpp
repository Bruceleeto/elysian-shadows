#include <AssetIO/proxy/elysian_warp_proxy.hpp>
/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>


/*!
    \qmltype Warp
    \inherits Component
    \since 1.5.8
    \inmodule Engine
    \brief Entity component for warping player from one location to another.

    Warp is a Component that can be attached to an Entity to move a player from one
    location to another upon collision. Warp requires an Entity to already have a
    Collidable present for collision before it can be attached.

    \sa Warps
*/


/*!
 * \qmlproperty Vector3 Warp::toPos
 *
 * \sa getToPosVec(), setToPosVec()
 */

/*!
 * \qmlproperty bool Warp::changeArea
 *
 * \sa getChangeArea(), setChangeArea()
 */

/*!
 * \qmlproperty bool Warp::changeLevel
 *
 * \sa getChangeLevel(), setChangeLevel()
 */

/*!
 * \qmlproperty bool Warp::obliqueTran
 *
 * \sa getObliqueTransition(), setObliqueTransition()
 */

/*!
 * \qmlproperty bool Warp::areaPath
 *
 * \sa getAreaPath(), setAreaPath()
 */

/*!
 * \qmlproperty bool Warp::levelPath
 *
 * \sa getLevelPath(), setLevelPath()
 */



/*!
 * \qmlmethod Vector3 Warp::getToPosVec()
 *
 * Returns a Vector3 object representing the destination location of the warp.
 * After the fade transition, when the warping is complete, this is where the
 * player is moved to. The Vector3 object returned here is actually modifying
 * the class itself, so there's no need to set it later on after changing the
 * object.
 */
/*!
 * \qmlmethod void Warp::setToPosVec(Vector3& pos)
 *
 * Sets the destination location of the warp.
 */

/*!
 * \qmlmethod bool Warp::getObliqueTransition()
 *
 * Returns whether or not the warp uses an oblique transition during the fade-out.
 * The oblique-transition is a 3D effect that rotates the map down along the X axis
 * towards the character in 3D space. It looks really good for entering doors. But
 * depending on how the level is designed, not every level will look good in 3D.
 */
/*!
 * \qmlmethod void Warp::setObliqueTransition(bool use)
 *
 * Sets the warp to use the oblique transition effect.
 */


/*!
 * \qmlmethod string Warp::getAreaPath()
 *
 * Returns the asset-relative path for the destination area when warping the player to another area.
 *
 */
/*!
 * \qmlmethod void Warp::setAreaPath(string path)
 *
 * Sets the asset-relative path for the destination area of the warp.
 * The "changeArea" property must also be set to true when using this field.
 */


/*!
 * \qmlmethod bool Warp::getLevelPath()
 *
 * Sets the asset-relative path for the destination Level when warping the player to another level.
 *
 */
/*!
 * \qmlmethod void Warp::setLevelPath(string path)
 * Sets the asset-relative path for the destination level of the warp.
 * The "changeLevel" property must also be set to true when using this field AND the corresponding
 * "areaPath" and "changeArea" properties must be set accordingly,
 */


/*!
 * \qmlmethod bool Warp::getChangeArea()
 *
 * Returns whether or not the warp leads to another area.
 */
/*!
 * \qmlmethod void Warp::setChangeArea(bool use)
 *
 * Sets the warp to swap to another area. If this is true, the "areaPath" property must also be provided.
 */


/*!
 * \qmlmethod bool Warp::getChangeLevel()
 *
 * Returns whether or not the warp leads to another level.
 */
/*!
 * \qmlmethod void Warp::setChangeLevel(bool use)
 *
 * Sets the warp to swap to another level. If this is true, the "levelPath" property must also be provided,
 * along with the "changeArea" flag being set to true with a corresponding "areaPath."
 */



namespace elysian::lua {
using namespace elysian;
using Warp = elysian::Warp;

void registerWarp(sol::state_view state) {
    if (state["Warp"].valid()) return;
    registerComponent(state);
    using T = elysian::Warp;
    auto type = registerType<T, elysian::Component>(state, "Warp");
    type["getCameraBoundaryRect"] = method<T>(&T::getCameraBoundaryRect);
    type["setCameraBoundaryRect"] = method<T>(&T::setCameraBoundaryRect);
    type["getToPosVec"] = method<T, Vector3()>(&T::getToPosVec);
    type["setToPosVec"] = method<T, void(const Vector3&)>(&T::setToPosVec);
    type["setAreaPath"] = method<T, void(const char*)>(&T::setAreaPath);
    type["getAreaPath"] = method<T, const char*() const>(&T::getAreaPath);
    type["setLevelPath"] = method<T, void(const char*)>(&T::setLevelPath);
    type["getLevelPath"] = method<T, const char*() const>(&T::getLevelPath);
    type["setChangeArea"] = method<T, void(bool)>(&T::setChangeArea);
    type["getChangeArea"] = method<T, bool() const>(&T::getChangeArea);
    type["setChangeLevel"] = method<T, void(bool)>(&T::setChangeLevel);
    type["getChangeLevel"] = method<T, bool() const>(&T::getChangeLevel);
    type["setObliqueTransition"] = method<T, void(bool)>(&T::setObliqueTransition);
    type["getObliqueTransition"] = method<T, bool() const>(&T::getObliqueTransition);
    property(type, "toPos", method<T, Vector3()>(&T::getToPosVec), method<T, void(const Vector3&)>(&T::setToPosVec));
    property(type, "changeArea", method<T, bool() const>(&T::getChangeArea), method<T, void(bool)>(&T::setChangeArea));
    property(type, "changeLevel", method<T, bool() const>(&T::getChangeLevel), method<T, void(bool)>(&T::setChangeLevel));
    property(type, "areaPath", method<T, const char*() const>(&T::getAreaPath), method<T, void(const char*)>(&T::setAreaPath));
    property(type, "levelPath", method<T, const char*() const>(&T::getLevelPath), method<T, void(const char*)>(&T::setLevelPath));
    property(type, "obliqueTran", method<T, bool() const>(&T::getObliqueTransition), method<T, void(bool)>(&T::setObliqueTransition));
    property(type, "cameraBoundaryRect", method<T>(&T::getCameraBoundaryRect), method<T>(&T::setCameraBoundaryRect));
    inheritProperties<T, elysian::Component>();
    finish<T>(type, "Warp", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}
}
