/* getpassword */

/* read a password from the user's terminal */


#define	CF_DEBUGS	0		/* run-time debugging */
#define	CF_DEBUGSHEX	0
#define	CF_TRAILNL	0		/* appears to be needed */


/* revision history:

	= 1998-03-17, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a replacement for some UNIX subroutine of the same sort of
	function ('getpass(3c)').

	I forget why this one is better but most of the UNIX system
	libraries calls are crap so I really do not need to look for the
	reason.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>


/* local defines */

#ifndef	PASSWDLEN
#define	PASSWDLEN	32
#endif

#ifndef	TTYFNAME
#define	TTYFNAME	"/dev/tty"
#endif


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */

#if	CF_DEBUGS
extern int	mkhexstr(char *,int,void *,int) ;
#endif


/* local variables */


/* exported subroutines */


int getpassword(prompt,passbuf,passlen)
const char	prompt[] ;
char		passbuf[] ;
int		passlen ;
{
	struct termios	old, new ;
	int		rs ;
	int		pl ;
	int		rlen = 0 ;

/* * read and write to "/dev/tty" if possible */

	if ((rs = u_open(TTYFNAME,O_RDWR,0666)) >= 0) {
	    int	fd = rs ;
	    if ((rs = uc_tcgetattr(fd,&old)) >= 0) {

	new = old ;
	new.c_lflag &= (~ ECHO) ;
	if ((rs = uc_tcsetattr(fd,TCSADRAIN,&new)) >= 0) {

	    pl = strlen(prompt) ;
	    rs = uc_write(fd,prompt,pl,-1) ;

	    passbuf[0] = '\0' ;
	    if (rs >= 0) {
	        rs = u_read(fd,passbuf,passlen) ;
	        rlen = rs ;
	    }

#if	CF_TRAILNL
	    if (rs >= 0)
	        rs = u_write(fd, "\n", 1) ;
#endif

	    uc_tcsetattr(fd,TCSADRAIN,&old) ; /* try! */

	    } /* end if (terminal attributes) */

	    } /* end if */
	    u_close(fd) ;
	} /* end if (open) */

	if ((rs >= 0) && (rlen > 0)) {
	    if ((passbuf[rlen - 1] == '\n') || (passbuf[rlen - 1] == '\r')) {
	        rlen -= 1 ;
	    }
	    passbuf[rlen] = '\0' ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("getpassword: ret rs=%d rlen=%u\n",rs,rlen) ;
#endif

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (getpassword) */


