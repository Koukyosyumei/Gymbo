check_language(CUDA)
if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)

    # to tell your code it was found via #ifdef USE_CUDA:
    add_definitions(-DUSE_CUDA)    

    include_directories("${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}")a


    message("Cuda version: " ${CUDA_VERSION})
    CUDA_ADD_LIBRARY(libcugymbo STATIC
        gdcuda.h
    )
endif()

#find_package(CUDA REQUIRED)
#if(CUDA_FOUND)
#    message("Cuda version: " ${CUDA_VERSION})
#    CUDA_ADD_LIBRARY(libcugymbo STATIC
#        gdcuda.h
#    )
#endif()
