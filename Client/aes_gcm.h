#pragma once
#include <string.h>
#if __STDC_VERSION__ > 199900L || __cplusplus > 201100L || defined(_MSC_VER)
#include <stdint.h>
#endif

#define AES___			256		/* 128 or 256 (or 192; not standardized in some modes) */
#define GCM_NONCE_LEN	12		/* RECOMMENDED. but other values are supported. */
#define TAG_SIZE	    16

#if AES___ == 0x100 || AES___ == 0xC0
#define AES_KEY_SIZE  (AES___ >> 3)
#else
#define AES_KEY_SIZE   16
#endif

namespace Crypto {

	const uint8_t VERSION_1[] = { 'v', '1', '0' };
	const uint8_t VERSION_2[] = { 'v', '2', '0' };
	const uint8_t PREFIX_NAME[] = { 'v', '2', '0' };
#define PREFIX_SIZE	sizeof(PREFIX_NAME)

	/// <summary>
	/// This function is used to calculate size of data plaintext.
	/// </summary>
	/// <param name="1. [IN] crTextLen">: This is a reference to the size variable (size_t) representing the size of the data to decrypt.</param>
	/// <returns> Size of the data encrypted</returns>
	size_t getPlainTextSize(const size_t crTextLen);

	/// <summary>
	/// This function is used to calculate size of data ciphertext
	/// </summary>
	/// <param name="1. [IN] pnTextLen">: This is a reference to the size variable (size_t) representing the size of the data to encrypt.</param>
	/// <returns>Size of the data decrypted</returns>
	size_t getCipherTextSize(const size_t pnTextLen);

	/// <summary>
	/// This function is used to encrypt data using the AES-GCM algorithm.
	/// </summary>
	/// <param name="1. [IN]  pnText">: This is a pointer to the BYTE array containing the data that needs to be encrypt.</param>
	/// <param name="2. [IN]  pnTextLen">: This is a reference to the size variable (size_t) representing the size of the data to encrypt.</param>
	/// <param name="3. [IN]  key_v2"></param>
	/// <param name="4. [IN]  key_size"></param>
	/// <param name="5. [OUT] crText">: This is a pointer to the BYTE array containing the data encrypted.</param>
	/// <returns>True or False</returns>
	bool encryptData(
		const uint8_t* pnText, const size_t pnTextLen,
		const uint8_t* key_v2, const size_t key_size,
		uint8_t* crText);

	/// <summary>
	/// This function is used to decrypt data encrypted using the AES-GCM algorithm.
	/// </summary>
	/// <param name="1. [IN]  crText">: This is a pointer to the BYTE array containing the data that needs to be decrypt.</param>
	/// <param name="2. [IN]  crTextLen">: This is a reference to the size variable (size_t) representing the size of the data to decrypt.</param>
	/// <param name="3. [IN]  key_v1">: </param>
	/// <param name="4. [IN]  key1_size">: </param>
	/// <param name="5. [IN]  key_v2">: </param>
	/// <param name="4. [IN]  key2_size">: </param>
	/// <param name="3. [OUT] pnText">: This is a pointer to the BYTE array containing the data decrypted.</param>
	/// <returns>True or False</returns>
	bool decryptData(
		const uint8_t* crText, const size_t crTextLen,
		const uint8_t* key_v1, const size_t key1_size,
		const uint8_t* key_v2, const size_t key2_size,
		uint8_t* pnText);
}

#define ENCRYPTION_FAILURE       0x1E
#define DECRYPTION_FAILURE       0x1D
#define AUTHENTICATION_FAILURE   0x1A
#define NO_ERROR_RETURNED        0x00
