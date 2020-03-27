project(network)
cmake_minimum_required(VERSION 3.11)

include_directories("C:\\soft\\python38\\include")
link_directories("C:\\soft\\python38\\libs")

find_package(Boost 1.72.0 COMPONENTS thread python )

if(Boost_FOUND)
	set(Boost_USE_MULTITHREADED ON)
	include_directories(${Boost_INCLUDE_DIRS})
	link_libraries(${Boost_LIBRARIES})
endif()

if(UNIX)
	add_compile_options(-fPIC)
endif()

add_compile_definitions(BOOST_PYTHON_STATIC_LIB)

link_libraries(utils)
aux_source_directory(. src)
add_library(network_shared SHARED ${src})
add_library(network ${src})

