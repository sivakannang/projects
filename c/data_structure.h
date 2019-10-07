
#ifndef _DATA_STRUCTURE_H_

	#define _DATA_STRUCTURE_H_

#define MAX_LEN_NAME 48
#define SWAP(A,B,TEMP) (TEMP=A, A=B, B=TEMP)
#define SWAP_STR(A,B,TEMP) (strcpy(TEMP, A), strcpy(A,B), strcpy(B,TEMP))

typedef int Index;

void fill(int a[], int len);
void display(int a[], int len);
void bubbleSort(int a[], int len);
void bubbleSortOptimized(int a[], int len);
void selectionSort(int a[], int len);

void fillString(char str[][MAX_LEN_NAME], int len);
void displayString(char str[][MAX_LEN_NAME], int len);
void sortString(char (*str)[MAX_LEN_NAME], int len);

Index binary_search(int *arr, int len, int key);

void librarySortAndSearch();
int cmpfunc (const void * a, const void * b);

#endif
