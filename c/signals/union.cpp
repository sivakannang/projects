#include "stdio.h"
#include "stdlib.h"

typedef union
{
	char gender;
	short age;
	int salary;
}MEMBER;                  // size will be 4 bytes


int main()

{

	MEMBER employee = {0};
	int fldCount = 3;
	int fldOffset = 0;

	printf("\nInitial values            ->Gender : %c, Age : %d , salary : %d", employee.gender, employee.age, employee.salary);

	for ( fldOffset = 0; fldOffset < fldCount; fldOffset++)
	{
		switch(fldOffset)
		{
			case 0:
				employee.gender = 'M';
				
				printf("\nafter set gender='M'      ->Gender : %c, Age : %d , salary : %d", employee.gender, employee.age, employee.salary);
				break;
			case 1:
				employee.age = 22;
				printf("\nafter set age = 22        ->Gender : %c, Age : %d , salary : %d", employee.gender, employee.age, employee.salary);
				break;
			case 2:
				employee.salary =17000;
				printf("\nafter set salary = 17000  ->Gender : %c, Age : %d , salary : %d", employee.gender, employee.age, employee.salary);
				break;

		}
	}

	return 0;

}


/**************************************** output **************************************

$ g++ union.cpp -o union

$ ./union.exe

Initial values            ->Gender : ,  Age : 0 ,    salary : 0
after set gender='M'      ->Gender : M, Age : 77 ,   salary : 77
after set age = 22        ->Gender : ,  Age : 22 ,   salary : 22
after set salary = 17000  ->Gender : h, Age : 17000, salary : 17000

***************************************** output **************************************/


