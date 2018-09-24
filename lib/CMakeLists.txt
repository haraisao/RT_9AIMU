#
#
cmake_minimum_required(VERSION 3.0)

include_directories( "${PROJECT_SOURCE_DIR}/include" )

file(GLOB C_SRCS *.c)
file(GLOB CXX_SRCS *.cpp)

add_library(imu_filter ${CXX_SRCS} ${C_SRCS})
