/* uc_msgdiscard */

/* interface component for UNIX® library-3c */
/* set the message-discard mode on the file descriptor */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/conf.h>
#include	<sys/stat.h>
#include	<sys/uio.h>
#include	<stropts.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* forward references */


/* exported subroutines */


int uc_msgdiscard(fd)
int	fd ;
{
	int	rs ;


#if	SYSHAS_STREAMS
	rs = u_ioctl(fd,I_SRDOPT,RMSGD) ;
#else
	rs = SR_NOSYS ;
#endif

	return rs ;
}
/* end subroutine (uc_msgdiscard) */


