/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <gyro_performance_api.h>
#if   defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h
#else
#include <SDL.h>
#include <SDL_timer.h>
#endif

int gyTimerInit(void) {
    return 1;
}

int gyTimerUninit(void) {
    return 1;
}

int64_t gyTimerTicks(void) {
    return SDL_GetTicks();
}
