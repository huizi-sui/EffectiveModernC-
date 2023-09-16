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

add_executable(chapter6_1 chapter6_1.cpp)
target_link_libraries(chapter6_1 pthread)

add_executable(chapter6_2 chapter6_2.cpp)
target_link_libraries(chapter6_2 pthread)

add_executable(chapter6_3 chapter6_3.cpp)
target_link_libraries(chapter6_3 pthread)

add_executable(chapter7_1 chapter7_1)
target_link_libraries(chapter7_1 pthread)

add_executable(chapter7_2 chapter7_2)
target_link_libraries(chapter7_2 pthread)

add_executable(ProducerAndComsumer1 ProducerAndComsumer1.cpp)
target_link_libraries(ProducerAndComsumer1 pthread)

add_executable(ProducerAndComsumer2 ProducerAndComsumer2.cpp)
target_link_libraries(ProducerAndComsumer2 pthread)

add_executable(ProducerAndComsumer3 ProducerAndComsumer3.cpp)
target_link_libraries(ProducerAndComsumer3 pthread)

add_executable(ProducerAndComsumer4 ProducerAndComsumer4.cpp)
target_link_libraries(ProducerAndComsumer4 pthread)