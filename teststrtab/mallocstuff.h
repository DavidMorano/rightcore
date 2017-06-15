/* mallocstuff */

/* variation on the 'malloc(3)' theme */


/* revision history:

	= 1998-06-11, David A­D­ Morano
	These subroutines were was originally written (inspired by others).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MALLOCSTUFF_INCLUDE
#define	MALLOCSTUFF_INCLUDE	1


#ifdef	__cplusplus
extern "C" {
#endif

extern char	*mallocbuf(void *,int) ;
extern char	*mallocstr(const char *) ;
extern char	*mallocstrw(const char *,int) ;
extern char	*mallocstrw(const char *,int) ;
extern char	*mallocint(int) ;

#ifdef	__cplusplus
}
#endif

#endif /* MALLOCSTFF_INCLUDE */


