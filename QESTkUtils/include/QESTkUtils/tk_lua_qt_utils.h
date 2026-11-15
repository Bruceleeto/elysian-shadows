/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef TK_LUA_QT_UTILS_H
#define TK_LUA_QT_UTILS_H
#include <sol/sol.hpp>
#include <QHash>
#include <QVariant>
#include <QString>

int sol_lua_push(sol::types<QString>, lua_State* state, const QString& value);
QString sol_lua_get(sol::types<QString>, lua_State* state, int index, sol::stack::record& tracking);
int sol_lua_push(sol::types<QVariant>, lua_State* state, const QVariant& value);
QVariant sol_lua_get(sol::types<QVariant>, lua_State* state, int index, sol::stack::record& tracking);

template<class Handler>
bool sol_lua_check(sol::types<QString>, lua_State*, int, Handler&&, sol::stack::record& tracking) {
    tracking.use(1);
    return true;
}
template<class Handler>
bool sol_lua_check(sol::types<QVariant>, lua_State*, int, Handler&&, sol::stack::record& tracking) {
    tracking.use(1);
    return true;
}
inline size_t qHash(const QVariant& value, size_t seed = 0) {
    if (value.typeId() == QMetaType::Bool) return qHash(value.toBool(), seed);
    if (value.canConvert(QMetaType(QMetaType::Double))) return qHash(value.toDouble(), seed);
    return qHash(value.toString(), seed);
}
Q_DECLARE_METATYPE(sol::object)
#endif
