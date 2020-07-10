cmake_minimum_required(VERSION 3.12)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug or Release")
endif()
project(igrf12 C Fortran)
enable_testing()

add_executable(igrf13_run src/igrf13_driver.f src/igrf13.f)
add_executable(igrf12_run src/igrf12_driver.f src/igrf12.f)
add_executable(igrf11_run src/igrf11_driver.f src/igrf11.f)

add_executable(geomag7 src/geomag70.c)
