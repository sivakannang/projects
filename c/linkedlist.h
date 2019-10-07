
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct Employee
{
	char name[48];
	char gender;
	short age;
	char department[48];
	int id;
	Employee *next;
};

typedef struct Employee EMPLOYEE;
typedef struct Employee NODE;

typedef enum
{
	INSERT_FRONT = 1,
	DELETE_FRONT,
	INSERT_REAR,
	DELETE_REAR,
	DISPLAY,
	GET_COUNT,
	SEARCH,
	SORT,
	REVERSE,
	PAIR_WISE_SWAP,
	REMOVE_DUPLICATES,
	CLONE,
	CONCAT,
	EXIT
}OPERATION;


EMPLOYEE getInfo();
void displayInfo(EMPLOYEE *employee);
void FREE(EMPLOYEE *employee);

EMPLOYEE *insert_front(EMPLOYEE employee, EMPLOYEE *head);
EMPLOYEE *delete_front(EMPLOYEE *head);
EMPLOYEE *insert_rear(EMPLOYEE employee, EMPLOYEE *head);
EMPLOYEE *delete_rear(EMPLOYEE *head);
int length(EMPLOYEE *head);
void displayList(EMPLOYEE *head);
EMPLOYEE *search(EMPLOYEE *head, int key);
EMPLOYEE *sort(EMPLOYEE *head);
EMPLOYEE *reverse(EMPLOYEE *head);
EMPLOYEE *pair_wise_swap(EMPLOYEE *head);
EMPLOYEE *concat(EMPLOYEE *first, EMPLOYEE *second);
NODE *removeDuplicateSorted(NODE *head);
NODE *removeDuplicateUnSorted(NODE *head);
#endif
