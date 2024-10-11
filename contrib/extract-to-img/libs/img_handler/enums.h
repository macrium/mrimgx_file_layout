#pragma once

/**
 * @file
 * @brief This file contains the definition of various enums used in the backup file reader library.
 *
 * These enums represent various aspects of a backup file, such as drive type, disk format, backup type, compression type, and more.
 * They are used throughout the library to provide a type-safe way of representing these aspects in the code.
 *
 * The NLOHMANN_JSON_SERIALIZE_ENUM macro is used to map these enum values to their corresponding string values in the JSON structures of the backup file.
 * This allows for easy serialization and deserialization of these values when reading from or writing to a backup file.
 */

#include "../../dependencies/include/nlohmann/json.hpp"

namespace ImageEnums
{
    enum class DriveType
    {
        LOCAL,
        NETWORK,
        CD,
        BLOCKSTORE
    };

    enum class DiskFormat
    {
        eMBR,
        eGPT,
        eDynamic,
        eUnknown
    };

    enum class BackupType
    {
        eFull,
        eIncremental,
        eDifferential,
        eAuto
    };

    enum class FilenamePrefixType
    {
        ePrefixImageID,
        ePrefixISODate
    };

    enum class CompressionType
    {
        eNone,
        eMedium,
        eHigh
    };

    enum class AES
    {
        eNone,
        eStandard,
        eMedium,
        eHigh
    };

    enum class KeyDerivation
    {
        ePBKDF2
    };

    enum class FileType
    {
        ePartition,
        eFileAndFolder
    };

    enum class FileSystemType
    {
        eFileSystemUnknown,
        eFileSystemReFS,
        eFileSystemExFAT,
        eFileSystemNTFS,
        eFileSystemFAT32,
        eFileSystemFAT16,
        eFileSystemFAT12,
        eFileSystemLinuxExt
    };

    enum class CompressionMethod
    {
        eZStd
    };

    enum class ConsolidationType
    {
        eNone,
        eDaily,
        eWeekly,
        eMonthly,
        eSyntheticFull,
        eIncrementalMerge
    };

    enum class CPUPrority
    {
        eIdle = 0,
        eBelowNormal = 1,
        eNormal = 3,
        eAboveNormal
    };

    enum class RetentionType
    {
        eRetentionFull,
        eRetentionDiff,
        eRetentionInc
    };

    enum class RetentionPeriod
    {
        eRetainCount,
        eRetainDays,
        eRetainWeeks
    };

    enum class PartitionType
    {
        Unallocated,
        Primary,
        Logical
    };

    enum class ShutdownLevel
    {
        eNone = -1,
        eShutdown,
        eHibernate,
        eSuspend,
        eReboot
    };

    enum class BitLocker
    {
        eBitLockerNone,
        eBitLockerLocked,
        eBitLockerUnlocked
    };

    enum class BackupFileType
    {
        eUndefinedFileType = -1,
        eImage = 0,
        eBackup,
        eExchange,
        eSQLServer,
        eClone
    };

    enum class Bootable
    {
        eNotSet,
        eYes,
        eNo
    };

    enum class FileAndFolderMatchType
    {
        eSingleMatch,
        eStrict,
        eAny
    };

}

namespace ImageEnums
{
    NLOHMANN_JSON_SERIALIZE_ENUM(
        DriveType,
        {{DriveType::LOCAL, "local"},
         {DriveType::NETWORK, "network"},
         {DriveType::CD, "cd"},
         {DriveType::BLOCKSTORE, "blockstore"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        BackupType,
        {{BackupType::eFull, "full"},
         {BackupType::eDifferential, "diff"},
         {BackupType::eIncremental, "inc"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        DiskFormat,
        {{DiskFormat::eMBR, "mbr"},
         {DiskFormat::eGPT, "gpt"},
         {DiskFormat::eDynamic, "dynamic"},
         {DiskFormat::eUnknown, "unknown"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        FilenamePrefixType,
        {{FilenamePrefixType::ePrefixImageID, "imageid"},
         {FilenamePrefixType::ePrefixImageID, "iso_date"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        CompressionType,
        {{CompressionType::eNone, "none"},
         {CompressionType::eMedium, "medium"},
         {CompressionType::eHigh, "high"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        CompressionMethod,
        {{CompressionMethod::eZStd, "zstd"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        AES,
        {{AES::eNone, "none"},
         {AES::eStandard, "aes-128"},
         {AES::eMedium, "aes-192"},
         {AES::eHigh, "aes-256"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        KeyDerivation,
        {{KeyDerivation::ePBKDF2, "pbkdf2"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        FileType,
        {{FileType::ePartition, "partition"},
         {FileType::eFileAndFolder, "file_and_folder"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        ConsolidationType,
        {{ImageEnums::ConsolidationType::eNone, "none"},
         {ImageEnums::ConsolidationType::eDaily, "daily"},
         {ImageEnums::ConsolidationType::eWeekly, "weekly"},
         {ImageEnums::ConsolidationType::eMonthly, "monthly"},
         {ImageEnums::ConsolidationType::eSyntheticFull, "synthetic_full"},
         {ImageEnums::ConsolidationType::eIncrementalMerge, "incremental_merge"}}

    );

    NLOHMANN_JSON_SERIALIZE_ENUM(
        CPUPrority,
        {{CPUPrority::eIdle, "idle"},
         {CPUPrority::eBelowNormal, "below_normal"},
         {CPUPrority::eNormal, "normal"},
         {CPUPrority::eAboveNormal, "above_normal"}}

    );

    NLOHMANN_JSON_SERIALIZE_ENUM(
        RetentionPeriod,
        {{RetentionPeriod::eRetainCount, "count"},
         {RetentionPeriod::eRetainDays, "days"},
         {RetentionPeriod::eRetainWeeks, "weeks"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        PartitionType,
        {{PartitionType::Unallocated, "unallocated"},
         {PartitionType::Primary, "primary"},
         {PartitionType::Logical, "logical"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        ShutdownLevel,
        {{ShutdownLevel::eNone, "none"},
         {ShutdownLevel::eShutdown, "shutdown"},
         {ShutdownLevel::eHibernate, "hibernate"},
         {ShutdownLevel::eSuspend, "suspend"},
         {ShutdownLevel::eReboot, "reboot"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        BitLocker,
        {{BitLocker::eBitLockerNone, "none"},
         {BitLocker::eBitLockerLocked, "locked"},
         {BitLocker::eBitLockerUnlocked, "unlocked"}});

    NLOHMANN_JSON_SERIALIZE_ENUM(
        BackupFileType,
        {{BackupFileType::eUndefinedFileType, "undefined"},
         {BackupFileType::eImage, "image"},
         {BackupFileType::eBackup, "file_and_folder"},
         {BackupFileType::eExchange, "exchange"},
         {BackupFileType::eSQLServer, "sql_server"},
         {BackupFileType::eClone, "clone"}});

    //	enum class FileSystemType { eFileSystemUnknown, eFileSystemReFS, eFileSystemExFAT, eFileSystemNTFS, eFileSystemFAT32, eFileSystemFAT16, eFileSystemFAT12, eFileSystemLinuxExt };

    NLOHMANN_JSON_SERIALIZE_ENUM(
        FileSystemType,
        {{FileSystemType::eFileSystemUnknown, "unknown"},
         {FileSystemType::eFileSystemReFS, "ReFS"},
         {FileSystemType::eFileSystemExFAT, "exFAT"},
         {FileSystemType::eFileSystemNTFS, "NTFS"},
         {FileSystemType::eFileSystemFAT32, "FAT32"},
         {FileSystemType::eFileSystemFAT16, "FAT16"},
         {FileSystemType::eFileSystemFAT12, "FAT12"},
         {FileSystemType::eFileSystemLinuxExt, "ext"}});

    enum class MediaType
    {
        RemovableMedia,
        FixedMedia,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(
        MediaType,
        {{MediaType::RemovableMedia, "removable_media"},
         {MediaType::FixedMedia, "fixed_media"}})

    MediaType GetMediaType(int Type);
    bool GetBootable(ImageEnums::Bootable Type);

    NLOHMANN_JSON_SERIALIZE_ENUM(
        FileAndFolderMatchType,
        {{FileAndFolderMatchType::eSingleMatch, "single_match"},
         {FileAndFolderMatchType::eStrict, "strict"},
         {FileAndFolderMatchType::eAny, "any"}});
}

std::string GetCompressionText(ImageEnums::CompressionType type);
std::string GetEncryptionText(ImageEnums::AES type);
std::string GetDiskFormatText(ImageEnums::DiskFormat type);
std::string GetFileSystemType(ImageEnums::FileSystemType type);