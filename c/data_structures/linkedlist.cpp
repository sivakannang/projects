// g++ linkedlist.cpp -o linkedlist

#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"


// Returns the number read. Exits on end of input, and skips a line that isn't a number.
static int readInt(const char *prompt)
{
	int value = 0;

	for (;;)
	{
		printf("%s", prompt);

		int ret = scanf("%d", &value);
		if ( ret == 1 )
			return value;

		if ( ret == EOF )
			exit(0);

		int c;
		while ( (c = getchar()) != '\n' && c != EOF );
	}
}

static NODE *newNode(NODE node, NODE *next)
{
	NODE *tmp = (NODE *)malloc(sizeof(NODE));

	if ( tmp == NULL )
	{
		perror("malloc");
		exit(1);
	}

	*tmp = node;
	tmp->next = next;

	return tmp;
}


int main()
{
	NODE *head = NULL;
	NODE *emp;

	for(;;)
	{
		printf("\n\n\n------- Linked List------\n\n");
		printf("\n01:Insert Front");
		printf("\n02:Delete Front");
		printf("\n03:Insert Rear");
		printf("\n04:Delete Rear");
		printf("\n05:Display List");
		printf("\n06:List Size");
		printf("\n07:Search");
		printf("\n08:Sort by id");
		printf("\n09:Reverse");
		printf("\n10:Pair wise swap");
		printf("\n11:Remove Duplicates on un-sorted");
		printf("\n12:Clone and display the copy");
		printf("\n13:Concat a clone to the end of the list");
		printf("\n14:Delete By Key");
		printf("\n15:Quit");

		switch( readInt("\n\nEnter the choice (1-15) : ") )
		{
			case INSERT_FRONT:
				head = insert_front(getInfo(), head);
				break;

			case DELETE_FRONT:
				head = delete_front(head);
				break;

			case INSERT_REAR:
				head = insert_rear(getInfo(), head);
				break;

			case DELETE_REAR:
				head = delete_rear(head);
				break;

			case LIST:
				displayList(head);
				break;

			case LIST_SIZE:
				printf("\nList length : %d", length(head));
				break;

			case SEARCH:
				emp = search(head, readInt("\nEnter emp id to search: "));
				if ( emp == NULL)
					printf("\nSearch data not found");
				else
					displayInfo(emp);
				break;

			case SORT:
				head = sort(head);
				break;

			case REVERSE:
				head = reverse(head);
				break;

			case PAIR_WISE_SWAP:
				head = pair_wise_swap(head);
				break;

			case REMOVE_DUPLICATES:
				head = removeDuplicateUnSorted(head);
				break;

			case CLONE:
				emp = clone(head);
				displayList(emp);
				freeList(emp);
				break;

			case CONCAT:
				// Must be a copy. Joining a list to itself would create a loop.
				head = concat(head, clone(head));
				break;

			case DELETE_BY_KEY:
				head = delete_bykey(readInt("\nEnter employee id to delete : "), head);
				break;

			case EXIT:
				freeList(head);
				return 0;

			default:
				printf("\nInvalid Choice");
				break;
		}
	}
}


NODE *insert_front(NODE node, NODE *head)
{
	return newNode(node, head);
}

NODE *delete_front(NODE *head)
{
	if ( head == NULL)
	{
		printf("\nList is Empty, cannot delete\n");
		return head;
	}

	NODE *tmp = head;

	head = head->next;

	FREE(tmp);

	return head;
}

NODE *insert_rear(NODE node, NODE *head)
{
	NODE *tmp = newNode(node, NULL);

	/* If list is empty return new node as the first node */
	if ( head == NULL) return tmp;

	/* If list exists obtain the address of the last node*/
	NODE *cur = head;
	while ( cur->next != NULL) cur = cur->next;

	cur->next = tmp;

	return head;
}

NODE *delete_rear(NODE *head)
{
	/* Check for empty list */
	if ( head == NULL){
		printf("\nList is empty, cannot be delete\n");
		return head;
	}

	/* Only one NODE is present and delete it */
	if ( head->next == NULL){
		FREE(head);
		return NULL;  // List is empty, so return NULL
	}


	/* Obtain address of the last node and the one just before it */
	NODE *prev = NULL;
	NODE *cur  = head;
	while ( cur->next != NULL){
		prev = cur;
		cur = cur->next;
	}

	/* delete the last node */
	prev->next = NULL; // Make last but one node as the last node
	FREE(cur);

	return head;
}

NODE *delete_bykey(int key, NODE *head)
{
	NODE *prev = NULL, *cur = head;

	if ( head == NULL){  //Check for Empty List
		printf("\nList is Empty\n");
		return NULL;
	}

	while ( cur != NULL && cur->id != key ){
		prev = cur;            // Save the address of current node
		cur = cur->next;       // Point cur to the next node
	}

	if ( cur == NULL){             // End of list, Key not found
		printf("\nNode not found. Search unsuccessful\n");
		return head;
	}

	if ( prev == NULL )            // Deleting the first node
		head = cur->next;
	else
		prev->next = cur->next;

	FREE(cur);

	return head;
}

int length(NODE *head)
{
	int count = 0;

	for ( NODE *cur = head; cur != NULL; cur = cur->next )
		count++;

	return count;
}

NODE *search(NODE *head, int key)
{
	for ( NODE *emp = head; emp != NULL; emp = emp->next )
	{
		if ( emp->id == key )
			return emp;
	}

	return NULL;
}


NODE *concat(NODE *first, NODE *second)
{
	if ( first == NULL )  return second;

	if ( second == NULL ) return first;

	NODE *cur = first;
	while( cur->next != NULL)
		cur = cur->next;

	cur->next = second;

	return first;
}

NODE *reverse(NODE *head)
{
	NODE *cur = head, *prev = NULL, *nxt = NULL;

	while ( cur != NULL )
	{
		nxt = cur->next;     // save next node
		cur->next = prev;    // reverse pointer
		prev = cur;          // move prev forward
		cur = nxt;           // move cur forward
	}

	return prev;                 // return new head
}

// Floyd's algorithm: if there is a loop, the fast pointer eventually catches the slow one
bool has_cycle(NODE *head)
{
	NODE *slow = head;
	NODE *fast = head;

	while ( fast && fast->next )
	{
		slow = slow->next;
		fast = fast->next->next;

		if ( slow == fast )
			return true;
	}
	return false;
}

NODE *clone(NODE *head)
{
	NODE *first = NULL;
	NODE **tail = &first;

	for ( NODE *cur = head; cur != NULL; cur = cur->next )
	{
		*tail = newNode(*cur, NULL);
		tail = &(*tail)->next;
	}

	return first;
}


NODE *removeDuplicateSorted(NODE *head)
{
	NODE *cur = head;
	NODE *dup = NULL;

	if ( cur == NULL){
		printf("\nList is Empty\n");
		return NULL;
	}

	while ( cur->next != NULL){
		if ( cur->id == cur->next->id){
			dup = cur->next;
			cur->next = cur->next->next;
			FREE(dup);
		}else{
			cur = cur->next;
		}
	}

	return head;
}

NODE *removeDuplicateUnSorted(NODE *head)
{
	if ( head == NULL){
		printf("\nList is Empty\n");
		return NULL;
	}

	// Removing duplicates can empty the rest of the list, making base NULL, so check it first
	for ( NODE *base = head; base != NULL; base = base->next ){

		NODE *prev = base;
		NODE *cur = base->next;

		while ( cur ){

			if ( base->id == cur->id){ //10 20 20 10 10
				NODE *dup = cur;
				cur = cur->next;
				prev->next = cur;
				FREE(dup);
			}
			else{
				prev = cur;
				cur = cur->next;
			}
		}
	}

	return head;
}

NODE *pair_wise_swap(NODE *head)
{
	if ( head == NULL || head->next == NULL)
		return head;

	NODE *prev = head;
	NODE *cur  = head->next;
	NODE *nxt  = NULL;

	head = head->next;

	while( cur )
	{
		nxt        = cur->next;
		cur->next  = prev;
		prev->next = nxt;

		if ( nxt == NULL || nxt->next == NULL )
			break;

		prev->next = nxt->next;

		prev = nxt;
		cur  = prev->next;
	}

	return head;
}


// Merges two lists sorted by id. Equal ids keep their order, which makes the sort stable.
static NODE *mergeSorted(NODE *a, NODE *b)
{
	NODE *first = NULL;
	NODE **tail = &first;

	while ( a && b )
	{
		NODE **smaller = (b->id < a->id) ? &b : &a;

		*tail = *smaller;
		tail = &(*smaller)->next;
		*smaller = (*smaller)->next;
	}

	*tail = a ? a : b;

	return first;
}

// Merge sort by id. It relinks the nodes, which is why qsort can't be used: the nodes aren't in an array.
NODE *sort(NODE *head)
{
	if ( head == NULL || head->next == NULL )
		return head;

	// Find the middle: the fast pointer moves twice as quickly as the slow one
	NODE *slow = head;
	NODE *fast = head->next;

	while ( fast && fast->next )
	{
		slow = slow->next;
		fast = fast->next->next;
	}

	NODE *second = slow->next;
	slow->next = NULL;

	return mergeSorted(sort(head), sort(second));
}

void displayList(NODE *head)
{
	NODE *tmp = head;

	if ( tmp == NULL)
	{
		printf("\nList is Empty\n");
		return ;
	}

	while(tmp)
	{
		displayInfo(tmp);
		tmp = tmp->next;
	}
}


EMPLOYEE getInfo()
{
	EMPLOYEE emp = {};

	printf("\n---------- Getting Employee Details --------------\n");

	emp.id = readInt("\nEnter ID number          : ");

	// The widths stop scanf overflowing the arrays: MAX_LEN_TEXT - 1, plus the NUL
	printf("\nEnter name               : ");
	if ( scanf("%47s", emp.name) != 1 ) exit(0);

	printf("\nEnter gender ( M or F )  : ");
	if ( scanf(" %c", &emp.gender) != 1 ) exit(0);

	emp.age = (short)readInt("\nEnter age                : ");

	printf("\nEnter department         : ");
	if ( scanf("%47s", emp.department) != 1 ) exit(0);

	printf("\n\n");

	return emp;
}

void displayInfo(EMPLOYEE *employee)
{
	putc('\n', stdout);
	printf("\nEmployee id              : %d", employee->id);
	printf("\nEmployee name            : %s", employee->name);
	printf("\nEmployee age             : %d", employee->age);
	printf("\nEmployee gender          : %c", employee->gender);
	printf("\nEmployee department      : %s", employee->department);
	putc('\n', stdout);
}

void FREE(EMPLOYEE *employee)
{
	printf("\nDeleting Record\n");

	displayInfo(employee);

	free(employee);

	printf("\nRecord deleted\n");
}

void freeList(NODE *head)
{
	while ( head )
	{
		NODE *nxt = head->next;
		free(head);
		head = nxt;
	}
}
