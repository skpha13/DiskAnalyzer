cmake_minimum_required(VERSION 3.26)
project(DiskAnalyzer C)

set(CMAKE_C_STANDARD 11)

include_directories(headers)

add_executable(DiskAnalyzer FileSystem.c)
