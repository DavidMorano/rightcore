/* isheader */

/************************************************************************
 *									
	David A.D. Morano
	- 95/10/06

	The subroutine 'isheader' checks the first word in a line to
	see if the line is a header line.  This subroutine returns
	just TRUE or FALSE.


**************************************************************************/



#include	<string.h>
#include	<stdio.h>
#include	<ctype.h>

#include	"localmisc.h"



/* the super fast header scanner ! */

int isheader(line)
char	line[] ;
{
	int	i ;

	char	*cp ;


	if (line[0] == '\n') return FALSE ;

	if (ISWHITE(line[0])) return TRUE ;

	if (strncmp(line,"From",4) == 0) return TRUE ;

	if (strncmp(line,">From",5) == 0) return TRUE ;

	if (strncmp(line,">>From",6) == 0) return TRUE ;

	cp = line ;
	while (*cp && (! ISWHITE(*cp)) && (*cp != ':')) cp += 1 ;

	if (*cp == ':') return TRUE ;

	while (ISWHITE(*cp)) cp += 1 ;

	if (*cp == ':') return TRUE ;

	return FALSE ;
}
/* end wubroutine (isheader) */


