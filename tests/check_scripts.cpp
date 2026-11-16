#include <lua.hpp>
#include <iostream>

// Parse scripts without executing engine calls or launching the editor.
int main(int argc, char** argv) {
    lua_State* state = luaL_newstate();
    if (!state) return 2;
    int failures = 0;
    for (int i = 1; i < argc; ++i) {
        if (luaL_loadfile(state, argv[i]) != LUA_OK) {
            std::cerr << lua_tostring(state, -1) << '\n';
            ++failures;
        }
        lua_settop(state, 0);
    }
    lua_close(state);
    std::cout << argc - 1 << " Lua files parsed; " << failures << " failures\n";
    return failures ? 1 : 0;
}
