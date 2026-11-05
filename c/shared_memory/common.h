#ifndef SHARED_MEMORY_COMMON_H
#define SHARED_MEMORY_COMMON_H

typedef struct
{
	char name[48];
	char gender;
	int  age;
	char department[48];
}STUDENT;

typedef struct
{
	char name[48];
	char gender;
	int  salary;
	char designation[48];
}EMPLOYEE;

typedef struct
{
	char name[48];
	char author[48];
	int  price;
}BOOK;

// The layout of the whole segment. A struct lets the compiler work out the offsets.
typedef struct
{
	STUDENT  student;
	EMPLOYEE employee;
	BOOK     book;
}SHARED_DATA;

// These return false at end of input
bool getStudentInfo(STUDENT *student);
bool getEmployeeInfo(EMPLOYEE *employee);
bool getBookInfo(BOOK *book);

void displayStudentInfo(const STUDENT *student);
void displayEmployeeInfo(const EMPLOYEE *employee);
void displayBookInfo(const BOOK *book);

// Reads a line into text, without the newline. Returns false at end of input.
bool readLine(const char *prompt, char *text, int size);

#endif
