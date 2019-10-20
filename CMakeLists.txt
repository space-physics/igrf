cmake_minimum_required(VERSION 3.3)
project(igrf12 C Fortran)
enable_testing()

add_executable(testigrf src/igrf12_demoprog.f src/igrf12.f)

add_executable(geomag7 src/geomag70.c)
