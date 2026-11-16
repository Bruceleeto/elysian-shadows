/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_system_api.h>
#include <libGyro/gyro_defines.h>
#include <gl/gyro_video_gl.h>
#include <gl/gyro_gl.h>
#include <SDL3/SDL.h>

#include "../common/gyro_system_callbacks.h"

#if !(defined(__linux__) && defined(__i386__))



#  if defined(__linux__) && defined(__x86_64__)

//execute x86 CPUID opcode
static inline void cpuid(int CPUInfo[4],int InfoType){
    __asm__ __volatile__ (
        "cpuid":
        "=a" (CPUInfo[0]),
        "=b" (CPUInfo[1]),
        "=c" (CPUInfo[2]),
        "=d" (CPUInfo[3]) :
        "a" (InfoType)
    );
}

static void _checkCPUCapabilities(void) {
    int SSE     = 0;

    int info[4];
    cpuid(info, 0);
    int nIds = info[0];

    //  Detect Instruction Set
    if (nIds >= 1){
        cpuid(info,0x00000001);
        SSE   = (info[3] & ((int)1 << 25)) != 0;
    }

    if(!SSE) {
        _gyLog(GY_DEBUG_ERROR, "SSE CPU Support Not Detected! Prepare for crash... and maybe consider a processor from this decade.");
    } else {
        _gyLog(GY_DEBUG_VERBOSE, "SSE CPU Extensions detected! Fast-ass math enabled!");
    }
}
#endif
#endif

#if GYRO_PLATFORM == GYRO_PLATFORM_Qt
extern int gyQtInit(void);
#endif

static GYSysInitParams _sysParams;

void _gySysPrintBuildInfo(const char*const platString);


static GYSystemInfo _systemInfo;

const char* gySystemPowerStateString(GY_SYSTEM_POWER_STATE state) {
    static const char* _stateStrings[] = {
        "POWER_STATE_UNKNOWN",
        "POWER_STATE_BATTERY",
        "POWER_STATE_AC",
        "POWER_STATE_CHARGING",
        "POWER_STATE_CHARGED"
    };
    return _stateStrings[state];
}

int gySystemPowerQueryInfo(GYSystemPowerInfo* info) {
    GY_ASSERT(info);
    SDL_PowerState state = SDL_GetPowerInfo(&info->batterySecondsRemaining, &info->batteryPercentageRemaining);
    switch(state) {
    case SDL_POWERSTATE_ON_BATTERY:
        info->state = GY_SYSTEM_POWER_STATE_BATTERY;
        break;
    case SDL_POWERSTATE_NO_BATTERY:
        info->state = GY_SYSTEM_POWER_STATE_AC;
        break;
    case SDL_POWERSTATE_CHARGING:
        info->state = GY_SYSTEM_POWER_STATE_CHARGING;
        break;
    case SDL_POWERSTATE_CHARGED:
        info->state = GY_SYSTEM_POWER_STATE_CHARGED;
        break;
    case SDL_POWERSTATE_UNKNOWN:
    default:
        info->state = GY_SYSTEM_POWER_STATE_UNKNOWN;
    }
    return 1;
}

const GYSystemInfo* gySystemQueryInfo(void) {
    return &_systemInfo;
}

extern void _gyQueryOperatingSystemInfoString(char* buffer, int buffSize);



static int _sdlQuerySystemInfo(GYSystemInfo* info) {
    int success = 1;
    int intBuff = 0;
    char strBuff[GYRO_SYSTEM_PLATFORM_NAME_SIZE];
    memset(&_systemInfo, 0, sizeof(GYSystemInfo));

    _gyLog(GY_DEBUG_VERBOSE, "[GY_SYSTEM_API]: Querying System Information");
    _gyPush();

    strncpy(strBuff, SDL_GetPlatform(), GYRO_SYSTEM_PLATFORM_NAME_SIZE - 1);
    strBuff[GYRO_SYSTEM_PLATFORM_NAME_SIZE - 1] = '\0';
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %50s", "Platform Name",    strBuff);
    if(info) strcpy(info->platformName, strBuff);

    _gyQueryOperatingSystemInfoString(_systemInfo.osInfo, GYRO_SYSTEM_PLATFORM_NAME_SIZE);
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %50s", "Operating System",    _systemInfo.osInfo);

    intBuff = SDL_GetNumLogicalCPUCores();
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %50d", "CPU Cores",    intBuff);
    if(info) info->cpuCount = intBuff;

    intBuff = SDL_GetSystemRAM();
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %47d MB", "System RAM",    intBuff);
    if(info) info->systemRam = intBuff;

    intBuff = SDL_GetCPUCacheLineSize();
    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %50d", "Cache Line Size",    intBuff);
    if(info) info->cacheLineSize = intBuff;

    _gyLog(GY_DEBUG_VERBOSE, "%-20s: %50s", "Compiler",    GYRO_COMPILER_VERSION_STRING);
    _gyPop(1);
    return success;
}

int gySysInit(const GYSysInitParams*const params) {
  const char* platString =
#if   defined(__linux__)
  "Linux";
#endif

    _gySysPrintBuildInfo(platString);
#if defined(__linux__) && defined(__x86_64__)
    _checkCPUCapabilities();
#endif

    _sdlQuerySystemInfo(&_systemInfo);

#if GYRO_PLATFORM == GYRO_PLATFORM_Qt
    gyQtInit();
#endif

    memcpy(&_sysParams, params, sizeof(GYSysInitParams));

    _gyPop(1);

    return 1;
}

int gySysUninit(void) {
    return 1;
}


unsigned int gySysStatsVram(void) {
    return 0;
}

unsigned int gySysStatsRam(void) {
    return 0;
}
