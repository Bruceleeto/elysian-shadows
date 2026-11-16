#include <SDL3/SDL.h>
#include <libGyro/gyro_rumble_api.h>
#include <libGyro/gyro_system_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); abort(); } } while (0)

/* Capture the effects passed to SDL without requiring force-feedback hardware. */
static SDL_HapticEffect captured;
static Uint32 playedIterations, features;
static unsigned created, destroyed, stopped, closed, directPlays, errors;
static Uint32 directDuration;
static bool failCreate, failRun;
static int hapticToken, joystickToken;
extern int _gyRumbleSDLTryOpenHapticJoystick(SDL_Joystick*, int);
extern int _gyRumbleSDLTryCloseHapticJoystick(SDL_Joystick*, int);

void _gyLog(GY_DEBUG_SEVERITY severity, const char* const format, ...) {
    (void)format;
    if (severity == GY_DEBUG_ERROR) ++errors;
}
void _gyPush(void) {}
void _gyPop(const unsigned count) { (void)count; }

bool __wrap_SDL_IsJoystickHaptic(SDL_Joystick* joystick) {
    CHECK(joystick == (SDL_Joystick*)&joystickToken);
    return true;
}
SDL_Haptic* __wrap_SDL_OpenHapticFromJoystick(SDL_Joystick* joystick) {
    CHECK(joystick == (SDL_Joystick*)&joystickToken);
    return (SDL_Haptic*)&hapticToken;
}
Uint32 __wrap_SDL_GetHapticFeatures(SDL_Haptic* haptic) {
    CHECK(haptic == (SDL_Haptic*)&hapticToken);
    return features;
}
bool __wrap_SDL_HapticRumbleSupported(SDL_Haptic* haptic) { (void)haptic; return true; }
bool __wrap_SDL_InitHapticRumble(SDL_Haptic* haptic) { (void)haptic; return true; }
void __wrap_SDL_CloseHaptic(SDL_Haptic* haptic) { (void)haptic; ++closed; }
int __wrap_SDL_CreateHapticEffect(SDL_Haptic* haptic, const SDL_HapticEffect* effect) {
    (void)haptic;
    if (failCreate) { SDL_SetError("intentional effect creation failure"); return -1; }
    captured = *effect;
    return ++created;
}
void __wrap_SDL_DestroyHapticEffect(SDL_Haptic* haptic, int effect) {
    (void)haptic;
    CHECK(effect > 0);
    ++destroyed;
}
bool __wrap_SDL_RunHapticEffect(SDL_Haptic* haptic, int effect, Uint32 iterations) {
    (void)haptic;
    CHECK(effect > 0);
    if (failRun) { SDL_SetError("intentional effect playback failure"); return false; }
    playedIterations = iterations;
    return true;
}
bool __wrap_SDL_StopHapticEffects(SDL_Haptic* haptic) { (void)haptic; ++stopped; return true; }
bool __wrap_SDL_PlayHapticRumble(SDL_Haptic* haptic, float strength, Uint32 duration) {
    (void)haptic;
    CHECK(strength >= 0 && strength <= 1);
    ++directPlays;
    directDuration = duration;
    return true;
}

int main(void) {
    const GYRumbleInitParams params = {.maxDevices = 1};
    CHECK(gyRumbleInit(&params));
    features = SDL_HAPTIC_CONSTANT | SDL_HAPTIC_RAMP | SDL_HAPTIC_SINE | SDL_HAPTIC_TRIANGLE | SDL_HAPTIC_LEFTRIGHT;
    SDL_Joystick* joystick = (SDL_Joystick*)&joystickToken;
    CHECK(_gyRumbleSDLTryOpenHapticJoystick(joystick, 0));
    GYRumble* handle;
    CHECK(gyRumblePoll(0, &handle));
    GYRumbleEffect effect = {.effectType = GY_RUMBLE_EFFECT_CONSTANT, .duration = 0.25f,
        .constant = {.intensity = 0.5f, .direction = {1, 0, 0}, .delay = 0.125f,
                     .startIntensity = 0.25f, .endIntensity = 0.75f,
                     .fadeInDuration = 0.0625f, .fadeOutDuration = 0.125f}};
    SDL_SetError("stale sentinel");
    CHECK(gyRumblePlayEffect(handle, &effect, 3));
    CHECK(captured.type == SDL_HAPTIC_CONSTANT && captured.constant.length == 250);
    CHECK(playedIterations == 3);
    CHECK(captured.constant.delay == 125 && captured.constant.level == 16383);
    CHECK(captured.constant.attack_length == 62 && captured.constant.fade_length == 125);
    CHECK(captured.constant.attack_level == 8191 && captured.constant.fade_level == 24575);
    CHECK(captured.constant.button == 0 && captured.constant.interval == 0);
    CHECK(errors == 0);

    memset(&effect, 0, sizeof(effect));
    effect.effectType = GY_RUMBLE_EFFECT_RAMP;
    effect.duration = 0.5f;
    effect.ramp.rampStartIntensity = -1;
    effect.ramp.rampEndIntensity = 1;
    effect.ramp.fadeInDuration = 0.125f;
    CHECK(gyRumblePlayEffect(handle, &effect, 1));
    CHECK(captured.type == SDL_HAPTIC_RAMP);
    CHECK(captured.ramp.start == -32768 && captured.ramp.end == 32767);
    CHECK(captured.ramp.attack_length == 125 && destroyed == 1);

    memset(&effect, 0, sizeof(effect));
    effect.effectType = GY_RUMBLE_EFFECT_PERIODIC;
    effect.duration = 0.125f;
    effect.periodic.periodicType = GY_RUMBLE_EFFECT_PERIODIC_SINE;
    effect.periodic.intensityPeak = 0.5f;
    effect.periodic.period = 0.0625f;
    effect.periodic.phaseShift = 9000;
    effect.periodic.fadeInDuration = 0.03125f;
    features = SDL_HAPTIC_TRIANGLE;
    CHECK(gyRumbleDeviceQuery(handle));
    CHECK(gyRumblePlayEffect(handle, &effect, 2));
    CHECK(captured.type == SDL_HAPTIC_TRIANGLE && captured.periodic.period == 62);
    CHECK(captured.periodic.attack_length == 31 && playedIterations == 2);
    CHECK(captured.periodic.phase == 9000);

    features = SDL_HAPTIC_LEFTRIGHT;
    CHECK(gyRumbleDeviceQuery(handle));
    CHECK(gyRumblePlayEffect(handle, &effect, 2));
    CHECK(directPlays == 1 && directDuration == 250);
    effect.effectType = GY_RUMBLE_EFFECT_LEFTRIGHT;
    effect.leftRight.largeIntensity = 0.5f;
    effect.leftRight.smallIntensity = 1;
    effect.duration = GY_RUMBLE_EFFECT_DURATION_INFINITE;
    CHECK(gyRumblePlayEffect(handle, &effect, 1));
    CHECK(captured.type == SDL_HAPTIC_LEFTRIGHT && captured.leftright.length == SDL_HAPTIC_INFINITY);
    CHECK(captured.leftright.large_magnitude == 32767 && captured.leftright.small_magnitude == 65535);
    CHECK(gyRumbleStop(handle) && stopped == 1);

    failCreate = true;
    CHECK(!gyRumblePlayEffect(handle, &effect, 1));
    CHECK(errors == 1 && directPlays == 2);
    failCreate = false;
    failRun = true;
    CHECK(!gyRumblePlayEffect(handle, &effect, 1));
    CHECK(errors == 2 && directPlays == 3);
    CHECK(_gyRumbleSDLTryCloseHapticJoystick(joystick, 0));
    CHECK(closed == 1 && !gyRumblePoll(0, &handle));
    CHECK(gyRumbleUninit());
    SDL_Quit();
    puts("SDL3 haptic effect and envelope tests passed");
    return 0;
}
