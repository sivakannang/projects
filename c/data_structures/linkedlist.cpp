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

// 1. Sort a Linked list
// 2. Remove duplicates from a sorted linked list
// 3. Remove duplicates from a unsorted linked list
// 4. Reverse a Linked List
// 5. Pair wise swap
// 6. Find/Detect loop is in linked list
// 7. Detect and Remove loop from a list
// 8. Middle of Linked List

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
		printf("\n05:Display List");
		printf("\n06:List Size");
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

			case LIST:
				displayList(head);
				break;

			case LIST_SIZE:
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


NODE *insert_front(NODE node, NODE *head)
{
	NODE *tmp = (NODE *)malloc(sizeof(NODE));

	*tmp = node;

	tmp->next = head;

	return tmp;
}

void display(NODE first)
{
	
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
	
	/* Obtain the new node and copy the item */
	NODE *tmp = (NODE *)malloc(sizeof(NODE));
	*tmp = node;
	tmp->next = NULL;

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


	/* Obtain address of the last node and just just previous to that */
	NODE *prev = NULL;
	NODE *cur  = head;
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

int length(NODE *head)
{
	int count = 0;

	if ( head == NULL) return 0;

	NODE *cur = head;
	while ( cur != NULL ){
		count++;
		cur = cur->next;
	}

	return count;
}

NODE *search(NODE *head, int key)
{
	if (head == NULL) return NULL;

	NODE *emp = head;

	while(emp != NULL){
		if ( emp->id == key )  return emp;
		emp = emp->next;
	}

	return NULL;
	
}


EMPLOYEE *concat(EMPLOYEE *first, EMPLOYEE *second)
{
	if ( first == NULL )  return second;

	if ( second == NULL ) return first;

	NODE *cur = first;
	while( cur->next != NULL)
		cur = cur->next;

	cur->next = second;

	return first;
}

// https://www.geeksforgeeks.org/reverse-a-linked-list/
NODE *reverse(NODE *head)
{
	
	NODE *cur = head, *prev = NULL, *nxt = NULL;

	if ( head == NULL || head->next == NULL )
		return head;

	while ( cur->next != NULL )
	{
		nxt = cur->next;     // Obtain the address of the second node of list to be reversed
		cur->next = prev;    // Attach first node of the list to be reversed, at the beginning of the partially reversed list
		prev = cur;          // Point cur to point to newly partially reversed list
		cur = nxt;           // Point first to point to the list to be reversed
	}

	cur->next = prev;
	return cur;                    // Contains address of the reversed list 
	
}

bool has_cycle(NODE *head)
{
	NODE *slow = head;
	NODE *fast = head;

	while ( slow && fast && fast->next )
	{
		slow = slow->next;
		fast = fast->next->next;

		if ( slow == fast )
			return true;
	}
	return false;
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
		{
			prev->next = nxt;
			break;
		}
		else
		{
			prev->next = nxt->next;
		}

		prev = nxt;
		cur  = prev->next;
	}

	return head;
}


int comp_id (const void * a, const void * b);

NODE *sort(NODE *head)
{
	//qsort(head, length(head), sizeof(EMPLOYEE), comp_id)i;
	return head;	
	
}

int comp_id (const void * a, const void * b)
{
	return ( ((EMPLOYEE *)a)->id - ((EMPLOYEE *)b)->id );
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
        scanf("%hu", &emp.age);

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

