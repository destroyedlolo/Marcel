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
