/* envhelp */

/* help w/ environment */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_SETEXECS	0		/* compile |setexecs| */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little honey helps daddy management the environment list for
        launching new programs.

	Synopsis:

	int envhelp_start(op,envbads,envv)
	ENVHELP		*op ;
	const char	**envbads ;
	const char	**envv ;

	Arguments:

	op		object pointer
	envbads		list of environment variables that are not included
	envv		environment array

	Returns:

	>=0		OK
	<0		error

	Synopsis:

	int envhelp_finish(op)
	ENVHELP		*op ;

	Arguments:

	op		object pointer

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"envhelp.h"


/* local defines */

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXHOSTNAMELEN + 40)
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(MAXHOSTNAMELEN + 40)
#endif

#define	NENVS		150
#define	DEFENVSTORESIZE	120


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy3w(char *,int,const char *,const char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	ctdecl(char *,int,long) ;
extern int	ctdecul(char *,int,ulong) ;
extern int	vstrkeycmp(const void **,const void **) ;
extern int	strkeycmp(const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy3w(char *,int,cchar *,cchar *,cchar *,int) ;


/* external variables */

extern cchar	**environ ; /* what is should be  */


/* forward reference */

static int	envhelp_copy(ENVHELP *,const char **,const char **) ;

#ifdef	COMMENT
static int	envhelp_envadd(ENVHELP *,const char *,const char *,int) ;
#endif /* COMMENT */


/* local variables */


/* exported subroutines */


int envhelp_start(ENVHELP *op,cchar **envbads,cchar **envv)
{
	const int	vo = (VECHAND_OCOMPACT | VECHAND_OSORTED) ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("envhelp_start: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (envv == NULL) envv = environ ;
	if ((rs = vechand_start(&op->env,NENVS,vo)) >= 0) {
	    const int	size = DEFENVSTORESIZE ;
	    if ((rs = strpack_start(&op->stores,size)) >= 0) {
		if (envv != NULL) {
	    	    rs = envhelp_copy(op,envbads,envv) ;
		}
	        if (rs < 0)
		    strpack_finish(&op->stores) ;
	    } /* end if (strpack_start) */
	    if (rs < 0)
		vechand_finish(&op->env) ;
	} /* end if (vechand_start) */

#if	CF_DEBUGS
	debugprintf("envhelp_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (envhelp_start) */


int envhelp_finish(ENVHELP *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = strpack_finish(&op->stores) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->env) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("envhelp_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (envhelp_finish) */


int envhelp_envset(ENVHELP *op,cchar *kp,cchar *vp,int vl)
{
	vechand		*elp = &op->env ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		size = 0 ;
	int		i = INT_MAX ;
	char		*p ;		/* 'char' for pointer arithmentic */

	if (kp == NULL) return SR_FAULT ;

	size += strlen(kp) ;
	size += 1 ;			/* for the equals sign character */
	if (vp != NULL) {
	    if (vl < 0) vl = strlen(vp) ;
	    size += strnlen(vp,vl) ;
	}
	size += 1 ;			/* terminating NUL */

	if ((rs = uc_malloc(size,&p)) >= 0) {
	    const char	*ep ;
	    char	*bp = p ;
	    bp = strwcpy(bp,kp,-1) ;
	    *bp++ = '=' ;
	    if (vp != NULL) bp = strwcpy(bp,vp,vl) ;
	    if ((rs = strpack_store(&op->stores,p,(bp-p),&ep)) >= 0) {
	        rs1 = vechand_search(elp,ep,vstrkeycmp,NULL) ;
	        if (rs1 >= 0) vechand_del(elp,rs1) ;
	        rs = vechand_add(elp,ep) ;
		i = rs ;
	    }
	    uc_free(p) ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (envhelp_envset) */


#if	CF_SETEXECS
int envhelp_setexecs(ENVHELP *op,cchar *pfn,cchar *argz)
{
	int		rs ;


	return rs ;
}
/* end subroutine (envhelp_setexecs) */
#endif /* CF_SETEXECS */


/* search for an entry in the list */
int envhelp_present(ENVHELP *op,cchar *kp,int kl,cchar **rpp)
{
	NULSTR		ks ;
	int		rs ;
	int		rs1 ;
	int		i = 0 ;
	const char	*cp ;

	if ((rs = nulstr_start(&ks,kp,kl,&cp)) >= 0) {
	    vechand	*elp = &op->env ;

	    rs = vechand_search(elp,cp,vstrkeycmp,rpp) ;
	    i = rs ;

	    rs1 = nulstr_finish(&ks) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (envhelp_present) */


int envhelp_sort(ENVHELP *op)
{
	return vechand_sort(&op->env,vstrkeycmp) ;
}
/* end subroutine (envhelp_sort) */


int envhelp_getvec(ENVHELP *op,cchar ***eppp)
{
	vechand		*elp = &op->env ;
	int		rs ;

	if ((rs = vechand_sort(elp,vstrkeycmp)) >= 0) {
	    if (eppp != NULL) {
	        rs = vechand_getvec(elp,eppp) ;
	    } else {
	        rs = vechand_count(elp) ;
	    }
	} /* end if (vechand_sort) */

	return rs ;
}
/* end subroutine (envhelp_getvec) */


/* private subroutines */


static int envhelp_copy(ENVHELP *op,cchar **envbads,cchar **envv)
{
	vechand		*elp = &op->env ;
	int		rs = SR_OK ;
	int		n = 0 ;

	if (envv != NULL) {
	    int		i ;
	    const char	*ep ;
	    if (envbads != NULL) {
	        for (i = 0 ; (rs >= 0) && (envv[i] != NULL) ; i += 1) {
	            ep = envv[i] ;
	            if (matkeystr(envbads,ep,-1) < 0) {
#if	CF_DEBUGS
			debugprintf("envhelp_copy: copy e> %s\n",ep) ;
#endif
	                n += 1 ;
	                rs = vechand_add(elp,ep) ;
	            } /* end if (was not bad) */
	        } /* end for (copy ENVs) */
	    } else {
	        for (i = 0 ; (rs >= 0) && (envv[i] != NULL) ; i += 1) {
	            ep = envv[i] ;
	            n += 1 ;
	            rs = vechand_add(elp,ep) ;
	        } /* end for (copy ENVs) */
	    } /* end if (envbads) */
	} /* end if (envv) */

#if	CF_DEBUGS
	debugprintf("envhelp_copy: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (envhelp_copy) */


