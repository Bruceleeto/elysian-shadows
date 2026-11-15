/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 */
#ifndef ELYSIAN_LUA_OBJECT_HPP
#define ELYSIAN_LUA_OBJECT_HPP
#include <AssetIO/utility/elysian_lua_values.hpp>
#include <ESGamma/crtp/elysian_lua_manager.hpp>
#ifndef ELYSIAN_ENGINE
#include <ESToolkit/AssetIO/tk_lua_observer.h>
#include <QESTkUtils/tk_lua_qt_utils.h>
#endif

namespace elysian {
template<class P, class T = P, class... Bases>
class LuaObject : public LuaManager
#ifndef ELYSIAN_ENGINE
    , public LuaObjectSignals
#endif
{
public:
    using LuaObjectType = LuaObject;
    LuaObject() = default;
    LuaObject(const LuaObject&) {}
    LuaObject& operator=(const LuaObject&) { return *this; }
    sol::object getLuaObject() const {
        return lua::make(const_cast<T*>(static_cast<const T*>(static_cast<const P*>(this))));
    }
    void fromTable(const sol::object& source) {
        auto self = lua::make(static_cast<T*>(static_cast<P*>(this)));
        lua::apply(self, source, lua::properties<T>);
    }
    static int lua_downCast(lua_State*) { return 1; }
#ifndef ELYSIAN_ENGINE
    static void initializeDelegates() {}
    static void initializeVirtualTableDelegates(QString) {}
    static void connectSignals(T& object, QObject* receiver) {
        auto* sender = static_cast<LuaObject*>(static_cast<P*>(&object));
        sender->disconnect(receiver);
        QObject::connect(sender, SIGNAL(fieldChanged(const sol::object&)), receiver, SLOT(onFieldChanged(const sol::object&)));
        (Bases::LuaObjectType::connectSignals(static_cast<Bases&>(object), receiver), ...);
    }
    static void registerEditor(sol::usertype<T>& type) {
        type.push();
        T::initializeDelegates();
        lua_State* state = type.lua_state();
        sol::table classTable(state, sol::stack_reference(state, -1));
        lua_pop(state, 1);
        sol::table views = sol::state_view(state).create_table();
        std::function<void(const std::vector<lua::Property>&)> addViews;
        addViews = [&](const auto& fields) {
            for (const auto& field : fields) {
                if (!field.array && field.children.empty()) continue;
                sol::table metadata = sol::state_view(state).create_table();
                metadata.push();
                T::initializeVirtualTableDelegates(QString::fromStdString(field.name));
                lua_pop(state, 1);
                views[field.name] = metadata;
                addViews(field.children);
            }
        };
        addViews(lua::properties<T>);
        type["__viewMetadata"] = views;
        lua::viewConnect = [](lua::View& view, void* target) {
            if (!view.observer) view.observer = std::make_shared<LuaObjectSignals>();
            auto* sender = static_cast<LuaObjectSignals*>(view.observer.get());
            auto* receiver = static_cast<QObject*>(target);
            sender->disconnect(receiver);
            QObject::connect(sender, SIGNAL(fieldChanged(const sol::object&)), receiver, SLOT(onFieldChanged(const sol::object&)));
        };
        lua::viewChanged = [](lua::View& view, const sol::object& key) {
            if (view.observer) static_cast<LuaObjectSignals*>(view.observer.get())->emitFieldChanged(key);
        };
        const char* metatables[] = {
            sol::usertype_traits<T>::metatable().c_str(),
            sol::usertype_traits<T*>::metatable().c_str(),
            sol::usertype_traits<const T>::metatable().c_str(),
            sol::usertype_traits<const T*>::metatable().c_str(),
            sol::usertype_traits<sol::d::u<T>>::metatable().c_str()
        };
        for (const auto* name : metatables) {
            luaL_getmetatable(state, name);
            if (lua_istable(state, -1)) {
                for (const char* key : {TK_LUA_TABLE_DELEGATE_FIELD_KEY, TK_LUA_TABLE_OUTER_DELEGATE_FIELD_KEY}) {
                    sol::object entry = classTable[key];
                    entry.push();
                    lua_setfield(state, -2, key);
                }
                lua_pushcfunction(state, [](lua_State* l) {
                    auto* object = sol::stack::get<T*>(l, 1);
                    auto* receiver = static_cast<QObject*>(lua_touserdata(l, 2));
                    connectSignals(*object, receiver);
                    return 0;
                });
                lua_setfield(state, -2, "__connectSignals");
            }
            lua_pop(state, 1);
        }
        lua::propertyChanged<T> = [](T& object, const sol::object& key) {
            static_cast<LuaObject&>(static_cast<P&>(object)).emitFieldChanged(key);
        };
        sol::object name = classTable["__typename"];
        if (name.is<std::string>()) _udTypeNames.push_back(QString::fromStdString(name.as<std::string>()));
    }
#endif
};
}
#ifndef ELYSIAN_ENGINE
template<class P, class T = P, class... Bases>
using LuaObject = elysian::LuaObject<P, T, Bases...>;
#endif
namespace elysian::lua {
template<class T> void initializeEditor(sol::usertype<T>& type) {
#ifndef ELYSIAN_ENGINE
    T::LuaObjectType::registerEditor(type);
#else
    (void)type;
#endif
}
}
#endif
