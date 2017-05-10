/* uc_mallocbuf */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-85, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is similar to 'uc_malloc(3uc)' except that it takes a
        string argument and copies it into the newly allocated space.

	Synopsis:

	int uc_mallocbuf(s,slen,rpp)
	const char	s[] ;
	int		slen ;
	void		**rpp ;

	Arguments:

	s		string pointer
	slen		string length
	rpp		resulting pointer

	Returns:

	>=0		length of allocated space
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int uc_mallocbuf(s,slen,vrpp)
const char	s[] ;
int		slen ;
const void	**vrpp ;
{
	int	rs ;
	int	size ;

	const char	**rpp = (const char **) vrpp ;

	char	*bp ;


#if	CF_DEBUGS
	debugprintf("uc_mallocbuf: ent slen=%d\n",slen) ;
#endif

	if (rpp == NULL)
	    return SR_FAULT ;

	*rpp = NULL ;
	if (s == NULL) 
	    return SR_FAULT ;

	if (slen < 0)
	    slen = strlen(s) ;

	size = (slen + 1) ;
	rs = uc_malloc(size,&bp) ;

	if (rs >= 0) {
	    memcpy(bp,s,slen) ;
	    bp[slen] = '\0' ;
	    *rpp = bp ;
	}

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (uc_mallocbuf) */


