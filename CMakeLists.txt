cmake_minimum_required(VERSION 3.5)
project(legged_sdk VERSION 1.0.0)

## Set compiler to use c++ 17 features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Use GNUInstallDirs to install libraries into correct locations on all platforms.
include(GNUInstallDirs)

## Put all binary files into /bin and libraries into /lib
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

## Import thirdparty libraries
add_subdirectory(thirdparty)

set(THREADS_HAVE_PTHREAD_ARG 1)
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include
    thirdparty/include
)

file(GLOB_RECURSE SDK_HEADERS
    include/**/*.hpp
    include/**/*.ipp
    include/**/*.h
    )
file(GLOB_RECURSE SDK_SOURCES
    include/idl/**/*.cxx)
add_library(legged_sdk ${SDK_HEADERS} ${SDK_SOURCES})
target_link_libraries(legged_sdk
    rt
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog 
    fastcdr
    fastdds
    $<$<BOOL:${MINGW}>:ws2_32>
    )
set_target_properties(legged_sdk PROPERTIES LINKER_LANGUAGE CXX)

## Install the library
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# For generating reference legged_sdkTargets.cmake file in the first time
install(TARGETS legged_sdk
    EXPORT legged_sdkTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
install(EXPORT legged_sdkTargets
    FILE legged_sdkTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/legged_sdk)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/legged_sdkConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
    )
configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/legged_sdkConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/legged_sdkConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/legged_sdk
    )

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/legged_sdkConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/legged_sdkConfigVersion.cmake
    # ${CMAKE_CURRENT_LIST_DIR}/cmake/legged_sdkTargets.cmake       # directly install modified cmake file including thirdparty libs configs
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/legged_sdk)