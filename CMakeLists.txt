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
  shm_read.c
  shmem.c
)

target_link_libraries( read_test ncurses m)
