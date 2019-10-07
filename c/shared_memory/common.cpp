#include <stdio.h>

#include "common.h"



STUDENT *getStudentInfo(STUDENT *student)
{
	printf("\n\n\n-------------- Getting Student Details -------------------\n");
	printf("\nName          : ");
	gets(student->name);

	printf("\nGender(M | F) : ");
	scanf("%c", &student->gender);

	printf("\nAge           : ");
	scanf("%d", &student->age);
	fpurge(stdin);

	printf("\nDepartment    : ");
	gets(student->department);

	return student;
}


void displayStudentInfo(STUDENT *student)
{
	printf("\n\n\n-------------- Printing Student Details -------------------\n");
	printf("\nName          : %s", student->name);
	printf("\nGender(M | F) : %c", student->gender);
	printf("\nAge           : %d", student->age);
	printf("\nDepartment    : %s", student->department);
}



EMPLOYEE *getEmployeeInfo(EMPLOYEE *employee)
{
	printf("\n\n\n-------------- Getting Employee Details -------------------\n");
	printf("\nName          : ");
	gets(employee->name);

	printf("\nGender(M | F) : ");
	scanf("%c", &employee->gender);

	printf("\nSalary        : ");
	scanf("%d", &employee->salary);
	fpurge(stdin);

	printf("\nDesignation   : ");
	gets(employee->designation);

	return employee;
}


void displayEmployeeInfo(EMPLOYEE *employee)
{
	printf("\n\n\n-------------- Printing Employee Details -------------------\n");
	printf("\nName          : %s", employee->name);
	printf("\nGender(M | F) : %c", employee->gender);
	printf("\nSalary        : %d", employee->salary);
	printf("\nDesignation   : %s", employee->designation);
}


BOOK *getBookInfo(BOOK *book)
{
	printf("\n\n\n-------------- Getting Book Details -------------------\n");
	printf("\nName          : ");
	gets(book->name);

	printf("\nAuthor        : ");
	gets(book->author);

	printf("\nPrice         : ");
	scanf("%d", &book->price);

	return book;
}


void displayBookInfo(BOOK *book)
{
	printf("\n\n\n-------------- Printing Book Details -------------------\n");
	printf("\nName          : %s", book->name);
	printf("\nAuthor        : %s", book->author);
	printf("\nPrice         : %d", book->price);
}


void *getPtrFromSharedMem(void *ptr, TYPE type)
{
	switch(type)
	{
		case T_STUDENT  : return ptr;
		case T_EMPLOYEE : return (char *)ptr + sizeof(STUDENT);
		case T_BOOK     : return (char *)ptr + sizeof(STUDENT) + sizeof(EMPLOYEE);
	}
}
