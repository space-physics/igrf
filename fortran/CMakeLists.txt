cmake_minimum_required(VERSION 3.3)
project(igrf12 Fortran)
enable_testing()

add_compile_options(-O3)

if(${CMAKE_Fortran_COMPILER_ID} STREQUAL GNU)
  add_compile_options(-mtune=native -Werror=array-bounds)
elseif(${CMAKE_Fortran_COMPILER_ID} STREQUAL Intel)

elseif(${CMAKE_Fortran_COMPILER_ID} STREQUAL PGI)
  add_compile_options(-Mallocatable=03)
elseif(${CMAKE_Fortran_COMPILER_ID} STREQUAL Flang) 
  add_compile_options(-Mallocatable=03)
  link_libraries(-static-flang-libs)
endif()

add_executable(testigrf igrf12_demoprog.f igrf12.f)

