/* uc_ftruncate */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/uio.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<poll.h>
#include	<errno.h>

#include	<vsystem.h>


/* local defines */

#define	TO_AGAIN	5


/* external subroutines */

extern int	msleep(int) ;


/* forward references */

static int	uc_ftruncater(int,offset_t) ;


/* exported subroutines */


int uc_ftruncate(int fd,offset_t len)
{
	int		rs = SR_OK ;
	if (len < 0) {
	    rs = u_seeko(fd,0L,SEEK_CUR,&len) ;
	}
	if (rs >= 0) {
	    rs = uc_ftruncater(fd,len) ;
	}
	return rs ;
}
/* end suboutine (uc_ftruncate) */


/* local subroutines */


static int uc_ftruncater(int fd,offset_t len)
{
	int	rs ;
	int	to_again = TO_AGAIN ;

again:
	if ((rs = ftruncate(fd,len)) < 0) rs = (- errno) ;

	if (rs < 0) {
	    switch (rs) {
	    case SR_INTR:
	        goto again ;
	    case SR_AGAIN:
	        if (to_again--) goto nap ;
	        break ;
	    } /* end switch */
	}

	return rs ;

/* take a little nap */
nap:
	msleep(1000) ;
	goto again ;
}
/* end subroutine (uc_ftruncater) */


