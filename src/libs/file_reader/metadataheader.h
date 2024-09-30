#pragma once

/**
 * @file
 * @brief This file contains the definition of various data structures used
 * in the backup file reader library.
 *
 * These data structures represent various components of a backup file, such
 * as the header, metadata blocks, and data blocks. They are used throughout
 * the library to provide a type-safe way of representing these components in
 * the code.
 *
 * The NLOHMANN_JSON_SERIALIZE_ENUM macro is used to map these data structure
 * values to their corresponding string values in the JSON structures of the
 * backup file. This allows for easy serialization and deserialization of
 * these values when reading from or writing to a backup file.
 */


 /**************************************** BLOCK NAMES ****************************************/
const char* const JSON_HEADER   = "$JSON   ";  // File header JSON data
const char* const BITMAP_HEADER = "$BITMAP ";  // Partition data
const char* const FAT_HEADER    = "$FAT    ";  // FAT32 FAT data
const char* const CBT_HEADER    = "$CBT    ";  // Changed Block Tracking data
const char* const MFT_HEADER    = "$MFT    ";  // Master File Table data
const char* const TRACK_0       = "$TRACK0 ";  // 1st 1MB of data on the disk
const char* const IDX_HEADER    = "$INDEX  ";  // Reserved sectors and data block index
const char* const EXT_PAR_TABLE = "$EPT    ";  // Extended Partition Table data
#define BLOCK_NAME_LENGTH 8                    // Length of block name strings
/************************************************************************************************/

const char  MAGIC_BYTES_VX[] = { "MACRIUM_FILE" };  // vX magic file bytes
// Don't write out the null terminator
const unsigned long MAGIC_BYTES_VX_SIZE = (unsigned long)(sizeof(MAGIC_BYTES_VX) - 1);

struct HeaderFlags
{
    unsigned char last_block  : 1;    // Flag indicating whether this is the last block in the file
    unsigned char compression : 1;  // Flag indicating whether the block is compressed
    unsigned char encryption : 1;   // Flag indicating whether the block is encrypted
    unsigned char unused : 5;       // Remaining unused bits

    HeaderFlags() : last_block (0), compression(0), encryption(0), unused(0) {};
};


struct MetadataBlockHeader
{
    char  block_name[8] = { 0 };      // Name of the block
    unsigned long block_Length = 0;   // Length of the block
    unsigned char  hash[16] = { 0 }; // MD5 hash of the block
    HeaderFlags flags;               // Flags for the block
    char padding[3] = { 0 };         // Padding for alignment
};
