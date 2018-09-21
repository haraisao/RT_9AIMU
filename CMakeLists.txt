#
#
cmake_minimum_required(VERSION 3.0)

project(imu)

add_executable(
  imud
  main.cpp
  serial.c
  shmem.c
  config.c

  Kalman.c 
  lib/my_filter.cpp
  lib/MadgwickAHRS.cpp
  lib/MahonyAHRS.cpp
  lib/complementary_filter.cpp
)

add_executable(
  read_test
  Kalman.c 
  shm_read.cpp
  shmem.c
  lib/my_filter.cpp
  lib/MadgwickAHRS.cpp
  lib/MahonyAHRS.cpp
  lib/complementary_filter.cpp
)

add_executable(
  read_test2
  Kalman.c 
  shm_read2.cpp
  shmem.c
  lib/MadgwickAHRS.cpp
  lib/MahonyAHRS.cpp
)

target_link_libraries( read_test ncurses m)
