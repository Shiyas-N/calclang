cmake_minimum_required(VERSION 3.13)
project(CalcLang)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================
# MSVC configuration
# ==========================================
if(MSVC)
    foreach(flag_var
            CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_DEBUG)
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
endif()

# ==========================================
# Disable DIA SDK to prevent LNK1181
# ==========================================
# Some LLVM builds (especially prebuilt binaries) reference the old VS2019 DIA SDK.
# This disables DIA usage completely, which is safe unless you explicitly need LLVM's PDB debug symbol support.
set(LLVM_ENABLE_DIA_SDK OFF CACHE BOOL "Disable DIA SDK" FORCE)
add_definitions(-DLLVM_ENABLE_DIA_SDK=0)

# ==========================================
# Find LLVM
# ==========================================
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# ==========================================
# LLVM include & definitions
# ==========================================
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS})

# ==========================================
# Source files
# ==========================================
set(SOURCES
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
    src/optimizer.cpp
    src/passes/ConstantFolding.cpp
    src/passes/DeadCodeElimination.cpp
    src/passes/AlgebraicSimplify.cpp
)

# ==========================================
# Executable
# ==========================================
add_executable(calclang ${SOURCES})

# Link LLVM libraries (core + optimization passes)
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
)

target_link_libraries(calclang ${llvm_libs})

# ==========================================
# Output directories
# ==========================================
set_target_properties(calclang PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)
