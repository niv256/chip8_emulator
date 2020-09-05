cmake_minimum_required(VERSION 3.15)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 14)

add_executable(chip8_emulator main.cpp chip8.cpp chip8.h)