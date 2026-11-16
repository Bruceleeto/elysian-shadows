/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_LUA_BINDING_HPP
#define ELYSIAN_LUA_BINDING_HPP

#include <sol/sol.hpp>
#include <AssetIO/utility/elysian_lua_identity.hpp>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace elysian::lua {

// Bind self as the registered type, then let C++ adjust it to the declaring
// base. Besides handling QObject-first layouts, this avoids GCC's alignment
// sanitizer false positive for converted base-to-derived member pointers.
template<class Signature> struct Method;
template<class R, class... A> struct Method<R(A...)> {
    template<class T, class C> static auto bind(R (C::*value)(A...)) {
        return [value](T& self, A... args) -> R {
            return std::invoke(value, static_cast<C&>(self), std::forward<A>(args)...);
        };
    }
};
template<class R, class... A> struct Method<R(A...) const> {
    template<class T, class C> static auto bind(R (C::*value)(A...) const) {
        return [value](const T& self, A... args) -> R {
            return std::invoke(value, static_cast<const C&>(self), std::forward<A>(args)...);
        };
    }
};
template<class T, class Signature, class C>
auto method(Signature C::* value) { return Method<Signature>::template bind<T>(value); }

struct Ownership;

// Every live guard, so the engine can destroy Lua-owned objects at shutdown
// while its systems still exist. Never destroyed: guards are collected when
// the Lua state closes during static destruction.
inline std::unordered_set<Ownership*>& ownerships() {
    static auto* live = new std::unordered_set<Ownership*>;
    return *live;
}

struct Ownership {
    void* pointer;
    void (*destroy)(void*);
    Ownership(void* p, void (*d)(void*)) : pointer(p), destroy(d) { ownerships().insert(this); }
    Ownership(const Ownership&) = delete;
    Ownership& operator=(const Ownership&) = delete;
    ~Ownership() {
        ownerships().erase(this);
        if (pointer) destroy(pointer);
    }
};

// Destroys every object still owned by Lua, leaving its userdata empty.
// Destructors may run Lua and destroy other owners, so rescan after each one.
inline void destroyOwnedObjects() {
    for (;;) {
        auto& live = ownerships();
        auto owner = std::find_if(live.begin(), live.end(), [](const Ownership* o) { return o->pointer; });
        if (owner == live.end()) return;
        void* pointer = (*owner)->pointer;
        (*owner)->pointer = nullptr;
        (*owner)->destroy(pointer);
    }
}

// The owner lives in the pointer userdata's uservalue, so releasing ownership
// does not replace the userdata or invalidate existing Lua references.
template<class T> struct Owned { std::unique_ptr<T> value; };
template<class T, class... A> Owned<T> own(A&&... args) {
    return {std::make_unique<T>(std::forward<A>(args)...)};
}

inline void releaseOwnership(const sol::object& value) {
    lua_State* l = value.lua_state();
    value.push();
    if (lua_getiuservalue(l, -1, 1) == LUA_TTABLE) {
        lua_getfield(l, -1, "__owner");
        if (auto guard = sol::stack::check_get<Ownership*>(l, -1)) {
            if (*guard) (*guard)->pointer = nullptr;
        }
        lua_pop(l, 1);
    }
    lua_pop(l, 2);
}

template<class T> T* take(const sol::object& value) {
    T* pointer = value.as<T*>();
    if (pointer) releaseOwnership(value);
    return pointer;
}

template<class T> int sol_lua_push(lua_State* l, Owned<T>&& owner) {
    T* pointer = owner.value.get();
    sol::stack::push(l, pointer);
    if (!pointer) return 1;
    lua_newtable(l);
    // Construct the guard in place: copying an owning guard would double-delete.
    sol::stack::push(l, std::make_unique<Ownership>(nullptr, [](void* p) { delete static_cast<T*>(p); }));
    auto* guard = sol::stack::get<Ownership*>(l, -1);
    guard->pointer = owner.value.release();
    lua_setfield(l, -2, "__owner");
    lua_setiuservalue(l, -2, 1);
    return 1;
}

struct Property {
    std::string name;
    bool writable;
    std::vector<Property> children;
    bool array = false;
};

// Metadata contains no Lua references and is safe across Lua-state recreation.
template<class T> inline std::vector<Property> properties;
template<class T> inline std::string className;
template<class T> inline void (*propertyChanged)(T&, const sol::object&) = nullptr;

template<class F> struct Arguments : Arguments<decltype(&F::operator())> {};
template<class R, class C, class... A> struct Arguments<R(C::*)(A...) const> { using tuple = std::tuple<A...>; };
template<class R, class C, class... A> struct Arguments<R(C::*)(A...)> { using tuple = std::tuple<A...>; };
template<class R, class... A> struct Arguments<R(*)(A...)> { using tuple = std::tuple<A...>; };

template<class T, class... Bases>
sol::usertype<T> registerType(sol::state_view state, const char* name) {
    properties<T>.clear();
    className<T> = name;
    registerIdentity<T, Bases...>();
    auto type = state.new_usertype<T>(name, sol::no_constructor,
        sol::base_classes, sol::bases<Bases...>());
    type["new"] = sol::make_object(state, sol::nil);
    return type;
}

struct ViewField {
    Property info;
    std::function<sol::object(const sol::object&)> read;
    std::function<void(const sol::object&, const sol::object&)> write;
};

// A live view retains its owning userdata. It never exposes a pointer into an
// object that can be collected while a nested field or array is still in use.
struct View {
    sol::object owner;
    std::string name;
    std::shared_ptr<const std::vector<ViewField>> fields;
    std::function<std::size_t(const sol::object&)> size;
    std::function<sol::object(const sol::object&, std::size_t)> read;
    std::function<void(const sol::object&, std::size_t, const sol::object&)> write;
    std::function<bool()> serializing;
    std::shared_ptr<void> observer = {};
};
inline void (*viewChanged)(View&, const sol::object&) = nullptr;
inline void (*viewConnect)(View&, void*) = nullptr;

template<class T, class... A> auto factory() {
    return [](A... args) { return own<T>(std::forward<A>(args)...); };
}

template<class T, class... A> auto sharedFactory() {
    return [](A... args) { return std::make_shared<T>(std::forward<A>(args)...); };
}

inline sol::object get(const sol::object& object, const sol::object& key) {
    lua_State* l = object.lua_state() ? object.lua_state() : key.lua_state();
    if (!l) return {};
    if (!object.valid() || (object.get_type() != sol::type::table && object.get_type() != sol::type::userdata))
        return sol::make_object(l, sol::nil);
    object.push();
    sol::stack::push(l, key);
    lua_gettable(l, -2);
    sol::object result(l, sol::stack_reference(l, -1));
    lua_pop(l, 2);
    return result;
}

inline void set(const sol::object& object, const sol::object& key, const sol::object& value) {
    lua_State* l = object.lua_state();
    object.push();
    sol::stack::push(l, key);
    sol::stack::push(l, value);
    lua_settable(l, -3);
    lua_pop(l, 1);
}

inline sol::object viewGet(const View& view, const sol::object& key) {
    if (key.is<std::string>() && key.as<std::string>() == "__ud") return view.owner;
    if (view.size && key.is<lua_Integer>()) {
        auto index = key.as<lua_Integer>();
        if (index > 0 && static_cast<std::size_t>(index) <= view.size(view.owner))
            return view.read(view.owner, static_cast<std::size_t>(index - 1));
    } else if (view.fields && key.is<std::string>()) {
        const auto name = key.as<std::string>();
        for (const auto& field : *view.fields)
            if (field.info.name == name) return field.read(view.owner);
    }
    return sol::make_object(key.lua_state(), sol::nil);
}

inline void viewSet(View& view, const sol::object& key, const sol::object& value) {
    if (view.size && view.write && key.is<lua_Integer>()) {
        auto index = key.as<lua_Integer>();
        if (index > 0) {
            view.write(view.owner, static_cast<std::size_t>(index - 1), value);
            if (viewChanged) viewChanged(view, key);
            return;
        }
    } else if (view.fields && key.is<std::string>()) {
        const auto name = key.as<std::string>();
        for (const auto& field : *view.fields) {
            if (field.info.name == name && field.write) {
                field.write(view.owner, value);
                if (viewChanged) viewChanged(view, key);
                return;
            }
        }
    }
    luaL_error(key.lua_state(), "Unknown or read-only field in %s", view.name.c_str());
}

inline void applyView(View& view, const sol::object& source) {
    if (view.fields) {
        for (const auto& field : *view.fields) {
            if (!field.write) continue;
            auto value = get(source, sol::make_object(source.lua_state(), field.info.name));
            if (value.valid()) field.write(view.owner, value);
        }
    } else if (view.write) {
        lua_State* l = source.lua_state();
        source.push();
        lua_len(l, -1);
        auto count = static_cast<std::size_t>(lua_tointeger(l, -1));
        lua_pop(l, 2);
        count = std::max(count, view.size(view.owner));
        for (std::size_t i = 0; i < count; ++i) {
            auto value = get(source, sol::make_object(l, i + 1));
            if (value.valid()) view.write(view.owner, i, value);
        }
    }
}

inline void registerView(sol::state_view state) {
    state.new_usertype<View>("__PropertyView", sol::no_constructor,
        "__connectSignals", [](View& view, void* receiver) { if (viewConnect) viewConnect(view, receiver); },
        sol::meta_function::index, &viewGet,
        sol::meta_function::new_index, &viewSet,
        sol::meta_function::length, [](const View& view) { return view.size ? view.size(view.owner) : 0; },
        sol::meta_function::to_string, [](const View& view) {
            auto name = get(view.owner, sol::make_object(view.owner.lua_state(), "__typename"));
            const auto prefix = name.valid() ? name.as<std::string>() + "." : std::string();
            return "userdata[" + prefix + view.name + "]: " + (view.size ? "array proxy" : "virtual proxy");
        },
        sol::meta_function::pairs, [](const View& view) {
            auto next = [view, index = std::size_t{0}]() mutable -> std::tuple<sol::object, sol::object> {
                auto l = view.owner.lua_state();
                if (view.size) {
                    if (index < view.size(view.owner)) {
                        const auto i = index++;
                        return {sol::make_object(l, i + 1), view.read(view.owner, i)};
                    }
                } else {
                    while (index < view.fields->size()) {
                        const auto& field = (*view.fields)[index++];
                        if (view.serializing && view.serializing() && !field.info.writable) continue;
                        return {sol::make_object(l, field.info.name), field.read(view.owner)};
                    }
                }
                return {sol::make_object(l, sol::nil), sol::make_object(l, sol::nil)};
            };
            return std::make_tuple(sol::as_function(std::move(next)), sol::nil, sol::nil);
        });
}

template<class T, class Getter>
ViewField field(const char* name, Getter getter) {
    return {{name, false, {}, false}, [getter](const sol::object& self) {
        return sol::make_object(self.lua_state(), std::invoke(getter, self.as<T&>()));
    }, {}};
}

template<class T, class Getter, class C, class R, class A>
ViewField field(const char* name, Getter getter, R (C::*setter)(A)) {
    auto entry = field<T>(name, getter);
    entry.info.writable = true;
    entry.write = [setter](const sol::object& self, const sol::object& value) {
        std::invoke(setter, self.as<T&>(), value.as<A>());
    };
    return entry;
}

template<class T, class Getter, class C, class R, class A>
ViewField field(const char* name, Getter getter, R (C::*setter)(A) const) {
    auto entry = field<T>(name, getter);
    entry.info.writable = true;
    entry.write = [setter](const sol::object& self, const sol::object& value) {
        std::invoke(setter, self.as<T&>(), value.as<A>());
    };
    return entry;
}

template<class T, class Getter, class Setter, std::enable_if_t<!std::is_member_pointer_v<Setter>, int> = 0>
ViewField field(const char* name, Getter getter, Setter setter) {
    auto entry = field<T>(name, getter);
    using Argument = std::tuple_element_t<1, typename Arguments<Setter>::tuple>;
    entry.info.writable = true;
    entry.write = [setter](const sol::object& self, const sol::object& value) {
        std::invoke(setter, self.as<T&>(), value.as<Argument>());
    };
    return entry;
}

inline ViewField group(const char* name, std::vector<ViewField> fields, std::function<bool()> serializing) {
    Property info{name, true, {}, false};
    for (const auto& field : fields) info.children.push_back(field.info);
    auto shared = std::make_shared<const std::vector<ViewField>>(std::move(fields));
    auto create = [name = std::string(name), shared, serializing](const sol::object& self) {
        return View{self, name, shared, {}, {}, {}, serializing};
    };
    return {std::move(info), [create](const sol::object& self) {
        return sol::make_object(self.lua_state(), create(self));
    }, [create](const sol::object& self, const sol::object& source) {
        auto view = create(self);
        applyView(view, source);
    }};
}

template<class T, class Getter, class Counter>
ViewField array(const char* name, Getter getter, Counter counter) {
    auto create = [name = std::string(name), getter, counter](const sol::object& self) {
        View view{self, name, {}, {}, {}, {}, {}};
        view.size = [counter](const sol::object& owner) { return std::invoke(counter, owner.as<T&>()); };
        view.read = [getter](const sol::object& owner, std::size_t index) {
            return sol::make_object(owner.lua_state(), std::invoke(getter, owner.as<T&>(), index));
        };
        return view;
    };
    return {{name, false, {}, true}, [create](const sol::object& self) {
        return sol::make_object(self.lua_state(), create(self));
    }, {}};
}

template<class T, class Getter, class C, class R, class I, class A, class Counter>
ViewField array(const char* name, Getter getter, R (C::*setter)(I, A), Counter counter) {
    auto entry = array<T>(name, getter, counter);
    auto read = entry.read;
    entry.info.writable = true;
    entry.read = [read, setter](const sol::object& self) {
        auto object = read(self);
        object.template as<View&>().write = [setter](const sol::object& owner, std::size_t index, const sol::object& value) {
            std::invoke(setter, owner.as<T&>(), static_cast<I>(index), value.as<A>());
        };
        return object;
    };
    entry.write = [read = entry.read](const sol::object& self, const sol::object& source) {
        auto object = read(self);
        applyView(object.template as<View&>(), source);
    };
    return entry;
}

template<class T, class Getter, class Setter, class Counter, std::enable_if_t<!std::is_member_pointer_v<Setter>, int> = 0>
ViewField array(const char* name, Getter getter, Setter setter, Counter counter) {
    using Index = std::tuple_element_t<1, typename Arguments<Setter>::tuple>;
    using Argument = std::tuple_element_t<2, typename Arguments<Setter>::tuple>;
    auto entry = array<T>(name, getter, counter);
    entry.info.writable = true;
    entry.read = [read = entry.read, setter](const sol::object& self) {
        auto object = read(self);
        object.template as<View&>().write = [setter](const sol::object& owner, std::size_t index, const sol::object& value) {
            std::invoke(setter, owner.as<T&>(), static_cast<Index>(index), value.as<Argument>());
        };
        return object;
    };
    entry.write = [read = entry.read](const sol::object& self, const sol::object& source) {
        auto object = read(self);
        applyView(object.template as<View&>(), source);
    };
    return entry;
}

template<class T>
void viewProperty(sol::usertype<T>& type, ViewField field) {
    auto read = [read = field.read](T&, sol::this_state l) {
        return read(sol::object(l, sol::stack_reference(l, 1)));
    };
    if (field.write) {
        auto write = [write = field.write, key = field.info.name](T& self, sol::object value, sol::this_state l) {
            write(sol::object(l, sol::stack_reference(l, 1)), value);
            if (propertyChanged<T>) propertyChanged<T>(self, sol::make_object(l, key));
        };
        type[field.info.name] = sol::property(read, write);
    } else type[field.info.name] = sol::property(read);
    properties<T>.push_back(std::move(field.info));
}

inline void apply(const sol::object& target, const sol::object& source, const std::vector<Property>& fields) {
    for (const auto& field : fields) {
        if (!field.writable) continue;
        auto key = sol::make_object(target.lua_state(), field.name);
        auto value = get(source, key);
        if (value.valid()) set(target, key, value);
    }
}

template<class T, class Getter, class Setter>
void property(sol::usertype<T>& type, const char* name, Getter getter, Setter setter) {
    if constexpr (std::is_member_function_pointer_v<Setter>) {
        property(type, name, getter, method<T>(setter));
        return;
    } else {
        using Argument = std::tuple_element_t<1, typename Arguments<Setter>::tuple>;
        type[name] = sol::property(getter, [setter, key = std::string(name)](T& self, Argument value, sol::this_state l) {
            std::invoke(setter, self, value);
            if (propertyChanged<T>) propertyChanged<T>(self, sol::make_object(l, key));
        });
    }
    properties<T>.push_back({name, true, {}, false});
}

template<class T> void eraseProperty(const char* name) {
    auto& list = properties<T>;
    list.erase(std::remove_if(list.begin(), list.end(), [name](const Property& p) { return p.name == name; }), list.end());
}

template<class T, class Getter>
void property(sol::usertype<T>& type, const char* name, Getter getter) {
    type[name] = sol::property(getter);
    properties<T>.push_back({name, false, {}, false});
}

template<class T, class... Bases>
void inheritProperties() {
    if constexpr (sizeof...(Bases) > 0) {
    auto append = [](const auto& fields) {
        for (const auto& field : fields) {
            auto& ownFields = properties<T>;
            if (std::none_of(ownFields.begin(), ownFields.end(), [&](const Property& ownField) { return ownField.name == field.name; }))
                ownFields.push_back(field);
        }
    };
    (append(properties<Bases>), ...);
    }
}

// Wrap the constructor once, retaining positional overloads as well as the
// engine's class/table and instance/table construction forms.
template<class T>
void tableConstructor(sol::usertype<T>& type) {
    sol::protected_function construct = type["new"];
    type.push();
    sol::table classTable(type.lua_state(), sol::stack_reference(type.lua_state(), -1));
    lua_pop(type.lua_state(), 1);
    type["new"] = [construct, classTable](sol::variadic_args args, sol::this_state state) -> sol::object {
        lua_State* l = state;
        int first = 0;
        if (args.size() && sol::object(args[0]) == classTable) ++first;
        const int count = static_cast<int>(args.size()) - first;
        sol::object source = sol::make_object(l, sol::nil);
        sol::object values = sol::make_object(l, sol::nil);
        if (count == 1 && args[first].get_type() == sol::type::table) {
            values = args[first];
        } else if ((count == 1 || count == 2) && args[first].is<T*>() &&
                   (count == 1 || args[first + 1].get_type() == sol::type::table)) {
            source = args[first];
            if (count == 2) values = args[first + 1];
        }
        sol::protected_function_result result = (source.valid() || values.valid())
            ? construct()
            : construct(sol::as_args(sol::variadic_args(l, args.stack_index() + first)));
        if (!result.valid()) {
            sol::error error = result;
            luaL_error(l, "%s", error.what());
        }
        sol::object object = result;
        if (source.valid()) apply(object, source, properties<T>);
        if (values.valid()) apply(object, values, properties<T>);
        return object;
    };
}

template<class T, class Serializing>
void finish(sol::usertype<T>& type, const char* name, Serializing serializing) {
    type["__typename"] = sol::make_object(type.lua_state(), name);
    type[sol::meta_function::to_string] = [name](T&) { return std::string("userdata[") + name + "]: proxy"; };
    type["fromTable"] = [](sol::object self, sol::object source) { apply(self, source, properties<T>); };
    type[sol::meta_function::pairs] = [serializing](sol::object self) {
        auto next = [self, serializing, index = std::size_t{0}]() mutable -> std::tuple<sol::object, sol::object> {
            while (index < properties<T>.size()) {
                const auto& field = properties<T>[index++];
                if (serializing() && !field.writable) continue;
                auto key = sol::make_object(self.lua_state(), field.name);
                return {key, get(self, key)};
            }
            return {sol::make_object(self.lua_state(), sol::nil), sol::make_object(self.lua_state(), sol::nil)};
        };
        return std::make_tuple(sol::as_function(std::move(next)), self, sol::nil);
    };

    // Scripts override engine methods by assigning to the class table
    // (function Character.getAnimNameFromStates ...). sol2 would store that
    // in the class table only, leaving instances calling the C++ binding, so
    // route the assignment through the usertype to replace the binding.
    // Functions are stored as bound methods, not usertype variables: instance
    // assignments (prefab.new = ...) must stay no-ops, as they were with OOLua,
    // where sol2 would write a variable for every instance of the class.
    type.push();
    sol::table classTable(type.lua_state(), sol::stack_reference(type.lua_state(), -1));
    lua_pop(type.lua_state(), 1);
    sol::table classMetatable = classTable[sol::metatable_key];
    classMetatable[sol::meta_function::new_index] = [type](sol::object, sol::object key, sol::object value, sol::this_state state) mutable {
        if (key.get_type() != sol::type::string) luaL_error(state, "class table keys must be strings");
        if (value.get_type() != sol::type::function) {
            type.set(key.as<std::string>(), value);
            return;
        }
        type.set(key.as<std::string>(), [function = sol::protected_function(value)](sol::variadic_args args, sol::this_state l) {
            sol::variadic_results results;
            std::string error;
            {
                sol::protected_function_result result = function(args);
                if (result.valid()) {
                    for (const auto& item : result) results.push_back(item);
                    return results;
                }
                error = sol::error(result).what();
            }
            luaL_error(l, "%s", error.c_str());
            return results;
        });
    };
}

} // namespace elysian::lua
#endif
