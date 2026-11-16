/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_performance_api.h>
#include <libGyro/gyro_platform.h>
#include <SDL3/SDL.h>

uint64_t _gyPerfFreqCached = 0;

uint64_t gyPerfCounter(void) {
    return SDL_GetPerformanceCounter();
}

uint64_t gyPerfFrequency(void) {
    return (_gyPerfFreqCached = SDL_GetPerformanceFrequency());
}

int gyPerfInit(const GYPerfInitParams* params) {
    (void)params;
    gyPerfFrequency(); //fill cache
    return 1;
}

int gyPerfUninit(void) {
    return 1;
}

