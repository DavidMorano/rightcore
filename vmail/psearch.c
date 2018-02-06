/* psearch */

/* perform the scan-line searching function */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1984-11-01, David A­D­ Morano
	Originally written for PCS.

*/

/* Copyright © 1984 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<curses.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mb.h"


/* external variables */

extern struct proginfo	g ;


/* exported subroutines */


int psearch(pip,mbp,str, dir)
struct proginfo	*pip ;
struct mailbox	*mbp ;
char		*str ;
int		dir ;				/* 1: forward; 2: back */
{
	int	l, i, offset, mn ;

	char	tmp[LINEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
		debugprintf("search string is \"%s\"\n",str) ;
#endif

/* look at message curr+i mod mbp->total */

	for (i = 1 ; i < mbp->total ; i += 1) {

	    if (dir == 1) offset = i ;

	    else offset = -i ;

	    if ((mbp->current + offset) > mbp->total) offset -= mbp->total ;

	    else if ((mbp->current + offset) < 0) offset += mbp->total ;

	    mn = messord[mbp->current + offset] ;

	    fetchfield(mn, "FROM:", tmp, LINEBUFLEN) ;

	    if (strncmp(tmp, "   ", 3) == 0)
	        fetchfrom(pip,mn, tmp, LINEBUFLEN) ;

	    l = strlen(tmp) ;

	    if (substring(tmp,l,str) >= 0) 
		break ;

	    fetchfield(mn, "SUBJECT:", tmp, LINEBUFLEN) ;

	    if (substring(tmp,l,str) >= 0) 
		break ;

	    fetchfield(mn, "DATE:", tmp, LINEBUFLEN) ;

	    if (substring(tmp,l,str) >= 0) 
		break ;

	    sprintf(tmp, "%4ld", messlen[mn]) ;

	    if (substring(tmp,l,str) >= 0) 
		break ;

	} /* end for */

	if (i >= mbp->total) 
		return -1 ;

	return (mbp->current + offset) ;
}
/* end subroutine (psearch) */


