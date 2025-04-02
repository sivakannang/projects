
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
	LIST,
	LIST_SIZE,
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
void FREE(NODE *node);

NODE *insert_front(NODE node, NODE *head);
NODE *delete_front(NODE *head);
NODE *insert_rear(NODE node, NODE *head);
NODE *delete_rear(NODE *head);
int length(NODE *head);
void displayList(NODE *head);
NODE *search(NODE *head, int key);
NODE *sort(NODE *head);
NODE *reverse(NODE *head);
NODE *pair_wise_swap(NODE *head);
NODE *concat(NODE *first, NODE *second);
NODE *removeDuplicateSorted(NODE *head);
NODE *removeDuplicateUnSorted(NODE *head);
#endif
