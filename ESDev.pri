exists( ESDevCommandLineArgs.pri ) {
    message("Including ESDevCommandLineArgs.pri in project file $${PWD}")
    include( ESDevCommandLineArgs.pri )
}

dreamcast {
    # KallistiOS locations come from the environment set up by KOS's environ.sh.
    KOS_BASE = $$(KOS_BASE)
    KOS_PORTS = $$(KOS_PORTS)
    isEmpty(KOS_BASE): error("KOS_BASE is not set. Source your KallistiOS environ.sh first.")
    isEmpty(KOS_PORTS): KOS_PORTS = $${KOS_BASE}/../kos-ports

    INCLUDEPATH = $${KOS_BASE}/include \
        $${KOS_PORTS}/include \
        $${KOS_BASE}/kernel/arch/dreamcast/include \
        $${KOS_BASE}/addons/include

    DEFINES+=_arch_dreamcast \
        _arch_sub_pristine \
        ELYSIAN_ENGINE
}

CONFIG -= exceptions

# Lua 5.5 comes from the system on Linux (Debian/Ubuntu: liblua5.5-dev).
!dreamcast {
    CONFIG += link_pkgconfig
    PKGCONFIG += lua5.5
}

# sol2 (header-only C++/Lua bindings), vendored in sol2/.
INCLUDEPATH += $$PWD/sol2/include
!dreamcast {
    # Type-check every argument passed from Lua to C++ (scripts get an error, not a crash).
    DEFINES += SOL_ALL_SAFETIES_ON=1 SOL_NO_CHECK_NUMBER_PRECISION=1
}


!dreamcast {
    QMAKE_CXXFLAGS += -Wall
}

#QMAKE_CFLAGS += -std=c99

!dreamcast {
    QMAKE_CFLAGS += -msse
    QMAKE_CXXFLAGS += -msse
}

CONFIG(debug, debug|release) {
#	QMAKE_CXXFLAGS += -O0//
#	QMAKE_CFLAGS += -O0
}

CONFIG(release, debug|release) {
#	QMAKE_CXXFLAGS += -03
#	QMAKE_CFLAGS += -03
}

DEFINES += QT_NO_KEYWORDS QT_NO_DYNAMIC_CAST

dreamcast {
    DEFINES+=GYRO_PLATFORM=GYRO_PLATFORM_DC
} else {
    DEFINES += GYRO_PLATFORM=GYRO_PLATFORM_Qt
}

#Qt 6 requires C++17.
CONFIG += c++17


CONFIG -= exceptions rtti
CONFIG += exceptions_off rtti_off



