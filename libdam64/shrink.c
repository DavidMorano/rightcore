/* shrink */

/* remove leading and trailing white space */



/****************************************************************************

	This subroutine will identify the non-white-space portion of
	the buffer by ignoring white at the beginning of the end of the
	given buffer.  No modifications to the buffer are made.

	Synopsis:

	int shrink(s,sl,rp)
	const char	*s ;
	int		sl ;
	char		**rp ;

	Arguments:

	+ buffer
	+ buffer length
	+ result buffer pointer

	Returns:

	+ non-white-space string length (if OK), otherwise BAD


****************************************************************************/



#include	<sys/types.h>
#include	<string.h>
#include	<ctype.h>

#include	"localmisc.h"



/* local defines */






int shrink(s,sl,rpp)
const char	*s ;
int		sl ;
char		**rpp ;
{


/* remove leading white-space */

	if (sl < 0) {

	    while (isspace(*s)) 
		s += 1 ;

	    sl = strlen(s) ;

	} else {

	    while ((sl > 0) && (isspace(*s))) {
	        s += 1 ;
	        sl -= 1 ;
	    } /* end while */

	} /* end if */

/* remove trailing white-space */

	while ((sl > 0) && isspace(s[sl - 1]))
	    sl -= 1 ;

/* return */

	if (rpp != NULL)
	    *rpp = s ;

	return sl ;
}
/* end subroutine (shrink) */



