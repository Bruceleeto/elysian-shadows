/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_LUA_VALUES_HPP
#define ELYSIAN_LUA_VALUES_HPP
#include <AssetIO/utility/elysian_lua_binding.hpp>
#include <cstdio>

namespace elysian::lua {
inline lua_State* activeState = nullptr;

template<class T> sol::object make(T&& value) {
    return sol::make_object(activeState, std::forward<T>(value));
}
inline sol::object make(std::nullptr_t) { return make(sol::nil); }
inline sol::object make() { return make(sol::nil); }
inline sol::table table() { return sol::state_view(activeState).create_table(); }
inline sol::table table(std::initializer_list<std::pair<sol::object, sol::object>> entries) {
    auto result = table();
    for (const auto& entry : entries) result.set(entry.first, entry.second);
    return result;
}
inline sol::object fromStack(int index = -1, bool remove = true) {
    auto value = sol::stack::get<sol::object>(activeState, index);
    if (remove) lua_remove(activeState, index);
    return value;
}
inline sol::object fromRegistry(int ref) { return sol::object(activeState, sol::ref_index(ref)); }
inline int type(const sol::object& value) { return value.valid() ? static_cast<int>(value.get_type()) : LUA_TNIL; }
inline bool indexable(const sol::object& value) {
    return value.valid() && (value.get_type() == sol::type::table || value.get_type() == sol::type::userdata);
}
template<class T = lua_Number> T number(const sol::object& value) {
    sol::stack::push(activeState, value);
    const auto result = static_cast<T>(lua_tonumber(activeState, -1));
    lua_pop(activeState, 1);
    return result;
}
inline bool boolean(const sol::object& value) {
    if (!value.valid()) return false;
    switch (value.get_type()) {
    case sol::type::boolean: return value.as<bool>();
    case sol::type::string: return value.as<std::string>() == "true";
    case sol::type::number: return value.as<lua_Number>() >= 1;
    case sol::type::lightuserdata: return value.as<void*>() != nullptr;
    case sol::type::none: case sol::type::nil: return false;
    default: return true;
    }
}
inline std::string string(const sol::object& value) {
    sol::protected_function tostring = sol::state_view(activeState)["tostring"];
    sol::protected_function_result result = tostring(value);
    return result.valid() ? result.get<std::string>() : std::string();
}
inline std::string typeName(const sol::object& value) {
    if (indexable(value)) {
        auto name = get(value, make("__typename"));
        if (name.get_type() == sol::type::string) return name.as<std::string>();
    }
    return lua_typename(activeState, type(value));
}
inline std::size_t length(const sol::object& value) {
    sol::stack::push(activeState, value);
    lua_len(activeState, -1);
    auto result = static_cast<std::size_t>(lua_tointeger(activeState, -1));
    lua_pop(activeState, 2);
    return result;
}
inline sol::object metatable(const sol::object& value) {
    sol::stack::push(activeState, value);
    sol::object result = make();
    if (lua_getmetatable(activeState, -1)) result = fromStack();
    lua_pop(activeState, 1);
    return result;
}
inline void setMetatable(const sol::object& value, const sol::object& mt) {
    sol::stack::multi_push(activeState, value, mt);
    lua_setmetatable(activeState, -2);
    lua_pop(activeState, 1);
}
template<class K, std::enable_if_t<!std::is_same_v<std::decay_t<K>, sol::object>, int> = 0>
sol::object get(const sol::object& value, K&& key) {
    return get(value, make(std::forward<K>(key)));
}
template<class K, class V, std::enable_if_t<!std::is_same_v<std::decay_t<K>, sol::object> || !std::is_same_v<std::decay_t<V>, sol::object>, int> = 0>
void set(const sol::object& value, K&& key, V&& v) {
    set(value, make(std::forward<K>(key)), make(std::forward<V>(v)));
}
template<class... A> sol::object call(const sol::object& function, A&&... args) {
    sol::protected_function f = function.as<sol::protected_function>();
    auto result = f(std::forward<A>(args)...);
    if (!result.valid()) {
        sol::error error = result;
        std::fprintf(stderr, "Lua call failed: %s\n", error.what());
        return make();
    }
    return result.return_count() ? result.template get<sol::object>() : make();
}
inline std::vector<std::pair<sol::object, sol::object>> entries(const sol::object& value) {
    std::vector<std::pair<sol::object, sol::object>> output;
    sol::protected_function pairs = sol::state_view(activeState)["pairs"];
    auto start = pairs(value);
    if (!start.valid()) return output;
    sol::protected_function next = start.get<sol::protected_function>(0);
    auto state = start.get<sol::object>(1);
    auto key = start.get<sol::object>(2);
    for (;;) {
        auto step = next(state, key);
        if (!step.valid()) break;
        key = step.get<sol::object>(0);
        if (!key.valid()) break;
        output.emplace_back(key, step.get<sol::object>(1));
    }
    return output;
}
template<class Container> sol::table sequence(const Container& values) {
    auto result = table();
    int index = 1;
    for (const auto& value : values) result[index++] = value;
    return result;
}
}
#endif
