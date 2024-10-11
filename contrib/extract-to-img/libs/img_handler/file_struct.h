#pragma once

#include <string.h>
#include <cstdint>
#include "enums.h"

#pragma pack(push, 1)   // Prevents DataBlockIndexElement from being padded to 32 bytes

struct DataBlockIndexElement
{
	int64_t  file_position;
	uint8_t  md5_hash[16];
	uint32_t block_length;
	uint16_t file_number;
};

struct DeltaDataBlockIndexElement
{
    DataBlockIndexElement data_block;
    uint32_t block_index;
};

namespace file_structs
{
    namespace Partition
    {
        struct File_System
        {
            ImageEnums::BitLocker bitlocker_state;
            char drive_letter;
            uint64_t end;
            uint32_t free_clusters;
            uint16_t lcn0_file_number;
            uint64_t lcn0_offset;
            uint64_t mft_offset;
            uint32_t mft_record_size;
            uint32_t partition_index;
            uint32_t reserved_sectors_byte_length = 0;
            uint32_t sectors_per_cluster;
            std::string shadow_copy;
            uint64_t start;
            uint32_t total_clusters;
            ImageEnums::FileType type;
            std::string volume_guid;
            std::string volume_label;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(File_System, type, mft_offset, mft_record_size, drive_letter, end, start, free_clusters, lcn0_offset,
            sectors_per_cluster, total_clusters, volume_guid, volume_label, lcn0_file_number, bitlocker_state, partition_index, shadow_copy, reserved_sectors_byte_length);

        struct Geometry
        {
            uint64_t boot_sector_offset;
            uint64_t end;
            uint64_t length;
            uint64_t start;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Geometry, start, end, length, boot_sector_offset)

        struct File_History
        {
            std::string file_name;
            int32_t file_number;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(File_History, file_name, file_number)

        struct Header
        {
            uint32_t block_count;
            uint32_t block_size;
            std::vector<File_History> file_history;
            uint32_t file_history_count;
            uint64_t partition_file_offset;
            int32_t partition_number;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Header, block_count, block_size, file_history, file_history_count,
             partition_file_offset, partition_number)

        struct Table_Entry
        {
            bool active;
            uint32_t boot_sector;
            uint16_t end_cylinder;
            uint8_t end_head;
            uint32_t num_sectors;
            ImageEnums::PartitionType partition_type;
            uint16_t start_cylinder;
            uint8_t start_head;
            uint8_t status;
            uint8_t type;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Table_Entry, boot_sector, end_cylinder, end_head, num_sectors, partition_type, 
            start_cylinder, start_head, status, type)
        
        struct Partition_Layout
        {
            File_System _file_system;
            Geometry _geometry;
            Header _header;
            Table_Entry _partition_table_entry;
            
            std::vector<DataBlockIndexElement> reserved_sectors;
            std::vector<DataBlockIndexElement> data_block_index;

            std::vector<DeltaDataBlockIndexElement> delta_data_block_index;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Partition_Layout, _file_system, _geometry, _header, _partition_table_entry)
    };

    namespace Disk
    {
        struct Descriptor
        {
            std::string disk_description;
            std::string disk_manufacturer;
            std::string disk_productid;
            std::string disk_revisonno;
            std::string disk_serialno;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Descriptor, disk_description, disk_manufacturer, disk_productid, disk_revisonno, disk_serialno)

        struct Geometry
        {
            uint32_t bytes_per_sector;
            uint64_t cylinders;
            uint64_t disk_size;
            std::string media_type;
            uint32_t sectors_per_track;
            uint32_t tracks_per_cylinder;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Geometry, bytes_per_sector, cylinders, disk_size, media_type, sectors_per_track,
            tracks_per_cylinder)

        struct Header
        {
            std::string disk_format;
            uint32_t disk_number;
            std::string disk_signature;
            int32_t imaged_partition_count;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Header, disk_format, disk_number, disk_signature, imaged_partition_count)

        struct Disk_Layout
        {
            Descriptor _descriptor;
            Geometry _geometry;
            Header _header;
            std::vector<Partition::Partition_Layout> partitions;

            std::vector <uint8_t>  track0;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Disk_Layout, _descriptor, _geometry, _header, partitions)
    };

    struct Header
    {
        std::string backup_format;
        std::string backup_guid;
        uint64_t backup_time;
        std::string backup_type;
        uint64_t backupset_time;
        bool delta_index;
        uint16_t file_number;
        uint16_t imaged_disks_count;
        std::string imageid;
        uint16_t increment_number;
        uint64_t index_file_position;
        int32_t json_version;
        std::string netbios_name;
        bool split_file;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Header, backup_format, backup_guid, backup_time, backup_type, backupset_time, delta_index,
        file_number, imaged_disks_count, imageid, increment_number, index_file_position, json_version, netbios_name, split_file)

    struct Compression
    {
        std::string compression_level;
        std::string compression_method;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Compression, compression_level, compression_method)

    struct Encryption
    {
        bool enable;
        uint32_t key_iterations;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Encryption, enable, key_iterations)

    struct File_Layout
    {
        Compression _compression;
        Encryption _encryption;
        Header _header;
        std::vector<Disk::Disk_Layout> disks;

    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(File_Layout, _compression, _encryption, _header, disks)
}

#pragma pack(pop)