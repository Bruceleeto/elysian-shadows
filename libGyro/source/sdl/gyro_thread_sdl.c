/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#   define GY_THREAD_PTHREADS

#ifdef GY_THREAD_PTHREADS
#   if defined(__linux__)
    /*Linux is fucking retarded and needs this defined
     * before any system headers if you want sched.h to
     * do its fucking job and actually work...
     */
#   define _GNU_SOURCE
#   endif
#endif

#include <libGyro/gyro_thread_api.h>
#include <libGyro/gyro_platform.h>
#include <libGyro/gyro_system_api.h>
#include <libGyro/gyro_defines.h>
#include <assert.h>
#include <stdio.h>
#include <limits.h>

#include <SDL3/SDL.h>

#ifdef GY_THREAD_PTHREADS
#   include <pthread.h>

#if defined(__linux__)
#   include <sched.h>
#endif

#endif

static struct SDLThreadContext {
#ifdef GY_THREAD_DEBUG
    GYThreadDebugInfo   dbg;
#endif
} _context;

static int _initialized = 0;

int gyThreadInit(const GYThreadInitParams *params) {
    if(_initialized) return 1;
    (void)params;
    _gyLog(GY_DEBUG_VERBOSE, "GYRO_THREAD_API: Initializing SDL Thread Back-End.");
    memset(&_context, 0, sizeof(struct SDLThreadContext));
    _initialized = 1;
    return 1;
}

int gyThreadUninit(void) {
    if(!_initialized) return 1;
    _initialized = 0;
    int success = 1;
    _gyLog(GY_DEBUG_VERBOSE, "GYRO_THREAD_API: Uninitializing SDL Thread Back-End.");
    _gyPush();

//Check for objects that were never deallocated
#ifdef GY_THREAD_DEBUG
    if(_context.dbg.activeThreadCount) {
        _gyLog(GY_DEBUG_WARNING, "Remaining Thread(s): %u",     _context.dbg.activeThreadCount);
        success = 0;
    }
    if(_context.dbg.activeMutexCount) {
        _gyLog(GY_DEBUG_WARNING, "Remaining Mutex(es): %u",     _context.dbg.activeMutexCount);
        success = 0;
    }
    if(_context.dbg.activeSemaphoreCount) {
        _gyLog(GY_DEBUG_WARNING, "Remaining Semaphore(s): %u",  _context.dbg.activeSemaphoreCount);
        success = 0;
    }
    if(_context.dbg.activeConditionCount)   {
        _gyLog(GY_DEBUG_WARNING, "Remaining Condition(s): %u",  _context.dbg.activeConditionCount);
        success = 0;
    }
#endif

    _gyPop(1);
    _initialized = 0;
    return success;
}

//==== THREAD API ======
GYThread* gyThreadCreate(const char* name,
                         GYThreadFunction fn,
                         void* data)
{
    assert(name && fn);
    SDL_Thread* thread = SDL_CreateThread(fn, name, data);
#ifdef GY_THREAD_DEBUG
    if(thread) ++_context.dbg.activeThreadCount;
#endif
    if (!thread) _gyLog(GY_DEBUG_ERROR, "Could not create thread: %s", SDL_GetError());
    return (GYThread*)thread;
}

int gyThreadKill(GYThread* thread) {
    (void)thread;
    return 0; //not supported by SDL threads
}

void gyThreadDetach(GYThread* thread) {
    if (!thread) return;
    //assert(thread); Safely accepts NULL threads
    SDL_DetachThread((SDL_Thread*)thread);
#ifdef GY_THREAD_DEBUG
    assert(_context.dbg.activeThreadCount);
    --_context.dbg.activeThreadCount;
#endif
}

const char* gyThreadName(GYThread* thread) {
    assert(thread);
    return SDL_GetThreadName((SDL_Thread*)thread);
}

int gyThreadWait(GYThread* thread) {
    if(!thread) return 0;
    char thdName[200] = { 0 };
    strncpy(thdName, gyThreadName(thread), sizeof(thdName) - 1);
    //assert(thread); Safely accepts NULL threads
    int retValue = 0;
    SDL_WaitThread((SDL_Thread*)thread, &retValue);
    _gyLog(GY_DEBUG_VERBOSE, "Thread[%s] joined with return value: [%d]", thdName, retValue);
#ifdef GY_THREAD_DEBUG
    assert(_context.dbg.activeThreadCount);
    --_context.dbg.activeThreadCount;
#endif
    return retValue;
}

int gyThreadPriority(GY_THREAD_PRIORITY priority) {
    SDL_ThreadPriority value;
    switch (priority) {
    case GY_THREAD_PRIORITY_LOW: value = SDL_THREAD_PRIORITY_LOW; break;
    case GY_THREAD_PRIORITY_MEDIUM: value = SDL_THREAD_PRIORITY_NORMAL; break;
    case GY_THREAD_PRIORITY_HIGH: value = SDL_THREAD_PRIORITY_HIGH; break;
    case GY_THREAD_PRIORITY_REALTIME: value = SDL_THREAD_PRIORITY_TIME_CRITICAL; break;
    default: return 0;
    }
    if (!SDL_SetCurrentThreadPriority(value)) {
        _gyLog(GY_DEBUG_ERROR, "Could not set thread priority: %s", SDL_GetError());
        return 0;
    }
    return 1;
}

int gyThreadCpuAffinity(GYCpuAffinityMask mask) {
    assert(mask);

#if   defined(GY_THREAD_PTHREADS)
    int success = 1;
    SDL_ThreadID sdlId = SDL_GetCurrentThreadID();

    //_gyLog(GY_DEBUG_VERBOSE, "Assigning Thread [%u] CPU Affinity.", sdlId);
    //_gyPush();

    cpu_set_t cpuMask;
    memset(&cpuMask, 0, sizeof(cpu_set_t));

    const unsigned maskSize = (sizeof(cpu_set_t) > sizeof(GYCpuAffinityMask))? sizeof(GYCpuAffinityMask) : sizeof(cpu_set_t);

    char buffer[sizeof(GYCpuAffinityMask)*8*5] = { 0 };
    char intBuff[10];
    strcat(buffer, "<");
    int first = 1;
    int cpuCount = gySystemQueryInfo()->cpuCount;

    for(unsigned i = 0; i < maskSize*8; ++i) {
        if((int)i >= cpuCount) break;
        uint32_t curByte = i/8;
        uint32_t curBit = i%8;
        uint8_t gyByte = *(((uint8_t*)mask) + curByte);
        if(gyByte & (0x1<<curBit)) {
            if(!first) {
                strcat(buffer, ", ");
            } else first = 0;
            sprintf(intBuff, "%u", i);
            strcat(buffer, intBuff);
            CPU_SET(i, &cpuMask);
        }
    }
    strcat(buffer, ">");
    _gyLog(GY_DEBUG_VERBOSE, "Setting CPU Affinity for Thread [%" SDL_PRIu64 "]: Cores: %s", sdlId, buffer);

    success &= !pthread_setaffinity_np(pthread_self(), sizeof(cpuMask), &cpuMask);
    if(!success) {
        _gyLog(GY_DEBUG_ERROR, "GY_THREAD_API: Failed to modify thread CPU affinity for thread: %" SDL_PRIu64, sdlId);
    }

   // _gyPop(1);
#else
    int success = 0;
#endif
    return success;
}

//==== MUTEX API ======
GYMutex* gyMutexCreate(void) {
    SDL_Mutex* mutex = SDL_CreateMutex();
#ifdef GY_THREAD_DEBUG
    if(mutex) ++_context.dbg.activeMutexCount;
#endif
    if (!mutex) _gyLog(GY_DEBUG_ERROR, "Could not create mutex: %s", SDL_GetError());
    return (GYMutex*)mutex;
}

int gyMutexDestroy(GYMutex* mtx) {
    if (!mtx) return 0;
    SDL_DestroyMutex((SDL_Mutex*)mtx);
#ifdef GY_THREAD_DEBUG
    assert(_context.dbg.activeMutexCount);
    --_context.dbg.activeMutexCount;
#endif
    return 1;
}

int gyMutexLock(GYMutex* mtx) {
    if (!(mtx)) return 0;
    SDL_LockMutex((SDL_Mutex*)mtx);
    return 1;
}

int gyMutexTryLock(GYMutex* mtx) {
    return mtx && SDL_TryLockMutex((SDL_Mutex*)mtx);
}

int gyMutexUnlock(GYMutex* mtx) {
    if (!(mtx)) return 0;
    SDL_UnlockMutex((SDL_Mutex*)mtx);
    return 1;
}

//==== SEMAPHORE API ======
GYSemaphore* gySemaphoreCreate(uint32_t initialValue) {
    SDL_Semaphore* sem = SDL_CreateSemaphore(initialValue);
#ifdef GY_THREAD_DEBUG
    if(sem) ++_context.dbg.activeSemaphoreCount;
#endif
    if (!sem) _gyLog(GY_DEBUG_ERROR, "Could not create semaphore: %s", SDL_GetError());
    return (GYSemaphore*)sem;
}
int gySemaphoreDestroy(GYSemaphore* sem) {
    if (!sem) return 0;
    SDL_DestroySemaphore((SDL_Semaphore*)sem);
#ifdef GY_THREAD_DEBUG
    --_context.dbg.activeSemaphoreCount;
#endif
    return 1;
}
int gySemaphorePost(GYSemaphore* sem) {
    if (!(sem)) return 0;
    SDL_SignalSemaphore((SDL_Semaphore*)sem);
    return 1;
}
int gySemaphoreWait(GYSemaphore* sem) {
    if (!(sem)) return 0;
    SDL_WaitSemaphore((SDL_Semaphore*)sem);
    return 1;
}

int gySemaphoreTryWait(GYSemaphore* sem) {
    return sem && SDL_TryWaitSemaphore((SDL_Semaphore*)sem);
}

uint32_t gySemaphoreValue(GYSemaphore* sem) {
    assert(sem);
    return SDL_GetSemaphoreValue((SDL_Semaphore*)sem);
}

int gySemaphoreWaitTimeout(GYSemaphore* sem, uint32_t ms) {
    if (!(sem)) return 0;
    const Uint64 deadline = SDL_GetTicks() + ms;
    for (;;) {
        const Uint64 now = SDL_GetTicks();
        const Uint64 remaining = now < deadline ? deadline - now : 0;
        const Sint32 timeout = ms == UINT32_MAX ? -1 : remaining > INT32_MAX ? INT32_MAX : (Sint32)remaining;
        if (SDL_WaitSemaphoreTimeout((SDL_Semaphore*)sem, timeout)) return 1;
        if (SDL_GetTicks() >= deadline) return 0;
    }
}

//=== CONDITIONAL API =====
GYCondition* gyConditionCreate(void) {
    SDL_Condition* cond = SDL_CreateCondition();
#ifdef GY_THREAD_DEBUG
    if(cond) ++_context.dbg.activeConditionCount;
#endif
    if (!cond) _gyLog(GY_DEBUG_ERROR, "Could not create condition: %s", SDL_GetError());
    return (GYCondition*)cond;
}

int gyConditionDestroy(GYCondition* cond) {
    if(!cond) return 0;
    SDL_DestroyCondition((SDL_Condition*)cond);
#ifdef GY_THREAD_DEBUG
    assert(_context.dbg.activeConditionCount);
    --_context.dbg.activeConditionCount;
#endif
    return 1;
}

int gyConditionBroadcast(GYCondition* cond) {
    if (!(cond)) return 0;
    SDL_BroadcastCondition((SDL_Condition*)cond);
    return 1;
}

int gyConditionSignal(GYCondition* cond) {
    if (!(cond)) return 0;
    SDL_SignalCondition((SDL_Condition*)cond);
    return 1;
}

int gyConditionWait(GYCondition* cond, GYMutex* mtx) {
    if (!(cond && mtx)) return 0;
    SDL_WaitCondition((SDL_Condition*)cond, (SDL_Mutex*)mtx);
    return 1;
}

int gyConditionWaitTimeout(GYCondition* cond, GYMutex* mtx, uint32_t ms) {
    if (!(cond && mtx)) return 0;
    const Uint64 deadline = SDL_GetTicks() + ms;
    for (;;) {
        const Uint64 now = SDL_GetTicks();
        const Uint64 remaining = now < deadline ? deadline - now : 0;
        const Sint32 timeout = ms == UINT32_MAX ? -1 : remaining > INT32_MAX ? INT32_MAX : (Sint32)remaining;
        if (SDL_WaitConditionTimeout((SDL_Condition*)cond, (SDL_Mutex*)mtx, timeout)) return 1;
        if (SDL_GetTicks() >= deadline) return 0;
    }
}

const GYThreadDebugInfo* gyThreadDebugQueryInfo(void) { return &_context.dbg; }

//==== ATOMICS ====

int gyAtomicAdd(GYAtomic* a, int v) {
    return SDL_AddAtomicInt((SDL_AtomicInt*)a, v);
}

int gyAtomicGet(GYAtomic* a) {
    return SDL_GetAtomicInt((SDL_AtomicInt*)a);
}

int gyAtomicSet(GYAtomic* a, int v) {
    return SDL_SetAtomicInt((SDL_AtomicInt*)a, v);
}

void gyCompilerBarrier(void) {
    SDL_CompilerBarrier();
}

