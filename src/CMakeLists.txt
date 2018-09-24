#
#
cmake_minimum_required(VERSION 3.0)

include_directories( "${PROJECT_SOURCE_DIR}/include" )

add_library(imu serial.c shmem.c config.c)
