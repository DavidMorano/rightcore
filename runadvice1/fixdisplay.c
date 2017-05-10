/* fixdisplay */

/* fix up the display variable */


#define	CF_DEBUGS	0


/* revision history:

	- 93/10/01, David A­D­ Morano

	This program was originally written.


	- 96/02/01, David A­D­ Morano

	This program was pretty extensively modified to take
	much more flexible combinations of user supplied paramters.


*/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<sys/wait.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"bfile.h"



/* exported subroutines */


void fixdisplay(host)
char	host[] ;
{
	struct utsname		uts ;

	int	len ;

	char	*hp ;
	char	*cp, *cp2 ;


/* fix up the environment variable DISPLAY, if found */

	if ((cp = getenv("DISPLAY")) == NULL) 
		return ;

/* is an adjustment needed ? */

	if (cp[0] == ':') {

	    hp = host ;
	    if ((host == NULL) || (host[0] == '\0')) {

	        uname(&uts) ;

	        if ((cp2 = strchr(uts.nodename,'.')) != NULL)
	            *cp2 = '\0' ;

	        hp = uts.nodename ;

	    }

	    len = strlen(hp) ;

	    cp2 = (char *) malloc(len + 30) ;

	    sprintf(cp2,"DISPLAY=%s%s",hp,cp) ;

	    putenv(cp2) ;

	}

}
/* end subroutine (fixdisplay) */



