// DS.cpp : Defines the entry point for the console application.
//


#ifdef WIN32
	#include "stdafx.h"
#endif

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#include "data_structure.h"

int main(int argc, char *args[])
{
	int len = 6;
	int a[6] = {0};
	char names[][MAX_LEN_NAME] = {"RANJITH", "SIVAKANNAN", "SIVAGANESHAN", "DINESH", "SIVAKARTHIKEYAN", "PRADEEP", "KASI", "KARTHI"};

	srand(time(0));	

	fill(a, len);
	display(a, len);
	bubbleSortOptimized(a, len);
	//selectionSort(a, len);
	display(a, len);
	Index pos = binary_search(a, len, a[2]);
	printf("\n Binary search Pos = %d", pos); 

	
	displayString(names, sizeof(names)/MAX_LEN_NAME);
	sortString(names, sizeof(names)/MAX_LEN_NAME);
	displayString(names, sizeof(names)/MAX_LEN_NAME);

	memset(names, 0, sizeof(names));
	fillString(names, sizeof(names)/MAX_LEN_NAME);
	displayString(names, sizeof(names)/MAX_LEN_NAME);
	sortString(names, sizeof(names)/MAX_LEN_NAME);
	displayString(names, sizeof(names)/MAX_LEN_NAME);


	librarySortAndSearch();

	return 0;
}


void fill(int a[], int len)
{
	
	while (len>0)
		a[--len] = rand();
}

void display(int a[], int len)
{
	short i = 0;
	
	printf("\n ----- display array starts----------");
	
	while( i<len ){
		printf("\na[%02d] = %d", i, a[i]);
		i++;
	}
	
	printf("\n ----- display array Ends----------");
	
	putc('\n', stdout);
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

	display(a, len);
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

	display(a, len);	
}

void selectionSort(int a[], int len)
{
	int i = 0, j = 0, temp = 0, small=0;

	for ( i=0; i<len-1; i++)
	{
		small = i;
		for ( j=i+1; j<len; j++)
		{
			if ( a[j] < a[small] )
				small = j;
		}

		SWAP(a[i], a[small], temp);
	}
	
	display(a, len);
}


void fillString(char str[][MAX_LEN_NAME], int len)
{
	char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	short i = 0, j =0;

	for ( int i =0 ; i < len; i++)
	{
		for ( j = 0; j < MAX_LEN_NAME/2; j++)
		{
			str[i][j] = charset[ rand() % (sizeof(charset)-1) ];
		}

		str[i][j] = 0;
	}
}


void displayString(char str[][MAX_LEN_NAME], int len)
{
	short i = 0;

	printf("\n ----- display array starts----------");

	for ( i=0; i<len; i++)
		printf("\n%s", str[i]);
	
	printf("\n ----- display array ends----------");

}


void sortString(char (*str)[MAX_LEN_NAME], int len)
{
	short small = 0, i = 0, j = 0;
	char temp[256] = {0};

	for( i=0; i<len-1; i++)
	{
		small = i;
		for( j=i+1; j<len; j++)
		{
			if ( strcmp(str[j], str[small]) < 0)
				small = j;

		}
		SWAP_STR(str[i], str[small], temp);
	}
}


Index binary_search(int *arr, int len, int key)
{
	int first = 0, last = len - 1, middle;

	while( first <= last)
	{
		middle = (first+last) / 2;

		if ( arr[middle] == key)
			return middle;
		else if ( arr[middle] < key )
			first = middle+1;
		else  // arr[middle] > key
			last = middle-1;
	}

	return -1;	

}


void librarySortAndSearch()
{
	//void qsort(void *base, size_t nitems, size_t size, int (*compar)(const void *, const void*))
	//void *bsearch(const void *key, const void *base, size_t nitems, size_t size, int (*compar)(const void *, const void *))

	int arr[12] = {0};
	Index *idx = 0;

	fill(arr, sizeof(arr)/sizeof(int));

	display(arr, sizeof(arr)/sizeof(int));
	qsort(arr, sizeof(arr)/sizeof(int), sizeof(int), cmpfunc);
	display(arr, sizeof(arr)/sizeof(int));

	idx = (Index *)bsearch(&arr[6], arr, sizeof(arr)/sizeof(int), sizeof(int), cmpfunc);

	if ( idx == NULL)
	{
		printf("\nData not found");
		return;
	}
	
	printf("\nData Found with key %d and data is %d", arr[6], key);

}

int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}
