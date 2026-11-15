/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#ifndef GYRO_SDL_H
#define GYRO_SDL_H




#ifdef __linux__ // :) Sweet!
#   include <SDL2/SDL.h>
#endif


/*
#ifdef WIN32 Yeah, WHAT?
#   include "sdl.h"
#endif
 */
//#else
//#endif

#define GY_SDL_CLEAR_ERROR() \
    (_clearSDLError(__FILE__, __LINE__, __func__))

#define GY_SDL_CHECK_ERROR() \
    (_checkSDLError(__FILE__, __LINE__, __func__))

static inline int _clearSDLError(const char* file, int line, const char* func) {
    const char* error = SDL_GetError();
    if(strlen(error)) {
        _gyLog(GY_DEBUG_ERROR, "SDL error found before: '%s' [%s:%d (func %s)]", error, file, line, func);
        return 1;
    }
    return 0;
}

static inline int _checkSDLError(const char* file, int line, const char* func) {
    const char* error = SDL_GetError();
    if(strlen(error)) {
        _gyLog(GY_DEBUG_ERROR, "SDL error raised: '%s' [%s:%d (func %s)]", error, file, line, func);
        return 1;
    }
    return 0;
}

#endif // GYRO_SDL_X86_H
