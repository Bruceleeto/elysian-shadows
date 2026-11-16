/*
 * Copyright (c) 2013-2026 Elysian Shadows Team
 * SPDX-License-Identifier: MIT
 * Additional permission for Elysian Shadows Kickstarter backers: see LICENSE.
 */

#include <libGyro/gyro_net_api.h>
#include <libGyro/gyro_system_api.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

/* Servers and connections have the same handle representation. As with the
 * original API, the VMU caller owns handles and serializes access to the set. */
typedef struct TcpSocket {
    int fd;
    int ready;
    int listener;
} TcpSocket;

static struct pollfd* _pollSet;
static TcpSocket** _monitored;
static unsigned _count;
static unsigned _capacity;
static int _initialized;
static char _addressString[INET_ADDRSTRLEN];
static char _hostname[NI_MAXHOST];

static void _netError(const char* operation) {
    _gyLog(GY_DEBUG_ERROR, "GYRO_NET_API: %s: %s", operation, strerror(errno));
}

static uint64_t _milliseconds(void) {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (uint64_t)time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

static TcpSocket* _newSocket(int fd, int listener) {
    TcpSocket* handle = calloc(1, sizeof(*handle));
    if (!handle) {
        close(fd);
        return NULL;
    }
    handle->fd = fd;
    handle->listener = listener;
    return handle;
}

int gyNetInit(const GYNetInitParams* params) {
    if (_initialized) return 1;
    if (!params || params->maxMonitoredConnections > INT_MAX) return 0;
    _capacity = params->maxMonitoredConnections;
    if (_capacity) {
        _pollSet = calloc(_capacity, sizeof(*_pollSet));
        _monitored = calloc(_capacity, sizeof(*_monitored));
        if (!_pollSet || !_monitored) {
            free(_pollSet);
            free(_monitored);
            _pollSet = NULL;
            _monitored = NULL;
            _capacity = 0;
            return 0;
        }
    }
    _count = 0;
    _initialized = 1;
    return 1;
}

int gyNetUninit(void) {
    for (unsigned i = 0; i < _count; ++i) _monitored[i]->ready = 0;
    free(_pollSet);
    free(_monitored);
    _pollSet = NULL;
    _monitored = NULL;
    _count = _capacity = 0;
    _initialized = 0;
    return 1;
}

int gyNetMonitoredConnectionsAdd(void* sock) {
    if (!_initialized || !sock) return 0;
    for (unsigned i = 0; i < _count; ++i)
        if (_monitored[i] == sock) return 1;
    if (_count == _capacity) return 0;
    TcpSocket* handle = sock;
    handle->ready = 0;
    _monitored[_count] = handle;
    _pollSet[_count] = (struct pollfd){handle->fd, POLLIN, 0};
    ++_count;
    return 1;
}

int gyNetMonitoredConnectionsRemove(void* sock) {
    if (!sock) return 0;
    TcpSocket* handle = sock;
    handle->ready = 0;
    for (unsigned i = 0; i < _count; ++i) {
        if (_monitored[i] == handle) {
            --_count;
            _monitored[i] = _monitored[_count];
            _pollSet[i] = _pollSet[_count];
            break;
        }
    }
    return 1;
}

int gyNetMonitoredConnectionsPoll(uint32_t timeout) {
    if (!_initialized) return -1;
    for (unsigned i = 0; i < _count; ++i) {
        _monitored[i]->ready = 0;
        _pollSet[i].revents = 0;
    }
    const uint64_t deadline = _milliseconds() + timeout;
    int result;
    for (;;) {
        const uint64_t now = _milliseconds();
        const uint64_t remaining = now < deadline ? deadline - now : 0;
        const int wait = remaining > INT_MAX ? INT_MAX : (int)remaining;
        result = poll(_pollSet, _count, wait);
        if (result > 0) break;
        if (result < 0 && errno != EINTR) {
            _netError("poll");
            return -1;
        }
        if (_milliseconds() >= deadline) return 0;
    }
    for (unsigned i = 0; i < _count; ++i) {
        /* EOF and errors must wake the consumer so recv can report disconnect. */
        _monitored[i]->ready = (_pollSet[i].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)) != 0;
    }
    return result;
}

int gyNetTcpSocketReady(const GYTcpSocket* sock) {
    return sock && ((const TcpSocket*)sock)->ready;
}

int gyNetResolveHost(GYIpAddress* addr, const char* host, uint16_t port) {
    if (!addr) return -1;
    addr->host = htonl(INADDR_NONE);
    addr->port = htons(port);
    if (!host) {
        addr->host = htonl(INADDR_ANY);
        return 0;
    }
    struct addrinfo hints = {0}, *result = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    const int status = getaddrinfo(host, NULL, &hints, &result);
    if (status != 0) {
        _gyLog(GY_DEBUG_ERROR, "GYRO_NET_API: Resolve %s: %s", host, gai_strerror(status));
        return -1;
    }
    addr->host = ((const struct sockaddr_in*)result->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(result);
    return 0;
}

const char* gyNetIpAddressToString(const GYIpAddress* addr) {
    if (!addr) return NULL;
    const struct in_addr ip = {addr->host};
    return inet_ntop(AF_INET, &ip, _addressString, sizeof(_addressString));
}

const char* gyNetResolveIp(GYIpAddress* addr) {
    if (!addr) return NULL;
    const struct sockaddr_in ip = {.sin_family = AF_INET, .sin_port = addr->port, .sin_addr = {addr->host}};
    const int status = getnameinfo((const struct sockaddr*)&ip, sizeof(ip), _hostname, sizeof(_hostname), NULL, 0, 0);
    if (status != 0) return gyNetIpAddressToString(addr);
    return _hostname;
}

int gyNetPeerIpAddress(GYIpAddress* addr, const GYTcpSocket* socket) {
    if (!addr || !socket) return 0;
    struct sockaddr_in ip;
    socklen_t size = sizeof(ip);
    if (getpeername(((const TcpSocket*)socket)->fd, (struct sockaddr*)&ip, &size) < 0) {
        _netError("getpeername");
        return 0;
    }
    addr->host = ip.sin_addr.s_addr;
    addr->port = ip.sin_port;
    return 1;
}

GYTcpSocket* gyNetTcpSocketOpen(const char* host, unsigned port) {
    GYIpAddress address;
    if (!host || port > UINT16_MAX || gyNetResolveHost(&address, host, port) != 0) return NULL;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        _netError("socket");
        return NULL;
    }
    const struct sockaddr_in ip = {.sin_family = AF_INET, .sin_port = address.port, .sin_addr = {address.host}};
    if (connect(fd, (const struct sockaddr*)&ip, sizeof(ip)) < 0) {
        _netError("connect");
        close(fd);
        return NULL;
    }
    return _newSocket(fd, 0);
}

int gyNetTcpSocketClose(GYTcpSocket* sock) {
    if (!sock) return 0;
    TcpSocket* handle = sock;
    gyNetMonitoredConnectionsRemove(handle);
    const int status = close(handle->fd);
    if (status < 0) _netError("close");
    free(handle);
    return status == 0;
}

int gyNetTcpSocketSend(const GYTcpSocket* socket, const void* data, size_t bytes) {
    if (!socket || (!data && bytes) || bytes > INT_MAX) return -1;
    const TcpSocket* handle = socket;
    size_t sent = 0;
    /* Preserve the API's send-all behavior. Suppress SIGPIPE per send, without
     * changing signal handling for the editor or any other thread. */
    while (sent < bytes) {
        const ssize_t count = send(handle->fd, (const char*)data + sent, bytes - sent, MSG_NOSIGNAL);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) {
            if (count < 0) _netError("send");
            return sent ? (int)sent : -1;
        }
        sent += count;
    }
    return (int)sent;
}

int gyNetTcpSocketRecv(const GYTcpSocket* socket, void* data, size_t maxBytes) {
    if (!socket || (!data && maxBytes)) return -1;
    TcpSocket* handle = (TcpSocket*)socket;
    handle->ready = 0;
    ssize_t count;
    do {
        count = recv(handle->fd, data, maxBytes > INT_MAX ? INT_MAX : maxBytes, 0);
    } while (count < 0 && errno == EINTR);
    if (count < 0) _netError("recv");
    return (int)count;
}

GYTcpServer* gyNetTcpSocketServerCreate(unsigned port) {
    if (!_initialized || port > UINT16_MAX) return NULL;
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        _netError("server socket");
        return NULL;
    }
    const int reuse = 1;
    const int flags = fcntl(fd, F_GETFL);
    const struct sockaddr_in ip = {.sin_family = AF_INET, .sin_port = htons(port), .sin_addr = {htonl(INADDR_ANY)}};
    if (flags < 0 || setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0 ||
        fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0 ||
        bind(fd, (const struct sockaddr*)&ip, sizeof(ip)) < 0 || listen(fd, SOMAXCONN) < 0) {
        _netError("create listener");
        close(fd);
        return NULL;
    }
    TcpSocket* server = _newSocket(fd, 1);
    if (server && !gyNetMonitoredConnectionsAdd(server)) {
        gyNetTcpSocketClose(server);
        return NULL;
    }
    return server;
}

int gyNetTcpSocketServerDestroy(GYTcpServer* server) {
    return gyNetTcpSocketClose(server);
}

GYTcpServer* gyNetTcpSocketServerCheckIncomingConnections(GYTcpServer* server) {
    TcpSocket* handle = server;
    if (!handle || !handle->listener || !handle->ready) return NULL;
    handle->ready = 0;
    int fd;
    do { fd = accept(handle->fd, NULL, NULL); } while (fd < 0 && errno == EINTR);
    if (fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) _netError("accept");
        return NULL;
    }
    /* Some BSD systems inherit O_NONBLOCK from the listener. Connections keep
     * the public API's blocking send/recv semantics; only polling is nonblocking. */
    const int flags = fcntl(fd, F_GETFL);
    if (flags < 0 || fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
        _netError("configure accepted socket");
        close(fd);
        return NULL;
    }
    return _newSocket(fd, 0);
}
