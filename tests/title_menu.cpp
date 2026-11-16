#include <lua.hpp>
#include <iostream>

int main() {
    lua_State* state = luaL_newstate();
    if(!state) return 2;
    luaL_openlibs(state);
    int result = luaL_dofile(state, "tests/title_menu.lua");
    if(result == LUA_OK) result = luaL_dofile(state, "tests/apartment_scene.lua");
    if(result != LUA_OK) std::cerr << lua_tostring(state, -1) << '\n';
    lua_close(state);
    return result == LUA_OK ? 0 : 1;
}
