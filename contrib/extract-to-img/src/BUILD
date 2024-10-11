load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")


cc_binary(
    name = "main",
    srcs = select({
        "@platforms//os:windows" : ["main_win.cpp"],
        "@platforms//os:linux" : ["main_linux.cpp"]
    }),
    deps = ["//libs"],
    copts = select({
        "@platforms//os:windows" : ["-std:c++17"],
        "@platforms//os:linux" : ["-std=c++17"]
    }),
)

