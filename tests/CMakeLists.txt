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

# OpenAL Soft null output exercises context lifetime without audio hardware.
add_executable(audio_context audio_context.c)
target_link_libraries(audio_context PRIVATE libGyro)
target_compile_options(audio_context PRIVATE -Werror)
add_test(NAME audio_context COMMAND audio_context)
set_tests_properties(audio_context PROPERTIES ENVIRONMENT "ALSOFT_DRIVERS=null" TIMEOUT 10)

# Exercise the real bloom passes without Qt windows or a physical GPU.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    pkg_check_modules(EGL QUIET IMPORTED_TARGET egl)
    if(EGL_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/assets/shaders/bloomComposite.fsh")
        add_executable(bloom_gl bloom_gl.c bloom_assets.cpp)
        target_link_libraries(bloom_gl PRIVATE libGyro PkgConfig::EGL)
        target_link_options(bloom_gl PRIVATE "LINKER:--wrap=glalGetProcAddress")
        target_compile_options(bloom_gl PRIVATE -Werror)
        add_test(NAME bloom_gl COMMAND bloom_gl)
        set_tests_properties(bloom_gl PROPERTIES
            WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/assets"
            ENVIRONMENT "EGL_PLATFORM=surfaceless;LIBGL_ALWAYS_SOFTWARE=1"
            SKIP_RETURN_CODE 77 TIMEOUT 30)
    endif()
endif()

# The title behavior runs against mocked services; no editor or game is launched.
if(EXISTS "${PROJECT_SOURCE_DIR}/assets/level/rewrite/00_title/area/title/title_behav.lua")
    add_executable(title_menu title_menu.cpp)
    target_link_libraries(title_menu PRIVATE PkgConfig::Lua)
    target_compile_options(title_menu PRIVATE -Werror)
    add_test(NAME title_menu COMMAND title_menu WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()
