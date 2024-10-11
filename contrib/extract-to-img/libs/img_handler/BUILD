load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "enums",
    hdrs = ["enums.h"],
    deps = ["//dependencies"]
)

cc_library(
    name = "file_struct_lib",
    hdrs = ["file_struct.h"],
    deps = [":enums"],
    visibility = ["//visibility:public"]
)

cc_library(
    name = "img_metadata_lib",
    hdrs = ["metadata.h"]
)

cc_library(
    name = "img_handler",
    srcs = ["img_handler.cpp"],
    hdrs = ["img_handler.h"],
    deps = ["//libs/file_handler:file_handler", ":img_metadata_lib", ":file_struct_lib"],
    visibility = ["//visibility:public"]
)