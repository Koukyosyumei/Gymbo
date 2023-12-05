# libcugymbo/CMakeLists.txt

# Find CUDA
find_package(CUDA REQUIRED)

# Set CUDA architecture (adjust as needed)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-arch=sm_61)

# Add CUDA library
add_library(libcugymbo
    gdcuda.h
)

# Specify CUDA as the language for this target
set_target_properties(libcugymbo PROPERTIES
    LANGUAGE CUDA
)

# Link CUDA libraries
target_link_libraries(libcugymbo
    PUBLIC
    CUDA::cudart
)
