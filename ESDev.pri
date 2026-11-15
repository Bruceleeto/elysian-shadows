exists( ESDevCommandLineArgs.pri ) {
    message("Including ESDevCommandLineArgs.pri in project file $${PWD}")
    include( ESDevCommandLineArgs.pri )
}

dreamcast {

    INCLUDEPATH=/home/jenkins/Dreamcast/cadcdev-kallistios/include \
        /home/jenkins/Dreamcast/cadcdev-kos-ports/include \
        /home/jenkins/Dreamcast/cadcdev-kallistios/kernel/arch/dreamcast/include \
        /home/jenkins/Dreamcast/cadcdev-kallistios/addons/include

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


