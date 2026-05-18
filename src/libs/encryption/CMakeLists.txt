add_library(encryption STATIC "encryption.cpp" "encryption.h" "framework.h" "pch.cpp" "pch.h")

if (WIN32)
    # Existing Windows build keeps using the bundled headers under src/dependencies/
    include_directories(../../dependencies/include)
else()
    # macOS / Linux use system-installed OpenSSL + nlohmann + zstd via MRIMGX_POSIX_INCLUDES
    target_include_directories(encryption PUBLIC ${MRIMGX_POSIX_INCLUDES})
    target_link_libraries(encryption PUBLIC OpenSSL::Crypto OpenSSL::SSL)
endif()
