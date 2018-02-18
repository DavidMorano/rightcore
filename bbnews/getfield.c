/* getfield */

/* an old subroutine to get extra message header fields */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before. In other words, the same old story : old
        garbage code needing to be replaced in entirety just to get something
        working with new code interfaces. Can't we all think "reusuable" ??

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
		David A.D. Morano

*									
*	FUNCTIONAL DESCRIPTION:						

*	The 'getfield' reads a file for a key descriptor and returns	
*	the value contained in it.
*									
*	PARAMETERS:							
*	file		filename					
*	key		key name (eg, 'FROM')				
*	valbuf		value for given key
	valbuflen	length of value buffer above
*									
*	RETURNED VALUE:							
		BAD	no header found
		OK	a header was found
*									
*	SUBROUTINES CALLED:						
		+ mm_getfield


	IMPORTANT NOTE:
	This subroutine is NOT RFC-822 compliant !!
	This is because 'mm_getfield()' is NOT RFC-822 compliant.
	We call that to do our work.


*									
*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<localmisc.h>


/* external subroutines */

extern int	mm_getfield() ;


/* external variables */


/* exported subroutines */


int getfield(fname,key,vbuf,vlen)
const char	fname[] ;
const char	key[] ;
char		vbuf[] ;
int		vlen ;
{
	bfile	mmfile, *fp = &mmfile ;

	int	rs ;

	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	vbuf[0] = '\0' ;

	if ((rs = bopen(fp,fname,"r",0666)) >= 0) {
	    struct ustat	sb ;

	    if ((rs = bcontrol(fp,BC_STAT,&sb)) >= 0) {
		int	fsize = (sb.st_size & INT_MAX) ;
		rs = mm_getfield(fp,0L,fsize,key,vbuf,vlen) ;
	    } /* end if */

	    bclose(fp) ;
	} /* end if (open-file) */

	return rs ;
}
/* end subroutine (getfield) */



