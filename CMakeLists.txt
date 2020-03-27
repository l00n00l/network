project(vcpu)
cmake_minimum_required(VERSION 3.11)

include_directories("C:\\soft\\python38\\include")
link_directories("C:\\soft\\python38\\libs")
add_compile_definitions(BOOST_PYTHON_STATIC_LIB)

add_subdirectory(network)
add_subdirectory(utils)

link_libraries(network utils)

add_executable(vcpu main.cpp)
add_executable("test" "test.cpp")

add_dependencies(network utils)
add_dependencies(vcpu utils network)
add_dependencies("test" utils )
