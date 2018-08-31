#
#
cmake_minimum_required(VERSION 3.0)

project(imu)

add_executable(
  imud
  main.c
  serial.c
  shmem.c
)

add_executable(
  read_test
  Kalman.c 
  shm_read.cpp
  shmem.c
  lib/MadgwickAHRS.cpp
  lib/MahonyAHRS.cpp
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
