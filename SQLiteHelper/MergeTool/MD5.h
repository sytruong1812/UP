#pragma once
#include <iostream>
#include <windows.h>

typedef unsigned char* POINTER;		//Pointer defines a generic pointer type
typedef unsigned short int UINT2;	//UINT2 defines a two byte word
typedef unsigned long int UINT4;	//UINT4 defines a four byte word

#define MD5_DIGEST_LENGTH 16

/* Constants for MD5Transform routine */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* x, y, z: are 32 bit state variables */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/*
FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
}

#define GG(a, b, c, d, x, s, ac) { \
    (a) += G((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
}

#define HH(a, b, c, d, x, s, ac) { \
    (a) += H((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
}

#define II(a, b, c, d, x, s, ac) { \
    (a) += I((b), (c), (d)) + (x) + (UINT4)(ac); \
    (a) = ROTATE_LEFT((a), (s)); \
    (a) += (b); \
}

/*
ROTATE_LEFT rotates x left n bits.
For example: Rotate left x = 0x12345678 by 8 bits: ROTALE_LEFT(x, 8)
	x = 0001 0010 0011 0100 0101 0110 0111 1000
	Shift right 24 bits (32-8=24)
	x = 0010 0110 0100 1010 0011 1000 0000 0000
	OR: Translation result with x
	x new = 0010 0110 0100 1010 0011 1010 0100 1100
	x new = 0x2697284C
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

typedef struct
{
	UINT4 state[4];				//State (ABCD)
	UINT4 count[2];				//Number of bits, modulo 2^64 (lsb first)
	BYTE buffer[64];	//Input buffer;
} MD5_CTX;

bool MD5Print(BYTE* digest);
bool MD5Hash(BYTE* data, size_t dataLen, BYTE* digest);
void MD5Init(MD5_CTX* context);
void MD5Update(MD5_CTX* context, BYTE* input, size_t inputLen);
void MD5Final(MD5_CTX* context, BYTE digest[16]);
