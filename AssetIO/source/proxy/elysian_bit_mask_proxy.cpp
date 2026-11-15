/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <AssetIO/proxy/elysian_bit_mask_proxy.hpp>


/*!
    \qmltype BitMask32
    \since 1.6.0
    \inmodule Engine
    \brief Represents a 32-bit packed integer of bitfields.

    BitMask32 is a convenience class for setting, extracting, and applying bitmasks to a 32-bit unsigned integer.
    Since Lua5.2 doesn't even support integers natively or have a bitmask library, ALWAYS use this class and don't
    try to do it yourself!
*/

/*!
 * \qmlproperty int BitMask32::value
 * The entire unsigned 32-bit integer field represented by the BitMask32
 * \sa getValue(), setValue()
 */
/*!
 * \qmlproperty array BitMask32::bitFields
 * Array of 32 booleans containing the values of each bit within the internal 32-bit unsigned integer.
 * \sa getBit(), setBit(), getBitCount()
 */
/*!
 * \qmlmethod void BitMask32::setValue(int value)
 *
 * Sets the value of the entire internal 32-bit unsigned integer field to \a{value}.
 * \sa getValue()
 */
/*!
 * \qmlmethod int BitMask32::getValue()
 *
 * Retrieves the value of the entire 32-bit unsigned integer field.
 * \sa setValue()
 */
/*!
 * \qmlmethod void BitMask32::setBit(int index, bool value)
 *
 * Sets the value of the single bit at location \a{index} to \a{value}.
 * \sa getBit()
 */
/*!
 * \qmlmethod bool BitMask32::getBit(int index)
 *
 * Returns the value of the single bit at location \a{index}.
 * \sa setBit()
 */
/*!
 * \qmlmethod int BitMask32::getBitCount()
 *
 * Returns the total number of bits in the internal unsiged 32-bit integer object... Better be 32.
 */

/*!
    \qmltype BitMask64
    \since 1.6.0
    \inmodule Engine
    \brief Represents a 64-bit packed integer of bitfields.

    BitMask64 is a convenience class for setting, extracting, and applying bitmasks to a 64-bit unsigned integer.
    Since Lua5.2 doesn't even support integers natively or have a bitmask library, ALWAYS use this class and don't
    try to do it yourself!
*/

/*!
 * \qmlproperty int BitMask64::value
 * The entire unsigned 64-bit integer field represented by the BitMask64
 * \sa getValue(), setValue()
 */
/*!
 * \qmlproperty array BitMask64::bitFields
 * Array of 64 booleans containing the values of each bit within the internal 64-bit unsigned integer.
 * \sa getBit(), setBit(), getBitCount()
 */
/*!
 * \qmlmethod void BitMask64::setValue(int value)
 *
 * Sets the value of the entire internal 64-bit unsigned integer field to \a{value}.
 * \sa getValue()
 */
/*!
 * \qmlmethod int BitMask64::getValue()
 *
 * Retrieves the value of the entire 64-bit unsigned integer field.
 * \sa setValue()
 */
/*!
 * \qmlmethod void BitMask64::setBit(int index, bool value)
 *
 * Sets the value of the single bit at location \a{index} to \a{value}.
 * \sa getBit()
 */
/*!
 * \qmlmethod bool BitMask64::getBit(int index)
 *
 * Returns the value of the single bit at location \a{index}.
 * \sa setBit()
 */
/*!
 * \qmlmethod int BitMask64::getBitCount()
 *
 * Returns the total number of bits in the internal unsiged 64-bit integer object... Better be 64.
 */



namespace elysian::lua {
using namespace elysian;
using BitMask32 = elysian::BitMask32;

void registerBitMask32(sol::state_view state) {
    if (state["BitMask32"].valid()) return;
    using T = elysian::BitMask32;
    auto type = registerType<T>(state, "BitMask32");
    type["new"] = sol::factories(factory<T>());
    type["setValue"] = method<T, void(uint32_t)>(&T::setValue);
    type["getValue"] = method<T, uint32_t() const>(&T::getValue);
    type["getBit"] = method<T, bool(unsigned) const>(&T::getBit);
    type["setBit"] = method<T, void(unsigned, bool)>(&T::setBit);
    type["getBitCount"] = method<T, unsigned() const>(&T::getBitCount);
    property(type, "value", method<T, uint32_t() const>(&T::getValue));
    viewProperty(type, array<T>("bitFields", method<T, bool(unsigned) const>(&T::getBit), method<T, void(unsigned, bool)>(&T::setBit), method<T, unsigned() const>(&T::getBitCount)));
    inheritProperties<T>();
    finish<T>(type, "BitMask32", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}

void registerBitMask64(sol::state_view state) {
    if (state["BitMask64"].valid()) return;
    using T = BitMask64;
    auto type = registerType<T>(state, "BitMask64");
    type["new"] = sol::factories(factory<T>());
    type["setValue"] = method<T, void(uint64_t)>(&T::setValue);
    type["getValue"] = method<T, uint64_t() const>(&T::getValue);
    type["getBit"] = method<T, bool(unsigned) const>(&T::getBit);
    type["setBit"] = method<T, void(unsigned, bool)>(&T::setBit);
    type["getBitCount"] = method<T, unsigned() const>(&T::getBitCount);
    property(type, "value", method<T, uint64_t() const>(&T::getValue));
    viewProperty(type, array<T>("bitFields", method<T, bool(unsigned) const>(&T::getBit), method<T, void(unsigned, bool)>(&T::setBit), method<T, unsigned() const>(&T::getBitCount)));
    inheritProperties<T>();
    finish<T>(type, "BitMask64", [] { return LuaManager::isSerializing(); });
    tableConstructor(type);
    initializeEditor<T>(type);
}
}
