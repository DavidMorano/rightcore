/* uc_getpw */

/* interface component for UNIX® library-3c */
/* password DB access */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SOLARISBUG	1		/* Solaris® errno bad on access */


/* revision history:

	= 1998-03-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine was written so that we could use a single interface to
	access the 'passwd' database on all UNIX® platforms.  This code module
	provides a platform independent implementation of UNIX® 'passwd'
	database access subroutines.

	Note:

	The non-reentrant versions of these subroutines do NOT set 'errno'.


*******************************************************************************/


#undef	LOCAL_SOLARIS
#define	LOCAL_SOLARIS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<passwdent.h>
#include	<localmisc.h>


/* local defines */

#define	TO_AGAIN	10

#define	RF_PWCOPY	0		/* flag to set need of 'pwcopy()' */
#if	defined(SYSHAS_GETPWENTR) && (SYSHAS_GETPWENTR > 0)
#else
#undef	RF_PWCOPY
#define	RF_PWCOPY	1		/* flag to set need of 'pwcopy()' */
#endif
#if	defined(SYSHAS_GETPWXXXR) && (SYSHAS_GETPWXXXR > 0)
#else
#undef	RF_PWCOPY
#define	RF_PWCOPY	1		/* flag to set need of 'pwcopy()' */
#endif


/* external subroutines */


/* external variables */

extern int	msleep(int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int uc_setpwent()
{
	setpwent() ;
	return SR_OK ;
}
/* end subroutine (uc_setpwent) */

int uc_endpwent()
{
	endpwent() ;
	return SR_OK ;
}
/* end subroutine (uc_endpwent) */

int uc_getpwent(struct passwd *pwp,char rbuf[],int rlen)
{
	struct passwd	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getpwent: pwp{%p}\n",pwp) ;
	debugprintf("uc_getpwent: rbuf{%p}\n",rbuf) ;
	debugprintf("uc_getpwent: rlen=%d\n",rlen) ;
#endif

	if (pwp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

/* note (carefully) that there is NO POSIX® standard version of this funtion */

	repeat {

#if	defined(SYSHAS_GETPWENTR) && (SYSHAS_GETPWENTR > 0)
	{
	    errno = 0 ;
	    rp = getpwent_r(pwp,rbuf,rlen) ;
	    if (rp != NULL) {
	        rs = passwdent_size(pwp) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#else /* defined(SYSHAS_GETPWENTR) && (SYSHAS_GETPWENTR > 0) */
	{
	    errno = 0 ;
	    rp = getpwent() ;
	    if (rp != NULL) {
	        rs = passwdent_load(pwp,rbuf,rlen,rp) ;
	    } else {
	        rs = (- errno) ;
	    }
	}
#endif /* defined(SYSHAS_GETPWENTR) && (SYSHAS_GETPWENTR > 0) */

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
		    break ;
	        case SR_INTR:
	            break ;
	        default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("uc_getpwent: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_getpwent) */

int uc_getpwnam(cchar name[],struct passwd *pp,char rbuf[],int rlen)
{
	struct passwd	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

#if	CF_DEBUGS
	debugprintf("uc_getpwnam: ent name=%s\n",name) ;
#endif

	repeat {

#if	defined(SYSHAS_GETPWXXXR) && (SYSHAS_GETPWXXXR > 0)
	{
	    rp = NULL ;
	    errno = 0 ;
	    rs = - getpwnam_r(name,pp,rbuf,rlen,&rp) ;
	    if (rs != 0) {
	        rs = (- errno) ;
	    } else if (rp == NULL) {
	        rs = SR_NOTFOUND ;
	    }
	    if (rs >= 0) {
	        rs = passwdent_size(pp) ;
	    }
	}
#else /* defined(SYSHAS_GETPWXXXR) && (SYSHAS_GETPWXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getpwnam(name) ;
	    if (rp != NULL) {
	        rs = passwdent_load(pp,rbuf,rlen,rp) ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	}
#endif /* defined(SYSHAS_GETPWXXXR) && (SYSHAS_GETPWXXXR > 0) */

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
		    break ;
	        case SR_INTR:
	            break ;
	        default:
		    f_exit = TRUE ;
		    break ;
	        } /* end switch */
	    } /* end if (error) */
	} until ((rs >= 0) || f_exit) ;

#if	CF_DEBUGS
	debugprintf("uc_getpwnam: ret rs=%d\n",rs) ;
	debugprintf("uc_getpwnam: username=%s\n",pp->pw_name) ;
#endif

	return rs ;
}
/* end subroutine (uc_getpwnam) */

int uc_getpwuid(uid_t uid,struct passwd *pp,char rbuf[],int rlen)
{
	struct passwd	*rp ;
	int		rs = SR_OK ;
	int		to_again = TO_AGAIN ;
	int		f_exit = FALSE ;

	repeat {

#if	defined(SYSHAS_GETPWXXXR) && (SYSHAS_GETPWXXXR > 0)
	{
	    rp = NULL ;
	    errno = 0 ;
	    rs = - getpwuid_r(uid,pp,rbuf,rlen,&rp) ;
	    if (rs != 0) {
	        rs = (- errno) ;
	    } else if (rp == NULL) {
	        rs = SR_NOTFOUND ;
	    }
	    if (rs >= 0) {
	        rs = passwdent_size(pp) ;
	    }
	}
#else /* defined(SYSHAS_GETPWXXXR) && (SYSHAS_GETPWXXXR > 0) */
	{
	    errno = 0 ;
	    rp = getpwuid(uid) ;
	    if (rp != NULL) {
	        rs = passwdent_load(pp,rbuf,rlen,rp) ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	}
#endif /* defined(SYSHAS_GETPWXXXR) && (SYSHAS_GETPWXXXR > 0) */

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
/* end subroutine (uc_getpwuid) */


