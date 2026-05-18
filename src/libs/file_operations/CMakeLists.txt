add_library(file_operations STATIC "file_operations.cpp" "file_operations.h" "pch.h" "pch.cpp" "framework.h")

if (NOT WIN32)
    # macOS / Linux: pick up nlohmann/json + zstd + openssl headers from MRIMGX_POSIX_INCLUDES
    target_include_directories(file_operations PUBLIC ${MRIMGX_POSIX_INCLUDES})
endif()
