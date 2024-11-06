#pragma once
#include <mutex>
#include <random>
#include <chrono>
#include "aes_tiny.h"

class AesManager 
{
private:
	uint8_t* key_;
	size_t key_size_;
	bool decrypt_flag_;
	bool encrypt_flag_;
	static AesManager* s_instance;
	//std::mutex mutex;
	AesManager();
	~AesManager();
public:
	static AesManager* instance()
	{
		if (!s_instance)
		{
			s_instance = new AesManager;
		}
		return s_instance;
	}
	void getKey(uint8_t* key, size_t& size);
	void setKey(const uint8_t* key, const size_t size);
	void setDecryptFlag(bool option);
	bool getDecryptFlag();
	void setEncryptFlag(bool option);
	bool getEncryptFlag();
	bool generate_nonce(uint8_t* data, size_t length);
	size_t getSizePlainText(const size_t crTextLen);
	size_t getSizeCipherText(const size_t pnTextLen);
	bool encryptData(const uint8_t* pnText, const size_t pnTextLen, uint8_t* crText);
	bool decryptData(const uint8_t* crText, const size_t crTextLen, uint8_t* pnText);
};

