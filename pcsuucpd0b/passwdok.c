/* passwdok */

/* check a password for validity */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_FCRYPT	1		/* use 'des_fcrypt()' */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was written from scratch to do what the previous
	program by the same name did.


*/


/******************************************************************************

	This subroutine will take as input a cleartext passwd and an
	encypted password.  It will return a boolean about whether or
	not the password is valid on the current system.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<des.h>

#include	"localmisc.h"



/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	MAXPATHLEN
#endif



/* external subroutines */


/* external variables */


/* forward references */


/* external variables */


/* local variables */


/* exported subroutines */


int passwdok(password,pw_stored)
const char	password[] ;
const char	pw_stored[] ;
{
	int	c1, c2 ;
	int	f_passed = FALSE ;
	int	f_cmp ;

	char	cryptbuf[40] ;
	char	*pw_given ;


#if	CF_DEBUGS
	debugprintf("passwdok: password=>%s<\n",password) ;
	debugprintf("passwdok: pw_stored=>%s<\n",pw_stored) ;
#endif

	c1 = strlen(password) ;

	c2 = strlen(pw_stored) ;

#if	CF_FCRYPT
	pw_given = des_fcrypt(password, pw_stored,cryptbuf) ;
#else
	pw_given = crypt(password, pw_stored) ;
#endif /* CF_FCRYPT */

#if	CF_DEBUGS
	debugprintf("passwdok: pw_given=>%s<\n",pw_given) ;
#endif

	f_cmp = (strcmp(pw_stored, pw_given) == 0) ;

/* if both the supplied and the stored passwords are zero length, allow it */

	if (f_cmp || ((c1 == 0) && (c2 == 0)))
	    f_passed = TRUE ;

#if	CF_DEBUGS
	debugprintf("passwdok: ret f_passed=%u\n",f_passed) ;
#endif

	return f_passed ;
}
/* end subroutine (passwdok) */



