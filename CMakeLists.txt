cmake_minimum_required(VERSION 4.2.0)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_MODULE_STD ON)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "4.2.0")
    set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD d0edc3af-4c50-42ea-a356-e2862fe7a444)
endif()

project(pp_2026 LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/utf-8 /W4)
endif()

# Последовательная версия
add_executable(matmul_seq src/matmul_seq.cpp)

# Параллельная версия с OpenMP
add_executable(matmul_omp src/matmul_omp.cpp)

# OpenMP для MSVC
if(MSVC)
    target_compile_options(matmul_omp PRIVATE /openmp)
else()
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(matmul_omp PUBLIC OpenMP::OpenMP_CXX)
    endif()
endif()