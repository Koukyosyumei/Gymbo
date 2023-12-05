
# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

file(GLOB projectSRC "*/*.h")

# Define the CUDA-specific library target
add_library(libcugymbo INTERFACE)

# Include directories
target_include_directories(libgymbo
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Set CUDA architecture, you may customize this based on your GPU architecture
set(CMAKE_CUDA_ARCHITECTURES 75 CACHE STRING "CUDA architectures" FORCE)

# Enable CUDA support
enable_language(CUDA)

# Specify CUDA standard
set_target_properties(libcugymbo PROPERTIES CUDA_STANDARD 11)

# Link against the CUDA runtime library
target_link_libraries(libcugymbo PUBLIC cuda)

# Specify additional dependencies if needed
# target_link_libraries(libcugymbo PUBLIC other_library)

# Install headers
# install(FILES gd_cuda.h DESTINATION include/libcugymbo)
