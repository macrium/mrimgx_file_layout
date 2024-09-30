#include "pch.h"
#include "metadata_block_reader.h"
#include "file_reader.h"
/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.

This file defines a set of cross-platform functions for reading data from a
Macrium Reflect X backup file. These functions include reading the header offset
and magic data, reading a metadata block, and reading the JSON data from the
backup file. They handle various aspects such as checking for encryption and
compression flags, decompressing data if necessary, and computing and verifying
the MD5 hash of the data. These functions provide a high-level interface for
accessing the contents of a Macrium Reflect X backup file.

Using cross-platform functions is important as it allows the code to be portable
and run on different operating systems without requiring system-specific
implementations. This increases the maintainability and scalability of the code,
making it more robust and versatile for various use cases and environments.

Zstandard is dual-licensed under BSD and GPLv2. For a complete
description, please see https://github.com/facebook/zstd/blob/dev/LICENSE.
===============================================================================
*/

#include "..\file_operations\file_operations.h"
#include "metadataheader.h"
#include "file_structs.h"


/**
 * Reads the header offset and magic data from a Macrium Reflect X backup file.
 *
 * The header offset is read into `llHeaderOffset` and the magic data is read into `Magic`.
 *
 * @param hFile The handle to the Macrium Reflect X backup file.
 * @param llHeaderOffset A reference to a uint64_t that will receive the header offset.
 * @param Magic A reference to a vector of uint8_t that will receive the magic data.
 */
void readHeaderOffsetAndMagicData(std::fstream* fileHandle, uint64_t& llHeaderOffset, std::vector<uint8_t>& Magic) {
	readFile(fileHandle, &llHeaderOffset, sizeof(llHeaderOffset));
	readFile(fileHandle, Magic.data(), MAGIC_BYTES_VX_SIZE);
}


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
void getDiskToRestoreFromDiskNumber(const file_structs::fileLayout& backupLayout, int& diskNumber, file_structs::Disk::DiskLayout& diskToRestore)

{
	if (diskNumber == -1) {
		diskToRestore = backupLayout.disks[0];
		diskNumber = diskToRestore._header.disk_number;
		return;
	}

	for (auto& disk : backupLayout.disks) {
		if (disk._header.disk_number == diskNumber) {
			diskToRestore = disk;
			return;
		}
	}

	throw std::runtime_error("Invalid disk number.");
}


/**
 * Calculates the offset to the end of a Macrium Reflect X backup file.
 *
 * This function calculates the offset by subtracting the size of the magic bytes (MAGIC_BYTES_VX_SIZE)
 * and the size of a int64_t from 0. The result is a negative number, which represents an offset from the end of the file.
 *
 * @return A LARGE_INTEGER that contains the calculated offset.
 */
std::streamoff calculateOffset() {
	constexpr int64_t magicBytesSize = static_cast<signed int>(MAGIC_BYTES_VX_SIZE);
	constexpr int64_t int64_tSize = static_cast<signed int>(sizeof(int64_t));
	constexpr int64_t offset = -((int64_t)magicBytesSize + int64_tSize);
	std::streamoff fileOffset = offset;
	return fileOffset;
}

/**
 * Checks the magic data of a Macrium Reflect X backup file.
 *
 * This function compares the provided magic data with the expected magic bytes (MAGIC_BYTES_VX).
 * If the magic data does not match the expected magic bytes, it throws a std::runtime_error.
 *
 * @param magicData A vector of uint8_t that contains the magic data to check.
 * @throws std::runtime_error if the magic data does not match the expected magic bytes.
 */
void checkMagicData(const std::vector<uint8_t>& magicData) {
	if (memcmp(magicData.data(), MAGIC_BYTES_VX, MAGIC_BYTES_VX_SIZE) != 0) {
		throw std::runtime_error("Invalid file: not a Macrium Reflect vX file.");
	}
}

/**
 * Parses a string of JSON data into a nlohmann::json object.
 *
 * This function uses the nlohmann::json::parse function to parse the input string.
 * If the parsed JSON is null, it throws a std::runtime_error.
 *
 * @param jsonStr The string of JSON data to parse.
 * @return A nlohmann::json object that represents the parsed JSON data.
 * @throws std::runtime_error if the parsed JSON is null.
 */
nlohmann::json parseJsonData(const std::string& jsonStr) {
	nlohmann::json j = nlohmann::json::parse(jsonStr);
	if (j.is_null()) {
		throw std::runtime_error("Parsed JSON is null.");
	}
	return j;
}

/**
 * Validates the password for a Macrium Reflect X backup file.
 *
 * This function checks if encryption is enabled for the backup file. If encryption is enabled,
 * it converts the password to UTF-8, derives the encryption key from the password, computes the HMAC of the derived key,
 * and compares it with the HMAC stored in the backup file. If the HMACs do not match, it throws a std::runtime_error.
 *
 * @param backupLayout A reference to a fileLayout object that represents the layout of the backup file.
 * @param password The password to validate.
 * @throws std::runtime_error if the password is invalid.
 */
void validatePassword(file_structs::fileLayout& backupLayout, const std::string& password) {
	if (!backupLayout._encryption.enable) {
		return;
	}
	getDerivedKey(backupLayout._header.imageid_binary, password, backupLayout._encryption.key_iterations, backupLayout._encryption.derived_key);
	auto passwordHmac = getKeyHMAC(backupLayout._encryption.derived_key);
	if (passwordHmac != backupLayout._encryption.hmac_binary) {
		throw std::runtime_error("Invalid password.");
	}
}

/*
 * Reads a Macrium Reflect X backup file and parses its JSON data.
 *
 * This function opens the file, reads the header offset and magic data, checks the magic data,
 * sets the file pointer to the header offset, reads the JSON data from the file, and parses the JSON data.
 *
 * @param filename The name of the file to read.
 * @throws std::runtime_error if an error occurs during reading or parsing.
 */
void readBackupFile(const std::wstring& filename, file_structs::fileLayout& backupLayout,const std::string password, bool loadIndex /* = true */) {
	// Open the file and get a handle to it
	auto fileGuard = openFileWithGuard(filename, true);
	std::fstream* fileHandle = fileGuard.get();

	// Calculate the offset to the end of the file
	std::streamoff fileOffset = calculateOffset();

	// Set the file pointer to the end of the file
	setFilePointer(fileHandle, fileOffset, std::ios::end);

	uint64_t headerOffset;
	std::vector<uint8_t> magicData(MAGIC_BYTES_VX_SIZE);
	// Read the header offset and magic data from the file
	readHeaderOffsetAndMagicData(fileHandle, headerOffset, magicData);

	// Check the magic data
	checkMagicData(magicData);

	// Set the file pointer to the header offset
	fileOffset = headerOffset;
	setFilePointer(fileHandle, fileOffset, std::ios::beg);

	// Read the JSON data from the file
	std::string jsonStr;
	readFileMetadataData(backupLayout, fileHandle, jsonStr);
	// Parse the JSON data
	nlohmann::json j = parseJsonData(jsonStr);
	// Convert the parsed JSON data to a fileLayout object
	backupLayout = j;
	// Store the JSON string in the backupLayout object for future reference
	backupLayout.jsonStr = jsonStr;

	// Set the filename in the backupLayout object
	backupLayout.file_name = filename;

	// validate password
	if (loadIndex && backupLayout._encryption.enable) {
		backupLayout._header.set_imageid_binary();
		backupLayout._encryption.convert_hmac_to_binary();
		validatePassword(backupLayout, password);
	}

	fileOffset = backupLayout._header.index_file_position;
	setFilePointer(fileHandle, fileOffset, std::ios::beg);

	// Check if the file is not split. 
	// split files have no data blocks, so we don't need to read them
	if (false == backupLayout._header.split_file) {
		// Iterate over each disk in the parsed file layout
		for (auto& disk : backupLayout.disks) {
			// Read the disk metadata
			readDiskMetadata(backupLayout, fileHandle, disk);
			// Iterate over each partition in the disk
			for (auto& partition : disk.partitions) {
				// Skip the partition metadata as it isn't used for this restore
				readPartitionMetadataData(backupLayout, fileHandle);
				int32_t blockCount;
				// Read the block count
				readFile(fileHandle, &blockCount, sizeof(blockCount));
				// If block count is greater than zero
				if (blockCount > 0) {
					// Resize the reserved sectors blocks vector to the block count
					partition.reserved_sectors_blocks.resize(blockCount);
					// Read the data blocks into the reserved sectors blocks vector
					readFile(fileHandle, partition.reserved_sectors_blocks.data(), sizeof(DataBlockIndexElement) * blockCount);
				}
				// Read the block count again
				readFile(fileHandle, &blockCount, sizeof(blockCount));
				// If block count is greater than zero
				if (blockCount > 0) {
					bool isdelta = backupLayout._header.delta_index;
					if (loadIndex) {
						if (isdelta) {
							// Resize the delta data blocks vector to the block count
							partition.delta_data_blocks.resize(blockCount);
							// Read the delta data blocks into the data blocks vector
							readFile(fileHandle, partition.delta_data_blocks.data(), sizeof(DeltaDataBlock) * blockCount);
						}
						else {
							// Resize the data blocks vector to the block count
							partition.data_blocks.resize(blockCount);
							// Read the data blocks into the data blocks vector
							readFile(fileHandle, partition.data_blocks.data(), sizeof(DataBlockIndexElement) * blockCount);

						}
					}  
					else {
						// Skip the data blocks
						fileOffset = isdelta ? sizeof(DeltaDataBlock) * blockCount : sizeof(DataBlockIndexElement) * blockCount;
						setFilePointer(fileHandle, fileOffset, std::ios::cur);
					}
				}
			}
		}
	}
}


