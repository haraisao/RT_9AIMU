# RT_9AIMU
This programs are for 9axes IMU sensor module provided by RT Corporation. This program can run on Raspberry Pi.

Released under the MIT License.

This software includes following external programs,
- MadwichAHRS.cpp, MahonyAHRS.cpp (from Open source IMU and AHRS algorithms, GNU General Public License v.3)

If you don't want to use above external programs, please delete them before building programs.

2020/06/04 Update for version 2.0

If you use this program with IMU version 1.0, you should edit 'include/RT_9S_IMU.h' and add '#define __OLD_VER__'.
