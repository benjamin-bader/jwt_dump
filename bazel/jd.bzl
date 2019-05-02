
def jd_copts(custom_opts = []):
  msvc_opts = [
    "/std:c++14",
    "/DUNICODE",
    "/D_UNICODE",
    "/DWIN32_LEAN_AND_MEAN",
  ]

  posix_opts = [
    "-std=c++14",
  ]

  darwin_opts = posix_opts + [
    # ASIO spuriously triggers this warning on macOS.
    "-Wno-unused-local-typedef",
  ]

  return custom_opts + select({
    "@bazel_tools//src/conditions:darwin": darwin_opts,
    "@bazel_tools//src/conditions:windows": msvc_opts,
    "@bazel_tools//src/conditions:linux_x86_64": posix_opts,
    "@bazel_tools//src/conditions:linux_aarch64": posix_opts,
    "@bazel_tools//src/conditions:freebsd": posix_opts,
    "//conditions:default": posix_opts,
  })

def jd_linkopts(custom_linkopts):
  return custom_linkopts + select({
    "@bazel_tools//src/conditions:darwin": [],
    "//conditions:default": [],
  })

def jd_cc_library(
    name,
    srcs = [],
    hdrs = [],
    copts = [],
    linkopts = [],
    visibility = None,
    external_deps = None,
    tcmalloc_dep = None,
    repository = "",
    linkstamp = None,
    tags = [],
    deps = [],
    strip_include_prefix = None):
  if tcmalloc_dep:
    deps += tcmalloc_dep

  if copts == None:
    copts = []

  native.cc_library(
    name = name,
    srcs = srcs,
    hdrs = hdrs,
    copts = jd_copts(copts),
    linkopts = jd_linkopts(linkopts),
    visibility = visibility,
    tags = tags,
    deps = deps,
  )

def jd_cc_test(
    name,
    srcs = [],
    copts = [],
    visibility = None,
    external_deps = None,
    tcmalloc_dep = None,
    repository = "",
    linkstamp = None,
    tags = [],
    deps = [],
    strip_include_prefix = None):
  if tcmalloc_dep:
    deps += tcmalloc_dep

  if copts == None:
    copts = []

  if "@com_google_googletest//:gtest_main" not in deps:
    deps += ["@com_google_googletest//:gtest_main"]

  native.cc_test(
    name = name,
    srcs = srcs,
    copts = jd_copts() + copts,
    visibility = visibility,
    tags = tags,
    deps = deps
  )

def jd_cc_binary(
    name,
    srcs = [],
    copts = [],
    linkopts = [],
    visibility = None,
    external_deps = None,
    tcmalloc_dep = None,
    repository = "",
    linkstamp = None,
    tags = [],
    deps = [],
    strip_include_prefix = None):
  if tcmalloc_dep:
    deps += tcmalloc_dep

  if copts == None:
    copts = []

  native.cc_binary(
    name = name,
    srcs = srcs,
    copts = jd_copts(copts),
    linkopts = jd_linkopts(linkopts),
    visibility = visibility,
    tags = tags,
    deps = deps
)