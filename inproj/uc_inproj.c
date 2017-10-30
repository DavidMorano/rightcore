/* uc_inproj */

/* interface component for UNIX® library-3c */
/* project DB access */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine was written so that we could use a single interface to
	access the 'passwd' database on all UNIX® platforms.  This code module
	provides a platform independent implementation of UNIX® 'passwd'
	database access subroutines.

	Symopsis:

	int uc_inproj(cchar *username,cchar *projname,char *rbuf,int rlen)

	Arguments:

	username	user to check
	projname	project to check
	rbuf		buffer to hold internal project entry data
	rlen		length of supplied buffer

	Returns:

	<0		error
	==0		NO (user not in project)
	>0		YES (user is in project)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<project.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	10


/* external subroutines */

extern int	msleep(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0)

int uc_inproj(cchar *username,cchar *projname,char *rbuf,int rlen)
{
	int		rs ;
	int		to_again = TO_AGAIN ;
	int		f_inproj = FALSE ;
	int		f_exit = FALSE ;

	repeat {
	    rs = SR_OK ;
	    errno = 0 ;
	    f_inproj = inproj(username,projname,rbuf,(size_t) rlen) ;
	    if (errno != 0) rs = (- errno) ;
	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
		        f_exit = TRUE ;
		    }
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return (rs >= 0) ? f_inproj : rs ;
}
/* end subroutine (uc_inproj) */


#else /* defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0) */


int uc_inproj(name,pjp,buf,buflen)
const char	name[] ;
struct project	*pjp ;
char		buf[] ;
int		buflen ;
{


	return SR_NOSYS ;
}
/* end subroutine (uc_inproj) */


#if	(! defined(SUBROUTINE_GETPROJID)) || (SUBROUTINE_GETPROJID == 0)
#define	SUBROUTINE_GETPROJID	1
projid_t getprojid(void) { return 0 } ;
#endif /* (! defined(SUBROUTINE_GETPROJID)) || (SUBROUTINE_GETPROJID == 0) */


#if	(! defined(SUBROUTINE_GETTASKID)) || (SUBROUTINE_GETTASKID == 0)
#define	SUBROUTINE_GETTASKID	1
taskid_t gettaskid(void) { return 0 } ;
taskid_t settaskid(projid_t pjid,uint_t flags) { return 0 } ;
#endif /* (! defined(SUBROUTINE_GETTASKID)) || (SUBROUTINE_GETTASKID == 0) */


#endif /* defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0) */


