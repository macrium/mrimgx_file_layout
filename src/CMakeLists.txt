# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-DUNICODE -D_UNICODE)


# Set the output directory for debug builds to ${PROJECT_SOURCE_DIR}/debug
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/debug)

# Set the output directory for release builds to ${PROJECT_SOURCE_DIR}/release
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/release)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Project name varies by target executable
if (WIN32)
    project ("img_to_vhdx")
else()
    project ("img_to_raw")
endif()


# --- Cross-platform: locate third-party dependencies on non-Windows ---
# On Windows, the existing project bundles OpenSSL / zstd / nlohmann_json under
# src/dependencies/. On macOS / Linux we rely on the system package manager
# (Homebrew or apt) so the build doesn't drag in prebuilt Windows binaries.
if (NOT WIN32)
    # Apple Silicon Homebrew prefix isn't on the default search path
    if (APPLE)
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/opt/homebrew/opt/openssl@3")
        if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
            set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0")
        endif()
    endif()

    find_package(OpenSSL REQUIRED)
    find_library(ZSTD_LIB     zstd        REQUIRED PATHS /opt/homebrew/lib /usr/local/lib /usr/lib)
    find_path(ZSTD_INCLUDE    zstd.h      REQUIRED PATHS /opt/homebrew/include /usr/local/include /usr/include)
    find_path(NLOHMANN_INCLUDE nlohmann/json.hpp REQUIRED PATHS /opt/homebrew/include /usr/local/include /usr/include)

    message(STATUS "POSIX build:")
    message(STATUS "  OpenSSL: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "  zstd:    ${ZSTD_LIB} (include: ${ZSTD_INCLUDE})")
    message(STATUS "  nlohmann: ${NLOHMANN_INCLUDE}")

    # The Macrium codebase uses some constructs (string literal -> uint8_t*,
    # narrowing conversions) that clang flags. Silence them rather than rewrite.
    add_compile_options(-w -fpermissive)

    set(MRIMGX_POSIX_INCLUDES
        ${NLOHMANN_INCLUDE}
        ${ZSTD_INCLUDE}
        ${OPENSSL_INCLUDE_DIR}
        CACHE INTERNAL "third-party include dirs for POSIX build"
    )
    set(MRIMGX_POSIX_ZSTD_LIB  ${ZSTD_LIB}        CACHE INTERNAL "zstd library for POSIX build")
endif()


# --- Cross-platform libraries (always built) ---
add_subdirectory("libs/encryption")
add_subdirectory("libs/file_operations")
add_subdirectory("libs/file_reader")
add_subdirectory("libs/restore")


# --- Windows-only: VHDX manager + img_to_vhdx ---
# These rely on virtdisk.dll (CreateVirtualDisk / AttachVirtualDisk), which is
# a Windows-specific API and is not implemented by Wine.
if (WIN32)
    add_subdirectory("libs/vhdx_manager")
    add_subdirectory("img_to_vhdx")
endif()


# --- POSIX equivalent: img_to_raw ---
# Writes a sparse raw disk image instead of a VHDX. Inspect with Sleuth Kit.
if (NOT WIN32)
    add_subdirectory("img_to_raw")
endif()
