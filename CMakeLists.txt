cmake_minimum_required(VERSION 3.13)
project(CalcLang LANGUAGES CXX)

# ==========================================
# Compiler and C++ standard
# ==========================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use multi-threaded runtime on MSVC
if(MSVC)
    foreach(flag_var
        CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
endif()

# ==========================================
# Disable DIA SDK (prevents LNK1181)
# ==========================================
set(LLVM_ENABLE_DIA_SDK OFF CACHE BOOL "Disable DIA SDK" FORCE)
add_definitions(-DLLVM_ENABLE_DIA_SDK=0)

# ==========================================
# Find LLVM
# ==========================================
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Show LLVM targets available
message(STATUS "LLVM targets available: ${LLVM_TARGETS_TO_BUILD}")

# ==========================================
# Source files
# ==========================================
set(SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
    src/optimizer.cpp
    src/backend.cpp
    src/jit.cpp
    src/benchmark.cpp
    src/passes/ConstantFolding.cpp
    src/passes/DeadCodeElimination.cpp
    src/passes/AlgebraicSimplify.cpp
)

# ==========================================
# Create executable
# ==========================================
add_executable(calclang ${SOURCES})

# ==========================================
# Link LLVM libraries
# ==========================================
# FIXED: Only link components that are available in your LLVM build
# Remove architecture-specific targets that may not be installed
llvm_map_components_to_libnames(llvm_libs
    support
    core
    irreader
    passes
    analysis
    transformutils
    scalaropts
    instcombine
    ipo
    target
    mc
    object
    mcjit
    executionengine
    asmparser
    orcjit
    # Only include native target (automatically matches your system)
    native
)

target_link_libraries(calclang ${llvm_libs})

# ==========================================
# Warnings & Debug Options
# ==========================================
if(MSVC)
    target_compile_options(calclang PRIVATE /W4 /permissive-)
else()
    target_compile_options(calclang PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# ==========================================
# Output directories
# ==========================================
set_target_properties(calclang PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)

# ==========================================
# Optional: Testing support
# ==========================================
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()