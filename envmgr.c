/* envmgr */

/* Environment Manager */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-18, David A­D­ Morano
	This object module was first written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Manage an environment ensemble.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"envmgr.h"


/* local defines */


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	vstrkeycmp(const char **,const char **) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern cchar	**environ ;


/* local structures */


/* forward references */


/* local variables */


/* exported variables */


/* exported subroutines */


int envmgr_start(ENVMGR *emp)
{
	const int	vo = (VECHAND_OCOMPACT | VECHAND_OSORTED) ;
	int		rs ;

	if ((rs = vechand_start(&emp->envlist,10,vo)) >= 0) {
	    if ((rs = vecstr_start(&emp->envstrs,2,0)) >= 0) {
		int	i ;
		for (i = 0 ; (rs >= 0) && (environ[i] != NULL) ; i += 1) {
	    	    rs = vechand_add(&emp->envlist,environ[i]) ;
		}
	    }
	    if (rs < 0)
	        vechand_finish(&emp->envlist) ;
	} /* end if (vechand_start) */

	return rs ;
}
/* end subroutine (envmgr_start) */


int envmgr_finish(ENVMGR *emp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = vecstr_finish(&emp->envstrs) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&emp->envlist) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (envmgr_finish) */


int envmgr_set(ENVMGR *emp,cchar *kp,cchar *vp,int vl)
{
	vecstr		*esp = &emp->envstrs ;
	int		rs ;
	if ((rs = vecstr_envset(esp,kp,vp,vl)) >= 0) {
	    vechand	*elp = &emp->envlist ;
	    const int	i = rs ;
	    cchar	*ep ;
	    if ((rs = vecstr_get(esp,i,&ep)) >= 0) {
		const int	nrs = SR_NOTFOUND ;
	        int		(*venvcmp)(const void **,const void **) ;
	        venvcmp = (int (*)(const void **,const void **)) vstrkeycmp ;
	        if ((rs = vechand_search(elp,kp,venvcmp,NULL)) >= 0) {
	            vechand_del(elp,rs) ;
	        } else if (rs == nrs) {
		    rs = SR_OK ;
		}
	        if (rs >= 0) {
		    rs = vechand_add(elp,ep) ;
	        }
	    } /* end if (vecstr_get) */
	} /* end if (vecstr_envset) */

	return rs ;
}
/* end subroutine (envmgr_set) */


int envmgr_getvec(ENVMGR *emp,cchar ***rppp)
{
	int		rs ;

	rs = vechand_getvec(&emp->envlist,rppp) ;

	return rs ;
}
/* end subroutine (envmgr_getvec) */


