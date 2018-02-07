cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(jwt_dump)

include_directories(include)

file(GLOB SOURCES "src/*.cc")

add_executable(jwt_dump ${SOURCES})
