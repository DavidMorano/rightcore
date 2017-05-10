/* cmppasswd */

/* check a password for validity */


#define	CF_DEBUGS	0


/* revision history:

	= David A.D. Morano, March 1996
	The program was written from scratch to do what
	the previous program by the same name did.

*/


/******************************************************************************

	This module will take as input a cleartext passwd and an
	encypted password.  It will return a boolean about whether or
	not the password is valid on the current system.



******************************************************************************/




#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<pwd.h>
#include	<shadow.h>
#include	<grp.h>
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>
#include	<des.h>

#include	"localmisc.h"



/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	MAXPATHLEN
#endif



/* external subroutines */


/* local forward references */


/* external variables */


/* global variables */


/* local variables */





int cmppasswd(password,pw_stored)
char	password[], pw_stored[] ;
{
	int	c1, c2 ;
	int	f_passed = FALSE ;
	int	f_cmp ;

	char	cryptbuf[40] ;
	char	*pw_given ;


	c1 = strlen(password) ;

	c2 = strlen(pw_stored) ;

	pw_given = des_fcrypt(password, pw_stored,cryptbuf) ;

	f_cmp = (strcmp(pw_stored, pw_given) == 0) ;

/* if both the supplied and the stored passwords are zero length, allow it */

	if (((c1 == 0) && (c2 == 0)) || f_cmp)
	    f_passed = TRUE ;


	return (f_passed) ? OK : BAD ;
}
/* end subroutine (cmppasswd) */



