cmake_minimum_required(VERSION 3.14...3.21)

project(igrf LANGUAGES Fortran)

enable_testing()

add_executable(igrf13_driver fortran/igrf13_driver.f90 fortran/igrf13.f)
set_target_properties(igrf13_driver PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
add_test(NAME igrf13 COMMAND $<TARGET_FILE:igrf13_driver> 2015.1 0 0 0 0 1)
