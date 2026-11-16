# libGyro-Kinetic: collision and rigid body physics.

add_library(libGyroKinetic STATIC
    source/gyro_kinetic_collision.c
    source/gyro_kinetic_physics.c)

target_include_directories(libGyroKinetic PUBLIC api)
target_link_libraries(libGyroKinetic PUBLIC libGyro)
