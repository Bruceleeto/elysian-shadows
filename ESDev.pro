# Top-level project: builds the libraries and ESToolkit.
# Build flags come from ESDev.pri, pulled in for every subproject by .qmake.conf.
#
# Build out of source, e.g.:
#   mkdir build && cd build && qmake ../ESDev.pro && make -j$(nproc)
#
# Optional: CONFIG+=tests (unit tests), CONFIG+=docs (Lua API docs)

TEMPLATE = subdirs
CONFIG += ordered
CONFIG += console

SUBDIRS += \
    libGyro/qtcreator/libGyro \
    Spine/qtcreator/Spine \
    libGyro-Kinetic \
    Lua5.2/qtcreator/Lua5_2 \
    OOLua/qtcreator/OOLua

tests {
    SUBDIRS += UnitTests
}

SUBDIRS += ESToolkit/ESToolkit

vulkan {
    DEFINES += LIBGYRO_VULKAN
}

libGyro.depends = Spine \
    LibGyro-Kinetic

OOLua.depends = Lua5_2

ESToolkit.depends += libGyro \
    OOLua \
    Spine \
    LibGyro-Kinetic

tests {
    ESToolkit.depends += UnitTests
}
