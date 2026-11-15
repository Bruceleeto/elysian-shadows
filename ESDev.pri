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
#CONFIG += warn_off


!dreamcast {
    QMAKE_CXXFLAGS += -Wall -Wno-unused-local-typedef -Wno-unknown-pragmas -Wno-inconsistent-missing-override
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

QMAKE_CXXFLAGS += -std=c++1z
#QMAKE_CFLAGS += -std=c11

!dreamcast {
QMAKE_CXXFLAGS += -Wno-unknown-warning-option #-fno-rtti -fno-exceptions
    CONFIG += c++14
}

CONFIG -= exceptions rtti

tests {
    DEFINES += ESTK_UNIT_TESTS
}


