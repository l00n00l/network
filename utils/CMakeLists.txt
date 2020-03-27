project(utils)
cmake_minimum_required(VERSION 3.11)


find_package(Boost 1.72.0 COMPONENTS thread filesystem "regex" locale)

if(Boost_FOUND)
	set(Boost_USE_MULTITHREADED ON)
	include_directories(${Boost_INCLUDE_DIRS})
	link_libraries(${Boost_LIBRARIES})
endif()

if(UNIX)
	add_compile_options(-fPIC)
endif()

set(LUASRC 
lua/lapi.c 
lua/lcode.c 
lua/lctype.c 
lua/ldebug.c 
lua/ldo.c 
lua/ldump.c 
lua/lfunc.c 
lua/lgc.c 
lua/llex.c 
lua/lmem.c 
lua/lobject.c 
lua/lopcodes.c 
lua/lparser.c 
lua/lstate.c 
lua/lstring.c 
lua/ltable.c 
lua/ltm.c 
lua/lundump.c 
lua/lvm.c 
lua/lzio.c 
lua/lauxlib.c 
lua/lbaselib.c 
lua/lbitlib.c 
lua/lcorolib.c 
lua/ldblib.c 
lua/liolib.c 
lua/lmathlib.c 
lua/loslib.c 
lua/lstrlib.c 
lua/ltablib.c 
lua/lutf8lib.c 
lua/loadlib.c 
lua/linit.c)

aux_source_directory(. src)
add_library(utils  ${src} ${LUASRC})

