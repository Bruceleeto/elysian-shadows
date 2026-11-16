/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef GYRO_GL_H
#define GYRO_GL_H

#include <libGyro/gyro_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

//GL 4.1 core entry points, loaded by glad (libGyro/external/glad).
//glad's header can't be mixed with Qt's OpenGL headers in one file.
#if GYRO_PLATFORM == GYRO_PLATFORM_Qt
#   include <glad/gl.h>
#endif

#ifdef __cplusplus
}
#endif

#endif // GYRO_GL_H
