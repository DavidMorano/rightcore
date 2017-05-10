/* hostequiv */

/* rough equivalent host check */


#define	CF_DEBUG	0


/* revision history:

	= 96/11/21, David A­D­ Morano

	This program was started by copying from the RSLOW program.


	= 96/12/12, David A­D­ Morano

	I modified the program to take the username and password
	from a specified file (for better security).


*/



/**************************************************************************

	Call as :



**************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stropts.h>
#include	<poll.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>

#include	"localmisc.h"



/* local defines */

extern char	*strshrink() ;
extern char	*strbasename() ;


/* external variables */


/* forward subroutines */


/* local variables */






/* compare host names */
int hostequiv(h1,h2,localdomain)
char	h1[], h2[] ;
char	localdomain[] ;
{
	int	f_h1 = FALSE, f_h2 = FALSE ;
	int	len1, len2 ;

	char	*cp, *cp1, *cp2 ;


	if ((cp1 = strchr(h1,'.')) != NULL) f_h1 = TRUE ;

	if ((cp2 = strchr(h2,'.')) != NULL) f_h2 = TRUE ;

	if (LEQUIV(f_h1,f_h2))
	    return (! strcasecmp(h1,h2)) ;

	if (f_h1) {

	    len1 = cp1 - h1 ;
	    len2 = strlen(h2) ;

	    if (len1 != len2) return FALSE ;

	    cp1 += 1 ;
	    if (strcasecmp(cp1,localdomain) != 0) return FALSE ;

	    return (strncasecmp(h1,h2,len1) == 0) ;

	}

	len1 = strlen(h1) ;

	len2 = cp2 - h2 ;
	if (len1 != len2) return FALSE ;

	cp2 += 1 ;
	if (strcasecmp(cp2,localdomain) != 0) return FALSE ;

	return (strncasecmp(h1,h2,len2) == 0) ;
}
/* end subroutine */

