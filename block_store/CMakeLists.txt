cmake_minimum_required (VERSION 2.8)
project(block_store)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -Wshadow -Wpedantic -D_XOPEN_SOURCE=500")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELEASE} -g")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Wpedantic -D_XOPEN_SOURCE=500")

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

include_directories(${bitmap_INCLUDE_DIRS} include)

add_library(${PROJECT_NAME} SHARED src/${PROJECT_NAME}.c)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} bitmap)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES include/${PROJECT_NAME}.h DESTINATION include)


set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
	CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

add_executable(${PROJECT_NAME}_test test/tests.cpp)
target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME} gtest pthread)
