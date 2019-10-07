//http://clc-wiki.net/wiki/C_standard_library:string.h:memmove

/***************************** Basic stuffs ************************************
 *
 * structure ( padding, packging, bitfields )
 * union
 * typedef
 * enum
 * writing own functions equal to standard string libraries
 * bit operations
 *
 * sizeof(enum), enum values, macros and enum difference ?
 * sizeof(union), union member values if we modify a value, application of unions ?
*******************************************************************************/


/********************* Dynamic memory allocation *******************************
 *
 * void *malloc(size_t size)
 * void *calloc(size_t nmemb, size_t size)
 * void *realloc(void *ptr, size_t size)
 * void free(void *ptr)
 *
 * realloc(NULL, 10*sizeof(int)) == malloc(10*sizeof(int))
 *
 * realloc(ptr, 0)               == free(ptr)
 *
 * near far and hugepointers
 *
 * how free() api, free the memory without we are passing the allocated memory size ?
 *
*******************************************************************************/


#ifdef _WIN32
#else
#endif

#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;

/*typedef enum
{
	false,
	true
}bool;*/



/************************* Structure Packing ***********************************/
#ifdef _WIN32

#pragma pack(push, 1) // exact fit - no padding
struct DATE{
	unsigned char nWeekDay;
	unsigned char nMonthDay;
	unsigned char nMonth;
	unsigned char nYear; 
}; 
#pragma pack(pop) //back to whatever the previous packing mode was

#else

struct DATE{
	unsigned char nWeekDay;
	unsigned char nMonthDay;
	unsigned char nMonth;
	unsigned char nYear;
}__attribute__((packed));

#endif

/******************************** Structure Bit Field *****************************/

typedef struct 
{
   unsigned char nWeekDay  : 3;    // 0..7   (3 bits)
   unsigned char nMonthDay : 5;    // 0..31  (5 bits)
   unsigned char nYear     : 7;    // 0..99 (7 bits)
   unsigned char           : 0;    // Force allignment to next boundary
   unsigned char nMonth    : 4;    // 1..12  (4 bits)

}DATE_BF;                          // Date->nYear == (*Date).nYear        and *Date.nYear will produce compiler err

/**********************************************************************************/

#define BIT_TEST(b, n) (b & (1<<n))
#define BIT_ON(b, n) (b|=(1<<n))
#define BIT_OFF(b, n) (b &= ~(1<<n))
#define BIT_TOGGLE(b, n) (b ^= (1<<n))
#define HIGH_NIBBLE(b) ( b>>4 )
#define LOW_NIBBLE(b) ( b & 0x0F )
#define BIT_ROTATE_RIGHT(b, n) ( b = (b>>n | b<<(8-n)) )
#define BIT_ROTATE_LEFT(b, n)  ( b = (b<<n | b>>(8-n)) )
#define BIT_SET_EVEN_POS(b) ( b |= 0xAA)
#define BIT_SET_ODD_POS(b)  ( b |= 0x55)
byte bitcount(unsigned int x);
const char *byte2Binary(byte x);
bool isLittleEndian( void );


#define ISDIGIT(a) (a >= '0' && a <= '9')
#define ISLOWER(a) ( a >= 'a' && a <= 'z')
#define ISUPPER(a) ( a >= 'A' && a <= 'Z')
#define ISALPHA(a) (ISLOWER(a) || ISUPPER(a))
#define ISALNUM(a) ( ISALPHA(a) || ISDIGIT(a) )
#define TOLOWER(a) (( ISUPPER(a)) ? (a + ('a' - 'A')) : a)
#define TOUPPER(a) (( ISLOWER(a)) ? (a - ('a' - 'A')) : a)
#define ISSPACE(a) ( a == ' ' || a == '\n' || a == '\r' || a == '\t' || a == '\v' || a == '\f' )

bool ISDIGIT_STR(const char *str);
bool ISALPHA_STR(const char *str);
bool ISALNUM_STR(const char *str);
char *TOLOWER_STR(char *str);
char *TOUPPER_STR(char *str);

size_t STRLEN(const char *str);
char *STRCPY(char *dest, const char *source);
char *STRNCPY(char *dest, const char *source, int len);
char *STRREV(char *str);
char *STRCAT(char *dest, const char *source);
char *STRNCAT(char *dest, const char *source, int len);
int STRCMP(const char *str1, const char *str2);
int STRICMP(const char *str1, const char *str2);
int STRNCMP(const char *str1, const char *str2, int len);
int STRINCMP(const char *str1, const char *str2, int len);
char *STRSTR(const char *str, const char *substr);
char *STRISTR(const char *str, const char *substr);
char *STRTRIM(char *str);
char *STR_CHAR_REPLACE(char *str, int value, int newValue);
char *STR_CHAR_REMOVE(char *str, int ch);
char *PAD_CHAR(char *src, int pLen, int ch);

void *MEMCPY(void *dest, const void *src, int len);
void *MEMSET(void *src, int value, int len);
void *MEMMOVE(void *dest, const void *src, int len);
int ATOI(const char *s);

void **allocate2dPtr(int row, int column, int blockSize);
void free2dPtr(void **ptr, int row, int column);

char *STRREV_BY_IDX_LEN(char *p, int sidx, int eidx);
char *WORDREV(char *s);


int main()
{
	char str[] = "my name is siva";
	printf("\n%s", str);
	printf("\n%s", WORDREV(str));
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


const char *byte2Binary(byte x)
{
    static char binary[9];
    MEMSET(binary, 0, sizeof(binary));

    byte n;

    for (n = 7; n >= 0; n--)
    {
		STRCAT(binary, BIT_TEST(x, n) ? "1" : "0");
    }

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
		str++;

	return *str ? false : true;
	
}


bool ISALPHA_STR(const char *str)
{
	if ( !str || !*str )
		return false;

	while(ISALPHA(*str))
		str++;

	return *str ? false : true;
}


bool ISALNUM_STR(const char *str)
{
	if ( !str || !*str )
		return false;

	while(ISALNUM(*str))
		str++;

	return *str ? false : true;
}


char *TOLOWER_STR(char *str)
{

	char *ptr =str;

	while(*str = TOLOWER(*str))
	     str++;

	return ptr;

}

char *TOUPPER_STR(char *str)
{
	char *ptr =str;

	while(*str = TOUPPER(*str))
	     str++;

	return ptr;

}


size_t STRLEN(const char *str)
{
	size_t length =0;

	while(*str++)
		length++;
	
	return length;
}


char *STRCPY(char *dest, const char *source)
{
	char *ptr = dest;

	while(*dest++ = *source++)
		;

	return ptr;
}



char *STRNCPY(char *dest , const char *source , int len)
{

	char *ptr = dest;

	while(( len-- > 0) && (*dest++ = *source++))
		;

	while( len-- > 0)
	    *dest++  = 0;

	return ptr;

}

char *STRCAT (char *dest, const char *source)
{
	char *ptr = dest;

	while(*dest)
		dest++;

	while(*dest++ = *source++)
		;

	return ptr;
}

char *STRNCAT (char *dest , const char *source , int len)
{
 
	char *ptr = dest;

	while( *dest)
		dest++;
 
	while(( len-- > 0) && (*dest++ = *source++) )
		;

	while(len-- > 0)
		*dest++ = 0;

	return ptr;
}


char *STRREV_BY_IDX_LEN(char *p, int sidx, int eidx) {

	char temp;
	while ( sidx < eidx)
		temp = *(p+sidx), *(p+sidx++) = *(p+eidx), *(p+eidx--) = temp;
	return p;
}

char *WORDREV(char *s){

	int sidx = 0, eidx = 0;
	char * p = s;

	STRREV(p); // my name is siva -> avis si eman ym

	while ( *p ) {
		if ( ISSPACE(*p) ) {
			STRREV_BY_IDX_LEN(s, sidx, eidx-1); // -1 for ignore space,  0 4-1, 5 7-1, 8 12-1, 13
			sidx = ++eidx;                      // +1 for move the idx from space to next char   5, 8, 13
		} else {
			++eidx;
		}
		p++;
	}
	
	if ( sidx != eidx)
		STRREV_BY_IDX_LEN(s, sidx, eidx-1); // 13, 15-1 

	return s;

}

char *STRREV(char *source)
{
	
	int len = STRLEN(source)-1;
	int pos =0, temp =0;

	while(pos <= len)
		temp = *(source+pos), *(source+pos++) = *(source+len), *(source+len--) = temp;

	return source;
		
}

char *STRREV_(char *str)
{

	char *p1, *p2;

    if (!str || !*str)  // If s==NULL || s[0] == NULL
        return str;

	p1=str, p2=str+STRLEN(str)-1;

    while(p2 > p1)
        *p1 ^= *p2, *p2 ^= *p1, *p1++ ^= *p2--;

    return str;
}


int STRCMP(const char *str1, const char *str2)
{
	while( *str1 && (*str1 == *str2) )
		str1++, str2++;

	return (*(unsigned char *)str1-*(unsigned char *)str2);
}


int STRICMP(const char *str1, const char *str2)
{
	while( *str1 && (TOUPPER(*str1) == TOUPPER(*str2)) )
		str1++, str2++;

	return TOUPPER(*str1) - TOUPPER(*str2);
}


int STRNCMP(const char *str1, const char *str2, int len)
{
	while( (--len > 0 ) && *str1 && (*str1 == *str2) )
			str1++, str2++;

	return (*(unsigned char *)str1-*(unsigned char *)str2);
}


int STRNICMP(const char *str1, const char *str2, int len)
{
	while( (--len > 0) && *str1 && (TOUPPER(*str1) == TOUPPER(*str2)) )
		str1++, str2++;

	return TOUPPER(*str1) - TOUPPER(*str2);
}


char *STRSTR(const char *str, const char *substr)
{
	int len = STRLEN(substr);

	while(*str)
		if ( STRNCMP(str++, substr, len) == 0)
			return (char *)str-1;
		
	return (char *)0;

}


char *STRISTR(const char *str, const char *substr)
{
	int len = STRLEN(substr);

	while(*str)
		if ( STRNICMP(str++, substr, len) == 0)
			return (char *)str-1;
		
	return (char *)0;

}

char *STRTRIM(char *str)
{
	int n = STRLEN(str);

	while( (n-->0) && ISSPACE(*(str+n)) )
		*(str+n) = 0;

	return str;
}


char *STR_CHAR_REPLACE(char *str, int value, int newValue)
{
	char *ptr = str;

	while(*str)
		(*str == value) ? (*str++ = newValue) : *str++;

	return ptr;

}

char *STR_CHAR_REMOVE(char *str, int ch)
{

	char *ptr = str;
	char *dst = str;

	while(*dst = *str++)
		(*dst == ch) ? dst : dst++;

	return ptr;

}

char *STR_REMOVE_DUP(char *s) {

	bool charset[256] = {0};
	char ch;
	int idx=0, pos=0, i = 0, j = 0;

	//O(n)
	while ( *(s+pos)) {

		ch = *(s+pos);
		if ( charset[ch] == false){
			charset[ch] = true;
			*(s+idx++) = *(s+pos);
		}
		pos++;
	}


	// O(2n)
	/*for ( i = 0; i < strlen(s); i++) {

		for ( j = 0; j < idx; j++) {
			if ( s[j] == s[i])
				break;
		}

		if ( j == idx)
			s[idx++] = s[i];

	}*/

	s[idx] = 0;

	return s;
}

char *PAD_CHAR(char *src, int pLen, int ch)
{

	int sLen, dir;
	char *p = src;

	pLen = ((dir = (pLen < 0) ? 'l' : 'r') == 'l') ? -pLen : pLen;

	src[pLen] = 0;
	sLen = STRLEN(src);

	if(sLen >= pLen)
		return src;

	(dir == 'l') ? MEMMOVE(src + pLen - sLen, src, sLen) : (p+=sLen);

	MEMSET(p, ch, pLen - sLen);

	return src;

}


void *MEMCPY(void *dest, const void *src, int len)
{
	void *p = dest;
	char *d = (char *)dest;
	char *s = (char *)src;

	while (len--)
		*d++ = *s++;

	return p;
}


void *MEMSET(void *src, int value, int len)
{
	register char *p = (char *)src;

	while(len--)
		*p++ = value;

	return src;
}

void *MEMMOVE(void *dest, const void *src, int len)
{

	void *p = dest;
	char *d = (char *)dest;
	char *s = (char *)src;

	if (len<=0)
		return p;

	if ( d >= s+len )  // No Overlap, Use memcpy when source is higher than dest
		return MEMCPY(d, s, len);

	s += len, d += len; // Copy backwards, from end to beginning

	while (len--)
		*--d = *--s;

	return p;
}


int ATOI(const char *s)
{
	int n, sign;
	n=0;

	while ( *s == ' ')  // skip white spaces
		s++;

	sign = (*s == '-') ? -1 : 1;

	if ( *s == '+' || *s == '-') // skip sign
		s++;


	while ( *s >= '0' && *s <='9')
		n = 10 * n + (*s++ - '0');

	return n * sign;
}


void **allocate2dPtr(int row, int column, int blockSize)
{
	void **ptr = (void **)calloc(row, sizeof(void *));
	
	if ( ptr == NULL)
		return NULL;

	while(row--)
		ptr[row] = calloc(column, blockSize);

	return ptr;
}

void free2dPtr(void **ptr, int row, int column)
{
	while(row--)
		free(ptr[row]), ptr[row] = NULL;

	free(ptr);
	ptr = NULL;
}
