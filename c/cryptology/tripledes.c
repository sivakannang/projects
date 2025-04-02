// TripleDes.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

typedef unsigned long card;
//#ifndef byte
typedef unsigned char byte;
//#endif

#define true 1
#define false 0

void BCDn2str(char *data, char *bcd, int BCDlen, int datalen,int numdigits);

void _TDES_ENCRYPT(byte *dst, byte *src, byte *key, int keyingOption);
void _TDES_DECRYPT(byte *dst, byte *src, byte *key, int keyingOption);

static void justdes2(byte * dst, const byte * src, const byte * key, int ctl);
void stdDES(byte * dst, byte * src, byte * key);
void stdSED(byte * dst, byte * src, byte * key);

int File_Read(const char *filename, char *data);
int File_Write(const char *filename, const char *data);

void printHexDump(char *title, unsigned char *str, int len);
int TDES_ENCRYPT_TRACK(byte * CIPHERTRACK, const byte * TRACK);

int padChar(char *src, int len, const char ch);
int hex2bin(byte * bin, const char *hex, int len) ;
static byte chr2nib(char chr);

void xor(const byte * b1, const byte * b2, byte * dst, int len);
void AND(const byte * b1, const byte * b2, byte * dst, int len);
void OR(const byte * b1, const byte * b2, byte * dst, int len);
int SEC_ECBCipher(byte *src, int srcLen, byte *dst, byte *key, byte keyLen, byte isCipher);
int SEC_CBCCipher(byte *src, int srcLen, byte *dst, byte *key, byte keyLen, byte isCipher);

//#define oddParityHex "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"

byte KEY_MASK[] = "\xC0\xC0\xC0\xC0\x00\x00\x00\x00\xC0\xC0\xC0\xC0\x00\x00\x00\x00";
byte PEK_MASK[] = "\x00\x00\x00\x00\x00\x00\x00\xFF\x00\x00\x00\x00\x00\x00\x00\xFF";
byte DEK_MASK[] = "\x00\x00\x00\x00\x00\xFF\x00\x00\x00\x00\x00\x00\x00\xFF\x00\x00";
byte REG8_MASK[]= "\xFF\xFF\xFF\xFF\xFF\xE0\x00\x00";
byte REG3_MASK[]       = "\x1F\xFF\xFF";
byte SHIFT_REG_MASK[]  = "\x10\x00\x00";
unsigned long REG3_MASK_N = 0x1FFFFF;
unsigned long SHIFT_REG_MASK_N = 0x100000;


#define _SUCCESS 0
#define _FAIL -1

#define _KEY_LEN_TRIPLE 3
#define _KEY_LEN_DOUBLE 2
#define _KEY_LEN_SINGLE 1

#define KEY "____testkeys____"
#define DATA "0123456789012345678901234567890123456789"

char * _STR_CHAR_REPLACE(char *s, int oc, int nc);
char * _STR_CHAR_REMOVE(char *s, int c);

short getPinBlock(char *pan, char *pin, byte *key, byte * pinBlock);


void siva();
void test();
void chk();
void getClearKey(byte *comp1, byte *comp2, byte *key);
void getCipherKey(byte *clrKey, byte *key, byte *cipherKey);
void getCheckSum(char *title, byte *key, byte *chkSum);

char *tFgets(FILE *fp, char *d, int len);

void keygen(byte *key, byte *ksn, byte *newkey);
void dukpt_black_box(byte *current_sk, byte *ksn_mod, byte *future_key);
void derive_key(byte *ipek, byte *ksn, byte *derivedkey);
void getTSN(byte *TSN,  int count);
int getCounter(byte *TSN);

short getPinBlock(char *pan, char *pin, byte *key, byte * pinBlock)
{
	byte i = 0;
	char ipPan[17] = {0};
	char ipPin[17] = {0};
	byte plainBinaryPan[17] = {0};
	byte plainBinaryPin[17] = {0};
	byte plainBinaryPinBlock[17] = {0};
	byte cipherPinBlock[9] = {0};

	printf("\nPan: %s", pan);
	printf("\nPin: %s", pin);

	strncpy(ipPan, &pan[strlen(pan)-13], 12);

	sprintf(ipPin, "%02d%s", (int)strlen(pin), pin);
	padChar(ipPin, 16, 'F');

	printf("\nPan: %s", ipPan);
	printf("\nPin: %s", ipPin);

	hex2bin(&plainBinaryPan[2] , ipPan, 6) ;
	hex2bin(plainBinaryPin , ipPin, 8) ;

	printHexDump("Binary Pan", plainBinaryPan, 8);
	printHexDump("Binary Pin", plainBinaryPin, 8);

	i = 0;
	while( i < 8) 
		plainBinaryPinBlock[i++] = plainBinaryPin[i] ^ plainBinaryPan[i];

	printHexDump("Plain Pin Block", plainBinaryPinBlock, 8);

	memcpy(pinBlock, plainBinaryPinBlock, 8);

   //_TDES_ENCRYPT(cipherPinBlock, plainBinaryPinBlock,  key, _KEY_LEN_DOUBLE);

	//printHexDump("Cipher  Pin Block", cipherPinBlock, 8);

	printf("\n\n");
		
	return 0;



}

int main(int argc, char* argv[])
{

	{
		siva();
		return 0;
	}

	byte cipher[41] = {'\0'};
	byte plain[41] = {0};
	int ret = 0, i = 0;

	{
		//getPinBlock("1000001234567890120", "1234", "\x67\x8F\xA8\xB6\xDA\x5E\x7C\x54\xA1\xD9\x16\x49\x62\xA4\xD6\x68");
		
	//	getPinBlock("1000001234567890120", "614444", (byte *)"6666666666666666");

		return 0;
	}

	{

		byte key[] = "6666666666666666";

		printf("\nData : %s", plain);
		printf("\nKey  : %s\n", key);
		i = 0;

		while ( i<8){
			_TDES_ENCRYPT(cipher+i, (byte *)(plain+i),  (byte *)key, _KEY_LEN_DOUBLE);
			i += 8;
		}

		i = 0;

		printf("\n\nClr Key 1 Checksum\n");

		while ( i<16){
			printf("%02X ", cipher[i++]);
		}

		return 0;
		
	}

	
	{

		byte key[] = "\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26";

		printf("\nData : %s", plain);
		printf("\nKey  : %s\n", key);
		i = 0;

		while ( i<8){
			_TDES_ENCRYPT(cipher+i, (byte *)(plain+i),  (byte *)key, _KEY_LEN_DOUBLE);
			i += 8;
		}

		i = 0;

		printf("\n\nClr Key 2 Checksum\n");

		while ( i<16){
			printf("%02X ", cipher[i++]);
		}
		
	}


	{

		i = 0;
		byte component1[] = "\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11\x11";
		byte component2[] = "\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26\x26";
		byte TMK[17] = {0};

		while( i < 16) {
			TMK[i] = (component1[i]) ^ (component2[i]);
			i++;
		}

		i = 0;

		printf("\nClear Key");
		while( i < 16) {
			printf("%02X ", TMK[i]);
			i++;
		}

		printf("\nData : %s", plain);
		printf("\nKey  : %s\n", TMK);
		i = 0;

		while ( i<8){
			_TDES_ENCRYPT(cipher+i, (byte *)(plain+i),  (byte *)TMK, _KEY_LEN_DOUBLE);
			i += 8;
		}

		i = 0;

		printf("\n\nClr Key Checksum\n");

		while ( i<16){
			printf("%02X ", cipher[i++]);
		}
		//printf("\n\n51 CA A0\n");

		return 0;
	}

	

	while ( i<40){
		_TDES_ENCRYPT(cipher+i, (byte *)(DATA+i),  (byte *)KEY, _KEY_LEN_DOUBLE);
		i += 8;
	}
	ret = File_Write("CIPHER.TXT", (char *)cipher);


	memset(cipher, '\0', sizeof(cipher));
	ret = File_Read("CIPHER.TXT", (char *)cipher);
	printf("\n Number of characters read : %d", ret);

	i = 0;
	while ( i< ret){
		_TDES_DECRYPT(plain+i, cipher+i, (byte *)KEY, _KEY_LEN_DOUBLE);
		i += 8;
	}

	

	printf("\nop: %s\n", plain);

	i = 0;
	while ( i < 40){
		int c = plain[i++];
		printf("%c %03d %02X\n", c, c, c);
	}
	
	return 0;
}

void _TDES_ENCRYPT(byte *dst, byte *src, byte *key, int keyingOption)
{
	byte tmp[8];
	
	switch (keyingOption)
	{
	case _KEY_LEN_TRIPLE: stdDES(dst, src, key), stdSED(tmp, dst, key+8), stdDES(dst, tmp, key+16); break;
	case _KEY_LEN_DOUBLE: stdDES(dst, src, key), stdSED(tmp, dst, key+8), stdDES(dst, tmp, key); break;
	case _KEY_LEN_SINGLE: default: stdDES(dst, src, key), stdSED(tmp, dst, key), stdDES(dst, tmp, key); break;
	}
}


void _TDES_DECRYPT(byte *dst, byte *src, byte *key, int keyingOption)
{
	byte tmp[8];

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
        l = l << 1 | (x & 1 << 0) | (x & 1 << 2) << 6 | (x & 1 << 4) << 12 | (x
                                                                              &
                                                                              1
                                                                              <<
                                                                              6)
            << 18;
        r = r << 1 | (x & 1 << 1) >> 1 | (x & 1 << 3) << 5 | (x & 1 << 5) << 11
            | (x & 1 << 7) << 17;
        x = key[i];
        c = c << 1 | (x & 1 << 7) << 17 | (x & 1 << 6) << 10 | (x & 1 << 5) << 3
            | (x & 1 << 4) >> 4;
        d = d << 1 | (x & 1 << 1) << 19 | (x & 1 << 2) << 10 | (x & 1 << 3) <<
            1;
    } while(--i >= 0);
    d |= c & 0x0F;
    c >>= 4;
    i = 24;
/* first round is special: one left shift on encipher, no shift en decipher */
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

/** Encrypt the binary array src using the key and put the result into the buffer dst.
 * The standard DES algorithm is used for encryption.
 * \param dst (O) Destination binary buffer.
 * \param src (I) Binary buffer to be encrypted.
 * \param key (I) DES encryption key.
 * \pre 
 *    - dst!=0
 *    - src!=0
 *    - key!=0
 * \remark all the buffer are 8 bytes long.
 * \return no
 * \sa Header: sys\\sys.h
 * \sa Source: sys\\std.c
 * \test tcab0057.c
 */
void stdDES(byte * dst, byte * src, byte * key) {
    justdes2(dst, src, key, 0);
}

/** Decrypt the binary array src using the key key and put the result into the buffer dst.
 * The standard DES algorithm is used for encryption.
 * \param dst (O) Destination binary buffer.
 * \param       src (I) Binary buffer to be encrypted.
 * \param       key (I) DES decryption key.
 * \pre 
 *    - dst!=0
 *    - src!=0
 *    - key!=0
 * \remark all the buffer are 8 bytes long.
 * \return no
 * \sa Header: sys\\sys.h
 * \sa Source: sys\\std.c
 * \test tcab0057.c
 */
void stdSED(byte * dst, byte * src, byte * key) {
    justdes2(dst, src, key, 1);
}


void BCDn2str(char *data, char *bcd, int BCDlen, int datalen,int numdigits)
{
	// datalen >= (numdigits)
	int i;
	memset(data,0x30,datalen);
	for(i = 0;i<numdigits;i++)
	{
		if((i%2) == 0)
		{
			data[datalen - i - 1] =  (((bcd[BCDlen - (i/2) - 1]) & 0x0F) + '0');
		}
		else
		{
			data[datalen - i - 1] =  (((bcd[BCDlen - (i/2) - 1] >> 4) & 0x0F) + '0');
		}
	}
}

int File_Write(const char *filename, const char *data)
{
	FILE *fp = NULL;

	fp = fopen(filename, "wb");
	if ( fp == NULL)
		return _FAIL;

	int ret = fwrite(data, 1, 40, fp);
	fclose(fp);

	return ret;

}



int File_Read(const char *filename, char *data)
{
	FILE *fp = NULL;

	fp = fopen(filename, "rb");
	if ( fp == NULL)
		return _FAIL;

	int ret = fread(data, 1, 101, fp);

	fclose(fp);

	return ret;
}

int TDES_ENCRYPT_TRACK(byte * CIPHERTRACK, const byte * TRACK)
{

	// Input  : TRACK2
	// Output : CIPHER_TRACK

	// Step 1 : _CIPHER_TEMP  = 3DES_ENCRYPT(TRACK2, NDK)
	// Step 2 : _CIPHER_TRACK = 3DES_ENCRYPT(_CIPHER_TEMP, IMK)

	byte TSK[33] = {'\0'};
	byte IMK[33] = {'\0'};
	byte CIPHERTEMP[65] = {'\0'};
	int ret = 0;
	int len = 0;
	int loc = 0;


	//ret = getTSK(TSK);
	strcpy((char *)TSK, "3211381567091368");
	if ( ret != _SUCCESS){
		return ret;
	}

	strcpy((char *)IMK, "KGFF0243NBSE9999");
	//ret = getIMK(IMK);
	if ( ret != _SUCCESS){
		return ret;
	}

	len = strlen((char *)TRACK);

	loc = 0;
	while( loc < len )
	{
		_TDES_ENCRYPT(CIPHERTEMP+loc, (byte *)TRACK+loc, TSK, _KEY_LEN_DOUBLE);
		loc += 8;
	}

	loc = 0;
	while( loc < len )
	{
		_TDES_ENCRYPT(CIPHERTRACK+loc, CIPHERTEMP+loc, IMK, _KEY_LEN_DOUBLE);
		loc += 8;
	}

	return _SUCCESS;

}

void printHexDump(char *title, unsigned char *str, int len)
{
	char hexData[1001] = {'\0'};

	int offset = 0;
	int sLen = 0;

	//printf("\n%s", title);
	sprintf(hexData, "%s : ", title);

	while(len--){
		sprintf(&hexData[strlen(hexData)], "%02X ", str[offset++] );
	}

	sLen = strlen(hexData);
	hexData[sLen] = 0;
	offset = 0;

	
	printf("\n%s", hexData);


}


int padChar(char *src, int len, const char ch)
{
	int dim;                    //the initial string length
	char *ptr;                  //points to the start of padding
	char dir;                   //pad direction (left or right)

	dir = 'r';
	if(len < 0) {               //left pad?
		dir = 'l';
		len = -len;
	}
	src[len] = 0;

	dim = strlen(src);
	if(dim >= len)
		return dim;             //string is long enough - nothing to pad

	ptr = src;                  //ptr will pont to the start of padding
	if(dir == 'l')
		memmove(src + len - dim, src, dim); //left padding: move characters to the right
	else
		ptr += dim;             //right padding: move the padding pointer to the end of the string

	memset(ptr, ch, len - dim);    //now pad dim characters chr starting from ptr

	return strlen(src);

}

int hex2bin(byte * bin, const char *hex, int len) {
    int ret;                    //to be returned: number of characters processed
    byte tmp;

    //VERIFY(bin);
    //VERIFY(hex);

    //if non-zero, len is the length of acceptor buffer bin
    if(!len) {                  //calcualte length if missing
        len = strlen(hex);
        //CHECK(len % 2 == 0, lblKO); //there should be 2 source characters for each output byte
        //len /= 2;
    }
    ret = 0;
    while(len--) {
        tmp = chr2nib(*hex++);  //get first nibble
        if(tmp >= 0x10)
            break;
        *bin = (byte) (tmp << 4);

        tmp = chr2nib(*hex++);  //get second nibble
		if ( tmp < 0x10 )
			*bin |= tmp;
        //CHECK(tmp < 0x10, lblKO);
        

        bin++;
        ret++;
    }
    return ret;
  lblKO:
    return 0;
}

static byte chr2nib(char chr) { //convert hexadecimal character to nibble
    switch (chr) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
          return (byte) (chr - '0');
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
          return (byte) (chr - 'A' + 10);
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
          return (byte) (chr - 'a' + 10);
      default:
          break;
    }
    return 0x10;                //KO
}

char * _STR_CHAR_REMOVE(char *s, int c)
{

	char *p = s;
	char *d = s;

	while(*d = *s++)
		(*d == c) ? d : d++;

	return p;

}

char * _STR_CHAR_REPLACE(char *s, int oc, int nc)
{
	char *p = s;

	while(*s)
		(*s == oc) ? (*s++ = nc) : *s++;

	return p;

}


int SEC_ECBCipher(byte *src, int srcLen, byte *dst, byte *key, byte keyLen, byte isCipher)
{

	int i = 0;
	int keyingOption = (keyLen == 16) ? _KEY_LEN_DOUBLE : ((keyLen == 24) ? _KEY_LEN_TRIPLE : _KEY_LEN_SINGLE );
	
	while ( i < srcLen )
		isCipher ? _TDES_ENCRYPT(dst+i, src+i, key, keyingOption) :  _TDES_DECRYPT(dst+i, src+i, key, keyingOption), i += 8 ;

	return i;

}

int SEC_CBCCipher(byte *src, int srcLen, byte *dst, byte *key, byte keyLen, byte isCipher)
{

	int i = 0;
	int keyingOption = (keyLen == 16) ? _KEY_LEN_DOUBLE : ((keyLen == 24) ? _KEY_LEN_TRIPLE : _KEY_LEN_SINGLE );
	byte result[8] = {0};
	byte source[8] = {0};
	byte nullData[8] = {0};

	if ( isCipher ) {
	
		while ( i < srcLen ) {

			xor(src+i, result, source, 8);
			_TDES_ENCRYPT(dst+i, source, key, keyingOption);
			memcpy(result, dst+i, 8);
       		i += 8 ;
		}

	} else {

		int offset = srcLen - 8;

		while ( offset >= 0){

			_TDES_DECRYPT(result, src+offset, key, keyingOption);
			offset > 0 ? xor(result, src+offset-8, dst+offset, 8) : xor(result, nullData, dst+offset, 8);
       		offset -= 8 ;
		}
	}

	return i;

}

void xor(const byte * b1, const byte * b2, byte * dst, int len) {
   
    while(len--)
        *dst++ = *b1++ ^ *b2++;
}


void AND(const byte * b1, const byte * b2, byte * dst, int len) {
   
    while(len--)
        *dst++ = *b1++ & *b2++;
}

void OR(const byte * b1, const byte * b2, byte * dst, int len) {
   
    while(len--)
        *dst++ = *b1++ | *b2++;
}

void getClearKey(byte *comp1, byte *comp2, byte *key)
{
	byte len = 0;

	printHexDump("Component 1", comp1, 16);
	printHexDump("Component 2", comp2, 16);

	while(len < 16)
		key[len++] = comp1[len] ^ comp2[len];

}

void getCipherKey(byte *clrKey, byte *key, byte *cipherKey)
{
	_TDES_ENCRYPT(cipherKey+0, clrKey+0, key, _KEY_LEN_DOUBLE);
	_TDES_ENCRYPT(cipherKey+8, clrKey+8, key, _KEY_LEN_DOUBLE);

}

void getCheckSum(char *title, byte *key, byte *chkSum)
{
	byte src[9] = {0}; 
	_TDES_ENCRYPT(chkSum+0, src+0, key, _KEY_LEN_DOUBLE);
	printf("\n%s : %02X %02X %02X", title,  chkSum[0], chkSum[1], chkSum[2]); 
}


void siva()
{

	byte chkSum[17] = {0};
	byte IPEK[17] = {0};

	byte BDK[] = "\x01\x23\x45\x67\x89\xAB\xCD\xEF\xFE\xDC\xBA\x98\x76\x54\x32\x10";
	byte KSN[17] = "\xFF\xFF\x98\x76\x54\x32\x10\xE0\x00\x00";   // KSN = combination of device serial number and transaction counter
	
	byte BDK_TEMP[17] = {0};
	byte future_key[10][17] = {0};

	// Step 1 : Generate left part of IPEK
	SEC_ECBCipher(KSN, 8, IPEK, BDK, 16, true);
	

	// Step 2 : Generate right part of IPEK
	xor(BDK, KEY_MASK, BDK_TEMP, 16);
	SEC_ECBCipher(KSN, 8, IPEK+8, BDK_TEMP, 16, true);
	
	printHexDump("BDK  ", BDK, 16);
	printHexDump("BDKT ", BDK_TEMP, 16);
	printHexDump("KSN  ", KSN, 10);
	printHexDump("IPEK ", IPEK, 16);
	printf("\n------------------");
	
	KSN[9] = 2;
	dukpt_black_box(IPEK, KSN, future_key[0]);
	printHexDump("KSN  ", KSN, 10);
	printHexDump("FK   ", future_key[0], 16);
	printf("\n------------------");
	
	int i= 0;
	int counter = 0;
	for ( i = 1; i<8; i++)
	{
		memset(KSN, 0, sizeof(KSN));
		getTSN(KSN, i+2);
		counter = getCounter(KSN);
		printf("\nCOunter : %d", counter);
		
		if ( counter  == 1)
		{
			//dukpt_black_box(future_key[i-1], KSN, future_key[i]);
			dukpt_black_box(IPEK, KSN, future_key[i]);
		}
		else
		{
			while(counter-- > 0)
				dukpt_black_box(future_key[i-1], KSN, future_key[i]);
		}
		//keygen(future_key[i-1], KSN, future_key[i]);
		//derive_key(future_key[i-1], KSN, future_key[i]);
		//printHexDump("KSN  ", KSN, 10);
		printHexDump("FK   ", future_key[i], 16);
		printf("\n------------------");
	}
	
	//derive_key(IPEK, KSN, future_key[5]);
	//printHexDump("KSN  ", KSN, 10);
	//printHexDump("FK   ", future_key[5], 16);
	//printf("\n------------------");
	
	
	
}

void getTSN(byte *TSN,  int count)
{
	byte binCount[4] = {0};
	char hexstr[7] = {0};
	sprintf(hexstr, "%06d", count);
	TSN[0] = 0xFF, TSN[1] = 0xFF, TSN[2] = 0x98; TSN[3] = 0x76, TSN[4] = 0x54, TSN[5] = 0x32, TSN[6] = 0x10, TSN[7] = 0xE0;
	hex2bin(binCount, hexstr, 3);
	TSN[7] = TSN[7] | binCount[0], TSN[8] = binCount[1], TSN[9] = binCount[2];
	printHexDump("TSN  ", TSN, 10);

}

int getCounter(byte *TSN)
{
	int count = 0;
	byte first = TSN[8];
	byte second = TSN[9];

	while(first)
	{
		if ( first & 1)
			count++;
		first>>=1;
	}

	while(second)
	{
		if ( second & 1)
			count++;
		second>>=1;

	}

	return count;
	
}

	
void dukpt_black_box(byte *current_sk, byte *ksn_mod, byte *future_key)
{
	byte current_sk_msb[9] = {0};
	byte current_sk_lsb[9] = {0};
	byte ksn_mod_lsb[9] = {0};
	byte message[9] = {0};
	byte temp[9] = {0};
	byte future_key_msb[9] = {0};
	byte future_key_lsb[9] = {0};
	byte current_sk_mod[17] = {0};
	
	
	// Left Key
	memcpy(current_sk_msb, current_sk+0, 8);
	memcpy(current_sk_lsb, current_sk+8, 8);
	memcpy(ksn_mod_lsb, ksn_mod+2, 8);
	//printHexDump("SKM  ", current_sk_msb, 8);
	//printHexDump("SKL  ", current_sk_lsb, 8);
	//printHexDump("KSNL ", ksn_mod_lsb, 8);
	xor(current_sk_lsb, ksn_mod_lsb, message, 8);
	//printHexDump("MSG  ", message, 8);
	//stdDES(dst, src, key)
	stdDES(temp, message, current_sk_msb);
	//printHexDump("DES  ", temp, 8);
	xor(temp, current_sk_lsb, future_key_lsb, 8);
	//printHexDump("FK R ", future_key_lsb, 8);
	
	// Right Key
	xor(current_sk, KEY_MASK, current_sk_mod, 16);
	memcpy(current_sk_msb, current_sk_mod+0, 8);
	memcpy(current_sk_lsb, current_sk_mod+8, 8);
	memcpy(ksn_mod_lsb, ksn_mod+2, 8);
	//printHexDump("SKM  ", current_sk_msb, 8);
	//printHexDump("SKL  ", current_sk_lsb, 8);
	//printHexDump("KSNL ", ksn_mod_lsb, 8);
	xor(current_sk_lsb, ksn_mod_lsb, message, 8);
	//printHexDump("MSG  ", message, 8);
	//stdDES(dst, src, key)
	stdDES(temp, message, current_sk_msb);
	//printHexDump("DES  ", temp, 8);
	xor(temp, current_sk_lsb, future_key_msb, 8);
	//printHexDump("FK L ", future_key_msb, 8);
	
	memcpy(future_key+0, future_key_msb, 8);
	memcpy(future_key+8, future_key_lsb, 8);
	//xor(temp, PEK_MASK, future_key, 16);
}

void derive_key(byte *ipek, byte *ksn, byte *derivedkey)
{
	
    //ksn_current = ksn.to_i(16)
	byte *ksn_current = ksn+2;
	byte ksn_reg[9] = {0};
	byte temp[17] = {0};
	byte reg_3[4] = {0};
	byte curkey[17] = {0};
	unsigned long shift_reg = 0;
      
    //# Get 8 least significant bytes
    //ksn_reg = ksn_current & LS16_MASK
	memcpy(ksn_reg, ksn_current, 8);

    //# Clear the 21 counter bits
    //ksn_reg = ksn_reg & REG8_MASK
	AND(ksn_reg, REG8_MASK, temp, 8);
	memcpy(ksn_reg, temp, 8);
      
    //# Grab the 21 counter bits
    //reg_3 = ksn_current & REG3_MASK
    //shift_reg = SHIFT_REG_MASK
	AND(ksn_current+5, REG3_MASK, reg_3, 3);
	shift_reg = SHIFT_REG_MASK_N;
      
    /*#Initialize "curkey" to be the derived "ipek"
      curkey = ipek.to_i(16)
      while (shift_reg > 0)
      	if shift_reg & reg_3 > 0
      	  ksn_reg = shift_reg | ksn_reg          
          curkey = keygen(curkey, ksn_reg)
      	end
      	shift_reg = shift_reg >> 1
      end
      hex_string_from_val(curkey, 16)
    end*/
	
	memcpy(curkey, ipek, 16);
	while( shift_reg > 0 )
	{
		if ( shift_reg & atoi(reg_3) > 0 )
		{
			byte temp1[9] = {0};
			byte temp2[9] = {0};
			memcpy(temp1+4, (byte *)&shift_reg, 4);
			memcpy(temp2, ksn_reg, 8);
			OR(temp1, temp2, ksn_reg, 8);
			memcpy(temp, curkey, 16);
			keygen(temp, ksn_reg, curkey);
		}
		shift_reg = shift_reg >> 1;
	}
	
	memcpy(derivedkey, curkey, 16);
}	
void keygen(byte *key, byte *ksn, byte *newkey)
{
	byte cr1[9] = {0};
	byte cr2[9] = {0};
	byte cr3[9] = {0};
	byte key2[17] = {0};
	
	memcpy(cr1, ksn+2, 8);
	stdDES(cr2, cr1, key);
	xor(key, KEY_MASK, key2, 16);
	stdDES(cr3, cr1, key2);
	memcpy(newkey+0, cr3, 8);
	memcpy(newkey+8, cr2, 8);
}
void test()
{

	FILE *fp = NULL;
	short ret = 0;
	byte mfkPlain[17] = {0};
	byte plainK1E[17] = {0};
	byte plainK2E[17] = {0};
	byte plainK3E[17] = {0};
	byte plainK4E[17] = {0};
	byte plainK5E[17] = {0};
	byte plainK6E[17] = {0};
	byte cipherK1E[17] = {0};
	byte cipherK2E[17] = {0};
	byte cipherK3E[17] = {0};
	byte cipherK4E[17] = {0};
	byte cipherK5E[17] = {0};
	byte cipherK6E[17] = {0};
	byte plainK1O[17] = {0};
	byte plainK2O[17] = {0};
	byte plainK3O[17] = {0};
	byte plainK4O[17] = {0};
	byte plainK5O[17] = {0};
	byte plainK6O[17] = {0};
	byte cipherK1O[17] = {0};
	byte cipherK2O[17] = {0};
	byte cipherK3O[17] = {0};
	byte cipherK4O[17] = {0};
	byte cipherK5O[17] = {0};
	byte cipherK6O[17] = {0};
	byte checkSum[17] = {0};
	char component1[101] = {0};
	char component2[101] = {0};

	byte plainTrack[101] = {0};
	byte plainPin[101] = {0};
	byte cipherTrack[101] = {0};
	byte plainPinBlock[101] = {0};
	byte cipherPinBlock[101] = {0};
	byte pan[51] = {0};
	byte tmpMFK[101] = {0};
	byte parity[101] = {0};
	byte oddParityHex[101] ={0};
	

	
	fp = fopen("key.txt", "r");
	if ( fp == NULL){
		printf("\nFile not found");
		system("pause");
		return;
	}

	

	printf("\n\n\n\n%s", fgets((char *)parity, 100, fp));
	memset(parity, 0, sizeof(parity));
	tFgets(fp, (char *)parity, 100);

	printf("\n\n\n\n%s", fgets((char *)oddParityHex, 100, fp));
	memset(oddParityHex, 0, sizeof(oddParityHex));
	tFgets(fp, (char *)oddParityHex, 100);


	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	memset(checkSum, 0, sizeof(checkSum));
	memset(component1, 0, sizeof(component1));
	memset(component2, 0, sizeof(component2));
	getClearKey((byte *)tFgets(fp, component1, 100), (byte *)tFgets(fp, component2, 100), tmpMFK);
	getClearKey(tmpMFK, parity, mfkPlain);
	printHexDump("Clear Key  ", mfkPlain, 16);
	getCheckSum("Check Sum  ", mfkPlain, checkSum);


	
	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	memset(checkSum, 0, sizeof(checkSum));
	memset(component1, 0, sizeof(component1));
	memset(component2, 0, sizeof(component2));
	getClearKey((byte *)tFgets(fp, component1, 100), (byte *)tFgets(fp, component2, 100), plainK1E);
	printf("\nEven Parity");
	printHexDump("Clear Key  ", plainK1E, 16);
	getCheckSum("Check Sum  ", plainK1E, checkSum);
	SEC_ECBCipher(plainK1E, 16, cipherK1E, mfkPlain, 16, true);
	printHexDump("Cipher Key ", cipherK1E, 16);
	memset(checkSum, 0, sizeof(checkSum));
	printf("\nOdd Parity");
	getClearKey(plainK1E, (byte *)oddParityHex, plainK1O);
	printHexDump("Clear Key  ", plainK1O, 16);
	getCheckSum("Check Sum  ", plainK1O, checkSum);
	SEC_ECBCipher(plainK1O, 16, cipherK1O, mfkPlain, 16, true);
	printHexDump("Cipher Key ", cipherK1O, 16);




	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	memset(checkSum, 0, sizeof(checkSum));
	memset(component1, 0, sizeof(component1));
	memset(component2, 0, sizeof(component2));
	getClearKey((byte *)tFgets(fp, component1, 100), (byte *)tFgets(fp, component2, 100), plainK2E);
	printf("\nEven Parity");
	printHexDump("Clear Key  ", plainK2E, 16);
	getCheckSum("Check Sum  ", plainK2E, checkSum);
	SEC_ECBCipher(plainK2E, 16, cipherK2E, mfkPlain, 16, true);
	printHexDump("Cipher Key ", cipherK2E, 16);
	memset(checkSum, 0, sizeof(checkSum));
	printf("\nOdd Parity");
	getClearKey(plainK2E, (byte *)oddParityHex, plainK2O);
	printHexDump("Clear Key  ", plainK2O, 16);
	getCheckSum("Check Sum  ", plainK2O, checkSum);
	SEC_ECBCipher(plainK2O, 16, cipherK2O, mfkPlain, 16, true);
	printHexDump("Cipher Key ", cipherK2O, 16);

	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	memset(checkSum, 0, sizeof(checkSum));
	memset(component1, 0, sizeof(component1));
	memset(component2, 0, sizeof(component2));
	getClearKey((byte *)tFgets(fp, component1, 100), (byte *)tFgets(fp, component2, 100), plainK3E);
	printf("\nEven Parity");
	printHexDump("Clear Key  ", plainK3E, 16);
	getCheckSum("Check Sum  ", plainK3E, checkSum);
	SEC_ECBCipher(plainK3E, 16, cipherK3E, plainK1E, 16, true);
	printHexDump("Cipher Key ", cipherK3E, 16);
	memset(checkSum, 0, sizeof(checkSum));
	printf("\nOdd Parity");
	getClearKey(plainK3E, (byte *)oddParityHex, plainK3O);
	printHexDump("Clear Key  ", plainK3O, 16);
	getCheckSum("Check Sum  ", plainK3O, checkSum);
	SEC_ECBCipher(plainK3O, 16, cipherK3O, plainK1O, 16, true);
	printHexDump("Cipher Key ", cipherK3O, 16);


	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	memset(checkSum, 0, sizeof(checkSum));
	memset(component1, 0, sizeof(component1));
	memset(component2, 0, sizeof(component2));
	getClearKey((byte *)tFgets(fp, component1, 100), (byte *)tFgets(fp, component2, 100), plainK4E);
	printf("\nEven Parity");
	printHexDump("Clear Key  ", plainK4E, 16);
	getCheckSum("Check Sum  ", plainK4E, checkSum);
	SEC_ECBCipher(plainK4E, 16, cipherK4E, plainK1E, 16, true);
	printHexDump("Cipher Key ", cipherK4E, 16);
	memset(checkSum, 0, sizeof(checkSum));
	printf("\nOdd Parity");
	getClearKey(plainK4E, (byte *)oddParityHex, plainK4O);
	printHexDump("Clear Key  ", plainK4O, 16);
	getCheckSum("Check Sum  ", plainK4O, checkSum);
	SEC_ECBCipher(plainK4O, 16, cipherK4O, plainK1O, 16, true);
	printHexDump("Cipher Key ", cipherK4O, 16);


	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	memset(checkSum, 0, sizeof(checkSum));
	memset(component1, 0, sizeof(component1));
	memset(component2, 0, sizeof(component2));
	getClearKey((byte *)tFgets(fp, component1, 100), (byte *)tFgets(fp, component2, 100), plainK5E);
	printf("\nEven Parity");
	printHexDump("Clear Key  ", plainK5E, 16);
	getCheckSum("Check Sum  ", plainK5E, checkSum);
	SEC_ECBCipher(plainK5E, 16, cipherK5E, plainK3E, 16, true);
	printHexDump("Cipher Key ", cipherK5E, 16);
	memset(checkSum, 0, sizeof(checkSum));
	printf("\nOdd Parity");
	getClearKey(plainK5E, (byte *)oddParityHex, plainK5O);
	printHexDump("Clear Key  ", plainK5O, 16);
	getCheckSum("Check Sum  ", plainK5O, checkSum);
	SEC_ECBCipher(plainK5O, 16, cipherK5O, plainK3O, 16, true);
	printHexDump("Cipher Key ", cipherK5O, 16);


	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	memset(checkSum, 0, sizeof(checkSum));
	memset(component1, 0, sizeof(component1));
	memset(component2, 0, sizeof(component2));
	getClearKey((byte *)tFgets(fp, component1, 100), (byte *)tFgets(fp, component2, 100), plainK6E);
	printf("\nEven Parity");
	printHexDump("Clear Key  ", plainK6E, 16);
	getCheckSum("Check Sum  ", plainK6E, checkSum);
	SEC_ECBCipher(plainK6E, 16, cipherK6E, plainK4E, 16, true);
	printHexDump("Cipher Key ", cipherK6E, 16);
	memset(checkSum, 0, sizeof(checkSum));
	printf("\nOdd Parity");
	getClearKey(plainK6E, (byte *)oddParityHex, plainK6O);
	printHexDump("Clear Key  ", plainK6O, 16);
	getCheckSum("Check Sum  ", plainK6O, checkSum);
	SEC_ECBCipher(plainK6O, 16, cipherK6O, plainK4O, 16, true);
	printHexDump("Cipher Key ", cipherK6O, 16);

	memset(component1, 0, sizeof(component1));
	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	fgets((char *)plainTrack, 100, fp);
	plainTrack[strlen((char *)plainTrack)-1] = 0; // Removing char '\n'
	int i = 0;
	while( (plainTrack[i] != '=') && (plainTrack[i] != 'D'))
		pan[i++] = plainTrack[i];
	padChar((char *)plainTrack, -40, '0');
	printf("\nPlain Track : %s", plainTrack);
	printf("\nEven Parity");
	memset(cipherTrack, 0, sizeof(cipherTrack));
	SEC_CBCCipher(plainTrack, 40, cipherTrack, plainK6E, 16, true);
	printHexDump("CipherTrack", cipherTrack, 40);
	printf("\nOdd Parity");
	memset(cipherTrack, 0, sizeof(cipherTrack));
	SEC_CBCCipher(plainTrack, 40, cipherTrack, plainK6O, 16, true);
	printHexDump("CipherTrack", cipherTrack, 40);


	memset(component1, 0, sizeof(component1));
	printf("\n\n\n\n%s", fgets(component1, 100, fp));
	fgets((char *)plainPin, 100, fp);
	plainPin[strlen((char *)plainPin)-1] = 0; // Removing char '\n'
	printf("\nEven Parity");
//	getPinBlock((char *)pan, (char *)plainPin, plainK5E);
	printf("\nOdd Parity");
//	getPinBlock((char *)pan, (char *)plainPin, plainK5O);
	

	
	fclose(fp);

	printf("\n\n");
	system("pause");
	

}


char *tFgets(FILE *fp, char *d, int len)
{
	short i = 0, j = 0;
	unsigned char s[101] = {0};
	char *p = d;

	p = fgets(d, len, fp);
	if ( p == NULL){
		printf("\n END Of File");
		return NULL;
	}

	//while(d[i] != '\n'){
	while((d[i] != '\n') && (j < 16)){
		int x;
		sscanf(&d[i], "%2x", &x);
		s[j++] = x;
        i += 2;
	}

	memcpy(d, (char *)s, j);
	d[j] = 0;

	return p;

}








