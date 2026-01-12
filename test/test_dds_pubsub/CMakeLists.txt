cmake_minimum_required(VERSION 3.5)
project(test_dds_pubsub VERSION 1.0.0)

## Set compiler to use c++ 17 features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/legged_sdk/lib/cmake")
find_package(legged_sdk REQUIRED)

include_directories(
    /opt/legged_sdk/include
)

add_executable(test_dds_pub
    test_dds_pub.cpp
)
target_link_libraries(test_dds_pub
    legged_sdk
)
add_executable(test_dds_sub
    test_dds_sub.cpp
)
target_link_libraries(test_dds_sub
    legged_sdk
)
add_executable(test_low_state_sub
    test_low_state_sub.cpp
)
target_link_libraries(test_low_state_sub
    legged_sdk
)
add_executable(test_low_state_pub
    test_low_state_pub.cpp
)
target_link_libraries(test_low_state_pub
    legged_sdk
)
add_executable(test_low_cmd_pub
    test_low_cmd_pub.cpp
)
target_link_libraries(test_low_cmd_pub
    legged_sdk
)
add_executable(test_low_cmd_sub
    test_low_cmd_sub.cpp
)
target_link_libraries(test_low_cmd_sub
    legged_sdk
)