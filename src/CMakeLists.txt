cmake_minimum_required(VERSION 4.0.0)
project(Assignment2 VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Werror)

set(BUILD_DIR "${CMAKE_SOURCE_DIR}/../build")
set(BIN_DIR "${CMAKE_SOURCE_DIR}/../bin")

set(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})

set(APPNAME Assignment2)

file(GLOB SOURCES *.cpp)

find_package(OpenMP REQUIRED)

add_executable(${APPNAME} ${SOURCES})

target_link_libraries(${APPNAME} PUBLIC OpenMP::OpenMP_CXX)
