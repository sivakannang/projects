// g++ data_structure.cpp -o data_structure

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data_structure.h"

int main()
{
	int a[6] = {0};
	int len = sizeof(a)/sizeof(a[0]);
	char names[][MAX_LEN_NAME] = {"RANJITH", "SIVAKANNAN", "SIVAGANESHAN", "DINESH", "SIVAKARTHIKEYAN", "PRADEEP", "KASI", "KARTHI"};
	int nameCount = sizeof(names)/sizeof(names[0]);

	srand(time(0));

	fill(a, len);
	display(a, len);
	bubbleSortOptimized(a, len);
	display(a, len);

	Index pos = binary_search(a, len, a[2]);
	printf("\n Binary search Pos = %d", pos);


	displayString(names, nameCount);
	sortString(names, nameCount);
	displayString(names, nameCount);

	memset(names, 0, sizeof(names));
	fillString(names, nameCount);
	displayString(names, nameCount);
	sortString(names, nameCount);
	displayString(names, nameCount);


	librarySortAndSearch();

	putc('\n', stdout);

	return 0;
}


void fill(int a[], int len)
{
	while (len>0)
		a[--len] = rand();
}

void display(int a[], int len)
{
	printf("\n ----- display array starts----------");

	for ( int i = 0; i < len; i++ )
		printf("\na[%02d] = %d", i, a[i]);

	printf("\n ----- display array Ends----------");

	putc('\n', stdout);
}

void bubbleSort(int a[], int len)
{
	int temp = 0;

	for ( int i = 0; i<len; i++)
	{
		for ( int j = 0; j<len-i-1; j++)
		{
			if ( a[j] > a[j+1] )
				SWAP(a[j], a[j+1], temp);
		}
	}
}


void bubbleSortOptimized(int a[], int len)
{
	int temp = 0;

	for ( int i = 0; i<len; i++)
	{
		bool isSwaped = false;

		for ( int j = 0; j<len-i-1; j++)
		{
			if ( a[j] > a[j+1] )
			{
				SWAP(a[j], a[j+1], temp);
				isSwaped = true;
			}
		}

		if ( isSwaped == false)   // No swaps means the array is already sorted
			break;
	}
}

void selectionSort(int a[], int len)
{
	int temp = 0;

	for ( int i=0; i<len-1; i++)
	{
		int small = i;

		for ( int j=i+1; j<len; j++)
		{
			if ( a[j] < a[small] )
				small = j;
		}

		SWAP(a[i], a[small], temp);
	}
}


void fillString(char str[][MAX_LEN_NAME], int len)
{
	const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for ( int i = 0; i < len; i++)
	{
		int j;

		for ( j = 0; j < MAX_LEN_NAME/2; j++)
			str[i][j] = charset[ rand() % (sizeof(charset)-1) ];

		str[i][j] = 0;
	}
}


void displayString(char str[][MAX_LEN_NAME], int len)
{
	printf("\n ----- display array starts----------");

	for ( int i=0; i<len; i++)
		printf("\n%s", str[i]);

	printf("\n ----- display array ends----------");
}


void sortString(char (*str)[MAX_LEN_NAME], int len)
{
	char temp[MAX_LEN_NAME] = {0};

	for( int i=0; i<len-1; i++)
	{
		int small = i;

		for( int j=i+1; j<len; j++)
		{
			if ( strcmp(str[j], str[small]) < 0)
				small = j;
		}

		// strcpy on overlapping memory is undefined, and these would be the same string
		if ( small != i )
			SWAP_STR(str[i], str[small], temp);
	}
}


Index binary_search(int *arr, int len, int key)
{
	int first = 0, last = len - 1;

	while( first <= last)
	{
		int middle = first + (last - first) / 2;   // (first+last) can overflow an int

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
	int len = sizeof(arr)/sizeof(arr[0]);

	fill(arr, len);

	display(arr, len);
	qsort(arr, len, sizeof(arr[0]), cmpfunc);
	display(arr, len);

	// bsearch returns a pointer to the matching element, not its index
	int *found = (int *)bsearch(&arr[6], arr, len, sizeof(arr[0]), cmpfunc);

	if ( found == NULL)
	{
		printf("\nData not found");
		return;
	}

	printf("\nData Found with key %d at index %d", *found, (int)(found - arr));
}

int cmpfunc (const void * a, const void * b)
{
	int x = *(const int *)a;
	int y = *(const int *)b;

	return (x > y) - (x < y);   // x - y can overflow when the values have opposite signs
}
