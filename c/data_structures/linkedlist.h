
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#define MAX_LEN_TEXT 48

struct Employee
{
	char name[MAX_LEN_TEXT];
	char gender;
	short age;
	char department[MAX_LEN_TEXT];
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
	DELETE_BY_KEY,
	EXIT
}OPERATION;


EMPLOYEE getInfo();
void displayInfo(EMPLOYEE *employee);
void FREE(NODE *node);
void freeList(NODE *head);

NODE *insert_front(NODE node, NODE *head);
NODE *delete_front(NODE *head);
NODE *insert_rear(NODE node, NODE *head);
NODE *delete_rear(NODE *head);
NODE *delete_bykey(int key, NODE *head);
int length(NODE *head);
void displayList(NODE *head);
NODE *search(NODE *head, int key);
NODE *sort(NODE *head);
NODE *reverse(NODE *head);
NODE *pair_wise_swap(NODE *head);
NODE *clone(NODE *head);
NODE *concat(NODE *first, NODE *second);
NODE *removeDuplicateSorted(NODE *head);
NODE *removeDuplicateUnSorted(NODE *head);
bool has_cycle(NODE *head);
#endif
