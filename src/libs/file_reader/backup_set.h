#pragma once

#include "..\file_operations\file_operations.h"

typedef std::shared_ptr<file_structs::fileLayout> BackupFileLayout;
typedef std::vector <BackupFileLayout> BackupSetFileLayouts;

/**
 * @struct BackupSet
 * @brief Represents a set of backup files and their associated handles.
 *
 * This structure contains a vector of unique pointers to `BackupFileAndHandle` objects, which represent backup files and their associated handles.
 * It also contains a map that maps an integer index to a `std::fstream*`. This map can be used to quickly look up a `std::fstream*` based on its associated index.
 *
 * @var fileLayouts A vector of unique pointers to `BackupFileAndHandle` objects.
 * @var indexHandleMap A map that maps an integer index to a `std::fstream*`.
 */
struct BackupSet
{
    // A unique pointer to the backup file layout that we're restoring
    BackupFileLayout backupFileLayoutForRestoration;

    // A map that maps an integer index to a std::fstream*.
    // This map can be used to quickly look up a std::fstream* based on its associated index.
    std::map<int, SharedFile> indexHandleMap;


    // Returns the file handle for the backup file at the given index.
    std::fstream* getFileHandle(const int index);

    // Returns the file layout for backupFileLayoutForRestoration
    file_structs::fileLayout& getBackupFileWithFullIndex();
};


//  Createsa a BackupSet struct.
// It takes a file path, a password, and an image ID as parameters.
// It calls the populateBackupSet function to read the backup set from the file at the given path,
// using the provided password and image ID. It then calls the buildIndex function to build an index for the backup set.
void createBackupSet(BackupSet& backupSet, const std::wstring& filePath, const std::string& password, const std::string& imageId);

