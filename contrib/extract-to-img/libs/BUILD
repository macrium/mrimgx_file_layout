load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "libs",
    deps = select({
        "@platforms//os:windows" : ["//libs/vhdx_handler:vhdx_handler", "//libs/img_handler:img_handler", "//libs/restore:restore"],
        "@platforms//os:linux" : ["//libs/linux_virtdisk_handler:linux_virtdisk_handler", "//libs/img_handler:img_handler", "//libs/restore:restore"]
    }),
    visibility = ["//visibility:public"]
)