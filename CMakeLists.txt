cmake_minimum_required(VERSION 3.13)
project("libgymbo" LANGUAGES C CXX)

# find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mtune=native -march=native")

set(SOURCE_DIR  "libgymbo")
set(TEST_DIR  "test")

include(GNUInstallDirs)

find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG)
pybind11_add_module(pylibgymbo src/main.cpp)

# include_directories("${PROJECT_SOURCE_DIR}/")

add_subdirectory(${SOURCE_DIR})

add_executable(gymbo gymbo.cpp)
target_link_libraries(gymbo libgymbo)

enable_testing()
add_subdirectory(${TEST_DIR})
