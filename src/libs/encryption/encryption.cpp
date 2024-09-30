#include "pch.h"
#include "framework.h"

/**
 * @file
 * @brief This file defines a set of wrapper functions for encryption and decryption
 * operations using the OpenSSL library. These functions provide a higher-level,
 * more convenient interface for performing common cryptographic operations
 * such as AES encryption/decryption, HMAC computation, and MD5 hash computation.
 * They also handle error checking and throw exceptions if any of the operations
 * fail, simplifying error handling in the code that uses these functions.
 *
 * OpenSSL is licensed under an Apache-style license, which basically means
 * that you are free to get and use it for commercial and non-commercial
 * purposes subject to some simple license conditions. For a complete
 * description, please see https://www.openssl.org/source/license.html
 *
 * @copyright (c) 2024 Paramount Software UK Limited. All rights reserved.
 * @license MIT License
 */

enum class EncryptionOperation
{
	Encrypt,
	Decrypt
};

/**
 * @brief Computes the HMAC-SHA256 of the given key.
 *
 * This function takes a key as input and computes its HMAC using the SHA256 algorithm.
 * The computed HMAC is stored in the provided array.
 *
 * @param key_array The key to compute the HMAC for.
 * @param hmac_array The array to store the computed HMAC.
 * @throws std::runtime_error If an error occurs during HMAC computation.
 */
void _getKeyHMACSHA256(const std::array<uint8_t, KEY_LENGTH>& key_array, std::array<uint8_t, SHA256_DIGEST_LENGTH>& hmac_array)
{
	// Create a new HMAC context
	auto ctx = std::unique_ptr<EVP_MD_CTX, decltype(&::EVP_MD_CTX_free)>(EVP_MD_CTX_new(), ::EVP_MD_CTX_free);
	if (!ctx) {
		throw std::runtime_error("Failed to create EVP_MD_CTX");
	}
	// Initialize the HMAC context with the key and the SHA256 algorithm
	const EVP_MD* md = EVP_sha256();
	if (EVP_DigestInit_ex(ctx.get(), md, NULL) != 1) {
		throw std::runtime_error("Failed to initialize EVP_MD_CTX");
	}
	// Create an EVP_PKEY structure from the key
	auto pkey = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>(EVP_PKEY_new_mac_key(EVP_PKEY_HMAC, NULL, key_array.data(), (int)key_array.size()), ::EVP_PKEY_free);
	if (!pkey) {
		throw std::runtime_error("Failed to create EVP_PKEY");
	}
	// Set the HMAC key
	if (EVP_DigestSignInit(ctx.get(), NULL, md, NULL, pkey.get()) != 1) {
		throw std::runtime_error("Failed to set HMAC key");
	}
	// Compute the HMAC
	size_t len = SHA256_DIGEST_LENGTH;
	if (EVP_DigestSignFinal(ctx.get(), hmac_array.data(), &len) != 1) {
		throw std::runtime_error("Failed to compute HMAC");
	}
	return;
}

/**
 * @brief Initializes an AES operation (encryption or decryption) based on the provided parameters.
 *
 * This function initializes an AES operation (either encryption or decryption) using the provided parameters.
 * It creates and returns a unique_ptr to an initialized EVP_CIPHER_CTX, which will automatically be freed when it goes out of scope.
 *
 * @param aes_variant The AES variant to use (10 for AES-128, 12 for AES-192, 14 for AES-256).
 * @param pDerivedKey The derived key to use for the operation.
 * @param pIV The initialization vector to use for the operation. Can be NULL for ECB mode.
 * @param Type If set to EncryptionOperation::Encrypt, the function initializes for encryption. If set to anything else, it initializes for decryption.
 * @return A unique_ptr to the initialized EVP_CIPHER_CTX. The unique_ptr will automatically free the context when it goes out of scope.
 * @throws std::runtime_error If an error occurs during initialization.
 */

std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> _initAESOperation(const int aes_variant, const uint8_t* pDerivedKey, const uint8_t* pIV, EncryptionOperation Type)
{
	static const std::map<int, std::function<const EVP_CIPHER* ()>> cipher_map = {
		{10, [pIV]() { return pIV != nullptr ? EVP_aes_128_cbc() : EVP_aes_128_ecb(); }},
		{12, [pIV]() { return pIV != nullptr ? EVP_aes_192_cbc() : EVP_aes_192_ecb(); }},
		{14, [pIV]() { return pIV != nullptr ? EVP_aes_256_cbc() : EVP_aes_256_ecb(); }}
	};

	auto cipher_type = cipher_map.at(aes_variant)();

	auto ctx = std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)>(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
	if (!ctx) {
		throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
	}

	if (Type == EncryptionOperation::Encrypt) {
		if (1 != EVP_EncryptInit_ex(ctx.get(), cipher_type, NULL, pDerivedKey, pIV)) {
			throw std::runtime_error("Failed to initialize AES encryption");
		}
	}
	else {
		if (1 != EVP_DecryptInit_ex(ctx.get(), cipher_type, NULL, pDerivedKey, pIV)) {
			throw std::runtime_error("Failed to initialize AES decryption");
		}
	}
	return ctx;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// Encryption functions for Macrium Reflect Image files
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Formats the Initialization Vector (IV) for AES encryption.
 *
 * This function formats the Initialization Vector (IV) for AES encryption using the provided parameters.
 * It returns a pointer to the formatted IV.
 *
 * @param pImageID The image ID used in IV generation.
 * @param disk_number The disk number used in IV generation.
 * @param partition_number The partition number used in IV generation.
 * @param block_index The block index used in IV generation.
 * @param dwpbkdf2Iterations The number of PBKDF2 iterations used in key derivation.
 * @param DerivedKey The derived key used in IV generation.
 * @param pIV The pointer to the IV to be formatted.
 * @return A pointer to the formatted IV.
 * @throws std::runtime_error If an error occurs during IV formatting.
 */
uint8_t* formatInitializationVectorForAES(const uint8_t* pImageID, const int32_t disk_number, const int32_t partition_number, const int32_t block_index, const int dwpbkdf2Iterations, const std::array<uint8_t, KEY_LENGTH>& DerivedKey, uint8_t* pIV)
{
	// Prepare the IV with a set/disk/partition/block - unique IV
	// using the imageid, disk_number, partition_number and block_index
	uint8_t data[AES_BLOCK_SIZE];
	memcpy(data, pImageID, 8); // Copy the first 8 bytes from the 'imageid' array into the 'data' array
	memcpy(data + 8, &disk_number, 2); // Copy the value of 'disk_number' into the 'data' array starting at the 9th byte (index 8). It assumes that 'disk_number' is a 16-bit integer (2 bytes)
	memcpy(data + 10, &partition_number, 2); // Copy the value of 'partition_number' into the 'data' array starting at the 11th byte (index 10). It assumes that 'partition_number' is a 16-bit integer (2 bytes)
	memcpy(data + 12, &block_index, 4); // Copy the value of 'block_index' into the 'data' array starting at the 13th byte (index 12). It assumes that 'block_index' is a 32-bit integer (4 bytes)
	// Hash the derived key
	uint8_t key_hash[SHA256_DIGEST_LENGTH];
	SHA256(DerivedKey.data(), DerivedKey.size(), key_hash); // Compute SHA-256 hash of the derived key
	// Create a new cipher context
	std::unique_ptr<EVP_CIPHER_CTX, decltype(&::EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(), ::EVP_CIPHER_CTX_free);
	if (!ctx) {
		throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
	}
	// Initialize the cipher context for AES-256 encryption in ECB mode
	if (EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_ecb(), NULL, key_hash, NULL) != 1) {
		throw std::runtime_error("Failed to initialize EVP_CIPHER_CTX");
	}
	int len = 0;
	// Encrypt the data using the initialized context and the key hash
	if (EVP_EncryptUpdate(ctx.get(), pIV, &len, data, AES_BLOCK_SIZE) != 1) {
		throw std::runtime_error("Failed to encrypt data");
	}
	// Clean up is handled by the unique_ptr
	return pIV; // Return the pointer to the IV
}

/**
 * @brief Generates a derived key using PBKDF2 SHA-256.
 *
 * This function generates a derived key from a given password using the PBKDF2 SHA-256 algorithm.
 * The derived key is stored in the provided array.
 *
 * @param imageid The image ID used in key derivation.
 * @param strPassword The password from which to derive the key.
 * @param iterations The number of PBKDF2 iterations used in key derivation.
 * @param DerivedKey The array to store the derived key.
 * @throws std::runtime_error If an error occurs during key derivation.
 */
void getDerivedKey(const uint8_t(&imageid)[8], const std::string& strPassword, const int iterations, std::array<uint8_t, KEY_LENGTH>& DerivedKey)
{
	// Hash the Image ID using SHA-256
	unsigned char imageid_hash[SHA256_DIGEST_LENGTH];
	if (!EVP_Digest(imageid, sizeof(imageid), imageid_hash, NULL, EVP_sha256(), NULL)) {
		throw std::runtime_error("Failed to compute hash of Image ID");
	}
	// Use PKCS5_PBKDF2_HMAC to generate the derived key
	// strPassword.c_str() is the password from which to derive the key
	// -1 means the function will calculate the length of the password string
	// imageid_hash is the salt value for the PBKDF2 function
	// sizeof(imageid_hash) is the length of the salt
	// iterations is the number of iterations for the PBKDF2 function
	// EVP_sha256() is the hash function to use
	// KEY_LENGTH is the length of the derived key
	// key_array.data() is the buffer where the derived key will be stored
	if (PKCS5_PBKDF2_HMAC(strPassword.c_str(), -1, imageid_hash, sizeof(imageid_hash), iterations, EVP_sha256(), KEY_LENGTH, DerivedKey.data()) != 1) {
		throw std::runtime_error("Failed to generate derived key"); // Throw an error if the key generation failed
	}
	return;
}

/**
 * @brief Generates a HMAC of the derived key.
 *
 * This function generates a HMAC of the derived key. The computed HMAC is stored in the returned array.
 *
 * @param DerivedKey The derived key to compute the HMAC for.
 * @return An array containing the HMAC of the derived key.
 * @throws std::runtime_error If an error occurs during HMAC computation.
 */
std::array<uint8_t, KEY_LENGTH> getKeyHMAC(std::array<uint8_t, KEY_LENGTH> DerivedKey)
{
	std::array<uint8_t, KEY_LENGTH> hmac;
	_getKeyHMACSHA256(DerivedKey, hmac);
	return hmac;
}

/**
 * @brief Computes the MD5 hash of the given data.
 *
 * This function takes a data as input and computes its MD5 hash.
 * The computed hash is stored in the returned array.
 *
 * @param inputData The data to compute the MD5 hash for.
 * @param inputDataLength The length of the data.
 * @return An array containing the MD5 hash of the data.
 * @throws std::runtime_error If an error occurs during hash computation.
 */
std::array<uint8_t, MD5_LENGTH> computeMD5Hash(const uint8_t* inputData, size_t inputDataLength)
{
	std::array<uint8_t, MD5_LENGTH> md5HashArray;
	unsigned int md5HashLength;
	if (!EVP_Digest(inputData, inputDataLength, md5HashArray.data(), &md5HashLength, EVP_md5(), NULL)) {
		throw std::runtime_error("Failed to compute MD5 hash of derived key");
	}
	return md5HashArray;
}


/**
 * @brief Encrypts the given data using AES-CBC encryption.
 *
 * This function encrypts the given data using AES-CBC encryption with the provided parameters.
 * The encrypted data is stored in the provided output buffer.
 *
 * @param aes_variant The AES variant to use (10 for AES-128, 12 for AES-192, 14 for AES-256).
 * @param pDerivedKey The derived key to use for encryption.
 * @param pIV The initialization vector to use for encryption.
 * @param pInBuffer The input buffer containing the data to encrypt.
 * @param pOutBuffer The output buffer to store the encrypted data.
 * @param nOutLen The length of the output buffer. Must be a multiple of 16.
 * @throws std::runtime_error If an error occurs during encryption.
 */
void encryptDataWithAESCBC(const int aes_variant, const uint8_t* pDerivedKey, const uint8_t* pIV, uint8_t* pInBuffer, uint8_t* pOutBuffer, int& nOutLen)
{
	if (nOutLen % 16 != 0) {
		throw std::runtime_error("Output buffer length must be a multiple of 16");
	}
	int nInLen = nOutLen;

	// Create and initialise the context
	auto ctx = _initAESOperation(aes_variant, pDerivedKey, pIV, EncryptionOperation::Encrypt);

	// Disable padding
	if (1 != EVP_CIPHER_CTX_set_padding(ctx.get(), 0)) {
		throw std::runtime_error("Failed to disable padding");
	}

	// Provide the message to be encrypted, and obtain the encrypted output
	if (1 != EVP_EncryptUpdate(ctx.get(), pOutBuffer, &nOutLen, pInBuffer, nInLen)) {
		throw std::runtime_error("Failed to encrypt data");
	}
	// Clean up is handled by the unique_ptr
}


/**
 * @brief Decrypts the given data using AES-CBC decryption.
 *
 * This function decrypts the given data using AES-CBC decryption with the provided parameters.
 *
 * @param aes_variant The AES variant to use (10 for AES-128, 12 for AES-192, 14 for AES-256).
 * @param pDerivedKey The derived key to use for decryption.
 * @param pIV The initialization vector to use for decryption.
 * @param input The input buffer containing the data to decrypt.
 * @param len The length of the input buffer. Must be a multiple of 16.
 * @throws std::runtime_error If an error occurs during decryption.
 */
void decryptDataWithAESCBC(const int aes_variant, uint8_t* pDerivedKey, uint8_t* pIV, uint8_t* input, int len)
{
	if (len % 16 != 0) {
		throw std::runtime_error("Input buffer length must be a multiple of 16");
	}
	// Create and initialise the context
	auto ctx = _initAESOperation(aes_variant, pDerivedKey, pIV, EncryptionOperation::Decrypt);

	// Disable padding
	if (1 != EVP_CIPHER_CTX_set_padding(ctx.get(), 0)) {
		throw std::runtime_error("Failed to disable padding");
	}
	// Provide the message to be decrypted, and obtain the decrypted output
	int plaintext_len;
	if (1 != EVP_DecryptUpdate(ctx.get(), input, &plaintext_len, input, len)) {
		throw std::runtime_error("Failed to decrypt data");
	}
	// Clean up is handled by the unique_ptr
}

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
void decryptDataWithAESECB(const int aes_variant, const uint8_t* pDerivedKey, uint8_t* input, int len)
{
	if (len % 16 != 0) {
		throw std::runtime_error("Output buffer length must be a multiple of 16");
	}

	// Create and initialise the context
	auto ctx = _initAESOperation(aes_variant, pDerivedKey, nullptr, EncryptionOperation::Decrypt);

	// Disable padding
	if (1 != EVP_CIPHER_CTX_set_padding(ctx.get(), 0)) {
		throw std::runtime_error("Failed to disable padding");
	}

	// Provide the message to be decrypted, and obtain the decrypted output
	int plaintext_len;
	if (1 != EVP_DecryptUpdate(ctx.get(), input, &plaintext_len, input, len)) {
		throw std::runtime_error("Failed to decrypt data");
	}
	// Clean up is handled by the unique_ptr
}
