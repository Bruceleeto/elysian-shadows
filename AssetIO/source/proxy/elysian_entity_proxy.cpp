/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>


/*!
    \qmltype Entity
    \since 1.5.8
    \inmodule Engine
    \brief Class representing all non-terrain Game Objects.

    \l{Entity} is the central class for representing any game object that inhabits an \l{Area}.
    An Entity can be a player, a warp, an NPC, an enemy, a bookcase, an invisible
    trigger region, a particle emitter, a dynamic light, a positional audio source,
    and virtually anything else.

    \section2 Entities as a Collection of Components
    Entities are an aggregate of Components, which may either be built into the engine
    or created custom as Behaviors. By attaching Components to an Entity, you are
    able to add to its run-time capabilities and behaviors. For example, adding a Renderable
    Component to an Entity makes it visible as either a rectangle or sprite. Adding a
    Collidable Component to an Entity makes it solid in the game world. Adding an AudioSource
    to an Entity gives it the ability to emit 3D positional audio. An Entity is basically a blank
    slate from which any non-tile game object can be constructed.

    Another powerful aspect of the Entity/Component architecture is the fact that these Components
    can be added and removed to modify the characteristics of an Entity at run-time, rather than having
    to statically define them at compile-time through the use of inheritance, as in traditional engines.
    This means the player could dyamically turn into a warp that swallows up the rest of the party
    and shits out a million particles with a crazy flash of dynamic light before morphing back into
    himself by simply adding and removing components... because why the fuck not?

    \section2 Built-in Components
    The following is a table of all the Components built into the engine and the capabilities they add
    to an Entity when attached.

    \table
    \header
        \li \l{Component} Type
        \li Capabilities
    \row
        \li \l{Renderable}
        \li Draws the Entity as a sprite or quad.
    \row
        \li \l{Skeleton}
        \li Draws the Entity as a Spine Skeleton with animation support.
    \row
        \li \l{Collidable}
        \li Gives the Entity solidity.
    \row
        \li \l{RigidBody}
        \li Gives the Entity rigid body physical traits and adds it to the physics simulator.
    \row
        \li \l{AudioSource}
        \li Allows the Entity to play positional sounds in 3D space.
    \row
        \li \l{Character}
        \li Gives the Entity the ability to jump, pick up objects, and other characteristics common to players, NPCs, or enemies.
    \row
        \li \l{Combat}
        \li Gives the Entity battle stats and allows it to engage in battle with other Entities.
    \row
        \li \l{Examinable}
        \li Gives the Entity dialog or a description.
    \row
        \li \l{LightSource}
        \li Gives the Entity a point, spot, or directional dynamic light.
    \row
        \li \l{ParticleEmitter}
        \li Allows the Entity to spawn particles for special effects.
    \row
        \li \l{Warp}
        \li Makes the Entity transport the player from one location to another upon collision.
    \endtable

    \section2 Behaviors as Custom Components
    \l{Behavior} is a special type of dynamic Component which allows you to create your own custom
    Components and extend Entity functionality beyond what was built into the engine. A \l{LuaBehavior}
    is a special kind of \l{Behavior} which is implemented within a Lua script, allowing you to use
    the Lua scriptig language to extend the engine. See the \l{Behavior} and \l{LuaBehavior} documentation
    for more details.

    \sa Component, Renderable, Collidable, Skeleton, RigidBody, Character, AudioSource, Combat, Examinable,
    LightSource, ParticleEmitter, Warp, Behavior, LuaBehavior
*/

/*!
 * \qmlproperty InstanceTransform Entity::transform
 *
 * \sa getTransform(), setTransform()
 */

/*!
 * \qmlproperty string Entity::name
 *
 * \sa getName(), setName()
 */
/*!
 * \qmlproperty string Entity::dispName
 *
 * \sa getDispName(), setDispName()
 */
/*!
 * \qmlproperty int Entity::elev
 *
 * \sa getElev(), setElev()
 */
/*!
 * \qmlproperty int Entity::planeType
 *
 * \sa getPlaneType(), setPlaneType()
 */
/*!
 * \qmlproperty bool Entity::shadowVolume
 *
 * \sa isShadowVolume(), setShadowVolume()
 */
/*!
 * \qmlproperty bool Entity::persistent
 *
 * \sa isPersistent(), setPersistent()
 */
/*!
 * \qmlproperty Renderable Entity::renderable
 *
 * \sa getRenderable(), setRenderable()
 */
/*!
 * \qmlproperty Collidable Entity::collidable
 *
 * \sa getCollidable(), setCollidable()
 */
/*!
 * \qmlproperty RigidBody Entity::rigidBody
 *
 * \sa getRigidBody(), setRigidBody()
 */
/*!
 * \qmlproperty Character Entity::character
 *
 * \sa getCharacter(), setCharacter()
 */
/*!
 * \qmlproperty Warp Entity::warp
 *
 * \sa getWarp(), setWarp()
 */
/*!
 * \qmlproperty Skeleton Entity::skeleton
 *
 * \sa getSkeleton(), setSkeleton()
 */
/*!
 * \qmlproperty LightSource Entity::lightSource
 *
 * \sa getLightSource(), setLightSource()
 */
/*!
 * \qmlproperty AudioSource Entity::audioSource
 *
 * \sa getAudioSource(), setAudioSource()
 */
/*!
 * \qmlproperty Examinable Entity::examinable
 *
 * \sa getExaminable(), setExaminable()
 */
/*!
 * \qmlproperty Combat Entity::combat
 *
 * \sa getCombat(), setCombat()
 */
/*!
 * \qmlproperty ParticleEmitter Entity::particleEmitter
 *
 * \sa getParticleEmitter(), setParticleEmitter()
 */
/*!
 * \qmlproperty Behavior Entity::behaviors
 *
 * \sa getBehaviorByIndex(), setBehaviorByIndex()
 */








/*!
 * \qmlmethod InstanceTransform& Entity::getTransform()
 *
 * Returns a reference to the entity's transform.
 */
/*!
 * \qmlmethod void Entity::setTransform(Lua_table_ref valueTable)
 *
 * Sets the entity's transform to the value supplied in valueTable.
 */
/*!
 * \qmlmethod bool Entity::isShadowVolume()
 *
 * Returns true or false depending on whether the entity casts shadows from dynamic lights or not.
 */
/*!
 * \qmlmethod void Entity::setShadowVolume(bool value)
 *
 * Sets whether the current entity casts shadows from dynamic light sources or not.
 */
/*!
 * \qmlmethod gyro.RECT_PLANE_TYPE Entity::getPlaneType()
 *
 * Returns the plane the current entity is tied to, as one of the faces of a cube.
 */
/*!
 * \qmlmethod void Entity::setPlaneType(gyro::RECT_PLANE_TYPE plane)
 *
 * Sets the plane the current entity is tied to, as one of the faces of a cube.
 */
/*!
 * \qmlmethod bool Entity::isPersistent()
 *
 * Returns true if the entity is to be loaded and saved as part of the current area.
 */
/*!
 * \qmlmethod void Entity::setPersistent(bool value)
 *
 * Flags the entity for being saved and loaded as part of the current area or not.
 */
/*!
 * \qmlmethod bool Entity::isActive()
 *
 * Returns whether the current entity is active within an area or not
 */


/*!
 * \qmlmethod void Entity::move(GYVector2& translationVec)
 *
 * Translates the entity by the provided vector, using a walking animation if applicable.
 */
/*!
 * \qmlmethod void Entity::setName(string name)
 *
 * Sets the entity's name to the provided name. The engine requires these names to be unique, so it will append
 * a number to the end if a duplicate exists when adding to the area.
 */
/*!
 * \qmlmethod string Entity::getName()
 *
 * Returns the (unique) name of the entity
 */
/*!
 * \qmlmethod void Entity::setDispName(string name)
 *
 * Sets the entity's display name to the provided name. This is what is shown as in-game text and does not have to be unique.
 */
/*!
 * \qmlmethod string Entity::getDispName()
 *
 * Returns the displayName of the entity.
 */
/*!
 * \qmlmethod void Entity::setElev(int elev)
 *
 * Sets the entity's elevation to "elev," and the entity's transform's position's z coordinate to elev*32.
 */
/*!
 * \qmlmethod int Entity::getElev()
 *
 * Returns the elevation of the current entity, equal to its transform's position's z / 32.
 */
/*!
 * \qmlmethod Renderable* Entity::addRenderable()
 *
 * Creates and adds a Renderable component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod Collidable* Entity::addCollidable()
 *
 * Creates and adds a Collidable component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod RigidBody* Entity::addRigidBody()
 *
 * Creates and adds a RigidBody component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod Character* Entity::addCharacter()
 *
 * Creates and adds a Character component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod Examinable* Entity::addExaminable()
 *
 * Creates and adds an Examinable component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod LightSource* Entity::addLightSource()
 *
 * Creates and adds a LightSource component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod AudioSource* Entity::addAudioSource()
 *
 * Creates and adds an AudioSource component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod ParticleEmitter* Entity::addParticleEmitter()
 *
 * Creates and adds a ParticleEmitter component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod Combat* Entity::addCombat()
 *
 * Creates and adds a Combat component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod Warp* Entity::addWarp()
 *
 * Creates and adds a Warp component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod Skeleton* Entity::addSkeleton()
 *
 * Creates and adds a Skeleton component to the entity (if one doesn't already exist) and returns a reference to the component.
 */
/*!
 * \qmlmethod LuaBehavior* Entity::addLuaBehavior(string scriptPath, Lua_table_ref params)
 *
 * Creates and adds a LuaBehavior component to the entity (if one doesn't already exist), with the provided parameter table, instantiates the behavior script specified at "scriptPath," and returns a reference to the component.
 */


/*!
 * \qmlmethod Renderable* Entity::getRenderable()
 *
 * Returns the entity's attached renderable component or nil if there isn't one.
 */
/*!
 * \qmlmethod Collidable* Entity::getCollidable()
 *
 * Returns the entity's attached Collidable component or nil if there isn't one.
 */
/*!
 * \qmlmethod RigidBody* Entity::getRigidBody()
 *
 * Returns the entity's attached RigidBody component or nil if there isn't one.
 */
/*!
 * \qmlmethod Character* Entity::getCharacter()
 *
 * Returns the entity's attached Character component or nil if there isn't one.
 */
/*!
 * \qmlmethod Examinable* Entity::getExaminable()
 *
 * Returns the entity's attached Examinable component or nil if there isn't one.
 */
/*!
 * \qmlmethod LightSource* Entity::getLightSource()
 *
 * Returns the entity's attached LightSource component or nil if there isn't one.
 */
/*!
 * \qmlmethod AudioSource* Entity::getAudioSource()
 *
 * Returns the entity's attached AudioSource component or nil if there isn't one.
 */
/*!
 * \qmlmethod ParticleEmitter* Entity::getParticleEmitter()
 *
 * Returns the entity's attached ParticleEmitter component or nil if there isn't one.
 */
/*!
 * \qmlmethod Combat* Entity::getCombat()
 *
 * Returns the entity's attached Combat component or nil if there isn't one.
 */
/*!
 * \qmlmethod Warp* Entity::getWarp()
 *
 * Returns the entity's attached Warp component or nil if there isn't one.
 */
/*!
 * \qmlmethod Skeleton* Entity::getSkeleton()
 *
 * Returns the entity's attached Skeleton component or nil if there isn't one.
 */
/*!
 * \qmlmethod LuaBehavior* Entity::getLuaBehavior(string scriptPath)
 *
 *  Returns the LuaBehavior component instantiated from the given script.
 */


/*!
 * \qmlmethod void Entity::setRenderable(Lua_table_ref valueTable)
 *
 * Creates a new Renderable component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setCollidable(Lua_table_ref valueTable)
 *
 * Creates a new Collidable component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setRigidBody(Lua_table_ref valueTable)
 *
 * Creates a new RigidBody component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setCharacter(Lua_table_ref valueTable)
 *
 * Creates a new Character component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setExaminable(Lua_table_ref valueTable)
 *
 * Creates a new Examinable component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setLightSource(Lua_table_ref valueTable)
 *
 * Creates a new LightSource component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setAudioSource(Lua_table_ref valueTable)
 *
 * Creates a new AudioSource component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setParticleEmitter(Lua_table_ref valueTable)
 *
 * Creates a new ParticleEmitter component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setCombat(Lua_table_ref valueTable)
 *
 * Creates a new Combat component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setWarp(Lua_table_ref valueTable)
 *
 * Creates a new Warp component and sets it equal to valueTable.
 */
/*!
 * \qmlmethod void Entity::setSkeleton(Lua_table_ref valueTable)
 *
 * Creates a new Skeleton component and sets it equal to valueTable.
 */


/*!
 * \qmlmethod void Entity::removeRenderable()
 *
 * Removes and deletes the attached Renderable component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeCollidable()
 *
 * Removes and deletes the attached Collidable component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeRigidBody()
 *
 * Removes and deletes the attached RigidBody component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeCharacter()
 *
 * Removes and deletes the attached Character component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeExaminable()
 *
 * Removes and deletes the attached Examinable component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeLightSource()
 *
 * Removes and deletes the attached LightSource component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeAudioSource()
 *
 * Removes and deletes the attached AudioSource component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeParticleEmitter()
 *
 * Removes and deletes the attached ParticleEmitter component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeCombat()
 *
 * Removes and deletes the attached Combat component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeWarp()
 *
 * Removes and deletes the attached Warp component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeSkeleton()
 *
 * Removes and deletes the attached Skeleton component (if it exists).
 */
/*!
 * \qmlmethod void Entity::removeLuaBehavior(string scriptPath)
 *
 * Removes and deletes the attached LuaBehavior instantiated from the LuaScript at "scriptPath."
 */
/*!
 * \qmlmethod Behavior* Entity::getBehaviorByIndex(int index)
 *
 * Returns the Behavior component at the specified index or nil if there isn't one.
 */
/*!
 * \qmlmethod int Entity::getBehaviorCount()
 *
 * Returns the number of Behavior components attached to the current entity.
 */
/*!
 * \qmlmethod void Entity::setBehaviorByIndex(int index, Lua_table_ref valueTable)
 *
 * Sets the Behavior at the specified index to the valueTable or inserts a new one if the specified index doesn't exist.
 */




























////
namespace elysian::lua {
using namespace elysian;
using RenderableObject = elysian::RenderableObject;
using Entity = elysian::Entity;

void registerRenderableObject(sol::state_view state) {
    if (state["RenderableObject"].valid()) return;
    using T = elysian::RenderableObject;
    auto type = registerType<T>(state, "RenderableObject");
    type["getTransform"] = method<T, InstanceTransform&()>(&T::getTransform);
}

void registerEntity(sol::state_view state) {
    if (state["Entity"].valid()) return;
    registerRenderableObject(state);
    using T = elysian::Entity;
    auto type = registerType<T, elysian::RenderableObject>(state, "Entity");
    type["new"] = sol::factories(factory<T>(), factory<T, Entity*>());
    type["move"] = method<T, void(const GYVector2&)>(&T::move);
    type["setName"] = method<T, void(const char*)>(&T::setName);
    type["getName"] = method<T, const char *() const>(&T::getName);
    type["setElev"] = method<T, void(const unsigned char)>(&T::setElev);
    type["getElev"] = method<T, unsigned char() const>(&T::getElev);
    type["addRenderable"] = method<T, Renderable*()>(&T::addRenderable);
    type["addCollidable"] = method<T, Collidable*()>(&T::addCollidable);
    type["addRigidBody"] = method<T, RigidBody*()>(&T::addRigidBody);
    type["addCharacter"] = method<T, Character*()>(&T::addCharacter);
    type["addWarp"] = method<T, Warp*()>(&T::addWarp);
    type["addAudioSource"] = method<T, AudioSource*()>(&T::addAudioSource);
    type["addLightSource"] = method<T, LightSource*()>(&T::addLightSource);
    type["addExaminable"] = method<T, Examinable*()>(&T::addExaminable);
    type["addCombat"] = method<T, Combat*()>(&T::addCombat);
    type["addParticleEmitter"] = method<T, ParticleEmitter*()>(&T::addParticleEmitter);
    type["addSkeleton"] = method<T, Skeleton*()>(&T::addSkeleton);
    type["addBehavior"] = [](T& self, sol::object behavior) { return self.addBehavior(take<Behavior>(behavior)); };
    type["addLuaBehavior"] = method<T, LuaBehavior*(const char*, sol::object)>(&T::addLuaBehavior);
    type["getRenderable"] = method<T, Renderable*() const>(&T::getRenderable);
    type["getCollidable"] = method<T, Collidable*() const>(&T::getCollidable);
    type["getRigidBody"] = method<T, RigidBody*() const>(&T::getRigidBody);
    type["getCharacter"] = method<T, Character*() const>(&T::getCharacter);
    type["getWarp"] = method<T, Warp*() const>(&T::getWarp);
    type["getAudioSource"] = method<T, AudioSource*() const>(&T::getAudioSource);
    type["getLightSource"] = method<T, LightSource*() const>(&T::getLightSource);
    type["getExaminable"] = method<T, Examinable*() const>(&T::getExaminable);
    type["getCombat"] = method<T, Combat*() const>(&T::getCombat);
    type["getParticleEmitter"] = method<T, ParticleEmitter*() const>(&T::getParticleEmitter);
    type["getSkeleton"] = method<T, Skeleton*() const>(&T::getSkeleton);
    type["getBehavior"] = method<T, Behavior*(const char*) const>(&T::getBehavior);
    type["getLuaBehavior"] = method<T, LuaBehavior*(const char*) const>(&T::getLuaBehavior);
    type["removeRenderable"] = method<T, void()>(&T::removeRenderable);
    type["removeCollidable"] = method<T, void()>(&T::removeCollidable);
    type["removeRigidBody"] = method<T, void()>(&T::removeRigidBody);
    type["removeCharacter"] = method<T, void()>(&T::removeCharacter);
    type["removeWarp"] = method<T, void()>(&T::removeWarp);
    type["removeAudioSource"] = method<T, void()>(&T::removeAudioSource);
    type["removeLightSource"] = method<T, void()>(&T::removeLightSource);
    type["removeExaminable"] = method<T, void()>(&T::removeExaminable);
    type["removeCombat"] = method<T, void()>(&T::removeCombat);
    type["removeParticleEmitter"] = method<T, void()>(&T::removeParticleEmitter);
    type["removeSkeleton"] = method<T, void()>(&T::removeSkeleton);
    type["removeBehavior"] = method<T, void(const char*)>(&T::removeBehavior);
    type["removeLuaBehavior"] = method<T, void(const char*)>(&T::removeLuaBehavior);
    type["getSelfTable"] = method<T, sol::object&()>(&T::getSelfTable);
    type["setSelfTable"] = method<T, void(sol::object)>(&T::setSelfTable);
    type["setShadowDepth"] = method<T, void(float)>(&T::setShadowDepth);
    type["setShadowVolume"] = method<T, void(bool)>(&T::setShadowVolume);
    type["setPlaneType"] = method<T, void(GY_RECT_PLANE_TYPE)>(&T::setPlaneType);
    type["getPlaneType"] = method<T, GY_RECT_PLANE_TYPE() const>(&T::getPlaneType);
    type["getShadowDepth"] = method<T, float() const>(&T::getShadowDepth);
    type["isShadowVolume"] = method<T, bool() const>(&T::isShadowVolume);
    type["setTransform"] = method<T, void(sol::object)>(&T::setTransform);
    type["setRenderable"] = method<T, void(sol::object)>(&T::setRenderable);
    type["setCollidable"] = method<T, void(sol::object)>(&T::setCollidable);
    type["setRigidBody"] = method<T, void(sol::object)>(&T::setRigidBody);
    type["setWarp"] = method<T, void(sol::object)>(&T::setWarp);
    type["setCharacter"] = method<T, void(sol::object)>(&T::setCharacter);
    type["setAudioSource"] = method<T, void(sol::object)>(&T::setAudioSource);
    type["setLightSource"] = method<T, void(sol::object)>(&T::setLightSource);
    type["setExaminable"] = method<T, void(sol::object)>(&T::setExaminable);
    type["setCombat"] = method<T, void(sol::object)>(&T::setCombat);
    type["setParticleEmitter"] = method<T, void(sol::object)>(&T::setParticleEmitter);
    type["setSkeleton"] = method<T, void(sol::object)>(&T::setSkeleton);
    type["setPersistent"] = method<T, void(const bool)>(&T::setPersistent);
    type["isPersistent"] = method<T, bool() const>(&T::isPersistent);
    type["isActive"] = method<T, bool() const>(&T::isActive);
    type["getDispName"] = method<T, const char*() const>(&T::getDispName);
    type["setDispName"] = method<T, void(const char*)>(&T::setDispName);
    type["getBehaviorByIndex"] = method<T, Behavior*(unsigned) const>(&T::getBehaviorByIndex);
    type["getBehaviorCount"] = method<T, unsigned() const>(&T::getBehaviorCount);
    type["setBehaviorByIndex"] = method<T, void(unsigned, sol::object)>(&T::setBehaviorByIndex);
    property(type, "name", method<T, const char *() const>(&T::getName), method<T, void(const char*)>(&T::setName));
    property(type, "dispName", method<T, const char*() const>(&T::getDispName), method<T, void(const char*)>(&T::setDispName));
    property(type, "elev", method<T, unsigned char() const>(&T::getElev), method<T, void(const unsigned char)>(&T::setElev));
    property(type, "planeType", method<T, GY_RECT_PLANE_TYPE() const>(&T::getPlaneType), method<T, void(GY_RECT_PLANE_TYPE)>(&T::setPlaneType));
    property(type, "shadowVolume", method<T, bool() const>(&T::isShadowVolume), method<T, void(bool)>(&T::setShadowVolume));
    property(type, "persistent", method<T, bool() const>(&T::isPersistent), method<T, void(const bool)>(&T::setPersistent));
    property(type, "transform", method<T, InstanceTransform&()>(&T::getTransform), method<T, void(sol::object)>(&T::setTransform));
    property(type, "renderable", method<T, Renderable*() const>(&T::getRenderable), method<T, void(sol::object)>(&T::setRenderable));
    property(type, "skeleton", method<T, Skeleton*() const>(&T::getSkeleton), method<T, void(sol::object)>(&T::setSkeleton));
    property(type, "collidable", method<T, Collidable*() const>(&T::getCollidable), method<T, void(sol::object)>(&T::setCollidable));
    property(type, "rigidBody", method<T, RigidBody*() const>(&T::getRigidBody), method<T, void(sol::object)>(&T::setRigidBody));
    property(type, "examinable", method<T, Examinable*() const>(&T::getExaminable), method<T, void(sol::object)>(&T::setExaminable));
    property(type, "warp", method<T, Warp*() const>(&T::getWarp), method<T, void(sol::object)>(&T::setWarp));
    property(type, "character", method<T, Character*() const>(&T::getCharacter), method<T, void(sol::object)>(&T::setCharacter));
    property(type, "lightSource", method<T, LightSource*() const>(&T::getLightSource), method<T, void(sol::object)>(&T::setLightSource));
    property(type, "audioSource", method<T, AudioSource*() const>(&T::getAudioSource), method<T, void(sol::object)>(&T::setAudioSource));
    property(type, "particleEmitter", method<T, ParticleEmitter*() const>(&T::getParticleEmitter), method<T, void(sol::object)>(&T::setParticleEmitter));
    property(type, "combat", method<T, Combat*() const>(&T::getCombat), method<T, void(sol::object)>(&T::setCombat));
    viewProperty(type, array<T>("behaviors", method<T, Behavior*(unsigned) const>(&T::getBehaviorByIndex), method<T, void(unsigned, sol::object)>(&T::setBehaviorByIndex), method<T, unsigned() const>(&T::getBehaviorCount)));
    inheritProperties<T, elysian::RenderableObject>();
    finish<T>(type, "Entity", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}
}
