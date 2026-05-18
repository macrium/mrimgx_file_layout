add_library(restore STATIC "restore.cpp" "restore.h" "crc32.cpp" "crc32.h" "framework.h" "pch.cpp")

if (WIN32)
    include_directories(../../dependencies/include)
else()
    target_include_directories(restore PUBLIC ${MRIMGX_POSIX_INCLUDES})
    target_link_libraries(restore PUBLIC file_reader file_operations encryption ${MRIMGX_POSIX_ZSTD_LIB})
endif()
