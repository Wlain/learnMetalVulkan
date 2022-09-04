cmake_minimum_required(VERSION 3.8)
project(learnMetalVulkan)

# metal-cpp
add_subdirectory(3rdparty/metal-cpp)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
# glfw
add_subdirectory(3rdparty/glfw)
# glew
add_subdirectory(3rdparty/glew/build/cmake)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/3rdparty/glfw/deps
        ${CMAKE_CURRENT_LIST_DIR}/3rdparty/glew/include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp samples/*.cpp samples/*.mm)
add_executable(learnMetalVulkan ${SOURCES} samples/windowGL.cpp)

target_link_libraries(learnMetalVulkan ${SDL2_LIBRARY} glfw glew metal-cpp)