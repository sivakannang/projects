// gcc tripledes.c -o tripledes
//
// Triple DES, ISO-0 PIN blocks and DUKPT key derivation (ANSI X9.24), checked against the
// published test vectors in main.
//
// For learning and for working with legacy payment systems. DES is broken, and NIST has
// retired Triple DES. ECB also reveals repeated blocks, so don't use it for general data.
// New code should use AES-GCM from a maintained library such as OpenSSL.

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint32_t card;
typedef unsigned char byte;

#define true 1
#define false 0

#define _SUCCESS 0
#define _FAIL -1

#define _KEY_LEN_TRIPLE 3
#define _KEY_LEN_DOUBLE 2
#define _KEY_LEN_SINGLE 1

#define BLOCK_LEN 8
#define KSN_LEN 10

static const byte KEY_MASK[16] = {0xC0,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00,0xC0,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00};
static const byte PEK_MASK[16] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};

void _TDES_ENCRYPT(byte *dst, const byte *src, const byte *key, int keyingOption);
void _TDES_DECRYPT(byte *dst, const byte *src, const byte *key, int keyingOption);
void stdDES(byte *dst, const byte *src, const byte *key);
void stdSED(byte *dst, const byte *src, const byte *key);

int SEC_ECBCipher(const byte *src, int srcLen, byte *dst, const byte *key, byte keyLen, byte isCipher);
int SEC_CBCCipher(const byte *src, int srcLen, byte *dst, const byte *key, byte keyLen, byte isCipher);

void xorBytes(const byte *b1, const byte *b2, byte *dst, int len);
int hex2bin(byte *bin, const char *hex, int len);
void printHexDump(const char *title, const byte *str, int len);

void getCheckValue(const byte *key, byte *kcv);
int getPinBlock(const char *pan, const char *pin, byte *pinBlock);

void derive_ipek(const byte *bdk, const byte *ksn, byte *ipek);
void dukpt_black_box(const byte *current_sk, const byte *ksn_reg, byte *future_key);
void derive_key(const byte *ipek, const byte *ksn, byte *derivedkey);
void derive_pin_key(const byte *ipek, const byte *ksn, byte *pinKey);


void _TDES_ENCRYPT(byte *dst, const byte *src, const byte *key, int keyingOption)
{
	byte tmp[BLOCK_LEN];

	switch (keyingOption)
	{
	case _KEY_LEN_TRIPLE: stdDES(dst, src, key), stdSED(tmp, dst, key+8), stdDES(dst, tmp, key+16); break;
	case _KEY_LEN_DOUBLE: stdDES(dst, src, key), stdSED(tmp, dst, key+8), stdDES(dst, tmp, key); break;
	case _KEY_LEN_SINGLE: default: stdDES(dst, src, key), stdSED(tmp, dst, key), stdDES(dst, tmp, key); break;
	}
}


void _TDES_DECRYPT(byte *dst, const byte *src, const byte *key, int keyingOption)
{
	byte tmp[BLOCK_LEN];

	switch (keyingOption)
	{
	case _KEY_LEN_TRIPLE: stdSED(dst, src, key+16), stdDES(tmp, dst, key+8), stdSED(dst, tmp, key); break;
	case _KEY_LEN_DOUBLE: stdSED(dst, src, key), stdDES(tmp, dst, key+8), stdSED(dst, tmp, key); break;
	case _KEY_LEN_SINGLE: default: stdSED(dst, src, key); stdDES(tmp, dst, key), stdSED(dst, tmp, key); break;
	}
}


static void justdes2(byte * dst, const byte * src, const byte * key, int ctl) {
    card x, c, d, r, l;
    int i;
    static const card s[64] = { /* S-tables and permutation E combined */
        0xD8D8DBBC, 0xD737D1C1, 0x8B047441, 0x35A9E2FE,
        0x146E9560, 0x8A420CFB, 0xF8FBAF1F, 0xC7B4DD10,
        0x7A97A497, 0x4CFCFA1C, 0x456ADA86, 0xFAC710E9,
        0xE52149EF, 0x338D2004, 0x1E5580F1, 0xE04A2F3D,
        0x870A4E20, 0x28BE9C1F, 0x74D5E339, 0x8240BD00,
        0x6AA1ABC3, 0x3F55E2A8, 0xAF1F56BC, 0x51BB11CF,
        0xB7FC035E, 0xE00307B0, 0x08A3B44B, 0x3F786D67,
        0x09967CBC, 0x45EB7B47, 0xF3683962, 0x9C14C6D2,
        0x16452B42, 0xADDACEBA, 0x58F91ABC, 0x8B68B547,
        0xFAA36659, 0x47BF8901, 0x671AEBA9, 0x30C452AB,
        0x493893E1, 0x72C16866, 0xB7C78574, 0xCD1E6B9A,
        0xB6DCD49C, 0x9822B7FB, 0x89B07E43, 0x77B78644,
        0xA566F5DF, 0xD22D6AC3, 0xAF9A0423, 0x77B71BBC,
        0x81DC043E, 0xC8837314, 0x78659153, 0xAF782C7D,
        0x8C0F78A0, 0x0D3095EF, 0x7A506B8E, 0x8445D610,
        0x5223AB47, 0x724C0C34, 0x45AF54BC, 0x38DBF9CB
    };

/* initial permutations IP and PC1 */
    l = r = c = d = 0;
    i = 7;
    do {
        x = src[i];
        l = l << 1 | (x & 1 << 0) | (x & 1 << 2) << 6 | (x & 1 << 4) << 12 | (x & 1 << 6) << 18;
        r = r << 1 | (x & 1 << 1) >> 1 | (x & 1 << 3) << 5 | (x & 1 << 5) << 11 | (x & 1 << 7) << 17;
        x = key[i];
        c = c << 1 | (x & 1 << 7) << 17 | (x & 1 << 6) << 10 | (x & 1 << 5) << 3 | (x & 1 << 4) >> 4;
        d = d << 1 | (x & 1 << 1) << 19 | (x & 1 << 2) << 10 | (x & 1 << 3) << 1;
    } while(--i >= 0);
    d |= c & 0x0F;
    c >>= 4;
    i = 24;
/* first round is special: one left shift on encipher, no shift on decipher */
    if(ctl != 0)
        goto startround;
  leftby1:
    c = c << 1 | ((c >> 27) & 1);
    d = d << 1 | ((d >> 27) & 1);
  startround:
/* a round - apply PC2, the S-boxes and permutation E */
    x = (s[((r >> 26 & 62) | (r & 1)) ^
           ((c >> 6 & 32) | (c >> 13 & 16) | (c >> 1 & 8) | (c >> 25 & 4) |
            (c >> 22 & 2) | (c >> 14 & 1))] & 0x00808202) ^ l;
    x ^= s[(r >> 23 & 63) ^
           ((c >> 20 & 32) | (c << 4 & 16) | (c >> 10 & 8) | (c >> 20 & 4) |
            (c >> 6 & 2)
            | (c >> 18 & 1))] & 0x40084010;
    x ^= s[(r >> 19 & 63) ^
           ((c & 32) | (c >> 5 & 16) | (c >> 13 & 8) | (c >> 22 & 4) |
            (c >> 1 & 2) | (c >> 20 & 1))] & 0x04010104;
    x ^= s[(r >> 15 & 63) ^
           ((c >> 7 & 32) | (c >> 17 & 16) | (c << 2 & 8) | (c >> 6 & 4) |
            (c >> 14 & 2) | (c >> 26 & 1))] & 0x80401040;
    x ^= s[(r >> 11 & 63) ^
           ((d >> 10 & 32) | (d & 16) | (d >> 22 & 8) | (d >> 17 & 4) |
            (d >> 8 & 2) | (d >> 1 & 1))] & 0x21040080;
    x ^= s[(r >> 7 & 63) ^
           ((d >> 21 & 32) | (d >> 12 & 16) | (d >> 2 & 8) | (d >> 9 & 4) |
            (d >> 22 & 2)
            | (d >> 8 & 1))] & 0x10202008;
    x ^= s[(r >> 3 & 63) ^
           ((d >> 7 & 32) | (d >> 3 & 16) | (d >> 14 & 8) | (d << 2 & 4) |
            (d >> 21 & 2) | (d >> 3 & 1))] & 0x02100401;
    x ^= s[((r & 31) | (r >> 26 & 32)) ^
           ((d >> 19 & 32) | (d >> 6 & 16) | (d >> 11 & 8) | (d >> 4 & 4) |
            (d >> 19 & 2)
            | (d >> 27 & 1))] & 0x08020820;
    l = r;
    r = x;
/* decide/perform key shifts */
    if((i & 7) == 0) {
        i -= 2;
        if(ctl == 0)
            goto leftby1;
        c = c >> 1 | (c & 1) << 27;
        d = d >> 1 | (d & 1) << 27;
        goto startround;
    }
    if(i != 6) {
        --i;
        if(ctl == 0) {
            c = c << 2 | (c >> 26 & 3);
            d = d << 2 | (d >> 26 & 3);
            goto startround;
        } else {
            c = c >> 2 | (c & 3) << 26;
            d = d >> 2 | (d & 3) << 26;
            goto startround;
        }
    }
/* final swap and permutations IP` */
    i = 7;
    do {
        *dst++ =
            (byte) ((r & 1 << 0) | (r >> 6 & 1 << 2) | (r >> 12 & 1 << 4) |
                    (r >> 18 & 1 << 6) | (l << 1 & 1 << 1) | (l >> 5 & 1 << 3) |
                    (l >> 11 & 1 << 5) | (l >> 17 & 1 << 7));
        l >>= 1;
        r >>= 1;
    } while(--i >= 0);
}

/* Single DES encrypt and decrypt of one 8-byte block, with an 8-byte key */
void stdDES(byte *dst, const byte *src, const byte *key) {
    justdes2(dst, src, key, 0);
}

void stdSED(byte *dst, const byte *src, const byte *key) {
    justdes2(dst, src, key, 1);
}


static int keyingOptionFor(byte keyLen)
{
	return (keyLen == 16) ? _KEY_LEN_DOUBLE : ((keyLen == 24) ? _KEY_LEN_TRIPLE : _KEY_LEN_SINGLE);
}

// Both ciphers return the bytes processed, or _FAIL if srcLen isn't a whole number of blocks
int SEC_ECBCipher(const byte *src, int srcLen, byte *dst, const byte *key, byte keyLen, byte isCipher)
{
	int i;
	int keyingOption = keyingOptionFor(keyLen);

	if ( srcLen % BLOCK_LEN != 0 )
		return _FAIL;

	for ( i = 0; i < srcLen; i += BLOCK_LEN )
	{
		if ( isCipher )
			_TDES_ENCRYPT(dst+i, src+i, key, keyingOption);
		else
			_TDES_DECRYPT(dst+i, src+i, key, keyingOption);
	}

	return srcLen;
}

// CBC with an all-zero IV. src and dst must not overlap.
int SEC_CBCCipher(const byte *src, int srcLen, byte *dst, const byte *key, byte keyLen, byte isCipher)
{
	int i;
	int keyingOption = keyingOptionFor(keyLen);
	byte previous[BLOCK_LEN] = {0};
	byte block[BLOCK_LEN];

	if ( srcLen % BLOCK_LEN != 0 )
		return _FAIL;

	for ( i = 0; i < srcLen; i += BLOCK_LEN )
	{
		if ( isCipher )
		{
			xorBytes(src+i, previous, block, BLOCK_LEN);
			_TDES_ENCRYPT(dst+i, block, key, keyingOption);
			memcpy(previous, dst+i, BLOCK_LEN);
		}
		else
		{
			_TDES_DECRYPT(block, src+i, key, keyingOption);
			xorBytes(block, previous, dst+i, BLOCK_LEN);
			memcpy(previous, src+i, BLOCK_LEN);
		}
	}

	return srcLen;
}


void xorBytes(const byte *b1, const byte *b2, byte *dst, int len)
{
	while(len--)
		*dst++ = *b1++ ^ *b2++;
}

static int chr2nib(char chr)
{
	if ( chr >= '0' && chr <= '9' ) return chr - '0';
	if ( chr >= 'A' && chr <= 'F' ) return chr - 'A' + 10;
	if ( chr >= 'a' && chr <= 'f' ) return chr - 'a' + 10;
	return -1;
}

// Converts len bytes' worth of hex. Returns the bytes written, stopping at the first invalid character.
int hex2bin(byte *bin, const char *hex, int len)
{
	int ret = 0;

	while ( ret < len )
	{
		int high = chr2nib(hex[ret*2]);
		int low = (high < 0) ? -1 : chr2nib(hex[ret*2 + 1]);

		if ( low < 0 )
			break;

		bin[ret++] = (byte)(high << 4 | low);
	}

	return ret;
}

void printHexDump(const char *title, const byte *str, int len)
{
	int i;

	printf("%s :", title);
	for ( i = 0; i < len; i++ )
		printf(" %02X", str[i]);
	printf("\n");
}


// Key check value: the first 3 bytes of a zero block encrypted with the key.
// It lets two parties confirm they hold the same key without revealing it.
void getCheckValue(const byte *key, byte *kcv)
{
	byte zero[BLOCK_LEN] = {0};
	byte out[BLOCK_LEN];

	_TDES_ENCRYPT(out, zero, key, _KEY_LEN_DOUBLE);
	memcpy(kcv, out, 3);
}


// ISO 9564 format 0 PIN block, before encryption: (0 + PIN length + PIN + F padding) XOR
// (0000 + rightmost 12 PAN digits, not counting the check digit). Never log the PAN, PIN or block.
int getPinBlock(const char *pan, const char *pin, byte *pinBlock)
{
	char pinField[32];   // Only the first 16 characters are used, so the F padding needs no exact count
	char panField[17];
	byte pinBin[BLOCK_LEN];
	byte panBin[BLOCK_LEN];
	int panLen = (int)strlen(pan);
	int pinLen = (int)strlen(pin);

	if ( panLen < 13 || pinLen < 4 || pinLen > 12 )
		return _FAIL;

	snprintf(pinField, sizeof(pinField), "%02d%sFFFFFFFFFFFF", pinLen, pin);
	snprintf(panField, sizeof(panField), "0000%.12s", pan + panLen - 13);

	if ( hex2bin(pinBin, pinField, BLOCK_LEN) != BLOCK_LEN || hex2bin(panBin, panField, BLOCK_LEN) != BLOCK_LEN )
		return _FAIL;

	xorBytes(pinBin, panBin, pinBlock, BLOCK_LEN);

	return _SUCCESS;
}


/************************************ DUKPT (ANSI X9.24) ***********************************
 *
 * Derived Unique Key Per Transaction: every transaction uses a different key, so stealing
 * one transaction key reveals nothing about the others.
 *
 * BDK   Base Derivation Key. Kept in the host's security module, never in a device.
 * KSN   Key Serial Number, 10 bytes, sent with each transaction. It holds the device id
 *       and, in the rightmost 21 bits, a transaction counter.
 * IPEK  Initial key, injected into the device. Derived from the BDK and the KSN.
 *
 ******************************************************************************************/

void derive_ipek(const byte *bdk, const byte *ksn, byte *ipek)
{
	byte block[BLOCK_LEN];
	byte maskedBdk[16];

	memcpy(block, ksn, BLOCK_LEN);
	block[7] &= 0xE0;                 // Clear the counter bits in this block

	_TDES_ENCRYPT(ipek, block, bdk, _KEY_LEN_DOUBLE);      // Left half of the IPEK

	xorBytes(bdk, KEY_MASK, maskedBdk, sizeof(maskedBdk));
	_TDES_ENCRYPT(ipek+8, block, maskedBdk, _KEY_LEN_DOUBLE);   // Right half of the IPEK
}

// The X9.24 non-reversible key generation process. ksn_reg is the rightmost 8 KSN bytes.
void dukpt_black_box(const byte *current_sk, const byte *ksn_reg, byte *future_key)
{
	byte modifiedKey[16];
	byte message[BLOCK_LEN];
	byte encrypted[BLOCK_LEN];
	byte right[BLOCK_LEN];
	byte left[BLOCK_LEN];

	// Right half of the new key
	xorBytes(current_sk+8, ksn_reg, message, BLOCK_LEN);
	stdDES(encrypted, message, current_sk);
	xorBytes(encrypted, current_sk+8, right, BLOCK_LEN);

	// Left half of the new key: the same steps, with a masked copy of the key
	xorBytes(current_sk, KEY_MASK, modifiedKey, sizeof(modifiedKey));
	xorBytes(modifiedKey+8, ksn_reg, message, BLOCK_LEN);
	stdDES(encrypted, message, modifiedKey);
	xorBytes(encrypted, modifiedKey+8, left, BLOCK_LEN);

	memcpy(future_key, left, BLOCK_LEN);
	memcpy(future_key+8, right, BLOCK_LEN);
}

// Walks the counter's set bits from the highest down, applying the black box once for each
void derive_key(const byte *ipek, const byte *ksn, byte *derivedkey)
{
	byte ksn_reg[BLOCK_LEN];
	card counter = ((card)(ksn[7] & 0x1F) << 16) | ((card)ksn[8] << 8) | ksn[9];
	card shift;

	memcpy(ksn_reg, ksn+2, BLOCK_LEN);
	ksn_reg[5] &= 0xE0;               // Clear the 21 counter bits
	ksn_reg[6] = 0;
	ksn_reg[7] = 0;

	memcpy(derivedkey, ipek, 16);

	for ( shift = 0x100000; shift > 0; shift >>= 1 )
	{
		if ( counter & shift )
		{
			ksn_reg[5] |= (byte)(shift >> 16);
			ksn_reg[6] |= (byte)(shift >> 8);
			ksn_reg[7] |= (byte)shift;

			dukpt_black_box(derivedkey, ksn_reg, derivedkey);
		}
	}
}

void derive_pin_key(const byte *ipek, const byte *ksn, byte *pinKey)
{
	byte key[16];

	derive_key(ipek, ksn, key);
	xorBytes(key, PEK_MASK, pinKey, sizeof(key));
}


/************************************ Tests ************************************************/

static int failures = 0;

static void check(const char *name, const byte *actual, const char *expectedHex, int len)
{
	byte expected[64];
	int ok = (hex2bin(expected, expectedHex, len) == len) && (memcmp(actual, expected, len) == 0);

	printHexDump(name, actual, len);

	if ( !ok )
	{
		printf("    FAIL : expected %s\n", expectedHex);
		failures++;
	}
}

int main(void)
{
	// The published X9.24 test values. These keys are public, and are never used for real.
	const char *PAN = "4012345678909";
	const char *PIN = "1234";
	const struct { byte counter; const char *encryptedPinBlock; } vectors[] = {
		{1, "1B9C1845EB993A7A"},
		{2, "10A01C8D02C69107"},
		{3, "18DC07B94797B466"},
	};

	byte bdk[16], ksn[KSN_LEN], ipek[16], kcv[3];
	byte pinBlock[BLOCK_LEN];
	byte plain[16 + BLOCK_LEN], cipher[sizeof(plain)], decrypted[sizeof(plain)];
	unsigned i;

	hex2bin(bdk, "0123456789ABCDEFFEDCBA9876543210", sizeof(bdk));
	hex2bin(ksn, "FFFF9876543210E00000", sizeof(ksn));

	derive_ipek(bdk, ksn, ipek);
	check("IPEK                 ", ipek, "6AC292FAA1315B4D858AB3A3D7D5933A", sizeof(ipek));

	if ( getPinBlock(PAN, PIN, pinBlock) != _SUCCESS )
	{
		printf("FAIL : getPinBlock\n");
		failures++;
	}

	for ( i = 0; i < sizeof(vectors)/sizeof(vectors[0]); i++ )
	{
		byte pinKey[16], encrypted[BLOCK_LEN];

		ksn[9] = vectors[i].counter;
		derive_pin_key(ipek, ksn, pinKey);
		SEC_ECBCipher(pinBlock, BLOCK_LEN, encrypted, pinKey, sizeof(pinKey), true);

		printf("KSN counter %d\n", vectors[i].counter);
		check("Encrypted PIN block  ", encrypted, vectors[i].encryptedPinBlock, BLOCK_LEN);
	}

	// Round trips
	memcpy(plain, "A 24-byte test message.!", sizeof(plain));

	SEC_ECBCipher(plain, sizeof(plain), cipher, bdk, sizeof(bdk), true);
	SEC_ECBCipher(cipher, sizeof(cipher), decrypted, bdk, sizeof(bdk), false);
	if ( memcmp(plain, decrypted, sizeof(plain)) != 0 ) { printf("FAIL : ECB round trip\n"); failures++; }

	SEC_CBCCipher(plain, sizeof(plain), cipher, bdk, sizeof(bdk), true);
	SEC_CBCCipher(cipher, sizeof(cipher), decrypted, bdk, sizeof(bdk), false);
	if ( memcmp(plain, decrypted, sizeof(plain)) != 0 ) { printf("FAIL : CBC round trip\n"); failures++; }

	if ( SEC_ECBCipher(plain, 7, cipher, bdk, sizeof(bdk), true) != _FAIL ) { printf("FAIL : partial block accepted\n"); failures++; }

	getCheckValue(bdk, kcv);
	printHexDump("BDK check value      ", kcv, sizeof(kcv));

	printf("%s\n", failures ? "TESTS FAILED" : "All tests passed");

	return failures ? 1 : 0;
}
