load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "backup_set",
    srcs = ["backup_set.cpp"],
    hdrs = ["backup_set.h"],
    deps = ["//libs/img_handler:img_handler", "//libs/file_handler:file_handler"],
    visibility = ["//visibility:public"]
)

cc_library(
    name = "restore",
    srcs = ["restore.cpp"],
    hdrs = ["restore.h"],
    deps = ["//libs/img_handler:file_struct_lib", "//libs/file_handler:file_handler", "backup_set"],
    visibility = ["//visibility:public"]
)

