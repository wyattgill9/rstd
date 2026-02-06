cmake_minimum_required(VERSION 3.31)
project(cpp_test VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(absl CONFIG REQUIRED)
find_package(benchmark REQUIRED)
find_package(CapnProto REQUIRED)

capnp_generate_cpp(CAPNP_SRCS CAPNP_HDRS
    schema/point.capnp
)

add_executable(main
    src/main.cc
    ${CAPNP_SRCS}
    ${CAPNP_HDRS}
)

target_compile_features(main PRIVATE cxx_std_23)

target_compile_options(main PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:
        $<$<CONFIG:Release>:-O3 -march=native>
        $<$<CONFIG:Debug>:-O0 -g>
    >
)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(main PRIVATE
    ${CAPNP_LIBRARIES}
    benchmark::benchmark
    absl::flat_hash_map
)
