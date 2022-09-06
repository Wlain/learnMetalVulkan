cmake_minimum_required(VERSION 3.8)
project(learnMetalVulkan)
set(CMAKE_CXX_STANDARD 17)

# Vulkan
find_package(Vulkan REQUIRED)
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
# glm
add_subdirectory(3rdparty/glm)

# shaderc
find_library(SHADERC_LIB shaderc_combined $ENV{VULKAN_SDK}/Lib)

include_directories(
        ${Vulkan_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/core/common
        ${PROJECT_SOURCE_DIR}/src/core/backend/handle
        ${PROJECT_SOURCE_DIR}/src/core/backend/gl
        ${PROJECT_SOURCE_DIR}/src/core/backend/vk
        ${PROJECT_SOURCE_DIR}/src/core/backend/mtl
        ${PROJECT_SOURCE_DIR}/3rdparty/glfw/deps
        ${CMAKE_CURRENT_LIST_DIR}/3rdparty/glew/include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.mm tests/*.cpp)
add_executable(learnMetalVulkan ${SOURCES})

target_link_libraries(learnMetalVulkan ${SDL2_LIBRARY} ${Vulkan_LIBRARIES} ${SHADERC_LIB} glfw glm glew metal-cpp)