#
#
cmake_minimum_required(VERSION 3.0)

project(imud)

include_directories( "${PROJECT_SOURCE_DIR}/include" )
include_directories( "${PROJECT_SOURCE_DIR}/lib" )

add_subdirectory(lib)
#add_subdirectory(src)

file(GLOB C_SRCS src/*.c)

add_executable( imud src/main.cpp ${C_SRCS})

target_link_libraries(imud imu_filter)

add_executable( read_test src/shm_read.cpp ${C_SRCS})
#add_executable( read_test2 shm_read2.cpp ${C_SRCS})

target_link_libraries(read_test imu_filter ncurses m)
