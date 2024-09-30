// restore.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include ".\zstd\zstd.h"
#include "..\file_reader\file_reader.h"
#include "..\file_operations\file_operations.h"
#include "..\encryption\encryption.h"
#include "crc32.h"
#include "restore.h"

/**
 * Struct: EncryptionIVParams
 * These parameters are used to generate a unique Initialization Vector (IV) for each block of data to be encrypted.
 * Fields:
 * - imageid: A pointer to the image ID used in IV generation.
 * - disk: The disk number used in IV generation.
 * - partition: The partition number used in IV generation.
 * - index: The block index used in IV generation.
 */
struct EncryptionIVParams
{
	const uint8_t* imageid;  // Pointer to the image ID
	uint16_t disk_number;     // Disk number
	uint16_t partition_number; // Partition number
	uint32_t index;    // Block index
	void Set(const file_structs::Header& header, const file_structs::Disk::DiskLayout& disk, const file_structs::Partition::PartitionLayout& partition, uint32_t _index) {
		imageid = &header.imageid_binary[0];
		disk_number = disk._header.disk_number;
		partition_number = partition._header.partition_number;
		index = _index;
	}
};

/**
 * @brief Sets a new disk ID for the provided disk.
 *
 * This function checks if the disk format is GPT or MBR and sets a new unique identifier accordingly.
 * For GPT, it creates a new GUID and assigns it to the disk_guid field of the GPT header.
 * For MBR, it generates a new disk ID by getting the current tick count and writes it into the boot code of the MBR.
 *
 * @param disk The disk for which to set a new ID.
 */
void setNewDiskID(file_structs::Disk::DiskLayout& disk)
{
	// Check if the disk format is GPT
	bool isGPT = disk._header.disk_format == ImageEnums::DiskFormat::eGPT;
	uint16_t bytesPerSector = disk._geometry.bytes_per_sector;

	if (isGPT) {
		// If the disk is GPT, the header is at sector 1
		gpt_header* gptHeader = reinterpret_cast<gpt_header*>(disk.track0.data() + bytesPerSector);

		// Create a new GUID to prevent a disk collision
		GUID newDiskGuid;
		if (CoCreateGuid(&newDiskGuid) == S_OK) {
			// Assign the new GUID to the disk
			memcpy(&gptHeader->disk_guid, &newDiskGuid, sizeof(newDiskGuid));

			// Before calculating the new CRC32 checksum, the existing checksum in the header must be set to 0.
			gptHeader->header_crc32 = 0;

			// Calculate the CRC32 checksum for the GPT header and store it in the header.
			// The checksum is calculated over the entire header, with the checksum field itself set to 0.
			// GPT disks are not loaded if the checksum is incorrect.
			gptHeader->header_crc32 = Calculate_CRC32((unsigned char*)gptHeader, gptHeader->header_size, sizeof(gpt_header));
		}
	}
	else {
		// If the disk is not GPT, it's MBR. Create a new disk ID to prevent a disk collision
		BootRecord* bootRecord = reinterpret_cast<BootRecord*>(disk.track0.data());
		// Create a random device
		std::random_device rd;
		// Create a distribution that spans the full range of uint32_t
		std::uniform_int_distribution<uint32_t> dist;
		// Generate a random 32-bit number
		uint32_t randomDiskId = dist(rd);
		// Assign the new disk ID to the boot code of the MBR
		memcpy(&bootRecord->bootCode[440], &randomDiskId, sizeof(uint32_t));
	}
}

/**
 * @brief Reads a block of data from a file, decrypts it if necessary, and decompresses it if it is compressed.
 *
 * @param fileHandle The handle to the file from which to read the block.
 * @param block The block to read.
 * @param isCompressed A flag indicating whether the block is compressed.
 * @return A unique_ptr to the read (and possibly decrypted and decompressed) data.
 */
std::unique_ptr<unsigned char[]> readBlock(file_structs::fileLayout& backupLayout, const EncryptionIVParams & ivParams, std::fstream* fileHandle, DataBlockIndexElement& block)
{
	if (block.block_length == 0) {
		return nullptr;
	}
	// Allocate a buffer to read the block
	std::unique_ptr<unsigned char[]> readBuffer = std::make_unique<unsigned char[]>(block.block_length);

	std::streamoff fileOffset = block.file_position;
	setFilePointer(fileHandle, fileOffset, std::ios::beg);
	readFile(fileHandle, readBuffer.get(), block.block_length);
	if (backupLayout._encryption.aes_type != ImageEnums::AES::eNone) {
		int aes_variant = backupLayout._encryption.getAESValue();
		uint8_t iv[16];
		formatInitializationVectorForAES(backupLayout._header.imageid_binary, ivParams.disk_number, ivParams.partition_number, ivParams.index, backupLayout._encryption.key_iterations, backupLayout._encryption.derived_key, iv);
		decryptDataWithAESCBC(aes_variant, backupLayout._encryption.derived_key.data(), iv, readBuffer.get(), block.block_length);
	}
	if (backupLayout._compression.compression_level != ImageEnums::CompressionType::eNone) {
		// Get the size of the compressed block
		auto compressedsize = ZSTD_findFrameCompressedSize(readBuffer.get(), block.block_length);
		// Get the size of the decompressed block
		auto decompressedsize = ZSTD_getFrameContentSize(readBuffer.get(), block.block_length);
		// Allocate a buffer for the decompressed data
		std::unique_ptr<unsigned char[]> decompressedBuffer = std::make_unique<unsigned char[]>(decompressedsize);

		// Decompress the block
		auto zout = ZSTD_decompress(decompressedBuffer.get(), (size_t)decompressedsize, readBuffer.get(), compressedsize);

		// If decompression failed, throw an exception
		if (ZSTD_isError(zout)) {
			throw std::runtime_error("Failed to decompress block.");
		}

		// Update the block length to the decompressed size
		block.block_length = static_cast<uint32_t>(decompressedsize);

		// Compute the MD5 hash of the decompressed data
		auto computedHash = computeMD5Hash(decompressedBuffer.get(), decompressedsize);

		// If the computed hash does not match the expected hash, throw an exception
		if (memcmp(computedHash.data(), block.md5_hash, sizeof(block.md5_hash)) != 0) {
			throw std::runtime_error("Block hash mismatch.");
		}

		// Replace the read buffer with the decompressed data
		readBuffer = std::move(decompressedBuffer);
	}
	// Return the read (and possibly decompressed) data
	return readBuffer;
}


/**
 * @brief Calculates the total size of the backup.
 *
 * This function calculates the total size of the backup by summing up the lengths of all data blocks and reserved sector blocks in all partitions of all disks in the backup layout.
 *
 * @param fileLayout The layout of the backup.
 * @return The total size of the backup in bytes.
 */
uint64_t calculateTotalBytes(file_structs::fileLayout& fileLayout)
{
	uint64_t totalBytes = 0;
	for (auto& disk : fileLayout.disks) {
		for (auto& partition : disk.partitions) {
			totalBytes += partition._file_system.reserved_sectors_byte_length;
			for (auto& block : partition.data_blocks) {
				if (block.block_length)
					totalBytes += partition._header.block_size;
			}
		}
	}
	return totalBytes;
}




/**
 * @brief Restores a disk from a backup file.
 *
 * This function performs the following steps:
 * 1. Opens the target disk file.
 * 2. Reads the backup file layout.
 * 3. Creates a backup set, which includes building an index and creating a map of file handles.
 * 4. Calculates the total size of the backup.
 * 5. Sets the file pointer to the beginning of the backup file.
 * 6. Selects the disk for restoration based on the provided disk number.
 * 7. Optionally sets a new disk ID to prevent a disk collision.
 * 8. Writes the track0 data to the target disk.
 * 9. If the disk format is MBR, restores the extended partition and logical drive boot records.
 * 10. Loops through each partition in the disk to restore.
 * 11. For each partition, writes FAT32 reserved sectors to the target disk.
 * 12. Loops through each data block in the partition.
 * 13. For each data block, reads the block data from the backup file.
 * 14. Writes the block data to the target disk.
 * 15. Outputs the progress of the restoration process.
 *
 * @param filePath The path to the backup file.
 * @param password The password for the backup file.
 * @param targetDiskHandle The handle to the target disk where the backup is being restored.
 * @param diskNumber The user provided disk number to restore. -1 if not supplied.
 * @param keepDiskId A flag indicating whether to keep the disk ID the same or set a new one.
 * @param outputProgress A callback function to output the progress of the restoration process. Default is nullptr.
 */
void restoreDisk(const std::wstring& filePath, const std::string& password,const std::wstring targetDisk, int diskNumber, bool keepDiskId, ProgressCallback outputProgress/*= nullptr*/)
{
	auto targetDiskFileStream = openFileWithGuard(targetDisk, false);
	auto targetDiskHandle = targetDiskFileStream.get();
	BackupSet backupSet;
	{
		file_structs::fileLayout backupLayout;
		readBackupFile(filePath, backupLayout, password);

		// Read the backup set, build the index and create a map of file handles 
		createBackupSet(backupSet, filePath, password, backupLayout._header.imageid);
	}
	// Get the backup layout for the first file in the backup set.
	// This is the layout of the backup file to restore with the index built from the backup set.
	file_structs::fileLayout& backupLayout = backupSet.getBackupFileWithFullIndex();

	// Get the backup file handle from the backup set
	std::fstream* backupFileHandle = backupSet.getFileHandle(backupLayout._header.file_number);

	// Calculate the total size of the backup
	auto totalBackupSize = calculateTotalBytes(backupLayout);

	uint64_t bytesProcessed = 0;
	std::streamoff  fileOffset = 0;

	// Set the file pointer to the beginning of the backup file
	setFilePointer(backupFileHandle, fileOffset, std::ios::beg);

	// Select the disk for restoration based on the provided disk number
	// diskNumber is -1 if not supplied
	file_structs::Disk::DiskLayout diskToRestore;
	getDiskToRestoreFromDiskNumber(backupLayout, diskNumber, diskToRestore);

	// A new disk ID prevents a disk collision. However, the disk ID should remain unchanged if the disk is bootable
	if (!keepDiskId) {
		setNewDiskID(diskToRestore);
	}
	// Write the track0 data to the target disk
	writeFile(targetDiskHandle, diskToRestore.track0.data(), (uint32_t)diskToRestore.track0.size());

	// If the disk format is MBR, restore the extended partition and logical drive boot records
	if (diskToRestore._header.disk_format == ImageEnums::DiskFormat::eMBR) {
		for (auto& extendedPartition : diskToRestore.extendedPartitions) {
			fileOffset = extendedPartition.offset;
			setFilePointer(targetDiskHandle, fileOffset, std::ios::beg);
			writeFile(targetDiskHandle, &extendedPartition.partitionSector, sizeof(extendedPartition.partitionSector));
		}
	}

	// Record the time when the restoration process starts
	auto lastUpdateTime = std::chrono::steady_clock::now();

	// Loop through each partition in the disk to restore
	for (auto& partition : diskToRestore.partitions) {
		fileOffset = partition._geometry.start + partition._geometry.boot_sector_offset;
		setFilePointer(targetDiskHandle, fileOffset, std::ios::beg);

		// write  FAT32 reserved sectors to the target disk
		if (partition._file_system.reserved_sectors_byte_length > 0) {
			// Loop through each reserved sector block in the partition
			auto totalBytesToWrite = partition._file_system.reserved_sectors_byte_length;
			uint32_t bytesWritten = 0;
			int index = 0;
			for (auto& reservedSectorBlock : partition.reserved_sectors_blocks) {
				// If encryption is enabled, set the Initialization Vector (IV) parameters
				EncryptionIVParams ivParams;
				if (backupLayout._encryption.aes_type != ImageEnums::AES::eNone) {
					ivParams.Set(backupLayout._header, diskToRestore, partition, index++);
				}
				// Read the block data from the backup file
				auto blockData = readBlock(backupLayout, ivParams, backupSet.getFileHandle(reservedSectorBlock.file_number), reservedSectorBlock);
				if (blockData != nullptr) {
					// Calculate the amount of data to write
					uint32_t bytesToWrite = min(reservedSectorBlock.block_length, totalBytesToWrite - bytesWritten);
					// Write the block data to the target disk
					writeFile(targetDiskHandle, blockData.get(), bytesToWrite);
					bytesWritten += bytesToWrite;
				}
			}
			// Update the amount of data processed
			bytesProcessed += partition._file_system.reserved_sectors_byte_length;
			if (outputProgress) outputProgress(totalBackupSize, bytesProcessed, lastUpdateTime);
		}
		int blockIndex = 0;
		// calculate the file offset for the first data block in the partition
		auto lcn0Start = partition._geometry.start + (partition._file_system.lcn0_offset - partition._file_system.start);
		// Loop through each data block in the partition
		int index = 0;
		for (auto& dataBlock : partition.data_blocks) {
			// If encryption is enabled, set the Initialization Vector (IV) parameters
			EncryptionIVParams ivParams;
			if (backupLayout._encryption.aes_type != ImageEnums::AES::eNone) {
				ivParams.Set(backupLayout._header, diskToRestore, partition, index++);
			}
			// Read the block data from the backup file
			auto blockData = readBlock(backupLayout, ivParams, backupSet.getFileHandle(dataBlock.file_number), dataBlock);
			if (blockData != nullptr && dataBlock.block_length != 0) {
				// Update the amount of data processed
				bytesProcessed += partition._header.block_size;
				// Calculate the file offset for the data block
				fileOffset = lcn0Start + (static_cast<uint64_t>(partition._header.block_size) * blockIndex);
				setFilePointer(targetDiskHandle, fileOffset, std::ios::beg);
				// Write the block data to the target disk
				writeFile(targetDiskHandle, blockData.get(), dataBlock.block_length);
				// Output the progress of the restoration process
				if (outputProgress) outputProgress(totalBackupSize, bytesProcessed, lastUpdateTime);
			}
			++blockIndex;
		}
	}

	// Subtract 10 seconds from lastUpdateTime to force an update
	lastUpdateTime -= std::chrono::seconds(10);
	if (outputProgress) outputProgress(totalBackupSize, bytesProcessed, lastUpdateTime);
}

