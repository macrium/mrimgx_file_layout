/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.

This C++ program is designed to convert a Macrium Reflect X backup file to a VHDX
file.

It performs the following operations:

1. Reads the backup file.
2. Parses its JSON data into a backupFile object.
3. Creates and mounts a VHDX file.
4. Restores a specified disk, or the 1st disk, to the VHDX file.

The output VHDX file can then be used for data recovery.


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
#pragma comment(lib, "crypt32")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libzstd.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "VirtDisk.lib")


#include <windows.h>
#include <iostream>

#include "..\libs\file_reader\file_reader.h"
#include "..\libs\vhdx_manager\vhdx_manager.h"
#include "..\libs\restore\restore.h"

#include "console_print.h"
#include "arg_validator.h"
#include "vhdx_handler.h"

// Function prototypes
// 

// Function to read and parse the backup file
file_structs::fileLayout handleBackupFile(const std::wstring& filename, const std::string& password);


/**
 * @brief The main function of the program.
 *
 * This function is the entry point of the program. It sets the console code
 * page to UTF-8, prints a disclaimer, and then handles the command line
 * parameters, backup file, and VHDX file. If the 'describe' flag is set,
 * it outputs the backup file's structure and content and then exits the program.
 * Otherwise, it restores the first disk (or entered disk number) to the VHDX
 * file and waits for the user to press any key before dismounting the VHDX
 * file and exiting the program. If any exceptions occur, it prints them to
 * the console and returns 1. Otherwise, it returns 0.
 *
 * @param argc The number of command line parameters.
 * @param argv The command line parameters.
 * @return 0 if the program runs successfully, or 1 if an exception occurs.
 */
int wmain(int argc, wchar_t* argv[]) {

    // Set the console code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);

    try {
        // Handle the command line parameters and get the filename, password, outputPath, diskNumber, and describe flag
        auto [filename, password, outputPath, diskNumber, keepDiskId, describe, jsonDump] = handleCommandLineParameters(argc, argv);

        // Convert the password from wide string to UTF-8 format. This is necessary because the encryption functions
        // used later in the program expect the password to be in UTF-8 format.
        auto passwordInUtf8Format = convertToUtf8(password);

        // Read the backup file and parse its JSON data into a backupFile object
        auto backupFile = handleBackupFile(filename, passwordInUtf8Format);

        // If the 'jsonDump' flag is set, output the JSON data from the backup file and exit the program
        if (jsonDump) {
            std::cout << backupFile.jsonStr << std::endl;
            return 0;
        }

        // Print the disclaimer.
        printDisclaimer();

        // If the 'describe' flag is set, output the backup file's structure and content, then exit the program.
        if (describe) {
            describeFile(backupFile);
            return 0;
        }
        // Prepare the VHDX file name
        std::wstring vhdxName;
        // Create and mount the VHDX file
        auto vhdxManager = handleVHDXFile(filename, outputPath, vhdxName, backupFile, diskNumber);

        // Print the restoring and to messages
        std::wcout << L"Restoring:\t" << filename << L"\n";
        std::wcout << L"To:\t\t" << vhdxName << L"\n\n";
        // Restore to the VHDX file
        restoreDisk(filename, passwordInUtf8Format, vhdxManager.GetDiskPath(), diskNumber, keepDiskId, outputProgress);

        // Update the properties of the VHDX file. This operation refreshes the system's view of the disk,
        // which is necessary after making changes to the disk, such as restoring a backup to it.
        if (!keepDiskId) {
            vhdxManager.UpdateDiskProperties(); // Only required if we're mounting the disk
        }
        // Print the success message and wait for the user to press any key before dismounting the VHDX file and exiting the program
        std::cout << "\n\nRestore successful.\n";
        if (!keepDiskId) {
            std::cout << "The restored file system(s) can be viewed in Windows File Explorer.\n";
            std::cout << "If not visible, please check the Windows Disk Management Console\n";
            std::cout << "and assign drive letter(s) if necessary.\n\n";
            std::cout << "Press any key to dismount the VHDX and exit . . .\n";
            std::cin.get();
        }
    }
    catch (const std::exception& e) {
        // Print any exceptions that occur
        std::string exceptionMessage = e.what();
        std::cerr << '\n' << exceptionMessage << '\n';

        // If the exception message starts with "Failed to write to file", print additional information about the possible cause of the error
        if (exceptionMessage.rfind("Failed to write to file", 0) == 0) {
            std::cerr << "Access to the mounted VHDX file has been denied.\n";
            std::cerr << "This issue may be due to restrictions imposed by security software installed on your system.\n";
            std::cerr << "Please check your security settings or contact your system administrator for further assistance.\n";
        }

        // Return 1 to indicate that an error occurred
        return 1;
    }

    // Return 0 to indicate that the program ran successfully
    return 0;

}




/**
 * @brief Handles the backup file.
 *
 * This function reads the backup file and parses its JSON data into a backupFile object.
 * If the backup file is an intermediate split file, it throws a runtime error.
 *
 * @param filename The name of the backup file.
 * @param password The password for the backup file (optional).
 * @return The backupFile object that holds the parsed JSON data from the backup file.
 * @throws std::runtime_error If the backup file is an intermediate split file.
 */
file_structs::fileLayout handleBackupFile(const std::wstring& filename, const std::string& password) {
    // Create a backupFile object to hold the parsed JSON data from the backup file
    file_structs::fileLayout backupFile;

    // Read the backup file and parse its JSON data into the backupFile object
    readBackupFile(filename, backupFile, password);

    // If the backup file is an intermediate split file, throw a runtime error
    if (backupFile._header.split_file) {
        throw std::runtime_error("Backup file is an intermediate split file. Use the file name of last file in the split.");
    }

    // Return the backupFile object
    return backupFile;
}

