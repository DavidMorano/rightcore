/* uc_getgr */

/* interface component for UNIX® library-3c */
/* subroutines to access GROUP databases */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SOLARISBUG	1		/* Solaris® errno bad on access */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine was written so that we could use a single interface to
	access the GROUP database on all UNIX® platforms.  This code module
	provides a platform independent implementation of UNIX® GROUP database
	access subroutines.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<storeitem.h>
#include	<groupent.h>
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


int uc_setgrent()
{
	setgrent() ;
	return SR_OK ;
}

int uc_endgrent()
{
	endgrent() ;
	return SR_OK ;
}

int uc_getgrent(struct group *grp,char rbuf[],int rlen)
{
	struct group	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getgrent: ent\n") ;
#endif

	repeat {

#if	defined(SYSHAS_GETGRENTR) && (SYSHAS_GETGRENTR > 0)
	{
	    errno = 0 ;
	    rp = getgrent_r(grp,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = groupent_size(grp) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETGRENTR) && (SYSHAS_GETGRENTR > 0) */
	{
	    errno = 0 ;
	    rp = getgrent() ;
	    if (rp != NULL) {
	        rs = groupent_load(grp,rbuf,rlen,rp) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#endif /* defined(SYSHAS_GETGRENTR) && (SYSHAS_GETGRENTR > 0) */

#if	CF_SOLARISBUG
		if (rs == SR_BADF) rs = SR_NOENT ;
#endif /* CF_SOLARISBUG */

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

	return rs ;
}
/* end subroutine (uc_getgrent) */

int uc_getgrnam(cchar *name,struct group *grp,char *rbuf,int rlen)
{
	struct group	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {

#if	defined(SYSHAS_GETGRXXXR) && (SYSHAS_GETGRXXXR > 0)
	{
	    rp = NULL ;
	    errno = 0 ;
	    rs = - getgrnam_r(name,grp,rbuf,rlen,&rp) ;
	    if (rs != 0) {
	        rs = (- errno) ;
	    } else if (rp == NULL) {
	        rs = SR_NOTFOUND ;
	    }
	    if (rs >= 0) {
	        rs = groupent_size(grp) ;
	    }
	}
#else /* defined(SYSHAS_GETGRXXXR) && (SYSHAS_GETGRXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getgrnam(name) ;
	    if (rp != NULL) {
	        rs = groupent_load(grp,rbuf,rlen,rp) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#endif /* defined(SYSHAS_GETGRXXXR) && (SYSHAS_GETGRXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
	                f_exit = TRUE ;
		    }
		    break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_getgrnam) */

int uc_getgrgid(gid_t gid,struct group *grp,char rbuf[],int rlen)
{
	struct group	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {

#if	defined(SYSHAS_GETGRXXXR) && (SYSHAS_GETGRXXXR > 0)
	{
	    rp = NULL ;
	    errno = 0 ;
	    rs = - getgrgid_r(gid,grp,rbuf,rlen,&rp) ;
	    if (rs != 0) {
	        rs = (- errno) ;
	    } else if (rp == NULL) {
	        rs = SR_NOTFOUND ;
	    }
	    if (rs >= 0) {
	        rs = groupent_size(grp) ;
	    }
	}
#else /* defined(SYSHAS_GETGRXXXR) && (SYSHAS_GETGRXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getgrgid(gid) ;
	    if (rp != NULL) {
	        rs = groupent_load(grp,rbuf,rlen,rp) ;
	    } else {
	        rs = (errno == 0) ? SR_NOTFOUND : (- errno) ;
	    }
	}
#endif /* defined(SYSHAS_GETGRXXXR) && (SYSHAS_GETGRXXXR > 0) */

	    if (rs < 0) {
	        switch (rs) {
	        case SR_AGAIN:
	            if (to_again-- > 0) {
	                msleep(1000) ;
		    } else {
			f_exit = TRUE ;
	            }
		    break ;
	        case SR_INTR:
	            break ;
		default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

	return rs ;
}
/* end subroutine (uc_getgrgid) */


