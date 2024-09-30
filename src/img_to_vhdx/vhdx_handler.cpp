
#include <windows.h>
#include "..\libs\file_reader\file_reader.h"
#include "..\libs\vhdx_manager\vhdx_manager.h"

/**
 * @file vhdx_handler.cpp
 *
 * This file contains functions for handling VHDX files. These functions are used
 * to create, mount, and manage VHDX files. They handle the low-level details of
 * interacting with the VHDX file format, such as creating the file with the
 * correct disk size and bytes per sector, mounting the file, and getting a handle
 * to the disk. They also handle error checking and exception handling for VHDX
 * file operations.
 */

/**
 * @brief Prepares the VHDX file name.
 *
 * This function constructs the VHDX file name based on the provided filename and outputPath.
 * If the outputPath is not empty and valid, it replaces the path in vhdxName with outputPath.
 * If the outputPath is not empty but invalid, it throws a runtime error.
 * It also deletes the VHDX file if it already exists.
 *
 * @param filename The name of the file to process.
 * @param outputPath The path where the output should be written (optional).
 * @return The prepared VHDX file name.
 * @throws std::runtime_error If the outputPath is not empty but invalid, or if failed to delete existing VHDX file.
 */
std::wstring prepareVhdxFileName(const std::wstring& filename, const std::wstring& outputPath) {
    // Initialize vhdxName with the filename
    std::wstring vhdxName = filename;
    // Find the position of ".mrimgx" in vhdxName
    std::wstring::size_type index = vhdxName.rfind(L".mrimgx");

    // If ".mrimgx" is found, replace it with ".vhdx"
    if (index != std::wstring::npos) {
        vhdxName.replace(index, 7, L".vhdx");
    }
    else {
        // If ".mrimgx" is not found, find the position of ".mrbakx" in vhdxName
        index = vhdxName.rfind(L".mrbakx");
        // If ".mrbakx" is found, replace it with ".vhdx"
        if (index != std::wstring::npos) {
            vhdxName.replace(index, 7, L".vhdx");
        }
    }

    // If outputPath is not empty, replace the path in vhdxName with outputPath
    if (!outputPath.empty()) {
        // If outputPath exists, replace the path in vhdxName with outputPath
        if (std::filesystem::exists(outputPath)) {
            std::filesystem::path p(vhdxName);
            vhdxName = (std::filesystem::path(outputPath) / p.filename()).wstring();
        }
        // If outputPath does not exist, throw a runtime error
        else {
            throw std::runtime_error("Provided output path does not exist.");
        }
    }

    // If the VHDX file already exists, delete it
    if (DeleteFile(vhdxName.c_str()) == 0 && GetLastError() != ERROR_FILE_NOT_FOUND) {
        throw std::runtime_error("Failed to delete existing VHDX file: " + std::to_string(GetLastError()));
    }

    // Return the vhdxName
    return vhdxName;
}


/**
 * @brief Handles the creation and mounting of the VHDX file.
 *
 * This function gets the disk layout for the disk to be restored, constructs the VHDX file name,
 * creates the VHDX file, and mounts it.
 *
 * @param filename The name of the backup file.
 * @param outputPath The path where the output should be written (optional).
 * @param vhdxName A reference to a string where the VHDX file name will be stored.
 * @param backupFile The backupFile object that holds the parsed JSON data from the backup file.
 * @param diskNumber The number of the disk to be restored (-1 means the first disk in the backup file).
 * @return The VHDXManager object that manages the VHDX file.
 */
VHDXManager handleVHDXFile(const std::wstring& filename, const std::wstring& outputPath, std::wstring& vhdxName, const file_structs::fileLayout& backupFile, int diskNumber) {
    // Get the disk layout for the disk to be restored. -1 means the first disk in the backup file
    file_structs::Disk::DiskLayout diskToRestore;
    getDiskToRestoreFromDiskNumber(backupFile, diskNumber, diskToRestore);

    // Construct the .vhdx file name
    vhdxName = prepareVhdxFileName(filename, outputPath);

    // Create a VHDXManager object
    VHDXManager vhdxManager;
    // Create the VHDX file with the specified name, disk size, and bytes per sector
    vhdxManager.CreateVHDX(vhdxName, diskToRestore._geometry.disk_size, backupFile.disks[0]._geometry.bytes_per_sector);
    // Mount the VHDX file
    vhdxManager.MountVHDX(vhdxName);

    // Return the VHDXManager object
    return vhdxManager;
}
