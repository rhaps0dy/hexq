cmake_minimum_required(VERSION 2.8)

project(HEXQ)

set(CMAKE_CXX_FLAGS_DEBUG  "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
add_definitions(-std=c++11)

add_executable(hexq directed_graph.cpp hexq_level.cpp main.cpp taxi_domain_mdp.cpp)
install(TARGETS hexq DESTINATION .)



