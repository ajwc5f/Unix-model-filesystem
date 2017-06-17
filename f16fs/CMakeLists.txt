cmake_minimum_required (VERSION 2.8)
project(f16fs)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror -g -D_XOPEN_SOURCE=700")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror -g -D_XOPEN_SOURCE=700")

include_directories(${block_store_INCLUDE_DIRS} ${bitmap_INCLUDE_DIRS} ${dyn_array_INCLUDE_DIRS} include)

add_library(${PROJECT_NAME} SHARED src/${PROJECT_NAME}.c src/backend.c)

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} block_store bitmap dyn_array)

add_executable(${PROJECT_NAME}_test test/tests.cpp)

# set to 1 to enable grad/bonus tests
target_compile_definitions(${PROJECT_NAME}_test PRIVATE GRAD_TESTS=0)
target_link_libraries(${PROJECT_NAME}_test gtest pthread dyn_array ${PROJECT_NAME})
