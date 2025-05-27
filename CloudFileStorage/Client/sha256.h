#pragma once
#include <windows.h>
#include <iostream>
#include <stdint.h>

#define SHA256_BLOCK_LENGTH		64
#define SHA256_DIGEST_LENGTH    32
#define DBL_INT_ADD(a, b, c) if (a > 0xffffffff - (c)) ++b; a += c;
#define SHFR(a, b) (a >> b)
#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (SHA256_DIGEST_LENGTH - (b))))
#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (SHA256_DIGEST_LENGTH - (b))))

#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ (SHFR(x,3)))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ (SHFR(x,10)))

namespace Crypto 
{
	typedef struct
	{
		uint8_t block[SHA256_BLOCK_LENGTH];
		uint32_t index;
		uint32_t bitcount[2];
		uint32_t state[8];
	} SHA256_CTX;

	void SHA256_Init(SHA256_CTX * ctx);
	void SHA256_Update(SHA256_CTX * ctx, uint8_t* data, uint32_t len);
	void SHA256_Final(SHA256_CTX * ctx, uint8_t digest[SHA256_DIGEST_LENGTH]);
	BOOL SHA256_Hash(BYTE* data, size_t len, BYTE* digest);

}
