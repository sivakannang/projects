#include <stdio.h>
#include <pthread.h> 

extern int *arr;

extern char* returnString(char* charPtr);
extern void setIntPointer(int* ptr);
extern void* print2ndToken(void* cptr);
extern void usePthread();
extern void useMyType(int caseN);


int main()
{
    char * charptr = "Welcome to Magnitude!";
    char charptr1[] = "Welcome,to,Magnitude!";
    int * intptr = NULL;
    
    printf("%d\n", *arr);
    printf("%s\n", charptr);
    charptr[2] = 'd';
    printf("%s\n", charptr);

    charptr = returnString(charptr);
    printf("%s\n", charptr);

    printf("%d", intptr);
    setIntPointer(intptr);
    printf("%d", *intptr);

    print2ndToken(charptr1);
    printf("%s", charptr1);

    usePthread();

    useMyType(1);

	return 0;
}