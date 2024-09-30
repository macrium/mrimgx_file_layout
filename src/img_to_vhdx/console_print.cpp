#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include "..\dependencies\include\openssl\opensslv.h"
#include "..\dependencies\include\nlohmann\json.hpp"
#include "..\dependencies\include\zstd\zstd.h"
#include "..\libs\file_reader\file_reader.h"

/**
 * @file console_print.cpp
 *
 * This file contains functions for printing information to the console. These
 * functions are used to output the structure and content of a backup file in a
 * human-readable format. They handle the formatting of different types of data,
 * such as disk sizes and dates/times, and they ensure that the output fits
 * within the console window's width.
 *
 * The file also contains several helper functions for wrapping text to fit
 * within the console window's width and formatting disk sizes.
 */

 /**
  * @brief Prints a disclaimer to the console.
  *
  * This function outputs a disclaimer to the console. It informs users that the
  * software is intended for demonstration purposes only, illustrates a method to
  * access data stored in a Macrium Reflect X backup file, comes with no warranty,
  * and should be used at their own risk.
  */

void printDisclaimer() {
	std::cout << "=================================================================\n";
	std::cout << "  DISCLAIMER:\n";
	std::cout << "  This software is for demonstration purposes only.\n";
	std::cout << "  It shows how to access data in a Macrium Reflect X backup file.\n";
	std::cout << "  This software comes with no warranty, expressed or implied.\n";
	std::cout << "  Use this software at your own risk.\n";
	std::cout << "\n";
	std::cout << "This program uses the following 3rd party libraries:\n";
	std::cout << std::left << std::setw(20) << "  OpenSSL:" << OPENSSL_VERSION_MAJOR << "." << OPENSSL_VERSION_MINOR << "." << OPENSSL_VERSION_PATCH << "\n";
	std::cout << std::left << std::setw(20) << "  nlohmann json:" << NLOHMANN_JSON_VERSION_MAJOR << "." << NLOHMANN_JSON_VERSION_MINOR << "." << NLOHMANN_JSON_VERSION_PATCH << "\n";
	std::cout << std::left << std::setw(20) << "  Zstd:" << ZSTD_VERSION_MAJOR << "." << ZSTD_VERSION_MINOR << "." << ZSTD_VERSION_RELEASE << "\n";
	std::cout << "=================================================================\n";
	std::cout << "\n";
}

/**
 * @brief Prints the help message.
 *
 * This function outputs a help message to the console. It provides information
 * on how to use the program and its parameters. It describes the purpose of
 * each parameter and whether it is optional or required.
 */
void printHelp() {
	std::wcout << L"Usage: filename [-p password] [-d disk] [-k keep_id] [-o output_path] [-desc describe] [-j json] [-h help]\n";
	std::wcout << L"filename: The name of the file to process.\n";
	std::wcout << L"-p password:\tThe password for the backup file (optional).\n";
	std::wcout << L"-d disk:\tThe disk number to restore (defaults to first disk if not supplied).\n";
	std::wcout << L"-k keep_id:\tDo not update the disk ID in the restored VHDX.\n";
	std::wcout << L"-o output_path:\tThe path for the VHDX (optional, defaults to backup file location).\n";
	std::wcout << L"-desc describe:\tOutputs the backup file's structure and content.\n";
	std::wcout << L"-j json:\tOutputs the backup file's json metadata.\n";
	std::wcout << L"-h help:\tDisplay this help message.\n";
	std::wcout << L"\n";
	std::wcout << L"Examples:\n";
	std::wcout << L"\timg_to_vhdx.exe c:\\backup.mrimgx -p mypassword -d 1 -o C:\\output\n";
	std::wcout << L"\timg_to_vhdx.exe c:\\backup.mrimgx -p mypassword -d 1\n";
	std::wcout << L"\timg_to_vhdx.exe c:\\backup.mrimgx -p mypassword\n";
	std::wcout << L"\timg_to_vhdx.exe c:\\backup.mrimgx\n";
	std::wcout << L"\timg_to_vhdx.exe c:\\backup.mrimgx describe\n";
	std::wcout << L"\timg_to_vhdx.exe c:\\backup.mrimgx json\n";

	// Exit the program
	exit(0);
}

/**
 * Formats a disk size from bytes to a human-readable string.
 *
 * This function takes a size in bytes and converts it to a string representation
 * in the appropriate unit (bytes, KB, MB, GB, or TB), depending on the size.
 * The output string is formatted to two decimal places.
 *
 * @param sizeInBytes The size in bytes to format.
 * @return A string representing the size in the appropriate unit.
 */
std::string formatDiskSize(uint64_t sizeInBytes) {
	const uint64_t KB = 1024;
	const uint64_t MB = KB * 1024;
	const uint64_t GB = MB * 1024;
	const uint64_t TB = GB * 1024;

	std::ostringstream out;
	out.precision(2); // Set precision to 2 decimal places
	out << std::fixed; // Ensure decimal places are always shown

	if (sizeInBytes >= TB) {
		out << static_cast<double>(sizeInBytes) / TB << " TB";
	}
	else if (sizeInBytes >= GB) {
		out << static_cast<double>(sizeInBytes) / GB << " GB";
	}
	else if (sizeInBytes >= MB) {
		out << static_cast<double>(sizeInBytes) / MB << " MB";
	}
	else {
		out << sizeInBytes << " bytes";
	}

	return out.str();
}

/**
 * @brief Describes the structure of a backup file.
 *
 * This function outputs the structure of a backup file to the console. It prints
 * information about the file, including the JSON and Macrium versions, file name,
 * image ID, backup GUID, backup time, netbios name, compression level, encryption
 * status, and comment.
 *
 * It also prints information about each disk in the backup file, including the
 * disk ID, format, and size, and information about each partition in the disk,
 * including the partition number, letter, size, volume GUID, label, and format.
 *
 * @param layout The structure of the backup file.
 */
void describeFile(const file_structs::fileLayout& layout) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;

	std::cout << "Macrium Reflect Backup File Structure\n";
	std::cout << std::string(columns, '_') << "\n";
	std::cout << "\n";
	std::cout << "Json:\t\t" << "v" << layout._header.json_version << "\n";
	std::cout << "Macrium:\t" << "v" << layout._auxiliary_data.macrium_reflect.file_version << "." << layout._auxiliary_data.macrium_reflect.build << "\n";
	std::cout << std::string(columns, '_') << "\n";
	std::cout << "\n";
	std::cout << "File Name:\t" << layout._auxiliary_data.backup_definition.filename << "\n";
	std::cout << "Image ID:\t" << layout._header.imageid << "\n";
	std::cout << "Backup GUID:\t" << layout._header.backup_guid << "\n";
	// Create a std::tm structure
	std::tm backupTime;
	// Convert backupset_time to a std::tm structure in a thread-safe way
	localtime_s(&backupTime, &layout._header.backupset_time);

	// Format backupTime as a date and time string
	std::cout << "Backup Time:\t" << std::put_time(&backupTime, "%Y-%m-%d %H:%M:%S") << "\n";
	std::cout << "Netbios:\t" << layout._header.netbios_name << "\n";
	std::cout << "Compression:\t" << GetCompressionText(layout._compression.compression_level) << "\n";
	std::cout << "Encryption:\t" << (layout._encryption.enable ? GetEncryptionText(layout._encryption.aes_type) : "not set") << "\n";
	std::cout << "Comment:\t" << layout._auxiliary_data.backup_definition.comment << "\n";
	for (auto Disk : layout.disks) {
		std::cout << std::string(columns, '_') << "\n";
		std::cout << "\n";
		std::cout << "Disk " << Disk._header.disk_number << ":\n";
		std::cout << "ID:\t" << Disk._header.disk_signature << "\n";
		std::cout << "Format:\t" << GetDiskFormatText(Disk._header.disk_format) << "\n";
		std::cout << "Size:\t" << formatDiskSize(Disk._geometry.disk_size) << "\n";

		for (auto Partition : Disk.partitions) {
			std::cout << "\n";
			std::cout << "\tPartition " << Partition._header.partition_number << ":\n";
			std::cout << "\tLetter:\t" << (Partition._file_system.drive_letter ? std::string(1, Partition._file_system.drive_letter) : std::string("N/A")) << "\n";
			std::cout << "\tSize:\t" << formatDiskSize(Partition._geometry.length) << "\n";
			std::cout << "\tVolume GUID:\t" << Partition._file_system.volume_guid << "\n";
			std::cout << "\tLabel:\t" << Partition._file_system.volume_label << "\n";
			std::cout << "\tFormat:\t" << GetFileSystemType(Partition._file_system.type) << "\n";
		}
	}
	std::cout << std::string(columns, '_') << "\n";
	std::cout << "\n";
}

/**
 * @brief Formats a size in bytes into a human-readable string.
 *
 * This function takes a size in bytes and converts it into a string representation in the appropriate unit (KB, MB, GB, or TB).
 * If the size is less than 1 KB, it is represented in bytes. The output string is formatted to two decimal places.
 *
 * @param sizeInBytes The size in bytes to format.
 * @return A string representing the size in the appropriate unit.
 */

std::string formatBytes(uint64_t sizeInBytes) {
	const uint64_t KB = 1024;
	const uint64_t MB = KB * 1024;
	const uint64_t GB = MB * 1024;
	const uint64_t TB = GB * 1024;

	std::ostringstream out;
	out.precision(2); // Set precision to 2 decimal places
	out << std::fixed; // Ensure decimal places are always shown

	if (sizeInBytes >= TB) {
		out << static_cast<double>(sizeInBytes) / TB << " TB";
	}
	else if (sizeInBytes >= GB) {
		out << static_cast<double>(sizeInBytes) / GB << " GB";
	}
	else if (sizeInBytes >= MB) {
		out << static_cast<double>(sizeInBytes) / MB << " MB";
	}
	else if (sizeInBytes >= KB) {
		out << static_cast<double>(sizeInBytes) / KB << " KB";
	}
	else {
		out << sizeInBytes << " bytes";
	}

	return out.str();
}


// Function to set the console window size if it's less than the specified width
void ensureConsoleWidth(int minWidth) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);

	int currentWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	if (currentWidth < minWidth) {
		// Set the console screen buffer size
		COORD bufferSize = { static_cast<SHORT>(minWidth), csbi.dwSize.Y };
		SetConsoleScreenBufferSize(hConsole, bufferSize);

		// Set the console window size
		SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(minWidth - 1), csbi.srWindow.Bottom - csbi.srWindow.Top };
		SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
	}
}



/**
 * @brief Outputs the progress of the restore.
 *
 * This function outputs the progress of the restore based on the total bytes and the bytes processed so far.
 * It calculates the progress percentage and if at least 1 second has passed since the last update or if the progress is 100%,
 * it creates a progress bar and prints it along with the progress percentage and the amount of data processed so far.
 *
 * @param totalBytes The total number of bytes in the process.
 * @param bytessofar The number of bytes processed so far.
 * @param lastUpdateTime A reference to the time point of the last update. This will be updated by the function.
 */
void outputProgress(const uint64_t totalBytes, uint64_t& bytesSoFar, std::chrono::steady_clock::time_point& lastUpdateTime) {
	static uint64_t totalBytesSoFar = 0; // Keep track of total bytes processed so far

	// Ensure the console width is at least 120
	ensureConsoleWidth(120);


	// Get the current time
	auto now = std::chrono::steady_clock::now();

	// Calculate the time difference in seconds
	auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdateTime).count();

	// If at least 1 second has passed since the last update
	if (timeDiff >= 1) {
		totalBytesSoFar += bytesSoFar; // Add bytes processed in this interval to total

		// Calculate the progress percentage
		int progressPercentage = (int)((totalBytesSoFar * 100) / totalBytes);

		// Define the total length of the progress bar
		const int progressBarLength = 50;

		// Calculate how many blocks of the progress bar should be filled
		int filledLength = (int)(progressPercentage / 100.0 * progressBarLength);

		// Ensure filledLength does not exceed the maximum
		if (filledLength > progressBarLength) {
			filledLength = progressBarLength;
		}

		// Create the progress bar string
		std::string progressBar(filledLength, '#');
		progressBar.append(progressBarLength - filledLength, '-');

		// Calculate the transfer rate in Mbits/s or Gbits/s
		double transferRate = (double)bytesSoFar * 8 / timeDiff / 1024 / 1024; // Mbits/s
		std::string rateUnit = "Mb/s";
		if (transferRate > 1024) {
			transferRate /= 1024; // Gbits/s
			rateUnit = "Gb/s";
		}

		// Print the progress bar, transfer rate
		std::cout << "\rProgress: [" << progressBar << "] " << progressPercentage << "% " << formatBytes(totalBytesSoFar) << " / " << formatBytes(totalBytes) << " at " << std::fixed << std::setprecision(2) << transferRate << " " << rateUnit << "       " << std::flush;

		// Reset bytesSoFar and lastUpdateTime for the next interval
		bytesSoFar = 0;
		lastUpdateTime = now;
	}
}
