#include "pch.h"
#include "..\file_operations\file_operations.h"
#include ".\zstd\zstd.h"
#include "..\encryption\encryption.h"
#include "file_reader.h"
#include "metadata_block_reader.h"
#include "metadataheader.h"

/**
 * @file
 * @brief This file contains the implementation of the Macrium Reflect X backup
 * file reader library, specifically for reading and interpreting metadata blocks.
 *
 * This library provides a set of cross-platform functions for reading metadata
 * blocks from a Macrium Reflect X backup file. It handles various aspects such
 * as reading the header offset and magic data, reading metadata blocks, and
 * reading the JSON data from the backup file. It also checks for encryption and
 * compression flags, decompresses data if necessary, decrypts data when required,
 * and computes and verifies the MD5 hash of the data.
 *
 * The functions in this file are implemented in a way that they can be used on
 * different operating systems without requiring system-specific implementations.
 * This makes the code more maintainable and scalable, and allows it to be used
 * in a wider range of environments.
 */


 /**
  * @brief Reads a metadata block from a Macrium Reflect X backup file.
  *
  * The size of the block is determined by `Header.block_Length`.
  * If the block is compressed, it is decompressed using Zstandard (ZSTD) decompression.
  * If the block is encrypted, it is decrypted using AES decryption in ECB mode.
  *
  * @param Header A reference to a `MetadataBlockHeader` object that contains
  * the block's metadata.
  * @param hFile A handle to the Macrium Reflect X backup file from which to
  * read the block.
  * @return A `std::unique_ptr` to an array of unsigned chars that contains the
  * block data. If the block length is 0, or if an error occurs during reading
  * or decompression, it returns `nullptr`.
  */

std::unique_ptr<unsigned char[]> readBlock(const file_structs::fileLayout& backupLayout, MetadataBlockHeader& Header, std::fstream* fileHandle)
{
	// If the block length is 0, return nullptr
	if (Header.block_Length == 0) {
		return nullptr;
	}

	// Allocate a buffer to read the block
	std::unique_ptr<unsigned char[]> readBuffer = std::make_unique<unsigned char[]>(Header.block_Length);

	// Read the block from the file
	readFile(fileHandle, readBuffer.get(), Header.block_Length);

	// Compute the MD5 hash of the decompressed data
	auto computedHash = computeMD5Hash(readBuffer.get(), Header.block_Length);


	// If the computed hash does not match the expected hash, throw an exception
	if (memcmp(computedHash.data(), Header.hash, sizeof(Header.hash)) != 0) {
		throw std::runtime_error("Block hash mismatch.");
	}
	
	// If the block is encrypted, decrypt it using AES decryption in ECB mode.
	// The AES variant and derived key are obtained from the backup layout.
	// The decrypted data replaces the original data in the read buffer.
	if (Header.flags.encryption) {
		int aes_variant = backupLayout._encryption.getAESValue();
		decryptDataWithAESECB(aes_variant, backupLayout._encryption.derived_key.data(), readBuffer.get(), Header.block_Length);
	}

	// If the block is compressed, decompress it
	if (Header.flags.compression) {
		// Get the size of the compressed block
		auto compressedsize = ZSTD_findFrameCompressedSize(readBuffer.get(), Header.block_Length);
		// Get the size of the decompressed block
		auto decompressedsize = ZSTD_getFrameContentSize(readBuffer.get(), Header.block_Length);

		// Allocate a buffer for the decompressed data
		std::unique_ptr<unsigned char[]> outBuffer = std::make_unique<unsigned char[]>(decompressedsize);

		// Decompress the block
		size_t zout = ZSTD_decompress(outBuffer.get(), (size_t)decompressedsize, readBuffer.get(), compressedsize);

		// If decompression failed, throw an exception
		if (ZSTD_isError(zout)) {
			throw std::runtime_error("Failed to decompress block.");
		}

		// update the block length to the decompressed size
		Header.block_Length = (uint32_t)decompressedsize;

		// Replace the read buffer with the decompressed data
		readBuffer = std::move(outBuffer);
	}

	// Return the read (and possibly decompressed) data
	return readBuffer;
}


/**
 * @brief Reads disk metadata from a Macrium Reflect X backup file.
 *
 * This function reads the metadata block by block until it finds the
 * TRACK_0 and EXT_PAR_TABLE blocks. It then reads these blocks and
 * assigns their contents to the `Disk` object.
 *
 * @param fileHandle A handle to the Macrium Reflect X backup file from
 * which to read the metadata.
 * @param Disk A reference to a DiskLayout object where the disk metadata
 * will be stored.
 */
void readDiskMetadata(const file_structs::fileLayout& backupLayout, std::fstream* fileHandle, file_structs::Disk::DiskLayout& Disk) {
	std::string strJSON;
	MetadataBlockHeader Header;
	bool track0Found = false; // Flag to track if TRACK_0 is found

	// Loop until the last block is found
	do {
		// Read the header of the next block
		readFile(fileHandle, &Header, sizeof(Header));

		// If the block name is TRACK_0
		if (memcmp(Header.block_name, TRACK_0, BLOCK_NAME_LENGTH) == 0) {
			if (Header.block_Length > 0) {
				auto blockData = readBlock(backupLayout, Header, fileHandle);

				// If reading the block failed, throw an exception
				if (blockData == nullptr) {
					throw std::runtime_error("Failed to read TRACK_0 block.");
				}

				// Assign the block data to the track0 field of the Disk object
				Disk.track0.assign(blockData.get(), blockData.get() + Header.block_Length);
				track0Found = true; // Set the flag to true
			}
		}
		else if (memcmp(Header.block_name, EXT_PAR_TABLE, BLOCK_NAME_LENGTH) == 0) {
			// If the block name is EXT_PAR_TABL
			if (Header.block_Length > 0) {
				// Read the block
				auto blockData = readBlock(backupLayout, Header, fileHandle);

				// If reading the block failed, throw an exception
				if (blockData == nullptr) {
					throw std::runtime_error("Failed to read EXT_PAR_TABLE block.");
				}

				// Get a pointer to the start of the data in the block
				unsigned char* pDataStart = blockData.get();
				pDataStart += sizeof(uint32_t);

				// Calculate the number of ExtendedPartition objects in the block
				uint32_t extendedPartitionCount = (Header.block_Length - sizeof(uint32_t)) / sizeof(ExtendedPartition);

				// For each ExtendedPartition object in the block
				for (uint32_t i = 0; i < extendedPartitionCount; i++) {
					// Create an ExtendedPartition object
					ExtendedPartition extendedPartition;

					// Copy the data from the block to the ExtendedPartition object
					memcpy(&extendedPartition, pDataStart, sizeof(ExtendedPartition));

					// Add the ExtendedPartition object to the extendedPartitions field of the Disk object
					Disk.extendedPartitions.push_back(extendedPartition);

					// Move the data pointer to the next ExtendedPartition object in the block
					pDataStart += sizeof(ExtendedPartition);
				}
			}
		}
		else {
			// Move the file pointer to the next block
			setFilePointer(fileHandle, Header.block_Length, std::ios::cur);
		}
	} while (Header.flags.last_block  == 0);

	// Check if TRACK_0 was found
	if (!track0Found) {
		throw std::runtime_error("TRACK_0 block not found.");
	}

	return;
}



/**
 * Reads file metadata from a Macrium Reflect X backup file.
 *
 * This function reads the metadata block by block until it finds the JSON_HEADER block.
 * It then reads this block and assigns its contents to the `strJSON` string.
 *
 * @param fileHandle A handle to the Macrium Reflect X backup file from which to read the metadata.
 * @param strJSON A reference to a string where the JSON metadata will be stored.
 */
void readFileMetadataData(const file_structs::fileLayout& backupLayout, std::fstream* fileHandle, std::string& strJSON) {
	MetadataBlockHeader Header;
	bool jsonHeaderFound = false; // Flag to track if JSON_HEADER is found


	// Loop until the last block is found
	do {
		// Read the header of the next block
		readFile(fileHandle, &Header, sizeof(Header));

		// If the block name is JSON_HEADER
		if (memcmp(Header.block_name, JSON_HEADER, BLOCK_NAME_LENGTH) == 0) {
			// Read the block
			auto blockData = readBlock(backupLayout, Header, fileHandle);

			// If reading the block failed, throw an exception
			if (blockData == nullptr) {
				throw std::runtime_error("Failed to read JSON_HEADER block.");
			}
			// Assign the block data to the strJSON string
			strJSON.assign(reinterpret_cast<const char*>(blockData.get()), Header.block_Length);
			jsonHeaderFound = true; // Set the flag to true
		}
		else {
			// Move the file pointer to the next block
			setFilePointer(fileHandle, Header.block_Length, std::ios::cur);
		}
	} while (Header.flags.last_block  == 0);

	// Check if JSON_HEADER was found
	if (!jsonHeaderFound) {
		throw std::runtime_error("JSON_HEADER block not found.");
	}

	return;
}

/**
 * Reads disk metadata from a Macrium Reflect X backup file.
 *
 * This function reads the metadata block by block until it finds the BITMAP_HEADER and/or IDX_HEADER blocks.
 * It then reads these blocks to validate the data. For IDX_HEADER, it repositions the file pointer to the end of the block.
 *
 * @param backupLayout A reference to the file layout structure containing encryption and other metadata.
 * @param fileHandle A handle to the Macrium Reflect X backup file from which to read the metadata.
 */
void readPartitionMetadataData(const file_structs::fileLayout& backupLayout, std::fstream* fileHandle) {
	MetadataBlockHeader Header;
	bool idxHeaderFound = false; // Flag to track if IDX_HEADER is found

	// Loop until the last block is found
	do {
		// Read the header of the next block
		readFile(fileHandle, &Header, sizeof(Header));

		// If the block name is BITMAP_HEADER
		if (memcmp(Header.block_name, BITMAP_HEADER, BLOCK_NAME_LENGTH) == 0) {
			// Read the block to validate the data
			// Do nothing with the block data if it exists
			auto blockData = readBlock(backupLayout, Header, fileHandle);
		}
		// IDX_HEADER is a special case where we just need to validate the block data
		// as we're reading the block indexes directly in file_reader.cpp - readBackupFile
		else if (memcmp(Header.block_name, IDX_HEADER, BLOCK_NAME_LENGTH) == 0) {
			// Read the block to validate the hash
			// Do nothing with the block data.  
			auto blockData = readBlock(backupLayout, Header, fileHandle);
			// Now reposition the file pointer to the end of the IDX_HEADER block
			// The blockData is processed in file_reader.cpp - readBackupFile
			// Note: IDX_HEADER is always the last block.
			setFilePointer(fileHandle, -static_cast<std::streamoff>(Header.block_Length), std::ios::cur);
			idxHeaderFound = true;
		}
		else {
			// Move the file pointer to the next block
			setFilePointer(fileHandle, Header.block_Length, std::ios::cur);
		}
	} while (Header.flags.last_block == 0);

	// Check if IDX_HEADER was found
	if (!idxHeaderFound) {
		throw std::runtime_error("IDX_HEADER block not found.");
	}

	return;
}


