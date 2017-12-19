/* vecstr_avmkstr */

/* make the Array-Vector and the String-table */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines are used when the caller wants to store a COPY of the
	passed string data into a vector.  These routines will copy and store
	the copied data in the list.  The advantage is that the caller does not
	have to keep the orginal data around in order for the list data to be
	accessed later.  String data (unlike "element" data) can not contain
	NULL characters-bytes.


*******************************************************************************/


#define	VECSTR_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecstr.h"


/* local defines */


/* external subroutines */

extern int	iceil(int,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */


/* local variables */


/* exported subroutines */


int vecstr_avmkstr(vecstr *op,cchar **av,int avsize,char *tab,int tabsize)
{
	int		rs = SR_OK ;
	int		size ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (av == NULL) return SR_FAULT ;
	if (tab == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

/* check supplied tabsize */

	if (op->stsize == 0) {
	    vecstr_strsize(op) ;
	}

	size = iceil(op->stsize,sizeof(int)) ;

	if (tabsize >= size) {
	    size = (op->c + 1) * sizeof(int) ;
	    if (avsize >= size) {
		int	i ;
		char	*bp = tab ;
	        *bp++ = '\0' ;
	        for (i = 0 ; op->va[i] != NULL ; i += 1) {
	            if (op->va[i] != NULL) {
	                av[c++] = bp ;
	                bp = strwcpy(bp,op->va[i],-1) + 1 ;
		    }
	        } /* end for */
	        av[c] = NULL ;
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_avmkstr) */


