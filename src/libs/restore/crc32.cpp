#include "pch.h"
#include "crc32.h"


static uint32_t crc32LookupTable[256]; // Lookup table array
static bool isCRCTableInitialized = false;

/**
 * Reverses the bits in a given value.
 *
 * This function takes a value and the number of bits to reflect, and returns the value with its bits reversed.
 * It's used in the CRC32 calculation process, specifically in the table initialization.
 *
 * @param ref The value whose bits are to be reflected.
 * @param bitCount The number of bits to reflect.
 * @return The value with its bits reversed.
 */
uint32_t ReverseBits(uint32_t ref, char bitCount)
{
    uint32_t reflectedValue(0);

    for (int i = 1; i < (bitCount + 1); i++) {
        if (ref & 1)
            reflectedValue |= 1 << (bitCount - i);
        ref >>= 1;
    }
    return reflectedValue;
}

/**
 * Initializes the CRC32 lookup table.
 *
 * This function generates a lookup table for the CRC32 calculation. It should be called once before any CRC32 calculations are done.
 * The lookup table uses the polynomial 0x04c11db7, which is the official polynomial used by CRC-32 in PKZip, WinZip and Ethernet.
 */
void Initialize_CRC32_Table()
{
    uint32_t polynomial = 0x04c11db7;

    for (int i = 0; i <= 0xFF; i++) {
        crc32LookupTable[i] = ReverseBits(i, 8) << 24;
        for (int j = 0; j < 8; j++)
            crc32LookupTable[i] = (crc32LookupTable[i] << 1) ^ (crc32LookupTable[i] & (1 << 31) ? polynomial : 0);
        crc32LookupTable[i] = ReverseBits(crc32LookupTable[i], 32);
    }
}

/**
 * Calculates the CRC32 checksum for a buffer of data.
 *
 * This function calculates the CRC32 checksum for a given buffer of data. It uses the polynomial 0x04c11db7.
 * The function can also continue a previous CRC calculation by providing the previous CRC value as the last parameter.
 *
 * @param buffer The buffer containing the data to calculate the CRC for.
 * @param dataLength The length of the data in the buffer to calculate the CRC for.
 * @param bufferLength The total length of the buffer.
 * @param initialCRC The initial CRC value. This can be the result of a previous CRC calculation to continue from. Defaults to 0xffffffff.
 * @return The calculated CRC32 checksum.
 */
uint32_t Calculate_CRC32(const uint8_t* buffer, uint32_t dataLength, uint32_t bufferLength, uint32_t initialCRC /*= 0xffffffff*/)
{
    if (dataLength > bufferLength) {
        return 0;
    }

    if (!isCRCTableInitialized) {
        Initialize_CRC32_Table();
        isCRCTableInitialized = true;
    }

    while (dataLength--)
        initialCRC = (initialCRC >> 8) ^ crc32LookupTable[(initialCRC & 0xFF) ^ *buffer++];
    return initialCRC ^ 0xffffffff;
}