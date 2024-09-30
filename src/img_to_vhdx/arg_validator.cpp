#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <filesystem>

/**
 * @file arg_validator.cpp
 *
 * This file contains functions for validating command line arguments. These
 * functions check the number, type, and format of the arguments, and they
 * ensure that the arguments meet the requirements of the program. They can
 * handle optional and required arguments, and they can provide error messages
 * for invalid arguments.
 */

// Function prototypes
void printHelp();

/**
 * Checks if the given filename has a valid extension.
 *
 * This function checks if the filename ends with either ".mrimgx" or ".mrbakx".
 * The check is case-insensitive.
 *
 * @param filename The filename to check.
 * @return true if the filename has a valid extension, false otherwise.
 */
bool isValidExtension(const std::wstring& filename) {
	if (filename.length() < 7) { // Minimum length to hold either extension
		return false;
	}

	std::wstring extension = filename.substr(filename.length() - 7);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	return extension == L".mrimgx" || extension == L".mrbakx";
}

/**
 * Converts a wide string to a UTF-8 string.
 *
 * This function uses the Windows API function WideCharToMultiByte to convert a wide string to a UTF-8 string.
 * If the wide string is empty, the function returns an empty string.
 *
 * @param wstr The wide string to convert.
 * @return The converted UTF-8 string.
 */
std::string convertToUtf8(const std::wstring& wstr)
{
    // If the wide string is empty, return an empty string
    if (wstr.empty()) return std::string();

    // Determine the number of bytes needed to store the converted string
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);

    // Create a string of the required size
    std::string strTo(sizeNeeded, 0);

    // Convert the wide string to a UTF-8 string
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);

    return strTo;
}



/**
 * @brief Parses command line parameters.
 *
 * This function parses command line parameters and returns a tuple containing the filename, password, output path, and disk number.
 * The filename is required, while the password, output path, and disk number are optional.
 * The function checks if the filename has a valid extension and if the file exists.
 * If the `-p` parameter is provided, the next parameter is the password.
 * If the `-o` parameter is provided, the next parameter is the output path.
 * If the `-d` parameter is provided, the next parameter is the disk number.
 * If the `-k` parameter is provided, the keepDiskId boolean is set to true.
 * if the 'describe' parameter is provided, a boolean is set to true.
 * if the 'json' parameter is provided, a boolean is set to true.
 * If the `-h` parameter is provided, an exception is thrown to indicate that help is requested.
 * If an unknown parameter is provided, an exception is thrown.
 *
 * @param argc The number of command line parameters.
 * @param argv The command line parameters.
 * @return A tuple containing the filename, password, output path, disk number, keepDiskId, describe, and jsonDump.
 * @throws std::invalid_argument if no filename is provided, if the filename has an invalid extension, if the file does not exist, if help is requested, or if an unknown parameter is provided.
 */
std::tuple<std::wstring, std::wstring, std::wstring, int, bool, bool, bool> parseCommandLineParameters(int argc, wchar_t* argv[])
{
    // Check if the help parameter is provided
    for (int i = 1; i < argc; i++) {
        if (std::wstring(argv[i]) == L"-h" || std::wstring(argv[i]) == L"help") {
            throw std::invalid_argument("Help requested.");
        }
    }

    // Check if the filename parameter is provided
    if (argc < 2) {
        throw std::invalid_argument("No filename provided.");
    }

    std::wstring filename = argv[1];
    if (!isValidExtension(filename)) {
        throw std::invalid_argument("Invalid file extension. Only .mrimgx and .mrbakx are allowed.");
    }

    // Check if the file exists
	std::filesystem::path path = filename;
	if (!std::filesystem::exists(path)) {
		throw std::invalid_argument("File does not exist.");
	}

    std::wstring password;
    std::wstring outputPath;
    int diskNumber = -1; // Default disk number
    bool describe = false; // Default describe value
    bool jsonDump = false; // Default json value
    bool keepDiskId = false;

    // Parse the optional parameters
    for (int i = 2; i < argc; i++) {
        if ((std::wstring(argv[i]) == L"-p" || std::wstring(argv[i]) == L"password") && i + 1 < argc) {
            password = argv[++i];
        }
        else if ((std::wstring(argv[i]) == L"-o" || std::wstring(argv[i]) == L"output_path") && i + 1 < argc) {
            outputPath = argv[++i];
        }
        else if ((std::wstring(argv[i]) == L"-d" || std::wstring(argv[i]) == L"disk") && i + 1 < argc) {
            diskNumber = std::stoi(argv[++i]);
        }
        else if (std::wstring(argv[i]) == L"-desc" || std::wstring(argv[i]) == L"describe") {
            describe = true;
        }
        else if (std::wstring(argv[i]) == L"-j" || std::wstring(argv[i]) == L"json") {
            jsonDump = true;
        }
        else if (std::wstring(argv[i]) == L"-k" || std::wstring(argv[i]) == L"keep_id") {
            keepDiskId = true;
        }
        else {
             throw std::invalid_argument("Unknown parameter " + convertToUtf8(argv[i]));
        }
    }

    return std::make_tuple(filename, password, outputPath, diskNumber, keepDiskId, describe, jsonDump);
}


/**
 * @brief Handles command line parameters.
 *
 * This function uses the parseCommandLineParameters function to parse the command line parameters.
 * It then returns a tuple containing the filename, password, output path, disk number, keepDiskId, describe, and jsonDump.
 * If the `-h` or `help` parameter is provided, it calls the printHelp function and rethrows the exception.
 * If any other invalid argument exception is thrown, it prints an error message and rethrows the exception.
 *
 * @param argc The number of command line parameters.
 * @param argv The command line parameters.
 * @return A tuple containing the filename, password, output path, disk number, keepDiskId, describe, and jsonDump.
 * @throws std::invalid_argument if an error occurs while parsing the command line parameters.
 */
std::tuple<std::wstring, std::wstring, std::wstring, int, bool, bool, bool> handleCommandLineParameters(int argc, wchar_t* argv[]) {
	std::wstring filename;
	std::wstring password;
	std::wstring outputPath;
    bool describe = false;
    bool jsonDump = false;
    bool keepDiskId = false;
    int diskNumber = -1;

	try {
		// Parse the command line parameters
		std::tie(filename, password, outputPath, diskNumber, keepDiskId, describe, jsonDump) = parseCommandLineParameters(argc, argv);
	}
	catch (const std::invalid_argument& e) {
		if (std::string(e.what()) == "Help requested.") {
			printHelp();
		}
		else {
			std::wcerr << L"Error: " << e.what() << std::endl;
		}
		throw;
	}

	return { filename, password, outputPath, diskNumber, keepDiskId, describe, jsonDump };
}