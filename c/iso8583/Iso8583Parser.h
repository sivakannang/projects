#ifndef ISO8583_PARSER_H
#define ISO8583_PARSER_H

/*
 * The comments give the ISO 8583 type and length. Sizes are one larger for the NUL.
 * An empty string means the field is absent, except for binary D52 and D64: see the table in the .c file.
 *
 * Only fields 2 to 64 are implemented. unpack rejects any message with fields 65 to 128.
 */
typedef struct packetField
{
	unsigned short pktLen;
	unsigned short isSecondaryBitMapSet;
	unsigned char MTI[5];
	unsigned char BITMAP[17];
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
	unsigned char D28[9];    //n 8 Amount, transaction fee
	unsigned char D29[9];    //n 8 Amount, settlement fee
	unsigned char D30[9];    //n 8 Amount, transaction processing fee
	unsigned char D31[9];    //n 8 Amount, settlement processing fee
	unsigned char D32[13];   //n ..11 Acquiring institution identification code
	unsigned char D33[13];   //n ..11 Forwarding institution identification code
	unsigned char D34[29];   //n ..28 Primary account number, extended
	unsigned char D35[39];   //z ..37 Track 2 data
	unsigned char D36[105];  //n ...104 Track 3 data (this format's 1-byte length limits it to 99)
	unsigned char D37[13];   //an 12 Retrieval reference number
	unsigned char D38[7];    //an 6 Authorization identification response
	unsigned char D39[3];    //an 2 Response code
	unsigned char D40[4];    //an 3 Service restriction code
	unsigned char D41[17];   //ans 8 Card acceptor terminal identification
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
	unsigned char D53[19];   //n 16 Security related control information
	unsigned char D54[121];  //an ...120 Additional amounts (this format's 1-byte length limits it to 99)
	unsigned char D55[1000]; //ans ...999 Reserved ISO
	unsigned char D56[1000]; //ans ...999 Reserved ISO
	unsigned char D57[1000]; //ans ...999 Reserved national
	unsigned char D58[1000]; //ans ...999 Reserved national
	unsigned char D59[1000]; //ans ...999 Reserved national
	unsigned char D60[1000]; //ans ...999 Reserved national
	unsigned char D61[1000]; //ans ...999 Reserved private
	unsigned char D62[1000]; //ans ...999 Reserved private
	unsigned char D63[1000]; //ans ...999 Reserved private
	unsigned char D64[3];    //b 16 Message authentication code (MAC)
}IsoPkt;

// Returns the packet length, or -1 if a field is invalid or the packet doesn't fit in pktSize
int pack(IsoPkt *isoPkt, unsigned char *pkt, int pktSize);

// Returns 0, or -1 if the packet is malformed. Safe to call on untrusted data.
int unpack(const unsigned char *pkt, int pktLen, IsoPkt *isoPkt);

#endif
