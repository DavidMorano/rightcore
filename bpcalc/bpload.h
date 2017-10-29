/* bpload */


/* revision history:

	= 2001-03-22, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */


#ifndef	BPLOAD_INCLUDE
#define	BPLOAD_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* object define */

#define	BPLOAD			struct bpload


/* this is what is put in the loadable module */

struct bpload {
	char	*name ;
	int	size ;
} ;


#endif /* BPLOAD_INCLUDE */


