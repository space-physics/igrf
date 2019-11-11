cmake_minimum_required(VERSION 3.3)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug or Release")
endif()
project(igrf12 C Fortran)
enable_testing()

add_executable(testigrf src/igrf12_demoprog.f src/igrf12.f)

add_executable(geomag7 src/geomag70.c)
