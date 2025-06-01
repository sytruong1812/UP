#include <string>
#include "aes_gcm.h"
#include <random>

#define KEY_SIZE		AES_KEY_SIZE
#define BLOCK_SIZE		(128 / 8)			/* Block length in AES is 'always' 128-bits.    */
#define Nb				(BLOCK_SIZE / 4)	/* The number of columns comprising a AES state */
#define Nk				(KEY_SIZE / 4)		/* The number of 32 bit words in a key.         */
#define ROUNDS			(Nk + 6)			/* The number of rounds in AES Cipher.          */

#define IMPLEMENT(x) (x) > 0

#define INCREASE_SECURITY  0				/* for more info, see the bottom of header file */
#define DONT_USE_FUNCTIONS 0
#define SMALL_CIPHER       0

namespace Crypto 
{
	/** Lookup-tables are "static constant", so that they can be placed in read-only
	 * storage instead of RAM. They can be computed dynamically trading ROM for RAM.
	 * This may be useful in (embedded) bootloader applications, where ROM is often
	 * limited. Note that sbox[y] = x, if and only if rsbox[x] = y. More details can
	 * be found at this wikipedia page: https://en.wikipedia.org/wiki/Rijndael_S-box
	 */
	static const char sbox[257] =
		"c|w{\362ko\3050\01g+\376\327\253v\312\202\311}""\372YG\360\255\324\242\257"
		"\234\244r\300\267\375\223&6\?\367\3144\245\345\361q\3301\25\4\307#\303\030"
		"\226\5\232\a\22\200\342\353\'\262u\t\203,\32\33nZ\240R;\326\263)\343/\204S"
		"\321\0\355 \374\261[j\313\2769JLX\317\320\357\252\373CM3\205E\371\02\177P<"
		"\237\250Q\243@\217\222\2358\365\274\266\332!\20\377\363\322\315\f\023\354_"
		"\227D\27\304\247~=d]\31s`\201O\334\"*\220\210F\356\270\24\336^\v\333\3402:"
		"\nI\06$\\\302\323\254b\221\225\344y\347\3107m\215\325N\251lV\364\352ez\256"
		"\b\272x%.\034\246\264\306\350\335t\37K\275\213\212p>\265fH\3\366\16a5W\271"
		"\206\301\035\236\341\370\230\21i\331\216\224\233\036\207\351\316U(\337\214"
		"\241\211\r\277\346BhA\231-\17\260T\273\26";

	static const char rsbox[257] =
		"R\tj\32506\2458\277@\243\236\201\363\327\373|\3439\202\233/\377\2074\216CD"
		"\304\336\351\313T{\2242\246\302#=\356L\225\vB\372\303N\b.\241f(\331$\262v["
		"\242Im\213\321%r\370\366d\206h\230\026\324\244\\\314]e\266\222lpHP\375\355"
		"\271\332^\25FW\247\215\235\204\220\330\253\0\214\274\323\n\367\344X\05\270"
		"\263E\6\320,\036\217\312?\17\2\301\257\275\3\1\023\212k:\221\21AOg\334\352"
		"\227\362\317\316\360\264\346s\226\254t\"\347\2555\205\342\3717\350\34u\337"
		"nG\361\32q\35)\305\211o\267b\16\252\30\276\33\374V>K\306\322y \232\333\300"
		"\376x\315Z\364\037\335\2503\210\a\3071\261\22\20Y\'\200\354_`Q\177\251\031"
		"\265J\r-\345z\237\223\311\234\357\240\340;M\256*\365\260\310\353\273<\203S"
		"\231a\027+\004~\272w\326&\341i\024cU!\f}";

	typedef uint8_t block_t[BLOCK_SIZE];
	typedef uint8_t state_t[Nb][4];

#define SBoxValue(x)       ( sbox[x])
#define InvSBoxValue(x)    (rsbox[x])    /* omitted dynamic s-box calculation */

#define COPY32BIT(x, y)   *(int32_t*) &y  = *(int32_t*) &x
#define XOR32BITS(x, y)   *(int32_t*) &y ^= *(int32_t*) &x

	/** XOR two 128bit numbers (blocks) called src and dest, so that: dest ^= src */
	static void xorBlock(const block_t src, block_t dest)
	{
		uint8_t i;
		for (i = 0; i < BLOCK_SIZE; ++i)      /* many CPUs have single instruction */
		{                                    /*  such as XORPS for 128-bit-xor.   */
			dest[i] ^= src[i];               /* see the file: x86-improvements    */
		}
	}

	/** doubling in GF(2^8): left-shift and if carry bit is set, xor it with 0x1b */
	static uint8_t xtime(uint8_t x)
	{
		return (x > 0x7f) * 0x1b ^ (x << 1);
	}

	static void KeyExpansion(const uint8_t* key, uint8_t* RoundKey)
	{
		uint8_t rcon = 1, i;

		for (i = KEY_SIZE; i < BLOCK_SIZE * (ROUNDS + 1); i += 4)
		{
			switch (i % KEY_SIZE)
			{
				case 0:
					memcpy(&RoundKey[i], &RoundKey[i - KEY_SIZE], KEY_SIZE);
#if Nk == 4
					if (!rcon)  rcon = 0x1b;     /* RCON may reach 0 only in AES-128. */
#endif
					RoundKey[i] ^= SBoxValue(RoundKey[i - 3]) ^ rcon;
					RoundKey[i + 1] ^= SBoxValue(RoundKey[i - 2]);
					RoundKey[i + 2] ^= SBoxValue(RoundKey[i - 1]);
					RoundKey[i + 3] ^= SBoxValue(RoundKey[i - 4]);
					rcon <<= 1;
					break;
#if Nk == 8                              /* additional round only for AES-256 */
				case 16:
					RoundKey[i] ^= SBoxValue(RoundKey[i - 4]);
					RoundKey[i + 1] ^= SBoxValue(RoundKey[i - 3]);
					RoundKey[i + 2] ^= SBoxValue(RoundKey[i - 2]);
					RoundKey[i + 3] ^= SBoxValue(RoundKey[i - 1]);
					break;
#endif
				default:
					XOR32BITS(RoundKey[i - 4], RoundKey[i]);
					break;
			}
		}
	}

	/** Add the round keys to the Rijndael state matrix (adding in GF means XOR). */
	static void AddRoundKey(const uint8_t round, block_t state, uint8_t* RoundKey)
	{
		xorBlock(RoundKey + BLOCK_SIZE * round, state);
	}

	/** Substitute values in the state matrix with associated values in the S-box */
	static void SubBytes(block_t state)
	{
		uint8_t i;
		for (i = 0; i < BLOCK_SIZE; ++i)
		{
			state[i] = SBoxValue(state[i]);
		}
	}

	/** Shift/rotate the rows of the state matrix to the left. Each row is shifted
	 * with a different offset (= Row number). So the first row won't be shifted. */
	static void ShiftRows(state_t* state)
	{
		uint8_t   temp = (*state)[0][1];
		(*state)[0][1] = (*state)[1][1];
		(*state)[1][1] = (*state)[2][1];
		(*state)[2][1] = (*state)[3][1];
		(*state)[3][1] = temp;           /* Rotated the 1st row 1 columns to left */

		temp = (*state)[0][2];
		(*state)[0][2] = (*state)[2][2];
		(*state)[2][2] = temp;
		temp = (*state)[1][2];
		(*state)[1][2] = (*state)[3][2];
		(*state)[3][2] = temp;           /* Rotated the 2nd row 2 columns to left */

		temp = (*state)[0][3];
		(*state)[0][3] = (*state)[3][3];
		(*state)[3][3] = (*state)[2][3];
		(*state)[2][3] = (*state)[1][3];
		(*state)[1][3] = temp;           /* Rotated the 3rd row 3 columns to left */
	}

	/** Use matrix multiplication in Galois field to mix the columns of the state */
	static void MixColumns(state_t* state)
	{
		uint8_t col[4], i;
		for (i = 0; i < Nb; ++i)         /* see: crypto.stack exchange.com/q/2402  */
		{
			COPY32BIT((*state)[i][0], col[0]);
			col[3] ^= col[1];
			col[1] = xtime(col[1] ^ col[0]);
			col[0] ^= col[2];
			col[2] = xtime(col[0]);
			col[0] ^= col[3];           /* xor of all elements in column = *col  */
			col[3] = xtime(col[3]);

			(*state)[i][0] ^= col[0] ^= col[1];
			(*state)[i][1] ^= col[0] ^= col[2];
			(*state)[i][2] ^= col[0] ^= col[3];
			(*state)[i][3] ^= col[0] ^= col[2];
		}
	}

	/** Encrypt a plaintext input block and save the result/ciphertext as output. */
	static void rijndaelEncrypt(const block_t input, block_t output, uint8_t* RoundKey)
	{
		uint8_t r;
		state_t* state = reinterpret_cast<state_t*>(output);

		/* copy the input to the state matrix, and beware of undefined behavior.. */
		if (input != output)   memcpy(state, input, BLOCK_SIZE);

		/* The encryption is carried out in #ROUNDS iterations, of which the first
		 * #ROUNDS-1 are identical. The last round doesn't involve mixing columns */
		for (r = 0; r != ROUNDS; )
		{
			AddRoundKey(r, output, RoundKey);
			SubBytes(output);
			ShiftRows(state);
			++r != ROUNDS ? MixColumns(state) : AddRoundKey(ROUNDS, output, RoundKey);
		}
	}

	/** function-pointer types, indicating functions that take fixed-size blocks: */
	typedef void (*fdouble_t)(block_t);
	typedef void (*fmix_t)(const block_t, block_t);
	typedef void (*fmix_t2)(const block_t, block_t, uint8_t*);

#define LAST                (BLOCK_SIZE - 1)      /*  last index in a block    */

#if INCREASE_SECURITY
#define BURN(key)           memset( key, 0, sizeof key )
#define SABOTAGE(buf, len)  memset( buf, 0, len )
#define MISMATCH            constmemcmp          /*  a.k.a secure memcmp      */
#else
#define MISMATCH            memcmp
#define SABOTAGE(buf, len)  (void)  buf
#define BURN(key)           (void)  key          /*  the line will be ignored */
#endif

	typedef size_t  count_t;

	/** xor a byte array with a big-endian integer, whose LSB is at specified pos */
	static void xorBEint(uint8_t* buff, size_t num, uint8_t pos)
	{
		do
			buff[pos--] ^= (uint8_t)num;
		while (num >>= 8);
	}

	/** increment the value of a 128-bit counter block, regarding its endian-ness */
	static void incBlock(block_t block, const char big)
	{
		uint8_t i;
		if (big)                                     /*  big-endian counter       */
		{
			for (i = LAST; !++block[i]; )  --i;      /*  increment the LSB,       */
		}                                            /*  ..until no overflow      */
		else
		{
			for (i = 0; !++block[i] && ++i < 4; );
		}
	}

	/** Divide a 128-bit number (big-endian block) by two in Galois field GF(128) */
	static void divideBblock(block_t array)
	{
		unsigned i, c = 0;
		for (i = 0; i < BLOCK_SIZE; c <<= 8)          /* from first to last byte,  */
		{                                            /*  prepend the previous LSB */
			c |= array[i];                           /*  then shift it to right.  */
			array[i++] = (uint8_t)(c >> 1);
		}                                            /* if block is odd (LSB = 1) */
		if (c & 0x100)  array[0] ^= 0xe1;            /* .. B ^= 11100001b << 120  */
	}

	/** Multiplication of two 128-bit numbers (big-endian blocks) in Galois field */
	static void mulGF128(const block_t x, block_t y)
	{
		uint8_t b, i;
		block_t result = { 0 };                      /*  working memory           */

		for (i = 0; i < BLOCK_SIZE; ++i)
		{
			for (b = 0x80; b; b >>= 1)               /*  check all the bits of X, */
			{
				if (x[i] & b)                        /*  ..and if any bit is set, */
				{
					xorBlock(y, result);             /*  ..add Y to the result    */
				}
				divideBblock(y);                     /*  Y_next = (Y / 2) in GF   */
			}
		}
		memcpy(y, result, sizeof result);            /*  result is saved into y   */
	}

	static void mixThenXor(fmix_t2 mix, const block_t B, block_t f, uint8_t* w,
		const uint8_t* X, uint8_t n, uint8_t* Y)
	{
		if (n == 0)  return;
		mix(B, f, w);                                 /*  Y = f(B) ^ X             */
		while (n--)
		{
			Y[n] = f[n] ^ X[n];
		}
	}

	/** xor the result with input data and then apply the digest/mixing function.
	 * repeat the process for each block of data until all blocks are digested... */
	static void xMac(const void* data, const size_t dataSize,
		const block_t seed, fmix_t mix, block_t result)
	{
		const uint8_t* x = reinterpret_cast<const uint8_t*>(data);
		count_t n = dataSize / BLOCK_SIZE;           /*   number of full blocks   */

		for (; n--; x += BLOCK_SIZE)
		{
			xorBlock(x, result);					/* M_next = mix(seed, M ^ X) */
			mix(seed, result);
		}
		for (n = dataSize % BLOCK_SIZE; n--; )       /* if any partial block left */
		{
			result[n] ^= x[n];
			if (n == 0)
			{
				mix(seed, result);
			}
		}
	}

	static void CTR_Cipher(const block_t iCtr, const char big,
		const void* input, const size_t dataSize, void* output, uint8_t* RoundKey)
	{
		block_t c, enc;
		count_t n = dataSize / BLOCK_SIZE;
		uint8_t* y = reinterpret_cast<uint8_t*>(output);;

		memcpy(output, input, dataSize);           /* do in-place en/decryption */
		memcpy(c, iCtr, sizeof c);
		if (big > 1)
		{
			incBlock(c, big);                      /*  pre-increment in CCM/GCM */
		}
		else if (!big)
		{
			c[LAST] |= 0x80;                       /*  set one bit in GCM-SIV   */
		}
		for (; n--; y += BLOCK_SIZE)
		{
			rijndaelEncrypt(c, enc, RoundKey);     /*  both in en[de]cryption:  */
			xorBlock(enc, y);                      /*  Y = Enc(Ctr) ^ X         */
			incBlock(c, big);                      /*  Ctr_next = Ctr + 1       */
		}
		mixThenXor(&rijndaelEncrypt, c, c, RoundKey, y, dataSize % BLOCK_SIZE, y);
	}

	/** calculate the GMAC of ciphertext and AAD using an authentication subkey H */
	static void GHash(const block_t H, const void* aData, const void* crtxt,
		const size_t aDataLen, const size_t crtxtLen, block_t gsh)
	{
		block_t len = { 0 };
		xorBEint(len, aDataLen * 8, LAST / 2);
		xorBEint(len, crtxtLen * 8, LAST);         /*  save bit-sizes into len  */

		xMac(aData, aDataLen, H, &mulGF128, gsh);  /*  first digest AAD, then   */
		xMac(crtxt, crtxtLen, H, &mulGF128, gsh);  /*  ..ciphertext, and then   */
		xMac(len, sizeof len, H, &mulGF128, gsh);  /*  ..bit sizes into GHash   */
	}

	/** encrypt zeros to get authentication subkey H, and prepare the IV for GCM. */
	static void GCM_Init(const uint8_t* key,
		const uint8_t* nonce,
		block_t authKey,
		block_t iv, uint8_t* RoundKey)
	{
		memcpy(RoundKey, key, KEY_SIZE);
		KeyExpansion(key, RoundKey);
		rijndaelEncrypt(authKey, authKey, RoundKey);         /* authKey = Enc(zero block) */
#if GCM_NONCE_LEN != 12
		GHash(authKey, NULL, nonce, 0, GCM_NONCE_LEN, iv);
#else
		memcpy(iv, nonce, 12);
		iv[LAST] = 1;
#endif
	}

	void AES_GCM_encrypt(const uint8_t* key, const uint8_t* nonce,
		const uint8_t* pntxt, const size_t ptextLen,
		const uint8_t* aData, const size_t aDataLen,
		uint8_t* crtxt, uint8_t auTag[16])
	{
		block_t H = { 0 }, iv = { 0 }, gsh = { 0 };
		uint8_t RoundKey[BLOCK_SIZE * ROUNDS + KEY_SIZE];
		GCM_Init(key, nonce, H, iv, RoundKey);               /*  get IV & auth. subkey H  */

		CTR_Cipher(iv, 2, pntxt, ptextLen, crtxt, RoundKey);
		rijndaelEncrypt(iv, auTag, RoundKey);                /*  tag = Enc(iv) ^ GHASH    */
		BURN(RoundKey);
		GHash(H, aData, crtxt, aDataLen, ptextLen, gsh);
		xorBlock(gsh, auTag);
	}

	char AES_GCM_decrypt(const uint8_t* key, const uint8_t* nonce,
		const uint8_t* crtxt, const size_t crtxtLen,
		const uint8_t* aData, const size_t aDataLen,
		const uint8_t* auTag, uint8_t* pntxt)
	{
		block_t H = { 0 }, iv = { 0 }, gsh = { 0 };
		uint8_t RoundKey[BLOCK_SIZE * ROUNDS + KEY_SIZE];
		GCM_Init(key, nonce, H, iv, RoundKey);
		GHash(H, aData, crtxt, aDataLen, crtxtLen, gsh);

		rijndaelEncrypt(iv, H, RoundKey);
		xorBlock(H, gsh);                          /*  tag = Enc(iv) ^ GHASH    */
		if (MISMATCH(gsh, auTag, 16))
		{                                          /*  compare tags and         */
			BURN(RoundKey);                        /*  ..proceed if they match  */
			return AUTHENTICATION_FAILURE;
		}
		CTR_Cipher(iv, 2, crtxt, crtxtLen, pntxt, RoundKey);
		BURN(RoundKey);
		return NO_ERROR_RETURNED;
	}

	bool make_random_bytes(uint8_t* data, size_t length)
	{
		if (data == NULL)
		{
			return false;
		}
		std::random_device rd;  // Seed for random number generation
		std::mt19937 generator(rd());   // Mersenne Twister RNG
		std::uniform_int_distribution<int> distribution(0, 255);    // Range for bytes
		for (size_t i = 0; i < length; ++i)
		{
			data[i] = static_cast<uint8_t>(distribution(generator));
		}
		return true;
	}

	size_t getPlainTextSize(const size_t crTextLen)
	{
		return crTextLen - PREFIX_SIZE - GCM_NONCE_LEN - TAG_SIZE;
	}
	size_t getCipherTextSize(const size_t pnTextLen)
	{
		return pnTextLen + PREFIX_SIZE + GCM_NONCE_LEN + TAG_SIZE;
	}

	bool encryptData(const uint8_t* pnText, const size_t pnTextLen,
		const uint8_t* key_v2, const size_t key_size,
		uint8_t* crText)
	{
		if (key_v2 == NULL || key_size == 0)
		{
			return false;
		}
		//Step1
		uint8_t nonce[GCM_NONCE_LEN];	//TODO: Random nonce(iv) here!
		if (make_random_bytes(nonce, GCM_NONCE_LEN) == false)
		{
			return false;
		}
		//Step2
		uint8_t tag[BLOCK_SIZE];
		size_t encrypt_size = pnTextLen;
		uint8_t* encrypt_data = new uint8_t[encrypt_size];
		AES_GCM_encrypt(key_v2, nonce, pnText, pnTextLen, NULL, 0, encrypt_data, tag);

		//Step3: Prefix (3 bytes) +  Nonce (12 bytes) + Cipher Data + Tag (16 bytes)
		memcpy(crText, PREFIX_NAME, PREFIX_SIZE);
		memcpy(crText + PREFIX_SIZE, nonce, GCM_NONCE_LEN);
		memcpy(crText + PREFIX_SIZE + GCM_NONCE_LEN, encrypt_data, encrypt_size);
		memcpy(crText + PREFIX_SIZE + GCM_NONCE_LEN + encrypt_size, tag, TAG_SIZE);

		if (encrypt_data != NULL)
		{
			delete[] encrypt_data;
		}
		return true;
	}

	bool decryptData(const uint8_t* crText, const size_t crTextLen,
		const uint8_t* key_v1, const size_t key1_size,
		const uint8_t* key_v2, const size_t key2_size,
		uint8_t* pnText)
	{
		//Step1
		uint8_t prefix[PREFIX_SIZE];
		uint8_t nonce[GCM_NONCE_LEN];
		uint8_t tag[TAG_SIZE];
		size_t decrypt_size = getPlainTextSize(crTextLen);
		uint8_t* decrypt_data = new uint8_t[decrypt_size];

		//Prefix (3 bytes) +  Nonce (12 bytes) + Cipher Data + Tag (16 bytes)
		memcpy(prefix, crText, PREFIX_SIZE);
		memcpy(nonce, crText + PREFIX_SIZE, GCM_NONCE_LEN);
		memcpy(decrypt_data, crText + PREFIX_SIZE + GCM_NONCE_LEN, decrypt_size);
		memcpy(tag, crText + PREFIX_SIZE + GCM_NONCE_LEN + decrypt_size, TAG_SIZE);

		//Step2
		const uint8_t* key_decrypt = NULL;
		size_t key_decrypt_size = 0;
		if (memcmp(prefix, VERSION_1, sizeof(VERSION_1)) == 0)
		{
			if (key_v1 == NULL || key1_size == 0)
			{
				delete[] decrypt_data;
				return false;
			}
			key_decrypt = key_v1;
			key_decrypt_size = key1_size;
		}
		else if (memcmp(prefix, VERSION_2, sizeof(VERSION_2)) == 0)
		{
			if (key_v2 == NULL || key2_size == 0)
			{
				delete[] decrypt_data;
				return false;
			}
			key_decrypt = key_v2;
			key_decrypt_size = key2_size;
		}
		else
		{
			delete[] decrypt_data;
			return false;
		}

		//Step3
		if (AES_GCM_decrypt(key_decrypt, nonce, decrypt_data, decrypt_size, NULL, 0, tag, pnText) == AUTHENTICATION_FAILURE)
		{
			delete[] decrypt_data;
			return false;
		}
		if (decrypt_data != NULL)
		{
			delete[] decrypt_data;
		}
		return true;
	}
}