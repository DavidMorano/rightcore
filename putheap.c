/* putheap */

/* put strings into a heap */


/* revision history:

	= 1998-11-01, David A­D­ Morano

	This subroutine was written for Rightcore Network Services.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<localmisc.h>


/* local defines */

#define		HEAPSIZE	1024


/* local variables */

static int	heap_len = 0 ;

static char	*heap_start ;
static char	*heap_current ;


/* exported subroutines */


char *putheap(s)
const char	*s ;
{
	int	l = strlen(s) + 1 ;


/* allocate more space if necessary */

	if (heap_len < l) {

	    heap_len = (HEAPSIZE > l) ? HEAPSIZE : l ;
	    if ((heap_start = (char *) malloc(heap_len)) == NULL)
	        return NULL ;

	    heap_current = heap_start ;
	}

/* store the string */

	strcpy(heap_current,s) ;

	heap_len -= l ;
	heap_current += l ;
	return (heap_current - l) ;
}
/* end subroutine (putheap) */


