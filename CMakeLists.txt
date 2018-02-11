cmake_minimum_required(VERSION 3.2.2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(jwt_dump)

enable_testing()

add_subdirectory(main)
add_subdirectory(test)
add_subdirectory(libjwt)
