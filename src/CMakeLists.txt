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

project ("img_to_vhdx")

# Include sub-projects.
add_subdirectory ("img_to_vhdx")

add_subdirectory("libs/encryption")


add_subdirectory("libs/file_operations")

add_subdirectory("libs/file_reader")

add_subdirectory("libs/restore")

add_subdirectory("libs/vhdx_manager")
