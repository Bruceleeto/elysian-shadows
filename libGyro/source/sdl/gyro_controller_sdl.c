/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_platform.h>
#include <libGyro/gyro_system_api.h>
#include <libGyro/gyro_input_api.h>
#include <libGyro/gyro_defines.h>
#include <sdl/gyro_sdl_input.h>
#include <sdl/gyro_sdl.inl>
#include <libGyro/gyro_file_api.h>

#define GYRO_CONTROLLER_SDL_X86_GUID_SIZE    1024

//External symbol defined in SDL_Haptic API!
extern int _gyRumbleSDLTryOpenHapticJoystick(SDL_Joystick* joystick, int port);
extern int _gyRumbleSDLTryCloseHapticJoystick(SDL_Joystick* joystick, int port);

typedef struct SDLControllerProfile {
    char    GUID[GYRO_CONTROLLER_SDL_X86_GUID_SIZE];
} SDLControllerProfile;

typedef struct SDLController {
    GYController            cont;
    SDL_Gamepad*     sdlController;
    SDL_Joystick*           sdlJoystick;
    SDL_JoystickID          sdlJoystickInstanceId;
    int                     port;
    unsigned                al[GY_CONTROLLER_ANALOG_COUNT];
    unsigned                ar[GY_CONTROLLER_ANALOG_COUNT];
    unsigned                ad[GY_CONTROLLER_ANALOG_COUNT];
    unsigned                au[GY_CONTROLLER_ANALOG_COUNT];
} SDLController;

extern GYInputInitParams   _initParams;
static SDLController*      _devices        = NULL;

inline static SDLController* _findControllerByInstanceID(SDL_JoystickID id) {
    for(unsigned i = 0; i < _initParams.controllerCount; ++i) {
        if(_devices[i].sdlJoystickInstanceId == id) return &_devices[i];
    }
    return NULL;
}

inline static SDLController* _claimFreeController(void) {
    for(unsigned i = 0; i < _initParams.controllerCount; ++i) {
        if(!_devices[i].sdlJoystick) return &_devices[i];
    }
    return NULL;
}

static int _sdlCloseController(SDLController* cont);

int _sdlControllerInit(const GYInputInitParams* params) {
    if (_devices) return 1;
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC)) {
        _gyLog(GY_DEBUG_ERROR, "GYRO_INPUT_API: SDL initialization failed: %s", SDL_GetError());
        return 0;
    }
    _devices = calloc(params->controllerCount, sizeof(*_devices));
    if (!_devices && params->controllerCount) {
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
        return 0;
    }
    for (unsigned i = 0; i < params->controllerCount; ++i) _devices[i].port = i;
    const int version = SDL_GetVersion();
    _gyLog(GY_DEBUG_VERBOSE, "SDL compile-time: %d.%d.%d; runtime: %d.%d.%d",
           SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION,
           SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
    return 1;
}

int _sdlControllerUninit(void) {
    if (_devices) {
        for (unsigned i = 0; i < _initParams.controllerCount; ++i)
            if (_devices[i].sdlJoystick) _sdlCloseController(&_devices[i]);
    }
    free(_devices);
    _devices = NULL;
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC);
    return 1;
}

static int _sdlOpenController(SDL_JoystickID id, SDLController* cont) {
    if (SDL_IsGamepad(id)) {
        cont->sdlController = SDL_OpenGamepad(id);
        if (cont->sdlController) cont->sdlJoystick = SDL_GetGamepadJoystick(cont->sdlController);
    } else {
        cont->sdlJoystick = SDL_OpenJoystick(id);
    }
    if (!cont->sdlJoystick) {
        _gyLog(GY_DEBUG_ERROR, "Could not open joystick %u: %s", id, SDL_GetError());
        if (cont->sdlController) SDL_CloseGamepad(cont->sdlController);
        cont->sdlController = NULL;
        return 0;
    }
    cont->sdlJoystickInstanceId = id;
    _gyRumbleSDLTryOpenHapticJoystick(cont->sdlJoystick, cont->port);
    return 1;
}

static int _sdlCloseController(SDLController* cont) {
    _gyRumbleSDLTryCloseHapticJoystick(cont->sdlJoystick, cont->port);
    if (cont->sdlController) SDL_CloseGamepad(cont->sdlController);
    else SDL_CloseJoystick(cont->sdlJoystick);
    cont->sdlController = NULL;
    cont->sdlJoystick = NULL;
    cont->sdlJoystickInstanceId = 0;
    memset(&cont->cont, 0, sizeof(cont->cont));
    return 1;
}

static void _sdlPrintControllerStats(SDLController* cont) {
    char guid[GYRO_CONTROLLER_SDL_X86_GUID_SIZE];
    SDL_GUIDToString(SDL_GetJoystickGUID(cont->sdlJoystick), guid, sizeof(guid));
    const char* name = SDL_GetJoystickName(cont->sdlJoystick);
    _gyLog(GY_DEBUG_VERBOSE, "Port %d: %s [%s], instance %u (%s)", cont->port,
           name ? name : "Unknown", guid, cont->sdlJoystickInstanceId,
           cont->sdlController ? "gamepad" : "joystick");
    if (cont->sdlController) {
        char* mappings = SDL_GetGamepadMapping(cont->sdlController);
        if (mappings) {
            _gyLog(GY_DEBUG_VERBOSE, "Button mappings: %s", mappings);
            SDL_free(mappings);
        } else {
            _gyLog(GY_DEBUG_ERROR, "Could not get gamepad mapping: %s", SDL_GetError());
        }
    }
}

void _sdlControllerDump(void) {
    if (!_devices) return;
    for (unsigned i = 0; i < _initParams.controllerCount; ++i) {
        if (_devices[i].sdlJoystick) _sdlPrintControllerStats(&_devices[i]);
        else _gyLog(GY_DEBUG_VERBOSE, "Port %u: disconnected", i);
    }
}

static void _sdlAttachController(SDL_JoystickID id) {
    if (_findControllerByInstanceID(id)) return;
    SDLController* cont = _claimFreeController();
    if (!cont) {
        _gyLog(GY_DEBUG_WARNING, "No free controller port for joystick %u", id);
        return;
    }
    if (_sdlOpenController(id, cont)) _sdlPrintControllerStats(cont);
}

static void _sdlRemoveController(const SDL_JoystickID instanceID) {
    SDLController*const cont = _findControllerByInstanceID(instanceID);
    if(!cont || !cont->sdlJoystick) return;
    _gyLog(GY_DEBUG_VERBOSE, "SDL Joystick Disconnected! [Port: %d]", cont->port);
    _sdlCloseController(cont);
}

static void _sdlControllerEvent(const SDL_JoystickID instanceID, const SDL_Event* event) {
    SDLController* cont = _findControllerByInstanceID(instanceID);
    if(!cont) return;
    enum GY_CONTROLLER_BUTTON gyButton;
    float analogVal;
    int state;

    switch(event->type) {
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        if(cont->sdlController) return; //process buttons from Controller events, not joystick!!!
        state = event->type == SDL_EVENT_JOYSTICK_BUTTON_DOWN? 1 : 0;

        switch(event->jbutton.button) {
        case 12: gyButton        = GY_CONTROLLER_BUT_DPAD_DOWN;  break;
        case 11: gyButton        = GY_CONTROLLER_BUT_DPAD_UP;    break;
        case 13: gyButton        = GY_CONTROLLER_BUT_DPAD_LEFT;  break;
        case 14: gyButton        = GY_CONTROLLER_BUT_DPAD_RIGHT; break;
        case 8:  gyButton        = GY_CONTROLLER_BUT_START;      break;
        case 9:  gyButton        = GY_CONTROLLER_BUT_SELECT;     break;
        case 10: gyButton        = GY_CONTROLLER_BUT_HOME;       break;
        case 0:  gyButton        = GY_CONTROLLER_BUT_X;          break;
        case 1:  gyButton        = GY_CONTROLLER_BUT_CIRCLE;     break;
        case 2:  gyButton        = GY_CONTROLLER_BUT_SQUARE;     break;
        case 3:  gyButton        = GY_CONTROLLER_BUT_TRIANGLE;   break;
        case 6:  gyButton        = GY_CONTROLLER_BUT_LANALOG;    break;
        case 7:  gyButton        = GY_CONTROLLER_BUT_RANALOG;    break;
        case 4:  gyButton        = GY_CONTROLLER_BUT_LSHOULDER;  break;
        case 5:  gyButton        = GY_CONTROLLER_BUT_RSHOULDER;  break;
        default:
            _gyLog(GY_DEBUG_VERBOSE, "GYRO_INPUT_API: Unidentified key - %d", state);
            return;
        }
        cont->cont.buttonsChanged = 1;
        cont->cont.buttons[gyButton] &= ~GY_BUT_FLAGS_IS_PRESSED;
        cont->cont.buttons[gyButton] |= state;

        break;


    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        state = event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN? 1 : 0;

        switch(event->gbutton.button) {
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:       gyButton        = GY_CONTROLLER_BUT_DPAD_DOWN;  break;
        case SDL_GAMEPAD_BUTTON_DPAD_UP:         gyButton        = GY_CONTROLLER_BUT_DPAD_UP;    break;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:       gyButton        = GY_CONTROLLER_BUT_DPAD_LEFT;  break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:      gyButton        = GY_CONTROLLER_BUT_DPAD_RIGHT; break;
        case SDL_GAMEPAD_BUTTON_START:           gyButton        = GY_CONTROLLER_BUT_START;      break;
        case SDL_GAMEPAD_BUTTON_BACK:            gyButton        = GY_CONTROLLER_BUT_SELECT;     break;
        case SDL_GAMEPAD_BUTTON_GUIDE:           gyButton        = GY_CONTROLLER_BUT_HOME;       break;
        case SDL_GAMEPAD_BUTTON_SOUTH:               gyButton        = GY_CONTROLLER_BUT_X;          break;
        case SDL_GAMEPAD_BUTTON_EAST:               gyButton        = GY_CONTROLLER_BUT_CIRCLE;     break;
        case SDL_GAMEPAD_BUTTON_WEST:               gyButton        = GY_CONTROLLER_BUT_SQUARE;     break;
        case SDL_GAMEPAD_BUTTON_NORTH:               gyButton        = GY_CONTROLLER_BUT_TRIANGLE;   break;
        case SDL_GAMEPAD_BUTTON_LEFT_STICK:       gyButton        = GY_CONTROLLER_BUT_LANALOG;    break;
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK:      gyButton        = GY_CONTROLLER_BUT_RANALOG;    break;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:    gyButton        = GY_CONTROLLER_BUT_LSHOULDER;  break;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:   gyButton        = GY_CONTROLLER_BUT_RSHOULDER;  break;
        default:
            _gyLog(GY_DEBUG_VERBOSE, "GYRO_INPUT_API: Unidentified Controller Button - %d", state);
            return;
        }
        cont->cont.buttonsChanged = 1;
        cont->cont.buttons[gyButton] &= ~GY_BUT_FLAGS_IS_PRESSED;
        cont->cont.buttons[gyButton] |= state;

        //_gyLog(GY_DEBUG_VERBOSE, "But[%d] -> %d", gyButton, cont->cont.buttons[gyButton]);

        break;

    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        const int axis = event->gaxis.axis;
        if (axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER || axis == SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) {
            const int trigger = axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? GY_CONTROLLER_TRIGGER_LEFT : GY_CONTROLLER_TRIGGER_RIGHT;
            cont->cont.triggers[trigger] = (float)event->gaxis.value / INT16_MAX;
            cont->cont.triggersChanged[trigger] = 1;
        } else if (axis >= SDL_GAMEPAD_AXIS_LEFTX && axis <= SDL_GAMEPAD_AXIS_RIGHTY) {
            const int stick = axis <= SDL_GAMEPAD_AXIS_LEFTY ? GY_CONTROLLER_ANALOG_LEFT : GY_CONTROLLER_ANALOG_RIGHT;
            analogVal = event->gaxis.value / (event->gaxis.value < 0 ? 32768.0f : 32767.0f);
            if (axis == SDL_GAMEPAD_AXIS_LEFTX || axis == SDL_GAMEPAD_AXIS_RIGHTX)
                cont->cont.analogSticks[stick].x = analogVal;
            else cont->cont.analogSticks[stick].y = analogVal;
            cont->cont.analogsChanged[stick] = 1;
        }
        break;
    }
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        if (cont->sdlController) return;
        analogVal = (float)event->jaxis.value/(float)INT16_MAX;
        switch(event->jaxis.axis) {
        case 0:
            cont->cont.analogSticks[GY_CONTROLLER_ANALOG_LEFT].x = analogVal;
            cont->cont.analogsChanged[GY_CONTROLLER_ANALOG_LEFT] = 1;
            break;
        case 1:
            cont->cont.analogSticks[GY_CONTROLLER_ANALOG_LEFT].y    = analogVal;
            cont->cont.analogsChanged[GY_CONTROLLER_ANALOG_LEFT] = 1;
            break;
        case 2: {
            float trigVal = 1.0f - (float)(event->jaxis.value - INT16_MIN)/(float)(INT16_MAX - INT16_MIN);
            cont->cont.triggers[GY_CONTROLLER_TRIGGER_LEFT]         = trigVal;
            cont->cont.triggersChanged[GY_CONTROLLER_TRIGGER_LEFT]  = 1;
            break;
        }
        case 3:
            cont->cont.analogSticks[GY_CONTROLLER_ANALOG_RIGHT].x   = analogVal;
            cont->cont.analogsChanged[GY_CONTROLLER_ANALOG_RIGHT]   = 1;
            break;
        case 4:
            cont->cont.analogSticks[GY_CONTROLLER_ANALOG_RIGHT].y   = analogVal;
            cont->cont.analogsChanged[GY_CONTROLLER_ANALOG_RIGHT] = 1;
            break;
        case 5: {
            float trigVal = 1.0f - (float)(event->jaxis.value - INT16_MIN)/(float)(INT16_MAX - INT16_MIN);
            cont->cont.triggers[GY_CONTROLLER_TRIGGER_RIGHT]         = trigVal;
            cont->cont.triggersChanged[GY_CONTROLLER_TRIGGER_RIGHT]  = 1;
            break;
        }
        default: break;
        }
        break;

    default: break;
    }

}

static void _sdlPollEvents(void) {
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            _sdlControllerEvent(event.jaxis.which, &event);
            break;
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            _sdlControllerEvent(event.gaxis.which, &event);
            break;
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            _sdlControllerEvent(event.gbutton.which, &event);
            break;
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            _sdlControllerEvent(event.jbutton.which, &event);
            break;
        case SDL_EVENT_JOYSTICK_ADDED:
            _sdlAttachController(event.jdevice.which);
            break;
        case SDL_EVENT_GAMEPAD_ADDED:
          //  _sdlAttachController(event.cdevice.which);
            break;
        case SDL_EVENT_JOYSTICK_REMOVED:

            _sdlRemoveController(event.jdevice.which);
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
          //  _sdlRemoveController(event.cdevice.which);
            break;
        }
    }

}

int gyInputPollEvents(void) {
    if (!_devices) return 0;
    for(unsigned slot = 0; slot < _initParams.controllerCount; ++slot) {
        GYVector2 analog = { 0.0f, 0.0f };
        memset(_devices[slot].al, 0, sizeof(unsigned)*GY_CONTROLLER_ANALOG_COUNT);
        memset(_devices[slot].ar, 0, sizeof(unsigned)*GY_CONTROLLER_ANALOG_COUNT);
        memset(_devices[slot].ad, 0, sizeof(unsigned)*GY_CONTROLLER_ANALOG_COUNT);
        memset(_devices[slot].au, 0, sizeof(unsigned)*GY_CONTROLLER_ANALOG_COUNT);
        _devices[slot].cont.buttonsChanged = 0;
        for(int a = 0; a < GY_CONTROLLER_ANALOG_COUNT; ++a)
            _devices[slot].cont.analogsChanged[a] = 0;
        for(int t = 0; t < GY_CONTROLLER_TRIGGER_COUNT; ++t)
            _devices[slot].cont.triggersChanged[t] = 0;

        if(!_devices[slot].sdlJoystick) {
            _devices[slot].cont.analogSticks[GY_CONTROLLER_ANALOG_LEFT]     = analog;
            _devices[slot].cont.analogSticks[GY_CONTROLLER_ANALOG_RIGHT]    = analog;
        } else {

            for(unsigned a = 0; a < GY_CONTROLLER_ANALOG_COUNT; ++a) {
                if(_devices[slot].cont.analogSticks[a].x < -GYRO_INPUT_CONTROLLER_ANALOG_MIN)     _devices[slot].al[a] = 1;
                else if(_devices[slot].cont.analogSticks[a].x > GYRO_INPUT_CONTROLLER_ANALOG_MIN) _devices[slot].ar[a] = 1;
                if(_devices[slot].cont.analogSticks[a].y < -GYRO_INPUT_CONTROLLER_ANALOG_MIN)     _devices[slot].au[a] = 1;
                else if(_devices[slot].cont.analogSticks[a].y > GYRO_INPUT_CONTROLLER_ANALOG_MIN) _devices[slot].ad[a] = 1;

            }
        }

        for(unsigned i = 0; i < GY_CONTROLLER_BUT_COUNT; ++i) {
            int wasDown = (_devices[slot].cont.buttons[i] & GY_BUT_FLAGS_IS_PRESSED);
            _devices[slot].cont.buttons[i] <<= 1;
            _devices[slot].cont.buttons[i] |= wasDown;
            _devices[slot].cont.buttons[i] &= 0x03;
        }
    }

    _sdlPollEvents();
    return 1;
}

int gyControllerPoll(const unsigned slot, struct GYController** const input) {
    if (!input) return 0;
    *input = NULL;
    if (!_devices || slot >= _initParams.controllerCount) return 0;

    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_LEFT]     &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_LEFT]     |= _devices[slot].al[GY_CONTROLLER_ANALOG_LEFT];
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_RIGHT]    &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_RIGHT]    |= _devices[slot].ar[GY_CONTROLLER_ANALOG_LEFT];
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_UP]       &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_UP]       |= _devices[slot].au[GY_CONTROLLER_ANALOG_LEFT];
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_DOWN]     &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG1_DOWN]     |= _devices[slot].ad[GY_CONTROLLER_ANALOG_LEFT];

    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_LEFT]     &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_LEFT]     |= _devices[slot].al[GY_CONTROLLER_ANALOG_RIGHT];
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_RIGHT]    &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_RIGHT]    |= _devices[slot].ar[GY_CONTROLLER_ANALOG_RIGHT];
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_UP]       &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_UP]       |= _devices[slot].au[GY_CONTROLLER_ANALOG_RIGHT];
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_DOWN]     &= ~GY_BUT_FLAGS_IS_PRESSED;
    _devices[slot].cont.buttons[GY_CONTROLLER_BUT_ANALOG2_DOWN]     |= _devices[slot].ad[GY_CONTROLLER_ANALOG_RIGHT];

    *input = &_devices[slot].cont;

    return _devices[slot].sdlJoystick? 1 : 0;
}

int gyControllerLoadMappingsFile(const char* filepath) {
    int     success         = 1;
    char*   resolvedPath    = NULL;
    size_t  pathLen         = 0;
    int     mappingsCount   = 0;

    assert(filepath);

    _gyLog(GY_DEBUG_VERBOSE, "GYRO_CONTROLLER_API: Importing SDL Controller Button Mappings. [%s]", filepath);
    _gyPush();

    if(!gyFileResolvePath(GY_PATH_ASSETS, filepath, &resolvedPath, &pathLen)) {
        _gyLog(GY_DEBUG_ERROR, "Failed to resolve asset's actual path!");
        success = 0;
    } else {

        if((mappingsCount = SDL_AddGamepadMappingsFromFile(resolvedPath)) < 0) {
            _gyLog(GY_DEBUG_ERROR, "Could not load gamepad mappings: %s", SDL_GetError());
            success = 0;
        } else {
            _gyLog(GY_DEBUG_VERBOSE, "%d mappings were added successfully.", mappingsCount);
            success = 1;
        }

    }

    free(resolvedPath);
    _gyPop(1);
    return success;
}
