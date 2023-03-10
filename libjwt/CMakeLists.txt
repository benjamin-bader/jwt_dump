set(libjwt_SRCS
    src/Base64.cc
    src/InputError.cc
    src/JsonPrinter.cc
    src/JsonVisitor.cc
    src/Jwt.cc
)

add_library(libjwt STATIC ${libjwt_SRCS})

target_include_directories(libjwt
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>

    PRIVATE
    src
)

target_link_libraries(libjwt nlohmann_json::nlohmann_json)
