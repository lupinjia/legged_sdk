find_package(Threads REQUIRED)

message(STATUS "Importing: ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/libfastdds.so")

add_library(fastdds SHARED IMPORTED GLOBAL)
set_target_properties(fastdds PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/libfastdds.so)
set_target_properties(fastdds PROPERTIES IMPORTED_NO_SONAME TRUE)
target_link_libraries(fastdds INTERFACE Threads::Threads fastcdr foonathan_memory)
target_link_directories(fastdds INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR})
target_include_directories(fastdds INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

message(STATUS "Importing: ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/libfastcdr.so")

add_library(fastcdr SHARED IMPORTED GLOBAL)
set_target_properties(fastcdr PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/libfastcdr.so)
set_target_properties(fastcdr PROPERTIES IMPORTED_NO_SONAME TRUE)
target_link_libraries(fastcdr INTERFACE Threads::Threads)
target_link_directories(fastcdr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR})
target_include_directories(fastcdr INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/fastcdr>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/fastcdr>)

message(STATUS "Importing: ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/libfoonathan_memory-0.7.3.so")

add_library(foonathan_memory SHARED IMPORTED GLOBAL)
set_target_properties(foonathan_memory PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/libfoonathan_memory-0.7.3.so)
set_target_properties(foonathan_memory PROPERTIES IMPORTED_NO_SONAME TRUE)
target_link_libraries(foonathan_memory INTERFACE Threads::Threads)
target_link_directories(foonathan_memory INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR})
target_include_directories(foonathan_memory INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

## Install targets
file(GLOB DDS_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_SYSTEM_PROCESSOR}/*")

install(FILES ${DDS_LIBRARIES} DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})