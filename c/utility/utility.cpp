// g++ utility.cpp -o utility
//
// Own versions of standard library functions, plus notes on structs, unions, enums and bits.

/***************************** Basic stuffs ************************************
 *
 * structure ( padding, packing, bitfields )
 * union
 * typedef
 * enum
 * writing own functions equal to standard string libraries
 * bit operations
 *
 * sizeof(enum)  -> an int on most systems. C++ can fix the type: enum Color : char { RED };
 * enum vs macro -> enum values are real constants with a type and scope, and a debugger can
 *                  show their names. A macro is only text replacement by the preprocessor.
 * enum values   -> count up from 0, unless set. Each one is the previous value + 1.
 * sizeof(union) -> the largest member, plus any padding needed for alignment.
 * union members -> share the same memory, so writing one changes what the others read. See ../signals/union.cpp
 * union uses    -> a value that is one of several types, usually with a tag saying which,
 *                  and viewing the same bytes in two ways, such as a packet or a register.
 *
*******************************************************************************/


/********************* Dynamic memory allocation *******************************
 *
 * void *malloc(size_t size)                    -> memory is not initialized
 * void *calloc(size_t nmemb, size_t size)      -> memory is zeroed, and nmemb * size is checked for overflow
 * void *realloc(void *ptr, size_t size)        -> resizes. May move the block, and returns NULL on failure, leaving ptr valid
 * void free(void *ptr)                         -> free(NULL) does nothing
 *
 * realloc(NULL, 10*sizeof(int)) is the same as malloc(10*sizeof(int))
 *
 * realloc(ptr, 0)  -> not a safe way to free: the result depends on the system, and it is
 *                     undefined behaviour in C23. Call free(ptr) instead.
 *
 * How does free() know the size?
 *                  -> The allocator records the block's size in its own bookkeeping,
 *                     typically in a small header just before the address malloc returned.
 *
 * Always keep the result of realloc in a temporary: p = realloc(p, n) loses p if it fails.
 *
*******************************************************************************/


/************************* Function pointers ***********************************
 *
 * int   *f(void)       - Function f returning an int*
 * int   (*g)(void)     - Pointer to function (g) returning an int
 * char (*h)(int, int)  - h is a pointer to a function which takes two int parameters and returns char
 *
*******************************************************************************/

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
size_t top_of_stack;


/************************* Structure Padding and Packing ***********************/

struct PADDED {
	char tag;       // 3 bytes of padding follow, so that value starts at a multiple of 4
	int  value;
};                      // sizeof is 8

#pragma pack(push, 1)   // exact fit - no padding
struct PACKED {
	char tag;
	int  value;     // Can be misaligned, which is slower, and can fault on some CPUs
};                      // sizeof is 5
#pragma pack(pop)       // back to whatever the previous packing mode was


/******************************** Structure Bit Field *****************************/

typedef struct
{
   unsigned char nWeekDay  : 3;    // 0..7   (3 bits)
   unsigned char nMonthDay : 5;    // 0..31  (5 bits)
   unsigned char nYear     : 7;    // 0..127 (7 bits)
   unsigned char           : 0;    // Force alignment to next boundary
   unsigned char nMonth    : 4;    // 1..12  (4 bits)

}DATE_BF;                          // Date->nYear is (*Date).nYear. *Date.nYear is a compiler error, because . binds tighter than *

/**********************************************************************************/

// Bit numbers count from 0, the lowest bit. The parentheses around the arguments
// make expressions work: without them, 1<<1|2 means (1<<1)|2 instead of 1<<3.
#define BIT_TEST(b, n) ((b) & (1<<(n)))
#define BIT_ON(b, n) ((b) |= (1<<(n)))
#define BIT_OFF(b, n) ((b) &= ~(1<<(n)))
#define BIT_TOGGLE(b, n) ((b) ^= (1<<(n)))
#define HIGH_NIBBLE(b) ((b)>>4)
#define LOW_NIBBLE(b) ((b) & 0x0F)
// For a byte. n must be 0 to 8
#define BIT_ROTATE_RIGHT(b, n) ((b) = ((b)>>(n) | (b)<<(8-(n))))
#define BIT_ROTATE_LEFT(b, n)  ((b) = ((b)<<(n) | (b)>>(8-(n))))
#define BIT_SET_EVEN_POS(b) ((b) |= 0xAA)   // Even positions counting from 1: bits 1, 3, 5, 7 counting from 0
#define BIT_SET_ODD_POS(b)  ((b) |= 0x55)   // Odd positions counting from 1: bits 0, 2, 4, 6 counting from 0
byte bitcount(unsigned int x);
const char *byte2Binary(byte x);
bool isLittleEndian( void );


// These use their argument more than once, so an argument like *p++ would run several times
#define ISDIGIT(a) ((a) >= '0' && (a) <= '9')
#define ISLOWER(a) ((a) >= 'a' && (a) <= 'z')
#define ISUPPER(a) ((a) >= 'A' && (a) <= 'Z')
#define ISALPHA(a) (ISLOWER(a) || ISUPPER(a))
#define ISALNUM(a) (ISALPHA(a) || ISDIGIT(a))
#define TOLOWER(a) (ISUPPER(a) ? ((a) + ('a' - 'A')) : (a))
#define TOUPPER(a) (ISLOWER(a) ? ((a) - ('a' - 'A')) : (a))
#define ISSPACE(a) ((a) == ' ' || (a) == '\n' || (a) == '\r' || (a) == '\t' || (a) == '\v' || (a) == '\f')

bool ISDIGIT_STR(const char *str);
bool ISALPHA_STR(const char *str);
bool ISALNUM_STR(const char *str);
char *TOLOWER_STR(char *str);
char *TOUPPER_STR(char *str);

size_t STRLEN(const char *str);
char *STRCPY(char *dest, const char *source);
char *STRNCPY(char *dest, const char *source, int len);
char *STRREV(char *str);
char *STRREV_XOR(char *str);
char *STRCAT(char *dest, const char *source);
char *STRNCAT(char *dest, const char *source, int len);
int STRCMP(const char *str1, const char *str2);
int STRICMP(const char *str1, const char *str2);
int STRNCMP(const char *str1, const char *str2, int len);
int STRNICMP(const char *str1, const char *str2, int len);
char *STRSTR(const char *str, const char *substr);
char *STRISTR(const char *str, const char *substr);
char *STRTRIM(char *str);
char *STR_CHAR_REPLACE(char *str, int value, int newValue);
char *STR_CHAR_REMOVE(char *str, int ch);
char *STR_REMOVE_DUP(char *str);
char *PAD_CHAR(char *src, int pLen, int ch);

void *MEMCPY(void *dest, const void *src, int len);
void *MEMSET(void *src, int value, int len);
void *MEMMOVE(void *dest, const void *src, int len);
int ATOI(const char *s);

void **allocate2dPtr(int row, int column, int blockSize);
void free2dPtr(void **ptr, int row);
size_t stack_size();

char *STRREV_BY_IDX_LEN(char *p, int sidx, int eidx);
char *WORDREV(char *s);
void funcPtrTest();
void pointerConversion();


int main()
{
	int x = 0;
	top_of_stack = (size_t)&x;

	char str[] = "my name is siva";
	printf("\n%s", str);
	printf("\n%s", WORDREV(str));

	printf("\nsizeof PADDED = %zu, PACKED = %zu, DATE_BF = %zu", sizeof(PADDED), sizeof(PACKED), sizeof(DATE_BF));
	printf("\nLittle endian : %s", isLittleEndian() ? "yes" : "no");
	printf("\n0xA5 in binary = %s, bits set = %d", byte2Binary(0xA5), bitcount(0xA5));

	funcPtrTest();
	pointerConversion();

	printf("\nStack used by a call : %zu bytes\n", stack_size());

	return 0;
}


byte bitcount(unsigned int x)
{
	byte count = 0;

	while(x)
	{
		count += x&1;
		x>>=1;
	}

	return count;
}


// Returns a static buffer, so the next call overwrites it, and it isn't thread-safe
const char *byte2Binary(byte x)
{
	static char binary[9];

	// n must be signed: an unsigned n >= 0 is always true, so the loop would never end
	for ( int n = 7; n >= 0; n-- )
		binary[7 - n] = BIT_TEST(x, n) ? '1' : '0';

	binary[8] = '\0';

	return binary;
}


bool isLittleEndian( void )
{
	unsigned long num = 0x12345678;

	char * temp = (char *)&num;

	return (*temp == 0x78);
}


bool ISDIGIT_STR(const char *str)
{
	if ( !str || !*str )
		return false;

	while(ISDIGIT(*str))
		++str;

	return *str ? false : true;
}


bool ISALPHA_STR(const char *str)
{
	if ( !str || !*str )
		return false;

	while(ISALPHA(*str))
		++str;

	return *str ? false : true;
}


bool ISALNUM_STR(const char *str)
{
	if ( !str || !*str )
		return false;

	while(ISALNUM(*str))
		++str;

	return *str ? false : true;
}


char *TOLOWER_STR(char *str)
{
	char *ptr = str;

	while( (*str = TOLOWER(*str)) )
		++str;

	return ptr;
}

char *TOUPPER_STR(char *str)
{
	char *ptr = str;

	while( (*str = TOUPPER(*str)) )
		++str;

	return ptr;
}


size_t STRLEN(const char *str)
{
	size_t length = 0;

	while(*str++)
		++length;

	return length;
}


char *STRCPY(char *dest, const char *source)
{
	char *ptr = dest;

	while( (*dest++ = *source++) )
		;

	return ptr;
}


// Like strncpy: pads with NULs up to len, and adds no NUL at all if source is len or longer
char *STRNCPY(char *dest, const char *source, int len)
{
	char *ptr = dest;

	while( (len-- > 0) && (*dest++ = *source++) )
		;

	while( len-- > 0 )
		*dest++ = 0;

	return ptr;
}

char *STRCAT(char *dest, const char *source)
{
	char *ptr = dest;

	while(*dest)
		dest++;

	while( (*dest++ = *source++) )
		;

	return ptr;
}

// Unlike strncpy, strncat always adds a NUL, and never pads
char *STRNCAT(char *dest, const char *source, int len)
{
	char *ptr = dest;

	while( *dest )
		dest++;

	while( (len-- > 0) && *source )
		*dest++ = *source++;

	*dest = '\0';

	return ptr;
}


char *STRREV_BY_IDX_LEN(char *p, int sidx, int eidx)
{
	while ( sidx < eidx )
	{
		char temp = p[sidx];
		p[sidx++] = p[eidx];
		p[eidx--] = temp;
	}

	return p;
}

// Reverses the order of the words: reverse the whole string, and then each word
char *WORDREV(char *s)
{
	int sidx = 0, eidx = 0;
	char *p = s;

	STRREV(p); // my name is siva -> avis si eman ym

	while ( *p ) {
		if ( ISSPACE(*p) ) {
			STRREV_BY_IDX_LEN(s, sidx, eidx-1); // -1 to leave out the space
			sidx = ++eidx;                      // +1 to move from the space to the next word
		} else {
			++eidx;
		}
		p++;
	}

	if ( sidx != eidx )
		STRREV_BY_IDX_LEN(s, sidx, eidx-1);  // The last word has no space after it

	return s;
}

char *STRREV(char *source)
{
	return STRREV_BY_IDX_LEN(source, 0, (int)STRLEN(source) - 1);
}

// The XOR swap needs no temporary variable, but it is slower and harder to read, so
// it is only a curiosity. It also breaks if both pointers are equal, so p2 > p1 matters.
char *STRREV_XOR(char *str)
{
	char *p1, *p2;

	if (!str || !*str)  // NULL or empty
		return str;

	p1 = str, p2 = str + STRLEN(str) - 1;

	while(p2 > p1)
		*p1 ^= *p2, *p2 ^= *p1, *p1++ ^= *p2--;

	return str;
}


// The C standard compares as unsigned char, so bytes above 127 sort after ASCII
int STRCMP(const char *str1, const char *str2)
{
	while( *str1 && (*str1 == *str2) )
		str1++, str2++;

	return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}


// Compares in lower case, as strcasecmp does. This decides the order of letters against
// characters between 'Z' and 'a', such as '_'.
static int lowerByte(const char *s)
{
	unsigned char c = *(const unsigned char *)s;

	return TOLOWER(c);
}

int STRICMP(const char *str1, const char *str2)
{
	while( *str1 && (lowerByte(str1) == lowerByte(str2)) )
		str1++, str2++;

	return lowerByte(str1) - lowerByte(str2);
}


int STRNCMP(const char *str1, const char *str2, int len)
{
	if ( len <= 0 )   // No characters to compare, so they are equal
		return 0;

	while( (--len > 0) && *str1 && (*str1 == *str2) )
		str1++, str2++;

	return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}


int STRNICMP(const char *str1, const char *str2, int len)
{
	if ( len <= 0 )
		return 0;

	while( (--len > 0) && *str1 && (lowerByte(str1) == lowerByte(str2)) )
		str1++, str2++;

	return lowerByte(str1) - lowerByte(str2);
}


// An empty substr matches at the start, even in an empty str, as strstr does
char *STRSTR(const char *str, const char *substr)
{
	int len = (int)STRLEN(substr);

	if ( len == 0 )
		return (char *)str;

	for ( ; *str; str++ )
	{
		if ( STRNCMP(str, substr, len) == 0 )
			return (char *)str;
	}

	return NULL;
}


char *STRISTR(const char *str, const char *substr)
{
	int len = (int)STRLEN(substr);

	if ( len == 0 )
		return (char *)str;

	for ( ; *str; str++ )
	{
		if ( STRNICMP(str, substr, len) == 0 )
			return (char *)str;
	}

	return NULL;
}

// Removes white space from both ends
char *STRTRIM(char *str)
{
	int n = (int)STRLEN(str);
	int start = 0;

	while( (n > 0) && ISSPACE(str[n-1]) )
		str[--n] = '\0';

	while( ISSPACE(str[start]) )
		start++;

	if ( start > 0 )
		MEMMOVE(str, str + start, n - start + 1);   // +1 for the NUL

	return str;
}


char *STR_CHAR_REPLACE(char *str, int value, int newValue)
{
	for ( char *p = str; *p; p++ )
	{
		if ( *p == value )
			*p = (char)newValue;
	}

	return str;
}

char *STR_CHAR_REMOVE(char *str, int ch)
{
	char *ptr = str;
	char *dst = str;

	while( (*dst = *str++) )
	{
		if ( *dst != ch )
			dst++;
	}

	return ptr;
}

// Keeps the first copy of each byte
char *STR_REMOVE_DUP(char *str)
{
	bool charset[UCHAR_MAX + 1] = {false};
	char *p = str;
	char *d = str;
	char ch;

	if ( str == NULL )
		return NULL;

	while ( (ch = *p++) )
	{
		// As an index it must be unsigned: bytes above 127, as in UTF-8, are negative as char
		unsigned char index = (unsigned char)ch;

		if ( !charset[index] )
		{
			charset[index] = true;
			*d++ = ch;
		}
	}

	*d = '\0';

	return str;
}

// Pads to a width of pLen: on the right, or on the left if pLen is negative. Longer strings are
// cut to that width. src must have room for the width plus the NUL.
char *PAD_CHAR(char *src, int pLen, int ch)
{
	int sLen;
	bool padLeft = (pLen < 0);
	char *p = src;

	if ( padLeft )
		pLen = -pLen;

	src[pLen] = '\0';
	sLen = (int)STRLEN(src);

	if ( sLen >= pLen )
		return src;

	if ( padLeft )
		MEMMOVE(src + pLen - sLen, src, sLen);
	else
		p += sLen;

	MEMSET(p, ch, pLen - sLen);

	return src;
}


void *MEMCPY(void *dest, const void *src, int len)
{
	char *d = (char *)dest;
	const char *s = (const char *)src;

	while ( len-- > 0 )
		*d++ = *s++;

	return dest;
}


void *MEMSET(void *src, int value, int len)
{
	char *p = (char *)src;

	while ( len-- > 0 )
		*p++ = (char)value;

	return src;
}

// Handles overlapping memory by choosing the direction that reads each byte before overwriting it
void *MEMMOVE(void *dest, const void *src, int len)
{
	char *d = (char *)dest;
	const char *s = (const char *)src;

	if ( len <= 0 )
		return dest;

	// Forwards is safe if dest is below src, or doesn't overlap it at all.
	// uintptr_t, because comparing pointers into different objects isn't defined.
	if ( (uintptr_t)d < (uintptr_t)s || (uintptr_t)d >= (uintptr_t)s + (uintptr_t)len )
		return MEMCPY(dest, src, len);

	// dest overlaps the end of src, so copy backwards, from end to beginning
	s += len, d += len;

	while ( len-- > 0 )
		*--d = *--s;

	return dest;
}


// Values too big for an int stop at INT_MAX or INT_MIN, like strtol does. (atoi itself is undefined there.)
int ATOI(const char *s)
{
	long long n = 0;
	bool negative;
	const long long limit = (long long)INT_MAX + 1;

	while ( ISSPACE(*s) )  // skip white space
		s++;

	negative = (*s == '-');

	if ( *s == '+' || *s == '-' ) // skip sign
		s++;

	while ( ISDIGIT(*s) )
	{
		n = (10 * n) + (*s++ - '0');

		if ( n > limit )
			n = limit;
	}

	if ( negative )
		return (int)-n;

	return (n > INT_MAX) ? INT_MAX : (int)n;
}


void **allocate2dPtr(int row, int column, int blockSize)
{
	void **ptr = (void **)calloc(row, sizeof(void *));

	if ( ptr == NULL )
		return NULL;

	for ( int i = 0; i < row; i++ )
	{
		ptr[i] = calloc(column, blockSize);

		if ( ptr[i] == NULL )     // Clean up the rows already allocated
		{
			free2dPtr(ptr, i);
			return NULL;
		}
	}

	return ptr;
}

void free2dPtr(void **ptr, int row)
{
	if ( ptr == NULL )
		return;

	while ( row-- > 0 )
		free(ptr[row]);

	free(ptr);
}


void funcPtrTest()
{
	char buffer[101] = {'\0'};
	char *(*funcP)(char *, const char *) = STRCPY;

	printf("\n%s", (*funcP)(buffer, "hello world"));
	printf("\n%s", funcP(buffer, "calling it without the * is the same"));
}

void pointerConversion()
{
	int a[3][3] = {
				{1,2,3},
				{4,5,6},
				{7,8,9}
		      };

	int *p = &a[0][0];   // Pointer to an int: p++ moves by one int
	int (*pp)[3] = a;    // Pointer to an array of 3 ints: pp++ moves by one row

	printf("\n *p = %d, **pp = %d", *p, **pp);
	p++, pp++;
	printf("\n *p = %d, **pp = %d", *p, **pp);
}

// Assumes the stack grows downwards, as on x86 and ARM, and that main set top_of_stack
size_t stack_size()
{
	int x = 0;

	return top_of_stack - (size_t)&x;
}
