include(ESDev.pri)

TEMPLATE = subdirs
CONFIG += ordered
CONFIG 	+= console


SUBDIRS +=  ../../libGyro/qtcreator/libGyro\
    ../../Spine/qtcreator/Spine \
    ../../libGyro-Kinetic

estk {
    SUBDIRS += ../../Lua5.2/qtcreator/Lua5_2 \
    ../../OOLua/qtcreator/OOLua
}

tests {
    SUBDIRS += ../../UnitTests
}

estk {
    SUBDIRS += ../../ESToolkit/ESToolkit
}


vulkan {
    DEFINES+=LIBGYRO_VULKAN
}


libGyro.depends = Spine \
        LibGyro-Kinetic

estk {

    OOLua.depends = Lua5_2

    ESToolkit.depends += libGyro \
      OOLua \
      Spine \
        LibGyro-Kinetic

}

tests {
    ESToolkit.depends += UnitTests
}



