# Docs suggest that "http_archive" and friends have deprecated implementations,
# and better ones can be had by manually loading them.
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_googletest",
    url = "https://github.com/google/googletest/archive/release-1.8.1.zip",
    sha256 = "927827c183d01734cc5cfef85e0ff3f5a92ffe6188e0d18e909c5efebf28a0c7",
    strip_prefix = "googletest-release-1.8.1",
)

http_archive(
    name = "json",
    url = "https://github.com/nlohmann/json/releases/download/v3.5.0/include.zip",
    sha256 = "3564da9c5b0cf2e032f97c69baedf10ddbc98030c337d0327a215ea72259ea21",
    strip_prefix = "",
    build_file = "//third_party:json.BUILD",
)

