// gcc Iso8583Parser.c -o iso8583
//
// Packs and unpacks ISO 8583 messages. Each field's format is one row in fieldTable,
// so pack and unpack share one description of the message and can't disagree.
//
// Packet layout:
//   2 bytes    total packet length in BCD, counting these 2 bytes
//   2 bytes    MTI in BCD
//   8 bytes    primary bitmap. Bit 1 set means 8 more bytes of secondary bitmap follow.
//   ...        the fields present in the bitmap, in order
//
// This layout, including the length that counts itself, matches the host format this
// parser was written for. Check it against your own host's specification.

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "Iso8583Parser.h"

typedef enum
{
	NUM_FIXED,   // BCD, left-padded with '0'. size is the bytes on the wire.
	NUM_LL,      // 1-byte BCD length in digits, then BCD digits. Odd lengths get a leading 0 nibble.
	TEXT_FIXED,  // ASCII, left-padded with '0'. size is the bytes on the wire.
	TEXT_LL,     // 1-byte BCD length, then ASCII
	TEXT_LLLL,   // 2-byte BCD length, then ASCII
	BIN_FIXED    // Raw bytes. size is the bytes on the wire.
}FieldType;

typedef struct
{
	int number;
	FieldType type;
	int size;           // Bytes on the wire, for the fixed types
	size_t offset;      // Where the value is in IsoPkt
	size_t capacity;    // Size of that array, including the NUL
	int isSensitive;    // Card data: never written to the log
}FieldDef;

#define FIELD(n, type, size, sensitive) { n, type, size, offsetof(IsoPkt, D##n), sizeof(((IsoPkt *)0)->D##n), sensitive }

static const FieldDef fieldTable[] =
{
	FIELD( 2, NUM_LL,      0, 1),
	FIELD( 3, NUM_FIXED,   3, 0),
	FIELD( 4, NUM_FIXED,   6, 0),
	FIELD( 5, NUM_FIXED,   6, 0),
	FIELD( 6, NUM_FIXED,   6, 0),
	FIELD( 7, NUM_FIXED,   5, 0),
	FIELD( 8, NUM_FIXED,   4, 0),
	FIELD( 9, NUM_FIXED,   4, 0),
	FIELD(10, NUM_FIXED,   4, 0),
	FIELD(11, NUM_FIXED,   3, 0),
	FIELD(12, NUM_FIXED,   3, 0),
	FIELD(13, NUM_FIXED,   2, 0),
	FIELD(14, NUM_FIXED,   2, 1),
	FIELD(15, NUM_FIXED,   2, 0),
	FIELD(16, NUM_FIXED,   2, 0),
	FIELD(17, NUM_FIXED,   2, 0),
	FIELD(18, NUM_FIXED,   2, 0),
	FIELD(19, NUM_FIXED,   2, 0),
	FIELD(20, NUM_FIXED,   2, 0),
	FIELD(21, NUM_FIXED,   2, 0),
	FIELD(22, NUM_FIXED,   2, 0),
	FIELD(23, NUM_FIXED,   2, 0),
	FIELD(24, NUM_FIXED,   2, 0),
	FIELD(25, NUM_FIXED,   1, 0),
	FIELD(26, NUM_FIXED,   1, 0),
	FIELD(27, NUM_FIXED,   1, 0),
	FIELD(28, NUM_FIXED,   4, 0),
	FIELD(29, NUM_FIXED,   4, 0),
	FIELD(30, NUM_FIXED,   4, 0),
	FIELD(31, NUM_FIXED,   4, 0),
	FIELD(32, NUM_LL,      0, 0),
	FIELD(33, NUM_LL,      0, 0),
	FIELD(34, TEXT_LL,     0, 1),
	FIELD(35, NUM_LL,      0, 1),
	FIELD(36, NUM_LL,      0, 1),
	FIELD(37, TEXT_FIXED, 12, 0),
	FIELD(38, TEXT_FIXED,  6, 0),
	FIELD(39, TEXT_FIXED,  2, 0),
	FIELD(40, TEXT_FIXED,  3, 0),
	FIELD(41, TEXT_FIXED,  8, 0),
	FIELD(42, TEXT_FIXED, 15, 0),
	FIELD(43, TEXT_FIXED, 40, 0),
	FIELD(44, TEXT_LL,     0, 0),
	FIELD(45, TEXT_LL,     0, 1),
	FIELD(46, TEXT_LLLL,   0, 0),
	FIELD(47, TEXT_LLLL,   0, 0),
	FIELD(48, TEXT_LLLL,   0, 0),
	FIELD(49, TEXT_FIXED,  3, 0),
	FIELD(50, TEXT_FIXED,  3, 0),
	FIELD(51, TEXT_FIXED,  3, 0),
	FIELD(52, BIN_FIXED,   8, 1),
	FIELD(53, NUM_FIXED,   8, 0),
	FIELD(54, TEXT_LL,     0, 0),
	FIELD(55, TEXT_LLLL,   0, 0),
	FIELD(56, TEXT_LLLL,   0, 0),
	FIELD(57, TEXT_LLLL,   0, 0),
	FIELD(58, TEXT_LLLL,   0, 0),
	FIELD(59, TEXT_LLLL,   0, 0),
	FIELD(60, TEXT_LLLL,   0, 0),
	FIELD(61, TEXT_LLLL,   0, 0),
	FIELD(62, TEXT_LLLL,   0, 0),
	FIELD(63, TEXT_LLLL,   0, 0),
	FIELD(64, BIN_FIXED,   2, 0),
};

#define FIELD_COUNT ((int)(sizeof(fieldTable) / sizeof(fieldTable[0])))

#define MAX_PKT_LEN 9999   // The largest value 2 BCD bytes can hold


static int File_Log(const char *format, ...) __attribute__((format(printf, 1, 2)));

static int File_Log(const char *format, ...)
{
	char buffer[4096];
	va_list args;
	FILE *fp;

	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	printf("%s\n", buffer);

	fp = fopen("log.txt", "a");
	if ( fp == NULL )
		return -1;

	fprintf(fp, "%s\n", buffer);
	fclose(fp);

	return 0;
}


/* Field number 1 is the leftmost bit of the first bitmap byte */
static void setBit(unsigned char *bitmap, int field)
{
	bitmap[(field - 1) / 8] |= (unsigned char)(0x80 >> ((field - 1) % 8));
}

static int testBit(const unsigned char *bitmap, int field)
{
	return (bitmap[(field - 1) / 8] & (0x80 >> ((field - 1) % 8))) != 0;
}


// Digits may be '0'-'9', or ':' to '?', which track 2 uses for its '=' separator (nibble 0xD)
static int isBcdChar(unsigned char c)
{
	return c >= '0' && c <= '?';
}

// Right-justifies digits in BCD: an odd count leaves a leading 0 nibble.
static void digitsToBcd(unsigned char *bcd, const unsigned char *digits, int digitCount, int bcdLen)
{
	int i;

	memset(bcd, 0, bcdLen);

	for ( i = 0; i < digitCount; i++ )
	{
		int nibble = digits[digitCount - 1 - i] - '0';
		unsigned char *target = &bcd[bcdLen - 1 - i/2];

		*target |= (unsigned char)((i % 2 == 0) ? nibble : nibble << 4);
	}
}

static void bcdToDigits(unsigned char *digits, const unsigned char *bcd, int digitCount, int bcdLen)
{
	int i;

	for ( i = 0; i < digitCount; i++ )
	{
		unsigned char value = bcd[bcdLen - 1 - i/2];
		int nibble = (i % 2 == 0) ? (value & 0x0F) : (value >> 4);

		digits[digitCount - 1 - i] = (unsigned char)('0' + nibble);
	}

	digits[digitCount] = '\0';
}

// Writes a decimal value as BCD length bytes
static void writeLength(unsigned char *out, int value, int bytes)
{
	char digits[16];   // Room for any int, though values here are at most 4 digits

	snprintf(digits, sizeof(digits), "%0*d", bytes * 2, value);
	digitsToBcd(out, (const unsigned char *)digits, bytes * 2, bytes);
}

// Reads BCD length bytes. Returns -1 if they contain a nibble that isn't a decimal digit.
static int readLength(const unsigned char *in, int bytes)
{
	int value = 0, i;

	for ( i = 0; i < bytes; i++ )
	{
		int high = in[i] >> 4, low = in[i] & 0x0F;

		if ( high > 9 || low > 9 )
			return -1;

		value = value * 100 + high * 10 + low;
	}

	return value;
}


static int isPresent(const FieldDef *def, const unsigned char *value)
{
	size_t i;

	if ( def->type != BIN_FIXED )
		return value[0] != '\0';

	// A binary value can start with a zero byte, so check all of it
	for ( i = 0; i < (size_t)def->size; i++ )
	{
		if ( value[i] != 0 )
			return 1;
	}

	return 0;
}

// Appends one field at out. Returns the bytes written, or -1 if it is invalid or doesn't fit.
static int packField(const FieldDef *def, const unsigned char *value, unsigned char *out, int room)
{
	int len = (def->type == BIN_FIXED) ? def->size : (int)strlen((const char *)value);
	int i, wire;

	switch ( def->type )
	{
		case NUM_FIXED:
		case NUM_LL:
			for ( i = 0; i < len; i++ )
			{
				if ( !isBcdChar(value[i]) )
					return -1;
			}
			break;

		default:
			break;
	}

	switch ( def->type )
	{
		case NUM_FIXED:
			if ( len > def->size * 2 || def->size > room )
				return -1;
			digitsToBcd(out, value, len, def->size);    // Unused leading nibbles stay 0, which is the left padding
			return def->size;

		case NUM_LL:
			wire = (len + 1) / 2;
			if ( len > 99 || 1 + wire > room )
				return -1;
			writeLength(out, len, 1);
			digitsToBcd(out + 1, value, len, wire);
			return 1 + wire;

		case TEXT_FIXED:
			if ( len > def->size || def->size > room )
				return -1;
			memset(out, '0', def->size - len);
			memcpy(out + def->size - len, value, len);
			return def->size;

		case TEXT_LL:
			if ( len > 99 || 1 + len > room )
				return -1;
			writeLength(out, len, 1);
			memcpy(out + 1, value, len);
			return 1 + len;

		case TEXT_LLLL:
			if ( len > 9999 || 2 + len > room )
				return -1;
			writeLength(out, len, 2);
			memcpy(out + 2, value, len);
			return 2 + len;

		case BIN_FIXED:
			if ( len > room )
				return -1;
			memcpy(out, value, len);
			return len;
	}

	return -1;
}

int pack(IsoPkt *isoPkt, unsigned char *pkt, int pktSize)
{
	unsigned char bitmap[16] = {0};
	int bitmapLen = isoPkt->isSecondaryBitMapSet ? 16 : 8;
	int offset = 2 + 2 + bitmapLen;   // Length, MTI, bitmap
	int i;

	if ( pktSize > MAX_PKT_LEN )
		pktSize = MAX_PKT_LEN;

	if ( pktSize < offset || strlen((const char *)isoPkt->MTI) != 4 )
		return -1;

	if ( isoPkt->isSecondaryBitMapSet )
		setBit(bitmap, 1);

	for ( i = 0; i < FIELD_COUNT; i++ )
	{
		const FieldDef *def = &fieldTable[i];
		const unsigned char *value = (const unsigned char *)isoPkt + def->offset;
		int written;

		if ( !isPresent(def, value) )
			continue;

		written = packField(def, value, pkt + offset, pktSize - offset);
		if ( written == -1 )
		{
			File_Log("pack : field %d is invalid or doesn't fit", def->number);
			return -1;
		}

		offset += written;
		setBit(bitmap, def->number);
	}

	writeLength(pkt, offset, 2);
	digitsToBcd(pkt + 2, isoPkt->MTI, 4, 2);
	memcpy(pkt + 4, bitmap, bitmapLen);
	memcpy(isoPkt->BITMAP, bitmap, bitmapLen);

	isoPkt->pktLen = (unsigned short)offset;

	return offset;
}


// Reads one field at in. Returns the bytes consumed, or -1 if it is malformed.
// Every length comes from the packet, so check it against both the packet and the destination.
static int unpackField(const FieldDef *def, const unsigned char *in, int room, unsigned char *value)
{
	int len, wire, header;

	switch ( def->type )
	{
		case NUM_FIXED:
			if ( def->size > room || (size_t)(def->size * 2) >= def->capacity )
				return -1;
			bcdToDigits(value, in, def->size * 2, def->size);
			return def->size;

		case TEXT_FIXED:
		case BIN_FIXED:
			if ( def->size > room || (size_t)def->size >= def->capacity )
				return -1;
			memcpy(value, in, def->size);
			value[def->size] = '\0';
			return def->size;

		case NUM_LL:
			if ( room < 1 || (len = readLength(in, 1)) == -1 )
				return -1;
			wire = (len + 1) / 2;
			if ( 1 + wire > room || (size_t)len >= def->capacity )
				return -1;
			bcdToDigits(value, in + 1, len, wire);
			return 1 + wire;

		case TEXT_LL:
		case TEXT_LLLL:
			header = (def->type == TEXT_LL) ? 1 : 2;
			if ( room < header || (len = readLength(in, header)) == -1 )
				return -1;
			if ( header + len > room || (size_t)len >= def->capacity )
				return -1;
			memcpy(value, in + header, len);
			value[len] = '\0';
			return header + len;
	}

	return -1;
}

int unpack(const unsigned char *pkt, int pktLen, IsoPkt *isoPkt)
{
	int bitmapLen, offset, declaredLen, i, field;

	memset(isoPkt, 0, sizeof(*isoPkt));

	if ( pktLen < 2 + 2 + 8 )
		return -1;

	declaredLen = readLength(pkt, 2);
	if ( declaredLen != pktLen )
	{
		File_Log("unpack : packet length %d doesn't match the %d bytes received", declaredLen, pktLen);
		return -1;
	}

	isoPkt->pktLen = (unsigned short)declaredLen;

	bcdToDigits(isoPkt->MTI, pkt + 2, 4, 2);

	// Bit 1 of the primary bitmap says whether the secondary bitmap follows
	isoPkt->isSecondaryBitMapSet = testBit(pkt + 4, 1);
	bitmapLen = isoPkt->isSecondaryBitMapSet ? 16 : 8;

	if ( pktLen < 2 + 2 + bitmapLen )
		return -1;

	memcpy(isoPkt->BITMAP, pkt + 4, bitmapLen);
	offset = 2 + 2 + bitmapLen;

	// Fields 65 to 128 can't be skipped, because their lengths aren't known
	for ( field = 65; field <= bitmapLen * 8; field++ )
	{
		if ( testBit(isoPkt->BITMAP, field) )
		{
			File_Log("unpack : field %d isn't supported", field);
			return -1;
		}
	}

	for ( i = 0; i < FIELD_COUNT; i++ )
	{
		const FieldDef *def = &fieldTable[i];
		unsigned char *value = (unsigned char *)isoPkt + def->offset;
		int consumed;

		if ( !testBit(isoPkt->BITMAP, def->number) )
			continue;

		consumed = unpackField(def, pkt + offset, pktLen - offset, value);
		if ( consumed == -1 )
		{
			File_Log("unpack : field %d is malformed", def->number);
			return -1;
		}

		offset += consumed;

		if ( def->isSensitive )
			File_Log("D%-3d : <hidden>", def->number);
		else
			File_Log("D%-3d : %s", def->number, value);
	}

	if ( offset != pktLen )
	{
		File_Log("unpack : %d bytes left over after the last field", pktLen - offset);
		return -1;
	}

	return 0;
}


/*********************************************** Tests *************************************************/

#define SET(pkt, n, text) strcpy((char *)(pkt).D##n, text)

static int failures = 0;

static void expectField(const char *name, const unsigned char *actual, const char *expected)
{
	if ( strcmp((const char *)actual, expected) != 0 )
	{
		printf("FAIL : %s : got '%s', expected '%s'\n", name, actual, expected);
		failures++;
	}
}

static void expect(const char *name, int condition)
{
	if ( !condition )
	{
		printf("FAIL : %s\n", name);
		failures++;
	}
}

int parserTest(void)
{
	static IsoPkt reqPkt, rspPkt;
	static unsigned char pkt[8192];
	int len;

	// Invented test values, not real card data
	strcpy((char *)reqPkt.MTI, "0200");
	SET(reqPkt,  2, "12345678901234567");
	SET(reqPkt,  3, "000000");
	SET(reqPkt,  4, "1234");
	SET(reqPkt, 14, "1214");
	SET(reqPkt, 27, "7");
	SET(reqPkt, 35, "1234567890123456=12142061234567");
	SET(reqPkt, 37, "RRN NUMBER");
	SET(reqPkt, 39, "00");
	SET(reqPkt, 41, "41012001");
	SET(reqPkt, 45, "Track 1 data");
	SET(reqPkt, 48, "Additional data - private");
	SET(reqPkt, 49, "356");
	memcpy(reqPkt.D52, "\x00\x11\x22\x33\x44\x55\x66\x77", 8);
	SET(reqPkt, 55, "EMV data");

	len = pack(&reqPkt, pkt, sizeof(pkt));
	expect("pack", len > 0);

	File_Log("Packet length : %d", len);

	expect("unpack", unpack(pkt, len, &rspPkt) == 0);

	expectField("MTI", rspPkt.MTI, "0200");
	expectField("D2 odd length", rspPkt.D2, "12345678901234567");
	expectField("D4 left padded", rspPkt.D4, "000000001234");
	expectField("D27 left padded", rspPkt.D27, "07");
	expectField("D35 separator", rspPkt.D35, "1234567890123456=12142061234567");
	expectField("D37 left padded", rspPkt.D37, "00RRN NUMBER");
	expectField("D48", rspPkt.D48, "Additional data - private");
	expectField("D55", rspPkt.D55, "EMV data");
	expect("D52 binary starting with zero", memcmp(rspPkt.D52, reqPkt.D52, 8) == 0);
	expect("D5 absent", rspPkt.D5[0] == '\0');

	// Malformed input must fail cleanly
	expect("reject truncated packet", unpack(pkt, len - 1, &rspPkt) == -1);
	expect("reject tiny packet", unpack(pkt, 3, &rspPkt) == -1);

	// A PAN length of 99 would overflow D2, which holds 20 digits
	{
		static unsigned char bad[64];

		writeLength(bad, sizeof(bad), 2);
		digitsToBcd(bad + 2, (const unsigned char *)"0200", 4, 2);
		setBit(bad + 4, 2);
		bad[12] = 0x99;
		expect("reject oversized field length", unpack(bad, sizeof(bad), &rspPkt) == -1);
	}

	// Values too long for the format, or digits that aren't BCD, must not be packed
	{
		static IsoPkt badPkt;

		strcpy((char *)badPkt.MTI, "0200");
		SET(badPkt, 41, "123456789");   // The array holds 16 characters, but this field is 8 on the wire
		expect("reject long fixed field", pack(&badPkt, pkt, sizeof(pkt)) == -1);

		memset(badPkt.D41, 0, sizeof(badPkt.D41));
		SET(badPkt, 2, "12AB");
		expect("reject non-digit PAN", pack(&badPkt, pkt, sizeof(pkt)) == -1);

		memset(badPkt.D2, 0, sizeof(badPkt.D2));
		memset(badPkt.D55, 'x', sizeof(badPkt.D55) - 1);
		expect("reject packet larger than the buffer", pack(&badPkt, pkt, 100) == -1);
	}

	printf("%s\n", failures ? "TESTS FAILED" : "All tests passed");

	return failures ? 1 : 0;
}

int main(void)
{
	return parserTest();
}
