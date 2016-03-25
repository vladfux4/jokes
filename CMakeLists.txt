cmake_minimum_required(VERSION 2.6)
project(jokes)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
file(GLOB SRC . main.cpp TernaryTree.cpp)
file(GLOB HDR . TernaryNode.h TernaryTree.h)

add_executable(${PROJECT_NAME} ${SRC} ${HDR})
