// file_operations.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.

This file defines a set of cross-platform wrapper functions for file operations.
These functions provide a higher-level, more convenient interface for performing
common file operations such as opening a file, setting the file pointer, getting
the current file offset, and reading data from a file. They also handle error
checking and throw exceptions if any of the operations fail, simplifying error
handling in the code that uses these functions.

Using cross-platform file operations is important as it allows the code to be
portable and run on different operating systems without requiring system-specific
implementations. This increases the maintainability and scalability of the code,
making it more robust and versatile for various use cases and environments.
===============================================================================
*/

// A custom deleter for std::fstream.
// This deleter will be used with std::shared_ptr to automatically close files when they are no longer in use.
struct fileDeleter {
    void operator()(std::fstream* file) const {
        if (file) {
            file->close();
            delete file;
        }
    }
};

// Defines a type for a shared pointer to a std::fstream.
// This shared pointer will automatically delete the std::fstream 
// and close the file when it goes out of scope.
typedef std::shared_ptr<std::fstream> SharedFile;


/**
 * Converts a wide string to a string.
 *
 * This function is used to convert wide strings (std::wstring) to strings (std::string).
 * It's particularly useful when you need to use a wide string in a context that requires a standard string.
 *
 * @param wstr The wide string to convert.
 * @ret
 */
std::string wideToString(const std::wstring& wstr) {
    std::string str(wstr.length(), ' ');
    size_t convertedChars = 0;
    wcstombs_s(&convertedChars, &str[0], str.size(), wstr.c_str(), _TRUNCATE);
    return str;
}

/**
 * Opens a file and returns a pointer to the file stream.
 *
 * @param filename The name of the file to open.
 * @param filename The name of the file to open.
 * @param read_only A flag indicating whether the file should be opened in read-only mode.
 * @return A pointer to the opened file stream.
 * @throws std::runtime_error if the file could not be opened.
 */
std::fstream* openFile(const std::wstring& filename, bool read_only) {
    std::fstream* file = new std::fstream();
    if (read_only)
		file->open(filename, std::ios::in | std::ios::binary);
	else
		file->open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file->is_open()) {
        char errorBuffer[100];
        strerror_s(errorBuffer, sizeof(errorBuffer), errno);
        std::string errorMessage = "Could not open file: " + wideToString(filename);
        errorMessage += ". Error: ";
        errorMessage += errorBuffer;
        delete file;
        throw std::runtime_error(errorMessage);
    }
    return file;
}
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
SharedFile openFileWithGuard(const std::wstring& filename, bool read_only) {
    if (filename.empty()) {
        throw std::invalid_argument("Filename cannot be empty.");
    }
    std::fstream* file = openFile(filename, read_only);
    return SharedFile(file, fileDeleter());
}

/**
 * Sets the file pointer of a file.
 *
 * @param file The file stream.
 * @param offset The new offset of the file pointer.
 * @throws std::runtime_error if the file pointer could not be set.
 */
void setFilePointer(std::fstream* file, std::streamoff offset, std::ios::seekdir pos) {
    file->seekg(offset, pos);
    if (file->fail()) {
        throw std::runtime_error("Failed to set file pointer.");
    }
}

/**
 * Gets the current file offset.
 *
 * This function retrieves the current file offset for the file specified by the file stream.
 *
 * @param file The file stream.
 * @return The current file offset.
 * @throws std::runtime_error if it fails to get the file pointer.
 */
std::streamoff getCurrentFileOffset(std::fstream* file) {
    std::streamoff offset = file->tellp();
    if (offset == -1) {
        throw std::runtime_error("Failed to get file pointer.");
    }
    return offset;
}

/**
 * Reads data from a file.
 *
 * @param file The file stream.
 * @param dataBuffer The buffer that receives the data from the file.
 * @param bytesToRead The number of bytes to be read from the file.
 * @throws std::runtime_error if the file could not be read.
 */
void readFile(std::fstream* file, void* dataBuffer, std::streamsize bytesToRead) {
    if (!file->is_open()) {
        throw std::runtime_error("Failed to read from file because the file stream is not open.");
    }
    file->read(static_cast<char*>(dataBuffer), bytesToRead);
    if (file->fail()) {
        std::string errorMessage = "Failed to read from file.";
        if (file->eof()) {
            errorMessage += " Attempted to read past the end of the file.";
        }
        else if (file->bad()) {
            errorMessage += " There was an error in the underlying I/O stream.";
        }
        else {
            errorMessage += " The operation failed due to an unspecified error.";
        }
        throw std::runtime_error(errorMessage);
    }
}

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
void writeFile(std::fstream* file, void* dataBuffer, std::streamsize bytesToWrite) {
    if (!file->is_open()) {
        throw std::runtime_error("Failed to write to file because the file stream is not open.");
    }
    file->write(static_cast<char*>(dataBuffer), bytesToWrite);
    if (file->fail()) {
        std::string errorMessage = "Failed to write to file.";
        if (file->eof()) {
            errorMessage += " Attempted to write past the end of the file.";
        }
        else if (file->bad()) {
            errorMessage += " There was an error in the underlying I/O stream.";
        }
        else {
            errorMessage += " The operation failed due to an unspecified error.";
        }
        throw std::runtime_error(errorMessage);
    }
}
