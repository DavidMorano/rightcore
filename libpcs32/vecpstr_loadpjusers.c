/* vecpstr_loadpjusers */

/* find and load UNIX® users who have the given project name */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-01-10, David A­D­ Morano
	This code was originally written.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds all users who have the given specified project.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecpstr.h>
#include	<getbufsize.h>
#include	<sysuserattr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnwcpy(char *,int,const char *,int) ;


/* local structures */


/* forward references */

static int	vecpstr_loadpjnent(vecpstr *,userattr_t *,cchar *) ;


/* local variables */


/* exported subroutines */


int vecpstr_loadpjusers(VECPSTR *ulp,cchar *pjn)
{
	const int	ualen = getbufsize(getbufsize_ua) ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		*uabuf ;

	if (ulp == NULL) return SR_FAULT ;
	if (pjn == NULL) return SR_FAULT ;
	if (pjn[0] == '\0') return SR_INVALID ;

	if ((rs = uc_malloc((ualen+1),&uabuf)) >= 0) {
	    SYSUSERATTR		su, *sup = &su ;
	    if ((rs = sysuserattr_open(sup,NULL)) >= 0) {
	        userattr_t	ua ;
	        while ((rs = sysuserattr_readent(sup,&ua,uabuf,ualen)) > 0) {
	            rs = vecpstr_loadpjnent(ulp,&ua,pjn) ;
	            c += rs ;
	            if (rs < 0) break ;
	        } /* end while */
	        rs1 = sysuserattr_close(sup) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sysuserattr) */
	    uc_free(uabuf) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_loadpjusers) */


/* local subroutines */


static int vecpstr_loadpjnent(vecpstr *ulp,userattr_t *uap,cchar *pjn)
{
	int		rs ;
	int		c = 0 ;
	cchar		*k = "project" ;
	cchar		*vp ;

#if	CF_DEBUGS
	    debugprintf("vecpstr_loadpjnent: u=%s\n",uap->name) ;
#endif

	if ((rs = uc_kvamatch(uap->attr,k,&vp)) >= 0) {
	    if (strwcmp(pjn,vp,rs) == 0) {
	        rs = vecpstr_adduniq(ulp,uap->name,-1) ;
	        if (rs < INT_MAX) c += 1 ;
	    }
	} else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	    debugprintf("vecpstr_loadpjnent: uc_kvamatch() rs=%d\n",rs) ;
#endif
	    rs = SR_OK ;
	}

#if	CF_DEBUGS
	    debugprintf("vecpstr_loadpjnent: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecpstr_loadpjnent) */


