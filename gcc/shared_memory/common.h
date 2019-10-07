
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

typedef enum
{
	T_STUDENT,
	T_EMPLOYEE,
	T_BOOK
}TYPE;

STUDENT *getStudentInfo(STUDENT *student);
void displayStudentInfo(STUDENT *student);

EMPLOYEE *getEmployeeInfo(EMPLOYEE *employee);
void displayEmployeeInfo(EMPLOYEE *employee);

BOOK *getBookInfo(BOOK *book);
void displayBookInfo(BOOK *book);

void *getPtrFromSharedMem(void *ptr, TYPE type);



