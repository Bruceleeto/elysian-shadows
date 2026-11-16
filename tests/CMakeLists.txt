# Headless Lua binding checks: ctest (or make test) in the build directory.
# check_scripts is a tool: check_scripts <file.lua>... reports Lua 5.5 syntax errors.

add_executable(binding binding.cpp)
target_include_directories(binding PRIVATE ${PROJECT_SOURCE_DIR}/AssetIO/include)
target_link_libraries(binding PRIVATE sol2 PkgConfig::Lua)

add_executable(qt_values qt_values.cpp ${PROJECT_SOURCE_DIR}/QESTkUtils/source/tk_lua_qt_utils.cpp)
target_include_directories(qt_values PRIVATE
    ${PROJECT_SOURCE_DIR}/AssetIO/include ${PROJECT_SOURCE_DIR}/QESTkUtils/include)
target_link_libraries(qt_values PRIVATE sol2 PkgConfig::Lua Qt6::Core)

add_executable(check_scripts check_scripts.cpp)
target_link_libraries(check_scripts PRIVATE PkgConfig::Lua)

foreach(t binding qt_values check_scripts)
    target_compile_options(${t} PRIVATE -Werror)
endforeach()

# Serialization checks run when the assets submodule is checked out.
add_test(NAME binding COMMAND binding WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
add_test(NAME qt_values COMMAND qt_values)
