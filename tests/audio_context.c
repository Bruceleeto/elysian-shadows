#include <libGyro/gyro_audio_api.h>
#include <libGyro/gyro_system_api.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); abort(); } } while (0)

static unsigned errors;
void _gyLog(GY_DEBUG_SEVERITY level, const char* const format, ...) {
    if(level != GY_DEBUG_ERROR && level != GY_DEBUG_WARNING) return;
    ++errors;
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}
void _gyPush(void) {}
void _gyPop(const unsigned count) { (void)count; }

int main(void) {
    CHECK(alcGetCurrentContext() == NULL);
    CHECK(gyAudUninit());
    for(unsigned i = 0; i < 2; ++i) {
        CHECK(gyAudInit());
        ALCcontext* context = alcGetCurrentContext();
        CHECK(context != NULL);
        CHECK(alcGetContextsDevice(context) != NULL);
        CHECK(alGetString(AL_VENDOR) != NULL);
        GYAudioSource* source = gyAudSourceCreate();
        CHECK(source != NULL);
        gyAudSourceDestroy(source);
        CHECK(alGetError() == AL_NO_ERROR);
        CHECK(gyAudUninit());
        CHECK(alcGetCurrentContext() == NULL);
        CHECK(gyAudUninit());
    }
    CHECK(errors == 0);
    puts("OpenAL context lifecycle passed");
    return 0;
}
