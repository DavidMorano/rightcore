/* lockmemalloc */


/* revision history:

	= 2015-04-06, David A­D­ Morano
	Written to have a place for the various KSH initialization subroutines.

*/

/* Copyright © 2015 David A­D­ Morano.  All rights reserved. */

#ifndef	LOCKMEMALLOC_INCLUDE
#define	LOCKMEMALLOC_INCLUDE	1


enum lockmemallocsets {
    lockmemallocset_end,
    lockmemallocset_begin,
    lockmemallocset_overlast
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	lockmemalloc_set(int) ;

#ifdef	__cplusplus
}
#endif

#endif /* LOCKMEMALLOC_INCLUDE	*/


