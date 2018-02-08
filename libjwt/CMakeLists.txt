set(libjwt_SRCS
  base64.cc
  json.cc
  jwt.cc)

add_library(libjwt STATIC ${libjwt_SRCS})

target_include_directories(libjwt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})