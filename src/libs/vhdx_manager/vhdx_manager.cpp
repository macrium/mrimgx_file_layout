#include "pch.h"

/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.
===============================================================================
*/
#include "pch.h"
#include "vhdx_manager.h"


// Define the VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT GUID
EXTERN_C const GUID DECLSPEC_SELECTANY VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT =
{ 0xec984aec, 0xa0f9, 0x47e9, { 0x90, 0x1f, 0x71, 0x41, 0x5a, 0x66, 0x34, 0x5b } };

/**
 * @brief Creates a VHDX file at the specified file path with the specified size and physical sector size.
 *
 * The VHDX file is created with a block size and sector size that are set to default values.
 * If the creation of the VHDX file fails, the function throws a runtime_error.
 *
 * @param filePath The path where the VHDX file will be created.
 * @param size The size of the VHDX file.
 * @param physicalSectorSize The physical sector size of the VHDX file.
 * @throw runtime_error If the creation of the VHDX file fails.
 */
void VHDXManager::CreateVHDX(const std::wstring& filePath, ULONGLONG size, ULONG physicalSectorSize)

{
    // Adjust size to be a multiple of physicalSectorSize
    if (size % physicalSectorSize != 0) {
        size = ((size / physicalSectorSize) + 1) * physicalSectorSize;
    }

    // Initialize the storage type for the VHDX.
    VIRTUAL_STORAGE_TYPE storageType = {};
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;

    // Initialize the parameters for creating the VHDX.
    CREATE_VIRTUAL_DISK_PARAMETERS parameters = {};
    memset(&parameters, 0, sizeof(CREATE_VIRTUAL_DISK_PARAMETERS));

    parameters.Version = CREATE_VIRTUAL_DISK_VERSION_2;

    parameters.Version2.MaximumSize = size;
    parameters.Version2.BlockSizeInBytes = CREATE_VIRTUAL_DISK_PARAMETERS_DEFAULT_BLOCK_SIZE;
    parameters.Version2.SectorSizeInBytes = physicalSectorSize;
    parameters.Version2.PhysicalSectorSizeInBytes = physicalSectorSize; 

    // Initialize a NULL for the VHDX.
    HANDLE vhdxCreateHandle = NULL;

    // Call the CreateVirtualDisk function to create the VHDX.
    HRESULT result = CreateVirtualDisk(
        &storageType,
        filePath.c_str(),
        VIRTUAL_DISK_ACCESS_NONE,
        NULL,
        CREATE_VIRTUAL_DISK_FLAG_NONE, 
        0,
        &parameters,
        NULL,
        &vhdxCreateHandle
    );

    // Check for the specific error code E_INVALIDARG.
    if (vhdxCreateHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create VHDX: " + std::to_string(result));
    }

    // If the creation of the VHDX failed, throw a runtime error.
    if (FAILED(result)) {
        CloseHandle(vhdxCreateHandle);
        throw std::runtime_error("Failed to create VHDX: " + std::to_string(result));
    }
}


/**
 * @brief Mounts a VHDX file from the specified file path.
 *
 * This function opens the VHDX file and then attaches it using the system's virtual disk service.
 * If the opening or attaching of the VHDX file fails, the function throws a runtime_error.
 *
 * @param filePath The path of the VHDX file to mount.
 * @throw runtime_error If the mounting fails.
 */
void VHDXManager::MountVHDX(const std::wstring& filePath)
{
    // Initialize the storage type for the VHDX.
    VIRTUAL_STORAGE_TYPE storageType = {};
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_VHDX;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;

    // Initialize the parameters for opening the VHDX.
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters = {};
    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    openParameters.Version2.GetInfoOnly = FALSE;
 

    // Call the OpenVirtualDisk function to open the VHDX.
    HRESULT result = OpenVirtualDisk(
        &storageType,
        filePath.c_str(),
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS,
        &openParameters,
        &vhdxHandle
    );

    // Create a smart std::fstream* to manage the VHDX std::fstream*.
   // UniqueHandle smartHandle(vhdxHandle);

    // If the opening of the VHDX succeeded, try to attach it.
    if (SUCCEEDED(result) && vhdxHandle != INVALID_HANDLE_VALUE) {
        // Attach the VHDX
        ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters = {};
        attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;

        result = AttachVirtualDisk(
            vhdxHandle,
            NULL,
            ATTACH_VIRTUAL_DISK_FLAG_NONE,
            0,
            &attachParameters,
            NULL
        );
    }

    // If the opening or attaching of the VHDX failed, throw a runtime error.
    if (FAILED(result) || vhdxHandle == INVALID_HANDLE_VALUE) {
        if (vhdxHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(vhdxHandle);
            vhdxHandle = INVALID_HANDLE_VALUE;
        }
        throw std::runtime_error("Failed to attach VHDX: " + std::to_string(result));
    }

    diskPath.resize(MAX_PATH);
    DWORD diskPathSizeInChars = MAX_PATH;
    result = GetVirtualDiskPhysicalPath(vhdxHandle, &diskPathSizeInChars, &diskPath[0]);

    if (FAILED(result)) {
        throw std::runtime_error("Failed to get physical path: " + std::to_string(result));
    }
}


/**
 * @brief Updates the properties of the target disk.
 *
 * This function opens the target disk for read/write access, then sends the IOCTL_DISK_UPDATE_PROPERTIES
 * control code to the disk device driver to update the properties of the disk. If any of these operations
 * fail, the function throws a runtime_error.
 *
 * @throw runtime_error If the opening of the target disk or the updating of its properties fails.
 */
void VHDXManager::UpdateDiskProperties()
{
    // Initialize a variable to receive the number of bytes returned by DeviceIoControl.
    DWORD bytesWritten = 0;

    // Open the target disk for read/write access.
    HANDLE targetDiskHandle = CreateFile(diskPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    // Send the IOCTL_DISK_UPDATE_PROPERTIES control code to the disk device driver.
    // This control code does not require any input data or output data.
    if (!DeviceIoControl(targetDiskHandle, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &bytesWritten, NULL))
    {
        // If DeviceIoControl fails, close the handle to the target disk and throw a runtime_error.
        CloseHandle(targetDiskHandle);
        throw std::runtime_error("Failed to update disk properties.");
    }

    // Close the handle to the target disk.
    CloseHandle(targetDiskHandle);
}
