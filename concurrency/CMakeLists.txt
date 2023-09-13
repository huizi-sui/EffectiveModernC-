cmake_minimum_required(VERSION 3.0.0)
project(concurrency)

# set(CMAKE_CXX_STANDARD 11)

add_executable(chapter3 chapter3.cpp)
target_link_libraries(chapter3 pthread)

add_executable(chapter4_1 chapter4_1.cpp)
target_link_libraries(chapter4_1 pthread)

add_executable(chapter4_2 chapter4_2.cpp)
target_link_libraries(chapter4_2 pthread)

add_executable(chapter5_1 chapter5_1.cpp)
target_link_libraries(chapter5_1 pthread)