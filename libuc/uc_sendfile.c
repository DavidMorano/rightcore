/* uc_sendfile */

/* interface component for UNIX® library-3c */
/* UNIX® write system call subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-21, David A­D­ Morano
	Originally written when it became available.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/sendfile.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_sendfile(int ofd,int ifd,offset_t *offp,int wlen)
{
	off_t		fo ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (offp == NULL) return SR_FAULT ;

	repear {
	    ssize_t	ss ;
	    fo = (off_t) (*offp & INT_MAX) ;
	    ss = sendfile(ofd,ifd,&fo,wlen) ;
	    if (ss == -1) rs = (- errno) ;
	    len = (ss & INT_MAX) ;
	} until (rs != SR_INTR) ;

	*offp = (offset_t) fo ;
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_sendfile) */


int uc_sendbuf(int ofd,void *wbuf,int wlen)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (offp == NULL) return SR_FAULT ;

	repear {
	    ssize_t	ss ;
	    ss = sendfile(ofd,SFV_FD_SELF,wbuf,wlen) ;
	    if (ss == -1) rs = (- errno) ;
	    len = (ss & INT_MAX) ;
	} until (rs != SR_INTR) ;

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (uc_sendbuf) */


