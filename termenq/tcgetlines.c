/* tcgetlines */

/* UNIX® terminal-control "get-lines" */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is an attempt at abstracting how to get the number of screen-lines
        that may have been set into the terminal driver device instance.

	Synopsis:

	int tcgetlines(fd)
	int	fd ;

	Arguments:

	fd		file-descriptor of terminal

	Returns:

	<0		error
	>=0		number of lines


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<termios.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* exported subroutines */


int tcgetlines(int fd)
{
	int		rs ;

	if (fd >= 0) {
	    struct winsize	ws ;
	    memset(&ws,0,sizeof(struct winsize)) ;
	    if ((rs = u_ioctl(fd,TIOCGWINSZ,&ws)) >= 0) {
	        rs = ws.ws_row ;
	    }
	} else
	    rs = SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("tcgetlines: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (tcgetlines) */


