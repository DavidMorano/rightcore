/* mesg */

/* enable or disable messaging on the user's terminal device */


/* revision history:

	= 1998-07-22, David A­D­ Morano

	This subroutine was originally written.  Unfortunately, it depends
	on the controlling terminal still being (or being) available on
	one of the file descriptors between 0 and 2 inclusive.	This may
	be a difficult thing to arrange for in certain circumstances!


	= 1999-01-10, David A­D­ Morano

	This subroutine was enhanced to use the POSIX 'ttyname_r()'
	subroutine.


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/****************************************************************************

 * mesg -- set current tty to accept or
 *	forbid write permission.
 *
 *	mesg (flag)
 *		!= 0	allow messages
 *		0	forbid messages

 *	return codes
 *		1  if messages were previously ON
 *		0  if messages were previously OFF
 *		<0	if error
 

****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */


/* exported subroutines */


int mesg(flag)
int	flag ;
{
	struct ustat	sb ;

	int	rs ;
	int	i, rc ;

	char	ttybuf[MAXPATHLEN + 1], *tty = ttybuf ;


	for (i = 0 ; i < 3 ; i += 1) {

#if	defined(SYSHAS_TTYNAMER) && SYSHAS_TTYNAMER
	    rs = ttyname_r(i,ttybuf,MAXPATHLEN) ;
#else
	    rs = SR_OK ;
	    if ((tty = ttyname(i)) == NULL)
	        rs = SR_NOENT ;
#endif

	    if (rs >= 0)
	        break ;

	} /* end for */

	rs = u_stat(tty, &sb) ;
	if (rs < 0) goto ret0 ;

	rc = (sb.st_mode & S_IWGRP) ? 1 : 0 ;

	if (flag) {

	    if (! rc) {

	        sb.st_mode |= S_IWGRP ;
	        rs = u_chmod(tty,sb.st_mode) ;

	    }

	} else {

	    if (rc) {

	        sb.st_mode &= (~ S_IWGRP) ;
	        rs = u_chmod(tty,sb.st_mode) ;

	    }

	} /* end if */

ret0:
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (mesg) */



