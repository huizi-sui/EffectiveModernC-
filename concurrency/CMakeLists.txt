cmake_minimum_required(VERSION 3.0.0)
project(concurrency)

# set(CMAKE_CXX_STANDARD 11)

add_executable(chapter3 chapter3.cpp)
target_link_libraries(chapter3 pthread)