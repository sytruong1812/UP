#include <Windows.h>
#include <iostream>
#include "aes_manager.h"

const uint8_t PREFIX_AES[] = { 'A', 'E', 'S' };
#define PREFIX_SIZE	sizeof(PREFIX_AES)
#define NONCE_SIZE	12
#define TAG_SIZE	16


//Singleton 
AesManager* AesManager::s_instance = NULL;

AesManager::AesManager() 
{
	key_ = NULL;
	key_size_ = 0;
	encrypt_flag_ = false;
	decrypt_flag_ = false;
}
AesManager::~AesManager() 
{
	if (key_ != NULL) {
		delete[] key_;
		key_ = NULL;
	}
	key_size_ = 0;
}

void AesManager::getKey(uint8_t* key, size_t& size)
{
	key = key_;
	size = key_size_;
}
void AesManager::setKey(const uint8_t* key, const size_t size)
{
	if (key_ != NULL) {
		delete[] key_;
	}
	key_ = new uint8_t[size];
	memcpy(key_, key, size);
	key_size_ = size;
}

void AesManager::setDecryptFlag(bool option)
{
	decrypt_flag_ = option;
}

bool AesManager::getDecryptFlag()
{
	return decrypt_flag_;
}

void AesManager::setEncryptFlag(bool option)
{
	encrypt_flag_ = option;
}

bool AesManager::getEncryptFlag()
{
	return encrypt_flag_;
}

bool AesManager::generate_nonce(uint8_t* data, size_t length)
{
	if (data == NULL) {
		return false;
	}
	std::random_device rd;  // Seed for random number generation
	std::mt19937 generator(rd());  // Mersenne Twister RNG
	std::uniform_int_distribution<int> distribution(0, 255); // Range for bytes
	for (size_t i = 0; i < length; ++i) {
		data[i] = static_cast<uint8_t>(distribution(generator));
	}
	return true;
}

/*
- Purpose: This function is used to calculate size of data plaintext
- Systax:
	1. [IN]		size_t		crTextLen
- Parameters:
	1. crTextLen: This is a reference to the size variable (size_t) representing the size of the data to decrypt.
- Return value: Size of the data encrypted
*/
size_t AesManager::getSizePlainText(const size_t crTextLen)
{
	return crTextLen - PREFIX_SIZE - NONCE_SIZE - TAG_SIZE;
}

/*
- Purpose: This function is used to calculate size of data ciphertext 
- Systax:
	1. [IN]		size_t		pnTextLen
- Parameters:
	1. pnTextLen: This is a reference to the size variable (size_t) representing the size of the data to encrypt.
- Return value: Size of the data decrypted
*/
size_t AesManager::getSizeCipherText(const size_t pnTextLen)
{
	return pnTextLen + PREFIX_SIZE + NONCE_SIZE + TAG_SIZE;
}

/*
- Purpose: This function is used to encrypt data using the AES-GCM algorithm.
- Systax:
	1. [IN]		uint8_t		pnText
	2. [IN]		size_t		pnTextLen
	3. [OUT]	uint8_t		crText
- Parameters:
	1. pnText: This is a pointer to the BYTE array containing the data that needs to be encrypt.
	2. pnTextLen: This is a reference to the size variable (size_t) representing the size of the data to encrypt.
	3. crText: This is a pointer to the BYTE array containing the data encrypted.
- Return value: true or false
*/
bool AesManager::encryptData(const uint8_t* pnText, const size_t pnTextLen, uint8_t* crText)
{
	if (key_ == NULL || key_size_ == 0) {
		return false;
	}
	uint8_t nonce[NONCE_SIZE];	//TODO: Random nonce here!
	if (generate_nonce(nonce, NONCE_SIZE) == false) {
		return false;
	}
	uint8_t tag[16];
	uint8_t* cipher_data = new uint8_t[pnTextLen];

	AES_GCM_encrypt(key_, nonce, pnText, pnTextLen, NULL, 0, cipher_data, tag);
	memcpy(crText, PREFIX_AES, PREFIX_SIZE);
	memcpy(crText + PREFIX_SIZE, nonce, NONCE_SIZE);
	memcpy(crText + PREFIX_SIZE + NONCE_SIZE, tag, TAG_SIZE);
	memcpy(crText + PREFIX_SIZE + NONCE_SIZE + TAG_SIZE, cipher_data, pnTextLen);

	if (cipher_data != NULL) {
		delete[] cipher_data;
	}
	return true;
}

/*
- Purpose: This function is used to decrypt data encrypted using the AES-GCM algorithm.
- Systax:
	1. [IN]		uint8_t		crText
	2. [IN]		size_t		crTextLen
	3. [OUT]	uint8_t		pnText
- Parameters:
	1. crText: This is a pointer to the BYTE array containing the data that needs to be decrypt.
	2. crTextLen: This is a reference to the size variable (size_t) representing the size of the data to decrypt.
	3. pnText: This is a pointer to the BYTE array containing the data decrypted.
- Return value: true or false
*/
bool AesManager::decryptData(const uint8_t* crText, const size_t crTextLen, uint8_t* pnText)
{
	if (key_ == NULL || key_size_ == 0) {
		return false;
	}
	uint8_t prefix[PREFIX_SIZE];
	uint8_t nonce[NONCE_SIZE];
	uint8_t tag[TAG_SIZE];
	size_t pnTextLen = crTextLen - PREFIX_SIZE - NONCE_SIZE - TAG_SIZE;
	uint8_t* plaint_data = new uint8_t[pnTextLen];

	memcpy(prefix, crText, PREFIX_SIZE);
	memcpy(nonce, crText + PREFIX_SIZE, NONCE_SIZE);
	memcpy(tag, crText + PREFIX_SIZE + NONCE_SIZE, TAG_SIZE);
	memcpy(plaint_data, crText + PREFIX_SIZE + NONCE_SIZE + TAG_SIZE, pnTextLen);
	//std::lock_guard<std::mutex> lock(mutex);
	if (AES_GCM_decrypt(key_, nonce, plaint_data, pnTextLen, NULL, 0, tag, pnText) == AUTHENTICATION_FAILURE) 
	{
		delete[] plaint_data;
		return false;
	}
	if (plaint_data != NULL) {
		delete[] plaint_data;
	}
	return true;
}