cmake_minimum_required(VERSION 3.22)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/.cache")

project(jwt_dump)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(GTest)
include(json)

enable_testing()

add_subdirectory(libjwt)
add_subdirectory(main)
add_subdirectory(test)
