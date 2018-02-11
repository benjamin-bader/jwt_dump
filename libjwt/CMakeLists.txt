set(libjwt_SRCS
  Base64.cc
  InputError.cc
  JsonLexer.cc
  JsonPrinter.cc
  Jwt.cc)

add_library(libjwt STATIC ${libjwt_SRCS})

target_include_directories(libjwt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})