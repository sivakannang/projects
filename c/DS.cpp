// DS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdlib.h"
#include "time.h"


void fill(int a[], int len);
void show(int a[], int len);
void bubbleSort(int a[], int len);
void bubbleSortOptimized(int a[], int len);
void selectionSort(int a[], int len);

#define SWAP(A,B,TEMP) (TEMP=A, A=B, B=TEMP)
#define SWAP_STR(A,B,TEMP) (strcpy(TEMP, A), strcpy(A,B), strcpy(B,TEMP))



int main(int argc, char *args[])
{
	int len = 6;
	int a[6] = {0};

	fill(a, len);
	show(a, len);
	//bubbleSortOptimized(a, len);
	//selectionSort(a, len);
	show(a, len);
	return 0;
}



void bubbleSort(int a[], int len)
{
	int i = 0, j= 0, temp = 0;

	for ( i = 0; i<len; i++)
	{
	
		for ( j = 0; j<len-i-1; j++)
		{
			if ( a[j] > a[j+1] )
			{
				temp = a[j];
				a[j] = a[j+1];
				a[j+1] = temp;
			}
		}

	}

	show(a, len);
}


void bubbleSortOptimized(int a[], int len)
{
	int i = 0, j= 0, temp = 0;
	bool isSwaped;

	for ( i = 0; i<len; i++)
	{
		isSwaped = false;
		for ( j = 0; j<len-i-1; j++)
		{
			if ( a[j] > a[j+1] )
			{
				SWAP(a[j], a[j+1], temp);
				isSwaped = true;
			}
		}


		if ( isSwaped == false)
			break;
	}

	show(a, len);	
}

void selectionSort(int a[], int len)
{
	int i = 0, j = 0, temp = 0, min=0;

	for ( i=0; i<len-1; i++)
	{
		min = i;
		for ( j=i+1; j<len; j++)
		{
			if ( a[j] < a[min] )
				min = j;
		}

		SWAP(a[i], a[min], temp);
	}
	
	show(a, len);
}


void fill(int a[], int len)
{
	srand(time(0));
	
	while (len>0)
		a[--len] = rand();
}

void show(int a[], int len)
{
	int i = 0;
	while( i<len ){
		printf("\na[%02d] = %d", i, a[i]);
		i++;
	}
	putc(stdout, '\n');
}
