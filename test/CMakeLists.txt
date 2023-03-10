file(GLOB SRCS "*.cc")

add_executable(testjwt ${SRCS})

target_link_libraries(
  testjwt
  libjwt
  GTest::gmock_main
)

add_test(NAME testjwt COMMAND testjwt)
