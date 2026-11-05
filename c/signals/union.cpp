// g++ union.cpp -o union
//
// All members of a union share the same memory, so writing one changes what the others read.
// The size is that of the largest member, here 4 bytes.
//
// Reading a member other than the last one written is allowed in C. In standard C++ it is
// undefined, though GCC and Clang document that it works as it does in C. The values below
// depend on the machine's byte order: this output is from a little-endian machine.

#include <stdio.h>

typedef union
{
	char gender;
	short age;
	int salary;
}MEMBER;

static void show(const char *title, const MEMBER *employee)
{
	printf("%-27s -> Gender : %c, Age : %d, salary : %d\n", title, employee->gender, employee->age, employee->salary);
}

int main()
{
	MEMBER employee = {0};

	employee.salary = 0;
	show("Initial values", &employee);

	employee.gender = 'M';
	show("after set gender = 'M'", &employee);

	employee.age = 22;
	show("after set age = 22", &employee);

	employee.salary = 17000;
	show("after set salary = 17000", &employee);

	return 0;
}


/**************************************** output **************************************

Initial values              -> Gender : , Age : 0, salary : 0
after set gender = 'M'      -> Gender : M, Age : 77, salary : 77
after set age = 22          -> Gender : , Age : 22, salary : 22
after set salary = 17000    -> Gender : h, Age : 17000, salary : 17000

***************************************** output **************************************/
