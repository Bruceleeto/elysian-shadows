/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef TK_LUA_OBSERVER_H
#define TK_LUA_OBSERVER_H

#include <AssetIO/utility/elysian_lua_values.hpp>
#include <QObject>
#include <QVariant>

#define TK_LUA_TABLE_USERDATA_FIELD_KEY         "__tkUd"
#define TK_LUA_TABLE_METAMETHODSET_FIELD_KEY    "__metaset"
#define TK_LUA_TABLE_PROPERTY_TABLE_FIELD_KEY   "__prop"
#define TK_LUA_TABLE_DELEGATE_FIELD_KEY         "__delegate"
#define TK_LUA_TABLE_OUTER_DELEGATE_FIELD_KEY   "__outerdelegate"
#define TK_LUA_TABLE_KEY_MAX_LENGTH             500


class TkLuaTableUserdata: public QObject {
    Q_OBJECT

private:
    int                 _propRef    = LUA_NOREF;
    int                 _origRef    = LUA_NOREF;

    static int          _metaRef;
    static lua_State*   _lvm;

Q_SIGNALS:
    //Notifies Qt's GUI models to update when Lua table fields change
    void    fieldChanged(const sol::object& key) const;
    void    fieldAdded(const sol::object& key) const;

public:
            TkLuaTableUserdata(void);
    virtual ~TkLuaTableUserdata(void);
    void    emitFieldChanged(const sol::object& key) const;
    void    emitFieldAdded(const sol::object& key) const;

    void    setOrigRef(const int ref);
    int     getOrigRef(void) const;

    void    setPropRef(const int ref);
    int     getPropRef(void) const;

    //static
    static TkLuaTableUserdata*  inject(lua_State*const l, const sol::object& ref);
    static void                 staticInitialize(void);

    //static bool                 hasTkUserdata(sol::object& ref);

private:
    static void                 _createMetatable(lua_State*const l);

    //metamethods
    static int                  _gcMetamethod(lua_State*const l);
    static int                  _indexMetamethod(lua_State*const l);
    static int                  _newIndexMetamethod(lua_State*const l);
    static int                  _pairsMetamethod(lua_State*const l);
    static int                  _ipairsMetamethod(lua_State*const l);
    static int                  _nextMetamethod(lua_State*const l);
    static int                  _lengthMetamethod(lua_State*const l);
    static int                  _metamethodSetMetamethod(lua_State* const l);
};


class LuaObjectSignals: public QObject {
    Q_OBJECT
Q_SIGNALS:
    //Notifies Qt's GUI models to update when Lua table fields change
    void fieldChanged(const sol::object& key);
    void fieldAdded(const sol::object& key);

public:
    void emitFieldChanged(const sol::object& key) { Q_EMIT fieldChanged(key); }
    void emitFieldAdded(const sol::object& key) { Q_EMIT fieldAdded(key); }
};

inline void TkLuaTableUserdata::setOrigRef(const int ref) { _origRef = ref; }
inline void TkLuaTableUserdata::setPropRef(const int ref) { _propRef = ref; }
inline int TkLuaTableUserdata::getOrigRef() const { return _origRef; }
inline int TkLuaTableUserdata::getPropRef() const { return _propRef; }
inline TkLuaTableUserdata::TkLuaTableUserdata() : QObject(nullptr) {}
inline void TkLuaTableUserdata::emitFieldAdded(const sol::object& key) const { Q_EMIT fieldAdded(key); }
inline void TkLuaTableUserdata::emitFieldChanged(const sol::object& key) const { Q_EMIT fieldChanged(key); }
#endif
