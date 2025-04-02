

typedef struct packetField
{
    	unsigned short pktLen;
	unsigned short isSecondaryBitMapSet;
	unsigned char header[11];
	unsigned char MTI[5];
	unsigned char BITMAP[17];
	unsigned char D1[25];    //b 64 || b 128 || b 192  Bit map 
	unsigned char D2[21];    //n..19 Primary account number(PAN)
	unsigned char D3[7];     //n 6 Processing code
	unsigned char D4[13];    //n 12 Amount, transaction
	unsigned char D5[13];    //n 12 Amount, settlement
	unsigned char D6[13];    //n 12 Amount, cardholder billing
	unsigned char D7[11];    //n 10 Transmission date & time
	unsigned char D8[9];     //n 8 Amount, cardholder billing fee
	unsigned char D9[9];     //n 8 Conversion rate, settlement
	unsigned char D10[9];    //n 8 Conversion rate, cardholder billing
	unsigned char D11[7];    //n 6 System trace audit number
	unsigned char D12[7];    //n 6 Time, local transaction (hhmmss)
	unsigned char D13[5];    //n 4 Date, local transaction (MMDD)
	unsigned char D14[5];    //n 4 Date, expiration
	unsigned char D15[5];    //n 4 Date, settlement
	unsigned char D16[5];    //n 4 Date, conversion
	unsigned char D17[5];    //n 4 Date, capture
	unsigned char D18[5];    //n 4 Merchant type
	unsigned char D19[5];    //n 3 Acquiring institution country code
	unsigned char D20[5];    //n 3 PAN extended, country code
	unsigned char D21[5];    //n 3 Forwarding institution. country code
	unsigned char D22[5];    //n 3 Point of service entry mode
	unsigned char D23[5];    //n 3 Application PAN sequence number
	unsigned char D24[5];    //n 3 Function code (ISO 8583:1993)/Network International identifier (NII)
	unsigned char D25[3];    //n 2 Point of service condition code
	unsigned char D26[3];    //n 2 Point of service capture code
	unsigned char D27[3];    //n 1 Authorizing identification response length
	unsigned char D28[13];   //n 8 Amount, transaction fee
	unsigned char D29[13];   //n 8 Amount, settlement fee
	unsigned char D30[13];   //n 8 Amount, transaction processing fee
	unsigned char D31[13];   //n 8 Amount, settlement processing fee
	unsigned char D32[13];   //n ..11	Acquiring institution identification code
	unsigned char D33[13];   //n ..11	Forwarding institution identification code
	unsigned char D34[29];   //n ..28	Primary account number, extended
	unsigned char D35[39];   //z ..37 Track 2 data
	unsigned char D36[105];  //n ...104 Track 3 data
	unsigned char D37[13];   //an 12 Retrieval reference number
	unsigned char D38[7];    //an 6 Authorization identification response
	unsigned char D39[3];    //an 2 Response code
	unsigned char D40[4];    //an 3 Service restriction code
	unsigned char D41[17];   //ans 16 Card acceptor terminal identification
	unsigned char D42[17];   //ans 15 Card acceptor identification code
	unsigned char D43[41];   //ans 40 Card acceptor name/location (1-23 address 24-36 city 37-38 state 39-40 country)
	unsigned char D44[26];   //an ..25 Additional response data
	unsigned char D45[77];   //an ..76 Track 1 data
	unsigned char D46[1000]; //an ...999 Additional data - ISO
	unsigned char D47[1000]; //an ...999 Additional data - national
	unsigned char D48[1000]; //an ...999 Additional data - private
	unsigned char D49[5];    //an 3 Currency code, transaction
	unsigned char D50[5];    //an 3 Currency code, settlement
	unsigned char D51[5];    //an 3 Currency code, cardholder billing
	unsigned char D52[9];    //b 64 Personal identification number data
	unsigned char D53[19];   //n 16 Security related control information  // National ID,  N 18, 17 or 13 digits '0' padding on left
	unsigned char D54[121];  //an ...120 Additional amounts  // Agent or Customer account balance AN 12
	unsigned char D55[1000]; //ans ...999 Reserved ISO
	unsigned char D56[1000]; //ans ...999 Reserved ISO
	unsigned char D57[1000]; //ans ...999 Reserved national
	unsigned char D58[1000]; //ans ...999 Reserved national
	unsigned char D59[1000]; //ans ...999 Reserved national
	unsigned char D60[1000]; //ans ...999 Reserved national
	unsigned char D61[1000]; //ans ...999 Reserved private          // Mini statement B 999
	unsigned char D62[1000]; //ans ...999 Reserved private
	unsigned char D63[1000]; //ans ...999 Reserved private                // Customer finger print, B 999, Biometric template data of customer. 
	unsigned char D64[3];    //b 16 Message authentication code (MAC)
	unsigned char D98[19];   //Ans 19 Biller ID/Code
	unsigned char D101[19];  //Ans 17 Customer Phone, optional field
	unsigned char D102[21];  //N 19 Source Account Number
	unsigned char D103[21];  //N 19 Destination Account Number
	unsigned char D104[31];  //N 19 Bill Number
	unsigned char D120[19];  //ans 16  Device ID / Transaction tracking ID 
	unsigned char D128[5];   //b 16    Client UUID 
		
	
}IsoPkt;


#define SETBIT(x,n) (x|=(1<<n))  // Set a bit , n 0 to 7
#define CLRBIT(x,n) (x&=~(1<<n))
#define TOGGLEBIT(x,n) (x^=(1<< n))
#define TESTBIT(x,n) (x&(1<<n))

#define BIT1 7
#define BIT2 6
#define BIT3 5
#define BIT4 4
#define BIT5 3
#define BIT6 2
#define BIT7 1
#define BIT8 0


#define LEN_D3 3 
#define LEN_D4 6
#define LEN_D5 6
#define LEN_D6 6
#define LEN_D7 5
#define LEN_D8 4
#define LEN_D9 4
#define LEN_D10 4
#define LEN_D11 3
#define LEN_D12 3
#define LEN_D13 2
#define LEN_D14 2
#define LEN_D15 2
#define LEN_D16 2
#define LEN_D17 2
#define LEN_D18 2
#define LEN_D19 2
#define LEN_D20 2
#define LEN_D21 2
#define LEN_D22 2
#define LEN_D23 2
#define LEN_D24 2
#define LEN_D25 1
#define LEN_D26 1
#define LEN_D27 1
#define LEN_D28 4
#define LEN_D29 4
#define LEN_D30 4
#define LEN_D31 4
#define LEN_D37 12
#define LEN_D38 6
#define LEN_D39 2
#define LEN_D40 3
#define LEN_D41 8
#define LEN_D42 15
#define LEN_D43 40
#define LEN_D49 3
#define LEN_D50 3
#define LEN_D51 3
#define LEN_D52 8
#define LEN_D53 8
#define LEN_D64 2

int pack(IsoPkt *isoPkt, unsigned char *pkt);
short unpack(unsigned char *pkt, int pktLen, IsoPkt *isoPkt);
int parserTest(void);

static int File_Log(const char* format, ...);
void strn2BCD(unsigned char *bcd, unsigned char *data, int datalen, int BCDlen);
void BCDn2str(char *data, char *bcd, int BCDlen, int datalen,int numdigits);
void printHexDump(const char *title, const unsigned char *str, int len);

