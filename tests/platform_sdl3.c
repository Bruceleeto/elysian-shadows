#include <SDL3/SDL.h>
#include <libGyro/gyro_input_api.h>
#include <libGyro/gyro_rumble_api.h>
#include <libGyro/gyro_thread_api.h>
#include <libGyro/gyro_performance_api.h>
#include <libGyro/gyro_net_api.h>
#include <libGyro/gyro_system_api.h>
#include <libGyro/gyro_file_api.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s (%s)\n", __FILE__, __LINE__, #expr, SDL_GetError()); abort(); } } while (0)

/* Supply the editor-owned hooks so the real platform objects can run headlessly. */
GYInputInitParams _initParams = {.controllerCount = 1};
static unsigned staleErrors;
void _gyLog(GY_DEBUG_SEVERITY level, const char* const format, ...) {
    char message[2048];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    if (level == GY_DEBUG_ERROR && strstr(message, "stale sentinel")) ++staleErrors;
}
void _gyPush(void) {}
void _gyPop(const unsigned count) { (void)count; }
const GYSystemInfo* gySystemQueryInfo(void) {
    static const GYSystemInfo info = {.cpuCount = 1};
    return &info;
}
int gyFileResolvePath(GYPathType type, const char* name, char** path, size_t* size) {
    (void)type;
    *path = strdup(name);
    *size = strlen(name);
    return *path != NULL;
}
extern int _sdlControllerInit(const GYInputInitParams* params);
extern int _sdlControllerUninit(void);

static Uint16 motorLow, motorHigh;
static unsigned rumbleCalls;
static bool SDLCALL rumble(void* user, Uint16 low, Uint16 high) {
    (void)user;
    motorLow = low;
    motorHigh = high;
    ++rumbleCalls;
    return true;
}

static void testControllers(void) {
    CHECK(SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "0"));
    CHECK(SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1"));
    const GYRumbleInitParams params = {.maxDevices = 1};
    CHECK(gyRumbleInit(&params));
    CHECK(_sdlControllerInit(&_initParams));
    CHECK((SDL_WasInit(0) & SDL_INIT_VIDEO) == 0);
    /* Do not open or rumble physical devices attached to the test machine. */
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);
    SDL_VirtualJoystickDesc desc;
    SDL_INIT_INTERFACE(&desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
    desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    desc.axis_mask = (1u << SDL_GAMEPAD_AXIS_COUNT) - 1;
    desc.button_mask = (1u << SDL_GAMEPAD_BUTTON_COUNT) - 1;
    desc.name = "libGyro test gamepad";
    desc.Rumble = rumble;
    SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
    CHECK(id != 0);
    SDL_SetError("stale sentinel");
    CHECK(gyInputPollEvents());
    GYController* controller;
    CHECK(gyControllerPoll(0, &controller));
    SDL_Joystick* joystick = SDL_GetJoystickFromID(id);
    CHECK(joystick);
    CHECK(SDL_IsGamepad(id));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, -32768));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 32767));
    CHECK(gyInputPollEvents());
    CHECK(gyControllerPoll(0, &controller));
    CHECK(controller->buttons[GY_CONTROLLER_BUT_X] & GY_BUT_FLAGS_IS_PRESSED);
    CHECK(controller->analogSticks[GY_CONTROLLER_ANALOG_LEFT].x == -1.0f);
    CHECK(controller->triggers[GY_CONTROLLER_TRIGGER_RIGHT] == 1.0f);
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false));
    CHECK(gyInputPollEvents());
    CHECK(gyControllerPoll(0, &controller));
    CHECK(!(controller->buttons[GY_CONTROLLER_BUT_X] & GY_BUT_FLAGS_IS_PRESSED));
    CHECK(controller->buttons[GY_CONTROLLER_BUT_X] & GY_BUT_FLAGS_WAS_PRESSED);

    GYRumble* handle;
    CHECK(gyRumblePoll(0, &handle));
    CHECK(gyRumblePlay(handle, 0.5f, 1000));
    CHECK(motorLow == 32767 && motorHigh == 32767);
    const GYRumbleEffect effect = {.effectType = GY_RUMBLE_EFFECT_LEFTRIGHT, .duration = 1,
                                  .leftRight = {.largeIntensity = 0.25f, .smallIntensity = 1}};
    CHECK(gyRumblePlayEffect(handle, &effect, 2));
    CHECK(motorLow == 16383 && motorHigh == 65535);
    CHECK(gyRumbleStop(handle));
    CHECK(motorLow == 0 && motorHigh == 0 && rumbleCalls >= 3);

    /* Exercise the file loader and allocator ownership, not just SDL's string API. */
    char filename[] = "/tmp/gyro-mappings-XXXXXX";
    int fd = mkstemp(filename);
    CHECK(fd >= 0);
    FILE* file = fdopen(fd, "w");
    CHECK(file);
    char guid[33];
    SDL_GUIDToString(SDL_GetJoystickGUID(joystick), guid, sizeof(guid));
    CHECK(fprintf(file, "%s,libGyro test mapping,a:b0,b:b1,leftx:a0,righttrigger:a5,platform:Linux,\n", guid) > 0);
    CHECK(fclose(file) == 0);
    CHECK(gyControllerLoadMappingsFile(filename));
    CHECK(unlink(filename) == 0);
    CHECK(!gyControllerLoadMappingsFile(filename));
    CHECK(SDL_DetachVirtualJoystick(id));
    CHECK(gyInputPollEvents());
    CHECK(!gyControllerPoll(0, &controller));
    CHECK(!gyRumblePoll(0, &handle));
    CHECK(!gyControllerPoll(1, &controller));
    id = SDL_AttachVirtualJoystick(&desc);
    CHECK(id != 0);
    CHECK(gyInputPollEvents());
    CHECK(gyControllerPoll(0, &controller));
    CHECK(_sdlControllerUninit());
    CHECK(SDL_GetJoystickFromID(id) == NULL);
    CHECK((SDL_WasInit(0) & SDL_INIT_GAMEPAD) == 0);
    CHECK(gyRumbleUninit());
    CHECK(staleErrors == 0);
}

typedef struct Worker {
    GYMutex* mutex;
    GYCondition* condition;
    GYSemaphore* sem;
    int ready;
} Worker;
static int worker(void* data) {
    Worker* w = data;
    CHECK(!gyMutexTryLock(w->mutex));
    CHECK(gySemaphorePost(w->sem));
    CHECK(gyMutexLock(w->mutex));
    w->ready = 1;
    CHECK(gyConditionSignal(w->condition));
    CHECK(gyMutexUnlock(w->mutex));
    return 37;
}

static void testThreads(void) {
    CHECK(gyThreadInit(NULL));
    Worker w = {.mutex = gyMutexCreate(), .condition = gyConditionCreate(), .sem = gySemaphoreCreate(0)};
    CHECK(w.mutex && w.condition && w.sem);
    SDL_SetError("stale sentinel");
    CHECK(!gySemaphoreTryWait(w.sem));
    CHECK(!gySemaphoreWaitTimeout(w.sem, 2));
    CHECK(gyMutexLock(w.mutex));
    CHECK(!gyConditionWaitTimeout(w.condition, w.mutex, 2));
    GYThread* thread = gyThreadCreate("test worker", worker, &w);
    CHECK(thread);
    CHECK(gySemaphoreWaitTimeout(w.sem, UINT32_MAX));
    while (!w.ready) CHECK(gyConditionWaitTimeout(w.condition, w.mutex, 1000));
    CHECK(gyMutexUnlock(w.mutex));
    CHECK(gyThreadWait(thread) == 37);
    CHECK(gySemaphorePost(w.sem));
    CHECK(gySemaphoreValue(w.sem) == 1);
    CHECK(gySemaphoreTryWait(w.sem));
    CHECK(gyConditionDestroy(w.condition));
    CHECK(gyMutexDestroy(w.mutex));
    CHECK(gySemaphoreDestroy(w.sem));
    GYAtomic value = 1;
    CHECK(gyAtomicAdd(&value, 2) == 1);
    CHECK(gyAtomicGet(&value) == 3);
    CHECK(gyAtomicSet(&value, 4) == 3);
    CHECK(gyAtomicGet(&value) == 4);
    CHECK(gyThreadUninit());
    CHECK(gyPerfInit(NULL));
    CHECK(gyPerfFrequency() > 0);
    const uint64_t before = gyPerfCounter();
    SDL_Delay(1);
    CHECK(gyPerfCounter() > before);
    CHECK(gyPerfUninit());
    CHECK(staleErrors == 0);
}

static unsigned reservePort(void) {
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(fd >= 0);
    struct sockaddr_in ip = {.sin_family = AF_INET, .sin_addr = {htonl(INADDR_LOOPBACK)}};
    CHECK(bind(fd, (struct sockaddr*)&ip, sizeof(ip)) == 0);
    socklen_t size = sizeof(ip);
    CHECK(getsockname(fd, (struct sockaddr*)&ip, &size) == 0);
    const unsigned port = ntohs(ip.sin_port);
    CHECK(close(fd) == 0);
    return port;
}

static void testNetwork(void) {
    const GYNetInitParams params = {.maxMonitoredConnections = 3};
    CHECK(gyNetInit(&params));
    GYIpAddress address;
    CHECK(gyNetResolveHost(&address, "127.0.0.1", 12345) == 0);
    CHECK(ntohs(address.port) == 12345 && address.host == htonl(INADDR_LOOPBACK));
    CHECK(strcmp(gyNetIpAddressToString(&address), "127.0.0.1") == 0);
    CHECK(gyNetResolveIp(&address));
    CHECK(gyNetResolveHost(&address, NULL, 0) == 0 && address.host == htonl(INADDR_ANY));
    CHECK(!gyNetTcpSocketServerCreate(65536));
    CHECK(!gyNetTcpSocketOpen("127.0.0.1", 65536));
    GYTcpServer* server = NULL;
    unsigned port = 0;
    for (unsigned i = 0; i < 16 && !server; ++i) {
        port = reservePort();
        server = gyNetTcpSocketServerCreate(port);
    }
    CHECK(server);
    CHECK(gyNetMonitoredConnectionsPoll(0) == 0);
    CHECK(!gyNetTcpSocketServerCheckIncomingConnections(server));
    GYTcpSocket* client = gyNetTcpSocketOpen("127.0.0.1", port);
    CHECK(client);
    CHECK(gyNetMonitoredConnectionsAdd(client));
    CHECK(gyNetMonitoredConnectionsAdd(client));
    CHECK(gyNetMonitoredConnectionsPoll(1000) > 0);
    GYTcpSocket* accepted = gyNetTcpSocketServerCheckIncomingConnections(server);
    CHECK(accepted);
    CHECK(!gyNetTcpSocketServerCheckIncomingConnections(server));
    CHECK(gyNetMonitoredConnectionsAdd(accepted));
    CHECK(!gyNetTcpSocketServerCreate(0)); /* full monitoring set */
    CHECK(gyNetPeerIpAddress(&address, client));
    CHECK(ntohs(address.port) == port);
    CHECK(strcmp(gyNetIpAddressToString(&address), "127.0.0.1") == 0);
    CHECK(gyNetMonitoredConnectionsPoll(2) == 0);
    CHECK(!gyNetTcpSocketReady(client) && !gyNetTcpSocketReady(accepted));
    const char message[] = "VMU frame";
    CHECK(gyNetTcpSocketSend(client, message, sizeof(message)) == sizeof(message));
    CHECK(gyNetMonitoredConnectionsPoll(1000) > 0);
    CHECK(gyNetTcpSocketReady(accepted) && !gyNetTcpSocketReady(client));
    char buffer[64];
    CHECK(gyNetTcpSocketRecv(accepted, buffer, sizeof(buffer)) == sizeof(message));
    CHECK(memcmp(buffer, message, sizeof(message)) == 0);
    CHECK(!gyNetTcpSocketReady(accepted));
    CHECK(gyNetTcpSocketSend(accepted, message, sizeof(message)) == sizeof(message));
    CHECK(gyNetMonitoredConnectionsPoll(1000) > 0);
    CHECK(gyNetTcpSocketReady(client));
    CHECK(gyNetTcpSocketRecv(client, buffer, sizeof(buffer)) == sizeof(message));
    CHECK(gyNetMonitoredConnectionsRemove(client));
    CHECK(gyNetMonitoredConnectionsAdd(client));
    CHECK(gyNetTcpSocketClose(client)); /* also removes a monitored descriptor */
    CHECK(gyNetMonitoredConnectionsPoll(1000) > 0);
    CHECK(gyNetTcpSocketReady(accepted));
    CHECK(gyNetTcpSocketRecv(accepted, buffer, sizeof(buffer)) == 0);
    CHECK(gyNetTcpSocketClose(accepted));

    /* A reset peer must report failure without delivering SIGPIPE to the app. */
    const int resetPeer = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(resetPeer >= 0);
    const struct sockaddr_in destination = {.sin_family = AF_INET, .sin_port = htons(port),
                                            .sin_addr = {htonl(INADDR_LOOPBACK)}};
    CHECK(connect(resetPeer, (const struct sockaddr*)&destination, sizeof(destination)) == 0);
    CHECK(gyNetMonitoredConnectionsPoll(1000) > 0);
    accepted = gyNetTcpSocketServerCheckIncomingConnections(server);
    CHECK(accepted && gyNetMonitoredConnectionsAdd(accepted));
    const struct linger reset = {.l_onoff = 1, .l_linger = 0};
    CHECK(setsockopt(resetPeer, SOL_SOCKET, SO_LINGER, &reset, sizeof(reset)) == 0);
    CHECK(close(resetPeer) == 0);
    CHECK(gyNetMonitoredConnectionsPoll(1000) > 0 && gyNetTcpSocketReady(accepted));
    CHECK(gyNetTcpSocketRecv(accepted, buffer, sizeof(buffer)) == -1);
    CHECK(gyNetTcpSocketSend(accepted, message, sizeof(message)) == -1);
    CHECK(gyNetTcpSocketClose(accepted));
    CHECK(gyNetTcpSocketServerDestroy(server));
    CHECK(gyNetMonitoredConnectionsPoll(0) == 0);
    CHECK(gyNetUninit());
    CHECK(gyNetInit(&params));
    CHECK(gyNetUninit());
}

int main(void) {
    testControllers();
    testThreads();
    testNetwork();
    SDL_Quit();
    puts("SDL3 platform and POSIX networking tests passed");
    return 0;
}
