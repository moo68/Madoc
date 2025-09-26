# This CMakeLists.txt file was made largely using AI! Beware!
cmake_minimum_required(VERSION 3.10)
project(Madoc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)

set(SOURCES ${CMAKE_SOURCE_DIR}/external/glad/src/glad.c
        src/main.cpp
        src/shader_utils.cpp
        include/madoc/shader_utils.h
        include/madoc/log_utils.h
        src/log_utils.cpp
        src/voronoi.cpp
        include/madoc/voronoi.h)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include ${CMAKE_SOURCE_DIR}/external/glfw/include ${CMAKE_SOURCE_DIR}/include )

target_link_libraries(${PROJECT_NAME} glfw)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()
