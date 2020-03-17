project(vcpu)
cmake_minimum_required(VERSION 3.11)

add_subdirectory(network)
add_subdirectory(utils)

link_libraries(network utils)

add_executable(vcpu main.cpp)
add_executable("test" "test.cpp")

add_dependencies(network utils)
add_dependencies(vcpu utils network)
add_dependencies("test" utils )
