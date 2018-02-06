/* getrunlevel */

/* return the run-level of the system */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_TMPX		1		/* try TMPX (reentrant) */
#define	CF_UTMPX	0		/* try UTMPX (standard UNIX®) */
#define	CF_UTMPACC	1		/* try |utmpacc_runlevel(3uc)| */


/* revision history:

	= 1998-11-18, David A­D­ Morano
        This subroutine was written to simplify getting the current "run-level"
        of the system.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We use the UTMPX database to query for the "run-level" of the system.
	If the UTMPX file is not found, we return SR_NOENT.  If we cannot
	access it we return SR_ACCESS.  If the file is found but there is no
	"run-level" record in it, we return zero (0).

	Synopsis:

	int getrunlevel(utmpxfname)
	const char	*utmpxfname ;

	Arguments:

	utmpxfname	UTMPX filename

	Returns:

	<0	one of: SR_NOENT, SR_ACCESS, other means some bad happened
	>=0	run-level (including '0' meaning no record found)

	Notes: Why some subroutine like this was not provided by the UNIX®
	developers themselves is a mystery.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)
#include	<utmpx.h>
#endif

#include	<vsystem.h>

#if	CF_TMPX || CF_UTMPACC
#include	<tmpx.h>
#endif

#if	CF_UTMPACC
#include	<utmpacc.h>
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

#if	CF_UTMPACC || CF_TMPX
extern int	tmpx_getrunlevel(TMPX *) ;
#endif


/* external variables */


/* local structures */


/* forward references */

#if	CF_UTMPACC || CF_TMPX
static int	getrunlevel_tmpx(cchar *) ;
#endif


/* local variables */


/* exported subroutines */


#if	CF_UTMPACC && CF_TMPX

int getrunlevel(cchar *utmpfname)
{
	int		rs ;
#if	CF_DEBUGS
	debugprintf("getrunlevel: ent utf=%s\n",utmpfname) ;
#endif
	if (utmpfname != NULL) {
	    rs = getrunlevel_tmpx(utmpfname) ;
	} else {
	    rs = utmpacc_runlevel() ;
#if	CF_DEBUGS
	    debugprintf("getrunlevel: utmpacc_runlevel() rs=%d\n",rs) ;
#endif
	}
#if	CF_DEBUGS
	debugprintf("getrunlevel: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (getrunlevel) */

#else /* CF_UTMPACC */

#if	CF_TMPX

int getrunlevel(const char *utmpfname)
{
	return getrunlevel_tmpx(utmpfname) ;
}
/* end subroutine (getrunlevel) */

#else /* CF_TMPX */

#if	CF_UTMPX && defined(SYSHAS_UTMPX) && (SYSHAS_UTMPX > 0)

int getrunlevel(cchar *utmpfname)
{
	struct utmpx	*up ;
	int		rs = SR_OK ;
	int		n = 0 ;
	int		f_utf = FALSE ;

	if ((utmpfname != NULL) && (utmpfname[0] != '\0')) {
#if	defined(SYSHAS_UTMPXNAME) && (SYSHAS_UTMPXNAME > 0)
	    {
	        int rc = utmpxname(utmpfname) ;
	        f_utf = TRUE ;
	        rs = (rc == 1) ? SR_OK : SR_INVALID ;
	    }
#else
	    rs = SR_NOSYS ;
#endif /* SYSHAS_UTMPXNAME */
	} /* end if */

	if (rs >= 0) {
	    setutxent() ;
	    while ((up = getutxent()) != NULL) {
	        if (up->ut_type == UTMPX_TRUNLEVEL) {
	            n = (up->ut_exit.e_termination & UCHAR_MAX) ;
		    break ;
	  	}
	    } /* end while */
	    endutxent() ;
#if	defined(SYSHAS_UTMPXNAME) && (SYSHAS_UTMPXNAME > 0)
	    if (f_utf) utmpxname(UTMPXFNAME) ;
#endif
	} /* end if (ok) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getrunlevel) */

#else /* CF_UTMPX */

int getrunlevel(cchar *utmpfname)
{
	struct utmp	*up ;
	int		rs = SR_OK ;
	int		n = 0 ;
	int		f_utf = FALSE ;

	if ((utmpfname != NULL) && (utmpfname[0] != '\0')) {
#if	defined(SYSHAS_UTMPNAME) && (SYSHAS_UTMPNAME > 0)
	    {
	        int	rc = utmpname(utmpfname) ;
	        f_utf = TRUE ;
	        rs = (rc == 1) ? SR_OK : SR_INVALID ;
	    }
#else
	    rs = SR_NOSYS ;
#endif
	} /* end if */

	n = 0 ;
	if (rs >= 0) {
	    setutent() ;
	    while ((up = getutent()) != NULL) {
	        if (up->ut_type == UTMP_TRUNLEVEL) {
	            n = (up->ut_exit.e_termination & UCHAR_MAX) ;
		    break ;
	  	}
	    } /* end while */
	    endutent() ;
#if	defined(SYSHAS_UTMPNAME) && (SYSHAS_UTMPNAME > 0)
	    if (f_utf) utmpname(UTMPFNAME) ;
#endif
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getrunlevel) */

#endif /* CF_UTMPX */

#endif /* CF_TMPX */

#endif /* CF_UTMPACC */


/* local subroutines */


#if	CF_UTMPACC || CF_TMPX
int getrunlevel_tmpx(const char *utmpfname)
{
	TMPX		ut ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	if ((rs = tmpx_open(&ut,utmpfname,O_RDONLY)) >= 0) {
	    rs = tmpx_getrunlevel(&ut) ;
	    n = rs ;
	    rs1 = tmpx_close(&ut) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (tmpx) */
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getrunlevel) */
#endif /* CF_TMPX */


