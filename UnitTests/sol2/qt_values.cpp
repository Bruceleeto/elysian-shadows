#include <QESTkUtils/tk_lua_qt_utils.h>
#include <AssetIO/utility/elysian_lua_values.hpp>
#include <cassert>
#include <iostream>

int main() {
    sol::state state;
    state.open_libraries(sol::lib::base);
    elysian::lua::activeState = state.lua_state();
    const QString text = QString::fromUtf8("Lua \\0 UTF-8: \xc3\xa9");
    state["text"] = text;
    assert(state.get<QString>("text") == text);
    state.set_function("echo", [](QString value) { return value; });
    auto result = state.safe_script("assert(echo(text) == text)", sol::script_pass_on_error);
    assert(result.valid());
    auto table = elysian::lua::table();
    elysian::lua::set(table, "integer", QVariant::fromValue(qlonglong(9007199254740993LL)));
    assert(elysian::lua::get(table, "integer").as<lua_Integer>() == 9007199254740993LL);
    elysian::lua::set(table, "value", QVariant(QString("false")));
    assert(!elysian::lua::get(table, "value").as<bool>());
    elysian::lua::set(table, "value", QVariant(QString("3.25")));
    assert(elysian::lua::get(table, "value").as<double>() == 3.25);
    elysian::lua::set(table, "value", QVariant(QString("nil")));
    assert(!elysian::lua::get(table, "value").valid());
    sol::object empty;
    assert(!elysian::lua::get(empty, "missing").valid());
    assert(elysian::lua::type(empty) == LUA_TNIL);
    sol::stack::push(state.lua_state(), QVariant::fromValue(qlonglong(42)));
    assert(sol::stack::pop<QVariant>(state.lua_state()).toLongLong() == 42);
    assert(lua_gettop(state.lua_state()) == 0);
    std::cout << "sol2 Qt value tests passed\n";
}
