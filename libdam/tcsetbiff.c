/* tcsetbiff */

/* UNIX® terminal-control "set-biffing" */


/* revision history:

	= 1998-08-20, David A­D­ Morano
	This is new code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is an attempt at abstracting how to set the state of the terminal
        with regard to biffing.

	Synopsis:

	int tcsetbiff(fd,f_new)
	int	fd ;
	int	f_new ;

	Arguments:

	fd		file-descriptor of terminal
	f_new		new TRUE or FALSE setting biffing

	Returns:

	<0		error
	>=0		previous TRUE or FALSE setting


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


int tcsetbiff(int fd,int f_new)
{
	struct ustat	sb ;
	int		rs ;
	int		f_old = FALSE ;

	if (fd < 0)
	    return SR_NOTOPEN ;

	if ((rs = u_fstat(fd,&sb)) >= 0) {
	    mode_t	m_old = sb.st_mode ;
	    f_old = (m_old & S_IWGRP) ;
	    if (! LEQUIV(f_old,f_new)) {
	        mode_t	m_new ;
	        if (f_new) {
	            m_new = (m_old | S_IXUSR) ;
	        } else {
	            m_new = (m_old & (~ S_IXUSR)) ;
		}
	        rs = u_fchmod(fd,m_new) ;
	    } /* end if (old and new were different) */
	} /* end if (stat) */

	return (rs >= 0) ? f_old : rs ;
}
/* end subroutine (tcsetbiff) */


