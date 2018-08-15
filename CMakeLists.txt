#
#
cmake_minimum_required(VERSION 3.0)

project(imu)

add_executable(
  imu
  main.c
  serial.c
  shmem.c
)

add_executable(
  read_test
  test_shm_read.c
  shmem.c
)
