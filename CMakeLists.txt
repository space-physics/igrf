cmake_minimum_required(VERSION 3.13)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Debug or Release")
endif()
project(igrf C Fortran)
enable_testing()

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/in.nml
"&in
model=13
date=2012.
alt=0
clt=23
xln=23
/
")

add_executable(igrf_run src/igrf_driver.f90 src/igrf13.f src/igrf12.f src/igrf11.f)
add_test(NAME IGRF13
 COMMAND $<TARGET_FILE:igrf_run> ${CMAKE_CURRENT_BINARY_DIR}/in.nml ${CMAKE_CURRENT_BINARY_DIR}/out.nml)


add_executable(geomag7 src/geomag70.c)
