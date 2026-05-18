add_library(file_reader STATIC "file_reader.cpp" "file_reader.h" "backup_set.cpp" "backup_set.h" "disk_structs.h" "enums.h" "enums_to_text.cpp" "file_structs.h" "framework.h" "metadata_block_reader.cpp" "metadata_block_reader.h" "metadataheader.h" "pch.cpp" "pch.h")

if (WIN32)
    include_directories(../../dependencies/include)
else()
    target_include_directories(file_reader PUBLIC ${MRIMGX_POSIX_INCLUDES})
    target_link_libraries(file_reader PUBLIC file_operations encryption ${MRIMGX_POSIX_ZSTD_LIB})
endif()
