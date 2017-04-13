cmake_minimum_required(VERSION 2.8.12)
project(igrf12 Fortran )

add_compile_options(-mtune=native)

add_executable(testigrf igrf12_demoprog.f igrf12.f)
