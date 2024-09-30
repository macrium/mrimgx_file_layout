# CMakeList.txt : CMake project for img_to_vhdx, include source and define
# project specific logic here.
#

# Specify the library directories
link_directories($<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/dependencies/lib/openssl/debug>)
link_directories($<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/dependencies/lib/openssl/release>)

link_directories($<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/dependencies/lib/zstd/debug>)
link_directories($<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/dependencies/lib/zstd/release>)

link_directories($<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/dependencies/lib/zlib/debug>)
link_directories($<$<CONFIG:Release>:${PROJECT_SOURCE_DIR}/dependencies/lib/zlib/release>)

# Add source to this project's executable.
add_executable (img_to_vhdx "img_to_vhdx.cpp" "arg_validator.cpp" "arg_validator.h" "console_print.cpp" "console_print.h" "img_to_vhdx.cpp" "vhdx_handler.cpp" "vhdx_handler.h")

include_directories(../dependencies/include)
 
# Link the libraries to your target
target_link_libraries(img_to_vhdx encryption file_operations file_reader restore vhdx_manager)

SET_TARGET_PROPERTIES(img_to_vhdx PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET img_to_vhdx PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
