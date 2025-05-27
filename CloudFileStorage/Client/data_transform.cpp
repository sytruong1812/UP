#include "data_transform.h"

namespace NetworkOperations 
{
	BOOL DataCompress::TransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out)
	{
		return TRUE;
	}

	BOOL DataCompress::ReverseTransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out)
	{
		return TRUE;
	}

	BOOL DataCryptor::TransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out)
	{
		return TRUE;
	}

	BOOL DataCryptor::ReverseTransformData(const BYTE* data_in, DWORD length_in, BYTE*& data_out, DWORD& length_out)
	{
		return TRUE;
	}
}