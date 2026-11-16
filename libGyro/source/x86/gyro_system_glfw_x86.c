/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_system_api.h>
#include <libGyro/gyro_defines.h>
#include <gl/gyro_video_gl.h>
#include <gl/gyro_gl.h>
#include <stdio.h>
#include <stdarg.h>
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
        //exit(0);
    } else {
        _gyLog(GY_DEBUG_VERBOSE, "SSE CPU Extensions detected! Fast-ass math enabled!");
    }
}
#endif
#endif

#if defined(__linux__)
#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#if GYRO_PLATFORM == GYRO_PLATFORM_Qt
extern int gyQtInit(void);
#endif

static GYSysInitParams _sysParams;

void gyDebugSigInit(void);
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
    //char strBuff[40];
    //gyDebugInit();

  const char* platString =
#if   defined(__linux__)
  "Linux";
#endif

    _gySysPrintBuildInfo(platString);
#if defined(__linux__) && defined(__x86_64__)
    _checkCPUCapabilities();
#endif

    _sdlQuerySystemInfo(&_systemInfo);

#if GYRO_PLATFORM == GYRO_PLATFORM_x86
    glfwInit();
#elif GYRO_PLATFORM == GYRO_PLATFORM_Qt
    gyQtInit();
#endif

    memcpy(&_sysParams, params, sizeof(GYSysInitParams));

    _gyPop(1);

    return 1;
}

int gySysUninit(void) {
#if GYRO_PLATFORM == GYRO_PLATFORM_x86
    glfwTerminate();
#endif

    //gyDebugUninit();
    return 1;
}

#if 0
void gyPrintf(const char *const str, ...) {
    va_list argptr;
    static char buffer[GYRO_PRINTF_BUFFER_SIZE];

    va_start(argptr, str);
    vsprintf(buffer, str, argptr);
    va_end(argptr);

    fprintf(stdout, "%s", buffer);
    fflush(stdout);
}
#endif

unsigned int gySysStatsVram(void) {
    return 0;
}

unsigned int gySysStatsRam(void) {
    return 0;
}





/* Intentionally using wrong preprocessor directive, should be __linux__.
 * This code was never actually compiling correctly for Linux, but it's standard POSIX so
 * this needs to be fuckin' fixed! */
#if defined(LINIIIIIIX)
static const char* gyDebugSigCodeToStr(int sig, int sig_code) {
    if(sig == SIGFPE)
        switch(sig_code) {
            case SI_USER:		return "SIGFPE (raised by program)";
            case FPE_INTDIV:	return "SIGFPE (integer divide by zero)";
            case FPE_INTOVF:	return "SIGFPE (integer overflow)";
            case FPE_FLTDIV:	return "SIGFPE (floating-point divide by zero)";
            case FPE_FLTOVF:	return "SIGFPE (floating-point overflow)";
            case FPE_FLTUND:	return "SIGFPE (floating-point underflow)";
            case FPE_FLTRES:	return "SIGFPE (floating-point inexact result)";
            case FPE_FLTINV:	return "SIGFPE (floating-point invalid operation)";
            case FPE_FLTSUB:	return "SIGFPE (subscript out of range)";
            default:			return "SIGFPE";
        }
    else if(sig == SIGSEGV)
        switch(sig_code) {
            case SI_USER:		return "SIGSEGV (rasied by program)";
            case SEGV_MAPERR:	return "SIGSEGV (address not mapped to object)";
            case SEGV_ACCERR:	return "SIGSEGV (invalid permissions for mapped object)";
            default:			return "SIGSEGV";
        }
    else if(sig == SIGTRAP)
        switch(sig_code) {
            case SI_USER:		return "SIGTRAP (raised by program)";
            case TRAP_BRKPT:	return "SIGTRAP (process breakpoint)";
            case TRAP_TRACE:	return "SIGTRAP	(process trace trap)";
            default:			return "SIGTRAP";
        }
    /* No suitable code found. */
    return strsignal(sig);
}

static void gyDebugSigHandler(int sig, siginfo_t* info, void* unused) {
    sigset_t x;
    sigfillset(&x);
    //DO NOT FUCKING FUCK WITH ME WHILE I'M HANDLING A SIGNAL!!!111
    sigprocmask(SIG_BLOCK, &x, NULL);

    (void) sig;
    (void) unused;
    int i, num;
    void* buf[64];
    char** symbols;

    num = backtrace(buf, 64);
    symbols = backtrace_symbols(buf, num);

    _gyLog(GY_DEBUG_VERBOSE, "**** BEGIN C/++ BACKTRACE ****");
    _gyPush();
    _gyLog(GY_DEBUG_VERBOSE, "RECIEVED SIGNAL: %s!",
           gyDebugSigCodeToStr(info->si_signo, info->si_code));
    for(i = 0; i < num; i++)
        _gyLog(GY_DEBUG_VERBOSE, "	%s", symbols[i]);
    _gyLog(GY_DEBUG_VERBOSE, "**** END C/++ BACKTRACE ****");
    _gyPop(1);

    if(_sysParams.segFaultHandler) {
        _sysParams.segFaultHandler(_sysParams.context);
        _exit(2);
    } else {
        _gyLog(GY_DEBUG_VERBOSE, "Exitting with default segfault handler.");
        //exit(-1);
    }
}


static void _regSigAction(const int sig, const char* name, struct sigaction*const sa) {
    struct sigaction so;
    sigaction(sig, sa, &so);
     if(so.sa_handler == SIG_IGN)
         _gyLog(GY_DEBUG_WARNING, "Unable to set up %s signal handler!", name);
}


#define _GY_SIGSTR(a) a, GYRO_STRINGIFY_MACRO(#a)

#endif

void gyDebugSigInit(void) {
// __LINUX__ => __linux__ change me when this is dereested!!!
#if defined(LIIIINIX)
    _gyLog(GY_DEBUG_VERBOSE, "Registering exception handler to POSIX signals.");
    //return;
    struct sigaction sa;

    /* Set up handler. */
    sa.sa_handler		= NULL;
    sa.sa_sigaction		= gyDebugSigHandler;

    sigfillset(&sa.sa_mask);
    sa.sa_flags			= SA_SIGINFO;

    /* Register signal handlers */
    _regSigAction(_GY_SIGSTR(SIGSEGV), &sa);
    _regSigAction(_GY_SIGSTR(SIGFPE), &sa);
    _regSigAction(_GY_SIGSTR(SIGTRAP), &sa);
    _regSigAction(_GY_SIGSTR(SIGBUS), &sa);
    _regSigAction(_GY_SIGSTR(SIGHUP), &sa);
    _regSigAction(_GY_SIGSTR(SIGQUIT), &sa);
    _regSigAction(_GY_SIGSTR(SIGILL), &sa);
    _regSigAction(_GY_SIGSTR(SIGABRT), &sa);
    //_regSigAction(_GY_SIG(SIGKILL), &sa);
    _regSigAction(_GY_SIGSTR(SIGTERM), &sa);

#endif
}

#if 0
#if GYRO_PLATFORM != GYRO_PLATFORM_Qt
int main(int argc, char *argv[])
{
    return gyro_entry_point(argc, argv);
}
#endif
#endif
