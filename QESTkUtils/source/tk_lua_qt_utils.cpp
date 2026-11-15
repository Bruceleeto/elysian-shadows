/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#include <QESTkUtils/tk_lua_qt_utils.h>

int sol_lua_push(sol::types<QString>, lua_State* state, const QString& value) {
    const QByteArray bytes = value.toUtf8();
    lua_pushlstring(state, bytes.constData(), static_cast<size_t>(bytes.size()));
    return 1;
}
QString sol_lua_get(sol::types<QString>, lua_State* state, int index, sol::stack::record& tracking) {
    tracking.use(1);
    size_t size = 0;
    const char* value = luaL_tolstring(state, index, &size);
    QString result = QString::fromUtf8(value, static_cast<qsizetype>(size));
    lua_pop(state, 1);
    return result;
}
int sol_lua_push(sol::types<QVariant>, lua_State* state, const QVariant& value) {
    if (!value.isValid() || value.isNull()) return sol::stack::push(state, sol::nil);
    if (value.typeId() == QMetaType::Bool) return sol::stack::push(state, value.toBool());
    switch (value.typeId()) {
    case QMetaType::Int: case QMetaType::UInt: case QMetaType::LongLong: case QMetaType::ULongLong:
        return sol::stack::push(state, value.toLongLong());
    case QMetaType::Float: case QMetaType::Double:
        return sol::stack::push(state, value.toDouble());
    default: break;
    }
    const QString text = value.toString();
    if (text == "nil") return sol::stack::push(state, sol::nil);
    if (text == "true" || text == "false") return sol::stack::push(state, text == "true");
    bool valid = false;
    auto integer = text.toLongLong(&valid);
    if (valid) return sol::stack::push(state, integer);
    auto number = text.toDouble(&valid);
    if (valid) return sol::stack::push(state, number);
    return sol_lua_push(sol::types<QString>(), state, text);
}
QVariant sol_lua_get(sol::types<QVariant>, lua_State* state, int index, sol::stack::record& tracking) {
    tracking.use(1);
    switch (lua_type(state, index)) {
    case LUA_TNIL: return {};
    case LUA_TBOOLEAN: return QVariant(lua_toboolean(state, index) != 0);
    case LUA_TNUMBER:
        if (lua_isinteger(state, index)) return QVariant::fromValue(static_cast<qlonglong>(lua_tointeger(state, index)));
        return QVariant(lua_tonumber(state, index));
    default:
        sol::stack::record ignored;
        return sol_lua_get(sol::types<QString>(), state, index, ignored);
    }
}
