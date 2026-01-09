cmake_minimum_required(VERSION 3.5)
project(test_unitree_thread VERSION 1.0.0)

## Set compiler to use c++ 17 features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_HAVE_PTHREAD_ARG 1)
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
list(APPEND CMAKE_PREFIX_PATH "/opt/unitree_robotics/lib/cmake")
find_package(unitree_sdk2 REQUIRED)

include_directories(
    /opt/unitree_robotics/include)

add_executable(test_unitree_thread
    test_unitree_thread.cpp
)
target_link_libraries(test_unitree_thread
    unitree_sdk2
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog
)
