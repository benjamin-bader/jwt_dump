load ("//bazel:jd.bzl", "jd_cc_library", "jd_cc_test")

jd_cc_library(
    name = "config",
    srcs = [],
    hdrs = ["config.h"],
    deps = [],
)

jd_cc_library(
    name = "base64",
    srcs = ["Base64.cc"],
    hdrs = ["Base64.h"],
    deps = [],
)

jd_cc_library(
    name = "fifo_map",
    srcs = [],
    hdrs = ["fifo_map.hpp"],
    deps = [],
)

jd_cc_library(
    name = "input_error",
    srcs = ["InputError.cc"],
    hdrs = ["InputError.h"],
    deps = [],
)

jd_cc_library(
    name = "termcolor",
    srcs = [],
    hdrs = ["termcolor.hpp"],
    deps = [],
)

jd_cc_library(
    name = "json_visitor",
    srcs = ["JsonVisitor.cc"],
    hdrs = ["JsonVisitor.h"],
    deps = [
        "@json",
        ":fifo_map",
    ],
)

jd_cc_library(
    name = "json_printer",
    srcs = ["JsonPrinter.cc"],
    hdrs = ["JsonPrinter.h"],
    deps = [
        "@json",
        ":config",
        ":json_visitor",
        ":termcolor",
    ],
)

jd_cc_library(
    name = "jwt",
    srcs = ["Jwt.cc"],
    hdrs = ["Jwt.h"],
    deps = [
        "@json",
        ":base64",
        ":input_error",
        ":json_visitor",
    ],
)

jd_cc_library(
    name = "libjwt",
    srcs = [],
    hdrs = [],
    deps = [
        "@json",
        ":config",
        ":input_error",
        ":json_printer",
        ":jwt",
    ],
    visibility = [
        "//visibility:public"
    ]
)
