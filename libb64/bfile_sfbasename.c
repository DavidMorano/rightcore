/* bfile_sfbasename */

/* get the base file name out of a path */


#define	CF_DEBUGS	0


/* revision history:

	= 93/07/17, David A­D­ Morano

	This subroutine was written for PPI emulator firmware.


*/


/************************************************************************

	This routine returns the length of the base name portion
	of the given path string.

	Arguments:

	+ s	given path string
	+ len	length of given path string (can be -1)
	+ rp	result pointer of beginning of found string

	Returns:

	+	length of found string


************************************************************************/


#include	"localmisc.h"
#include	"bfile.h"





int bfile_sfbasename(s,len,rp)
char	*s ;
int	len ;
char	**rp ;
{
	int	si ;
	int	sl ;


	if (len < 0)
		len = strlen(s) ;

	sl = len ;

/* remove trailing slash characters */

	while ((sl > 0) && (s[sl - 1] == '/'))
		sl -= 1 ;

/* find the next previous slash character (if there is one) */

	for (si = sl ; si > 0 ; si -= 1) {

	    if (s[si - 1] == '/') break ;

	}

	if (rp != NULL)
		*rp = s + si ;

	return (sl - si) ;
}
/* end subroutine (bfile_sfbasename) */



