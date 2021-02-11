
int arr[10] = {0};

char* returnString(char* charPtr)
{
    char lcharPtr[] = "Welcome to Magnitude! Again!!";
    return lcharPtr;
}

void setIntPointer(int* ptr)
{
    static int x = 10;
    ptr = &x;
    return;
}

char *strtok(char *str, const char *delim);
int printf ( const char * format, ... );

void* print2ndToken(void* cptr)
{
   char *token = 0;
   token = strtok(cptr, ",");

   while( token != 0 ) {
      token = strtok(0, ",");
      printf( " %s\n", token );
      return 0;
   }
   return 0;
}

typedef struct
{
    char str[5000000];
}myType;


void useMyType(int caseN)
{
    if (1 == caseN)
    {
        myType val;
        val.str[0] = '1';
        val.str[1] = 0;
        printf("Created only 1 val\n");
    }
    else if (2 == caseN)
    {
        myType val;
        val.str[0] = '1';
        val.str[1] = 0;

        myType val1;
        val1.str[0] = '2';
        val1.str[1] = 0;

        printf("Created only 2 val\n");
    }
}

#include <pthread.h> 

void usePthread()
{
    int i; 
    pthread_t tid[3]; 
  
    char charptr1[] = "Welcome,to,Magnitude!";

    for (i = 0; i < 3; i++) 
        pthread_create(&tid[i], NULL, print2ndToken, (void *)charptr1); 
  
    for (i = 0; i < 3; i++) 
        pthread_join(tid[i], NULL); 
}