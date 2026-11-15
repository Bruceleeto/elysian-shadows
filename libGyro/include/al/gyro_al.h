/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef GYRO_AL_H
#define GYRO_AL_H

#include <libGyro/gyro_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

//this is no good
#if GYRO_PLATFORM == GYRO_PLATFORM_x86
#       define AL_LIBTYPE_STATIC
#		include <AL/al.h>
#       include <AL/alc.h>
#elif GYRO_PLATFORM == GYRO_PLATFORM_Qt

#       define AL_LIBTYPE_STATIC
#		include <AL/al.h>
#       include <AL/alc.h>
#endif

extern ALenum              _alFmtStereo32;
extern ALenum              _alFmtMono32;
extern ALboolean           _alExtFloat32;

#ifdef __cplusplus
}
#endif


#endif // GYRO_AL_H
