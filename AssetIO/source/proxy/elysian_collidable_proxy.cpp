#include <AssetIO/proxy/elysian_collidable_proxy.hpp>
/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_entity_proxy.hpp>


/*!
    \qmltype CollisionFlags
    \since 1.6.0
    \inmodule Engine
    \brief Flags used by Engine to determine whether or not two Colliders can collide.

    Before the Engine checks for collision between two Colliders, it always checks their flags first to see
    if they can even collide with each other. This is an optimization that allows you to specify which groups
    of entities can collide with other groups of Entities to save on expensive collision checking. For example,
    the engine should not have to check if enemy or NPC Entities collide with Warps. This is also useful when
    using the Lua collision triggers such as onCollide(), onCollisionEnter(), and onCollisionExit() to trigger
    an event when two objects collide, as you can specify exactly which Colliders you are interested in rather
    than having to filter out collisions within Lua code.

    \section2 Determining Collision Checks
    The engine determines whether or not it should bother checking if Collidable A collided with Collidable B by
    performing a bitwise AND operation between Collidable A's typeAgainst property and Collidable B's type property.
    \code
    if(entityA.collidable.attrib.flags.typeAgainst.value &
        entityB.collidable.attrib.flags.type.value) then
        --perform collision check
    end
    \endcode

    \section2 Bit Field Values
    Bit field 1 in type and typeAgainst is always reserved for tiles, so a Collidable with typeAgainst.bitFields[1] set
    to true, means the Collidable can collide with solid Tiles in the map. The other bit fields are not reserved by the
    engine, but a good practice would be to treat them uniformly in every Area. For example, bitField[2] could be for
    party members, while bitField[3] could be for enemies, and bitField[4] could be for NPCs. Choose a convention and
    enforce it throughout the Project.

    \sa BitMask32, Collidable
*/

/*!
 * \qmlproperty BitMask32 CollisionFlags::type
 *
 * 32-bit bitmask representing a Collidable's collision type. This type is referenced to determine whether other
 * Collidables can collide with this one or not.
 * \sa getType(), setType()
 */
/*!
 * \qmlproperty BitMask32 CollisionFlags::typeAgainst
 *
 * 32-bit bitmask representing the types of Collidables this Collidable can collide with.
 * \sa getTypeAgainst(), setTypeAgainst()
 */
/*!
 * \qmlmethod BitMask32* CollisionFlags::getType()
 *
 * Returns the bitmask representing the Collider's type.
 * \sa setType()
 */
/*!
 * \qmlmethod void CollisionFlags::setType(BitMask32 type)
 *
 * Sets the Collider's type bitmask to \a{type}.
 * \sa getType()
 */
/*!
 * \qmlmethod BitMask32* CollisionFlags::getTypeAgainst()
 *
 * Returns the bitmask representing the types of Colliders this Colider can collide with.
 * \sa setTypeAgainst()
 */
/*!
 * \qmlmethod void CollisionFlags::setTypeAgainst(BitMask32 type)
 *
 * Sets the Collider's typeAgainst bitmask to \a{type}.
 * \sa getTypeAgainst()
 */

/*!
    \qmltype CollidableStaticAttrib
    \since 1.6.0
    \inmodule Engine
    \brief Collidable attributes used by the rigid body physics engine.

    CollidableStaticAttrib is a collection of attributes that is used internally by the physics engine
    to simulate friction and rotations of a moving Entity. It is also used in calculations for energy transfer
    and penetration resolution between two colliding bodies.

    \sa Collidable, CollidableAttrib
*/
/*!
 * \qmlproperty float CollidableStaticAttrib::invMass
 *
 * Floating point value representing a Collidable's inverse mass. Mass is stored inversely so that 0 can be used
 * to represent an immovable object with infinite mass.
 * \sa getInverseMass(), setInverseMass()
 */
/*!
 * \qmlproperty float CollidableStaticAttrib::coeffOfRest
 *
 * Floating point value representing the amount of energy that is conserved when the object collides with another.
 * A value of 1 would make the object continue bouncing infinitely, as in PONG, while a value of 0 would make the
 * object stop instantly after a collision.
 * \sa getCoeffOfRest(), setCoeffOfRest()
 */
/*!
 * \qmlproperty float CollidableStaticAttrib::invMomOfInertia
 *
 * Floating point value representing a Collider's inverse moment of inertia. This is how easily the object tends to
 * rotate when a force is applied to it. A value of 0 means the object cannot rotate. A value of 1 means all
 * energy tranferred to the object results in angular rotation, while the object's position never moves (like a fan).
 * \sa getCoeffOfRest(), setCoeffOfRest()
 */
/*!
 * \qmlproperty float CollidableStaticAttrib::angDamping
 *
 * Floating point value representing a Collider's angular damping. This is how much rotational energy is lost every
 * frame. You can think of this as rotational friction. A value of 0 means the object will never rotate, because it
 * instantly loses all rotational energy. A value of 1 means the object will never stop rotation once it starts,
 * because it never loses rotational energy.
 * \sa getAngDamping(), setAngDamping()
 */
/*!
 * \qmlproperty float CollidableStaticAttrib::zCoeffOfRest
 *
 * Floating point value representing the Collidable's coefficient of restitution on the Z axis. This is how much
 * energy is conserved when the object collides with another on the Z axis (either landing on another object or
 * another object landing on it). A value of 1 would make this object bounce up and down infinitely, while a value
 * of 0 would make it stop the instant it lands on another object.
 * \sa getCoeffOfRest(), setCoeffOfRest()
 */
/*!
 * \qmlproperty Vector3 CollidableStaticAttrib::damping
 *
 * 3D vector representing the linear damping of the Collidable. This is how much energy is lost on each axis every
 * frame. A value of <1, 0.5, 0> would mean that the object never stops moving on the X axis, the object loses half
 * of its energy on the Y axis every frame, and the object never moves on the Z axis, because it instantly loses all
 * of its energy every frame.
 * \sa getDampingVec(), setDampingVec()
 */
/*!
 * \qmlmethod float CollidableStaticAttrib::getInverseMass()
 *
 * Returns the inverse mass of the Collidable.
 * \sa invMass, setInverseMass()
 */
/*!
 * \qmlmethod void CollidableStaticAttrib::setInverseMass(float value)
 *
 * Sets the inverse mass of the Collidable to \a{value}.
 * \sa invMass, getInverseMass()
 */
/*!
 * \qmlmethod float CollidableStaticAttrib::getCoeffOfRest()
 *
 * Returns the coefficient of restitution of the Collidable.
 * \sa coeffOfRest, setCoeffOfRest()
 */
/*!
 * \qmlmethod void CollidableStaticAttrib::setCoeffOfRest(float value)
 *
 * Sets the coefficient of restition of the Collidable to \a{value}.
 * \sa coeffOfRest, getCoeffOfRest()
 */
/*!
 * \qmlmethod float CollidableStaticAttrib::getInvMomOfInertia()
 *
 * Retuns the inverse moment of inertia of the Collidable.
 * \sa invMomOfInertia, setInvMomOfInertia()
 */
/*!
 * \qmlmethod void CollidableStaticAttrib::setInvMomOfInertia(float val)
 *
 * Sets the inverse moment of inertia of the Collidable to \a{val}.
 * \sa invMomOfInertia, getInvMomOfInertia()
 */
/*!
 * \qmlmethod float CollidableStaticAttrib::getAngDamping()
 *
 * Retuns the angular damping of the Collidable.
 * \sa angDamping, setAngDamping()
 */
/*!
 * \qmlmethod void CollidableStaticAttrib::setAngDamping(float val)
 *
 * Sets the angular damping of the Collidable to \a{val}.
 * \sa angDamping, getAngDamping()
 */
/*!
 * \qmlmethod float CollidableStaticAttrib::getZCoeffOfRest()
 *
 * Returns the coefficient of restitution for the Collidable on the Z axis.
 * \sa zCoeffOfRest, setZCoeffOfRest()
 */
/*!
 * \qmlmethod void CollidableStaticAttrib::setZCoeffOfRest(float val)
 *
 * Sets the cofficient of restitution for the Collidable on the Z axis to \a{val}.
 * \sa zCoeffOfRest, getZCoeffOfRest()
 */
/*!
 * \qmlmethod Vector3 CollidableStaticAttrib::getDampingVec()
 *
 * Returns the 3D vector representing the damping coefficients of the Collidable on each axis.
 * \sa damping, setDampingVec()
 */
/*!
 * \qmlmethod void CollidableStaticAttrib::setDampingVec(Vector3 vec)
 *
 * Sets the 3D vector representing the damping coefficients of the Collidable on each axis to \a{vec}.
 * \sa damping, getDampingVec()
 */

/*!
    \qmltype CollidableAttrib
    \since 1.6.0
    \inmodule Engine
    \brief Collection of attributes to be shared among similar Colliders.

    CollidableAttrib is a collection of various attributes contained by a Collidable that is used by the
    CollisionSystem during movement and upon collision with the Collidable. It also provides a way for multiple
    similar Colliders to share the same attributes to reduce on run-time memory overhead and save time when
    configuring Colliders.

    Data represented by CollidableAttrib:
    \list
    \li colliderType - rectangle, triangle, circle
    \li relPos, relSize - collider transform
    \li staticAttrib - used by the physics engine (mass, moment of inertia, etc)
    \li \l{flags} - flags which objects the Collidable can interact with
    \endlist

    \sa Collidable, CollidableStaticAttrib, CollisionFlags, CollisionSystem
*/

/*!
 * \qmlproperty COLLIDER_TYPE CollidableAttrib::colliderType
 *
 * Enumeration representing the geometric shape of the Collidable's collider.
 * \sa gyro::COLLIDER_TYPE, getColliderType(), setColliderType()
 */
/*!
 * \qmlproperty Vector3 CollidableAttrib::relPos
 *
 * 3D vector representing the position of the collider relative to the Entity's position.
 * <0, 0, 0> puts the collider at the center of the Entity.
 * \sa getRelPosVec(), setRelPosVec()
 */
/*!
 * \qmlproperty Vector3 CollidableAttrib::relSize
 *
 * 3D vector representing the size of the collider relative to the Entity's size.
 * <1, 1, 1> sets the collider to the same size as the Entity.
 * \sa getRelSizeVec(), setRelSizeVec()
 */
/*!
 * \qmlproperty CollidableStaticAttrib* CollidableAttrib::staticAttrib
 *
 * Object representing physical attributes of the Collidable such as mass, moment of inertia,
 * and damping, for use by the physics engine.
 * \sa CollidableStaticAttrib, getStaticAttrib(), setStaticAttrib()
 */
/*!
 * \qmlproperty CollisionFlags CollidableAttrib::flags
 *
 * Collection of flags used by the CollisionSystem to determine which objects can collide with
 * the Collidable.
 * \sa CollisionFlags, getFlags(), setFlags()
 */

/*!
 * \qmlmethod COLLIDER_TYPE CollidableAttrib::getColliderType()
 *
 * Sets the 3D vector representing the damping coefficients of the Collidable on each axis to \a{vec}.
 * \sa gyro::COLLIDER_TYPE, colliderType, setColliderType()
 */
/*!
 * \qmlmethod void CollidableAttrib::setColliderType(COLLIDER_TYE type)
 *
 * Sets the colliderType of the Collidable to \a{type}.
 * \sa gyro::COLLIDER_TYPE, colliderType, getColliderType()
 */
/*!
 * \qmlmethod Vector3 CollidableAttrib::getRelPosVec()
 *
 * Returns the value of relPos.
 * \sa relPos, setRelPosVec()
 */
/*!
 * \qmlmethod void CollidableAttrib::setRelPosVec(Vector3 pos)
 *
 * Sets the value of relPos to \a{pos}.
 * \sa relPos, getRelPosVec()
 */
/*!
 * \qmlmethod Vector3 CollidableAttrib::getRelSizeVec()
 *
 * Returns the value of relSize.
 * \sa relSize, setRelSizeVec()
 */
/*!
 * \qmlmethod void CollidableAttrib::setRelSizeVec(Vector3 size)
 *
 * Sets the value of relSize to \a{size}.
 * \sa relSize, getRelSizeVec()
 */
/*!
 * \qmlmethod CollidableStaticAttrib* CollidableAttrib::getStaticAttrib()
 *
 * Returns a reference to staticAttrib.
 * \sa staticAttrib, CollidableStaticAttrib, setStaticAttrib()
 */
/*!
 * \qmlmethod void CollidableAttrib::setStaticAttrib(Lua_table table)
 *
 * Initializes staticAttrib to the value of the \a{table} Lua table.
 * \sa staticAttrib, CollidableStaticAttrib, getStaticAttrib()
 */
/*!
 * \qmlmethod CollisionFlags* CollidableAttrib::getFlags()
 * Returns a reference to flags.
 * \sa flags, CollisionFlags, setFlags()
 */
/*!
 * \qmlmethod void CollidableAttrib::setFlags(Lua_table table)
 * Initializes flags to the value of the \a{table} Lua table.
 * \sa flags, CollisionFlags, getFlags()
 */
/*!
 * \qmlmethod void CollidableAttrib::setRelPos(float x, float y, float z)
 * Sets the value of relPos to the vector provided by <\a{x}, \a{y}, \a{z}>.
 * \sa relPos, setRelPosVec, getRelPosVec
 */
/*!
 * \qmlmethod void CollidableAttrib::setRelSize(float x, float y, float z)
 * Sets the value of relSize to the vector provided by <\a{x}, \a{y}, \a{z}>.
 * \sa relSize, setRelSizeVec, getRelSizeVec
 */

/*!
    \qmltype CollisionContact
    \since 1.6.0
    \inmodule Engine
    \brief Represents a contact from a collision between two Collidables.

    The CollisionSystem creates a CollisionContact object when a collision is detected between two
    Collidables. A CollisionContact contains references to the two bodies that were involved in the
    contact along with the point of contact and various other derived data used by the CollisionSystem in
    calculations to resolve the collision.

    \sa Collidable, CollisionSystem
*/
/*!
 * \qmlmethod Collidable* CollisionContact::getCollidable(int index)
 * Returns a reference to one of the two Collidables involved in the collision. When \a{index} is 0,
 * Collidable A is returned. When \a{index} is 1, Collidable B is returned. All derived data are relative
 * to Collidable A. In the case of an Entity colliding with terrain, Collidable A will be the Entity's
 * Collidable, and Collidable B will be nil.
 */
/*!
 * \qmlmethod void CollisionContact::getWorldPos(Vector2* pos)
 * Sets \a{pos} equal to the world location of the CollisionContact.
 */
/*!
 * \qmlmethod void CollisionContact::getNormal(Vector2* normal)
 * Sets \a{normal} equal to the normal vector of the CollisionContact. The normal a normalized unit vector
 * representing the direction of the collision from Body A's perspective.
 * \sa getDepth()
 */
/*!
 * \qmlmethod void CollisionContact::getObjPos(int index, Vector2& relPos)
 * Sets \a{relPos} equal to the position of the Collidable given by \a{index} relative to the point of contact.
 */
/*!
 * \qmlmethod float CollisionContact::getSeparatingVel()
 * Retuns the rate at which the two Collidables involved in the CollisionContact are moving away from each other.
 * This value will be negative if the two are moving towards each other.
 * \sa getClosingVel()
 */
/*!
 * \qmlmethod float CollisionContact::getClosingVel()
 * Retuns the rate at which the two Collidables involved in the CollisionContact are moving towards each other.
 * This value will be negative if the two are moving away from each other.
 * \sa getSeparatingVel()
 */
/*!
 * \qmlmethod float CollisionContact::getDeltaVel()
 * Retuns the overall change in velocity between when the two bodies collided and when the collision is resolved.
 * \sa getSeparatingVel(), getClosingVel()
 */
/*!
 * \qmlmethod float CollisionContact::getDepth()
 * Returns the depth of penetration of the collision in the direction of the collision normal.
 * \sa getNormal()
 */

/*!
    \qmltype Collidable
    \inherits Component
    \since 1.6.0
    \inmodule Engine
    \brief Component attached to an Entity to make it a solid, collidable object.

    The Collidable component is attached to an Entity to give it collision geometry and
    solidity in the game world. The CollisionSystem inspects each Collidable which can be
    configured to check for collision between other Collidables and solid tiles on the map
    each frame. A Collidable \b{must} have a valid CollidableAttrib object set as its \l{attrib},
    if it's attached to an active Entity.

    \sa CollidableAttrib, CollisionSystem
*/
/*!
 * \qmlproperty bool Collidable::active
 *
 * Boolean property set to true if the Collidable should be used by the CollisionSystem for collision
 * checks every frame or false if not. This can be used to temporarily make an Entity solid or walkable.
 * \sa isActive(), setActive()
 */
/*!
 * \qmlproperty int Collidable::contactCount
 *
 * Read-only property containing the number of \l{CollisionContact}s (Collidables or tiles) involving the Collidable.
 * \sa getContactCount(), CollisionContact
 */
/*!
 * \qmlproperty bool Collidable::stackedOnGround
 *
 * Read-only boolean property that is set to true if the current Collidable is either resting on the ground or is
 * resting ontop of a stack of entities that is resting on the ground.
 * \sa getStackedOnGround()
 */
/*!
 * \qmlproperty bool Collidable::restingOnBody
 *
 * Read-only boolean property that is set to true if the current Collidable is resting ontop of another Collidable.
 * \sa getRestingOnBody()
 */
/*!
 * \qmlproperty bool Collidable::worldSpace
 *
 * Boolean property that is set to true if collision checks involving the Collidable are to be performed in
 * world space (default) or false if they are to be performed in screen space. Screen space collision checks are for
 * when you are checking for collision between overlay elements on a GUI rather than on the map.
 * \sa getWorldSpace(), setWorldSpace()
 */
/*!
 * \qmlproperty bool Collidable::resolvePen
 *
 * Boolean property that is set to true if collisions between the Collidable and another body should be resolved by the
 * CollisionSystem (default). If set to false, \l{CollisionContact}s are still generated, and the collision is still
 * reported, but the collision itself is ignored. This should be set to false for objects that are not solid yet are
 * still used as collision triggers, such as Warps, which don't appear solid, but colliding with them triggers a map change.
 * \sa getResolvePenetration(), setResolvePenetration()
 */
/*!
 * \qmlproperty CollidableAttrib* Collidable::attrib
 *
 * Property containing a reference to the CollidableAttrib object holding the attributes for the Collidable, or nil if
 * none exists. This value \b{must not} be equal to nil if the Entity is to be activated within an Area.
 * \sa getAttrib(), setAttrib()
 */
/*!
 * \qmlmethod void Collidable::setAttrib(CollidableAttrib* attrib)
 * Sets the \l{attrib} property to \a{attrib}.
 * \sa attrib, getAttrib()
 */
/*!
 * \qmlmethod CollidableAttrib* Collidable::getAttrib()
 * Returns the CollidableAttrib reference set as the \l{attrib} property or nil if none has been set.
 * \sa attrib, setAttrib()
 */
/*!
 * \qmlmethod bool Collidable::getResolvePenetration()
 * Returns the value of the \l{resolvePen} property.
 * \sa resolvePen, setResolvePenetration()
 */
/*!
 * \qmlmethod void Collidable::setResolvePenetration(bool value)
 * Sets the value of the \l{resolvePen} to \a{value}.
 * \sa resolvePen, getResolvePenetration()
 */
/*!
 * \qmlmethod bool Collidable::getWorldSpace()
 * Retuns the value of the worldSpace property.
 * \sa worldSpace, setWorldSpace()
 */
/*!
 * \qmlmethod void Collidable::setWorldSpace(bool value)
 * Sets the value of the worldSpace property to \a{value}.
 * \sa worldSpace, getWorldSpace()
 */
/*!
 * \qmlmethod bool Collidable::isActive()
 * Retuns the value of the \l{active} property.
 * \sa active, setActive()
 */
/*!
 * \qmlmethod void Collidable::setActive(bool value)
 * Sets the value of the \l{active} property to \a{value}.
 * \sa worldSpace, getWorldSpace()
 */
/*!
 * \qmlmethod int Collidable::getContactCount()
 * Retuns the number of bodies currently colliding with the Collidable.
 * \sa contactCount
 */
/*!
 * \qmlmethod bool Collidable::getStackedOnGround()
 * Retuns the value of the stackedOnGround property.
 * \sa stackedOnGround
 */
/*!
 * \qmlmethod bool Collidable::getRestingOnBody()
 * Retuns the value of the restingOnBody property.
 * \sa restingOnBody
 */


namespace elysian::lua {
using namespace elysian;
using CollisionFlags = elysian::CollisionFlags;
using CollidableStaticAttrib = elysian::CollidableStaticAttrib;
using CollidableAttrib = elysian::CollidableAttrib;
using CollisionContact = elysian::CollisionContact;
using Collidable = elysian::Collidable;

void registerCollisionFlags(sol::state_view state) {
    if (state["CollisionFlags"].valid()) return;
    using T = elysian::CollisionFlags;
    auto type = registerType<T>(state, "CollisionFlags");
    type["getType"] = method<T, BitMask32&()>(&T::getType);
    type["getTypeAgainst"] = method<T, BitMask32&()>(&T::getTypeAgainst);
    type["setType"] = method<T, void(const BitMask32)>(&T::setType);
    type["setTypeAgainst"] = method<T, void(const BitMask32)>(&T::setTypeAgainst);
    property(type, "type", method<T, BitMask32&()>(&T::getType), method<T, void(const BitMask32)>(&T::setType));
    property(type, "typeAgainst", method<T, BitMask32&()>(&T::getTypeAgainst), method<T, void(const BitMask32)>(&T::setTypeAgainst));
    inheritProperties<T>();
    finish<T>(type, "CollisionFlags", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}

void registerCollidableStaticAttrib(sol::state_view state) {
    if (state["CollidableStaticAttrib"].valid()) return;
    using T = elysian::CollidableStaticAttrib;
    auto type = registerType<T>(state, "CollidableStaticAttrib");
    type["setDamping"] = method<T, void(float)>(&T::setDamping);
    type["setZDamping"] = method<T, void(float)>(&T::setZDamping);
    type["setDamping3f"] = method<T, void(float, float, float)>(&T::setDamping3f);
    type["getDampingVec"] = method<T, Vector3()>(&T::getDampingVec);
    type["setDampingVec"] = method<T, void(const Vector3&)>(&T::setDampingVec);
    type["setInverseMass"] = method<T, void(const float)>(&T::setInverseMass);
    type["setCoeffOfRest"] = method<T, void(const float)>(&T::setCoeffOfRest);
    type["setInvMomOfInertia"] = method<T, void(const float)>(&T::setInvMomOfInertia);
    type["setAngDamping"] = method<T, void(const float)>(&T::setAngDamping);
    type["setZCoeffOfRest"] = method<T, void(const float)>(&T::setZCoeffOfRest);
    type["getInverseMass"] = method<T, float() const>(&T::getInverseMass);
    type["getCoeffOfRest"] = method<T, float() const>(&T::getCoeffOfRest);
    type["getInvMomOfInertia"] = method<T, float() const>(&T::getInvMomOfInertia);
    type["getAngDamping"] = method<T, float() const>(&T::getAngDamping);
    type["getZCoeffOfRest"] = method<T, float() const>(&T::getZCoeffOfRest);
    property(type, "invMass", method<T, float() const>(&T::getInverseMass), method<T, void(const float)>(&T::setInverseMass));
    property(type, "coeffOfRest", method<T, float() const>(&T::getCoeffOfRest), method<T, void(const float)>(&T::setCoeffOfRest));
    property(type, "invMomOfInertia", method<T, float() const>(&T::getInvMomOfInertia), method<T, void(const float)>(&T::setInvMomOfInertia));
    property(type, "angDamping", method<T, float() const>(&T::getAngDamping), method<T, void(const float)>(&T::setAngDamping));
    property(type, "zCoeffOfRest", method<T, float() const>(&T::getZCoeffOfRest), method<T, void(const float)>(&T::setZCoeffOfRest));
    property(type, "damping", method<T, Vector3()>(&T::getDampingVec), method<T, void(const Vector3&)>(&T::setDampingVec));
    inheritProperties<T>();
    finish<T>(type, "CollidableStaticAttrib", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}

void registerCollidableAttrib(sol::state_view state) {
    if (state["CollidableAttrib"].valid()) return;
    using T = elysian::CollidableAttrib;
    auto type = registerType<T>(state, "CollidableAttrib");
    type["new"] = sol::factories(sharedFactory<T>());
    type["getColliderType"] = method<T, GY_COLLIDER_TYPE() const>(&T::getColliderType);
    type["setColliderType"] = method<T, void(GY_COLLIDER_TYPE)>(&T::setColliderType);
    type["setRelPos"] = method<T, void(float, float, float)>(&T::setRelPos);
    type["setRelSize"] = method<T, void(float, float, float)>(&T::setRelSize);
    type["getRelPosVec"] = method<T, Vector3()>(&T::getRelPosVec);
    type["getRelSizeVec"] = method<T, Vector3()>(&T::getRelSizeVec);
    type["setRelPosVec"] = method<T, void(const Vector3&)>(&T::setRelPosVec);
    type["setRelSizeVec"] = method<T, void(const Vector3&)>(&T::setRelSizeVec);
    type["getFlags"] = method<T, CollisionFlags*()>(&T::getFlags);
    type["setFlags"] = method<T, void(const sol::object)>(&T::setFlags);
    type["getStaticAttrib"] = method<T, CollidableStaticAttrib*()>(&T::getStaticAttrib);
    type["setStaticAttrib"] = method<T, void(const sol::object)>(&T::setStaticAttrib);
    property(type, "colliderType", method<T, GY_COLLIDER_TYPE() const>(&T::getColliderType), method<T, void(GY_COLLIDER_TYPE)>(&T::setColliderType));
    property(type, "relPos", method<T, Vector3()>(&T::getRelPosVec), method<T, void(const Vector3&)>(&T::setRelPosVec));
    property(type, "relSize", method<T, Vector3()>(&T::getRelSizeVec), method<T, void(const Vector3&)>(&T::setRelSizeVec));
    property(type, "staticAttrib", method<T, CollidableStaticAttrib*()>(&T::getStaticAttrib), method<T, void(const sol::object)>(&T::setStaticAttrib));
    property(type, "flags", method<T, CollisionFlags*()>(&T::getFlags), method<T, void(const sol::object)>(&T::setFlags));
    inheritProperties<T>();
    finish<T>(type, "CollidableAttrib", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerCollisionContact(sol::state_view state) {
    if (state["CollisionContact"].valid()) return;
    using T = elysian::CollisionContact;
    auto type = registerType<T>(state, "CollisionContact");
    type["getCollidable"] = method<T, Collidable*(unsigned) const>(&T::getCollidable);
    type["getWorldPos"] = method<T, void(GYVector2&) const>(&T::getWorldPos);
    type["getNormal"] = method<T, void(GYVector2&) const>(&T::getNormal);
    type["getObjPos"] = method<T, void(unsigned, GYVector2&) const>(&T::getObjPos);
    type["getSeparatingVel"] = method<T, float() const>(&T::getSeparatingVel);
    type["getClosingVel"] = method<T, float() const>(&T::getClosingVel);
    type["getDeltaVel"] = method<T, float() const>(&T::getDeltaVel);
    type["getDepth"] = method<T, float() const>(&T::getDepth);
}

void registerCollidable(sol::state_view state) {
    if (state["Collidable"].valid()) return;
    registerComponent(state);
    using T = elysian::Collidable;
    auto type = registerType<T, elysian::Component>(state, "Collidable");
    type["getAttrib"] = method<T, std::shared_ptr<CollidableAttrib>() const>(&T::getAttrib);
    type["setAttrib"] = method<T, void(std::shared_ptr<CollidableAttrib>)>(&T::setAttrib);
    type["getContactCount"] = method<T, unsigned() const>(&T::getContactCount);
    type["isActive"] = method<T, bool() const>(&T::isActive);
    type["setActive"] = method<T, void(bool)>(&T::setActive);
    type["setResolvePenetration"] = method<T, void(bool)>(&T::setResolvePenetration);
    type["getResolvePenetration"] = method<T, bool() const>(&T::getResolvePenetration);
    type["getWorldSpace"] = method<T, bool() const>(&T::getWorldSpace);
    type["setWorldSpace"] = method<T, void(const bool)>(&T::setWorldSpace);
    type["getStackedOnGround"] = method<T, bool() const>(&T::getStackedOnGround);
    type["getRestingOnBody"] = method<T, bool() const>(&T::getRestingOnBody);
    property(type, "active", method<T, bool() const>(&T::isActive), method<T, void(bool)>(&T::setActive));
    property(type, "contactCount", method<T, unsigned() const>(&T::getContactCount));
    property(type, "stackedOnGround", method<T, bool() const>(&T::getStackedOnGround));
    property(type, "restingOnBody", method<T, bool() const>(&T::getRestingOnBody));
    property(type, "worldSpace", method<T, bool() const>(&T::getWorldSpace), method<T, void(const bool)>(&T::setWorldSpace));
    property(type, "resolvePen", method<T, bool() const>(&T::getResolvePenetration), method<T, void(bool)>(&T::setResolvePenetration));
    property(type, "attrib", method<T, std::shared_ptr<CollidableAttrib>() const>(&T::getAttrib), method<T, void(std::shared_ptr<CollidableAttrib>)>(&T::setAttrib));
    inheritProperties<T, elysian::Component>();
    finish<T>(type, "Collidable", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}
}
