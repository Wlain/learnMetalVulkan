cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

# 定义库
add_library(metal-cpp metal-cpp.cpp)

# 头文件引入
target_include_directories(metal-cpp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# 链接库
target_link_libraries(metal-cpp
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework Foundation"
        "-framework QuartzCore")
