#include <cstdint>

const char MAGIC_BYTES_VX[] = {"MACRIUM_FILE"};
const unsigned long MAGIC_BYTES_VX_SIZE = (unsigned long)(sizeof(MAGIC_BYTES_VX) - 1);

struct MetadataHeaderFlags
{
    unsigned char LastBlock : 1;
    unsigned char Compression : 1;
    unsigned char Encryption : 1;
    unsigned char Unused : 5;
};

struct MetadataBlockHeader
{
    char BlockName[8] = {0};
    uint32_t BlockLength = 0;
    unsigned char Hash[16] = {0};
    MetadataHeaderFlags Flags;
    char Padding[3] = {0};
};

const char* const JSON_HEADER   = "$JSON   ";  // File header JSON data
const char* const BITMAP_HEADER = "$BITMAP ";  // Partition data
const char* const FAT_HEADER    = "$FAT    ";  // FAT32 FAT data
const char* const CBT_HEADER    = "$CBT    ";  // Changed Block Tracking data
const char* const MFT_HEADER    = "$MFT    ";  // Master File Table data
const char* const TRACK_0       = "$TRACK0 ";  // 1st 1MB of data on the disk
const char* const IDX_HEADER    = "$INDEX  ";  // Reserved sectors and data block index
const char* const EXT_PAR_TABLE = "$EPT    ";  // Extended Partition Table data
#define BLOCK_NAME_LENGTH 8                    // Length of block name strings