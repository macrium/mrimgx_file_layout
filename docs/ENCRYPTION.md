<img src="../assets/ReflectX.png" width="300"> <br>Experience data independence with love, from us to you. <img src="../assets/Love_Heart_symbol.svg" width="30">


***

## Encryption Implementation Overview for Macrium Reflect X
Macrium Reflect uses AES-CBC encryption, PBKDF2 for secure password handling, and SHA-256 hashing. AES-CBC (Advanced Encryption Standard in Cipher Block Chaining mode) provides a high level of encryption, making it practically impervious to decryption attempts without the correct key. PBKDF2 (Password-Based Key Derivation Function 2) is used to securely transform your password into a robust key by applying a hashing function, in this case, SHA-256 (Secure Hash Algorithm 256 bits), multiple times. This process significantly enhances the resistance against brute-force attacks. Furthermore, Macrium Reflect stores your password as an HMAC (Hash-Based Message Authentication Code) using SHA-256, a procedure that validates the integrity and authenticity of stored data, ensuring that your backups remain both secure and accessible only to those with the correct credentials. 

Macrium Reflect offers AES-128, AES-192, and AES-256 encryption, enabling you to align with both regulatory compliance standards and performance needs. 
#### AES-CBC: 

AES-CBC is a secure encryption algorithm where each plaintext cipher block is XORed (exclusive or operation) with the previous ciphertext block before being encrypted  This approach allows for high levels of security and performance, making it an ideal choice for encrypting discrete data blocks for random access.

#### Initialization Vector

The Initialization Vector (IV) is crucial in the AES-CBC mode for ensuring the uniqueness of each encrypted data block within a backup set.

There are 5 elements used for each IV:

1. The "imageid" is a unique, randomly generated identifier for each backup set, found in the header's JSON data, ensuring no duplicates across backup sets.
```json
    "_header": {
        "backup_guid": "89a54134-2f03-4ed4-9ab7-139a165326a5",
        "backup_time": 1709555568,
        "backupset_time": 1709555568,
        "file_number": 0,
        "imaged_disks_count": 1,
        `"imageid":` "0AD2FD9362D493B1",
        "increment_number": 0,
        "netbios_name": "MS-W-0",
        "split_file": false
    },
```   
2. The Disk number in the backup. 
```json
    "disks": [
        {
            "_header": {
                "disk_format": "gpt",
                `"disk_number"`: 2,
                "disk_signature": "6A578E4D-7B5C-4CC1-85D3-FFCA233A9B93",
                "imaged_partition_count": 1
            },
        }
    ]

```
3. The Partition number in the backup.
```json
"partitions": [
    {
        "_header": {
            "block_count": 1965792,
            "block_size": 65536,
            "file_history": [
                {
                    "file_name": "B:\\vx\\D684BA87241263E2-demo-00-00.mrimgx",
                    "file_number": 0
                }
            ],
            "file_history_count": 1,
            "partition_file_offset": 0,
            `"partition_number"`: 1
        }
    }
]
```
4. The index number of the block being encrypted.
5. Derived Key for ESSIV: Used for ESSIV (Encrypted Salt-Sector Initialization Vector) initialization to counter watermarking attacks.

Using OpenSSL:
```c++
uint8_t* formatInitializationVectorForAES(const uint8_t* imageid, const int32_t disk_number, const int32_t partition_number, const int32_t block_index, const std::array<uint8_t, KEY_LENGTH>& DerivedKey, uint8_t* pIV)
{
	// Prepare the IV with a set/disk/partition/block - unique IV
	// using the imageid, disk_number, partition_number and block_index
	uint8_t data[AES_BLOCK_SIZE];
	memcpy(data, imageid, 8); // Copy 'imageid' array into the 'data' array
	memcpy(data + 8, &((byte)disk_number), 2); // Copy the value of 'disk_number' into the 'data' array starting at the 9th byte (index 8). 'disk_number' is a 16-bit integer (2 bytes)
	memcpy(data + 10, &((byte)partition_number), 2); // Copy the value of 'partition_number' into the 'data' array starting at the 11th byte (index 10). 'partition_number' is a 16-bit integer (2 bytes)
	memcpy(data + 12, &block_index, 4); // Copy the value of 'block_index' into the 'data' array starting at the 13th byte (index 12). 'block_index' is a 32-bit integer (4 bytes)
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
```

#### Key Derivation with PBKDF2

To convert user-provided passwords into secure encryption keys, we use PBKDF2 (Password-Based Key Derivation Function 2). We exceed current security recommendations by using 600,000 iterations, and this number is periodically reviewed to ensure resistance against brute-force attacks.

Using OpenSSL:
```c++
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
```
### Validating user password attempts
The password is securely stored by applying an HMAC (Hash-Based Message Authentication Code) using SHA-256 to a key derived from the original password.
```json
    "_encryption": {
        "aes_type": "aes-256",
        "enable": true,
        `"hmac"`: "3f40170bef9287feb86e8a811453441259d81f2272d31f7c861644cddfd7b814",
        "key_derivation": "pbkdf2",
        "key_iterations": 600000
    },
```
Using OpenSSL:
```c++
constexpr int KEY_LENGTH = 32;
void getKeyHMACSHA256(const std::array<uint8_t, KEY_LENGTH>& key_array, std::array<uint8_t, SHA256_DIGEST_LENGTH>& hmac_array)
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
```


