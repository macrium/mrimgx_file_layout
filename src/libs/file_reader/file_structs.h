#pragma once

#include "enums.h"
#include "disk_structs.h"

#include "..\encryption\encryption.h"

#pragma pack(push, 1)

/**
 * @file
 * @brief This file contains the definition of various data structures used in the backup file reader library.
 *
 * These data structures represent various components of a backup file, such as the header, metadata blocks, and data blocks.
 * They are used throughout the library to provide a type-safe way of representing these components in the code.
 *
 * The NLOHMANN_JSON_SERIALIZE_ENUM macro is used to map these data structure values to their corresponding string values in the JSON structures of the backup file.
 * This allows for easy serialization and deserialization of these values when reading from or writing to a backup file.
 */


 /**
  * @struct DataBlockIndexElement
  * @brief  Represents a data block in the file.
  *
  * @var    file_position
  *         Position of the data block in the file (64-bit integer).
  *
  * @var    md5_hash
  *         MD5 hash of the data block (16-byte array).
  *
  * @var    block_length
  *         Length of the data block.
  *
  * @var    file_number
  *         Number of the file (16-bit integer).
  */
struct DataBlockIndexElement
{
	int64_t  file_position;
	uint8_t  md5_hash[16];
	uint32_t block_length;
	uint16_t file_number;
};


/**
 * @struct DeltaDataBlock
 * @brief  A structure representing a delta data block in the file.
 *
 * @var    DeltaDataBlock::datablock
 *         An object representing a data block.
 *
 * @var    DeltaDataBlock::index
 *         The index of the delta data block.
 */
struct DeltaDataBlock
{
public:
	DataBlockIndexElement delta_data_block;
	uint32_t   block_index;
};


#pragma pack(pop)


/**
 * @namespace file_structs
 * @brief Contains JSON data structures for representing data in Macrium X backup files.
 *
 * The structs are populated using the nlohmann json library. For the JSON schema,
 * including additional field information, see:
 * https://github.com/macriumsoftware/mrimg_file_layout/tree/main/schema
 *
 * Fields marked as [REQUIRED] are necessary for full data recovery. All other fields
 * provide supplementary information.
 */
namespace file_structs
{
	namespace Partition
	{
		// partition table entry data
		struct PartitionEntry
		{
			uint8_t  status = 0;
			uint8_t  start_head = 0;
			uint16_t  start_cylinder = 0;
			uint8_t  type = 0;
			uint8_t  end_head = 0;
			uint16_t  end_cylinder = 0;
			uint32_t boot_sector = 0;
			uint32_t num_sectors = 0;
			bool  active = false;
			ImageEnums::PartitionType partition_type = ImageEnums::PartitionType::Primary;
		};

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
			PartitionEntry, status, start_head, start_cylinder, end_head, end_cylinder, boot_sector, num_sectors, active, type, partition_type);

		// backup set file history data
		struct FileHistory
		{
			std::string file_name;
			int32_t file_number = 0;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FileHistory, file_name, file_number);

		// partition header data. 
		struct PartitionHeader
		{
			uint32_t block_count = 0;
			uint32_t block_size = 0; // [REQUIRED]
			uint64_t partition_file_offset = 0;
			uint32_t file_history_count = 0;
			int32_t partition_number = 0; // [REQUIRED]
			std::vector<FileHistory> file_history;
		};

		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
			PartitionHeader, block_count, block_size, partition_file_offset, partition_number, file_history_count, file_history);


		// file system data
		struct FileSystem
		{
			char drive_letter = 0;
			uint64_t end = 0;
			uint64_t start = 0;
			uint64_t fat_offset = 0;
			uint32_t free_clusters = 0;
			uint64_t lcn0_offset = 0; // [REQUIRED]
			uint32_t linux_blocks_per_group = 0;
			uint32_t linux_group_count = 0;
			uint64_t mft_offset = 0;
			uint32_t mft_record_size = 0;
			uint32_t reserved_sectors_byte_length = 0; // [REQUIRED]
			uint32_t sectors_per_cluster = 0;
			uint32_t total_clusters = 0;
			int32_t partition_index = 0;
			std::string volume_guid;
			std::string volume_label;
			std::string shadow_copy;
			uint16_t lcn0_file_number = 0;
			ImageEnums::BitLocker bitlocker_state = ImageEnums::BitLocker::eBitLockerNone;
			ImageEnums::FileSystemType type = ImageEnums::FileSystemType::eFileSystemUnknown;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FileSystem, type, linux_blocks_per_group, linux_group_count, mft_offset, mft_record_size, fat_offset, drive_letter, end, start, free_clusters, lcn0_offset,
			reserved_sectors_byte_length, sectors_per_cluster, total_clusters, volume_guid, volume_label, lcn0_file_number, bitlocker_state, partition_index, shadow_copy);


		// partition geometry - offsets and lengths
		struct PartitionGeometry
		{
			uint64_t start = 0; // [REQUIRED]
			uint64_t end = 0;
			uint64_t length = 0;
			uint64_t boot_sector_offset = 0; // [REQUIRED]
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PartitionGeometry, start, end, length, boot_sector_offset)


		// container for the partition structs	
		struct PartitionLayout
		{
			PartitionHeader _header; // [REQUIRED]
			PartitionGeometry _geometry; // [REQUIRED]
			PartitionEntry _partition_table_entry;
			FileSystem _file_system; // [REQUIRED]
			std::vector <DataBlockIndexElement> data_blocks; // [REQUIRED]
			std::vector <DeltaDataBlock> delta_data_blocks; // [REQUIRED]
			std::vector <DataBlockIndexElement> reserved_sectors_blocks; // [REQUIRED]
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PartitionLayout, _header, _geometry, _partition_table_entry, _file_system);
	}

	namespace Disk
	{
		// disk header
		struct DiskHeader
		{
			std::string disk_signature;
			ImageEnums::DiskFormat disk_format = ImageEnums::DiskFormat::eMBR; // [REQUIRED]
			int32_t disk_number = 0; // [REQUIRED]
			uint32_t extended_partition_sector_offset = 0;
			int32_t extended_partition_count = 0;
			int32_t imaged_partition_count = 0;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DiskHeader, extended_partition_sector_offset, extended_partition_count, disk_format, disk_signature, disk_number, imaged_partition_count);

		// from IOCTL_DISK_GET_DRIVE_GEOMETRY_EX
		struct DiskGeometry
		{
			int32_t bytes_per_sector = 0; // [REQUIRED]
			uint64_t cylinders = 0;
			ImageEnums::MediaType media_type = ImageEnums::MediaType::FixedMedia;
			int32_t sectors_per_track = 0;
			int32_t tracks_per_cylinder = 0;
			uint64_t disk_size = 0; // [REQUIRED]
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DiskGeometry, bytes_per_sector, cylinders, media_type, sectors_per_track, tracks_per_cylinder, disk_size);

		// from IOCTL_STORAGE_QUERY_PROPERTY  
		struct DiskDescriptor
		{
			std::string disk_description;
			std::string disk_manufacturer;
			std::string disk_productid;
			std::string disk_revisonno;
			std::string disk_serialno;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DiskDescriptor, disk_description, disk_manufacturer, disk_productid, disk_revisonno, disk_serialno);

		struct DiskLayout
		{
			DiskHeader _header; // [REQUIRED]
			DiskGeometry _geometry; // [REQUIRED]
			DiskDescriptor _descriptor;
			std::vector<Partition::PartitionLayout> partitions; // [REQUIRED]

			// supplemental fields from  $TRACK0 & $EPTadded after parsing from 
			std::vector <uint8_t>  track0;
			std::vector<ExtendedPartition> extendedPartitions;
		};
		NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DiskLayout, _header, _geometry, _descriptor, partitions);

	}


	struct PostBackup
	{
		bool enable = false;
		ImageEnums::ShutdownLevel level = ImageEnums::ShutdownLevel::eNone;
		bool force = false;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(PostBackup, enable, level, force);

	struct Retention
	{
		bool enable = false;
		ImageEnums::RetentionPeriod period = ImageEnums::RetentionPeriod::eRetainCount;
		uint32_t interval = 0;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Retention, enable, period, interval);

	struct DiskSpaceManagement
	{
		bool low_space_purge = false;
		uint32_t low_space_gb_threshold = 0;
		bool incrementals_forever = false;
		bool apply_to_all_sets_in_folder = false;
		bool run_before = false;

		Retention full_retention;
		Retention incremental_retention;
		Retention differential_retention;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DiskSpaceManagement, low_space_purge, low_space_gb_threshold, run_before,
		incrementals_forever, apply_to_all_sets_in_folder, full_retention, incremental_retention, differential_retention);


	struct viBootMap
	{
		int32_t system_disk = 0;
		int32_t viboot_disk = 0;
		std::string cookie;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(viBootMap, system_disk, viboot_disk);

	struct viBoot
	{
		std::map<int32_t, int32_t> disk_map;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(viBoot, disk_map);

	struct bdfEmail
	{
		bool success_enable = false;
		bool success_attach_log = false;
		bool success_attach_vss_log = false;
		std::string success_recipients;
		std::string success_subject;
		std::string success_content;
		bool warning_enable = false;
		bool warning_attach_log = false;
		bool warning_attach_vss_log = false;
		std::string warning_recipients;
		std::string warning_subject;
		std::string warning_content;
		bool failure_enable = false;
		bool failure_include_cancelled = false;
		bool failure_include_skipped = false;
		bool failure_attach_log = false;
		bool failure_attach_vss_log = false;
		std::string failure_recipients;
		std::string failure_subject;
		std::string failure_content;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(bdfEmail, success_enable, success_attach_log, success_attach_vss_log, success_recipients, success_subject, success_content, warning_enable, warning_attach_log, warning_attach_vss_log, warning_recipients, warning_subject, warning_content, failure_enable, failure_include_cancelled, failure_include_skipped, failure_attach_log, failure_attach_vss_log, failure_recipients, failure_subject, failure_content);



	struct bdfDisk
	{
		int32_t number = 0;
		std::string signature;
		std::vector<int32_t> partitions;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(bdfDisk, number, signature, partitions);



	struct bdfFolderFilters
	{
		std::string path;
		bool is_folder_source = false;
		bool is_recursive = false;
		std::string include_files;
		std::string exclude_files;
		std::string include_folders;
		std::string exclude_folders;
		bool exclude_system_files = false;
		bool exclude_hidden_files = false;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(bdfFolderFilters, path, is_folder_source, is_recursive, include_files, exclude_files, include_folders, exclude_folders, exclude_system_files, exclude_hidden_files);


	struct bdfFileAndFolder
	{
		bool backup_ntfs_permissions = false;
		bool follow_system_junction_points = false;
		bool follow_user_junction_points = false;
		ImageEnums::FileAndFolderMatchType match_type = ImageEnums::FileAndFolderMatchType::eSingleMatch;
		std::vector<bdfFolderFilters> folder_filters;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(bdfFileAndFolder, backup_ntfs_permissions, follow_system_junction_points, follow_user_junction_points, match_type, folder_filters);


	struct BackupDefinition
	{
		ImageEnums::BackupFileType file_type = ImageEnums::BackupFileType::eImage;
		ImageEnums::FileType backup_format = ImageEnums::FileType::ePartition;
		std::string filename;
		std::string backup_definition_file;
		std::string comment;
		std::string filename_template;
		ImageEnums::CompressionType compression_level = ImageEnums::CompressionType::eMedium;
		bool intelligent_sector_copy = true;
		bool prefix_unique = false;
		ImageEnums::ConsolidationType consolidation_type = ImageEnums::ConsolidationType::eNone;
		ImageEnums::CPUPrority cpu_priority = ImageEnums::CPUPrority::eBelowNormal;

		std::vector <std::string> alternative_locations;
		std::vector <bdfDisk> disks;

		bool auto_verify = false;
		bool check_filesystem = false;
		bool limit_file_size = false;
		uint64_t file_size_limit = 0;
		int32_t rate_limit = -1;
		PostBackup post_backup_shutdown;

		DiskSpaceManagement disk_space_management;
		std::string site_manager_cookie;
		bool sitemanager_cookie_enabled = false;
		viBoot viboot;
		bool viboot_enabled = false;

		bdfEmail email;
		bool email_enabled = false;
		bdfFileAndFolder file_and_folder;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(BackupDefinition, file_type, backup_format, filename, backup_definition_file, comment, filename_template, compression_level, intelligent_sector_copy, prefix_unique, consolidation_type, cpu_priority, alternative_locations, disks, auto_verify, check_filesystem, limit_file_size, file_size_limit, rate_limit, post_backup_shutdown, disk_space_management, site_manager_cookie, sitemanager_cookie_enabled, viboot, viboot_enabled, email, email_enabled, file_and_folder);


	struct Header
	{
		std::string imageid; // [REQUIRED]
		uint8_t imageid_binary[8] = { 0 }; // [REQUIRED]
		uint16_t file_number = 0; // [REQUIRED]
		uint16_t increment_number = 0; // [REQUIRED]
		uint16_t imaged_disks_count = 0;
		std::vector<int32_t> merged_files; // REQUIRED
		bool split_file = false; // [REQUIRED]
		std::string netbios_name;
		__time64_t backup_time = 0;
		__time64_t backupset_time = 0;
		std::string backup_guid;
		uint64_t index_file_position = 0; // [REQUIRED]
		bool delta_index = true; // [REQUIRED]
		int32_t json_version = 1;
		ImageEnums::BackupType backup_type = ImageEnums::BackupType::eFull;
		ImageEnums::FileType backup_format = ImageEnums::FileType::ePartition;


		void set_imageid_binary() {
			if (imageid.length() == 16) {
				for (int i = 0; i < 8; ++i) {
					std::string byteString = imageid.substr(i * 2, 2);
					imageid_binary[i] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16));
				}
			}
		}
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Header, imageid, file_number, increment_number, imaged_disks_count, merged_files, split_file, netbios_name, backup_time, backupset_time, backup_guid, index_file_position, delta_index, backup_type, backup_format, json_version);

	struct ReflectVersionInfo
	{
		int major = 0;
		int minor = 0;
		int build = 0;
		int file_version = 0;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ReflectVersionInfo, major, minor, build, file_version);


	struct softwareVersion
	{
		std::string name_and_version;
		void in();
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(softwareVersion, name_and_version);


	struct AuxiliaryData
	{
		uint64_t highest_file_offset = 0;
		ImageEnums::DriveType target_drive_type = ImageEnums::DriveType::LOCAL;
		std::string boot_partition;
		std::string system_partition;
		int32_t collision_number = 0;
		ImageEnums::FilenamePrefixType unique_prefix_type = ImageEnums::FilenamePrefixType::ePrefixImageID;
		std::string destination;
		ReflectVersionInfo macrium_reflect;
		std::vector<softwareVersion> third_party_software_version_list;
		BackupDefinition backup_definition;
		bool bootable = false;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AuxiliaryData, bootable, boot_partition, system_partition, highest_file_offset, target_drive_type, collision_number, unique_prefix_type,
		destination, macrium_reflect, backup_definition);

	struct Encryption
	{
		bool enable = false; // [REQUIRED]
		int32_t key_iterations = 0; // [REQUIRED]
		std::string hmac = ""; // [REQUIRED]
		std::array<uint8_t, KEY_LENGTH> hmac_binary = { 0 }; // [REQUIRED]
		std::array<uint8_t, KEY_LENGTH> derived_key = { 0 }; // [REQUIRED]
		ImageEnums::AES aes_type = ImageEnums::AES::eNone;	 // [REQUIRED]

		ImageEnums::KeyDerivation key_derivation = ImageEnums::KeyDerivation::ePBKDF2;
		void convert_hmac_to_binary() {
			if (hmac.length() == 64) {
				for (int i = 0; i < 32; ++i) {
					std::string byteString = hmac.substr(i * 2, 2);
					hmac_binary[i] = static_cast<uint8_t>(std::stoul(byteString, nullptr, 16)); // Assign value to array element
				}
			}
		}
		int getAESValue() const {
			switch (aes_type) {
			case ImageEnums::AES::eStandard:
				return 10;
			case ImageEnums::AES::eMedium:
				return 12;
			case ImageEnums::AES::eHigh:
				return 14;
			default:
				throw std::invalid_argument("Invalid AES type");
			};
		}
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Encryption, enable, key_iterations, hmac, aes_type, key_derivation);

	struct Compression
	{
		ImageEnums::CompressionMethod compression_method = ImageEnums::CompressionMethod::eZStd;
		ImageEnums::CompressionType compression_level = ImageEnums::CompressionType::eMedium; // [REQUIRED]
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Compression, compression_method, compression_level)

	struct fileLayout
	{
		Header _header; // [REQUIRED]
		AuxiliaryData _auxiliary_data;
		Encryption _encryption; // [REQUIRED]
		Compression _compression; // [REQUIRED]
		std::vector<Disk::DiskLayout> disks; // [REQUIRED]

		// supplemental fields added after parsing
		std::wstring file_name;
		std::string jsonStr;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(fileLayout, _header, _auxiliary_data, _encryption, _compression, disks)
};
