cmake_minimum_required(VERSION 3.5)
project(test_periodic_task VERSION 1.0.0)

## Set compiler to use c++ 17 features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/legged_sdk/lib/cmake")
find_package(legged_sdk REQUIRED)

include_directories(
    /opt/legged_sdk/include
)

add_executable(test_periodic_task
    test_periodic_task.cpp
)
target_link_libraries(test_periodic_task
    legged_sdk
)
