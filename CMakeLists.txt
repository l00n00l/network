project(vcpu)

cmake_minimum_required(VERSION 3.11)

add_subdirectory(network)
add_subdirectory(utils)
add_subdirectory(rapidjson)

link_libraries(network utils)

add_executable(vcpu main.cpp)

