#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

bool readLine(const char *prompt, char *text, int size)
{
	printf("%s", prompt);
	fflush(stdout);

	if ( fgets(text, size, stdin) == NULL )
		return false;

	size_t len = strcspn(text, "\n");

	// The line was longer than the buffer: discard the rest, so it isn't read as the next answer
	if ( text[len] != '\n' )
	{
		int c;
		while ( (c = getchar()) != '\n' && c != EOF );
	}

	text[len] = '\0';

	return true;
}

static bool readChar(const char *prompt, char *value)
{
	char text[16];

	if ( !readLine(prompt, text, sizeof(text)) )
		return false;

	*value = text[0];

	return true;
}

static bool readInt(const char *prompt, int *value)
{
	char text[32];

	if ( !readLine(prompt, text, sizeof(text)) )
		return false;

	*value = atoi(text);

	return true;
}


bool getStudentInfo(STUDENT *student)
{
	printf("\n-------------- Getting Student Details -------------------\n");

	return readLine("Name          : ", student->name, sizeof(student->name))
		&& readChar("Gender(M | F) : ", &student->gender)
		&& readInt("Age           : ", &student->age)
		&& readLine("Department    : ", student->department, sizeof(student->department));
}


void displayStudentInfo(const STUDENT *student)
{
	printf("\n-------------- Printing Student Details -------------------\n");
	printf("Name          : %.47s\n", student->name);
	printf("Gender(M | F) : %c\n", student->gender);
	printf("Age           : %d\n", student->age);
	printf("Department    : %.47s\n", student->department);
}


bool getEmployeeInfo(EMPLOYEE *employee)
{
	printf("\n-------------- Getting Employee Details -------------------\n");

	return readLine("Name          : ", employee->name, sizeof(employee->name))
		&& readChar("Gender(M | F) : ", &employee->gender)
		&& readInt("Salary        : ", &employee->salary)
		&& readLine("Designation   : ", employee->designation, sizeof(employee->designation));
}


void displayEmployeeInfo(const EMPLOYEE *employee)
{
	printf("\n-------------- Printing Employee Details -------------------\n");
	printf("Name          : %.47s\n", employee->name);
	printf("Gender(M | F) : %c\n", employee->gender);
	printf("Salary        : %d\n", employee->salary);
	printf("Designation   : %.47s\n", employee->designation);
}


bool getBookInfo(BOOK *book)
{
	printf("\n-------------- Getting Book Details -------------------\n");

	return readLine("Name          : ", book->name, sizeof(book->name))
		&& readLine("Author        : ", book->author, sizeof(book->author))
		&& readInt("Price         : ", &book->price);
}


void displayBookInfo(const BOOK *book)
{
	printf("\n-------------- Printing Book Details -------------------\n");
	printf("Name          : %.47s\n", book->name);
	printf("Author        : %.47s\n", book->author);
	printf("Price         : %d\n", book->price);
}
