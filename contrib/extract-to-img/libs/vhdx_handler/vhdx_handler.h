#pragma once


#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <string>

void CreateVDisk(std::wstring path, unsigned long long size, unsigned long sectorSize);

void MountVDisk(std::wstring path, std::wstring& diskPath);

void UpdateDiskProperties(std::wstring diskPath);