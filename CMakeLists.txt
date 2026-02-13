set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_minimum_required(VERSION 3.10.0)
project(file-share VERSION 1.0.2 LANGUAGES C CXX)

add_executable(file-share
    src/main.cpp
    src/config.cpp
)

target_include_directories(file-share PRIVATE libs/cpp-httplib)