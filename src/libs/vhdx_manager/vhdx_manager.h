#pragma once

/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.
===============================================================================
*/

/**
* @file
* @brief This is the primary include file for this library.
*
* Including this file in a project gives access to all the functionality provided by this library.
* No other include files are necessary.
**/

// VHDXManager class provides methods to create and mount VHDX files.
class VHDXManager {
private:
    HANDLE vhdxHandle;
    std::wstring diskPath;
public:
    // Default constructor
    VHDXManager() : vhdxHandle(INVALID_HANDLE_VALUE) {};
    // Default destructor
    ~VHDXManager() { if (vhdxHandle != INVALID_HANDLE_VALUE) CloseHandle(vhdxHandle);
    };

    /**
     * @brief Creates a VHDX file at the specified file path with the specified size and physical sector size.
     *
     * The VHDX file is created with a block size and sector size that are set to default values.
     * If the creation of the VHDX file fails, the function throws a runtime_error.
     *
     * @param filePath The path where the VHDX file will be created.
     * @param size The size of the VHDX file to be created.
     * @param physicalSectorSize The physical sector size of the VHDX file to be created.
     * @throw runtime_error If the creation of the VHDX file fails.
     */
    void CreateVHDX(const std::wstring& filePath, ULONGLONG size, ULONG physicalSectorSize);

    /**
     * @brief Mounts a VHDX file from the specified file path.
     *
     * If the mounting of the VHDX file fails, the function throws a runtime_error.
     *
     * @param filePath The path of the VHDX file to be mounted.
     * @throw runtime_error If the mounting of the VHDX file fails.
     */
    void MountVHDX(const std::wstring& filePath);

    /**
    * @brief Updates the properties of the target disk.
    *
    * This function sends the IOCTL_DISK_UPDATE_PROPERTIES control code to the disk device driver
    * to update the properties of the disk. If the operation fails, the function throws a runtime_error.
    *
    * @throw runtime_error If the updating of the disk properties fails.
    */
    void UpdateDiskProperties();

    /**
     * @brief Gets the path of the target disk.
     *
     * This function returns the path of the target disk as a wide string.
     *
     * @return The path of the target disk.
     */
    std::wstring  GetDiskPath() const { return diskPath; };
};
