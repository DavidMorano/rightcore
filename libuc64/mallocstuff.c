/* mallocstuff */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UCMALLOC	1		/* use 'uc_malloc(3uc)' */
#define	CF_LIBMALLOC	0		/* use 'libmalloc(3malloc)' */


/* revision history:

	= 1998-06-11, David A­D­ Morano
	These subroutines were was originally written (inspired by others).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

        These subroutines allocate a fixed amount of memory and then copy the
        user supplied thing into it.


****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#if	CF_LIBMALLOC
#include	<malloc.h>
#endif

#include	<vsystem.h>
#include	<localmisc.h>

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

char	*mallocbuf(void *,int) ;
char	*mallocstr(const char *) ;
char	*mallocstrw(const char *,int) ;
char	*malloctest(int) ;


/* exported subroutines */


char *mallocbuf(b,blen)
void		*b ;
int		blen ;
{
	char	*rp = NULL ;


	if (b == NULL)
	    return NULL ;

	if (blen < 0)
	    return NULL ;

#if	CF_UCMALLOC
	if (uc_malloc((blen + 1),&rp) < 0)
	    rp = NULL ;
#else
	rp = (char *) malloc(blen + 1) ;
#endif

	if (rp != NULL)
	    memcpy(rp,b,blen) ;

	return rp ;
}
/* end subroutine (mallocbuf) */


/* allocate a buffer and copy a string into it */
char *mallocstr(s)
const char	*s ;
{
	int	slen ;

	char	*rp = NULL ;


	if (s == NULL)
	    return NULL ;

	slen = strlen(s) ;

#if	CF_UCMALLOC
	if (uc_malloc((slen + 1),&rp) < 0)
	    rp = NULL ;
#else
	rp = (char *) malloc(slen + 1) ;
#endif

	if (rp != NULL)
	    strcpy(rp,s) ;

	return rp ;
}
/* end subroutine (mallocstr) */


/* allocate a buffer and copy a counted string into it */
char *mallocstrw(s,slen)
const char	s[] ;
int		slen ;
{
	int	f = FALSE ;

	char	*rp = NULL ;


	if (s == NULL) 
	    return NULL ;

	if (slen < 0) {
	    f = TRUE ;
	    slen = strlen(s) ;
	}

#if	CF_UCMALLOC
	if (uc_malloc((slen + 1),&rp) < 0)
	    rp = NULL ;
#else
	rp = (char *) malloc(slen + 1) ;
#endif /* CF_UCMALLOC */

	if (rp != NULL) {

	    if (f) {
	        memcpy(rp,s,(slen + 1)) ;
	    } else
	        strwcpy(rp,s,slen) ;

	} /* end if */

	return rp ;
}
/* end subroutine (mallocstrw) */


/* allocate an integer sized piece of memory and copy the integer to it */
char *mallocint(v)
int		v ;
{
	int	len ;

	char	*rp = NULL ;


	len = sizeof(int) ;

#if	CF_UCMALLOC
	if (uc_malloc(len + 1,&rp) < 0)
		rp = NULL ;
#else
	rp = (char *) malloc(len + 1) ;
#endif

	if (rp != NULL)
	    memcpy(rp,(char *) &v,sizeof(int)) ;

	return rp ;
}
/* end subroutine (mallocint) */



