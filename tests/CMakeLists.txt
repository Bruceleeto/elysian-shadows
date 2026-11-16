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

# Exercise the real libGyro backends using a virtual gamepad and loopback TCP.
add_executable(platform_sdl3 platform_sdl3.c)
target_link_libraries(platform_sdl3 PRIVATE libGyro)
target_compile_options(platform_sdl3 PRIVATE -Werror)
add_test(NAME platform_sdl3 COMMAND platform_sdl3)
set_tests_properties(platform_sdl3 PROPERTIES TIMEOUT 20)

# Linux link wrapping captures force-feedback effects without physical hardware.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(haptic_sdl3 haptic_sdl3.c)
    target_link_libraries(haptic_sdl3 PRIVATE libGyro)
    target_compile_options(haptic_sdl3 PRIVATE -Werror)
    foreach(function IsJoystickHaptic OpenHapticFromJoystick GetHapticFeatures
            HapticRumbleSupported InitHapticRumble CloseHaptic CreateHapticEffect
            DestroyHapticEffect RunHapticEffect StopHapticEffects PlayHapticRumble)
        target_link_options(haptic_sdl3 PRIVATE "LINKER:--wrap=SDL_${function}")
    endforeach()
    add_test(NAME haptic_sdl3 COMMAND haptic_sdl3)
    set_tests_properties(haptic_sdl3 PROPERTIES TIMEOUT 20)
endif()
