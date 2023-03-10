set(main_SRCS
main.cc)

add_executable(jwt_dump ${main_SRCS})

target_link_libraries(
    jwt_dump
    libjwt
)

install(TARGETS jwt_dump DESTINATION bin)
