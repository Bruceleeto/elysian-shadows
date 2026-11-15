/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_LUA_IDENTITY_HPP
#define ELYSIAN_LUA_IDENTITY_HPP

#include <sol/sol.hpp>

namespace elysian::lua {

inline char rawIdentityKey;
inline void rawIdentityTable(lua_State* state) {
    lua_rawgetp(state, LUA_REGISTRYINDEX, &rawIdentityKey);
    if (!lua_isnil(state, -1)) return;
    lua_pop(state, 1);
    lua_newtable(state);
    lua_newtable(state);
    lua_pushliteral(state, "k");
    lua_setfield(state, -2, "__mode");
    lua_setmetatable(state, -2);
    lua_pushvalue(state, -1);
    lua_rawsetp(state, LUA_REGISTRYINDEX, &rawIdentityKey);
}
inline void markIdentityStorage(lua_State* state, int index, int kind) {
    index = lua_absindex(state, index);
    rawIdentityTable(state);
    lua_pushvalue(state, index);
    lua_pushinteger(state, kind);
    lua_rawset(state, -3);
    lua_pop(state, 1);
}
inline int identityStorage(lua_State* state, int index) {
    index = lua_absindex(state, index);
    rawIdentityTable(state);
    lua_pushvalue(state, index);
    lua_rawget(state, -2);
    int raw = static_cast<int>(lua_tointeger(state, -1));
    lua_pop(state, 2);
    return raw;
}
inline bool isRawIdentity(lua_State* state, int index) { return identityStorage(state, index) == 1; }
template<class T> inline bool (*identityFindBases)(lua_State*, T*) = nullptr;
template<class T> inline sol::detail::unique_tag sharedCast = nullptr;
template<class T> inline char identityKey;
template<class T> inline void (*identityAliases)(lua_State*, T*, int) = nullptr;

template<class T> void identityTable(lua_State* state) {
    lua_rawgetp(state, LUA_REGISTRYINDEX, &identityKey<std::remove_cv_t<T>>);
    if (!lua_isnil(state, -1)) return;
    lua_pop(state, 1);
    lua_newtable(state);
    lua_newtable(state);
    lua_pushliteral(state, "v");
    lua_setfield(state, -2, "__mode");
    lua_setmetatable(state, -2);
    lua_pushvalue(state, -1);
    lua_rawsetp(state, LUA_REGISTRYINDEX, &identityKey<std::remove_cv_t<T>>);
}

template<class T> bool findIdentity(lua_State* state, T* pointer) {
    identityTable<T>(state);
    lua_rawgetp(state, -1, pointer);
    lua_remove(state, -2);
    if (!lua_isnil(state, -1)) return true;
    lua_pop(state, 1);
    return false;
}

template<class T> void rememberIdentity(lua_State* state, T* pointer, int index) {
    index = lua_absindex(state, index);
    identityTable<T>(state);
    lua_pushvalue(state, index);
    lua_rawsetp(state, -2, pointer);
    lua_pop(state, 1);
}

template<class T, class... Bases> void registerIdentity() {
    sharedCast<T> = &sol::detail::inheritance<T>::template type_unique_cast_with<std::shared_ptr<T>, Bases...>;
    identityFindBases<T> = [](lua_State* state, T* pointer) {
        if constexpr (sizeof...(Bases) == 0) {
            (void)state;
            (void)pointer;
            return false;
        } else return (findIdentity(state, static_cast<Bases*>(pointer)) || ...);
    };
    identityAliases<T> = [](lua_State* state, T* pointer, int index) {
        rememberIdentity(state, pointer, index);
        (rememberIdentity(state, static_cast<Bases*>(pointer), index), ...);
    };
}

template<class T> int pushIdentity(lua_State* state, T* pointer) {
    using Type = std::remove_cv_t<T>;
    auto aliases = identityAliases<Type>;
    if (pointer && aliases && findIdentity(state, pointer)) return 1;
    if (pointer && aliases && identityFindBases<Type>(state, const_cast<Type*>(pointer))) {
        // Pointer userdata has a fixed pointer slot. Promote a base-first view
        // in place so existing Lua keys retain their identity and gain derived methods.
        if (isRawIdentity(state, -1)) {
            void* memory = sol::detail::align_usertype_pointer(lua_touserdata(state, -1));
            *static_cast<T**>(memory) = pointer;
            luaL_getmetatable(state, sol::usertype_traits<T*>::metatable().c_str());
            lua_setmetatable(state, -2);
            aliases(state, const_cast<Type*>(pointer), -1);
            return 1;
        }
        lua_pop(state, 1);
    }
    sol::stack::push<sol::detail::as_pointer_tag<T>>(state, pointer);
    if (pointer && aliases) markIdentityStorage(state, -1, 1);
    if (pointer && aliases) aliases(state, const_cast<Type*>(pointer), -1);
    return 1;
}

template<class T> void markShared(lua_State* state) {
    if (lua_isnil(state, -1)) return;
    markIdentityStorage(state, -1, 2);
    if (sharedCast<T>) {
        void* memory = sol::detail::align_usertype_unique_tag(lua_touserdata(state, -1));
        *static_cast<sol::detail::unique_tag*>(memory) = sharedCast<T>;
    }
}

template<class T> int pushIdentity(lua_State* state, const std::shared_ptr<T>& pointer) {
    // Keep the public identity in a pointer userdata. Shared ownership lives in
    // a native sol2 holder in its uservalue, allowing a base-first identity to
    // gain its derived type later without replacing the Lua value.
    pushIdentity(state, pointer.get());
    if (!pointer) return 1;
    const int object = lua_gettop(state);
    markIdentityStorage(state, object, 1);
    if (lua_getiuservalue(state, object, 1) != LUA_TTABLE) {
        lua_pop(state, 1);
        lua_newtable(state);
    }
    lua_getfield(state, -1, "__sharedOwner");
    if (identityStorage(state, -1) == 2) {
        void* memory = sol::detail::align_usertype_unique_tag(lua_touserdata(state, -1));
        auto cast = *static_cast<sol::detail::unique_tag*>(memory);
        if (cast(nullptr, nullptr, sol::usertype_traits<T>::qualified_name(),
                 sol::usertype_traits<std::shared_ptr<void>>::qualified_name()) != 0) {
            lua_pop(state, 2);
            return 1;
        }
    }
    lua_pop(state, 1);
    sol::stack::push<sol::detail::as_unique_tag<std::shared_ptr<T>>>(state, pointer);
    markShared<T>(state);
    lua_setfield(state, -2, "__sharedOwner");
    lua_setiuservalue(state, object, 1);
    return 1;
}

template<class T> std::shared_ptr<T> nativeShared(lua_State* state, int index, sol::stack::record& tracking) {
    tracking.use(1);
    void* memory = lua_touserdata(state, index);
    if (!memory) return {};
    memory = sol::detail::align_usertype_unique_destructor(memory);
    auto destroy = *static_cast<sol::detail::unique_destructor*>(memory);
    memory = sol::detail::align_usertype_unique_tag<true, false>(memory);
    auto cast = *static_cast<sol::detail::unique_tag*>(memory);
    memory = sol::detail::align_usertype_unique<std::shared_ptr<T>, true, false>(memory);
    if (destroy == &sol::detail::usertype_unique_alloc_destroy<T, std::shared_ptr<T>>)
        return *static_cast<std::shared_ptr<T>*>(memory);
    std::shared_ptr<T> result;
    const int operation = cast(memory, &result, sol::usertype_traits<T>::qualified_name(),
        sol::usertype_traits<std::shared_ptr<void>>::qualified_name());
    if (operation == 1) return *static_cast<std::shared_ptr<T>*>(memory);
    if (operation != 2) luaL_error(state, "Incompatible shared ownership type");
    return result;
}

template<class T, class Handler>
bool checkShared(lua_State* state, int index, Handler&& handler, sol::stack::record& tracking) {
    tracking.use(1);
    void* memory = sol::detail::align_usertype_unique_tag(lua_touserdata(state, index));
    auto cast = *static_cast<sol::detail::unique_tag*>(memory);
    if (cast(nullptr, nullptr, sol::usertype_traits<T>::qualified_name(),
             sol::usertype_traits<std::shared_ptr<void>>::qualified_name()) != 0) return true;
    handler(state, index, sol::type::userdata, sol::type_of(state, index), "incompatible shared ownership type");
    return false;
}

inline bool pushSharedOwner(lua_State* state, int index) {
    if (!isRawIdentity(state, index)) return false;
    if (lua_getiuservalue(state, index, 1) == LUA_TTABLE) {
        lua_getfield(state, -1, "__sharedOwner");
        lua_remove(state, -2);
        if (!lua_isnil(state, -1)) return true;
    }
    lua_pop(state, 1);
    return false;
}

} // namespace elysian::lua

// sol::types participates in ADL for both bound return values and ordinary
// stack pushes. Unregistered types retain sol2's normal representation.
namespace sol {
template<class T>
std::shared_ptr<T> sol_lua_get(types<std::shared_ptr<T>>, lua_State* state, int index, stack::record& tracking) {
    if (elysian::lua::pushSharedOwner(state, index)) {
        auto result = elysian::lua::nativeShared<T>(state, -1, tracking);
        lua_pop(state, 1);
        return result;
    }
    return elysian::lua::nativeShared<T>(state, index, tracking);
}
template<class T, class Handler>
bool sol_lua_check(types<std::shared_ptr<T>>, lua_State* state, int index, Handler&& handler, stack::record& tracking) {
    if (lua_isnil(state, index)) {
        tracking.use(1);
        return true;
    }
    if (elysian::lua::pushSharedOwner(state, index)) {
        bool result = elysian::lua::checkShared<T>(state, -1, std::forward<Handler>(handler), tracking);
        lua_pop(state, 1);
        return result;
    }
    if (elysian::lua::identityStorage(state, index) != 2) {
        tracking.use(1);
        handler(state, index, type::userdata, type_of(state, index), "object has no shared owner");
        return false;
    }
    return elysian::lua::checkShared<T>(state, index, std::forward<Handler>(handler), tracking);
}
template<class T, std::enable_if_t<std::is_class_v<T>, int> = 0>
int sol_lua_push(types<T*>, lua_State* state, T* pointer) {
    return elysian::lua::pushIdentity(state, pointer);
}
template<class T>
int sol_lua_push(types<std::shared_ptr<T>>, lua_State* state, const std::shared_ptr<T>& pointer) {
    return elysian::lua::pushIdentity(state, pointer);
}
}
#endif
