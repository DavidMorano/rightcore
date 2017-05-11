/* tcsetlines */

/* UNIX® terminal-control "set-lines" */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We try to set the number of lines inside the terminal driver.

	Synopsis:

	int tcsetlines(fd,lines)
	int	fd ;
	int	lines ;

	Arguments:

	fd		file-descriptor of termainal
	lines		number of lines to try to set

	Returns:

	<0		error
	>=0		success (previous number of lines)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<termios.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int tcsetlines(int fd,int lines)
{
	struct winsize	ws ;
	int		rs ;
	int		plines = 0 ;

	if (fd < 0)
	    return SR_NOTOPEN ;

	if (lines < 0) {
	    lines = 0 ;
	} else if (lines > SHORT_MAX) {
	    lines = SHORT_MAX ;
	}

	memset(&ws,0,sizeof(struct winsize)) ;
	if ((rs = u_ioctl(fd,TIOCGWINSZ,&ws)) >= 0) {
	    plines = ws.ws_row ;
	    ws.ws_row = (short) lines ;
	    rs = u_ioctl(fd,TIOCSWINSZ,&ws) ;
	}

	return (rs >= 0) ? plines : rs ;
}
/* end subroutine (tcsetlines) */


