#pragma once
/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.

This file defines a set of functions for reading data from a Macrium Reflect X
backup file. These functions include reading the header offset and magic data,
reading a metadata block, and reading the JSON data from the backup file. They
handle various aspects such as checking for encryption and compression flags,
decompressing data if necessary, and computing and verifying the MD5 hash of
the data. These functions provide a high-level interface for accessing the
contents of a Macrium Reflect X backup file.

The library uses the following external libraries:
- nlohmann JSON library for JSON data extraction.
  License: [MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)
- ztd compression library for decompression.
  License: [Boost Software License](https://github.com/facebook/zstd/blob/dev/LICENSE.)
===============================================================================
*/

/**
 * @file
 * @brief This is the primary header file for the Macrium Reflect X backup file reader library.
 *
 * This library provides a set of cross-platform functions for reading and interpreting data from a Macrium Reflect X backup file.
 * It handles various aspects such as reading the header offset and magic data, reading metadata blocks, and reading the JSON data from the backup file.
 * It also checks for encryption and compression flags, decompresses data if necessary, and computes and verifies the MD5 hash of the data.
 *
 * Including this file in a project gives access to the entire functionality provided by this library,
 * including the data structures used to represent the backup file and the functions used to read and interpret the backup file.
 * No other include files are necessary.
 */

#include "file_structs.h"
#include "backup_set.h"

/**
 * Reads a Macrium Reflect X backup file and parses its JSON data.
 *
 * @param filename The name of the file to read.
 * @throws std::runtime_error if an error occurs during reading or parsing.
 */
void readBackupFile(const std::wstring& filename, file_structs::fileLayout& backupLayout, const std::string password, bool loadIndex = true);


/**
 * @brief Selects a disk to restore from the backup layout based on the provided disk number.
 *
 * This function selects a disk to restore from the backup layout based on the provided disk number.
 * If the disk number is -1, it selects the first disk in the backup layout.
 * If the disk number is not -1, it loops through all the disks in the backup layout and selects the disk with the matching disk number.
 * If no disk with the matching disk number is found, it throws a runtime error.
 *
 * @param backupLayout The layout of the backup.
 * @param diskNumber The disk number of the disk to restore. If -1, the first disk in the backup layout is selected.
 * @param diskToRestore The selected disk to restore.
 * @throws std::runtime_error if no disk with the matching disk number is found.
 */
void getDiskToRestoreFromDiskNumber(const file_structs::fileLayout& backupLayout, int& diskNumber, file_structs::Disk::DiskLayout& diskToRestore);

