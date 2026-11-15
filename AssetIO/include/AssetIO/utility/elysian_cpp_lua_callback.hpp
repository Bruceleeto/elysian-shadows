/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_CPP_LUA_CALLBACK_HPP
#define ELYSIAN_CPP_LUA_CALLBACK_HPP
#include <AssetIO/utility/elysian_lua_values.hpp>
#include <cstdio>
#include <variant>

namespace elysian {
template<class Ret, class... Args> class CppLuaCallback {
public:
    using StdCallback = std::function<Ret(Args...)>;
    enum class Type { Null, Cpp, Lua };
private:
    std::variant<std::monostate, StdCallback, sol::protected_function> _callback;
public:
    CppLuaCallback() = default;
    CppLuaCallback(std::nullptr_t) {}
    CppLuaCallback(const sol::object& value) { setValue(value); }
    template<class F, std::enable_if_t<std::is_invocable_r_v<Ret, F, Args...> && !std::is_same_v<std::decay_t<F>, CppLuaCallback>, int> = 0>
    CppLuaCallback(F&& callback) : _callback(StdCallback(std::forward<F>(callback))) {}
    void setValue(std::nullptr_t) { _callback = std::monostate(); }
    void setValue(const sol::object& value) {
        if (value.valid() && value.get_type() == sol::type::function) _callback = value.as<sol::protected_function>();
        else setValue(nullptr);
    }
    template<class F, std::enable_if_t<std::is_invocable_r_v<Ret, F, Args...> && !std::is_same_v<std::decay_t<F>, CppLuaCallback>, int> = 0>
    void setValue(F&& callback) { _callback = StdCallback(std::forward<F>(callback)); }
    template<class F, std::enable_if_t<!std::is_same_v<std::decay_t<F>, CppLuaCallback>, int> = 0> CppLuaCallback& operator=(F&& value) { setValue(std::forward<F>(value)); return *this; }
    CppLuaCallback& operator=(std::nullptr_t) { setValue(nullptr); return *this; }
    Type getType() const { return static_cast<Type>(_callback.index()); }
    bool isNull() const { return !static_cast<bool>(*this); }
    explicit operator bool() const {
        if (auto* cpp = std::get_if<StdCallback>(&_callback)) return static_cast<bool>(*cpp);
        if (auto* lua = std::get_if<sol::protected_function>(&_callback)) return lua->valid();
        return false;
    }
    StdCallback getCppCallback() const { return std::get<StdCallback>(_callback); }
    sol::object getLuaCallback() const { return std::get<sol::protected_function>(_callback); }
    template<class... A> Ret operator()(A&&... args) const {
        if (auto* cpp = std::get_if<StdCallback>(&_callback)) return (*cpp)(std::forward<A>(args)...);
        if (auto* lua = std::get_if<sol::protected_function>(&_callback)) {
            auto result = (*lua)(std::forward<A>(args)...);
            if (result.valid()) {
                if constexpr (std::is_void_v<Ret>) return;
                else return result.template get<Ret>();
            }
            sol::error error = result;
            std::fprintf(stderr, "Lua callback failed: %s\n", error.what());
        }
        return Ret();
    }
    friend bool operator==(const CppLuaCallback& callback, std::nullptr_t) { return callback.isNull(); }
    friend bool operator!=(const CppLuaCallback& callback, std::nullptr_t) { return !callback.isNull(); }
    friend bool operator==(const CppLuaCallback& callback, const sol::object& value) {
        return callback.getType() == Type::Lua && callback.getLuaCallback() == value;
    }
    friend bool operator!=(const CppLuaCallback& callback, const sol::object& value) { return !(callback == value); }
};
}
#endif
