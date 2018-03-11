/* tcgetws */

/* UNIX® terminal-control "get-window-size" */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Get the window size from the terminal.

	Synopsis:

	int tcgetws(fd)
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


int tcgetws(int fd,struct winsize *wsp)
{
	int		rs ;

	if (wsp == NULL) return SR_FAULT ;

	if (fd >= 0) {
	    memset(wsp,0,sizeof(struct winsize)) ;
	    rs = u_ioctl(fd,TIOCGWINSZ,wsp) ;
	} else
	    rs = SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("tcgetws: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (tcgetws) */


