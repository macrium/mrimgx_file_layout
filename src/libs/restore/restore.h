#pragma once
/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.

The program uses the following libraries:
- nlohmann JSON library for JSON data extraction.
  License: [MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)
- OpenSSL library for decryption.
  License:  [Apache License 2.0](https://www.openssl.org/source/license.html)
- Zlib library, a dependency of OpenSSL.
  License: [Zlib License](https://zlib.net/zlib_license.html)
- ztd compression library for decompression.
  License: [Boost Software License](https://github.com/facebook/zstd/blob/dev/LICENSE.)
===============================================================================
*/

/**
 * @file
 * @brief This is the primary include file for this library.
 *
 * Including this file in a project gives access to all the functionality provided by this library.
 * No other include files are necessary.
 */

 /**
 * @brief Type alias for a callback function used to report progress during disk restoration.
 *
 * This callback function is invoked periodically during the disk restoration process to report the progress.
 * It takes three parameters: the total number of bytes to be processed, the number of bytes processed so far,
 * and the time point of the last update. The function does not return a value.
 */
using ProgressCallback = std::function<void(const uint64_t totalBytes, uint64_t& bytesSoFar, std::chrono::steady_clock::time_point& lastUpdateTime)>;

/**
 * @brief Restores a disk from a backup file.
 *
 * This function restores a disk from a backup file. It takes the path to the backup file, a password, a target disk, and an optional progress callback function as parameters.
 *
 * @param filePath The path to the backup file.
 * @param password The password for the backup file.
 * @param targetDisk The path to the target disk where the backup is being restored.
 * @param diskNumber The user provided disk number to restore. -1 if not supplied.
 * @param outputProgress An optional callback function to output the progress of the restoration process. Default is nullptr.
 */

void restoreDisk(const std::wstring& filePath, const std::string& password, const std::wstring targetDisk, int diskNumber, bool keepDiskId, ProgressCallback outputProgress = nullptr);
