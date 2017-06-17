cmake_minimum_required (VERSION 2.8)

#project(OSLib)

# Neato
#set(CMAKE_DISABLE_SOURCE_CHANGES ON)
#set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)

#CMAKE_EXPORT_COMPILE_COMMANDS=ON
# Interesting, creats a listing of the compilation commands
# Neat for inspection

# Still doesn't work for building :/
#include(ProcessorCount)
#ProcessorCount(N)
#if(NOT N EQUAL 0)
	#message(STATUS "Building with ${N} cores!")
	#set(MAKEOPTS ${MAKEOPTS} -j${N})
#  set(CTEST_BUILD_FLAGS -j${N})
#  set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${N})
#endif()


if(APPLE)
	# Make policy CMP0042 warning go away
	set(CMAKE_MACOSX_RPATH ON)
endif()



#if (NOT CMAKE_BUILD_TYPE)
#	message(STATUS "Build type not set, defaulting to Release")
#	set(CMAKE_BUILD_TYPE Release)
#endif()


add_subdirectory(dyn_array)

add_subdirectory(bitmap)

add_subdirectory(block_store)

add_subdirectory(f16fs)

# My hero http://stackoverflow.com/a/16404000

