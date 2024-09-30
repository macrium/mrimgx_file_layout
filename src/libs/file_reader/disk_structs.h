#pragma once
#include <cstdint>

#pragma pack(push, 1)
/******************************************************************************************************/
/* Disk Partition Structures
/******************************************************************************************************/

typedef struct Partition {
    uint8_t  status;
    uint8_t  startHead;
    uint16_t startSectorCylinder;
    uint8_t  type;
    uint8_t  endHead;
    uint16_t endSectorCylinder;
    uint32_t bootSectorOffset;   // Offset to Partition Boot Sector from Partition Table
    uint32_t numberOfSectors;
    // ...
} Partition;

typedef struct BootRecord {
    uint8_t  bootCode[442];
    uint32_t signature;
    Partition partitionEntries[4];
    uint16_t idCode;
} BootRecord;

typedef struct BootTrack {
    BootRecord masterBootRecord;
    uint8_t  track0[31744];
} BootTrack;

#pragma pack(pop)

typedef struct ExtendedPartition {
    BootRecord partitionSector;
    uint64_t offset;
    uint16_t number;
} ExtendedPartition;


#pragma pack(push, 1)
/******************************************************************************************************/
// GPT Structures
#define EFI_PMBR_OSTYPE_EFI 0xEF
#define EFI_PMBR_OSTYPE_EFI_GPT 0xEE
#define MSDOS_MBR_SIGNATURE 0xaa55
#define GPT_BLOCK_SIZE 512

#ifdef _LINUX
#define GPT_HEADER_SIGNATURE 0x5452415020494645ULL
#else
#define GPT_HEADER_SIGNATURE 0x5452415020494645
#endif

#define GPT_HEADER_REVISION_V1_02 0x00010200
#define GPT_HEADER_REVISION_V1_00 0x00010000
#define GPT_HEADER_REVISION_V0_99 0x00009900
#define GPT_PRIMARY_PARTITION_TABLE_LBA 1

typedef struct _efi_guid_t {
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];

	bool operator == (const _efi_guid_t& that) const
	{
		if (memcmp(data4, that.data4, sizeof(data4)) != 0) {
			return false;
		}
		return data1 == that.data1
			&& data2 == that.data2
			&& data3 == that.data3;
	}
} efi_guid_t;

typedef efi_guid_t EFI_GUID_T;
typedef efi_guid_t* PEFI_GUID_T;
typedef efi_guid_t* LPEFI_GUID_T;


typedef struct _gpt_header {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t header_crc32;
	uint32_t reserved1;
	uint64_t my_lba;
	uint64_t alternate_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	efi_guid_t disk_guid;
	uint64_t partition_entry_lba;
	uint32_t num_partition_entries;
	uint32_t sizeof_partition_entry;
	uint32_t partition_entry_array_crc32;
	uint8_t reserved2[GPT_BLOCK_SIZE - 92];
}  gpt_header;
typedef gpt_header GPT_HEADER;
typedef gpt_header* PGPT_HEADER;
typedef gpt_header* LPGPT_HEADER;

typedef struct _gpt_entry_attributes {
	uint64_t required_to_function : 1;
	uint64_t reserved : 47;
	uint64_t type_guid_specific : 16;

	bool operator == (const _gpt_entry_attributes& that) const
	{
		return required_to_function == that.required_to_function
			&& reserved == that.reserved
			&& type_guid_specific == that.type_guid_specific;
	}
}  gpt_entry_attributes;
typedef gpt_entry_attributes GPT_ENTRY_ATTRIBUTES;
typedef gpt_entry_attributes* PGPT_ENTRY_ATTRIBUTES;
typedef gpt_entry_attributes* LPGPT_ENTRY_ATTRIBUTES;

typedef struct _gpt_entry {
	efi_guid_t partition_type_guid = {};
	efi_guid_t unique_partition_guid = {};
	uint64_t starting_lba = 0;
	uint64_t ending_lba = 0;
	gpt_entry_attributes attributes;
	uint16_t partition_name[72 / sizeof(uint16_t)] = {};

	bool operator == (const _gpt_entry& that) const
	{
		int size = 72 / sizeof(uint16_t);
		for (int i = 0; i < size; i++)
		{
			if (partition_name[i] != that.partition_name[i])
			{
				return false;
			}
		}

		return partition_type_guid == that.partition_type_guid
			&& unique_partition_guid == that.unique_partition_guid
			&& starting_lba == that.starting_lba
			&& ending_lba == that.ending_lba
			&& attributes == that.attributes;
	}
}  gpt_entry;
typedef gpt_entry GPT_ENTRY;
typedef gpt_entry* PGPT_ENTRY;
typedef gpt_entry* LPGPT_ENTRY;


/*
   These values are only defaults.  The actual on-disk structures
   may define different sizes, so use those unless creating a new GPT disk!
*/

#define GPT_DEFAULT_RESERVED_PARTITION_ENTRY_ARRAY_SIZE 16384
/*
   Number of actual partition entries should be calculated
   as:
*/
#define GPT_DEFAULT_RESERVED_PARTITION_ENTRIES \
        (GPT_DEFAULT_RESERVED_PARTITION_ENTRY_ARRAY_SIZE / \
         sizeof(gpt_entry))



#define EFI_GPT_PRIMARY_PARTITION_TABLE_LBA 1


#define GPT_BLOCK_SIZE 512
#define GPT_HEADER_REVISION_V1 0x00010000
#define GPT_PRIMARY_PARTITION_TABLE_LBA 1


#define UNUSED_ENTRY_GUID    \
     0x00000000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#define PARTITION_SYSTEM_GUID \
     0xC12A7328, 0xF81F, 0x11d2, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
#define LEGACY_MBR_PARTITION_GUID \
     0x024DEE41, 0x33E7, 0x11d3, 0x9D, 0x69, 0x00, 0x08, 0xC7, 0x81, 0xF3, 0x9F
#define PARTITION_MSFT_RESERVED_GUID \
     0xE3C9E316, 0x0B5C, 0x4DB8, 0x81, 0x7D, 0xF9, 0x2D, 0xF0, 0x02, 0x15, 0xAE
#define PARTITION_BASIC_DATA_GUID \
     0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7
#define PARTITION_RAID_GUID \
     0xa19d880f, 0x05fc, 0x4d3b, 0xa0, 0x06, 0x74, 0x3f, 0x0f, 0x84, 0x91, 0x1e
#define PARTITION_SWAP_GUID \
     0x0657fd6d, 0xa4ab, 0x43c4, 0x84, 0xe5, 0x09, 0x33, 0xc8, 0x4b, 0x4f, 0x4f
#define PARTITION_LVM_GUID \
     0xe6d6d379, 0xf507, 0x44c2, 0xa2, 0x3c, 0x23, 0x8f, 0x2a, 0x3d, 0xf9, 0x28
#define PARTITION_RESERVED_GUID \
     0x8da63339, 0x0007, 0x60c0, 0xc4, 0x36, 0x08, 0x3a, 0xc8, 0x23, 0x09, 0x08

#define PARTITION_LDM_METADATA_GUID \
	 0x5808C8AA, 0x7E8F, 0x42E0, 0x85, 0xD2, 0xE1, 0xE9, 0x04, 0x34, 0xCF, 0xB3

#define PARTITION_LDM_DATA_GUID \
     0xAF9B60A0, 0x1431, 0x4F62, 0xBC, 0x68, 0x33, 0x11, 0x71, 0x4A, 0x69, 0xAD  

#define PARTITION_APPLE_BOOT \
     0x426F6F74, 0x0000, 0x11AA, 0xAA, 0x11, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC 

#define PARTITION_STORAGE_POOL_PHYSICAL_GUID \
     0xE75CAF8F, 0xF680, 0x4CEE, 0XAF, 0xA3, 0xB0, 0x01, 0xE5, 0x6E, 0xFC, 0x2D

#define PARTITION_MSFT_RECOVERY_GUID \
	 0xde94bba4,0x06d1,0x4d40,0xa1,0x6a,0xbf,0xd5,0x01,0x79,0xd6,0xac


#pragma pack(pop)
