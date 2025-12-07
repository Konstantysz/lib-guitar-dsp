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

# Configure PFFFT options
set(PFFFT_USE_TYPE_FLOAT ON CACHE BOOL "" FORCE)
set(PFFFT_USE_TYPE_DOUBLE OFF CACHE BOOL "" FORCE)
set(INSTALL_PFFFT OFF CACHE BOOL "" FORCE)
set(PFFFT_USE_BENCH_FFTW OFF CACHE BOOL "" FORCE)
set(PFFFT_USE_BENCH_GREEN OFF CACHE BOOL "" FORCE)
set(PFFFT_USE_BENCH_KISS OFF CACHE BOOL "" FORCE)
set(PFFFT_USE_BENCH_POCKET OFF CACHE BOOL "" FORCE)
set(PFFFT_USE_FFTPACK OFF CACHE BOOL "" FORCE)
set(PFFFT_DISABLE_LINK_WITH_M ON CACHE BOOL "" FORCE)

# Include PFFFT library (handles SIMD configuration internally)
add_subdirectory(external/pffft EXCLUDE_FROM_ALL)

# Create guitar-dsp library (static)
add_library(guitar-dsp STATIC
    src/PitchDetector.cpp
    src/YinPitchDetector.cpp
    src/MpmPitchDetector.cpp
    src/HybridPitchDetector.cpp
    src/NoteConverter.cpp
    src/PitchStabilizer.cpp
    src/FFTProcessor.cpp
)

target_include_directories(guitar-dsp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link PFFFT
target_link_libraries(guitar-dsp PUBLIC PFFFT)

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
