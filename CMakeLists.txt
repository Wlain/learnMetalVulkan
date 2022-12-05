cmake_minimum_required(VERSION 3.8)
project(learnMetalVulkan)
set(CMAKE_CXX_STANDARD 17)
set(SDL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SDL)

# submodule
# Vulkan
find_package(Vulkan REQUIRED)
# metal-cpp
add_subdirectory(3rdparty/metal-cpp)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
# SDL2
file(GLOB SDL_LIBS "${SDL_DIR}/lib/*.dylib")
file(GLOB SDL_INCLUDE "${SDL_DIR}/include")
include_directories(${SDL2_INCLUDE_DIR})
# glfw
add_subdirectory(3rdparty/glfw)
# glew
add_subdirectory(3rdparty/glew/build/cmake)
# glm
add_subdirectory(3rdparty/glm)
# spdlog
add_subdirectory(3rdparty/spdlog)
# shaderc
find_library(SHADERC_LIB shaderc_combined $ENV{VULKAN_SDK}/Lib)
# SPIRV-Cross
add_subdirectory(3rdparty/spirv-cross)

include_directories(
        ${SDL_INCLUDE}
        ${Vulkan_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/utils
        ${PROJECT_SOURCE_DIR}/src/core/common
        ${PROJECT_SOURCE_DIR}/src/core/backend/handle
        ${PROJECT_SOURCE_DIR}/src/core/backend/gl
        ${PROJECT_SOURCE_DIR}/src/core/backend/vk
        ${PROJECT_SOURCE_DIR}/src/core/backend/mtl
        ${PROJECT_SOURCE_DIR}/3rdparty
        ${PROJECT_SOURCE_DIR}/3rdparty/glfw/deps
        ${PROJECT_SOURCE_DIR}/3rdparty/glew/include)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.mm tests/*.cpp)
add_executable(learnMetalVulkan ${SOURCES})
file(COPY tests/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY tests/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(learnMetalVulkan ${SDL_LIBS} ${Vulkan_LIBRARIES} ${SHADERC_LIB} glfw glm glew spdlog metal-cpp spirv-cross-glsl spirv-cross-msl)