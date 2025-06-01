#pragma once
#include "utils.h"
#include "aes_gcm.h"
#include "zlib/zip.h"
#include "zlib/unzip.h"

namespace NetworkOperations 
{
	class IDataTransform 
	{
	public:
		virtual ~IDataTransform() = default;
		virtual BOOL TransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out) = 0;
		virtual BOOL ReverseTransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out) = 0;
	};

	class DataCompress : IDataTransform {
	private:
		std::string password_;
	public:
		DataCompress() {}
		~DataCompress() {}
		BOOL TransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out) override;
		BOOL ReverseTransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out) override;
	};

	class DataCryptor : IDataTransform {
	private:
		std::string key_;
		std::string iv_;
	public:
		DataCryptor(std::string key, std::string iv) : key_(key), iv_(iv) {}
		~DataCryptor() {}
		BOOL TransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out) override;
		BOOL ReverseTransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out) override;
	};
}