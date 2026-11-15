/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_instance_transform_proxy.hpp>

/*!
    \qmltype Vector2
    \since 1.5.8
    \inmodule Engine
    \brief Represents a 2-dimensional point or vector within the Engine.

    A simple 2D vector containing an \l{x} and \l{y} component.
    Favor this type over any built-in or utility Lua vector types
    or libraries, as this type uses libGyro's hardware acceleration
    and SIMD instructions on platforms where they are available.

    \section2 Coordinate System
    In Elysian Shadows, the coordinate system (in screen space) is as follows:
    \table
        \header
            \li Position (Screen Space)
            \li Description
        \row
            \li <0, 0>
            \li Top left of screen
        \row
            \li <SCREEN_WIDTH, 0>
            \li Top Right of screen
        \row
            \li <0, SCREEN_HEIGHT>
            \li Bottom Left of screen
        \row
            \li <SCREEN_WIDTH, SCREEN_HEIGHT>
            \li Bottom right of screen
    \endtable
    The Z axis located with the positive Z direction coming out directly towards the viewer in the 2D perspective, meaning the furthest back object starts at
    Z coordinate 0 with closer objects having increasing Z values as they approach the viewer.

    \section2 Rotations
    Any rotations and angles are applied \e counterclockwise (in radians), with the positive
    X axis being at 0 degrees/radians. Unless a direction vector is taken, rotations performed
    around the Z axis (this is always the case with Vector2).

    \section2 Overloaded Operators
    The following overloaded operators are implemented in Lua through the respective metamethods.
    \table
        \header
            \li \b Symbol
            \li \b Operation
            \li \b Metamethod
        \row
            \li \b ==
            \li Equality
            \li \l{Vector2::_eq()}
        \row
            \li \b +
            \li Addition
            \li \l{Vector2::_add()}
        \row
            \li \b -
            \li Subtraction
            \li \l{Vector2::_sub()}
        \row
            \li \b *
            \li Multiplication
            \li \l{Vector2::_mult()}
        \row
            \li \b /
            \li Division
            \li \l{Vector2::_div()}
    \endtable

    \section2 Compound Assignment Operations
    Since Lua does not support in-place compound assignment operators such as +=, -=, *=, and /=, they are implemented as normal methods.
    If you are performing an operation that can be represented by one of these operators, you should always prefer to use the corresponding
    method than to use the traditional Lua method, as it saves memory and run-time overhead by not requiring extra copies.

    For example:
    \code
    vec1 = vec1 + vec2
    \endcode
    should be avoided in favor of:
    \code
    vec1:addVec(vec2)
    \endcode

    The following compound operations are supported.
    \table
        \header
            \li \b Operator
            \li \b RValue Type
            \li \b Method
        \row
            \li \b +=
            \li Vector2
            \li \l{Vector2::addVec()}
        \row
            \li \b -=
            \li Vector2
            \li \l{Vector2::subVec()}
        \row
            \li \b *=
            \li Vector2
            \li \l{Vector2::multVec()}
        \row
            \li \b /=
            \li Vector2
            \li \l{Vector2::divVec()}
        \row
            \li \b *=
            \li float
            \li \l{Vector2::multScalar()}
        \row
            \li \b /=
            \li float
            \li \l{Vector2::divScalar()}
    \endtable

    \section2 2D Cross Products
    Since the cross-product is ambiguous in 2D (or is undefined, depending on your interpretation
    of the mathematics), yet the operation is often still needed in certain scenarios (such as physics),
    the cross-product between two 2-dimension vectors can be interpreted as either a vector or a scalar
    value.

    \table
        \header
            \li Result
            \li Method
            \li Operation
            \li Geometric Interpretation
        \row
            \li float
            \li \l scalarCrossProduct()
            \li U\sub x * V\sub y - U\sub y * V\sub x
            \li Perpendicular dot product - Dot product of the perpendicular of U with V.
        \row
            \li Vector2
            \li \l vectorCrossProduct()
            \li <U\sub y * scalar, -U\sub x * scalar>
            \li Scaled perpendicular of given vector. Generalized form, determinant of 2x2 row matrix formed by the vectors.
    \endtable

*/


/*!
 * \qmlproperty float Vector2::x
 *
 * \sa getX(), setX()
 */
/*!
 * \qmlproperty float Vector2::y
 *
 * \sa getY(), setY()
 */

/*!
 * \qmlmethod Vector2::Vector2()
 *
 * Creates a new Vector2 with all elements initialized to 0.
 */
/*!
 * \qmlmethod Vector2::Vector2(float xComp, float yComp)
 *
 * Creates a new Vector2 with \l {Vector2::x} initialized to \a xComp and \l{Vector2::y} initialized to \a yComp.
 */
/*!
 * \qmlmethod Vector2::Vector2(Vector2 other)
 *
 * Copy-constructs a new Vector2 whose values are initialized to the values of \a other.
 */
/*!
 * \qmlmethod Vector2 Vector2::_add(Vector2 rhs)
 *
 * Overloaded addition operator (+) metamethod. Adds the components of \a rhs to the components of the given vector and returns the result as a new vector.
 * \sa _sub(), _div(), _mult(), _eq()
 */
/*!
 * \qmlmethod Vector2 Vector2::_sub(Vector2 rhs)
 *
 * Overloaded subtraction operator (-) metamethod. Subtracts the components of \a rhs from the components of the given vector and returns the result as a new vector.
 * \sa _add(), _div(), _mult(), _eq()
 */
/*!
 * \qmlmethod Vector2 Vector2::_mult(Vector2 rhs)
 *
 * Overloaded multiplication operator (*) metamethod. Multiplies the components of \a rhs by the components of the given vector and returns the result as a new vector.
 * \sa _add(), _div(), _sub(), _eq()
 */
/*!
 * \qmlmethod Vector2 Vector2::_div(Vector2 rhs)
 *
 * Overloaded division operator (/) metamethod. Divides the components of the current vector by the components of \a rhs and returns the result as a new vector.
 * \sa _add(), _sub(), _mult(), _eq()
 */
/*!
 * \qmlmethod Vector2 Vector2::_eq(Vector2 rhs)
 *
 * Overloaded equality comparison operator (==) metamethod. Compares the components of the current vector to the components of \a rhs and returns true if they are equal or false otherwise.
 * \sa _add(), _sub(), _mult(), _div()
 */
/*!
 * \qmlmethod void Vector2::addVec(Vector2 rhs)
 *
 * Adds the components of \a rhs to the components of the given vector.
 * \sa subVec(), multVec(), divVec()
 */
/*!
 * \qmlmethod void Vector2::subVec(Vector2 rhs)
 *
 * Subtracts the components of \a rhs from the components of the given vector.
 * \sa addVec(), multVec(), divVec()
 */
/*!
 * \qmlmethod void Vector2::multVec(Vector2 rhs)
 *
 * Multiplies the components of \a rhs by the components of the given vector.
 * \sa subVec(), addVec(), divVec()
 */
/*!
 * \qmlmethod void Vector2::divVec(Vector2 rhs)
 *
 * Divides the components of the given vector by the components of \a rhs.
 * \sa subVec(), multVec(), multVec()
 */
/*!
 * \qmlmethod void Vector2::multScalar(float scalar)
 *
 * Multiplies each component of \a rhs by \a scalar.
 * \sa divScalar()
 */
/*!
 * \qmlmethod void Vector2::divScalar(float scalar)
 *
 * Divides each component of \a rhs by \a scalar.
 * \sa multScalar()
 */
/*!
 * \qmlmethod float Vector2::getX()
 *
 * Returns the value of the vector's X component.
 * \sa {Vector2::x}, setX()
 */
/*!
 * \qmlmethod float Vector2::getY()
 *
 * Returns the value of the vector's Y component.
 * \sa {Vector2::y}, setY()
 */
/*!
 * \qmlmethod void Vector2::setX(float val)
 *
 * Sets the value of the vector's X component.
 * \sa {Vector2::x}, getX()
 */
/*!
 * \qmlmethod void Vector2::setY(float val)
 *
 * Sets the value of the vector's Y component.
 * \sa {Vector2::y}, getY()
 */
/*!
 * \qmlmethod float Vector2::getMagnitude()
 *
 * Returns the magnitude of the 2D vector. If the vector were to represent a point, this
 * is geometrically the distance from the origin to the point represented by the vector.
 *
 * \e{NOTE: This is not a property, so every time you call this function, the magnitude will
 * have to be calculated, which requires a square root. Do not call this function repeatedly
 * unless the Vector has actually changed. If not, cache the value rather than recalling this
 * function. }
 * \sa normalize(), getUnitVector()
 */
/*!
 * \qmlmethod void Vector2::normalize()
 *
 * Converts the given vector into a unit (direction) vector of unit length (1.0).
 * \sa getMagnitude(), getUnitVector()
 */
/*!
 * \qmlmethod float Vector2::distance(Vector2 other)
 *
 * Finds the distance between two points, represented as vectors, given the current vector
 * and \a other vector, using the distance formula (magnitude of the difference vector).
 * \e { NOTE: When you only need a relative distance, in the case of simply comparing distances
 * between points, rather than the absolute distance for a calculation, ALWAYS favor squaredDistance(),
 * because it saves on performing and unecessary square root operation.}
 * \sa squaredDistance()
 */
/*!
 * \qmlmethod float Vector2::squaredDistance(Vector2 other)
 *
 * Returns the value of the distance between the two vectors squared. Always use this function rather than
 * the distance() method when you only care about relative distances, such as when you're comparing the distances
 * between a set of vectors to see which is furthest, and you don't care about the actual exact distance value. This
 * will save on unecessary slow square root calls.
 * \sa distance()
 */
/*!
 * \qmlmethod float Vector2::dotProduct(Vector2 other)
 *
 * Returns the dot (or scalar) product between the current vector and the \a {other} vector.
 * \sa scalarCrossProduct(), vectorCrossProduct()
 */
/*!
 * \qmlmethod float Vector2::scalarCrossProduct(Vector2 other)
 *
 * Returns the cross product between the current vector and \a other vector in the form of a scalar.
 * \sa {2D Cross Products}, dotProduct(), vectorCrossProduct()
 */
/*!
 * \qmlmethod Vector2 Vector2::vectorCrossProduct(Vector2 other)
 *
 * Returns the cross product between the current vector and \a other vector in the form of a vector.
 * \sa {2D Cross Products}, scalarCrossProduct(), dotProduct()
 */
/*!
 * \qmlmethod float Vector2::toAngle()
 *
 * Returns the angle between the vector representing the positive X axis and the given vector using atan2.
 * The result is in radians.
 * \sa rotate(), getRotatedVector()
 */
/*!
 * \qmlmethod Vector2 Vector2::getUnitVector()
 *
 * Returns the angle between the vector representing the positive X axis and the given vector using atan2.
 * The result is in radians.
 * \sa normalize(), getMagnitude()
 */
/*!
 * \qmlmethod void Vector2::rotate(float radians)
 *
 * Rotates the current vector \e counterclockwise about the origin by the given \a { radians } (with the positive X axis being 0 degrees).
 *
 * \e {NOTE: The vector will be rotated about the positive Z axis.}
 *
 * \sa getRotatedVector(), rotateAboutPoint(), getRotatedVectorAboutPoint(), toAngle()
 */
/*!
 * \qmlmethod Vector2 Vector2::getRotatedVector(float radians)
 *
 * Returns a new vector as the current vector rotated \e counterclockwise about the origin by the given \a { radians } (with the positive X axis being 0 degrees).
 *
 * \e {NOTE: The vector will be rotated about the positive Z axis.}
 * \sa rotate(), rotateAboutPoint(), getRotatedVectorAboutPoint(), toAngle()
 */
/*!
 * \qmlmethod void Vector2::rotateAboutPoint(Vector2 pivot, float radians)
 *
 * Rotates the current vector \e counterclockwise about the given \a pivot by the given \a radians (with the positive X axis being 0 degrees).
 *
 * \e {NOTE: The vector will be rotated about the positive Z axis.}
 * \sa getRotatedVectorAboutPoint(), rotate(), getRotatedVector(), toAngle()
 */
/*!
 * \qmlmethod void Vector2::getRotatedVectorAboutPoint(Vector2 pivot, float radians)
 *
 * Returns a new vector as the current vector rotated \e counterclockwise about the given \a pivot by the given \a radians (with the positive X axis being 0 degrees).
 *
 * \e {NOTE: The vector will be rotated about the positive Z axis.}
 * \sa rotateAboutPoint(), rotate(), getRotatedVector(), toAngle()
 */

/*!
    \qmltype Vector3
    \since 1.5.8
    \inmodule Engine
    \brief Represents a 3-dimensional vector.
    A simple 3D vector containing an \l{x}, \l{y}, and \l{z} component.

    \section2 Overloaded Operators
    The following overloaded operators are implemented in Lua through the respective metamethods.
    \table
        \header
            \li \b Symbol
            \li \b Operation
            \li \b Metamethod
        \row
            \li \b ==
            \li Equality
            \li \l{Vector3::_eq()}
        \row
            \li \b +
            \li Addition
            \li \l{Vector3::_add()}
        \row
            \li \b -
            \li Subtraction
            \li \l{Vector3::_sub()}
        \row
            \li \b *
            \li Multiplication
            \li \l{Vector3::_mult()}
        \row
            \li \b /
            \li Division
            \li \l{Vector3::_div()}
    \endtable

    \section2 Compound Assignment Operations
    Since Lua does not support in-place compound assignment operators such as +=, -=, *=, and /=, they are implemented as normal methods.
    If you are performing an operation that can be represented by one of these operators, you should always prefer to use the corresponding
    method than to use the traditional Lua method, as it saves memory and run-time overhead by not requiring extra copies.

    For example:
    \code
    vec1 = vec1 + vec2
    \endcode
    should be avoided in favor of:
    \code
    vec1:addVec(vec2)
    \endcode

    The following compound operations are supported.
    \table
        \header
            \li \b Operator
            \li \b RValue Type
            \li \b Method
        \row
            \li \b +=
            \li Vector2
            \li \l{Vector3::addVec()}
        \row
            \li \b -=
            \li Vector2
            \li \l{Vector3::subVec()}
        \row
            \li \b *=
            \li Vector2
            \li \l{Vector3::multVec()}
        \row
            \li \b /=
            \li Vector2
            \li \l{Vector3::divVec()}
        \row
            \li \b *=
            \li float
            \li \l{Vector3::multScalar()}
        \row
            \li \b /=
            \li float
            \li \l{Vector3::divScalar()}
    \endtable
*/

/*!
 * \qmlproperty float Vector3::x
 *
 * \sa getX(), setX()
 */
/*!
 * \qmlproperty float Vector3::y
 *
 * \sa getY(), setY()
 */
/*!
 * \qmlproperty float Vector3::z
 *
 * \sa getZ(), setZ()
 */

/*!
 * \qmlmethod Vector3::Vector()
 *
 * Creates a new Vector3 with all elements initialized to 0.
 */
/*!
 * \qmlmethod Vector3::Vector3(float xComp, float yComp, float zComp)
 *
 * Creates a new Vector2 with \l {Vector3::x} initialized to \a xComp, \l{Vector3::y} initialized to \a yComp, and \l{Vector3::z} initialized to \a zComp.
 */
/*!
 * \qmlmethod Vector3::Vector3(Vector3 other)
 *
 * Copy-constructs a new Vector3 whose values are initialized to the values of \a other.
 */

/*!
 * \qmlmethod float Vector3::getX()
 *
 * Returns the value of the vector's X component.
 */
/*!
 * \qmlmethod float Vector3::getY()
 *
 * Returns the value of the vector's Y component.
 */
/*!
 * \qmlmethod float Vector3::getZ()
 *
 * Returns the value of the vector's Z component.
 */
/*!
 * \qmlmethod void Vector3::setX(float val)
 *
 * Sets the value of the vector's X component.
 */
/*!
 * \qmlmethod void Vector3::setY(float val)
 *
 * Sets the value of the vector's Y component.
 */
/*!
 * \qmlmethod void Vector3::setZ(float val)
 *
 * Sets the value of the vector's Z component.
 */
/*!
 * \qmlmethod Vector3 Vector3::_add(Vector3 rhs)
 *
 * Overloaded addition operator (+) metamethod. Adds the components of \a rhs to the components of the given vector and returns the result as a new vector.
 * \sa _sub(), _div(), _mult(), _eq()
 */
/*!
 * \qmlmethod Vector3 Vector3::_sub(Vector3 rhs)
 *
 * Overloaded subtraction operator (-) metamethod. Subtracts the components of \a rhs from the components of the given vector and returns the result as a new vector.
 * \sa _add(), _div(), _mult(), _eq()
 */
/*!
 * \qmlmethod Vector3 Vector3::_mult(Vector3 rhs)
 *
 * Overloaded multiplication operator (*) metamethod. Multiplies the components of \a rhs by the components of the given vector and returns the result as a new vector.
 * \sa _add(), _div(), _sub(), _eq()
 */
/*!
 * \qmlmethod Vector3 Vector3::_div(Vector3 rhs)
 *
 * Overloaded division operator (/) metamethod. Divides the components of the current vector by the components of \a rhs and returns the result as a new vector.
 * \sa _add(), _sub(), _mult(), _eq()
 */
/*!
 * \qmlmethod Vector3 Vector3::_eq(Vector3 rhs)
 *
 * Overloaded equality comparison operator (==) metamethod. Compares the components of the current vector to the components of \a rhs and returns true if they are equal or false otherwise.
 * \sa _add(), _sub(), _mult(), _div()
 */
/*!
 * \qmlmethod void Vector3::addVec(Vector3 rhs)
 *
 * Adds the components of \a rhs to the components of the given vector.
 * \sa subVec(), multVec(), divVec()
 */
/*!
 * \qmlmethod void Vector3::subVec(Vector3 rhs)
 *
 * Subtracts the components of \a rhs from the components of the given vector.
 * \sa addVec(), multVec(), divVec()
 */
/*!
 * \qmlmethod void Vector3::multVec(Vector3 rhs)
 *
 * Multiplies the components of \a rhs by the components of the given vector.
 * \sa subVec(), addVec(), divVec()
 */
/*!
 * \qmlmethod void Vector3::divVec(Vector3 rhs)
 *
 * Divides the components of the given vector by the components of \a rhs.
 * \sa subVec(), multVec(), multVec()
 */
/*!
 * \qmlmethod void Vector3::multScalar(float scalar)
 *
 * Multiplies each component of \a rhs by \a scalar.
 * \sa divScalar()
 */
/*!
 * \qmlmethod void Vector3::divScalar(float scalar)
 *
 * Divides each component of \a rhs by \a scalar.
 * \sa multScalar()
 */
/*!
 * \qmlmethod float Vector3::getMagnitude()
 *
 * Returns the magnitude of the 3D vector. If the vector were to represent a point, this
 * is geometrically the distance from the origin to the point represented by the vector.
 *
 * \e{NOTE: This is not a property, so every time you call this function, the magnitude will
 * have to be calculated, which requires a square root. Do not call this function repeatedly
 * unless the Vector has actually changed. If not, cache the value rather than recalling this
 * function. }
 * \sa normalize(), getUnitVector()
 */
/*!
 * \qmlmethod void Vector3::normalize()
 *
 * Converts the given vector into a unit (direction) vector of unit length (1.0).
 * \sa getMagnitude(), getUnitVector()
 */
/*!
 * \qmlmethod float Vector3::distance(Vector3 other)
 *
 * Finds the distance between two points, represented as vectors, given the current vector
 * and \a other vector, using the distance formula (magnitude of the difference vector).
 * \e { NOTE: When you only need a relative distance, in the case of simply comparing distances
 * between points, rather than the absolute distance for a calculation, ALWAYS favor squaredDistance(),
 * because it saves on performing and unecessary square root operation.}
 * \sa squaredDistance()
 */
/*!
 * \qmlmethod float Vector3::squaredDistance(Vector3 other)
 *
 * Returns the value of the distance between the two vectors squared. Always use this function rather than
 * the distance() method when you only care about relative distances, such as when you're comparing the distances
 * between a set of vectors to see which is furthest, and you don't care about the actual exact distance value. This
 * will save on unecessary slow square root calls.
 * \sa distance()
 */
/*!
 * \qmlmethod float Vector3::dotProduct(Vector3 other)
 *
 * Returns the dot (or scalar) product between the current vector and the \a {other} vector.
 * \sa crossProduct()
 */
/*!
 * \qmlmethod Vector3 Vector3::crossProduct(Vector3 other)
 *
 * Returns the cross product between the current vector and \a other vector in the form of a vector.
 * \sa dotProduct()
 */
/*!
 * \qmlmethod Vector3 Vector3::getUnitVector()
 *
 * Returns the angle between the vector representing the positive X axis and the given vector using atan2.
 * The result is in radians.
 * \sa normalize(), getMagnitude()
 */



/*!
    \qmltype Vector4
    \since 1.5.8
    \inmodule Engine
    \brief Represents a 4-dimensional vector.
    A simple 4D vector containing an \l{x}, \l{y}, \l{z}, and \l{w} component.
*/

/*!
 * \qmlproperty float Vector4::x
 *
 * \sa getX(), setX()
 */
/*!
 * \qmlproperty float Vector4::y
 *
 * \sa getY(), setY()
 */
/*!
 * \qmlproperty float Vector4::z
 *
 * \sa getZ(), setZ()
 */
/*!
 * \qmlproperty float Vector4::w
 *
 * \sa getW(), setW()
 */

/*!
 * \qmlmethod float Vector4::getX()
 *
 * Returns the value of the vector's X component.
 */
/*!
 * \qmlmethod float Vector4::getY()
 *
 * Returns the value of the vector's Y component.
 */
/*!
 * \qmlmethod float Vector4::getZ()
 *
 * Returns the value of the vector's Z component.
 */
/*!
 * \qmlmethod float Vector4::getW()
 *
 * Returns the value of the vector's W component.
 */
/*!
 * \qmlmethod void Vector4::setX(float val)
 *
 * Sets the value of the vector's X component.
 */
/*!
 * \qmlmethod void Vector4::setY(float val)
 *
 * Sets the value of the vector's Y component.
 */
/*!
 * \qmlmethod void Vector4::setZ(float val)
 *
 * Sets the value of the vector's Z component.
 */
/*!
 * \qmlmethod void Vector4::setW(float val)
 *
 * Sets the value of the vector's W component.
 */



/*!
    \qmltype Quaternion
    \since 1.6.1
    \inmodule Engine
    \brief Represents an orientation or rotation in 3D space
    A simple 3D vector containing an \l{x}, \l{y}, and \l{z} component.

    \section2 Overloaded Operators
    The following overloaded operators are implemented in Lua through the respective metamethods.
    \table
        \header
            \li \b Symbol
            \li \b Operation
            \li \b Metamethod
        \row
            \li \b ==
            \li Equality
            \li \l{Vector3::_eq()}
        \row
            \li \b +
            \li Addition
            \li \l{Vector3::_add()}
        \row
            \li \b -
            \li Subtraction
            \li \l{Vector3::_sub()}
        \row
            \li \b *
            \li Multiplication
            \li \l{Vector3::_mult()}
        \row
            \li \b /
            \li Division
            \li \l{Vector3::_div()}
    \endtable
*/
/*!
    \qmltype InstanceTransform
    \since 1.5.8
    \inmodule Engine
    \brief Represents a 3D instance transform with position, scale, and rotation.

    InstanceTransform represents a set of transforms: scale, rotation, and translation,
    applied in that order, to take geometry from local coordinates to its final size,
    orientation, and world-coordiate position in 3D space.

    The following table describes the properties used for each transform.
    \table
    \header
        \li transform
        \li properties used
    \row
        \li translation
        \li \l{pos}
    \row
        \li rotation
        \li \l {orient} and \l{angle}
    \row
        \li scale
        \li \l{size} * \l{scale}
    \endtable
*/
/*!
 * \qmlproperty Vector3 InstanceTransform::pos
 * 3D Vector representing position.
 * \sa getPosX(), getPosY(), getPosZ(), setPosX(), setPosY(), setPosZ()
 */
/*!
 * \qmlproperty Vector3 InstanceTransform::size
 *  3D vector representing size. This is multiplied by scale to determine final transform size.
 *  Prefer to use size to represent the base size of an entity.
 * \sa getSizeX(), getSizeY(), getSizeZ(), setSizeX(), setSizeY(), setSizeZ()
 */
/*!
 * \qmlproperty Vector3 InstanceTransform::scale
 * 3D vector representing scale. This is multiplied by size to deterine final transform size.
 * Prefer to use scale to represent a multiplier applied to the base size of an entity.
 * \sa getScaleX(), getScaleY(), getScaleZ(), setScaleX(), setScaleY(), setScaleZ()
 */
/*!
 * \qmlproperty Vector3 InstanceTransform::orient
 * 3D UNIT vector representing the direction of rotation.
 * \sa getOrientX(), getOrientY(), getOrientZ(), setOrientX(), setOrientY(), setOrientZ()
 */
/*!
 * \qmlproperty float InstanceTransform::angle
 *
 * \sa getAngle(), setAngle()
 */

/*!
 * \qmlmethod void InstanceTransform::setPos3f(float x, float y, float z)
 *
 * Sets the 3D position vector from 3 floats.
 */
/*!
 * \qmlmethod void InstanceTransform::setPos2f(float x, float y)
 *
 * Sets the 2D position vector from 2 floats. Z coordinate does not change.
 */
/*!
 * \qmlmethod void InstanceTransform::translate3f(float x, float y, float z)
 *
 * Adds the <x,y,z> vector to the current position.
 */
/*!
 * \qmlmethod void InstanceTransform::translate2f(float x, float y)
 *
 * Adds the <x,y> vector to the current position.
 */
/*!
 * \qmlmethod void InstanceTransform::setSize3f(float x, float y, float z)
 *
 * Sets the 3D size to the <x, y, z> vector.
 */
/*!
 * \qmlmethod void InstanceTransform::setSize2f(float x, float y)
 *
 * Sets the 2D size to the <x, y> vector. The Z size does not change.
 */
/*!
 * \qmlmethod void InstanceTransform::setScale3f(float x, float y, float z)
 *
 * Sets the 3D scale to the <x, y, z> vector.
 */
/*!
 * \qmlmethod void InstanceTransform::setScale2f(float x, float y)
 *
 * Sets the 2D scale to the <x, y> vector. The Z size does not change.
 */
/*!
 * \qmlmethod void InstanceTransform::scale3f(float x, float y, float z)
 *
 * Multiplies the 3D scale vector by <x, y, z>.
 */
/*!
 * \qmlmethod void InstanceTransform::scale2f(float x, float y)
 *
 * Multiplies the 2D scale vector by <x, y> with the z component unaffected.
 */
/*!
 * \qmlmethod void InstanceTransform::setOrient3f(float x, float y, float z)
 *
 * Sets the direction of rotation to the UNIT VECTOR <x, y, z>
 */
/*!
 * \qmlmethod void InstanceTransform::setAngle(float radians)
 *
 * Sets the transform to rotate about the orientation vector by <radians>.
 */
/*!
 * \qmlmethod float InstanceTransform::getAngle()
 *
 * Returns the number of radians the transform is rotated about the orientation vector.
 */
/*!
 * \qmlmethod Vector3& InstanceTransform::getPos()
 *
 * Returns a reference to the 3D position vector.
 */
/*!
 * \qmlmethod Vector3& InstanceTransform::getSize()
 *
 * Returns a reference to the 3D size vector.
 */
/*!
 * \qmlmethod Vector3& InstanceTransform::getScale()
 *
 * Returns a reference to the 3D scale vector.
 */
/*!
 * \qmlmethod Vector3& InstanceTransform::getOrient()
 *
 * Returns a reference to the 3D orientation vector.
 */

/*!
 * \qmlmethod float InstanceTransform::getPosX()
 *
 * Returns the position vector's X component.
 */
/*!
 * \qmlmethod float InstanceTransform::getPosY()
 *
 * Returns the position vector's Y component.
 */
/*!
 * \qmlmethod float InstanceTransform::getPosZ()
 *
 * Returns the position vector's Z component.
 */

/*!
 * \qmlmethod float InstanceTransform::getSizeX()
 *
 * Returns the size vector's X component.
 */
/*!
 * \qmlmethod float InstanceTransform::getSizeY()
 *
 * Returns the size vector's Y component.
 */
/*!
 * \qmlmethod float InstanceTransform::getSizeZ()
 *
 * Returns the size vector's Z component.
 */

/*!
 * \qmlmethod float InstanceTransform::getScaleX()
 *
 * Returns the scale vector's X component.
 */
/*!
 * \qmlmethod float InstanceTransform::getScaleY()
 *
 * Returns the scale vector's Y component.
 */
/*!
 * \qmlmethod float InstanceTransform::getScaleZ()
 *
 * Returns the scale vector's Z component.
 */

/*!
 * \qmlmethod float InstanceTransform::getOrientX()
 *
 * Returns the orient vector's X component.
 */
/*!
 * \qmlmethod float InstanceTransform::getOrientY()
 *
 * Returns the orient vector's Y component.
 */
/*!
 * \qmlmethod float InstanceTransform::getOrientZ()
 *
 * Returns the orient vector's Z component.
 */

/*!
 * \qmlmethod void InstanceTransform::setPosX(float val)
 *
 * Sets the position vector's X component.
 */
/*!
 * \qmlmethod void InstanceTransform::setPosY(float val)
 *
 * Sets the position vector's Y component.
 */
/*!
 * \qmlmethod void InstanceTransform::setPosZ(float val)
 *
 * Sets the position vector's Z component.
 */

/*!
 * \qmlmethod void InstanceTransform::setSizeX(float val)
 *
 * Sets the size vector's X component.
 */
/*!
 * \qmlmethod void InstanceTransform::setSizeY(float val)
 *
 * Sets the size vector's Y component.
 */
/*!
 * \qmlmethod void InstanceTransform::setSizeZ(float val)
 *
 * Sets the size vector's Z component.
 */

/*!
 * \qmlmethod void InstanceTransform::setScaleX(float val)
 *
 * Sets the scale vector's X component.
 */
/*!
 * \qmlmethod void InstanceTransform::setScaleY(float val)
 *
 * Sets the scale vector's Y component.
 */
/*!
 * \qmlmethod void InstanceTransform::setScaleZ(float val)
 *
 * Sets the scale vector's Z component.
 */

/*!
 * \qmlmethod void InstanceTransform::setOrientX(float val)
 *
 * Sets the orient vector's X component.
 */
/*!
 * \qmlmethod void InstanceTransform::setOrientY(float val)
 *
 * Sets the orient vector's Y component.
 */
/*!
 * \qmlmethod void InstanceTransform::setOrientZ(float val)
 *
 * Sets the orient vector's Z component.
 */


////////
namespace elysian::lua {
using namespace elysian;
using GYVector2 = ::GYVector2;
using Vector2 = elysian::Vector2;
using Vector3 = elysian::Vector3;
using Vector4 = elysian::Vector4;
using Quaternion = elysian::Quaternion;
using InstanceTransform = elysian::InstanceTransform;

void registerGYVector2(sol::state_view state) {
    if (state["GYVector2"].valid()) return;
    using T = ::GYVector2;
    auto type = registerType<T>(state, "GYVector2");
    type["new"] = sol::factories(factory<T>());
    type["getX"] = [](const T& self) { return self.x; };
    type["setX"] = [](T& self, decltype(T::x) value) { self.x = value; };
    type["getY"] = [](const T& self) { return self.y; };
    type["setY"] = [](T& self, decltype(T::y) value) { self.y = value; };
}

void registerVector2(sol::state_view state) {
    if (state["Vector2"].valid()) return;
    using T = elysian::Vector2;
    auto type = registerType<T>(state, "Vector2");
    type["new"] = sol::factories(factory<T>(), factory<T, const Vector2&>(), factory<T, const GYVector2&>(), factory<T, float, float>(), factory<T, float>());
    type["getX"] = method<T, float() const>(&T::getX);
    type["getY"] = method<T, float() const>(&T::getY);
    type["setX"] = method<T, void(float) const>(&T::setX);
    type["setY"] = method<T, void(float) const>(&T::setY);
    type["addVec"] = method<T>(&T::addVec);
    type["subVec"] = method<T>(&T::subVec);
    type["multVec"] = method<T>(&T::multVec);
    type["divVec"] = method<T>(&T::divVec);
    type["multScalar"] = method<T>(&T::multScalar);
    type["divScalar"] = method<T>(&T::divScalar);
    type["normalize"] = method<T>(&T::normalize);
    type["getMagnitude"] = method<T>(&T::getMagnitude);
    type["squaredDistance"] = method<T>(&T::squaredDistance);
    type["distance"] = method<T>(&T::distance);
    type["dotProduct"] = method<T>(&T::dotProduct);
    type["scalarCrossProduct"] = method<T>(&T::scalarCrossProduct);
    type["vectorCrossProduct"] = method<T>(&T::vectorCrossProduct);
    type["getUnitVector"] = method<T>(&T::getUnitVector);
    type["rotate"] = method<T>(&T::rotate);
    type["toAngle"] = method<T>(&T::toAngle);
    type["getRotatedVector"] = method<T>(&T::getRotatedVector);
    type["rotateAboutPoint"] = method<T>(&T::rotateAboutPoint);
    type["getRotatedVectorAboutPoint"] = method<T>(&T::getRotatedVectorAboutPoint);
    type[sol::meta_function::equal_to] = [](const T& a, const T& b) { return a == b; };
    type[sol::meta_function::addition] = [](const T& a, const T& b) { return a + b; };
    type[sol::meta_function::subtraction] = [](const T& a, const T& b) { return a - b; };
    type[sol::meta_function::multiplication] = [](const T& a, const T& b) { return a * b; };
    type[sol::meta_function::division] = [](const T& a, const T& b) { return a / b; };
    property(type, "x", method<T, float() const>(&T::getX), method<T, void(float) const>(&T::setX));
    property(type, "y", method<T, float() const>(&T::getY), method<T, void(float) const>(&T::setY));
    inheritProperties<T>();
    finish<T>(type, "Vector2", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerGYVector3(sol::state_view state) {
    if (state["GYVector3"].valid()) return;
    using T = GYVector3;
    auto type = registerType<T>(state, "GYVector3");
    type["new"] = sol::factories(factory<T>());
    type["getX"] = [](const T& self) { return self.x; };
    type["setX"] = [](T& self, decltype(T::x) value) { self.x = value; };
    type["getY"] = [](const T& self) { return self.y; };
    type["setY"] = [](T& self, decltype(T::y) value) { self.y = value; };
    type["getZ"] = [](const T& self) { return self.z; };
    type["setZ"] = [](T& self, decltype(T::z) value) { self.z = value; };
}

void registerVector3(sol::state_view state) {
    if (state["Vector3"].valid()) return;
    using T = elysian::Vector3;
    auto type = registerType<T>(state, "Vector3");
    type["new"] = sol::factories(factory<T>(), factory<T, const Vector2&>(), factory<T, const Vector3&>(), factory<T, const Vector2&, float>(), factory<T, float, float, float>(), factory<T, float>());
    type["getX"] = method<T, float() const>(&T::getX);
    type["getY"] = method<T, float() const>(&T::getY);
    type["getZ"] = method<T, float() const>(&T::getZ);
    type["setX"] = method<T, void(float) const>(&T::setX);
    type["setY"] = method<T, void(float) const>(&T::setY);
    type["setZ"] = method<T, void(float) const>(&T::setZ);
    type["addVec"] = method<T>(&T::addVec);
    type["subVec"] = method<T>(&T::subVec);
    type["multVec"] = method<T>(&T::multVec);
    type["divVec"] = method<T>(&T::divVec);
    type["multScalar"] = method<T>(&T::multScalar);
    type["divScalar"] = method<T>(&T::divScalar);
    type["normalize"] = method<T>(&T::normalize);
    type["getMagnitude"] = method<T>(&T::getMagnitude);
    type["squaredDistance"] = method<T>(&T::squaredDistance);
    type["distance"] = method<T>(&T::distance);
    type["dotProduct"] = method<T>(&T::dotProduct);
    type["crossProduct"] = method<T>(&T::crossProduct);
    type["getUnitVector"] = method<T>(&T::getUnitVector);
    type[sol::meta_function::equal_to] = [](const T& a, const T& b) { return a == b; };
    type[sol::meta_function::addition] = [](const T& a, const T& b) { return a + b; };
    type[sol::meta_function::subtraction] = [](const T& a, const T& b) { return a - b; };
    type[sol::meta_function::multiplication] = [](const T& a, const T& b) { return a * b; };
    type[sol::meta_function::division] = [](const T& a, const T& b) { return a / b; };
    property(type, "x", method<T, float() const>(&T::getX), method<T, void(float) const>(&T::setX));
    property(type, "y", method<T, float() const>(&T::getY), method<T, void(float) const>(&T::setY));
    property(type, "z", method<T, float() const>(&T::getZ), method<T, void(float) const>(&T::setZ));
    inheritProperties<T>();
    finish<T>(type, "Vector3", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerGYVector4(sol::state_view state) {
    if (state["GYVector4"].valid()) return;
    using T = GYVector4;
    auto type = registerType<T>(state, "GYVector4");
    type["new"] = sol::factories(factory<T>());
    type["getX"] = [](const T& self) { return self.x; };
    type["setX"] = [](T& self, decltype(T::x) value) { self.x = value; };
    type["getY"] = [](const T& self) { return self.y; };
    type["setY"] = [](T& self, decltype(T::y) value) { self.y = value; };
    type["getZ"] = [](const T& self) { return self.z; };
    type["setZ"] = [](T& self, decltype(T::z) value) { self.z = value; };
    type["getW"] = [](const T& self) { return self.w; };
    type["setW"] = [](T& self, decltype(T::w) value) { self.w = value; };
}

void registerVector4(sol::state_view state) {
    if (state["Vector4"].valid()) return;
    using T = elysian::Vector4;
    auto type = registerType<T>(state, "Vector4");
    type["new"] = sol::factories(factory<T>());
    type["getX"] = method<T, float() const>(&T::getX);
    type["getY"] = method<T, float() const>(&T::getY);
    type["getZ"] = method<T, float() const>(&T::getZ);
    type["getW"] = method<T, float() const>(&T::getW);
    type["setX"] = method<T, void(float) const>(&T::setX);
    type["setY"] = method<T, void(float) const>(&T::setY);
    type["setZ"] = method<T, void(float) const>(&T::setZ);
    type["setW"] = method<T, void(float) const>(&T::setW);
    property(type, "x", method<T, float() const>(&T::getX), method<T, void(float) const>(&T::setX));
    property(type, "y", method<T, float() const>(&T::getY), method<T, void(float) const>(&T::setY));
    property(type, "z", method<T, float() const>(&T::getZ), method<T, void(float) const>(&T::setZ));
    property(type, "w", method<T, float() const>(&T::getW), method<T, void(float) const>(&T::setW));
    inheritProperties<T>();
    finish<T>(type, "Vector4", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerQuaternion(sol::state_view state) {
    if (state["Quaternion"].valid()) return;
    using T = elysian::Quaternion;
    auto type = registerType<T>(state, "Quaternion");
    type["new"] = sol::factories(factory<T>());
    inheritProperties<T>();
    finish<T>(type, "Quaternion", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerInstanceTransform(sol::state_view state) {
    if (state["InstanceTransform"].valid()) return;
    using T = elysian::InstanceTransform;
    auto type = registerType<T>(state, "InstanceTransform");
    type["apply"] = method<T, void() const>(&T::apply);
    type["setPos2f"] = method<T, void(float, float)>(&T::setPos);
    type["setPos3f"] = method<T, void(float, float, float)>(&T::setPos);
    type["translate2f"] = method<T, void(float, float)>(&T::translate);
    type["translate3f"] = method<T, void(float, float, float)>(&T::translate);
    type["setSize2f"] = method<T, void(float, float)>(&T::setSize);
    type["setSize3f"] = method<T, void(float, float, float)>(&T::setSize);
    type["setScale2f"] = method<T, void(float, float)>(&T::setScale);
    type["setScale3f"] = method<T, void(float, float, float)>(&T::setScale);
    type["scale2f"] = method<T, void(float, float)>(&T::scale);
    type["scale3f"] = method<T, void(float, float, float)>(&T::scale);
    type["setOrient3f"] = method<T, void(float, float, float)>(&T::setOrient);
    type["rotate"] = method<T, void(float)>(&T::rotate);
    type["getPos"] = method<T, GYVector3&()>(&T::getPos);
    type["getSize"] = method<T, GYVector3&()>(&T::getSize);
    type["getScale"] = method<T, GYVector3&()>(&T::getScale);
    type["getOrient"] = method<T, GYVector3&()>(&T::getOrient);
    type["getPosZ"] = method<T, float() const>(&T::getPosZ);
    type["setPosZ"] = method<T, void(const float)>(&T::setPosZ);
    type["getPosX"] = method<T, float() const>(&T::getPosX);
    type["setPosX"] = method<T, void(const float)>(&T::setPosX);
    type["getPosY"] = method<T, float() const>(&T::getPosY);
    type["setPosY"] = method<T, void(const float)>(&T::setPosY);
    type["getSizeZ"] = method<T, float() const>(&T::getSizeZ);
    type["setSizeZ"] = method<T, void(const float)>(&T::setSizeZ);
    type["getSizeX"] = method<T, float() const>(&T::getSizeX);
    type["setSizeX"] = method<T, void(const float)>(&T::setSizeX);
    type["getSizeY"] = method<T, float() const>(&T::getSizeY);
    type["setSizeY"] = method<T, void(const float)>(&T::setSizeY);
    type["getScaleZ"] = method<T, float() const>(&T::getScaleZ);
    type["setScaleZ"] = method<T, void(const float)>(&T::setScaleZ);
    type["getScaleX"] = method<T, float() const>(&T::getScaleX);
    type["setScaleX"] = method<T, void(const float)>(&T::setScaleX);
    type["getScaleY"] = method<T, float() const>(&T::getScaleY);
    type["setScaleY"] = method<T, void(const float)>(&T::setScaleY);
    type["setAngle"] = method<T, void(const float)>(&T::setAngle);
    type["getAngle"] = method<T, float() const>(&T::getAngle);
    type["setOrientX"] = method<T, void(const float)>(&T::setOrientX);
    type["getOrientX"] = method<T, float() const>(&T::getOrientX);
    type["setOrientY"] = method<T, void(const float)>(&T::setOrientY);
    type["getOrientY"] = method<T, float() const>(&T::getOrientY);
    type["setOrientZ"] = method<T, void(const float)>(&T::setOrientZ);
    type["getOrientZ"] = method<T, float() const>(&T::getOrientZ);
    property(type, "angle", method<T, float() const>(&T::getAngle), method<T, void(const float)>(&T::setAngle));
    viewProperty(type, group("pos", {
            field<T>("x", method<T, float() const>(&T::getPosX), method<T, void(const float)>(&T::setPosX)),
            field<T>("y", method<T, float() const>(&T::getPosY), method<T, void(const float)>(&T::setPosY)),
            field<T>("z", method<T, float() const>(&T::getPosZ), method<T, void(const float)>(&T::setPosZ))
        }, [] { return LuaManager::isSerializing(); }));
    viewProperty(type, group("orient", {
            field<T>("x", method<T, float() const>(&T::getOrientX), method<T, void(const float)>(&T::setOrientX)),
            field<T>("y", method<T, float() const>(&T::getOrientY), method<T, void(const float)>(&T::setOrientY)),
            field<T>("z", method<T, float() const>(&T::getOrientZ), method<T, void(const float)>(&T::setOrientZ))
        }, [] { return LuaManager::isSerializing(); }));
    viewProperty(type, group("size", {
            field<T>("x", method<T, float() const>(&T::getSizeX), method<T, void(const float)>(&T::setSizeX)),
            field<T>("y", method<T, float() const>(&T::getSizeY), method<T, void(const float)>(&T::setSizeY)),
            field<T>("z", method<T, float() const>(&T::getSizeZ), method<T, void(const float)>(&T::setSizeZ))
        }, [] { return LuaManager::isSerializing(); }));
    viewProperty(type, group("scale", {
            field<T>("x", method<T, float() const>(&T::getScaleX), method<T, void(const float)>(&T::setScaleX)),
            field<T>("y", method<T, float() const>(&T::getScaleY), method<T, void(const float)>(&T::setScaleY)),
            field<T>("z", method<T, float() const>(&T::getScaleZ), method<T, void(const float)>(&T::setScaleZ))
        }, [] { return LuaManager::isSerializing(); }));
    inheritProperties<T>();
    finish<T>(type, "InstanceTransform", [] { return LuaManager::isSerializing(); });
    initializeEditor<T>(type);
}
}
