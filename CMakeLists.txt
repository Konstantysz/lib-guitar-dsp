cmake_minimum_required(VERSION 3.20)

project(lib-guitar-dsp VERSION 0.0.1 LANGUAGES CXX C)

# C++20 standard for library, C for PFFFT
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 99)

# Git submodule initialization (for PFFFT)
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Initializing PFFFT submodule...")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}")
        endif()
    endif()
endif()

# Verify PFFFT submodule exists
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/pffft/pffft.c")
    message(FATAL_ERROR "PFFFT submodule not found. Run: git submodule update --init --recursive")
endif()

# PFFFT library (C99, BSD license)
add_library(pffft STATIC
    external/pffft/pffft.c
)

target_include_directories(pffft PUBLIC
    external/pffft
)

# Advanced SIMD configuration with compiler feature detection
include(CheckCXXCompilerFlag)

if(MSVC)
    # Windows with MSVC
    check_cxx_compiler_flag(/arch:AVX2 COMPILER_SUPPORTS_AVX2)
    if(COMPILER_SUPPORTS_AVX2)
        target_compile_options(pffft PRIVATE /arch:AVX2)
        message(STATUS "PFFFT: Enabling AVX2 for MSVC")
    else()
        target_compile_definitions(pffft PRIVATE PFFFT_SIMD_DISABLE)
        message(STATUS "PFFFT: AVX2 not supported, using scalar implementation")
    endif()
else()
    # Non-Windows platforms
    check_cxx_compiler_flag(-mavx2 COMPILER_SUPPORTS_AVX2)
    check_cxx_compiler_flag(-mfma COMPILER_SUPPORTS_FMA)

    if(COMPILER_SUPPORTS_AVX2 AND COMPILER_SUPPORTS_FMA AND
       (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64" OR
        CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64" OR
        CMAKE_SYSTEM_PROCESSOR MATCHES "i386"))
        target_compile_options(pffft PRIVATE -mavx2 -mfma)
        message(STATUS "PFFFT: Enabling AVX2 and FMA for ${CMAKE_SYSTEM_PROCESSOR}")
    else()
        target_compile_definitions(pffft PRIVATE PFFFT_SIMD_DISABLE)
        message(STATUS "PFFFT: Using scalar implementation on ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

# Create guitar-dsp library (static)
add_library(guitar-dsp STATIC
    src/PitchDetector.cpp
    src/YinPitchDetector.cpp
    src/MpmPitchDetector.cpp
    src/HybridPitchDetector.cpp
    src/NoteConverter.cpp
    src/PitchStabilizer.cpp
)

target_include_directories(guitar-dsp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link PFFFT
target_link_libraries(guitar-dsp PUBLIC pffft)

# Math library on Unix
if(UNIX AND NOT APPLE)
    target_link_libraries(guitar-dsp PUBLIC m)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(guitar-dsp PRIVATE /W4 /WX)
else()
    target_compile_options(guitar-dsp PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wno-unused-parameter
    )
endif()
