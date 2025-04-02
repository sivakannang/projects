// IsoParser.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "stddef.h"
#include "stdarg.h"
#include "errno.h"
#include "Iso8583Parser.h"
//#include "windows.h"

char *padChar(char *src, int pLen, int ch)
{

        int sLen, dir;
        char *p = src;

        dir = (pLen < 0) ? 'l' : 'r';
        if ( dir == 'l' )
                pLen = -pLen;

        src[pLen] = 0;
        sLen = strlen(src);

        if(sLen >= pLen)
                return src;

        if (dir == 'l')
                memmove(src + pLen - sLen, src, sLen);
        else
                (p+=sLen);

        memset(p, ch, pLen - sLen);

        return src;

}

int pack(IsoPkt *isoPkt, unsigned char *pkt)
{
    	int offset = 0, bcdLen = 0, asciiLen = 0;
	unsigned char bitmap[17] = {0};
	unsigned char tempBuffer[1001] = {'\0'};
	unsigned char lenBuffer[11] = {'\0'};
	short bitMapLen = isoPkt->isSecondaryBitMapSet ? 16 : 8;
	short headerLen = 0;

	offset += 2; // Length
	offset += headerLen; // Header
	offset += 2; // MTI
	offset += bitMapLen; // Bit Map

	File_Log("\n----------------- Packing---------------------");

	/************** Field 1  Parsing **********************/

	if ( *(isoPkt->D1)  )
	{
		SETBIT(bitmap[0], BIT1);
	}

	/************** Field 2  ( n ..19 - Primary Account Number ) **********************/

	if ( *(isoPkt->D2)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D2);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		strn2BCD(pkt+offset, isoPkt->D2, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[0], BIT2);

	}
	

	/************** Field 3 ( n 6 - Processing code) **********************/
	
	if ( *(isoPkt->D3)  )
	{
		asciiLen = strlen((char *)isoPkt->D3);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, isoPkt->D3, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[0], BIT3);
	}
	

	/************** Field 4  ( n 12 - Amount, transaction ) **********************/

	if ( *(isoPkt->D4)  )
	{
        	memset(tempBuffer, 0, sizeof(tempBuffer));
    		strcpy((char *)tempBuffer, (char *)isoPkt->D4);
        	padChar((char *)tempBuffer, -LEN_D4*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[0], BIT4);
	}

	/************** Field 5 ( n 12 - Amount, settlement ) **********************/

	if ( *(isoPkt->D5)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D5);
        	padChar((char *)tempBuffer, -LEN_D5*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[0], BIT5);
	}

	/************** Field 6  ( n 12 - Amount, cardholder billing ) **********************/

	if ( *(isoPkt->D6)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D6);
		padChar((char *)tempBuffer, -LEN_D6*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[0], BIT6);
	}

	/************** Field 7  ( n 10 - Transmission date & time) **********************/

	if ( *(isoPkt->D7)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D7);
        	padChar((char *)tempBuffer, -LEN_D7*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[0], BIT7);
	}

	/************** Field 8  ( n 8 - Amount, cardholder billing fee) **********************/

	if ( *(isoPkt->D8)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D8);
        	padChar((char *)tempBuffer, -LEN_D8*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[0], BIT8);
	}


	/************** Field 9  ( n 8 - Conversion rate, settlement ) **********************/

	if ( *(isoPkt->D9)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D9);
        	padChar((char *)tempBuffer, -LEN_D9*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT1);
	}

	/************** Field 10  ( n 8 - Conversion rate, cardholder billing ) **********************/

	if ( *(isoPkt->D10)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D10);
        	padChar((char *)tempBuffer, -LEN_D10*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT2);
	}

	/************** Field 11  ( n 6 - Systems trace audit number ) **********************/

	if ( *(isoPkt->D11)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D11);
        	padChar((char *)tempBuffer, -LEN_D11*2, '0');
        
		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT3);
	}

	/************** Field 12  ( n 6 - Time, local transaction (hhmmss) ) **********************/

	if ( *(isoPkt->D12)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D12);
        	padChar((char *)tempBuffer, -LEN_D12*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT4);
	}

	/************** Field 13  ( n 4 -Date, local transaction (MMDD) ) **********************/

	if ( *(isoPkt->D13)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D13);
        	padChar((char *)tempBuffer, -LEN_D13*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT5);
	}

	/************** Field 14  ( n 4 - Date, expiration ) **********************/

	if ( *(isoPkt->D14)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D14);
        	padChar((char *)tempBuffer, -LEN_D14*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT6);
	}

	/************** Field 15  ( n 4 - Date, settlement ) **********************/

	if ( *(isoPkt->D15)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D15);
        	padChar((char *)tempBuffer, -LEN_D15*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT7);
	}

	/************** Field 16  ( n 4 - Date, conversion ) **********************/

	if ( *(isoPkt->D16)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D16);
        	padChar((char *)tempBuffer, -LEN_D16*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[1], BIT8);
	}
	
	/************** Field 17  ( n 4 - Date, capture ) *********************/

	if ( *(isoPkt->D17)  )
	{
	    	memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D17);
        	padChar((char *)tempBuffer, -LEN_D17*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT1);
	}


	/************** Field 18  ( n 4 - Merchant type ) **********************/

	if ( *(isoPkt->D18)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D18);
        	padChar((char *)tempBuffer, -LEN_D18*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT2);
	}

	/************** Field 19  ( n 3 - Acquiring institution country code ) **********************/

	if ( *(isoPkt->D19)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D19);
        	padChar((char *)tempBuffer, -LEN_D19*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT3);
	}

	/************** Field 20  ( n 3 - PAN extended, country code ) **********************/

	if ( *(isoPkt->D20)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D20);
        	padChar((char *)tempBuffer, -LEN_D20*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT4);
	}

	/************** Field 21  ( n 3 - Forwarding institution. country code ) **********************/

	if ( *(isoPkt->D21)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D21);
        	padChar((char *)tempBuffer, -LEN_D21*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT5);
	}

	/************** Field 22  ( n 3 - Point of service entry mode ) **********************/

	if ( *(isoPkt->D22)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D22);
        	padChar((char *)tempBuffer, -LEN_D22*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT6);
	}

	/************** Field 23  ( n 3 - Application PAN number ) **********************/

	if ( *(isoPkt->D23)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D23);
        	padChar((char *)tempBuffer, -LEN_D23*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT7);
	}

	/************** Field 24  ( n 3 - Function code (ISO 8583:1993)/Network International identifier (NII) ) **********************/

	if ( *(isoPkt->D24)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D24);
        	padChar((char *)tempBuffer, -LEN_D24*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[2], BIT8);
	}

	/************** Field 25  ( n 2 - Point of service condition code ) **********************/

	if ( *(isoPkt->D25)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D25);
        	padChar((char *)tempBuffer, -LEN_D25*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT1);
	}

	/************** Field 26  ( n 2 - Point of service capture code ) **********************/

	if ( *(isoPkt->D26)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D26);
        	padChar((char *)tempBuffer, -LEN_D26*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT2);
	}

	/************** Field 27  ( n 1 - Authorizing identification response asciiLength ) / Axis **********************/

	if ( *(isoPkt->D27)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D27);
        	padChar((char *)tempBuffer, -LEN_D27*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT3);
	}

	/************** Field 28  ( n 8 - Amount, transaction fee ) / SBI **********************/

	if ( *(isoPkt->D28)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D28);
        	padChar((char *)tempBuffer, -LEN_D28*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT4);
	}

	/************** Field 29  ( n 8 - Amount, settlement fee ) / HDFC **********************/

	if ( *(isoPkt->D29)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D29);
        	padChar((char *)tempBuffer, -LEN_D29*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT5);
	}

	/************** Field 30  ( n 8 - Amount, transaction processing fee )  / Citi **********************/

	if ( *(isoPkt->D30)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D30);
        	padChar((char *)tempBuffer, -LEN_D30*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT6);
	}

	/************** Field 31  ( n 8 - Amount, settlement processing fee ) / ICICI **********************/

	if ( *(isoPkt->D31)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D31);
        	padChar((char *)tempBuffer, -LEN_D31*2, '0');
        
    		asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT7);
	}

	/************** Field 32  ( n ..11 - Acquiring institution identification code ) / uniPAY **********************/

	if ( *(isoPkt->D32)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D32);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		strn2BCD(pkt+offset, isoPkt->D32, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[3], BIT8);

	}

	/************** Field 33  ( n ..11 - Forwarding institution identification code ) **********************/

	if ( *(isoPkt->D33)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D33);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		strn2BCD(pkt+offset, isoPkt->D33, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[4], BIT1);

	}

	/************** Field 34  ( ns ..28 - Primary account number, extended ) **********************/

	if ( *(isoPkt->D34)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
    		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D34);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		memcpy(pkt+offset, isoPkt->D34, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[4], BIT2);

	}


	/************** Field 35  ( z ..37 - Track 2 data ) **********************/

	if ( *(isoPkt->D35)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D35);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		strn2BCD(pkt+offset, isoPkt->D35, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[4], BIT3);

	}

	/************** Field 36  ( n ..104 - Track 3 data ) **********************/

	if ( *(isoPkt->D36)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D36);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		strn2BCD(pkt+offset, isoPkt->D36, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[4], BIT4);

	}


	/************** Field 37  ( an 12 - Retrieval reference number ) **********************/

	if ( *(isoPkt->D37)  )
	{
        	memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D37);
        	padChar((char *)tempBuffer, -LEN_D37, '0');
        	asciiLen = strlen((char *)tempBuffer);
		memcpy(pkt+offset, tempBuffer, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[4], BIT5);
	}

	/************** Field 38  ( an 6 - Authorization identification response ) **********************/

	if ( *(isoPkt->D38)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D38);
        	padChar((char *)tempBuffer, -LEN_D38, '0');
        	asciiLen = strlen((char *)tempBuffer);
    		memcpy(pkt+offset, tempBuffer, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[4], BIT6);
	}

	/************** Field 39  ( an 2 - Response code ) **********************/

	if ( *(isoPkt->D39)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D39);
        	padChar((char *)tempBuffer, -LEN_D39, '0');
        	asciiLen = strlen((char *)tempBuffer);
    		memcpy(pkt+offset, tempBuffer, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[4], BIT7);
	}

	/************** Field 40  ( an 3 - Service restriction code ) **********************/

	if ( *(isoPkt->D40)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D40);
        	padChar((char *)tempBuffer, -LEN_D40, '0');
        	asciiLen = strlen((char *)tempBuffer);
    		memcpy(pkt+offset, tempBuffer, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[4], BIT8);
	}
	
	/************** Field 41  ( ans 16 - Card acceptor terminal identification ) **********************/

	if ( *(isoPkt->D41)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D41);
        	padChar((char *)tempBuffer, -LEN_D41, '0');
        	asciiLen = strlen((char *)tempBuffer);
        	memcpy(pkt+offset, tempBuffer, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[5], BIT1);
	}

	/************** Field 42  ( ans 15 - Card acceptor identification code ) **********************/

	if ( *(isoPkt->D42)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D42);
        	padChar((char *)tempBuffer, -LEN_D42, '0');
        	asciiLen = strlen((char *)tempBuffer);
        	memcpy(pkt+offset, tempBuffer, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[5], BIT2);
	}
	
	/************** Field 43  ( ans 40 - Card acceptor name/location (1-23 address 24-36 city 37-38 state 39-40 country)  ) **********************/

	if ( *(isoPkt->D43)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D43);
        	padChar((char *)tempBuffer, -LEN_D43, '0');
        	asciiLen = strlen((char *)tempBuffer);
        	memcpy(pkt+offset, tempBuffer, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[5], BIT3);
	}
	
	/************** Field 44  ( an ..25 - Additional response data ) **********************/

	if ( *(isoPkt->D44)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D44);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		memcpy(pkt+offset, isoPkt->D44, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[5], BIT4);

	}

	/************** Field 45  ( an ..76 -Track 1 data ) **********************/

	if ( *(isoPkt->D45)  )
	{

		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D45);
		sprintf((char *)lenBuffer, "%02d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 1);
		offset += 1;

		memcpy(pkt+offset, isoPkt->D45, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[5], BIT5);

	}

	/************** Field 46  ( an ...999  - Additional data - ISO ) **********************/

	if ( *(isoPkt->D46)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
    		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D46);
		sprintf((char *)lenBuffer, "%04d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;
        
        	memcpy(pkt+offset, isoPkt->D46, asciiLen);
    		offset += asciiLen;
		SETBIT(bitmap[5], BIT6);
	}

	
	/************** Field 47  ( an ...999 - Additional data - national ) **********************/

	if ( *(isoPkt->D47)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D47);
		sprintf((char *)lenBuffer, "%04d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D47, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[5], BIT7);
	}

	/************** Field 48  ( an ...999 - Additional data - private ) **********************/

	if ( *(isoPkt->D48)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		asciiLen = strlen((char *)isoPkt->D48);
		sprintf((char *)lenBuffer, "%04d", asciiLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D48, asciiLen);
		offset += asciiLen;
		SETBIT(bitmap[5], BIT8);
	}


	/************** Field 49  ( a or n 3 - Currency code, transaction ) **********************/

	if ( *(isoPkt->D49)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D49);
        	padChar((char *)tempBuffer, -LEN_D49, '0');
        	asciiLen = strlen((char *)tempBuffer);
        	memcpy(pkt+offset, tempBuffer, asciiLen);
    		offset += asciiLen;
		SETBIT(bitmap[6], BIT1);
	}

	/************** Field 50  ( a or n 3 - Currency code, settlement, , using as an 3 )***********/

	if ( *(isoPkt->D50)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D50);
        	padChar((char *)tempBuffer, -LEN_D50, '0');
        	asciiLen = strlen((char *)tempBuffer);
        	memcpy(pkt+offset, tempBuffer, asciiLen);
    		offset += asciiLen;
		SETBIT(bitmap[6], BIT2);
	}

	/************** Field 51  ( a or n 3 - Currency code, cardholder billing, using as an 3 ) *****/

	if ( *(isoPkt->D51)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D51);
        	padChar((char *)tempBuffer, -LEN_D51, '0');
        	asciiLen = strlen((char *)tempBuffer);
        	memcpy(pkt+offset, tempBuffer, asciiLen);
    		offset += asciiLen;
		SETBIT(bitmap[6], BIT3);
	}

	/************** Field 52  ( b 64 - Personal identification number data, using as an 3 ) ********/

	if ( *(isoPkt->D52)  )
	{
		bcdLen = LEN_D52;
		memcpy(pkt+offset, isoPkt->D52, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[6], BIT4);
	}



	/************** Field 53  ( n 16 - Security related control information ) **********************/

	if ( *(isoPkt->D53)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
        	strcpy((char *)tempBuffer, (char *)isoPkt->D53);
        	padChar((char *)tempBuffer, -LEN_D53*2, '0');
        
        	asciiLen = strlen((char *)tempBuffer);
		bcdLen = (asciiLen/2)+(asciiLen%2);
		strn2BCD(pkt+offset, tempBuffer, asciiLen, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[6], BIT5);
	}

	/************** Field 54  ( an ...120 - Additional amounts ) **********************/

	if ( *(isoPkt->D54)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D54);
		sprintf((char *)lenBuffer, "%02d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 2, 1);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 1;

		memcpy(pkt+offset, isoPkt->D54, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[6], BIT6);
	}

	/************** Field 55  ( an ...999 - Reserved ISO ) **********************/

	if ( *(isoPkt->D55)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D55);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D55, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[6], BIT7);
	}

	/************** Field 56  ( ans ...999 - Reserved ISO ) **********************/

	if ( *(isoPkt->D56)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D56);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D56, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[6], BIT8);
	}


	/************** Field 57  ( ans ...999 - Reserved national ) **********************/

	if ( *(isoPkt->D57)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D57);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D57, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT1);
	}

	/************** Field 58  ( ans ...999 - Reserved national ) **********************/

	if ( *(isoPkt->D58)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D58);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D58, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT2);
	}

	/************** Field 59  ( ans ...999 - Reserved national use ) **********************/

	if ( *(isoPkt->D59)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D59);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D59, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT3);
	}

	/************** Field 60  ( ans ...999 - Reserved national ) ***********/

	if ( *(isoPkt->D60)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D60);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D60, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT4);
	}

	/************** Field 61  ( ans ...999 - Reserved private ) **********************/

	if ( *(isoPkt->D61)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D61);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D61, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT5);
	}

	/************** Field 62  ( ans ...999 - Reserved private ) **********************/

	if ( *(isoPkt->D62)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D62);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D62, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT6);
	}

	/************** Field 63  ( ans ...999 - Reserved private ) **********************/

	if ( *(isoPkt->D63)  )
	{
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		bcdLen = strlen((char *)isoPkt->D63);
		sprintf((char *)lenBuffer, "%04d", bcdLen);
		strn2BCD(tempBuffer, lenBuffer, 4, 2);
		memcpy(pkt+offset, tempBuffer, 2);
		offset += 2;

		memcpy(pkt+offset, isoPkt->D63, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT7);
	}

	/************** Field 64  ( b 16 - Message authentication code (MAC) ) **********************/

	if ( *(isoPkt->D64) )
	{
		bcdLen = LEN_D64;
		memcpy(pkt+offset, isoPkt->D64, bcdLen);
		offset += bcdLen;
		SETBIT(bitmap[7], BIT8);
	}


	/****************************************  Setting Length ***********************************/

	memset(tempBuffer, 0, sizeof(tempBuffer));
	memset(lenBuffer, 0, sizeof(lenBuffer));
	sprintf((char *)lenBuffer, "%04d", offset);
	strn2BCD(tempBuffer, lenBuffer, 4, 2);
	memcpy(pkt, tempBuffer, 2);
	isoPkt->pktLen = offset;

	/****************************************  Setting Header ***********************************/

	if ( headerLen > 0)
	{
		strn2BCD(tempBuffer, isoPkt->header, 10, 5);
		memcpy(pkt+2, tempBuffer, 5);
	}

	/****************************************  Setting MTI    ***********************************/

	memset(tempBuffer, 0, sizeof(tempBuffer));
	memset(lenBuffer, 0, sizeof(lenBuffer));
	strncpy((char *)lenBuffer, (char *)isoPkt->MTI, 4);
	strn2BCD(tempBuffer, lenBuffer, 4, 2);
	memcpy(pkt+2+headerLen, tempBuffer, 2);

	/****************************************  Setting BitMap ***********************************/

	memcpy(pkt+2+headerLen+2, bitmap, bitMapLen);
	memcpy(isoPkt->BITMAP, bitmap, bitMapLen);

	/****************************************  Completion of Packing ***********************************/

	File_Log("Packet Length  : %d", isoPkt->pktLen);

	printHexDump("Raw Packet    ", pkt, isoPkt->pktLen);	

	return offset;
}

short unpack(unsigned char *pkt, int pktLen, IsoPkt *isoPkt)
{
	
	int offset = 0, bcdLen = 0, asciiLen = 0;
	unsigned char bitmap[17] = {0};
	char tempBuffer[1001] = {'\0'};
	char lenBuffer[11] = {'\0'};
	short bitMapLen = 0, headerLen = 0;

	// Parsing Length

	memcpy(lenBuffer, pkt+offset, 2);
	BCDn2str(tempBuffer, lenBuffer, 2, 4, 4);
	isoPkt->pktLen = atoi(tempBuffer);
	memset(tempBuffer, 0, sizeof(tempBuffer));
	memset(lenBuffer, 0, sizeof(lenBuffer));
	offset += 2;

	File_Log("\n----------------- Unpacking---------------------");

	File_Log("Packet Length  : %d", isoPkt->pktLen);

	printHexDump("Raw Packet    ", pkt, pktLen);

	

	// Parsing Header
	if ( headerLen > 0)
	{
		memcpy(lenBuffer, pkt+offset, 5);
		BCDn2str(tempBuffer, lenBuffer, 5, 10, 10);
		strncpy((char *)isoPkt->header, tempBuffer, strlen(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memset(tempBuffer, 0, sizeof(tempBuffer));
		offset += 5;
		File_Log("Packet Header  : %s", isoPkt->header);
	}

	//Parsing MTI
	memcpy(lenBuffer, pkt+offset, 2);
	BCDn2str(tempBuffer, lenBuffer, 2, 4, 4);
	strncpy((char *)isoPkt->MTI, tempBuffer, strlen(tempBuffer));
	memset(lenBuffer, 0, sizeof(lenBuffer));
	memset(tempBuffer, 0, sizeof(tempBuffer));
	offset += 2;
	File_Log("Packet MTI     : %s", isoPkt->MTI);


	bitMapLen = ( TESTBIT(bitmap[0], BIT1) ) ? 16 : 8;

	//D1 Parsing - BitMap
	memcpy((char *)isoPkt->D1, pkt+offset, bitMapLen);
	memcpy(bitmap, pkt+offset, bitMapLen);
        memcpy(isoPkt->BITMAP, bitmap, bitMapLen);	
	offset += bitMapLen;
	File_Log("Packet BitMap  : %02x %02x %02x %02x %02x %02x %02x %02x", bitmap[0], bitmap[1], bitmap[2], bitmap[3], bitmap[4], bitmap[5], bitmap[6], bitmap[7]);


	/************** Field 1  Parsing **********************/

	if ( TESTBIT(bitmap[0], BIT1) )
	{
	}

	/************** Field 2  ( n ..19 - Primary Account Number ) **********************/

	if ( TESTBIT(bitmap[0], BIT2) )
	{
		bcdLen = 1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		asciiLen = atoi((char *)tempBuffer);
		bcdLen = (asciiLen / 2) + (asciiLen % 2);
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D2, (char *)tempBuffer, bcdLen, asciiLen, asciiLen);
		offset += bcdLen;
		File_Log("D2             : %s", isoPkt->D2);
	}

	/************** Field 3 ( n 6 - Processing code) **********************/

	if ( TESTBIT(bitmap[0], BIT3) )
	{
		bcdLen = LEN_D3;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D3, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D3             : %s", isoPkt->D3);
	}

	/************** Field 4  ( n 12 - Amount, transaction ) **********************/

	if ( TESTBIT(bitmap[0], BIT4) )
	{
		bcdLen = LEN_D4;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D4, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D4             : %s", isoPkt->D4);
	}
	
	/************** Field 5 ( n 12 - Amount, settlement ) **********************/

	if ( TESTBIT(bitmap[0], BIT5) )
	{
		bcdLen = LEN_D5;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D5, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D5             : %s", isoPkt->D5);
	}


	/************** Field 6  ( n 12 - Amount, cardholder billing ) **********************/

	if ( TESTBIT(bitmap[0], BIT6) )
	{
		bcdLen = LEN_D6;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D6, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D6             : %s", isoPkt->D6);
	}

	/************** Field 7  ( n 10 - Transmission date & time) **********************/

	if ( TESTBIT(bitmap[0], BIT7) )
	{
		bcdLen = LEN_D7;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D7, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D7             : %s", isoPkt->D7);

	}


	/************** Field 8  ( n 8 - Amount, cardholder billing fee) **********************/

	if ( TESTBIT(bitmap[0], BIT8) )
	{
		bcdLen = LEN_D8;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D8, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D8             : %s", isoPkt->D8);
	}



	/************** Field 9  ( n 8 - Conversion rate, settlement ) **********************/

	if ( TESTBIT(bitmap[1], BIT1) )
	{
		bcdLen = LEN_D9;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D9, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D9             : %s", isoPkt->D9);
	}

	/************** Field 10  ( n 8 - Conversion rate, cardholder billing ) **********************/

	if ( TESTBIT(bitmap[1], BIT2) )
	{
		bcdLen = LEN_D10;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D10, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D10            : %s", isoPkt->D10);
	}

	/************** Field 11  ( n 6 - Systems trace audit number ) **********************/

	if ( TESTBIT(bitmap[1], BIT3) )
	{
		bcdLen = LEN_D11;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D11, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D11            : %s", isoPkt->D11);

	}

	/************** Field 12  ( n 6 - Time, local transaction (hhmmss) ) **********************/

	if ( TESTBIT(bitmap[1], BIT4) )
	{
		bcdLen = LEN_D12;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D12, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D12            : %s", isoPkt->D12);
	}

	/************** Field 13  ( n 4 -Date, local transaction (MMDD) ) **********************/

	if ( TESTBIT(bitmap[1], BIT5) )
	{
		bcdLen = LEN_D13;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D13, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D13            : %s", isoPkt->D13);
	}

	/************** Field 14  ( n 4 - Date, expiration ) **********************/

	if ( TESTBIT(bitmap[1], BIT6) )
	{
		bcdLen = LEN_D14;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D14, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D14            : %s", isoPkt->D14);

	}

	/************** Field 15  ( n 4 - Date, settlement ) **********************/

	if ( TESTBIT(bitmap[1], BIT7) )
	{
		bcdLen = LEN_D15;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D15, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D15            : %s", isoPkt->D15);
	}

	/************** Field 16  ( n 4 - Date, conversion ) **********************/

	if ( TESTBIT(bitmap[1], BIT8) )
	{
		bcdLen = LEN_D16;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D16, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D16            : %s", isoPkt->D16);
	}


	/************** Field 17  ( n 4 - Date, capture ) *********************/

	if ( TESTBIT(bitmap[2], BIT1) )
	{
		bcdLen = LEN_D17;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D17, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D17            : %s", isoPkt->D17);
	}


	/************** Field 18  ( n 4 - Merchant type ) **********************/

	if ( TESTBIT(bitmap[2], BIT2) )
	{
		bcdLen = LEN_D18;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D18, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D18            : %s", isoPkt->D18);
	}

	/************** Field 19  ( n 3 - Acquiring institution country code ) **********************/

	if ( TESTBIT(bitmap[2], BIT3) )
	{
		bcdLen = LEN_D19;
		asciiLen = (bcdLen*2)-1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D19, (char *)tempBuffer, bcdLen, asciiLen, asciiLen);
		padChar((char *)isoPkt->D19, 3, '\0');
		offset += bcdLen;
		File_Log("D19            : %s", isoPkt->D19);
	}

	/************** Field 20  ( n 3 - PAN extended, country code ) **********************/

	if ( TESTBIT(bitmap[2], BIT4) )
	{
		bcdLen = LEN_D20;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D20, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		padChar((char *)isoPkt->D20, 3, '\0');
		offset += bcdLen;
		File_Log("D20            : %s", isoPkt->D20);
	}

	/************** Field 21  ( n 3 - Forwarding institution. country code ) **********************/

	if ( TESTBIT(bitmap[2], BIT5) )
	{
		bcdLen = LEN_D21;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D21, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		padChar((char *)isoPkt->D21, 3, '\0');
		offset += bcdLen;
		File_Log("D21            : %s", isoPkt->D21);
	}

	/************** Field 22  ( n 3 - Point of service entry mode ) **********************/

	if ( TESTBIT(bitmap[2], BIT6) )
	{
		bcdLen = LEN_D22;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D22, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		//padChar((char *)isoPkt->D22, 3, '\0');
		offset += bcdLen;
		File_Log("D22            : %s", isoPkt->D22);
	}

	/************** Field 23  ( n 3 - Application PAN number ) **********************/

	if ( TESTBIT(bitmap[2], BIT7) )
	{
		bcdLen = LEN_D23;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D23, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		//padChar((char *)isoPkt->D23, 3, '\0');
		offset += bcdLen;
		File_Log("D23            : %s", isoPkt->D23);
	}

	/************** Field 24  ( n 3 - Function code (ISO 8583:1993)/Network International identifier (NII) ) **********************/

	if ( TESTBIT(bitmap[2], BIT8) )
	{
		bcdLen = LEN_D24;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D24, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		//padChar((char *)isoPkt->D24, 3, '\0');
		offset += bcdLen;
		File_Log("D24            : %s", isoPkt->D24);
	}


	/************** Field 25  ( n 2 - Point of service condition code ) **********************/

	if ( TESTBIT(bitmap[3], BIT1) )
	{
		bcdLen = LEN_D25;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D25, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D25            : %s", isoPkt->D25);
	}


	/************** Field 26  ( n 2 - Point of service capture code ) **********************/

	if ( TESTBIT(bitmap[3], BIT2) )
	{
		bcdLen = LEN_D26;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D26, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D26            : %s", isoPkt->D26);

	}
	

	/************** Field 27  ( n 1 - Authorizing identification response asciiLength ) **********************/

	if ( TESTBIT(bitmap[3], BIT3) )
	{
		bcdLen = LEN_D27;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D27, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		//padChar((char *)isoPkt->D27, 1, '\0');
		offset += bcdLen;
		File_Log("D27            : %s", isoPkt->D27);
	}


	/************** Field 28  ( n 8 - Amount, transaction fee ) **********************/

	if ( TESTBIT(bitmap[3], BIT4) )
	{
		bcdLen = LEN_D28;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D28, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D28            : %s", isoPkt->D28);
	}


	/************** Field 29  ( n 8 - Amount, settlement fee ) **********************/

	if ( TESTBIT(bitmap[3], BIT5) )
	{
		bcdLen = LEN_D29;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D29, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D29            : %s", isoPkt->D29);
	}


	/************** Field 30  ( n 8 - Amount, transaction processing fee ) **********************/

	if ( TESTBIT(bitmap[3], BIT6) )
	{
		bcdLen = LEN_D30;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D30, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D30            : %s", isoPkt->D30);
	}


	/************** Field 31  ( n 8 - Amount, settlement processing fee ) **********************/

	if ( TESTBIT(bitmap[3], BIT7) )
	{
		bcdLen = LEN_D31;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D31, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D31            : %s", isoPkt->D31);
	}


	/************** Field 32  ( n ..11 - Acquiring institution identification code ) **********************/

	if ( TESTBIT(bitmap[3], BIT8) )
	{
		bcdLen = 1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		asciiLen = atoi((char *)tempBuffer);
		bcdLen = (asciiLen / 2) + (asciiLen % 2);
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D32, (char *)tempBuffer, bcdLen, asciiLen, asciiLen);
		offset += bcdLen;
		File_Log("D32            : %s", isoPkt->D32);
	}


	/************** Field 33  ( n ..11 - Forwarding institution identification code ) **********************/

	if ( TESTBIT(bitmap[4], BIT1) )
	{
		bcdLen = 1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		asciiLen = atoi((char *)tempBuffer);
		bcdLen = (asciiLen / 2) + (asciiLen % 2);
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D33, (char *)tempBuffer, bcdLen, asciiLen, asciiLen);
		offset += bcdLen;
		File_Log("D33            : %s", isoPkt->D33);
	}

	/************** Field 34  ( ns ..28 - Primary account number, extended ) **********************/

	if ( TESTBIT(bitmap[4], BIT2) )
	{
		bcdLen = 1;
    		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D34, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D34            : %s", isoPkt->D34);
	}

	/************** Field 35  ( z ..37 - Track 2 data ) **********************/

	if ( TESTBIT(bitmap[4], BIT3) )
	{
		bcdLen = 1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		asciiLen = atoi((char *)tempBuffer);
		bcdLen = (asciiLen / 2) + (asciiLen % 2);
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D35, (char *)tempBuffer, bcdLen, asciiLen, asciiLen);
		offset += bcdLen;
		File_Log("D35            : %s", isoPkt->D35);

	}

	/************** Field 36  ( n ..104 - Track 3 data ) **********************/

	if ( TESTBIT(bitmap[4], BIT4) )
	{
		bcdLen = 1;
    		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		asciiLen = atoi((char *)tempBuffer);
		bcdLen = (asciiLen / 2) + (asciiLen % 2);
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D36, (char *)tempBuffer, bcdLen, asciiLen, asciiLen);
		offset += bcdLen;
		File_Log("D36            : %s", isoPkt->D36);

	}

	/************** Field 37  ( an 12 - Retrieval reference number ) **********************/

	if ( TESTBIT(bitmap[4], BIT5) )
	{
		asciiLen = LEN_D37; 
		memcpy((char *)isoPkt->D37, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D37            : %s", isoPkt->D37);

	}

	/************** Field 38  ( an 6 - Authorization identification response ) **********************/

	if ( TESTBIT(bitmap[4], BIT6) )
	{
		asciiLen = LEN_D38; 
		memcpy((char *)isoPkt->D38, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D38            : %s", isoPkt->D38);
	}

	/************** Field 39  ( an 2 - Response code ) **********************/

	if ( TESTBIT(bitmap[4], BIT7) )
	{
		asciiLen = LEN_D39; 
		memcpy((char *)isoPkt->D39, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D39            : %s", isoPkt->D39);
	}


	/************** Field 40  ( an 3 - Service restriction code ) **********************/

	if ( TESTBIT(bitmap[4], BIT8) )
	{
		asciiLen = LEN_D40; 
		memcpy((char *)isoPkt->D40, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D40            : %s", isoPkt->D40);
	}

	

	/************** Field 41  ( ans 16 - Card acceptor terminal identification ) **********************/


	if ( TESTBIT(bitmap[5], BIT1) )
	{
		asciiLen = LEN_D41; 
		memcpy((char *)isoPkt->D41, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D41            : %s", isoPkt->D41);
	}

		
	/************** Field 42  ( ans 15 - Card acceptor identification code ) **********************/

	if ( TESTBIT(bitmap[5], BIT2) )
	{
		asciiLen = LEN_D42; 
		memcpy((char *)isoPkt->D42, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D42            : %s", isoPkt->D42);
	}

	
	/************** Field 43  ( ans 40 - Card acceptor name/location (1-23 address 24-36 city 37-38 state 39-40 country)  ) **********************/

	if ( TESTBIT(bitmap[5], BIT3) )
	{
		asciiLen = 40; 
		memcpy((char *)isoPkt->D43, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D43            : %s", isoPkt->D43);
	}

	
	/************** Field 44  ( an ..25 - Additional response data ) **********************/

	if ( TESTBIT(bitmap[5], BIT4) )
	{
		bcdLen = 1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D44, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D44            : %s", isoPkt->D44);
	}

	
	/************** Field 45  ( an ..76 -Track 1 data ) **********************/

	if ( TESTBIT(bitmap[5], BIT5) )
	{
		bcdLen = 1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D45, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D45            : %s", isoPkt->D45);
	}

	
	/************** Field 46  ( an ...999  - Additional data - ISO ) **********************/

	if ( TESTBIT(bitmap[5], BIT6) )
	{
		bcdLen = 2;
    		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D46, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D46            : %s", isoPkt->D46);
	}

	
	/************** Field 47  ( an ...999 - Additional data - national ) **********************/

	if ( TESTBIT(bitmap[5], BIT7) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D47, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D47            : %s", isoPkt->D47);
	}

		
	/************** Field 48  ( an ...999 - Additional data - private ) **********************/

	if ( TESTBIT(bitmap[5], BIT8) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D48, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D48            : %s", isoPkt->D48);
	}

	/************** Field 49  ( a or n 3 - Currency code, transaction , using as an 3 ) ***********/

	if ( TESTBIT(bitmap[6], BIT1) )
	{	
		asciiLen = LEN_D49; 
    		memcpy((char *)isoPkt->D49, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D49            : %s", isoPkt->D49);
	}
    

	/************** Field 50  ( a or n 3 - Currency code, settlement, using as an 3 ) **********/

	if ( TESTBIT(bitmap[6], BIT2) )
	{
		asciiLen = LEN_D50; 
		memcpy((char *)isoPkt->D50, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D50            : %s", isoPkt->D50);
	}

	/************** Field 51  ( a or n 3 - Currency code, cardholder billing, using as an 3 ) ******/

	if ( TESTBIT(bitmap[6], BIT3) )
	{
		asciiLen = LEN_D51; 
		memcpy((char *)isoPkt->D51, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D51            : %s", isoPkt->D51);
	}

	/************** Field 52  ( b 64 - Personal identification number data ) **********************/

	if ( TESTBIT(bitmap[6], BIT4) )
	{
		asciiLen = LEN_D52; 
		memcpy((char *)isoPkt->D52, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D52            : %s", isoPkt->D52);
	}

	/************** Field 53  ( n 18 - Security related control information ) **********************/


	if ( TESTBIT(bitmap[6], BIT5) )
	{
		bcdLen = LEN_D53;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memcpy(tempBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)isoPkt->D53, (char *)tempBuffer, bcdLen, bcdLen*2, bcdLen*2);
		offset += bcdLen;
		File_Log("D53            : %s", isoPkt->D53);
	}

	/************** Field 54  ( an ...120 - Additional amounts ) **********************/

	if ( TESTBIT(bitmap[6], BIT6) )
	{
		bcdLen = 1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D54, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D54            : %s", isoPkt->D54);
	}

	/************** Field 55  ( an ...999 - Reserved ISO ) **********************/

	if ( TESTBIT(bitmap[6], BIT7) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D55, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D55            : %s", isoPkt->D55);
	}

	/************** Field 56  ( ans ...999 - Reserved ISO ) **********************/

	if ( TESTBIT(bitmap[6], BIT8) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D56, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D56            : %s", isoPkt->D56);
	}

	/************** Field 57  ( ans ...999 - Reserved national ) **********************/

	if ( TESTBIT(bitmap[7], BIT1) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D57, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D57            : %s", isoPkt->D57);
	}

	/************** Field 58  ( ans ...999 - Reserved national ) **********************/

	if ( TESTBIT(bitmap[7], BIT2) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D58, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D58            : %s", isoPkt->D58);
	}

	/************** Field 59  ( ans ...999 - Reserved national use ) **********************/

	if ( TESTBIT(bitmap[7], BIT3) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D59, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D59            : %s", isoPkt->D59);
	}

	/************** Field 60  ( ans ...999 - Reserved national  ) **********************/

	if ( TESTBIT(bitmap[7], BIT4) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D60, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D60            : %s", isoPkt->D60);
	}

	/************** Field 61  ( ans ...999 - Reserved private ) **********************/

	if ( TESTBIT(bitmap[7], BIT5) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D61, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D61            : %s", isoPkt->D61);
	}

	/************** Field 62  ( ans ...999 - Reserved private ) **********************/

	if ( TESTBIT(bitmap[7], BIT6) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D62, pkt+offset, bcdLen);
		offset += bcdLen;
		File_Log("D62            : %s", isoPkt->D62);

	}

	/************** Field 63  ( ans ...999 - Reserved private ) **********************/

	if ( TESTBIT(bitmap[7], BIT7) )
	{
		bcdLen = 2;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		memset(lenBuffer, 0, sizeof(lenBuffer));
		memcpy(lenBuffer, pkt+offset, bcdLen);
		BCDn2str((char *)tempBuffer, (char *)lenBuffer, bcdLen, bcdLen*2, bcdLen*2);
		tempBuffer[bcdLen*2]='\0';
		offset += bcdLen;

		bcdLen = atoi((char *)tempBuffer);
		memcpy(isoPkt->D63, pkt+offset, bcdLen);	
		offset += bcdLen;
		File_Log("D63            : %s", isoPkt->D63);

	}

	/************** Field 64  ( b 16 - Message authentication code (MAC) ) **********************/

	if ( TESTBIT(bitmap[7], BIT8) )
	{
		asciiLen = LEN_D64; 
		memcpy((char *)isoPkt->D64, pkt+offset, asciiLen);
		offset += asciiLen;
		File_Log("D64            : %s", isoPkt->D64);
	}    

	return 0;


}

void strn2BCD(unsigned char *bcd, unsigned char *data, int dataasciiLen, int BCDasciiLen)
{
	int i,rem,div;
	memset(bcd,0,BCDasciiLen);
	div = dataasciiLen / 2;
	rem = dataasciiLen % 2;
	for(i = 0;i < (div);i++)
	{
		bcd[BCDasciiLen - i - 1] = (((data[dataasciiLen - 2*i -1 ] - '0')) | ((data[dataasciiLen - (2*i) - 2] - '0') << 4));
	}
	if(rem != 0)
	{
		bcd[(BCDasciiLen - div) - 1] = (data[0] - '0');
	}
}

void BCDn2str(char *data, char *bcd, int BCDasciiLen, int dataasciiLen,int numdigits)
{
	// dataasciiLen >= (numdigits)
	int i;
	memset(data,0x30,dataasciiLen);
	for(i = 0;i<numdigits;i++)
	{
		if((i%2) == 0)
		{
			data[dataasciiLen - i - 1] =  (((bcd[BCDasciiLen - (i/2) - 1]) & 0x0F) + '0');

		}
		else
		{
			data[dataasciiLen - i - 1] =  (((bcd[BCDasciiLen - (i/2) - 1] >> 4) & 0x0F) + '0');
		}

	}

	
}

/*********************** Already Defined in userEntry.c file *********************************
 *
 *
int padChar(char *src, int len, const char ch)
{
	int dim;                    //the initial string asciiLength
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

*
*
************************************************************************************************/


static int File_Log(const char* format, ...)
{
	
	int ret=0;
	char buffer[10240] = {'\0'};
	char logfile[256] = {'\0'};

	va_list args;
	va_start(args, format);
	vsprintf (&buffer[strlen(buffer)], format, args );
	va_end(args);	

	sprintf(logfile, "%s", "log.txt");

	printf("\n%s", buffer);

	FILE *fp = fopen (logfile, "a");
        if ( fp == NULL )
		return 0;	

	ret = fwrite(buffer, strlen(buffer), 1, fp);
	fwrite("\n", 1, 1, fp);

	fclose(fp);

	return ret;


}

void printHexDump(const char *title, const unsigned char *str, int len)
{
	char hexData[1024 * 3] = {'\0'};

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

	File_Log(hexData);

}

int parserTest(void)
{
	short len = 0;
	IsoPkt reqPkt = {0};
	IsoPkt rspPkt = {0};
	unsigned char pkt[1025] ={0};

	strcpy((char *)reqPkt.header, "0001110222");
	strcpy((char *)reqPkt.MTI, "0200");
	strcpy((char *)reqPkt.D2,  "12345678901234567");
	strcpy((char *)reqPkt.D3,  "000000");
	strcpy((char *)reqPkt.D4,  "123456789012");
    	strcpy((char *)reqPkt.D5,  "878787878277");
    	strcpy((char *)reqPkt.D6,  "343434343434");
    	strcpy((char *)reqPkt.D7,  "5555555555");
    	strcpy((char *)reqPkt.D8,  "444441");
    	strcpy((char *)reqPkt.D9,  "1111121");
    	strcpy((char *)reqPkt.D10, "5555");
	strcpy((char *)reqPkt.D11, "22");
	strcpy((char *)reqPkt.D12, "101010");
	strcpy((char *)reqPkt.D13, "2103");
	strcpy((char *)reqPkt.D14, "1214");
	strcpy((char *)reqPkt.D22, "021");
	strcpy((char *)reqPkt.D23, "005");
	strcpy((char *)reqPkt.D24, "999");
	strcpy((char *)reqPkt.D25, "79");
	strcpy((char *)reqPkt.D26, "69");
    	strcpy((char *)reqPkt.D27, "7");
    	strcpy((char *)reqPkt.D28, "28");
    	strcpy((char *)reqPkt.D29, "2929");
    	strcpy((char *)reqPkt.D30, "303030");
    	strcpy((char *)reqPkt.D31, "31313131");
    	strcpy((char *)reqPkt.D32, "3232323");
    	strcpy((char *)reqPkt.D33, "33333");
    	strcpy((char *)reqPkt.D34, "Pan Extended");
	strcpy((char *)reqPkt.D35, "1234567890123456=12142061234567");
    	strcpy((char *)reqPkt.D36, "34512345678901234567890123456789012345678901234567890123456789012");
    	strcpy((char *)reqPkt.D37, "RRN NUMBER");
    	strcpy((char *)reqPkt.D38, "989897");
    	strcpy((char *)reqPkt.D39, "00");
    	strcpy((char *)reqPkt.D40, "SRC");
	strcpy((char *)reqPkt.D41, "41012001");
	strcpy((char *)reqPkt.D42, "123456789012345");
    	strcpy((char *)reqPkt.D43, "ADDRESS 1-23 address 24-36 city 37-38");
	strcpy((char *)reqPkt.D44, "Additional response data");
    	strcpy((char *)reqPkt.D45, "Track 1 data,  an ..76");
    	strcpy((char *)reqPkt.D46, "Additional data - ISO       an ...999");
    	strcpy((char *)reqPkt.D47, "Additional data - national  an ...999");
    	strcpy((char *)reqPkt.D48, "Additional data - private   an ...999");
    	strcpy((char *)reqPkt.D49, "D49"); // a or n 3
    	strcpy((char *)reqPkt.D50, "D50"); // a or n 3
	strcpy((char *)reqPkt.D51, "D51"); // a or n 3
	strcpy((char *)reqPkt.D52, "12345678");
	strcpy((char *)reqPkt.D53, "6543211234567890");
	strcpy((char *)reqPkt.D54, "Additional amounts");
    	strcpy((char *)reqPkt.D55, "Reserved ISO  E M V   D A T A");
    	strcpy((char *)reqPkt.D56, "Reserved ISO  DE56");
    	strcpy((char *)reqPkt.D57, "Reserved national  DE57");
    	strcpy((char *)reqPkt.D58, "Reserved national  DE58");
    	strcpy((char *)reqPkt.D59, "Reserved national  DE59");
    	strcpy((char *)reqPkt.D60, "Reserved national  DE60");
	strcpy((char *)reqPkt.D61, "Reserved Private DE61......................");
	strcpy((char *)reqPkt.D62, "Reserved Private DE61......................");
	strcpy((char *)reqPkt.D63, "Reserved Private DE61......................");
	

	// Packing
	len = pack(&reqPkt, pkt);	
	
	//Unpacking
	unpack(pkt, len, &rspPkt);

	printf("\n");


	return 0;
}


/*********************************************** Test Function for Iso8583 Parser *************************
 *
 */

int main()
{
	parserTest();
	return 0;
}

/*
*
* ********************************************************************************************************/

