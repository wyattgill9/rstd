cmake_minimum_required(VERSION 3.31)
project(cpp_test VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES 
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

add_executable(main
    src/main.cc
)

target_compile_features(main PRIVATE cxx_std_23)

target_compile_options(main PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:
        $<$<CONFIG:Release>:-O3 -march=native>
        $<$<CONFIG:Debug>:-O0 -g>
    >
)
