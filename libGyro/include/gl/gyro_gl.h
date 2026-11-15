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

//this is no good
#if GYRO_PLATFORM == GYRO_PLATFORM_x86
#elif GYRO_PLATFORM == GYRO_PLATFORM_Qt

//#       include <windows.h>
//#if !defined(__gl_h_) && !defined(__GL_H__) && !defined(__X_GL_H)

#       include <GL/glew.h>
#define QT_NO_OPENGL_ES_2
//#       include <GL/gl.h>
//#       include <GL/glu.h>
//#endif
#      // define QOPENGL_H //THIS WILL REEST GLEW OTHERWISE!!!
#endif

#ifdef __cplusplus
}
#endif

#endif // GYRO_GL_H
