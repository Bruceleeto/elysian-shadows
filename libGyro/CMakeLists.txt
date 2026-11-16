# libGyro: the platform layer (video, audio, input, files, timers).

add_library(libGyro STATIC
    source/common/turfs_common.c
    source/common/gyro_file.c
    source/common/gyro_asset_bank.c
    source/common/gyro_matrix_stack.c
    source/common/gyro_shader.c
    source/common/gyro_system.c
    source/common/gyro_texture.c
    source/common/gyro_wav.c
    source/common/turfs_file_stdio.c
    source/common/gyro_video.c
    source/common/gyro_timer.c
    source/common/gyro_test.c
    source/common/gyro_visual_memory_common.c
    source/x86/turfs_unix.c)

if(PLATFORM_DREAMCAST)
    target_sources(libGyro PRIVATE
        source/dc/gyro_audio_dc.c
        source/dc/gyro_input_dc.c
        source/dc/gyro_system_dc.c
        source/dc/gyro_texture_dc.c
        source/dc/gyro_timer_dc.c
        source/dc/gyro_video_dc.c
        source/dc/gyro_wav_dc.c
        source/dc/gyro_file_dc.c)
    target_include_directories(libGyro PRIVATE lib/dc/shlib)
else()
    target_sources(libGyro PRIVATE
        source/gl/gyro_radial_blur_gl.c
        source/gl/gyro_chromatic_aberration_gl.c
        source/sdl/gyro_performance_sdl.c
        source/sdl/gyro_rumble_sdl.c
        source/gl/gyro_bloom_gl.c
        source/gl/gyro_gl_utils.c
        source/sdl/gyro_thread_sdl.c
        source/qt/gyro_sensor_qt.cpp
        source/gl/gyro_lighting_gl.c
        source/gl/gyro_shaders_gl.c
        source/gl/gyro_video_gl.c
        source/al/gyro_audio_al.c
        source/al/gyro_wav_al.c
        source/al/gyro_ogg_al.c
        source/x86/gyro_matrix_x86.c
        source/x86/gyro_system_glfw_x86.c
        source/qt/gyro_texture_qt.cpp
        source/qt/gyro_debug_qt.cpp
        source/qt/gyro_input_qt.cpp
        source/qt/gyro_timer_qt.cpp
        source/qt/gyro_file_qt.cpp
        source/qt/gyro_video_qt.cpp
        source/sdl/gyro_controller_sdl.c
        source/posix/gyro_net_posix.c
        external/glad/src/gl.c)
    target_include_directories(libGyro PUBLIC external/glad/include)

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(libGyro PRIVATE source/linux/gyro_system_linux.c)
    endif()

    pkg_check_modules(GYRO_DEPS REQUIRED IMPORTED_TARGET
        sdl3 openal zlib ogg vorbis vorbisenc vorbisfile)
    target_link_libraries(libGyro PUBLIC PkgConfig::GYRO_DEPS Threads::Threads
        Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
endif()

target_include_directories(libGyro PUBLIC include api)
target_link_libraries(libGyro PUBLIC Spine)
