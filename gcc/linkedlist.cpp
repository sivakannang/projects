// linkedList.cpp : Defines the entry point for the console application.
//

#ifdef WIN32
#include "stdafx.h"
#endif

#include "stdio.h"
#include "stdlib.h"
#include "linkedlist.h"




EMPLOYEE *head;


NODE *delete_bykey(int key, NODE *head);

// 1. Remove duplicates from a sorted linked list
// 2. Remove duplicates from a unsorted linked list
// 3. Sort a linked list // Merge
// 4. Reverse a Linked List
// 5. Find/Detect loop is in linked list
// 6. Copy a linked list
// 7 . Middle of Linked List

int main()
{
	EMPLOYEE employee = {0};
	EMPLOYEE *emp;

	int choice = 0, key = 0;
	

	for(;;)
	{
		printf("\n\n\n------- Linked List------\n\n");
		printf("\n01:Insert Front");
		printf("\n02:Delete Front");
		printf("\n03:Insert Rear");
		printf("\n04:Delete Rear");
		printf("\n05:Display");
		printf("\n06:Length");
		printf("\n07:Search");
		printf("\n08:Sort");
		printf("\n09:Reverse");
		printf("\n10:Pair wise swap");
		printf("\n11:Remove Duplicates on un-sorted");
		printf("\n12:Clone");
		printf("\n13:Concat");
		printf("\n14:Delete By Key");
		printf("\n15:Quit");

		printf("\n\nEnter the choice (1-15) : ");
		scanf("%d", &choice);

		switch(choice)
		{
			case INSERT_FRONT:
			        employee = getInfo();	
				head = insert_front(employee, head);
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

			case DISPLAY:
				displayList(head);
				break;

			case GET_COUNT:
				printf("\nList length : %d", length(head));
				break;

			case SEARCH: 
				printf("\nEnter emp id to search: ");
				scanf("%d", &key);
				emp = search(head, key);
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

			//case 10:
				//first = removeDuplicateSorted(first);
				//display(first);
				//break;
				

			case REMOVE_DUPLICATES:
				head = removeDuplicateUnSorted(head);
				break;

			case 12:
				//clone(head);
				break;

			case 13:
				head = concat(head, NULL);
				break;

			case 14:
				printf("\nEnter employee id to delete : ");
				scanf("%d", &key);
				head = delete_bykey(key, head);
				break;

			case 15: exit(0);
			default: printf("\nInvalid Choice"); break;
		}
		
	}
	return 0;
}


EMPLOYEE *insert_front(EMPLOYEE employee, EMPLOYEE *head)
{
	EMPLOYEE *temp = (EMPLOYEE *)malloc(sizeof(EMPLOYEE));

	*temp = employee;
	temp->next = head;

	return temp;
}

void display(NODE first)
{
	
}

EMPLOYEE *delete_front(EMPLOYEE *head)
{
	EMPLOYEE *temp;

	if ( head == NULL){
		printf("\nList is Empty, cannot delete\n");
		return head;
	}

	temp = head;          // Retain address of the first node to be deleted
	temp = temp->next;    // Obtain address of the second node
	
	FREE(head);           // Delete the first node

	return temp;          // Return the address of the first node
}

EMPLOYEE *insert_rear(EMPLOYEE employee, EMPLOYEE *head)
{
	
	EMPLOYEE *temp;  // Points to newly created node
	EMPLOYEE *cur;   // To hold the address of the last node
	
	
	/* Obtain the new node and copy the item */
	temp = (EMPLOYEE *)malloc(sizeof(EMPLOYEE));
	*temp = employee;
	temp->next = NULL;

	/* If list is empty return new node as the first node */
	if ( head == NULL) return temp;

	/* If list exists obtain the address of the last node*/
	cur = head;
	while ( cur->next != NULL) cur = cur->next;

	cur->next = temp;

	return head; 
}

EMPLOYEE *delete_rear(EMPLOYEE *head)
{
	EMPLOYEE *cur, *prev;

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


	/* Obtain address of the last node and just just previous to that */
	prev = NULL;
	cur = head;
	while ( cur->next != NULL){
		prev = cur;
		cur = cur->next;
	}

	/* delete the last node and return to available list */
	prev->next = NULL; // Make last but one node as the last node
	FREE(cur);

	return head;
}

NODE *delete_bykey(int key, NODE *head)
{

	NODE *prev, *cur;

	if ( head == NULL){  //Check for Empty List
		printf("\nList is Empty\n");
		return NULL;
	}

	if ( key == head->id){
		cur = head;           // Save the address of the first node
		head = head->next;   // Point first to second node in the list
		free(cur), cur = 0;    // Delete the first node
		return head;          // Return second node as the first node
	}

	prev = NULL;
	cur = head;

	while ( cur != NULL){
		if ( key == cur->id) break;
		prev = cur;            // Save the address of current node
		cur = cur->next;       // Point cur to the next node
	}

	if ( cur == NULL){             // End of list, Key not found
		printf("\nNode not found. Search unsuccessful\n");
		return head;
	}

	prev->next = cur->next;
	free(cur), cur = 0;

	return head;

}

int length(EMPLOYEE *head)
{
	EMPLOYEE *cur;
	int count = 0;

	if ( head == NULL) return 0;

	cur = head;
	while ( cur != NULL ){
		count++;
		cur = cur->next;
	}

	return count;
}

EMPLOYEE *search(EMPLOYEE *head, int key)
{
	EMPLOYEE *emp;

	if (head == NULL) return NULL;

	emp = head;

	while(emp != NULL){
		if ( emp->id == key )  return emp;
		emp = emp->next;
	}

	return NULL;
	
}


EMPLOYEE *concat(EMPLOYEE *first, EMPLOYEE *second)
{
	EMPLOYEE *cur;

	if ( first == NULL )  return second;

	if ( second == NULL ) return first;

	cur = first;
	while( cur->next != NULL)
		cur = cur->next;

	cur->next = second;

	return first;
}

EMPLOYEE *reverse(EMPLOYEE *head)
{
	
	EMPLOYEE *cur = head, *prev = NULL, *next = NULL;

	while ( cur != NULL )
	{
		next = cur->next;     // Obtain the address of the second node of list to be reversed
		cur->next = prev;     // Attach first node of the list to be reversed, at the beginning of the partially reversed list
		prev = cur;           // Point cur to point to newly partially reversed list
		cur = next;           // Point first to point to the list to be reversed
	}

	return prev;                    // Contains address of the reversed list 

	
}



/*NODE *clone(NODE *head)
{
	NODE *list, *temp, *first;
	
	list = head;
	first = head;
	
	while (first != NULL){
		temp = (NODE)malloc(1 * sizeof(NODE));
		*temp = *first;
		first = first->next;
	}

	return list;
}*/



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
	
	NODE *base = head;
	NODE *prev = NULL;
	NODE *cur = NULL;
	NODE *dup = NULL;


	if ( base == NULL){
		printf("\nList is Empty\n");
		return NULL;
	}

	while ( base->next != NULL){
		
		
		cur = base->next;
		prev = base;

		while ( cur){

			if ( base->id == cur->id){ //10 20 20 10 10
				dup = cur;
				cur = cur->next;
				prev->next = cur;
				FREE(dup);
			}
			else{
				prev = cur;
				cur = cur->next;
				
			}
		}
		base = base->next;
	}

	return head;
}

EMPLOYEE *pair_wise_swap(EMPLOYEE *head)
{
	EMPLOYEE *first = head;
	EMPLOYEE *second = NULL;
	EMPLOYEE *prev   = NULL;

	if ( head == NULL || head->next == NULL)
		return head;

	head = head->next;

	while( first && first->next )
	{
		second = first->next;
		first->next = second->next;
		second->next = first;	
		
		if ( prev != NULL)
			prev->next = second;
		prev = first;
		first = first->next;

	}

	return head;
}


int comp_id (const void * a, const void * b);

EMPLOYEE *sort(EMPLOYEE *head)
{
	//qsort(head, length(head), sizeof(EMPLOYEE), comp_id);	
	
}

int comp_id (const void * a, const void * b)
{
	return ( ((EMPLOYEE *)a)->id - ((EMPLOYEE *)b)->id );
}

void displayList(EMPLOYEE *head)
{
	EMPLOYEE *tmp = head;
	
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
	EMPLOYEE emp = {0};

	printf("\n---------- Getting Employee Details --------------\n");

	printf("\nEnter ID number          : ");
	fflush(stdin);
	scanf("%d", &emp.id);

	printf("\nEnter name               : ");
	fflush(stdin);
	scanf("%s", emp.name);

	printf("\nEnter gender ( M or F )  : ");
	fflush(stdin);
        scanf(" %c", &emp.gender);

	printf("\nEnter age                : ");
	fflush(stdin);
        scanf("%d", &emp.age);

	printf("\nEnter department         : ");	
	fflush(stdin);
	scanf("%s", emp.department);

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
	employee = NULL;

	printf("\nRecord deleted\n");
}

void flush(FILE *std)
{
	int c;

	while((c = getchar()) != '\n' && c != EOF)
		;
}
