#pragma once
/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.
===============================================================================
*/

/**
 * @file
 * @brief This is the primary include file for the file operations library.
 *
 * This library provides a set of cross-platform wrapper functions for file operations.
 * These functions provide a higher-level, more convenient interface for performing
 * common file operations such as opening a file, setting the file pointer, getting
 * the current file offset, and reading and writing data from/to a file. They also handle error
 * checking and throw exceptions if any of the operations fail, simplifying error
 * handling in the code that uses these functions.
 *
 * Including this file in a project gives access to all the functionality provided by this library.
 * No other include files are necessary.
 */

#include <fstream>


// A typedef for a std::shared_ptr that holds a std::fstream and uses FileDeleter to automatically close the file when it's no longer in use.
typedef std::shared_ptr<std::fstream> SharedFile;

/**
 * Opens a file and returns a pointer to the file stream.
 *
 * @param filename The name of the file to open.
 * @param read_only A flag indicating whether the file should be opened in read-only mode.
 * @return A pointer to the opened file stream.
 * @throws std::runtime_error if the file could not be opened.
 */
std::fstream* openFile(const std::wstring& filename, bool read_only);

/**
 * Opens a file and returns a shared_ptr to the file stream with a custom deleter.
 *
 * This function opens a file with the given filename and returns a shared_ptr to the file stream.
 * The shared_ptr has a custom deleter that automatically closes the file when the shared_ptr is destroyed.
 * This ensures that the file is always closed properly, even if an exception is thrown.
 *
 * @param filename The name of the file to open.
 * @param read_only A flag indicating whether the file should be opened in read-only mode.
 * @return A shared_ptr to the file stream, with a custom deleter that closes the file.
 */
SharedFile openFileWithGuard(const std::wstring& filename, bool read_only);

/**
 * Sets the file pointer of a file.
 *
 * @param file The file stream.
 * @param offset The new offset of the file pointer.
 * @throws std::runtime_error if the file pointer could not be set.
 */
void setFilePointer(std::fstream* file, std::streamoff offset, std::ios::seekdir pos);

/**
 * Gets the current file offset.
 *
 * This function retrieves the current file offset for the file specified by the file stream.
 *
 * @param file The file stream.
 * @return The current file offset.
 * @throws std::runtime_error if it fails to get the file pointer.
 */
std::streamoff getCurrentFileOffset(std::fstream* file);

/**
 * Reads data from a file.
 *
 * @param file The file stream.
 * @param dataBuffer The buffer that receives the data from the file.
 * @param bytesToRead The number of bytes to be read from the file.
 * @throws std::runtime_error if the file could not be read.
 */
void readFile(std::fstream* file, void* dataBuffer, std::streamsize bytesToRead);

/**
 * Writes data to a file.
 *
 * This function writes the specified number of bytes from the buffer to the file specified by the file stream.
 * If the function fails to write to the file, it throws a runtime error with the corresponding error message.
 *
 * @param file The file stream.
 * @param dataBuffer The buffer containing the data to be written to the file.
 * @param bytesToWrite The number of bytes to be written to the file.
 * @throws std::runtime_error if the file could not be written to.
 */
void writeFile(std::fstream* file, void* dataBuffer, std::streamsize bytesToWrite);
