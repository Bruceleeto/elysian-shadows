# Elysian Shadows: libGyro, Spine, libGyro-Kinetic and ESToolkit.
#
#   mkdir build && cd build
#   cmake ..        # Linux: ESToolkit
#   kos-cmake ..    # Dreamcast: ESGamma.elf (not working yet)
#   make -j$(nproc)

cmake_minimum_required(VERSION 3.16)

project(ElysianShadows LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

add_compile_options(-Wall -Wextra "$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions;-fno-rtti>")
add_compile_definitions(QT_NO_KEYWORDS QT_NO_DYNAMIC_CAST QT_NO_EXCEPTIONS)

# sol2 (header-only C++/Lua bindings), a submodule in sol2/.
add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE sol2/include)

if(PLATFORM_DREAMCAST)
    add_compile_definitions(GYRO_PLATFORM=GYRO_PLATFORM_DC)
else()
    add_compile_definitions(GYRO_PLATFORM=GYRO_PLATFORM_Qt)

    find_package(PkgConfig REQUIRED)
    find_package(Threads REQUIRED)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets Network Help Charts Sql)
    pkg_check_modules(Lua REQUIRED IMPORTED_TARGET lua5.5)

    # Type-check every argument passed from Lua to C++ (scripts get an error, not a crash).
    target_compile_definitions(sol2 INTERFACE SOL_ALL_SAFETIES_ON=1 SOL_NO_CHECK_NUMBER_PRECISION=1)
endif()

add_subdirectory(Spine)
add_subdirectory(libGyro)
add_subdirectory(libGyro-Kinetic)
add_subdirectory(ESToolkit/ESToolkit)

if(NOT PLATFORM_DREAMCAST)
    enable_testing()
    add_subdirectory(tests)
endif()
