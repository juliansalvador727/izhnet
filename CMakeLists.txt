cmake_minimum_required(VERSION 3.20)

project(izhnet LANGUAGES CXX)

# ---- Global settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(IZHNET_BUILD_TESTS "Build izhnet tests" ON)

# ---- Library ----
add_library(izhnet STATIC
  src/model/izhikevich.cpp
  src/network/network.cpp
  src/sim/simulator.cpp
  src/io/spike_logger.cpp
  src/analysis/metrics.cpp
)

target_include_directories(izhnet
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Nice warnings (GCC/Clang/MSVC)
if (MSVC)
  target_compile_options(izhnet PRIVATE /W4 /permissive-)
else()
  target_compile_options(izhnet PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- OpenMP ----
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
  target_link_libraries(izhnet PUBLIC OpenMP::OpenMP_CXX)
  target_compile_definitions(izhnet PUBLIC IZHNET_HAS_OPENMP=1)
else()
  target_compile_definitions(izhnet PUBLIC IZHNET_HAS_OPENMP=0)
endif()

# ---- CLI executable ----
add_executable(izhnet_cli
  src/main.cpp
)

target_link_libraries(izhnet_cli PRIVATE izhnet)

# ---- Tests ----
if (IZHNET_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
