/* expandline */

/* subroutine to expand a line out for TROFF rendering */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_PREFIX	1		/* prefix? */


/* revision history:

	= 1987-09-10, David A­D­ Morano
	This subroutine was originally written for the TEXTSET program.

*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will take an input line and expand it out so that it
        will render correctly under TROFF processing. This subroutine also
        peforms standard tab expansion to tabstops at every eight (8) columns.

	Synopsis:
	int expandline(char *rbuf,int elen,cchar *ibp,int il,int *flagp)

	Arguments:
	- ibp		input buffer pointer
	- ilen		input buffer length
	- obp		output buffer pointer
	- olen		output buffer length
	- flags		flags to control the operation

	Returns:
	-		length of output line processed


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* local structures */


/* forward references */


/* exported variables */


/* local variables */


/* exported subroutines */


int expandline(char *obp,int ol,cchar *ibp,int il,int *flagp)
{
	int		c ;
	int		j, k ;
	int		i = 0 ;
	int		l = 0 ;
	int		op = 0  ;
	int		cur_l = 0 ;
	int		last_l = 0 ;
	char		*cur_obp = NULL ;
	char		*last_obp = NULL ;


	*flagp = FALSE ;
	if (ol < 2)
	    return -1 ;

	if (il && ((*ibp == '.') || (*ibp == '\''))) {

	    *flagp = TRUE ;
#if	CF_PREFIX
	    *obp++ = '\\' ;
	    *obp++ = '&' ;
	    ol -= 2 ;
#endif

	} /* end if */

	for (i = 0 ; (i < il) && (l < (ol - 8)) ; i += 1) {

	    last_obp = cur_obp ;
	    cur_obp = obp ;

	    last_l = cur_l ;
	    cur_l = l ;

	    c = *ibp ;
	    if (c == 0)  {

	        ibp += 1 ;

	    } else if (c == '\\') {

	        *obp++ = *ibp++ ;
	        *obp++ = '\\' ;
	        l += 2 ;
	        op += 1 ;

	    } else if (c == '\t') {

	        k = 8 - (op & 7) ;
	        for (j = 0 ; j < k ; j += 1) {
	            *obp++ = ' ' ;
	        }

	        l += k ;
	        ibp += 1 ;
	        op += k ;

	    } else if ((c & 0x7F) == '\010') {

	        ibp += 1 ;
	        if (l > 0) {
	            l = last_l ;
	            op -= 1 ;
	            obp = last_obp ;
	        }

	    } else if (c == '\n') {

	        *obp++ = *ibp++ ;
	        l += 1 ;
	        op += 1 ;

	    } else if (((c & 0x7F) < 0x20) || (c == 0x7F)) {

	        *obp++ = '\\' ;
	        *obp++ = '(' ;
	        *obp++ = 's' ;
	        *obp++ = 'q' ;
	        ibp += 1 ;
	        l += 4 ;
	        op += 1 ;

	    } else {

	        *obp++ = *ibp++ ;
	        l += 1 ;
	        op += 1 ;

	    }

	} /* end for */

	return l ;
}
/* end subroutine (expandline) */


