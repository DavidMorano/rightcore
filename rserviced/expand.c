/* version %I% last modified %G% */

/* parse out a quoted field */

/*
	David A.D. Morano
	November 1991
*/


/*****************************************************************************

	This subroutine is used to parse out a quoted field from
	a buffer which is passed as an argument.  The resultant
	(parsed out) field is placed into the return buffer provided.

	Arguments:
	- buffer to receive resultant parsed out string
	- length of buffer receiving parsed out string
	- buffer containing the string to be parsed
	- length of string to be parsed

	Returns:
	- length of parsed out string


*****************************************************************************/



#include	"localmisc.h"

#include	<ascii.h>

#include	"sattool.h"


int expand(rbuf,rlen,buf,len,sep)
char	*rbuf, *buf ;
int	rlen, len ;
struct expand	*sep ;
{
	int	elen = 0, l ;

	char	*rbp = rbuf ;
	char	*bp = buf ;
	char	*cp ;

/* fast forward to a non-blank type character */

	while ((len > 0) && ((*bp == C_SP) || (*bp == C_TAB))) {

	    bp += 1 ;
	    len -= 1 ;
	}

	if (len == 0) return 0 ;

#ifdef	COMMENT
	rlen -= 1 ;			/* reserve for zero terminator */
#endif

	while ((len > 0) && (elen < rlen)) {

	    switch ((int) *bp) {

	    case '%':
	        bp += 1 ;
	        len -= 1 ;
	        if (len == 0) return elen ;

	        switch ((int) *bp) {

	        case 'a':
	            cp = sep->a ;
	            l = strlen(cp) ;

	            break ;

	        case 's':
	            cp = sep->s ;
	            l = strlen(cp) ;

	            break ;

	        case 'f':
	            cp = sep->f ;
	            l = strlen(cp) ;

	            break ;

	        case 'm':
	            cp = sep->m ;
	            l = strlen(cp) ;

	            break ;

	        case 'd':
	            cp = sep->d ;
	            l = strlen(cp) ;

	            break ;

	        case 'l':
	            cp = sep->l ;
	            l = strlen(cp) ;

	            break ;

	        case 'c':
	            cp = sep->c ;
	            l = strlen(cp) ;

	            break ;

	        case 'r':
	            cp = sep->r ;
	            l = strlen(cp) ;

	            break ;

	        case 'w':
	            cp = sep->w ;
	            l = strlen(cp) ;

	            break ;

	        default:
	            cp = bp ;
	            l = 1 ;
	        }

	        bp += 1 ;
	        len -= 1 ;

	        if ((elen + l) > rlen) return BAD ;

	        strncpy(rbp,cp,l) ;

	        rbp += l ;
	        elen += l ;
	        break ;

	    default:
	        *rbp++ = *bp++ ;
	        elen += 1 ;
	        len -= 1 ;

	    } /* end switch */

	} /* end while */

	return elen ;
}


