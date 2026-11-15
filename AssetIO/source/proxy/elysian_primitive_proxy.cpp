/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_primitive_proxy.hpp>

/*!
    \qmltype Color4
    \since 1.6.0
    \inmodule Engine
    \brief 32-Bit Color in RGBA format.

    Color4 is a 32-bit color in RGBA format, with 8 bits for each channel.
*/
/*!
\qmlproperty byte Color4::r

Value of the 8-bit red channel. 0 is none, 255 is full.
\sa getR(), setR()
*/
/*!
\qmlproperty byte Color4::g

Value of the 8-bit green channel. 0 is none, 255 is full.
\sa getG(), setG()
*/
/*!
\qmlproperty byte Color4::b

Value of the 8-bit blue channel. 0 is none, 255 is full.
\sa getB(), setB()
*/
/*!
\qmlproperty byte Color4::a

Value of the 8-bit alpha channel. 0 fully transparent, 255 is fully opaque.
\sa getA(), setA()
*/
/*!
 * \qmlmethod byte Color4::getR()
 *
 * Returns the value of the \a{r} property.
 * \sa r, setR()
 */
/*!
 * \qmlmethod void Color4::setR(byte val)
 *
 * Sets the value of the \a{r} property to \a{val}.
 * \sa r, getR()
 */
/*!
 * \qmlmethod byte Color4::getA()
 *
 * Returns the value of the \a{a} property.
 * \sa a, setA()
 */
/*!
 * \qmlmethod void Color4::setA(byte val)
 *
 * Sets the value of the \a{a} property to \a{val}.
 * \sa a, getA()
 */
/*!
 * \qmlmethod byte Color4::getG()
 *
 * Returns the value of the \a{g} property.
 * \sa g, setG()
 */
/*!
 * \qmlmethod void Color4::setG(byte val)
 *
 * Sets the value of the \a{g} property to \a{val}.
 * \sa g, getG()
 */
/*!
 * \qmlmethod byte Color4::getB()
 *
 * Returns the value of the \a{b} property.
 * \sa b, setB()
 */
/*!
 * \qmlmethod void Color4::setB(byte val)
 *
 * Sets the value of the \a{b} property to \a{val}.
 * \sa b, getB()
 */

/*!
    \qmltype PrimAttrib
    \since 1.6.0
    \inmodule Engine
    \brief Base class for all Renderable Primitive Attributes.

    PrimAttrib is an abstract base class inherited by all primitive attributes.
*/




/*!
    \qmltype SpriteAttrib
    \inherits PrimAttrib
    \since 1.6.0
    \inmodule Engine
    \brief Renderable attributes for sprite primitives.

    SpriteAttrib contains renderable attributes used by all sprite primitives. An Entity can optionally
    use a \l{SpriteAttrib} object through the \l{Renderable} \l{Component}.

    \e{NOTE: All texture assets for use within ESTk should be saved in the PNG format!}

    \sa Renderable, QuadSpriteAttrib, BlendMode, Color4
*/
/*!
\qmlproperty string SpriteAttrib::texture

Asset-relative path to a PNG to use as texture or nil if no path is set.
\sa getTexPath(), setTexPath()
*/
/*!
\qmlproperty string SpriteAttrib::normalMap

Asset-relative path to a PNG to use as a normal map or nil if no path is set.
\sa getNormalMapPath(), setNormalMapPath()
*/
/*!
\qmlproperty string SpriteAttrib::specularMap

Asset-relative path to a PNG to use as a specular map or nil if no path is set.
\sa getSpecularMapPath(), setSpecularMapPath()
*/
/*!
\qmlproperty Material* SpriteAttrib::material

Material to be used by the sprite or nil if no material is set.
\sa getMaterial(), setMaterial()
*/
/*!
\qmlproperty Color4 SpriteAttrib::color

Color to be applied to the sprite. Defaults to white and fully opaque.
\sa getColor4(), setColor4()
*/
/*!
\qmlproperty int SpriteAttrib::frame

Sprite frame when using a sprite sheet texture. Frames are numbered from
left to right and top to bottom, starting at frame 0 and increasing. If no texture is
present, frame is ignored. If a texture is present, and frame is equal to -1,
the sprite is rendered using custom texture coordinates.
\sa getFrame(), setFrame(), setTexCoord()
*/
/*!
\qmlproperty BlendMode SpriteAttrib::blendMode

Blend state used by renderer to blend into framebuffer when using a non-opaque
alpha value.
\sa getBlendMode(), setBlendMode()
*/
/*!
 * \qmlmethod string SpriteAttrib::getTexPath()
 *
 * Returns the value of the \l{texture} property.
 * \sa texture, setTexPath()
 */
/*!
 * \qmlmethod void SpriteAttrib::setTexPath(string path)
 *
 * Sets the value of the \l{texture} property to \a{path}.
 * \sa texture, getTexPath()
 */
/*!
 * \qmlmethod string SpriteAttrib::getNormalMapPath()
 *
 * Returns the value of the \l{normalMap} property.
 * \sa normalMap, setNormalMapPath()
 */
/*!
 * \qmlmethod void SpriteAttrib::setNormalMapPath(string path)
 *
 * Sets the value of the \l{normalMap} property to \a{path}.
 * \sa normalMap, getNormalMapPath()
 */
/*!
 * \qmlmethod string SpriteAttrib::getSpecularMapPath()
 *
 * Returns the value of the \l{specularMap} property.
 * \sa specularMap, setSpecularMapPath()
 */
/*!
 * \qmlmethod void SpriteAttrib::setSpecularMapPath(string path)
 *
 * Sets the value of the \l{specularMap} property to \a{path}.
 * \sa specularMap, getSpecularMapPath()
 */
/*!
 * \qmlmethod Material* SpriteAttrib::getMaterial()
 *
 * Returns the value of the \l{material} property.
 * \sa material, setMaterial()
 */
/*!
 * \qmlmethod void SpriteAttrib::setMaterial(Material* mat)
 *
 * Sets the value of the \l{material} property to \a{mat}.
 * Ownership of \a{mat} is transferred from Lua to the engine.
 * \sa material, getMaterial()
 */
/*!
 * \qmlmethod BlendMode SpriteAttrib::getBlendMode()
 *
 * Returns the value of the \l{blendMode} property.
 * \sa blendMode, setBlendMode()
 */
/*!
 * \qmlmethod void SpriteAttrib::setBlendMode(BlendMode mode)
 *
 * Sets the value of the \l{blendMode} property to \a{mode}.
 * \sa blendMode, getBlendMode()
 */
/*!
 * \qmlmethod Color4 SpriteAttrib::getColor4()
 *
 * Returns the value of the \l{color} property.
 * \sa color, setColor4()
 */
/*!
 * \qmlmethod void SpriteAttrib::setColor4(Color4 col)
 *
 * Sets the value of the \l{color} property to \a{col}.
 * \sa color, getColor4()
 */
/*!
 * \qmlmethod int SpriteAttrib::getFrame()
 *
 * Returns the value of the \l{frame} property.
 * \sa frame, setFrame()
 */
/*!
 * \qmlmethod void SpriteAttrib::setFrame(int fr)
 *
 * Sets the value of the \l{frame} property to \a{fr}.
 * \sa frame, getFrame()
 */
/*!
 * \qmlmethod void SpriteAttrib::loadTexture(string filePath, int sheetWidth, int sheetHeight, int spriteWidth, int spriteHeight)
 *
 * Loads the PNG texture from the asset-relative path given by \a{filePath} with the image dimensions of
 * [\a{sheetWidth}x\a{sheetHeight}], splitting it into frames of size [\a{spriteWidth}x\a{spriteHeight}].
 * If the texture is a standard image and not a multi-frame spritesheet, simply pass spriteWidth and spriteHeight
 * with the same dimensions as sheetWidth and sheetHeight, so the entire image will be used as the first (default) frame.
 * \sa texture, setTexPath()
 */
/*!
 * \qmlmethod void SpriteAttrib::setTexCoord(int index, float u, float v)
 *
 * Sets the texture coordinates for the vertex given by \a{index} manually to <\a{u}, \a{v}>.
 * The \l{frame} must be set to -1 to use custom texture coordinates.
 * \sa frame, setFrame()
 */
/*!
 * \qmlmethod void SpriteAttrib::setColor(int r, int g, int b, int a)
 *
 * Sets the \a{color} property from separate RGBA colors. Valid values range from 0-255.
 * \sa color, setColor4(), getColor4()
 */
/*!
 * \qmlmethod void SpriteAttrib::setAlpha(int a)
 *
 * Sets the alpha component of the \a{color} property to \a{a}.
 * \sa getAlpha(), color, setColor()
 */
/*!
 * \qmlmethod int SpriteAttrib::getAlpha()
 *
 * Returns the value of the alpha component of the \a{color} property.
 * \sa getAlpha(), color, setColor()
 */
/*!
    \qmltype QuadSpriteAttrib
    \inherits SpriteAttrib
    \since 1.6.0
    \inmodule Engine
    \brief Renderable attributes for gourad shaded sprite primitives.

    QuadSpriteAttrib is an extension of SpriteAttrib that allows a sprite to be rendered with
    each of the four vertices getting its own color rather than a single solid color. When different
    colors are used, the polygon is filled in with a gradient using Gourad Shading.

    The color of the first vertex is taken from the parent class: SpriteAttrib::color.

    \sa Renderable
*/
/*!
 * \qmlmethod Color4 QuadSpriteAttrib::getVertexColor4(int index)
 *
 * Returns the 32-bit RGBA color associated with the vertex given by \a{index}.
 * Valid values are 0-3.
 * \sa setVertexColor4
 */
/*!
 * \qmlmethod void QuadSpriteAttrib::setVertexColor4(int index, Color4 color)
 *
 * Sets the 32-bit RGBA color associated with the vertex given by \a{index} to \a{color}.
 * Valid values for \a{index} are 0-3.
 * \sa getVertexColor4
 */
/*!
 * \qmlmethod void QuadSpriteAttrib::setVerticesAlpha(int a0, int a1, int a2, int a3)
 *
 * Sets the alpha values of all four vertices to the values given by \a{a0}, \a{a1}, \a{a2}, and \a{a3}.
 * Valid values are 0-255 where 0 is fully transparent and 255 is fully opaque.
 */
/*!
    \qmltype Material
    \since 1.6.0
    \inmodule Engine
    \brief Describes how a polygon interacts with a LightSource to create its final color.

    A Material is used to modify how a polygon, sprite, or Entity interacts with dynamic lighting
    to produce the final colors of the object. It contains properties used as color/intensity
    multipliers within the lighting equations. For example, a Material can be used to make an object
    reflect more \l{diffuse} green light than red and blue for some alien-ass space ship effect. A
    material can be used to make a metallic object extra shiny by increasing the intensity of the
    \l{specular} component. It can also make an object still emit light even when there is no LightSource
    present within a scene through its \l{emissive} property.

    \sa LightSource, LightSystem
*/
/*!
\qmlproperty Vector4 Material::ambient

4D vector serving as a multiplier for the RGBA components of the final ambient lighting color.
\sa getAmbientColor(), setAmbientColor()
*/
/*!
\qmlproperty Vector4 Material::diffuse

4D vector serving as a multiplier for the RGBA components of the final diffuse lighting color.
\sa getDiffuseColor(), setDiffuseColor()
*/
/*!
\qmlproperty Vector4 Material::specular

4D vector serving as a multiplier for the RGBA components of the final specular lighting color.
\sa getSpecularColor(), setSpecularColor()
*/
/*!
\qmlproperty Vector4 Material::emissive

4D vector serving as an RGBA base color to which all lighting components and colors are added,
meaning this color is present with or without external light.
\sa getEmissiveColor(), setEmissiveColor()
*/
/*!
\qmlproperty float Material::shininess

Floating point scalar multiplier applied to the shininess component within the specular lighting
equations.
\sa getShininess(), setShininess()
*/
/*!
 * \qmlmethod Vector4 Material::getAmbientColor()
 *
 * Returns the value of the \l{ambient} property.
 * \sa ambient, setAmbientColor()
 */
/*!
 * \qmlmethod void Material::setAmbientColor(Vector4 color)
 *
 * Sets the value of the \l{ambient} property to \a{color}.
 * \sa ambient, getAmbientColor()
 */
/*!
 * \qmlmethod Vector4 Material::getDiffuseColor()
 *
 * Returns the value of the \l{diffuse} property.
 * \sa diffuse, setDiffuseColor()
 */
/*!
 * \qmlmethod void Material::setDiffuseColor(Vector4 color)
 *
 * Sets the value of the \l{diffuse} property to \a{color}.
 * \sa diffuse, getDiffuseColor()
 */
/*!
 * \qmlmethod Vector4 Material::getSpecularColor()
 *
 * Returns the value of the \l{specular} property.
 * \sa specular, setSpecularColor()
 */
/*!
 * \qmlmethod void Material::setSpecularColor(Vector4 color)
 *
 * Sets the value of the \l{specular} property to \a{specular}.
 * \sa specular, getSpecularColor()
 */
/*!
 * \qmlmethod Vector4 Material::getEmissiveColor()
 *
 * Returns the value of the \l{emissive} property.
 * \sa emissive, setEmissiveColor()
 */
/*!
 * \qmlmethod void Material::setEmissiveColor(Vector4 color)
 *
 * Sets the value of the \l{emissive} property to \a{color}.
 * \sa emissive, getEmissiveColor()
 */
/*!
 * \qmlmethod float Material::getShininess()
 *
 * Returns the value of the \l{shininess} property.
 * \sa shininess, setShininess()
 */
/*!
 * \qmlmethod void Material::setShininess(float factor)
 *
 * Sets the value of the \l{shininess} property to \a{factor}.
 * \sa shininess, getShininess()
 */
/*!
    \qmltype BlendMode
    \since 1.6.1
    \inmodule Engine
    \brief Encapsulates a cross-platform Blend Mode state

    A BlendMode object encapsulates the cross-platform blend mode state
    used by libGyro when rendering a primitive. The blend mode identifiers and naming
    conventions are based on the OpenGL blend-modes. Please reference the
    \l{https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBlendFunc.xhtml}{OpenGL 4 Reference Pages}
    for more info.

    \section2 Blending Equations
    To determine the blended RGBA values of a pixel, the system uses the following equations:
    \table
        \header
            \li Component
            \li Formula
        \row
            \li R\sub d
            \li min(k\sub R, R\sub{s}s\sub R + R\sub{d}d\sub{R})
        \row
            \li G\sub d
            \li min(k\sub G, G\sub{s}s\sub G + G\sub{d}d\sub{G})
        \row
            \li B\sub d
            \li min(k\sub B, B\sub{s}s\sub B + B\sub{d}d\sub{B})
        \row
            \li A\sub d
            \li min(k\sub A, A\sub{s}s\sub A + A\sub{d}d\sub{A})
    \endtable

    \table
        \header
            \li Symbol
            \li Property
            \li Details
        \row
            \li s\sub <RGBA>
            \li srcFactor
            \li Coefficient which gets multiplied by the source pixel colors (current values sampled from framebuffer)
        \row
            \li d\sub <RGBA>
            \li dstFactor
            \li Coefficient which gets multiplied by the destination pixel colors (values of fragment being written to framebuffer)
        \row
            \li k\sub <RGBA>
            \li constColor
            \li Optional Constant color which an be specified as a minimum or additional factor
    \endtable

    \section2 BLEND_FACTOR
    The \l{srcFactor} and \l{dstFactor} blend factors may be one of the following constant enumeration values:
    \table
        \header
            \li Constant
            \li GL Enumeration
            \li Details
        \row
            \li BLEND_ZERO
            \li GL_ZERO
            \li (0, 0, 0, 0)
        \row
            \li BLEND_ONE
            \li GL_ONE
            \li (1, 1, 1, 1)
        \row
            \li BLEND_SRC_COLOR
            \li GL_SRC_COLOR
            \li (R\sub{s0}/k\sub R, G\sub{s0}/k\sub G, B\sub{s0}/k\sub B, A\sub{s0}/k\sub A)
        \row
            \li BLEND_ONE_MINUS_SRC_COLOR
            \li GL_ONE_MINUS_SRC_COLOR
            \li (1, 1, 1, 1) − (R\sub{s0}/k\sub R, G\sub{s0}/k\sub G, B\sub{s0}/k\sub B, A\sub{s0}/k\sub A)
        \row
            \li BLEND_DST_COLOR
            \li GL_DST_COLOR
            \li (R\sub{d}/k\sub R, G\sub{d}/k\sub G, B\sub{d}/k\sub B, A\sub{d}/k\sub A)
        \row
            \li BLEND_ONE_MINUS_DST_COLOR
            \li GL_ONE_MINUS_DST_COLOR
            \li (1, 1, 1, 1) −( R\sub{d}/k\sub R, G\sub{d}/k\sub G, B\sub{d}/k\sub B, A\sub{d}/k\sub A)
        \row
            \li BLEND_SRC_ALPHA
            \li GL_SRC_ALPHA
            \li (A\sub{s0}/k\sub A, A\sub{s0}/k\sub A, A\sub{s0}/kA, A\sub{s0}/k\sub A)
        \row
            \li BLEND_ONE_MINUS_SRC_ALPHA
            \li GL_ONE_MINUS_SRC_ALPHA
            \li (1, 1, 1, 1) − (A\sub{s0}/k\sub A, A\sub{s0}/k\sub A, A\sub{s0}/k\sub A, A\sub{s0}/k\sub A)
        \row
            \li BLEND_DST_ALPHA
            \li GL_DST_ALPHA
            \li (A\sub{d}/k\sub A, A\sub{d}/k\sub A, A\sub{d}/k\sub A, A\sub{d}/k\sub A)
        \row
            \li BLEND_ONE_MINUS_DST_ALPHA
            \li GL_ONE_MINUS_DST_ALPHA
            \li (1, 1, 1, 1) − (A\sub{d}/k\sub A, A\sub{d}/k\sub A, A\sub{d}/k\sub A, A\sub{d}/k\sub A)
        \row
            \li BLEND_CONSTANT_COLOR
            \li GL_CONSTANT_COLOR
            \li (R\sub c, G\sub c, B\sub c, A\sub c)
        \row
            \li BLEND_ONE_MINUS_CONSTANT_COLOR
            \li GL_ONE_MINUS_CONSTANT_COLOR
            \li (1, 1, 1, 1) − (R\sub c, G\sub c, B\sub c, A\sub c)
        \row
            \li BLEND_CONSTANT_ALPHA
            \li GL_CONSTANT_ALPHA
            \li (A\sub c, A\sub c, A\sub c, A\sub c)
        \row
            \li BLEND_ONE_MINUS_CONSTANT_ALPHA
            \li GL_ONE_MINUS_CONSTANT_ALPHA
            \li (1, 1, 1, 1) − (A\sub c, A\sub c, A\sub c, A\sub c)
        \row
            \li BLEND_SRC_ALPHA_SATURATE
            \li GL_SRC_ALPHA_SATURATE
            \li (i, i, i, 1)
        \row
            \li BLEND_SRC1_COLOR
            \li GL_SRC1_COLOR
            \li (R\sub{s1}/k\sub R, G\sub{s1}/k\sub G, B\sub{s1}/k\sub B, A\sub{s1}/k\sub A)
        \row
            \li BLEND_ONE_MINUS_SRC1_COLOR
            \li GL_ONE_MINUS_SRC1_COLOR
            \li (1, 1, 1, 1) − (R\sub{s1}/k\sub R, G\sub{s1}/k\sub G, B\sub{s1}/k\sub B, A\sub{s1}/k\sub A)
        \row
            \li BLEND_SRC1_ALPHA
            \li GL_SRC1_ALPHA
            \li (A\sub{s1}/k\sub A, A\sub{s1}/k\sub A, A\sub{s1}/k\sub A, A\sub{s1}/k\sub A)
        \row
            \li BLEND_ONE_MINUS_SRC1_ALPHA
            \li GL_ONE_MINUS_SRC1_ALPHA
            \li (1, 1, 1, 1) − (A\sub{s1}/k\sub A, A\sub{s1}/k\sub A, A\sub{s1}/k\sub A, A\sub{s1}/k\sub A)
    \endtable


    \sa Renderable, Color4, SpriteAttrib
*/
/*!
\qmlproperty BLEND_FACTOR BlendMode::srcFactor
Coefficient/factor multiplied by the source color sampled from the framebuffer when applying the blend equation.
\sa getSrcFactor(), setSrcFactor()
*/
/*!
\qmlproperty BLEND_FACTOR BlendMode::dstFactor
Coefficient/factor multiplied by the destination color being written to the framebuffer.
\sa getSrcFactor(), setSrcFactor()
*/
/*!
\qmlproperty Color4 BlendMode::constColor
Optional constant color to be used in blend equation.
\sa getConstColor(), setConstColor()
*/
/*!
 * \qmlmethod BLEND_FACTOR BlendMode::getSrcFactor()
 *
 * Returns the value of the \l{srcFactor} property.
 * \sa srcFactor, setSrcFactor()
 */
/*!
 * \qmlmethod void BlendMode::setSrcFactor(BLEND_FACTOR factor)
 *
 * Sets the value of the \l{srcFactor} property to \a{factor}.
 * \sa srcFactor, getSrcFactor()
 */
/*!
 * \qmlmethod BLEND_FACTOR BlendMode::getDstFactor()
 *
 * Returns the value of the \l{dstFactor} property.
 * \sa dstFactor, setDstFactor()
 */
/*!
 * \qmlmethod void BlendMode::setDstFactor(BLEND_FACTOR factor)
 *
 * Sets the value of the \l{dstFactor} property to \a{factor}.
 * \sa dstFactor, getDstFactor()
 */
/*!
 * \qmlmethod Color4 BlendMode::getConstColor()
 *
 * Returns the value of the \l{constColor} property.
 * \sa constColor, setConstColor()
 */
/*!
 * \qmlmethod void BlendMode::setConstColor(Color4 color)
 *
 * Sets the value of the \l{constColor} property to \a{color}.
 * \sa constColor, getConstColor()
 */


namespace elysian::lua {
using namespace elysian;
using Color4 = elysian::Color4;
using Material = elysian::Material;
using BlendMode = elysian::BlendMode;
using PrimAttrib = elysian::PrimAttrib;
using SpriteAttrib = elysian::SpriteAttrib;
using QuadSpriteAttrib = elysian::QuadSpriteAttrib;
using Recti = elysian::Recti;

void registerGYColor4(sol::state_view state) {
    if (state["GYColor4"].valid()) return;
    using T = GYColor4;
    auto type = registerType<T>(state, "GYColor4");
    type["new"] = sol::factories(factory<T>());
    type["getA"] = [](const T& self) { return self.a; };
    type["setA"] = [](T& self, decltype(T::a) value) { self.a = value; };
    type["getR"] = [](const T& self) { return self.r; };
    type["setR"] = [](T& self, decltype(T::r) value) { self.r = value; };
    type["getG"] = [](const T& self) { return self.g; };
    type["setG"] = [](T& self, decltype(T::g) value) { self.g = value; };
    type["getB"] = [](const T& self) { return self.b; };
    type["setB"] = [](T& self, decltype(T::b) value) { self.b = value; };
}

void registerColor4(sol::state_view state) {
    if (state["Color4"].valid()) return;
    using T = elysian::Color4;
    auto type = registerType<T>(state, "Color4");
    type["new"] = sol::factories(factory<T>());
    type["getR"] = method<T, unsigned char() const>(&T::getR);
    type["getG"] = method<T, unsigned char() const>(&T::getG);
    type["getB"] = method<T, unsigned char() const>(&T::getB);
    type["getA"] = method<T, unsigned char() const>(&T::getA);
    type["setR"] = method<T, void(const unsigned char) const>(&T::setR);
    type["setG"] = method<T, void(const unsigned char) const>(&T::setG);
    type["setB"] = method<T, void(const unsigned char) const>(&T::setB);
    type["setA"] = method<T, void(const unsigned char) const>(&T::setA);
    property(type, "r", method<T, unsigned char() const>(&T::getR), method<T, void(const unsigned char) const>(&T::setR));
    property(type, "g", method<T, unsigned char() const>(&T::getG), method<T, void(const unsigned char) const>(&T::setG));
    property(type, "b", method<T, unsigned char() const>(&T::getB), method<T, void(const unsigned char) const>(&T::setB));
    property(type, "a", method<T, unsigned char() const>(&T::getA), method<T, void(const unsigned char) const>(&T::setA));
    inheritProperties<T>();
    finish<T>(type, "Color4", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerMaterial(sol::state_view state) {
    if (state["Material"].valid()) return;
    using T = elysian::Material;
    auto type = registerType<T>(state, "Material");
    type["new"] = sol::factories(factory<T>());
    type["setAmbientColor"] = method<T, void(const GYVector4&)>(&T::setAmbientColor);
    type["setDiffuseColor"] = method<T, void(const GYVector4&)>(&T::setDiffuseColor);
    type["setSpecularColor"] = method<T, void(const GYVector4&)>(&T::setSpecularColor);
    type["setEmissiveColor"] = method<T, void(const GYVector4&)>(&T::setEmissiveColor);
    type["setShininess"] = method<T, void(float)>(&T::setShininess);
    type["getAmbientColor"] = method<T, void(GYVector4&) const>(&T::getAmbientColor);
    type["getDiffuseColor"] = method<T, void(GYVector4&) const>(&T::getDiffuseColor);
    type["getSpecularColor"] = method<T, void(GYVector4&) const>(&T::getSpecularColor);
    type["getEmissiveColor"] = method<T, void(GYVector4&) const>(&T::getEmissiveColor);
    type["getShininess"] = method<T, float() const>(&T::getShininess);
    type["getEmissive"] = method<T, Vector4()>(&T::getEmissive);
    type["getDiffuse"] = method<T, Vector4()>(&T::getDiffuse);
    type["getAmbient"] = method<T, Vector4()>(&T::getAmbient);
    type["getSpecular"] = method<T, Vector4()>(&T::getSpecular);
    property(type, "ambient", method<T, Vector4()>(&T::getAmbient));
    property(type, "diffuse", method<T, Vector4()>(&T::getDiffuse));
    property(type, "specular", method<T, Vector4()>(&T::getSpecular));
    property(type, "emissive", method<T, Vector4()>(&T::getEmissive));
    property(type, "shininess", method<T, float() const>(&T::getShininess), method<T, void(float)>(&T::setShininess));
    inheritProperties<T>();
    finish<T>(type, "Material", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerBlendMode(sol::state_view state) {
    if (state["BlendMode"].valid()) return;
    using T = elysian::BlendMode;
    auto type = registerType<T>(state, "BlendMode");
    type["new"] = sol::factories(factory<T>());
    type["getConstColor"] = method<T, Color4()>(&T::getConstColor);
    type["setConstColor"] = method<T>(&T::setConstColor);
    type["getSrcFactor"] = method<T>(&T::getSrcFactor);
    type["setSrcFactor"] = method<T>(&T::setSrcFactor);
    type["getDstFactor"] = method<T>(&T::getDstFactor);
    type["setDstFactor"] = method<T>(&T::setDstFactor);
    property(type, "constColor", method<T, Color4()>(&T::getConstColor), method<T>(&T::setConstColor));
    property(type, "srcFactor", method<T>(&T::getSrcFactor), method<T>(&T::setSrcFactor));
    property(type, "dstFactor", method<T>(&T::getDstFactor), method<T>(&T::setDstFactor));
    inheritProperties<T>();
    finish<T>(type, "BlendMode", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerPrimAttrib(sol::state_view state) {
    if (state["PrimAttrib"].valid()) return;
    using T = elysian::PrimAttrib;
    auto type = registerType<T>(state, "PrimAttrib");
}

void registerSpriteAttrib(sol::state_view state) {
    if (state["SpriteAttrib"].valid()) return;
    registerPrimAttrib(state);
    using T = elysian::SpriteAttrib;
    auto type = registerType<T, elysian::PrimAttrib>(state, "SpriteAttrib");
    type["new"] = sol::factories(sharedFactory<T>());
    type["setFrame"] = method<T, void(unsigned)>(&T::setFrame);
    type["getFrame"] = method<T, int() const>(&T::getFrame);
    type["setColor"] = method<T, void(unsigned char, unsigned char, unsigned char, unsigned char)>(&T::setColor);
    type["getAlpha"] = method<T, unsigned char() const>(&T::getAlpha);
    type["getTexPath"] = method<T, const char*() const>(&T::getTexPath);
    type["setTexPath"] = method<T, void(char const*)>(&T::setTexPath);
    type["getColor4"] = method<T, Color4()>(&T::getColor4);
    type["setColor4"] = method<T, void(const Color4&)>(&T::setColor4);
    type["setAlpha"] = method<T, void(unsigned char)>(&T::setAlpha);
    type["loadTexture"] = method<T, void(const char *, unsigned, unsigned, unsigned, unsigned)>(&T::loadTexture);
    type["loadSheet"] = method<T, void(const char *, unsigned, unsigned)>(&T::loadSheet);
    type["setTexCoord"] = method<T, void(unsigned, float, float)>(&T::setTexCoord);
    type["setMaterial"] = [](T& self, sol::object arg0) { return std::invoke(method<T, void(Material*)>(&T::setMaterial), self, take<Material>(arg0)); };
    type["getMaterial"] = method<T, Material*() const>(&T::getMaterial);
    type["getNormalMapPath"] = method<T, const char*() const>(&T::getNormalMapPath);
    type["getSpecularMapPath"] = method<T, const char*() const>(&T::getSpecularMapPath);
    type["setNormalMapPath"] = method<T, void(const char*)>(&T::setNormalMapPath);
    type["setSpecularMapPath"] = method<T, void(const char*)>(&T::setSpecularMapPath);
    type["getBlendMode"] = method<T, BlendMode&()>(&T::getBlendMode);
    type["setBlendMode"] = method<T, void(const BlendMode&)>(&T::setBlendMode);
    property(type, "texture", method<T, const char*() const>(&T::getTexPath), method<T, void(char const*)>(&T::setTexPath));
    property(type, "normalMap", method<T, const char*() const>(&T::getNormalMapPath), method<T, void(const char*)>(&T::setNormalMapPath));
    property(type, "specularMap", method<T, const char*() const>(&T::getSpecularMapPath), method<T, void(const char*)>(&T::setSpecularMapPath));
    property(type, "material", method<T, Material*() const>(&T::getMaterial), [](T& self, sol::object arg0) { return std::invoke(method<T, void(Material*)>(&T::setMaterial), self, take<Material>(arg0)); });
    property(type, "color", method<T, Color4()>(&T::getColor4), method<T, void(const Color4&)>(&T::setColor4));
    property(type, "blendMode", method<T, BlendMode&()>(&T::getBlendMode), method<T, void(const BlendMode&)>(&T::setBlendMode));
    property(type, "frame", method<T, int() const>(&T::getFrame), method<T, void(unsigned)>(&T::setFrame));
    inheritProperties<T, elysian::PrimAttrib>();
    finish<T>(type, "SpriteAttrib", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerQuadSpriteAttrib(sol::state_view state) {
    if (state["QuadSpriteAttrib"].valid()) return;
    registerSpriteAttrib(state);
    registerPrimAttrib(state);
    using T = elysian::QuadSpriteAttrib;
    auto type = registerType<T, elysian::SpriteAttrib, elysian::PrimAttrib>(state, "QuadSpriteAttrib");
    type["new"] = sol::factories(sharedFactory<T>());
    type["setVertexColor4"] = method<T, void(const unsigned short, const Color4&)>(&T::setVertexColor4);
    type["getVertexColor4"] = method<T, Color4(const unsigned short)>(&T::getVertexColor4);
    type["setVerticesAlpha"] = method<T, void(const unsigned char, const unsigned char, const unsigned char, const unsigned char)>(&T::setVerticesAlpha);
}

void registerRecti(sol::state_view state) {
    if (state["Recti"].valid()) return;
    using T = elysian::Recti;
    auto type = registerType<T>(state, "Recti");
    type["new"] = sol::factories(factory<T>());
    type["setTop"] = method<T>(&T::setTop);
    type["setBottom"] = method<T>(&T::setBottom);
    type["setLeft"] = method<T>(&T::setLeft);
    type["setRight"] = method<T>(&T::setRight);
    type["getTop"] = method<T>(&T::getTop);
    type["getBottom"] = method<T>(&T::getBottom);
    type["getLeft"] = method<T>(&T::getLeft);
    type["getRight"] = method<T>(&T::getRight);
    property(type, "top", method<T>(&T::getTop), method<T>(&T::setTop));
    property(type, "bottom", method<T>(&T::getBottom), method<T>(&T::setBottom));
    property(type, "left", method<T>(&T::getLeft), method<T>(&T::setLeft));
    property(type, "right", method<T>(&T::getRight), method<T>(&T::setRight));
    inheritProperties<T>();
    finish<T>(type, "Recti", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}
}
