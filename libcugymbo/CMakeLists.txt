
# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

file(GLOB projectSRC "*/*.h")

# Find CUDA
find_package(CUDA REQUIRED)

# Set CUDA architecture (adjust as needed)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-arch=sm_61)

# Define the CUDA-specific library target
cuda_add_library(libcugymbo INTERFACE)
