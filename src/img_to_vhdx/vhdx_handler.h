#pragma once

// Function to handle VHDX file creation and mounting
VHDXManager handleVHDXFile(const std::wstring& filename, const std::wstring& outputPath, std::wstring& vhdxName, const file_structs::fileLayout& backupFile, int diskNumber);
