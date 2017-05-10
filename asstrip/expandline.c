/* expandline */

/* subroutine to expand a line out for TROFF rendering */


#define	F_DEBUG		1


/*
	= David A.D. Morano, September 1987
	This subroutine was originally written for the
	TEXTSET program.


*/


/*******************************************************************

	This subroutine will take an input line and expand it out
	so that it will render correctly under TROFF processing.
	This subroutine also peforms standard tab expansion to
	tabstop at every eight (8) columns.
	

	Arguments :
	- ibp		input buffer pointer
	- ilen		input buffer length
	- obp		output buffer pointer
	- olen		output buffer length
	- flags		flags to control the operation

	Returns :
	length of output line processed



*********************************************************************/




#include	<sys/types.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>

#include	"misc.h"



/* local defines */

#define	F_PREFIX	1



/* externals */


/* forward references */


/* local structures */


/* global data */


/* local data */




int expandline(ibp,il,obp,ol,flagp)
char	*ibp, *obp ;
int	il, ol ;
int	*flagp ;
{
	int	c, op = 0  ;
	int	i = 0, j, k, l = 0 ;
	int	f_escape = FALSE ;
	int	cur_l, last_l ;

	char	*cur_obp, *last_obp ;


		*flagp = FALSE ;
	if (ol < 2) return -1 ;

	if (il && ((*ibp == '.') || (*ibp == '\''))) {

		*flagp = TRUE ;
#if	F_PREFIX
		*obp++ = '\\' ;
		*obp++ = '&' ;
		ol -= 2 ;
#endif

	}

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

	    } else if (((c & 0x7F) < 0x20) || ((c & 0x7F) == 0x7F)) {

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
	}

	return l ;
}
/* end subroutine (expandline) */



