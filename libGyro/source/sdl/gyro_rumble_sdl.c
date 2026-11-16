/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_rumble_api.h>
#include <sdl/gyro_sdl_input.h>
#include <libGyro/gyro_system_api.h>
#include <sdl/gyro_sdl.inl>
#include <assert.h>

// Joystick and gamepad pointers are borrowed from the controller backend.
typedef struct SDLHapticDevice {
    GYRumble _gyRumble;
    SDL_Haptic* _haptic;
    SDL_Joystick* _joystick;
    SDL_Gamepad* _gamepad;
    int _port;
    int _curEffect;
    bool _rumbleReady;
} SDLHapticDevice;

static SDLHapticDevice* _devices;
static unsigned _deviceCapacity;
static bool _initialized;

static int _rumbleResult(bool result, const char* operation) {
    if (!result) _gyLog(GY_DEBUG_ERROR, "GYRO_RUMBLE: %s failed: %s", operation, SDL_GetError());
    return result;
}

static SDLHapticDevice* _getSDLHapticDeviceByGYRumble(const GYRumble* rumble) {
    for (unsigned i = 0; i < _deviceCapacity; ++i)
        if (&_devices[i]._gyRumble == rumble) return &_devices[i];
    return NULL;
}

static void _releaseDevice(SDLHapticDevice* device) {
    if (device->_haptic) SDL_CloseHaptic(device->_haptic);
    else if (device->_gamepad) _rumbleResult(SDL_RumbleGamepad(device->_gamepad, 0, 0, 0), "Stop gamepad rumble");
    else if (device->_joystick) _rumbleResult(SDL_RumbleJoystick(device->_joystick, 0, 0, 0), "Stop joystick rumble");
    memset(device, 0, sizeof(*device));
    device->_port = -1;
    device->_curEffect = -1;
}

int _gyRumbleSDLTryOpenHapticJoystick(SDL_Joystick* joystick, int port) {
    if (!joystick || port < 0) return 1;
    SDLHapticDevice* device = NULL;
    for (unsigned i = 0; i < _deviceCapacity; ++i) {
        if (_devices[i]._port == port) return 1;
        if (!device && _devices[i]._port < 0) device = &_devices[i];
    }
    if (!device) return 1;
    if (SDL_IsJoystickHaptic(joystick)) {
        device->_haptic = SDL_OpenHapticFromJoystick(joystick);
        if (!device->_haptic) {
            _gyLog(GY_DEBUG_ERROR, "Could not open haptic device: %s", SDL_GetError());
            return 0;
        }
        if (SDL_HapticRumbleSupported(device->_haptic)) {
            device->_rumbleReady = SDL_InitHapticRumble(device->_haptic);
            if (!device->_rumbleReady) {
                _rumbleResult(false, "Initialize haptic rumble");
                _releaseDevice(device);
                return 0;
            }
        }
    } else {
        SDL_PropertiesID props = SDL_GetJoystickProperties(joystick);
        if (!props) return _rumbleResult(false, "Get joystick properties");
        if (!SDL_GetBooleanProperty(props, SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN, false)) return 1;
        SDL_JoystickID id = SDL_GetJoystickID(joystick);
        if (SDL_IsGamepad(id)) device->_gamepad = SDL_GetGamepadFromID(id);
    }
    device->_joystick = joystick;
    device->_port = port;
    if (!gyRumbleDeviceQuery(&device->_gyRumble)) {
        _releaseDevice(device);
        return 0;
    }
    return 1;
}

int _gyRumbleSDLTryCloseHapticJoystick(SDL_Joystick* joystick, int port) {
    (void)joystick;
    for (unsigned i = 0; i < _deviceCapacity; ++i)
        if (_devices[i]._port == port) _releaseDevice(&_devices[i]);
    return 1;
}

int gyRumbleDeviceQuery(GYRumble* dev) {
    SDLHapticDevice* device = _getSDLHapticDeviceByGYRumble(dev);
    if (!device || !device->_joystick) return 0;
    dev->featureFlags = device->_haptic ? SDL_GetHapticFeatures(device->_haptic) : SDL_HAPTIC_LEFTRIGHT;
    if (!dev->featureFlags) return _rumbleResult(false, "Get haptic features");
    return 1;
}

int gyRumbleInit(const GYRumbleInitParams* params) {
    if (_initialized) return 1;
    if (!params) return 0;
    if (!_rumbleResult(SDL_InitSubSystem(SDL_INIT_HAPTIC), "Initialize haptics")) return 0;
    _devices = calloc(params->maxDevices, sizeof(*_devices));
    if (!_devices && params->maxDevices) {
        SDL_QuitSubSystem(SDL_INIT_HAPTIC);
        return 0;
    }
    _deviceCapacity = params->maxDevices;
    for (unsigned i = 0; i < _deviceCapacity; ++i) {
        _devices[i]._port = -1;
        _devices[i]._curEffect = -1;
    }
    _initialized = true;
    return 1;
}

int gyRumbleUninit(void) {
    if (!_initialized) return 1;
    for (unsigned i = 0; i < _deviceCapacity; ++i) _releaseDevice(&_devices[i]);
    free(_devices);
    _devices = NULL;
    _deviceCapacity = 0;
    _initialized = false;
    SDL_QuitSubSystem(SDL_INIT_HAPTIC);
    return 1;
}

int gyRumblePoll(unsigned slot, GYRumble** handle) {
    if (!handle) return 0;
    *handle = NULL;
    if (slot >= _deviceCapacity) return 0;
    *handle = &_devices[slot]._gyRumble;
    return _devices[slot]._port >= 0;
}

static Uint16 _rumbleMagnitude(float intensity) {
    if (!(intensity > 0.0f)) return 0;
    if (intensity >= 1.0f) return UINT16_MAX;
    return intensity * UINT16_MAX;
}

static int _playMotors(SDLHapticDevice* device, float large, float small, uint32_t duration) {
    const Uint16 low = _rumbleMagnitude(large), high = _rumbleMagnitude(small);
    if (device->_gamepad)
        return _rumbleResult(SDL_RumbleGamepad(device->_gamepad, low, high, duration), "Gamepad rumble");
    return _rumbleResult(SDL_RumbleJoystick(device->_joystick, low, high, duration), "Joystick rumble");
}

int gyRumblePlay(GYRumble* dev, float intensity, uint32_t duration) {
    SDLHapticDevice* device = _getSDLHapticDeviceByGYRumble(dev);
    if (!device || !device->_joystick) return 0;
    if (!device->_haptic) return _playMotors(device, intensity, intensity, duration);
    if (!device->_rumbleReady) return 0;
    return _rumbleResult(SDL_PlayHapticRumble(device->_haptic, intensity, duration), "Haptic rumble");
}

int gyRumbleStop(GYRumble* dev) {
    SDLHapticDevice* device = _getSDLHapticDeviceByGYRumble(dev);
    if (!device || !device->_joystick) return 0;
    if (!device->_haptic) return _playMotors(device, 0, 0, 0);
    return _rumbleResult(SDL_StopHapticEffects(device->_haptic), "Stop haptic effects");
}

static uint32_t _effectMilliseconds(float seconds) {
    if (seconds == GY_RUMBLE_EFFECT_DURATION_INFINITE) return SDL_HAPTIC_INFINITY;
    if (!(seconds > 0.0f)) return 0;
    double ms = seconds * 1000.0;
    return ms >= UINT32_MAX ? UINT32_MAX : (uint32_t)ms;
}

inline static int _gyPeriodicTypeToSDLType(int type) {
    switch(type) {
    case GY_RUMBLE_EFFECT_PERIODIC_SINE:            return SDL_HAPTIC_SINE;
    case GY_RUMBLE_EFFECT_PERIODIC_TRIANGLE:        return SDL_HAPTIC_TRIANGLE;
    case GY_RUMBLE_EFFECT_PERIODIC_SAWTOOTHUP:      return SDL_HAPTIC_SAWTOOTHUP;
    case GY_RUMBLE_EFFECT_PERIODIC_SAWTOOTHDOWN:    return SDL_HAPTIC_SAWTOOTHDOWN;
    default:                                        return 0;
    }
}

inline static const char* _gyRumblePeriodicTypeStr(int type) {
    switch(type) {
    case SDL_HAPTIC_SINE:           return "SINE";
    case SDL_HAPTIC_TRIANGLE:       return "TRIANGLE";
    case SDL_HAPTIC_SAWTOOTHDOWN:   return "SAWTOOTHDOWN";
    case SDL_HAPTIC_SAWTOOTHUP:     return "SAWTOOTHUP";
    default:                        return "DIRECT";
    }
}

inline static int _gyRumbleEffectPeriodicSupportPriority(int flags, int mask1, int mask2, int mask3) {
    if(flags & mask1) return mask1;
    else if(flags & mask2) return mask2;
    else if(flags & mask3) return mask3;
    else return 0;
}


int gyRumblePlayEffect(GYRumble *dev, const GYRumbleEffect *effect, uint32_t iterations) {
    assert(effect);
    if(!dev) return 0;

    SDLHapticDevice* sdlDev = _getSDLHapticDeviceByGYRumble(dev);
    if(!sdlDev || !sdlDev->_joystick) return 0;

    if (!iterations) return 1;
    const uint32_t duration = _effectMilliseconds(effect->duration);
    const uint64_t total = (uint64_t)duration * iterations;
    const uint32_t fallbackDuration = duration == SDL_HAPTIC_INFINITY || total >= UINT32_MAX ? UINT32_MAX : (uint32_t)total;
    float fallbackIntensity = 1.0f;
    SDL_HapticEffect sdlEffect = {0};
    switch(effect->effectType) {
    case GY_RUMBLE_EFFECT_LEFTRIGHT:
        if (!sdlDev->_haptic) return _playMotors(sdlDev, effect->leftRight.largeIntensity, effect->leftRight.smallIntensity, fallbackDuration);
        fallbackIntensity = (effect->leftRight.largeIntensity > 0)? effect->leftRight.largeIntensity : effect->leftRight.smallIntensity;
        if(!(dev->featureFlags & SDL_HAPTIC_LEFTRIGHT)) {
            _gyLog(GY_DEBUG_WARNING, "[GYRO_RUMBLE] RumbleDevice does not support LEFTRIGHT effects! (Falling through to direct playback)");
            return gyRumblePlay(dev, fallbackIntensity, fallbackDuration);
        }
        sdlEffect.leftright.type            = SDL_HAPTIC_LEFTRIGHT;
        sdlEffect.leftright.length          = duration;
        sdlEffect.leftright.large_magnitude = effect->leftRight.largeIntensity * UINT16_MAX;
        sdlEffect.leftright.small_magnitude = effect->leftRight.smallIntensity * UINT16_MAX;

        break;

    case GY_RUMBLE_EFFECT_CONSTANT:
        if(!(dev->featureFlags & SDL_HAPTIC_CONSTANT)) {
            fallbackIntensity = effect->constant.intensity;
            _gyLog(GY_DEBUG_WARNING, "[GYRO_RUMBLE] RumbleDevice does not support CONSTANT effects! (Falling through to direct playback)");
            return gyRumblePlay(dev, fallbackIntensity, fallbackDuration);
        }

        sdlEffect.constant.type             = SDL_HAPTIC_CONSTANT;
        sdlEffect.constant.length           = duration;
        sdlEffect.constant.attack_level     = effect->constant.startIntensity  * INT16_MAX;
        sdlEffect.constant.fade_level       = effect->constant.endIntensity    * INT16_MAX;
        sdlEffect.constant.attack_length    = effect->constant.fadeInDuration  * 1000.0f;
        sdlEffect.constant.fade_length      = effect->constant.fadeOutDuration * 1000.0f;
        sdlEffect.constant.delay            = effect->constant.delay            * 1000.0f;
        sdlEffect.constant.direction.type   = SDL_HAPTIC_CARTESIAN;
        sdlEffect.constant.direction.dir[0] = effect->constant.direction.x * ((effect->constant.direction.x < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.constant.direction.dir[1] = effect->constant.direction.y * ((effect->constant.direction.y < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.constant.direction.dir[2] = effect->constant.direction.z * ((effect->constant.direction.z < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.constant.level            = effect->constant.intensity * ((effect->constant.intensity < 0)? -INT16_MIN : INT16_MAX);
        break;

    case GY_RUMBLE_EFFECT_RAMP:
        if(!(dev->featureFlags & SDL_HAPTIC_RAMP)) {
            fallbackIntensity = (effect->ramp.startIntensity + effect->ramp.endIntensity)*0.5f;
            _gyLog(GY_DEBUG_WARNING, "[GYRO_RUMBLE] RumbleDevice does not support RAMP effects! (Falling through to direct playback)");
            return gyRumblePlay(dev, fallbackIntensity, fallbackDuration);
        }

        sdlEffect.ramp.type             = SDL_HAPTIC_RAMP;
        sdlEffect.ramp.length           = duration;
        sdlEffect.ramp.attack_level     = effect->ramp.startIntensity  * INT16_MAX;
        sdlEffect.ramp.fade_level       = effect->ramp.endIntensity    * INT16_MAX;
        sdlEffect.ramp.attack_length    = effect->ramp.fadeInDuration  * 1000.0f;
        sdlEffect.ramp.fade_length      = effect->ramp.fadeOutDuration * 1000.0f;
        sdlEffect.ramp.delay            = effect->ramp.delay            * 1000.0f;
        sdlEffect.ramp.direction.type   = SDL_HAPTIC_CARTESIAN;
        sdlEffect.ramp.direction.dir[0] = effect->ramp.direction.x * ((effect->ramp.direction.x < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.ramp.direction.dir[1] = effect->ramp.direction.y * ((effect->ramp.direction.y < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.ramp.direction.dir[2] = effect->ramp.direction.z * ((effect->ramp.direction.z < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.ramp.start            = effect->ramp.rampStartIntensity * ((effect->ramp.rampStartIntensity < 0)? -INT16_MIN : INT16_MAX);
        sdlEffect.ramp.end              = effect->ramp.rampEndIntensity * ((effect->ramp.rampEndIntensity < 0)? -INT16_MIN : INT16_MAX);
        break;


    case GY_RUMBLE_EFFECT_PERIODIC: {
        fallbackIntensity = effect->periodic.intensityPeak;
        int actualType = _gyPeriodicTypeToSDLType(effect->periodic.periodicType);

        switch(actualType) {
        case SDL_HAPTIC_SINE:
            if(!(dev->featureFlags & SDL_HAPTIC_SINE)) {
                actualType = _gyRumbleEffectPeriodicSupportPriority(dev->featureFlags, SDL_HAPTIC_TRIANGLE, SDL_HAPTIC_SAWTOOTHUP, SDL_HAPTIC_SAWTOOTHDOWN);
            }
            break;
        case SDL_HAPTIC_TRIANGLE:
            if(!(dev->featureFlags & SDL_HAPTIC_TRIANGLE)) {
                actualType = _gyRumbleEffectPeriodicSupportPriority(dev->featureFlags, SDL_HAPTIC_SINE, SDL_HAPTIC_SAWTOOTHUP, SDL_HAPTIC_SAWTOOTHDOWN);
            }
            break;
        case SDL_HAPTIC_SAWTOOTHUP:
            if(!(dev->featureFlags & SDL_HAPTIC_SAWTOOTHUP)) {
                actualType = _gyRumbleEffectPeriodicSupportPriority(dev->featureFlags, SDL_HAPTIC_SAWTOOTHDOWN, SDL_HAPTIC_TRIANGLE, SDL_HAPTIC_SINE);
            }
            break;
        case SDL_HAPTIC_SAWTOOTHDOWN:
            if(!(dev->featureFlags & SDL_HAPTIC_SAWTOOTHDOWN)) {
                actualType = _gyRumbleEffectPeriodicSupportPriority(dev->featureFlags, SDL_HAPTIC_SAWTOOTHUP, SDL_HAPTIC_TRIANGLE, SDL_HAPTIC_SINE);
            }
            break;
        }
        int sdlEffectType = _gyPeriodicTypeToSDLType(effect->periodic.periodicType);

        if(actualType != sdlEffectType) {
            _gyLog(GY_DEBUG_WARNING, "[GYRO_RUMBLE] Periodic type %s not supported on device. Falling back to %s.", _gyRumblePeriodicTypeStr(sdlEffectType), _gyRumblePeriodicTypeStr(actualType));

            if(!actualType) {
                return gyRumblePlay(dev, fallbackIntensity, fallbackDuration);
            }
        }

        sdlEffect.periodic.type             = actualType;
        sdlEffect.periodic.length           = duration;
        sdlEffect.periodic.attack_level     = effect->periodic.startIntensity  * INT16_MAX;
        sdlEffect.periodic.fade_level       = effect->periodic.endIntensity    * INT16_MAX;
        sdlEffect.periodic.attack_length    = effect->periodic.fadeInDuration  * 1000.0f;
        sdlEffect.periodic.fade_length      = effect->periodic.fadeOutDuration * 1000.0f;
        sdlEffect.periodic.delay            = effect->periodic.delay            * 1000.0f;
        sdlEffect.periodic.direction.type   = SDL_HAPTIC_CARTESIAN;
        sdlEffect.periodic.direction.dir[0] = effect->periodic.direction.x * ((effect->periodic.direction.x < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.periodic.direction.dir[1] = effect->periodic.direction.y * ((effect->periodic.direction.y < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.periodic.direction.dir[2] = effect->periodic.direction.z * ((effect->periodic.direction.z < 0)? -INT8_MIN : INT8_MAX);
        sdlEffect.periodic.period           = effect->periodic.period * 1000.0f;
        sdlEffect.periodic.magnitude        = effect->periodic.intensityPeak   * ((effect->periodic.intensityPeak < 0)? -INT16_MIN : INT16_MAX);
        sdlEffect.periodic.offset           = effect->periodic.intensityOffset * ((effect->periodic.intensityOffset < 0)? -INT16_MIN : INT16_MAX);
        sdlEffect.periodic.phase            = effect->periodic.phaseShift;
        break;
    }
    case GY_RUMBLE_EFFECT_NONE:
        _gyLog(GY_DEBUG_ERROR, "[GYRO_RUMBLE] Tried to play a RumbleEffect of type NONE!");
        return 0;
    default: return 0;
    }

    if(sdlDev->_curEffect >= 0) {
        SDL_DestroyHapticEffect(sdlDev->_haptic, sdlDev->_curEffect);
    }

    sdlDev->_curEffect = SDL_CreateHapticEffect(sdlDev->_haptic, &sdlEffect);
    if(sdlDev->_curEffect < 0) {
        _rumbleResult(false, "Haptic effect");
        gyRumblePlay(dev, fallbackIntensity, fallbackDuration);
        return 0;
    }

    if(!SDL_RunHapticEffect(sdlDev->_haptic, sdlDev->_curEffect, iterations)) {
        _rumbleResult(false, "Haptic effect");
        gyRumblePlay(dev, fallbackIntensity, fallbackDuration);
        return 0;
    }

    return 1;
}


