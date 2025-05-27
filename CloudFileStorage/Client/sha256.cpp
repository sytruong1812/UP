#include "sha256.h"

namespace Crypto {

	static uint32_t k[64] = {
		0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
		0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
		0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
		0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
		0xe49b69c1UL, 0xefbe4786UL, 0xfc19dc6UL, 0x240ca1ccUL,
		0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
		0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
		0xc6e00bf3UL, 0xd5a79147UL, 0x6ca6351UL, 0x14292967UL,
		0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
		0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
		0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
		0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
		0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
		0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
		0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
		0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL,
	};

	static void SHA256Transform(SHA256_CTX* ctx, uint8_t* data)
	{
		uint32_t i, j;
		uint32_t m[64];
		uint32_t t1, t2;
		uint32_t a, b, c, d, e, f, g, h;

		for (i = 0, j = 0; i < 16; ++i, j += 4)
		{
			m[i] = (data[j] << 24)
				| (data[j + 1] << 16)
				| (data[j + 2] << 8)
				| (data[j + 3]);
		}
		for (; i < 64; ++i)
		{
			m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
		}

		a = ctx->state[0];
		b = ctx->state[1];
		c = ctx->state[2];
		d = ctx->state[3];
		e = ctx->state[4];
		f = ctx->state[5];
		g = ctx->state[6];
		h = ctx->state[7];

		for (i = 0; i < 64; ++i)
		{
			t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
			t2 = EP0(a) + MAJ(a, b, c);
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		ctx->state[0] += a;
		ctx->state[1] += b;
		ctx->state[2] += c;
		ctx->state[3] += d;
		ctx->state[4] += e;
		ctx->state[5] += f;
		ctx->state[6] += g;
		ctx->state[7] += h;
	}

	void SHA256_Init(SHA256_CTX* ctx)
	{
		ctx->index = 0;
		ctx->bitcount[0] = 0;
		ctx->bitcount[1] = 0;
		ctx->state[0] = 0x6a09e667UL;
		ctx->state[1] = 0xbb67ae85UL;
		ctx->state[2] = 0x3c6ef372UL;
		ctx->state[3] = 0xa54ff53aUL;
		ctx->state[4] = 0x510e527fUL;
		ctx->state[5] = 0x9b05688cUL;
		ctx->state[6] = 0x1f83d9abUL;
		ctx->state[7] = 0x5be0cd19UL;
	}

	void SHA256_Update(SHA256_CTX* ctx, uint8_t* data, uint32_t len)
	{
		for (uint32_t i = 0; i < len; ++i)
		{
			ctx->block[ctx->index] = data[i];
			ctx->index++;
			if (ctx->index == 64)
			{
				SHA256Transform(ctx, ctx->block);
				DBL_INT_ADD(ctx->bitcount[0], ctx->bitcount[1], 512);
				ctx->index = 0;
			}
		}
	}

	void SHA256_Final(SHA256_CTX* ctx, uint8_t digest[SHA256_DIGEST_LENGTH])
	{
		uint32_t i = ctx->index;
		if (ctx->index < 56)
		{
			ctx->block[i++] = 0x80;
			while (i < 56)
			{
				ctx->block[i++] = 0x00;
			}
		}
		else
		{
			ctx->block[i++] = 0x80;
			while (i < 64)
			{
				ctx->block[i++] = 0x00;
			}
			SHA256Transform(ctx, ctx->block);
			memset(ctx->block, 0, 56);
		}

		DBL_INT_ADD(ctx->bitcount[0], ctx->bitcount[1], ctx->index * 8);
		ctx->block[63] = (uint32_t)ctx->bitcount[0];
		ctx->block[62] = (uint32_t)(ctx->bitcount[0] >> 8 & 0xFF);
		ctx->block[61] = (uint32_t)(ctx->bitcount[0] >> 16 & 0xFF);
		ctx->block[60] = (uint32_t)(ctx->bitcount[0] >> 24 & 0xFF);
		ctx->block[59] = (uint32_t)ctx->bitcount[1];
		ctx->block[58] = (uint32_t)(ctx->bitcount[1] >> 8 & 0xFF);
		ctx->block[57] = (uint32_t)(ctx->bitcount[1] >> 16 & 0xFF);
		ctx->block[56] = (uint32_t)(ctx->bitcount[1] >> 24 & 0xFF);
		SHA256Transform(ctx, ctx->block);

		for (i = 0; i < 4; ++i)
		{
			digest[i] = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
			digest[i + 4] = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
			digest[i + 8] = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
			digest[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
			digest[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
			digest[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
			digest[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
			digest[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
		}
	}

	BOOL SHA256_Hash(BYTE* data, size_t len, BYTE* digest)
	{
		if (!digest || !data)
		{
			return FALSE;
		}
		SHA256_CTX c;
		SHA256_Init(&c);
		SHA256_Update(&c, data, (uint32_t)len);
		SHA256_Final(&c, digest);
		return TRUE;
	}
}