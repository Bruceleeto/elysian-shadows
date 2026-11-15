#include <AssetIO/proxy/elysian_renderable_proxy.hpp>
/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>

/*!
    \qmltype Renderable
    \inherits Component
    \since 1.6.0
    \inmodule Engine
    \brief Component attached to an Entity to make it a sprite or quad.

    \l{Renderable} is a \l{Component} that can be attached to an \l{Entity}, allowing
    it to be rendered as a textured or untextured sprite or quad with an optional
    normal map, specular map, and \l{Material}. This class should be the preferred method
    of rendering for an \l{Entity}, as it's fast, lightweight, and requires minimal
    vertex data. For more complex geometry, see the \l{Skeleton} Component.

    The Renderable Component's texture can be a standard image or a uniformly-spaced
    sprite sheet, which it can split into frames automatically. The sprite index can be set
    through \a{frame} and can be modified at run-time for simple animations.

    Renderable can also use arbitrary texture coordinates to draw any region of
    a texture for special effects or animation. To do this, \l{frame} must be set to
    -1, then custom UV coords can be set through the \l{uvCoords} property. The default
    texture wrap setting is repeat, so you can tile a sprite by simply oversampling to
    do special effects like a waterfall by looping over UV coordinates.

    Multiple Renderables using the same sprite sheets, sprites, and vertex colors
    should share the same SpriteAttrib or QuadSpriteAttrib objects as their
    \l{spriteAttrib} properties to prevent duplicate objects in memory and make it
    easier to manage groups of similar Entities.

    \sa SpriteAttrib, QuadSpriteAttrib, Material
*/
/*!
\qmlproperty int Renderable::frame

If the current texture is a sprite sheet, frame is the sheet index of the current sprite.
If no texture is loaded, frame is ignored. If a texture is present and frame is equal to
-1, texture coordinates are fetched from the \l{uvCoords} property.
\sa getFrame(), setFrame()
*/
/*!
\qmlproperty TEX_ORIENT_TYPE Renderable::texOrient

Enumeration representing any horizontal or vertical flipping or 90 degree rotations applied
to the sprite.
\sa gyro::TEX_ORIENT_TYPE, getTexOrient(), setTexOrient()
*/
/*!
\qmlproperty SpriteAttrib* Renderable::spriteAttrib

Reference to a SpriteAttrib OR QuadSpriteAttrib object to be used by the Renderable when
rendering or nil if neither has been specified.
\sa getSpriteAttrib(), getQuadSpriteAttrib(), setSpriteAttrib()
*/
/*!
\qmlproperty Vector2 Renderable::offset

Offset of the sprite's position relative to the Entity's position. The default value is
<0, 0> which places the sprite exactly at the Entity's position.
\sa getOffsetVec(), setOffsetVec()
*/
/*!
\qmlproperty array Renderable::uvCoords

4-element array (indices 1-4) of Vector2 objects used as custom UV coordinates when
rendering the sprite. The \l{frame} property must be set to -1 for these values to
be used, because they aren't by default.
\sa getUVCoords(), setUVCoords(), getUVCoordsCount()
*/
/*!
 * \qmlmethod int Renderable::getFrame()
 *
 * Returns the value of the \l{frame} property.
 * \sa frame, setFrame()
 */
/*!
 * \qmlmethod void Renderable::setFrame(int fr)
 *
 * Sets the value of the \l{frame} property to \a{fr}.
 * \sa frame, getFrame()
 */
/*!
 * \qmlmethod TEX_ORIENT_TYPE Renderable::getTexOrient()
 *
 * Returns the value of the \l{texOrient} property.
 * \sa gyro::TEX_ORIENT_TYPE, texOrient, setTexOrient()
 */
/*!
 * \qmlmethod void Renderable::setTexOrient(TEX_ORIENT_TYPE orient)
 *
 * Sets the value of the \l{texOrient} property to \a{orient}.
 * \sa gyro::TEX_ORIENT_TYPE, texOrient, getTexOrient()
 */
/*!
 * \qmlmethod SpriteAttrib* Renderable::getSpriteAttrib()
 *
 * Returns the value of the \l{spriteAttrib} property.
 * This works fine whether \{spriteAttrib} is of type SpriteAttrib or
 * QuadSpriteAttrib, since QuadSpriteAttrib inherits from SpriteAttrib.
 * \sa spriteAttrib, setSpriteAttrib(), getQuadSpriteAttrib
 */
/*!
 * \qmlmethod QuadSpriteAttrib* Renderable::getQuadSpriteAttrib()
 *
 * Returns the value of the \l{spriteAttrib} property as type QuadSpriteAttrib.
 * \e{WARNING: This is only valid if \l{spriteAttrib} is of type QuadSpriteAttrib
 * or nil. Calling this function with \l{spriteAttrib} of type SpriteAttrib will
 * cause undefined behavior (ie fucking crash).}
 * \sa spriteAttrib, setSpriteAttrib(), getSpriteAttrib()
 */
/*!
 * \qmlmethod void Renderable::setSpriteAttrib(SpriteAttrib* attrib)
 *
 * Sets the value of the \l{spriteAttrib} property to \a{attrib}.
 * \a{attrib} can be of type SpriteAttrib, QuadSpriteAttrib, or nil.
 * If it's not nil, ownership is transferred from Lua to the engine.
 * \sa spriteAttrib, getSpriteAttrib(), getQuadSpriteAttrib()
 */
/*!
 * \qmlmethod Vector2 Renderable::getOffsetVec()
 *
 * Returns the value of the \l{offset} property.
 * \sa offset, setOffsetVec()
 */
/*!
 * \qmlmethod void Renderable::setOffsetVec(Vector2 vec)
 *
 * Sets the value of the \l{offset} property to \a{vec}.
 * \sa offset, getOffsetVec()
 */
/*!
 * \qmlmethod Vector2 Renderable::getUVCoords(int index)
 *
 * Returns the value of the \l{uvCoords} property at the given \a{index}.
 * Valid values for \a{index} are 1-4.
 * \sa uvCoords, setUVCoords(), getUVCoordsCount()
 */
/*!
 * \qmlmethod void Renderable::setUVCoords(int index, Vector2 coords)
 *
 * Sets the value of the \l{uvCoords} property at the given \a{index} to \a{coords}.
 * Valid values for \a{index} are 1-4.
 * \sa uvCoords, getUVCoords(), getUVCoordsCount()
 */
/*!
 * \qmlmethod int Renderable::getUVCoordsCount()
 *
 * Method used internally for retrieving the length of the \l{uvCoords} array.
 * This should always return 4.
 * \sa uvCoords, getUVCoords(), setUVCoords()
 */
/*!
 * \qmlmethod void Renderable::setTexCoord(int index, float u, float v)
 *
 * Sets the value of the \l{uvCoords} property at the given \a{index} to the
 * 2D vector created by <\a{u}, \a{v}>. Valid values for \a{index} are 1-4.
 * \sa uvCoords, getUVCoords(), setUVCoords(), getUVCoordsCount()
 */



namespace elysian::lua {
using namespace elysian;
using Renderable = elysian::Renderable;

void registerRenderable(sol::state_view state) {
    if (state["Renderable"].valid()) return;
    registerComponent(state);
    using T = elysian::Renderable;
    auto type = registerType<T, elysian::Component>(state, "Renderable");
    type["getOffsetVec"] = method<T, Vector2()>(&T::getOffsetVec);
    type["setOffsetVec"] = method<T, void(const Vector2&)>(&T::setOffsetVec);
    type["getFrame"] = method<T, unsigned() const>(&T::getFrame);
    type["setFrame"] = method<T, void(unsigned)>(&T::setFrame);
    type["getTexOrient"] = method<T, int() const>(&T::getTexOrient);
    type["setTexOrient"] = method<T, void(GY_RECT_TEX_ORIENT)>(&T::setTexOrient);
    type["getSpriteAttrib"] = method<T, std::shared_ptr<SpriteAttrib>() const>(&T::getSpriteAttrib);
    type["getQuadSpriteAttrib"] = method<T, std::shared_ptr<QuadSpriteAttrib>() const>(&T::getQuadSpriteAttrib);
    type["setSpriteAttrib"] = method<T, void(std::shared_ptr<SpriteAttrib>)>(&T::setSpriteAttrib);
    type["setTexCoord"] = method<T, void(const unsigned, const float, const float)>(&T::setTexCoord);
    type["getUVCoordsCount"] = method<T, unsigned() const>(&T::getUVCoordsCount);
    type["getUVCoords"] = method<T, Vector2(const unsigned)>(&T::getUVCoords);
    type["setUVCoords"] = method<T, void(const unsigned, const Vector2&)>(&T::setUVCoords);
    property(type, "frame", method<T, unsigned() const>(&T::getFrame), method<T, void(unsigned)>(&T::setFrame));
    property(type, "texOrient", method<T, int() const>(&T::getTexOrient), method<T, void(GY_RECT_TEX_ORIENT)>(&T::setTexOrient));
    property(type, "spriteAttrib", method<T, std::shared_ptr<SpriteAttrib>() const>(&T::getSpriteAttrib), method<T, void(std::shared_ptr<SpriteAttrib>)>(&T::setSpriteAttrib));
    property(type, "offset", method<T, Vector2()>(&T::getOffsetVec), method<T, void(const Vector2&)>(&T::setOffsetVec));
    viewProperty(type, array<T>("uvCoords", method<T, Vector2(const unsigned)>(&T::getUVCoords), method<T, void(const unsigned, const Vector2&)>(&T::setUVCoords), method<T, unsigned() const>(&T::getUVCoordsCount)));
    inheritProperties<T, elysian::Component>();
    finish<T>(type, "Renderable", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}
}
