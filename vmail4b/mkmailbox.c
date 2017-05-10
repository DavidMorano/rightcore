/* mkmailbox */

/* make a mailbox file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1994-01-23, David A­D­ Morano
        This module was copied and modified from the VMAIL original. A variety
        of enhancements were made to prevent it from crashing due to short
        buffers. No checks were being made about whether a copy into a buffer
        was overflowing! Yes, this was one of the causes of the spread of the
        1988 Internet worm. Of course, nobody likes a program that crashes
        either (including myself). It was the crashing of this (and other)
        programs that lead me to fix this crap up in the first place!

	= 1996-06-18, David A­D­ Morano
	I did:
		- remove old mail spool locks

	= 1996-07-24, David A­D­ Morano
	I rewrote the "getnewmail" subroutine in part to :
		- lock user's "new" mailbox when fetching new mail
		  NOTE: This has to be removed when real proper
			mailbox handling is implemented.
		- guard against corrupted "new" mailbox on new mail
		  from file system full
		- added full binary compatibility for new mail

	= 2007-11-13, David A­D­ Morano
        Oh man! How long have I been toiling with this thing? I added the
        ability to grab mail from multiple users. I also rewrote from the
        original (before I started cleaning up this crap in 1994) much of the
        way that this process takes place.

*/

/* Copyright © 1994,1996,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This subroutine creates (makes) a mailbox file.

	Synopsis:

	int mkmailbox(pip,mbname)
	struct proginfo	*pip ;
	const char	mbname[] ;

	Arguments:

	pip		program information pointer
	mbname		mailbox-name to setup

	Returns:

	>=0	created
	<0	failed


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_FLAGS		(O_WRONLY | O_CREAT)


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;


/* external variables */


/* local (forward) subroutines */


/* local variables */


/* exported subroutines */


int mkmailbox(pip,mbname)
struct proginfo	*pip ;
const char	mbname[] ;
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		fd ;
	char		mbfname[MAXNAMELEN + 1] ;

	if (pip == NULL) return SR_FAULT ;
	if (mbname == NULL) return SR_FAULT ;

	if (mbname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("mkmailbox: mbname=%s\n",mbname) ;
#endif

	if (pip->folderdname == NULL)
	    goto ret0 ;

	if (pip->folderdname[0] == '\0')
	    goto ret0 ;

	rs = mkpath2(mbfname,pip->folderdname,mbname) ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_stat(mbfname,&sb) ;
	if (rs < 0) {

	    rs = u_open(mbfname,O_FLAGS,0664) ;
	    fd = rs ;
	    if (rs >= 0) {

	        if (pip->gid_mail > 0)
	            u_fchown(fd,-1,pip->gid_mail) ;

	        u_close(fd) ;

	    } /* end if (opened) */

	} /* end if (file didn't exist) */

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("mkmailbox: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mkmailbox) */


