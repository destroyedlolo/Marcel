/* DList.c
 *
 * Implementation of a Double Linked List
 *
 * Cover by Creative Commons Attribution-NonCommercial 3.0 License
 * (http://creativecommons.org/licenses/by-nc/3.0/)
 *
 * 16/07/2015 LF - Creation
 */
#include "DList.h"

#include <stddef.h>
#include <stdio.h>

void DLListInit( struct DList *l ){
	l->first = l->last = NULL;
}

void DLAdd( struct DList *l, struct DLNode *n ){
puts("*d* DLAdd()");
	n->next = NULL;
	if(!(n->prev = l->last))	/* The list is empty */
		l->first = n;
	else
		n->prev->next = n;
	l->last = n;
}

void DLRemove( struct DList *l, struct DLNode *n ){
puts("*d* DLRemove()");
	if(n->next)
		n->next->prev = n->prev;
	else	/* Last of the list */
		l->last = n->prev;

	if(n->prev)
		n->prev->next = n->next;
	else	/* First of the list */
		l->first = n->next;
}
