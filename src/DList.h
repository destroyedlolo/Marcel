/* DList.h
 *
 * Definition of a Double Linked List
 *
 * Cover by Creative Commons Attribution-NonCommercial 3.0 License
 * (http://creativecommons.org/licenses/by-nc/3.0/)
 *
 * 16/07/2015 LF - Creation
 */

#ifndef DLIST_H
#define DLIST_H

struct DLNode {
	struct DLNode *prev;
	struct DLNode *next;
};

struct DList {
	struct DLNode *first;
	struct DLNode *last;
};

void DLListInit( struct DList * );
void DLAdd( struct DList *, struct DLNode * );
void DLRemove( struct DList *, struct DLNode * );

#endif
