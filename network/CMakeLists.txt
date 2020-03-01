project(network)

cmake_minimum_required(VERSION 3.11)

find_package(Boost 1.72.0 COMPONENTS thread)

if(Boost_FOUND)
	set(Boost_USE_MULTITHREADED ON)
	include_directories(${Boost_INCLUDE_DIRS})
	link_libraries(${Boost_LIBRARIES})
endif()

link_libraries(utils)

aux_source_directory(. src)
add_library(network_shared SHARED ${src})
add_library(network ${src})

