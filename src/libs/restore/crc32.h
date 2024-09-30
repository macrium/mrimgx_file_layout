#pragma once

/**
 * Calculates the CRC32 checksum for a buffer of data.
 *
 * This function calculates the CRC32 checksum for a given buffer of data. It uses the polynomial 0x04C11DB7.
 * The function can also continue a previous CRC calculation by providing the previous CRC value as the last parameter.
 *
 * @param buf The buffer containing the data to calculate the CRC for.
 * @param len The length of the data in the buffer to calculate the CRC for.
 * @param Bufferlen The total length of the buffer.
 * @param ulCRC The initial CRC value. This can be the result of a previous CRC calculation to continue from. Defaults to 0xffffffff.
 * @return The calculated CRC32 checksum.
 */
uint32_t Calculate_CRC32(const uint8_t* buf, uint32_t len, uint32_t Bufferlen, uint32_t ulCRC = 0xffffffff);