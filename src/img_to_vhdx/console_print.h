#pragma once

// Prints a disclaimer to the console.
void printDisclaimer();

// Prints the help message.
void printHelp();

// Describes the structure of a backup file.
void describeFile(const file_structs::fileLayout& layout);

// Outputs the progress of the restore.
void outputProgress(const uint64_t totalBytes, uint64_t& bytesSoFar, std::chrono::steady_clock::time_point& lastUpdateTime);
