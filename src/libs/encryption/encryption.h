#pragma once
/*
===============================================================================
Copyright (c) 2024 Paramount Software UK Limited. All rights reserved.

Licensed under the MIT License.

This file defines a set of wrapper functions for encryption and decryption
operations using the OpenSSL library. These functions provide a higher-level,
more convenient interface for performing common cryptographic operations such
as AES encryption/decryption, HMAC computation, and MD5 hash computation.
They also handle error checking and throw exceptions if any of the operations
fail, simplifying error handling in the code that uses these functions.

The library uses the following external libraries:
- OpenSSL library for decryption.
  License:  [Apache License 2.0](https://www.openssl.org/source/license.html)
- Zlib library, a dependency of OpenSSL.
  License: [Zlib License](https://zlib.net/zlib_license.html)
===============================================================================
*/

/**
 * @file
 * @brief This is the primary include file for this library.
 *
 * Including this file in a project gives access to all the functionality provided by this library.
 * No other include files are necessary.
 */

 // Define the block size for AES encryption/decryption
constexpr int AES_BLOCK_SIZE = 16;

// Define the length of the key used in encryption/decryption
constexpr int KEY_LENGTH = 32;

// Define the length of the MD5 hash
constexpr int MD5_LENGTH = 16;


/**
 * @brief Encrypts data using AES-CBC (Cipher Block Chaining) mode.
 *
 * This function encrypts the given data using AES-CBC encryption with the provided parameters.
 * The encrypted data is stored in the provided output buffer.
 *
 * @param aes_variant The AES variant to use (10 for AES-128, 12 for AES-192, 14 for AES-256).
 * @param p8DerivedKeyBuffer The derived key to use for encryption.
 * @param pIV The initialization vector to use for encryption.
 * @param pInBuffer The input data to encrypt.
 * @param pOutBuffer The output buffer to store the encrypted data.
 * @param nOutLen The length of the output data.
 * @throws std::runtime_error If an error occurs during encryption.
 */
void encryptDataWithAESCBC(const int aes_variant, const uint8_t* p8DerivedKeyBuffer, const uint8_t* pIV, uint8_t* pInBuffer, uint8_t* pOutBuffer, int& nOutLen);

/**
 * @brief Decrypts data using AES-CBC (Cipher Block Chaining) mode.
 *
 * This function decrypts the given data using AES-CBC decryption with the provided parameters.
 *
 * @param aes_variant The AES variant to use (10 for AES-128, 12 for AES-192, 14 for AES-256).
 * @param pDerivedKey The derived key to use for decryption.
 * @param pIV The initialization vector to use for decryption.
 * @param input The input data to decrypt.
 * @param len The length of the input data.
 * @throws std::runtime_error If an error occurs during decryption.
 */
void decryptDataWithAESCBC(const int aes_variant, uint8_t* pDerivedKey, uint8_t* pIV, uint8_t* input, int len);

/**
 * @brief Decrypts the given data using AES decryption in ECB mode.
 *
 * This function decrypts the given data using AES decryption in ECB mode with the provided parameters.
 *
 * @param aes_variant The AES variant to use (10 for AES-128, 12 for AES-192, 14 for AES-256).
 * @param pDerivedKey The derived key to use for decryption.
 * @param input The input buffer containing the data to decrypt.
 * @param len The length of the input buffer. Must be a multiple of 16.
 * @throws std::runtime_error If an error occurs during decryption.
 */
void decryptDataWithAESECB(const int aes_variant, const uint8_t* pDerivedKey, uint8_t* input, int len);

/**
 * @brief Prepares the Initialization Vector (IV) for AES encryption.
 *
 * This function prepares the Initialization Vector (IV) for AES encryption. The IV is unique for each set/disk/partition/block, and is generated using the image ID, disk number, partition number, and block index.
 *
 * @param pImageID The image ID used in IV generation.
 * @param disk_number The disk number used in IV generation.
 * @param partition_number The partition number used in IV generation.
 * @param block_index The block index used in IV generation.
 * @param dwpbkdf2Iterations The number of PBKDF2 iterations used in key derivation.
 * @param DerivedKey The derived key used in IV generation.
 * @param pIV The pointer to the IV.
 * @return A pointer to the formatted IV.
 * @throws std::runtime_error If an error occurs during IV preparation.
 */
uint8_t* formatInitializationVectorForAES(const uint8_t* pImageID, const int32_t disk_number, const int32_t partition_number, const int32_t block_index, const int dwpbkdf2Iterations, const std::array<uint8_t, KEY_LENGTH>& DerivedKey, uint8_t* pIV);

/**
 * @brief Generates a HMAC of the derived key.
 *
 * This function generates a HMAC of the derived key. The computed HMAC is stored in the returned array.
 *
 * @param DerivedKey The derived key to compute the HMAC for.
 * @return An array containing the HMAC of the derived key.
 * @throws std::runtime_error If an error occurs during HMAC computation.
 */
std::array<uint8_t, KEY_LENGTH> getKeyHMAC(std::array<uint8_t, KEY_LENGTH> DerivedKey);

/**
 * @brief Retrieves the derived key using PBKDF2.
 *
 * This function retrieves the derived key using PBKDF2. The derived key is generated from the password and the image ID.
 *
 * @param imageid The image ID used in key derivation.
 * @param strPassword The password from which to derive the key.
 * @param iterations The number of PBKDF2 iterations used in key derivation.
 * @param DerivedKey The array to store the derived key.
 * @throws std::runtime_error If an error occurs during key derivation.
 */
void getDerivedKey(const uint8_t(&imageid)[8], const std::string& strPassword, const int iterations, std::array<uint8_t, KEY_LENGTH>& DerivedKey);

/**
 * @brief Computes the MD5 hash of the given input data.
 *
 * This function computes the MD5 hash of the given input data. The computed hash is stored in the returned array.
 *
 * @param inputData The data to compute the MD5 hash for.
 * @param inputDataLength The length of the input data.
 * @return The MD5 hash of the input data.
 * @throws std::runtime_error If an error occurs during hash computation.
 */
std::array<uint8_t, MD5_LENGTH> computeMD5Hash(const uint8_t* inputData, size_t inputDataLength);
