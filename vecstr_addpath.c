/* vecstr_addpath */

/* add a "path" compnent to the string-list */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	------------------------------------------------------------------------
	Name:

	vecstr_addpathclean

	Description:

	This subroutine adds a "path" componment to the vector-string
	list.  It cleans up the path-component first by passing it through
	'pathclean(3dam)'.

	Synopsis:

	int vecstr_addpathclean(lp,pp,pl)
	vecstr		*lp ;
	const char	*pp ;
	int		pl ;

	Arguments:

	lp		vector-string list object pointer
	pp		path-string pointer
	pl		length of given string

	Returns:

	>=0		OK
	<0		some error


	------------------------------------------------------------------------
	Name:

	vecstr_addcspath

	Description:

	This subroutine will add (uniquely) the Condig-String (CS) 
	PATH value to the vector-list.

	Synopsis:

	int vecstr_addcspath(vsp)
	vecstr		*vsp ;

	Arguments:

	vsp		pointer to VECSTR object

	Returns:

	>=0		number of elements loaded
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
#define	DEFPATH		"/usr/preroot/bin:/usr/xpg4/bin:" \
				"/usr/ccs/bin:/usr/bin:/usr/sbin:/sbin"
#elif	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#define	DEFPATH		"/usr/preroot/bin:/usr/bin:/bin:/usr/sbin:/sbin"
#else
#define	DEFPATH		"/usr/preroot/bin:/usr/bin:/usr/sbin:/sbin"
#endif


/* external subroutines */

extern int	sncpy1w(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mknpath1(char *,int,const char *) ;
extern int	mknpath2(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const int	rsdefs[] = {
	SR_NOTFOUND,
	SR_OVERFLOW,
	0
} ;


/* exported subroutines */


int vecstr_addpathclean(vecstr *vlp,cchar *lp,int ll)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (vlp == NULL) return SR_FAULT ;
	if (lp == NULL) return SR_FAULT ;

	if (ll < 0) ll = strlen(lp) ;

	if (ll > 0) {
	    cchar	*tp ;
	    char	dname[MAXPATHLEN + 1] ;
	    while ((tp = strnpbrk(lp,ll,":;")) != NULL) {
		if ((tp-lp) >= 0) {
	    	    if ((rs = pathclean(dname,lp,(tp-lp))) >= 0) {
		        rs = vecstr_adduniq(vlp,dname,rs) ;
		        if (rs < INT_MAX) c += 1 ;
		    }
		}
		ll -= ((tp+1)-lp) ;
		lp = (tp+1) ;
		if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (ll > 0)) {
	    	if ((rs = pathclean(dname,lp,ll)) >= 0) {
		    rs = vecstr_adduniq(vlp,dname,rs) ;
		    if (rs < INT_MAX) c += 1 ;
		}
	    }
	} /* end if (non-zero) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_addpathclean) */


int vecstr_addpath(vecstr *vlp,cchar *lp,int ll)
{
	int		rs = SR_OK ;
	int		c = 0 ;

	if (vlp == NULL) return SR_FAULT ;
	if (lp == NULL) return SR_FAULT ;

	if (ll < 0) ll = strlen(lp) ;

	if (ll > 0) {
	    cchar	*tp ;
	    while ((tp = strnpbrk(lp,ll,":;")) != NULL) {
		if ((tp-lp) >= 0) {
		    rs = vecstr_adduniq(vlp,lp,(tp-lp)) ;
		    if (rs < INT_MAX) c += 1 ;
		}
		ll -= ((tp+1)-lp) ;
		lp = (tp+1) ;
		if (rs < 0) break ;
	    } /* end while */
	    if ((rs >= 0) && (ll > 0)) {
		rs = vecstr_adduniq(vlp,lp,ll) ;
		if (rs < INT_MAX) c += 1 ;
	    }
	} /* end if (non-zero) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_addpath) */


int vecstr_addcspath(vecstr *vsp)
{
	const int	plen = (2*MAXPATHLEN) ;
	int		rs ;
	int		c = 0 ;
	char		*pbuf ;

	if (vsp == NULL) return SR_FAULT ;

	if ((rs = uc_malloc((plen+1),&pbuf)) >= 0) {
	    int		dl = -1 ;
	    cchar	*dp = DEFPATH ;
	    if ((rs = uc_confstr(_CS_PATH,pbuf,plen)) >= 0) {
		dl = rs ;
	        dp = pbuf ;
	    } else if (isOneOf(rsdefs,rs)) {
	        rs = SR_OK ;
	    }
	    if (rs >= 0) {
	        rs = vecstr_addpath(vsp,dp,dl) ;
	        c += rs ;
	    } /* end if */
	    uc_free(pbuf) ;
	} /* end if (m-a-f) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_addcspath) */


