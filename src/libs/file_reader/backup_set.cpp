#include "pch.h"
#include "..\file_operations\file_operations.h"
#include "file_structs.h"
#include "file_reader.h"
#include "backup_set.h"

/**
 * @file
 * @brief This file contains the implementation of the Macrium Reflect X backup set handling.
 *
 * This library provides a set of cross-platform functions for managing and manipulating backup sets.
 * It handles various aspects such as creating a backup set from a given backup file, adding a backup file to a set,
 * building an index for the backup set, and retrieving specific backup files from the set.
 */


// Helper functions
/**
 * @brief Maps the delta data blocks to the corresponding full data blocks.
 *
 * This function is used when restoring a backup. It iterates over each delta data block in the delta partition layout,
 * and maps it to the corresponding data block in the full partition layout using the index in the delta data block.
 *
 * @param partitionToRestore - The partition being restored.
 * @param partitionToMap    - The previous delta index to map into partitionToRestore
 *
 * Note: partitionToRestore has already had the full index copied into it. This function will map the delta index into it.
 *       
 */
void mapDeltaToFullIndex(file_structs::Partition::PartitionLayout& partitionToRestore,const file_structs::Partition::PartitionLayout& partitionToMap) {
    // Iterate over each delta data block and map it to the corresponding full data block
    for (const auto& deltaBlock : partitionToMap.delta_data_blocks) {
        // Check if the index is within the current size of the vector
        if (deltaBlock.block_index >= partitionToRestore.data_blocks.size()) {
            // Resize the vector to accommodate the index
            partitionToRestore.data_blocks.resize(deltaBlock.block_index + 1);
        }
        // Use the index in deltaBlock to access the corresponding data block in full.data_blocks
        partitionToRestore.data_blocks[deltaBlock.block_index] = deltaBlock.delta_data_block;
    }
}

/**
 * @brief Retrieves the index of the last full backup before any delta backups.
 *
 * This function iterates over the backup set and returns the index of the last full backup (not delta) before any delta backups.
 * If no such backup is found, an exception is thrown.
 *
 * @param backupSet The BackupSet to search.
 * @param backupSetFileLayouts The file layouts for the backup set.
 * @return The index of the last full backup before any delta backups.
 * @throws std::runtime_error if no full backup is found before any delta backups.
 */
size_t getFullIndexBeforeDelta(const BackupSetFileLayouts& backupSetFileLayouts)
{
    for (size_t i = 0; i < backupSetFileLayouts.size(); ++i) {
        // Find the first full index (not delta) backup before any delta backups
        // Split files contain no block indexes, they only contain the header and the actual data blocks 
        if (!backupSetFileLayouts[i]->_header.delta_index &&
            !backupSetFileLayouts[i]->_header.split_file) {
            return i;
        }
    }
    throw std::runtime_error("getFullIndexBeforeDelta - failed");
}


// ==============================
// struct BackupSet functions
// ==============================

/**
 * @brief Retrieves the file handle for a given file number in a backup set.
 *
 * This function iterates over all the file handles in the backup set. If the file number of the current file handle matches the requested index, the handle is returned.
 * If no matching file handle is found, an exception is thrown.
 *
 * @param index The file number to search for.
 * @return The file handle for the file with the given number.
 * @throws std::runtime_error if no file with the given number is found in the backup set.
 */
std::fstream* BackupSet::getFileHandle(const int index) 
{
    try {
        return indexHandleMap.at(index).get();
    }
    catch (const std::out_of_range&) {
        throw std::runtime_error("getFileHandle - File not found");
    }
}

/**
 * @brief Retrieves the backup file with a full index in a backup set.
 *
 * This function returns a reference to the backup file with a full index in the backup set.
 * If no backup file with a full index is found, an exception is thrown.
 *
 * @return A reference to the backup file with a full index.
 * @throws std::runtime_error if no backup file with a full index is found in the backup set.
 */
file_structs::fileLayout& BackupSet::getBackupFileWithFullIndex()
{
    if (!backupFileLayoutForRestoration) {
        throw std::runtime_error("getBackupFileWithFullIndex - File not found");
    }
    return *(backupFileLayoutForRestoration);
}

// ==============================
// Implementation Functions
// ==============================

/**
 * @brief Builds an index for the backup set.
 *
 * This function builds an index for the backup set to facilitate quick access to the backup files.
 * If the backup set contains a delta index, the function builds a full index by mapping the delta data blocks to the corresponding full data blocks.
 *
 * @param backupSet The BackupSet to build an index for.
 * @param backupSetFileLayouts The file layouts for the backup set.
 */
void buildIndex(BackupSet& backupSet, const BackupSetFileLayouts& backupSetFileLayouts) 
{
    // If there is no delta index, there is no need to build an index
    if (!backupSet.backupFileLayoutForRestoration->_header.delta_index) {
        return;
    }

    // this is the most recent full index before the delta
    int lastFullIndexBeforeDelta = (int)getFullIndexBeforeDelta(backupSetFileLayouts);
    // Iterate over each disk in the backup set
    for (size_t diskIndex = 0; diskIndex < backupSet.backupFileLayoutForRestoration->disks.size(); ++diskIndex) {

        auto& diskToRestore = backupSet.backupFileLayoutForRestoration->disks[diskIndex];

        // Iterate over each partition in the disk
        for (size_t partitionIndex = 0; partitionIndex < diskToRestore.partitions.size(); ++partitionIndex) {

            auto& partitionToRestore = diskToRestore.partitions[partitionIndex];

            // Copy the data blocks from the last full index
            partitionToRestore.data_blocks = backupSetFileLayouts[lastFullIndexBeforeDelta].get()->disks[diskIndex].partitions[partitionIndex].data_blocks;

            // Iterate over each delta file in the backup set afer the last full index 
            for (int backupSetFileIndex = lastFullIndexBeforeDelta; backupSetFileIndex >= 0; backupSetFileIndex--) {

                const auto& backupFileInSet = *(backupSetFileLayouts[backupSetFileIndex]);

                // Split files contain no block indexes, they only contain the header and the actual data blocks 
                if (!backupFileInSet._header.split_file) {
                    auto& partitionToMap = backupFileInSet.disks[diskIndex].partitions[partitionIndex];
                    // Copy each delta data block index to the corresponding full data block index
                    mapDeltaToFullIndex(partitionToRestore, partitionToMap);
                }
            }
        }
    }
}

/**
 * @brief Adds a backup file to the backup set.
 *
 * This function creates a BackupFileAndHandle object for the backup file and adds it to the backup set.
 * It also adds the file number and file handle to the indexHandleMap in the backupSet, which is used to quickly find the file handle for a given file number.
 * The function also handles file merges due to consolidation. This enables merged files to reference the file handle of the consolidated file.
 *
 * @param backupSet The BackupSet object to add the backup file to.
 * @param filePath The path to the backup file.
 * @param backupFile The file layout of the backup file.
 * @param backupSetFileLayouts The file layouts for the backup set.
 */
void addBackupFileToSet(BackupSet& backupSet, const std::filesystem::path& filePath, const file_structs::fileLayout& backupFile, BackupSetFileLayouts& backupSetFileLayouts) {
    // Create a BackupFileAndHandle object for the backup file
    auto backupFileLayout = backupFile;

    // Add the file number and file handle to the indexHandleMap in the backupSet
    // The indexHandleMap is used to quickly find the file handle for a given file number
    auto sharedFileHandle = openFileWithGuard(filePath.wstring(), true);
    backupSet.indexHandleMap[backupFile._header.file_number] = sharedFileHandle;

    // Handle file merges due to consolidation
    // This enables merged files to reference the file handle of the consolidated file
    for (auto mergedFileNumber : backupFile._header.merged_files) {
        backupSet.indexHandleMap[mergedFileNumber] = sharedFileHandle;
    }

    // Move the shared_ptr to backupFileLayout into the fileLayouts vector in the backupSet
    backupSetFileLayouts.push_back(std::make_shared<file_structs::fileLayout>(backupFile));
}

// ==============================
// Public Functions
// ==============================

/**
 * @brief Creates a backup set from a given backup file.
 *
 * This function populates a BackupSet object with backup files from a given file path.
 * It then builds an index for the backup set to facilitate quick access to the backup files.
 *
 * @param backupSet The BackupSet object to populate.
 * @param filePath The path to the backup file.
 * @param password The password for the backup file.
 * @param imageId The image ID for the backup file.
 */
void createBackupSet(BackupSet& backupSet, const std::wstring& filePath, const std::string& password, const std::string& imageId)
{
    // Create a BackupSetFileLayouts object to hold the file layouts for the backup set
    BackupSetFileLayouts backupSetFileLayouts;

    // Create a file layout for the image file
    file_structs::fileLayout backupFile;
    // Setting loadIndex to false saves memory and time.
    // The index is not needed here
    bool loadIndex = false;
    // Read the backup file
    readBackupFile(filePath, backupFile, password, loadIndex);

    // Find the rest of the files in this backup set
    std::filesystem::path path(filePath);
    std::string extension = path.extension().string();
    if (!extension.empty()) {
        std::filesystem::path directory = path.parent_path();
        // Iterate over all files in the directory
        for (const auto& fileName : std::filesystem::directory_iterator(directory)) {
            // If the file is a regular file and has the same extension as the backup file
            if (fileName.is_regular_file() && fileName.path().extension() == extension) {
                // Create a new BackupFileAndHandle and fileLayout using std::make_unique for efficient memory allocation
                //auto backupFileLayout = std::make_unique<BackupFileAndHandle>();
                auto setbackupFile = std::make_unique<file_structs::fileLayout>();
                try {
                    // Setting loadIndex to false saves memory and time.
                    // The index is not needed here
                    bool loadIndex = false;
                    // Read the backup file
                    readBackupFile(fileName.path().wstring(), *setbackupFile, password, loadIndex);
                    // If the image ID, increment number, and file name match the backup file
                    if (setbackupFile->_header.imageid == imageId &&
                        setbackupFile->_header.increment_number <= backupFile._header.increment_number) {
                        loadIndex = true;

                        // Read the backup file again with loadIndex set to true
                        readBackupFile(fileName.path().wstring(), *setbackupFile, password, loadIndex);
                        // Add the backup file handle to the backup set
                        addBackupFileToSet(backupSet, fileName.path().wstring(), *setbackupFile, backupSetFileLayouts);
                    }
                }
                catch (const std::exception& e) {
                    // Invalid password or corrupt file errors do not fail the entire backup set
                    // There may be files in the folder that are not part of the backup set
                    std::cerr << "Caught exception: " << e.what() << '\n';
                }
            }
        }
        // Sort the backup set by file number in descending order
        std::sort(backupSetFileLayouts.begin(), backupSetFileLayouts.end(),
            [](const std::shared_ptr<file_structs::fileLayout>& a, const std::shared_ptr<file_structs::fileLayout>& b) {
            return a->_header.file_number > b->_header.file_number;
        }
        );

        backupSet.backupFileLayoutForRestoration = backupSetFileLayouts[0];
        // Build the full index for the backup set. This maps delta incrementals into the block index.
        buildIndex(backupSet, backupSetFileLayouts);

    }
    else {
        // If the backup file does not have an extension, throw an exception
        throw std::runtime_error("populateBackupSet - invalid file extension");
    }
}


