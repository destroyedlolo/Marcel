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
