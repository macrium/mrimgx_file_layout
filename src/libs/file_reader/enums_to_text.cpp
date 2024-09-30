#include "pch.h"
#include "file_structs.h"
#include "enums.h"

/**
 * @file
 * @brief This file contains functions that convert enumeration values to text.
 */

std::string GetCompressionText(ImageEnums::CompressionType type)
{
	switch (type)
	{
	case ImageEnums::CompressionType::eNone:
		return "none";
	case ImageEnums::CompressionType::eMedium:
		return "medium";
	case ImageEnums::CompressionType::eHigh:
		return "high";
	default:
		return "unknown";
	}
}

std::string GetEncryptionText(ImageEnums::AES type)
{
	switch (type)
	{
	case ImageEnums::AES::eNone:
		return "none";
	case ImageEnums::AES::eStandard:
		return "aes-128";
	case ImageEnums::AES::eMedium:
		return "aes-192";
	case ImageEnums::AES::eHigh:
		return "aes-256";
	default:
		return "unknown";
	}
}

std::string GetDiskFormatText(ImageEnums::DiskFormat type)
{
	switch (type)
	{
	case ImageEnums::DiskFormat::eMBR:
		return "mbr";
	case ImageEnums::DiskFormat::eGPT:
		return "gpt";
	case ImageEnums::DiskFormat::eDynamic:
		return "dynamic";
	default:
		return "unknown";
	}
}

std::string GetFileSystemType(ImageEnums::FileSystemType type)
{
	switch (type)
	{
	case ImageEnums::FileSystemType::eFileSystemReFS:
		return "ReFS";
	case ImageEnums::FileSystemType::eFileSystemNTFS:
		return "NTFS";
	case ImageEnums::FileSystemType::eFileSystemFAT32:
		return "FAT32";
	case ImageEnums::FileSystemType::eFileSystemFAT16:
		return "FAT16";
	case ImageEnums::FileSystemType::eFileSystemFAT12:
		return "FAT12";
	case ImageEnums::FileSystemType::eFileSystemExFAT:
		return "ExFAT";
	case ImageEnums::FileSystemType::eFileSystemLinuxExt:
		return "Linux Ext";
	default:
		return "unknown";
	}
}