# img_to_raw: POSIX (macOS/Linux) equivalent of img_to_vhdx.
# Writes a sparse raw disk image instead of using virtdisk.dll's CreateVirtualDisk.
add_executable(img_to_raw "img_to_raw.cpp")
target_include_directories(img_to_raw PRIVATE ../libs ${MRIMGX_POSIX_INCLUDES})
target_link_libraries(img_to_raw PRIVATE restore file_reader file_operations encryption)
