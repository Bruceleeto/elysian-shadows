/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_area_proxy.hpp>
#include <AssetIO/proxy/elysian_entity_proxy.hpp>


/*!
    \qmltype TileEntry
    \since 1.6.0
    \inmodule Engine
    \brief Represents the tile attributes at a single map location.

    A TileEntry represents a tile index and various other attributes stored at ever map location.
    The map is a 3D array of TileEntry objects stored within an Area. For performance reasons,
    a TileEntry is actually a packed 32-bit integer whose attributes are represented by bitfields.
    For the sake of simplicity and your sanity, a TileEntry is treated like a traditional object in Lua.

    \sa {Area}, {Tile Layers}, {Tile}, {Areas}
*/

/*!
 * \qmlmethod int TileEntry::getIndex()
 *
 * Returns the tile index of the TileEntry. This index represents its tile ID and is used to determine the tile's
 * position within the tilesheet and its attributes within a Level's tileset array.
 * \sa Tile, setIndex()
 */
/*!
 * \qmlmethod void TileEntry::setIndex(int index)
 *
 * Sets the tile index of the TileEntry to \a{index}.
 * \sa Tile, getIndex()
 */
/*!
 * \qmlmethod int TileEntry::getElev()
 *
 * Returns the elevation of the TileEntry.
 * \sa setElev()
 */
/*!
 * \qmlmethod void TileEntry::setElev(int elev)
 *
 * Sets the elevation of the TileEntry to \a{elev}. Elevation is essentially the z coordinate of the tile and is used
 * to give the tile height in 3D space. Every unit of elevation is equal to 32 pixels, or one tile size. Valid values
 * for \a{elev} range from 0-31.
 * \sa getElev()
 */
/*!
 * \qmlmethod RECT_PLANE_TYPE TileEntry::getPlane()
 *
 * Returns the plane type of the TileEntry. Plane type can be thought of as the face of a cube the tile is applied to
 * in 3D space.
 * \sa getPlane(), gyro::RECT_PLANE_TYPE
 */
/*!
 * \qmlmethod void TileEntry::setPlane(RECT_PLANE_TYPE plane)
 *
 * Sets the face of the 3D cube the TileEntry will be applied to to \a{plane}.
 * \sa setPlane(), gyro::RECT_PLANE_TYPE
 */
/*!
 * \qmlmethod TEX_ORIENT_TYPE TileEntry::getOrient()
 *
 * Retuns the orientation of the TileEntry (flipped horizontal/vertical, rotated 90/180/270 degrees, etc).
 * \sa setOrient(), gyro::TEX_ORIENT_TYPE
 */
/*!
 * \qmlmethod void TileEntry::setOrient(TEX_ORIENT_TYPE type)
 *
 * Sets the orientation of the TileEntry to the flip/rotation given by \a{type}.
 * \sa getOrient(), gyro::TEX_ORIENT_TYPE
 */
/*!
 * \qmlmethod bool TileEntry::getNormal()
 *
 * Convenience function around getPlane() which returns true for any plane type that is not flat.
 * \sa setNormal(), getPlane(), gyro::RECT_PLANE_TYPE
 */
/*!
 * \qmlmethod bool TileEntry::setNormal(bool val)
 *
 * Convenience function around setPlane() which sets the plane type to gyro.RECT_PLANE_FRONT when \a{val} is true
 * and gyro.RECT_PLANE_BOTTOM when \a{val} is false.
 * \sa getNormal(), setPlane(), gyro::RECT_PLANE_TYPE
 */
/*!
 * \qmlmethod int TileEntry::getValue()
 *
 * Returns the raw 32-bit integer representation of the TileEntry.
 * \sa setValue()
 */
/*!
 * \qmlmethod void TileEntry::setValue(int val)
 *
 * Sets the 32-bit integer value of TilEntry to \a{val}, which means it must be the correct bitfield format.
 * Do not use this function unless you really know what the fuck you're doing.
 * \sa getValue()
 */

/*!
    \qmltype Area
    \since 1.6.0
    \inmodule Engine
    \brief Class representing an Area and encapsulating a map and its entities.

    An Area represents a sigle map within ESTk and is located within a Level. The Area gets its tile attributes
    and tile sheets from its parent level, but contains its own map and all entities inhabiting the map.

    \section2 Retrieving the Current Area
    The current Area can be retrieved through the Engine static class:
    \code
    curArea = Engine.getCurArea()
    \endcode
    NOTE: \e {Do not EVER cache an Area object between frames, since the player can warp to another area, and it can change.
    Always fetch it from the Engine at the beginning of the frame.}

    \section2 Representation
    \list
    \li \b {Map}: The map is represented as a 3D array of TileEntry objects. The first dimension is the layer. An area has three tile layers by default and can be upgraded to four in the editor.
    Use getLayerCount() when iterating tile layers. The other two represent the <x, y> location of the tile.
    \li \b {Entities}: Entities are stored within two separate arrays. The first is activeEntities, which stores entities
    that are active within the current scene. The second array is inactiveEntities, which can store entities that will
    later be spawned or added to the scene dynamically through a Lua script.
    \endlist

    \sa {Areas}, {Tile Layers}, {TileEntry}, {Engine}
*/

/*!
 * \qmlproperty string Area::dirName
 * The name of the directory containing the Area.
 * \sa getDirName(), setDirName()
 */
/*!
 * \qmlproperty string Area::nameString
 * Display name of the Area that will be used in-game.
 * \sa getNameString(), setNameString()
 */
/*!
 * \qmlproperty string Area::path
 * Read-only asset-relative file path of Area directory.
 * \sa getPath()
 */
/*!
 * \qmlproperty table Area::size
 * Virtual table containing the width and height dimensions of the map in tiles.
 * Members:
 * \list
 * \li width: int
 * \li height: int
 * \endlist
 * \sa getMapWidth(), setMapWidth(), getMapHeight(), setMapHeight()
 */
/*!
 * \qmlproperty int Area::size::width
 * Width of the Area's map in tiles.
 * \sa getMapWidth(), getMapHeight()
 */
/*!
 * \qmlproperty array Area::activeEntities
 * An array containing all active entities within the Area.
 * \sa getActiveEntityByIndex(), setActiveEntityByIndex(), getActiveEntityCount()
 */
/*!
 * \qmlproperty array Area::inactiveEntities
 * An array containing all inactive entities within the Area.
 * \sa getInactiveEntityByIndex(), setInactiveEntityByIndex(), getInactiveEntityCount()
 */
/*!
 * \qmlproperty table Area::music
 * Virtual table containing arrays for both background and auxiliary music OGG tracks.
 * Members:
 * \list
 * \li bgm: array
 *  \list
 *  \li 1: string
 *  \li 2: string
 *  \li 3: string \endlist
 * \li aux: array
 *  \list
 *  \li 1: string
 *  \li 2: string
 *  \li 3: string
 *  \endlist
 * \endlist
 * \sa getBgmOggByTrack(), setBgmOggByTrack(), getBgmOggTrackCount(), getAuxOggByTrack(), setAuxOggByTrack(), getAuxOggTrackCount()
 */
/*!
 * \qmlproperty bool Area::useSun
 * When set to true, tells the \l{DaySystem} to automatically add the sun \l{Entity} to the Area to simulate the
 * time of day with the sun rising and setting for outdoor Areas. If set to false, no sun is added. The
 * default value is true.
 * \sa getUseSun(), setUseSun(), DaySystem
 */
/*!
 * \qmlproperty Vector4 Area::ambientColor
 * 4D RGBA vector representing the intensity of ambient light in the Area. Ambient light is light that is always present everywhere
 * in equal amounts. If this is set to <0, 0, 0, 0>, the entire Area will be pitch black. For outdoor Areas, this should be
 * set to a low value to simulate night, then the \l{useSun} property should be set to true, so the sun entity emits sunlight
 * to light the rest of the Area during the day. Without the sun, this can be used to simulate ambient color within caves or
 * indoor locations.
 * \sa getAmbientColor(), setAmbientColor()
 */
/*!
 * \qmlproperty Vector3 Area::startPosition
 * 3D default starting position used when a user swaps to the Area within ESTk. This is only for the Toolkit, as for standalone Engine builds,
 * the player would be entering the Area to a position specified by a \l{Warp}.
 * \sa getStartPosition(), setStartPosition()
 */

/*!
 * \qmlmethod string Area::getDirName()
 *
 * Retrieves the name of the directory containing the Area.
 */
/*!
 * \qmlmethod string Area::getPath()
 *
 * Retrieves the asset-relative path of the Area directory.
 */
/*!
 * \qmlmethod void Area::setDirName(string name)
 *
 * Changes the name of the Area directory to \a name.
 */
/*!
 * \qmlmethod void Area::setNameString(string name)
 *
 * Changes the display name of the Area to \a name.
 */
/*!
 * \qmlmethod string Area::getNameString()
 *
 * Returns the display name of the Area.
 */
/*!
 * \qmlmethod TileEntry Area::getTileEntry(int layer, int x, int y)
 *
 * Returns the TileEntry object for the map location given by <\a{x}, \a{y}> on the tile layer given by \a layer.
 * \a{x} and \a{y} must be within the range of the map, and \a{layer} must be between 0 and 2.
 */
/*!
 * \qmlmethod void Area::setTileEntry(int layer, int x, int y, TileEntry entry)
 *
 * Sets the TileEntry at position <\a{x}, \a{y}> on \a{layer} to \a{entry}. The position and layer must be within
 * the map boundaries.
 */
/*!
 * \qmlmethod void Area::addEntity(Entity* entity)
 *
 * Adds \a{entity} to the Area. Ownership of \a{entity} is transferred from Lua to the engine.
 */
/*!
 * \qmlmethod void Area::removeEntity(Entity* entity)
 *
 * Removes \a{entity} from the Area. Ownership is transferred from the engine to Lua.
 */
/*!
 * \qmlmethod Entity* Area::findEntity(string name)
 *
 * Searches the Area for an entity with the given \a{name}. Returns a reference to that entity if found or nil if not.
 * This function does a linear search, so it's relatively expensive. Only use this initially to find an entity, then
 * cache its reference rathe than looking it up every frame.
 */
/*!
 * \qmlmethod int Area::getLayerCount()
 *
 * Returns the number of tile layers stored in this area (three or four).
 */

/*!
 * \qmlmethod void Area::setMapHeight(int height)
 *
 * Sets the height of the map to the tile height given by \a{height}, either enlarging to shrinking the map. If enlarged, the TileEntry values
 * at each map location are nondeterministic and need to be filled in. If shrank, all TileEntry values that are no longer
 * within the map boundaries will be lost.
 */
/*!
 * \qmlmethod void Area::setMapWidth(int width)
 *
 * Sets the width of the map to the tile width given by \a{width}, either enlarging to shrinking the map. If enlarged, the TileEntry values
 * at each map location are nondeterministic and need to be filled in. If shrank, all TileEntry values that are no longer
 * within the map boundaries will be lost.
 */
/*!
 * \qmlmethod int Area::getMapHeight()
 *
 * Returns the height of the map in tiles.
 */
/*!
 * \qmlmethod int Area::getMapWidth()
 *
 * Returns the width of the map in tiles.
 */
/*!
 * \qmlmethod void Area::setActiveEntityByIndex(int index, Entity* entity)
 * Sets the active Entity at \a{index} to \a{entity}. If \a{index} is 1 greater than the number of active entities,
 * \a{entity} is added as an active entity. Ownership of \a{entity} is transferred from Lua to the engine.
 */
/*!
 * \qmlmethod void Area::setInactiveEntityByIndex(int index, Entity* entity)
 * Sets the inactive Entity at \a{index} to \a{entity}. If \a{index} is 1 greater than the number of inactive entities,
 * \a{entity} is added as an inactive entity. Ownership of \a{entity} is transferred from Lua to the engine.
 */
/*!
 * \qmlmethod Entity* Area::getActiveEntityByIndex(int index)
 * Returns the active entity at \a{index} or nil if there isn't one.
 */
/*!
 * \qmlmethod Entity* Area::getInactiveEntityByIndex(int index)
 * Returns the inactive entity at \a{index} or nil if there isn't one.
 */
/*!
 * \qmlmethod int Area::getActiveEntityCount()
 * Returns the number of active entities within the Area.
 */
/*!
 * \qmlmethod int Area::getInactiveEntityCount()
 * Returns the number of inactive entities within the Area.
 */
/*!
 * \qmlmethod bool Area::getUseSun()
 * Returns true if the Area uses the default sun directional light and false if it doesn't.
 * \sa {DaySystem}
 */
/*!
 * \qmlmethod void Area::setUseSun(bool val)
 * If \a{val} is true, tells the engine to use the default sun directional light within the Area.
 * If \a{val} is false, tells the engine not to use the sun.
 * This should be set to true for all outdoor scenes.
 * \sa {DaySystem}
 */
/*!
 * \qmlmethod Vector4 Area::getAmbientColor()
 * Retuns a Vector4 representing an ambient color in the format of <R, G, B, A>. The ambient color is the
 * base lighting color effecting the entire area.
 */
/*!
 * \qmlmethod void Area::setAmbientColor(Vector4 color)
 * Sets the ambient color of the Area to the <R, G, B, A> vector provided by \a{color}.
 */
/*!
 * \qmlmethod string Area::getBgmOggByTrack(int track)
 * Returns the path of the background OGG music file for the area to be played on \a{track}.
 * Valid tracks are 1-3.
 * \sa AudioSystem
 */
/*!
 * \qmlmethod void Area::setBgmOggByTrack(int track, string oggPath)
 * Loads the OGG music file from the path given by \a{oggPath} and sets it as the background music
 * for the Area on \a{track.} Valid tracks are 1-3.
 * \sa AudioSystem
 */
/*!
 * \qmlmethod int Area::getBgmOggTrackCount()
 * Returns the number of background OGG tracks per Area supported by the engine. This should always return 3.
 * \sa AudioSystem
 */
/*!
 * \qmlmethod string Area::getAuxOggByTrack(int track)
 * Returns the path of the auxiliary OGG sound file for the area to be played on \a{track}.
 * Valid tracks are 1-3.
 * \sa AudioSystem
 */
/*!
 * \qmlmethod void Area::setAuxOggByTrack(int track, string oggPath)
 * Loads the OGG music file from the path given by \a{oggPath} and sets it as the auxiliary sound
 * for the Area on \a{track.} Valid tracks are 1-3.
 * \sa AudioSystem
 */
/*!
 * \qmlmethod int Area::getAuxOggTrackCount()
 * Returns the number of auxiliary sound OGG tracks per Area supported by the engine. This should always return 3.
 * \sa AudioSystem
 */
/*!
 * \qmlmethod Vector3& Area::getStartPosition()
 * Returns the starting location for the Area when a user swaps to it within ESTk rather than warping.
 * \sa setStartPosition
 */
/*!
 * \qmlmethod void Area::setStartPosition(const Vector3&)
 * Sets the starting location for the Area when a user swaps to it within ESTk rather than warping.
 * \sa getStartPosition
 */


namespace elysian::lua {
using namespace elysian;
using TileEntry = elysian::TileEntry;
using Area = elysian::Area;

void registerTileEntry(sol::state_view state) {
    if (state["TileEntry"].valid()) return;
    using T = elysian::TileEntry;
    auto type = registerType<T>(state, "TileEntry");
    type["new"] = sol::factories(factory<T>(), factory<T, uint32_t>());
    type["getIndex"] = method<T, uint16_t() const>(&T::getIndex);
    type["setIndex"] = method<T, void(uint16_t)>(&T::setIndex);
    type["setNormal"] = method<T, void(bool)>(&T::setNormal);
    type["getNormal"] = method<T, bool() const>(&T::getNormal);
    type["getPlane"] = method<T, GY_RECT_PLANE_TYPE() const>(&T::getPlane);
    type["setPlane"] = method<T, void(GY_RECT_PLANE_TYPE)>(&T::setPlane);
    type["getValue"] = method<T, uint32_t() const>(&T::getValue);
    type["setValue"] = method<T, void(uint32_t)>(&T::setValue);
    type["getOrient"] = method<T, GY_RECT_TEX_ORIENT() const>(&T::getOrient);
    type["setOrient"] = method<T, void(GY_RECT_TEX_ORIENT)>(&T::setOrient);
    type["getElev"] = method<T, unsigned char() const>(&T::getElev);
    type["setElev"] = method<T, void(unsigned char)>(&T::setElev);
}

void registerArea(sol::state_view state) {
    if (state["Area"].valid()) return;
    using T = elysian::Area;
    auto type = registerType<T>(state, "Area");
    type["getDirName"] = method<T, const char*() const>(&T::getDirName);
    type["getPath"] = method<T, const char*() const>(&T::getPath);
    type["setDirName"] = method<T, void(const char*)>(&T::setDirName);
    type["setNameString"] = method<T, void(const char*)>(&T::setNameString);
    type["getStartPosition"] = method<T, Vector3&()>(&T::getStartPosition);
    type["setStartPosition"] = method<T, void(const Vector3&)>(&T::setStartPosition);
    type["getNameString"] = method<T, const char*() const>(&T::getNameString);
    type["getTileEntry"] = method<T, TileEntry(const unsigned, const unsigned, const unsigned) const>(&T::getTileEntry);
    type["setTileEntry"] = method<T, void(unsigned, unsigned, unsigned, TileEntry)>(&T::setTileEntry);
    type["addEntity"] = [](T& self, sol::object arg0) { return std::invoke(method<T, void(Entity*)>(&T::addEntity), self, take<Entity>(arg0)); };
    type["removeEntity"] = method<T, void(Entity*)>(&T::removeEntity);
    type["findEntity"] = method<T, Entity*(const char*)>(&T::findEntity);
    type["getMapWidth"] = method<T, unsigned short() const>(&T::getMapWidth);
    type["getMapHeight"] = method<T, unsigned short() const>(&T::getMapHeight);
    type["setMapHeight"] = method<T, void(const unsigned short)>(&T::setMapHeight);
    type["getLayerCount"] = method<T, unsigned() const>(&T::getLayerCount);
    type["setMapWidth"] = method<T, void(const unsigned short)>(&T::setMapWidth);
    type["setActiveEntityByIndex"] = [](T& self, const unsigned arg0, sol::object arg1) { return std::invoke(method<T, void(const unsigned, Entity*)>(&T::setActiveEntityByIndex), self, arg0, take<Entity>(arg1)); };
    type["getActiveEntityByIndex"] = method<T, Entity*(unsigned) const>(&T::getActiveEntityByIndex);
    type["getActiveEntityCount"] = method<T, unsigned() const>(&T::getActiveEntityCount);
    type["setInactiveEntityByIndex"] = [](T& self, const unsigned arg0, sol::object arg1) { return std::invoke(method<T, void(const unsigned, Entity*)>(&T::setInactiveEntityByIndex), self, arg0, take<Entity>(arg1)); };
    type["getInactiveEntityByIndex"] = method<T, Entity*(unsigned) const>(&T::getInactiveEntityByIndex);
    type["getInactiveEntityCount"] = method<T, unsigned() const>(&T::getInactiveEntityCount);
    type["getUseSun"] = method<T, bool() const>(&T::getUseSun);
    type["setUseSun"] = method<T, void(bool)>(&T::setUseSun);
    type["getAmbientColor"] = method<T, Vector4()>(&T::getAmbientColor);
    type["setAmbientColor"] = method<T, void(const Vector4&)>(&T::setAmbientColor);
    type["getBgmOggByTrack"] = method<T, const char*(unsigned) const>(&T::getBgmOggByTrack);
    type["getAuxOggByTrack"] = method<T, const char*(unsigned) const>(&T::getAuxOggByTrack);
    type["getBgmOggTrackCount"] = method<T, unsigned() const>(&T::getBgmOggTrackCount);
    type["getAuxOggTrackCount"] = method<T, unsigned() const>(&T::getAuxOggTrackCount);
    type["setBgmOggByTrack"] = method<T, void(unsigned, const char*)>(&T::setBgmOggByTrack);
    type["setAuxOggByTrack"] = method<T, void(unsigned, const char*)>(&T::setAuxOggByTrack);
    type["setOggGain"] = method<T>(&T::setOggGain);
    type["getOggGain"] = method<T>(&T::getOggGain);
    property(type, "dirName", method<T, const char*() const>(&T::getDirName), method<T, void(const char*)>(&T::setDirName));
    property(type, "nameString", method<T, const char*() const>(&T::getNameString), method<T, void(const char*)>(&T::setNameString));
    property(type, "path", method<T, const char*() const>(&T::getPath));
    viewProperty(type, group("size", {
            field<T>("width", method<T, unsigned short() const>(&T::getMapWidth), method<T, void(const unsigned short)>(&T::setMapWidth)),
            field<T>("height", method<T, unsigned short() const>(&T::getMapHeight), method<T, void(const unsigned short)>(&T::setMapHeight))
        }, [] { return LuaManager::isSerializing(); }));
    property(type, "useSun", method<T, bool() const>(&T::getUseSun), method<T, void(bool)>(&T::setUseSun));
    property(type, "ambientColor", method<T, Vector4()>(&T::getAmbientColor), method<T, void(const Vector4&)>(&T::setAmbientColor));
    viewProperty(type, group("music", {
            array<T>("bgm", method<T, const char*(unsigned) const>(&T::getBgmOggByTrack), method<T, void(unsigned, const char*)>(&T::setBgmOggByTrack), method<T, unsigned() const>(&T::getBgmOggTrackCount)),
            array<T>("aux", method<T, const char*(unsigned) const>(&T::getAuxOggByTrack), method<T, void(unsigned, const char*)>(&T::setAuxOggByTrack), method<T, unsigned() const>(&T::getAuxOggTrackCount)),
            field<T>("gain", method<T>(&T::getOggGain), method<T>(&T::setOggGain))
        }, [] { return LuaManager::isSerializing(); }));
    property(type, "startPosition", method<T, Vector3&()>(&T::getStartPosition), method<T, void(const Vector3&)>(&T::setStartPosition));
    viewProperty(type, array<T>("activeEntities", method<T, Entity*(unsigned) const>(&T::getActiveEntityByIndex), [](T& self, const unsigned arg0, sol::object arg1) { return std::invoke(method<T, void(const unsigned, Entity*)>(&T::setActiveEntityByIndex), self, arg0, take<Entity>(arg1)); }, method<T, unsigned() const>(&T::getActiveEntityCount)));
    viewProperty(type, array<T>("inactiveEntities", method<T, Entity*(unsigned) const>(&T::getInactiveEntityByIndex), [](T& self, const unsigned arg0, sol::object arg1) { return std::invoke(method<T, void(const unsigned, Entity*)>(&T::setInactiveEntityByIndex), self, arg0, take<Entity>(arg1)); }, method<T, unsigned() const>(&T::getInactiveEntityCount)));
    #ifndef ELYSIAN_ENGINE
    eraseProperty<T>("dirName");
    eraseProperty<T>("startPosition");
    property(type, "dirName", method<T, const char*() const>(&T::getDirName));
    property(type, "startPosition", method<T, Vector3&()>(&T::getStartPosition), method<T, void(const Vector3&)>(&T::setStartPosition));
    #endif
    inheritProperties<T>();
    finish<T>(type, "Area", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}
}
