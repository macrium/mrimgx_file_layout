#pragma once
#include "file_structs.h"

/**
 * Reads file metadata from a Macrium Reflect X backup file.
 *
 * @param fileHandle A handle to the Macrium Reflect X backup file from which to read the metadata.
 * @param strJSON A reference to a string where the JSON metadata will be stored.
 */
void readFileMetadataData(const file_structs::fileLayout& backupLayout, std::fstream* fileHandle, std::string& strJSON);

/**
 * Reads disk metadata from a Macrium Reflect X backup file.
 *
 * @param fileHandle A handle to the Macrium Reflect X backup file from which to read the metadata.
 * @param Disk A reference to a DiskLayout object where the disk metadata will be stored.
 */
void readDiskMetadata(const file_structs::fileLayout& parsedFileLayout, std::fstream* fileHandle, file_structs::Disk::DiskLayout& Disk);


/**
 * Reads disk metadata from a Macrium Reflect X backup file.
 *
 * This function reads the metadata block by block until it finds the BITMAP_HEADER or IDX_HEADER blocks.
 * It then reads these blocks to validate the data. For IDX_HEADER, it repositions the file pointer to the end of the block.
 *
 * @param backupLayout A reference to the file layout structure containing encryption and other metadata.
 * @param fileHandle A handle to the Macrium Reflect X backup file from which to read the metadata.
 */
void readPartitionMetadataData(const file_structs::fileLayout& backupLayout, std::fstream* fileHandle);