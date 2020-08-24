cmake_minimum_required(VERSION 3.12)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug or Release")
endif()
project(igrf C Fortran)
enable_testing()

add_executable(igrf13_run igrf13_driver.f igrf13.f)
add_executable(igrf12_run igrf12_driver.f igrf12.f)
add_executable(igrf11_run igrf11_driver.f igrf11.f)

add_executable(geomag7 geomag70.c)
