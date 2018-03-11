/* nusers */

/* find number of users (logged-in) on system */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_TMPX		1		/* try TMPX */
#define	CF_UTMPX	0		/* try UTMPX */


/* revision history:

	= 1998-11-18, David A­D­ Morano
	This little subroutine was put together to get the current number of
	logged-in users without having to resort to an underlying API that may
	be different for different OSes.  Other motivations were along the same
	lines as to why the subroutine 'ncpu(3dam)' was written.  Among these
	was thread-safeness.  We completely live in a multi-threaded world; but
	we are constantly buffeted by subroutines which still think that we live
	in a single-threaded world.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We try to figure out the number of users logged into the system.  Sadly
	(tragically) not all systems support the UTMPX interface!!  Can you
	believe it?  Why can't those systems that do not support UTMPX get
	their tragic butt-acts together?

	Of course, unless the TMPX object (or some other MT-safe API) is used
	in the implementation of this subroutine (which TMPX object is not even
	presently coded up here to be conditionally compiled), it is not
	MT-safe nor reentrant.

	Notes: Why some subroutine like this was not provided by the UNIX®
	developers themselves is a mystery.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)
#include	<utmpx.h>
#endif

#include	<vsystem.h>

#if	CF_TMPX || CF_UTMPX
#include	<tmpx.h>
#endif

#include	<localmisc.h>


/* local defines */

#ifndef	UTMPXFNAME
#define	UTMPXFNAME	"/var/adm/utmpx"
#endif

#ifndef	UTMPFNAME
#define	UTMPFNAME	"/var/adm/utmp"
#endif


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


#if	CF_TMPX

int nusers(cchar *utmpfname)
{
	TMPX		ut ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("nusers-tmpx: utmpfname=%s\n",utmpfname) ;
#endif

	if ((rs = tmpx_open(&ut,utmpfname,O_RDONLY)) >= 0) {

	    rs = tmpx_nusers(&ut) ;
	    n = rs ;

	    rs1 = tmpx_close(&ut) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (tmpx) */

#if	CF_DEBUGS
	debugprintf("nusers-tmpx: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (nusers) */

#else /* CF_TMPX */

#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)

int nusers(cchar *utmpfname)
{
	struct utmpx	*up ;
	int		rs = SR_OK ;
	int		rc ;
	int		n ;
	int		f_utf = FALSE ;

	if ((utmpfname != NULL) && (utmpfname[0] != '\0')) {

#if	defined(SYSHAS_UTMPXNAME) && (SYSHAS_UTMPXNAME > 0)
	    f_utf = TRUE ;
	    rc = utmpxname(utmpfname) ;

	    rs = (rc == 1) ? SR_OK : SR_INVALID ;
#else
	    rs = SR_NOSYS ;
#endif

	} /* end if */

	n = 0 ;
	if (rs >= 0) {
	    int	f ;

	    setutxent() ;
	    while ((up = getutxent()) != NULL) {
	        f = (up->ut_type == UTMPX_TUSERPROC) ;
	        f = f && (up->ut_user[0] != '.') ;
		if (f) n += 1 ;
	    } /* end while */
	    endutxent() ;

#if	defined(SYSHAS_UTMPXNAME) && (SYSHAS_UTMPXNAME > 0)
	    if (f_utf)
	        utmpxname(UTMPXFNAME) ;
#endif

	} /* end if (ok) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (nusers) */

#else /* CF_UTMPX */

int nusers(cchar *utmpfname)
{
	struct utmp	*up ;
	int		rs = SR_OK ;
	int		rc ;
	int		n ;
	int		f_utf = FALSE ;

	if ((utmpfname != NULL) && (utmpfname[0] != '\0')) {

#if	defined(SYSHAS_UTMPNAME) && (SYSHAS_UTMPNAME > 0)
	    f_utf = TRUE ;
	    rc = utmpname(utmpfname) ;

	    rs = (rc == 1) ? SR_OK : SR_INVALID ;
#else
	    rs = SR_NOSYS ;
#endif

	} /* end if */

	n = 0 ;
	if (rs >= 0) {
	    int	f ;

	    setutent() ;
	    while ((up = getutent()) != NULL) {
	        f = (up->ut_type == UTMP_TUSERPROC) ;
	        f = f && (up->ut_user[0] != '.') ;
		if (f) n += 1 ;
	    } /* end while */
	    endutent() ;

#if	defined(SYSHAS_UTMPNAME) && (SYSHAS_UTMPNAME > 0)
	    if (f_utf)
	        utmpname(UTMPFNAME) ;
#endif

	} /* end if (ok) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (nusers) */

#endif /* CF_UTMPX */

#endif /* CF_TMPX */


