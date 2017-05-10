/* prognamecache */

/* name-cache management */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
	This subroutine manages a (simple) name cahce.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We maintain a process cache for real-names.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<userinfo.h>
#include	<namecache.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	mkuiname(char *,int,USERINFO *) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int prognamecache_namecache(PROGINFO *) ;


/* local variables */


/* exported subroutines */


int prognamecache_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("prognamecache_begin: entered\n") ;
#endif

	if (pip == NULL) return SR_FAULT ;

	if (pip->uip == NULL) pip->uip = uip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("prognamecache_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (prognamecache_begin) */


int prognamecache_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	if (pip->namecache != NULL) {
	    NAMECACHE	*ncp = (NAMECACHE *) pip->namecache ;
	    rs1 = namecache_finish(ncp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(pip->namecache) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->namecache = NULL ;
	}

	return rs ;
}
/* end subroutine (prognamecache_end) */


int prognamecache_lookup(PROGINFO *pip,cchar *un,cchar **rpp)
{
	NAMECACHE	*ncp ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("prognamecache_lookup: entered un=%s\n",un) ;
#endif

	if (pip == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;
	if (un[0] == '\0') return SR_INVALID ;

	if (pip->namecache == NULL) {
	    rs = prognamecache_namecache(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("prognamecache_lookup: mid rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    ncp = (NAMECACHE *) pip->namecache ;
	    rs = namecache_lookup(ncp,un,rpp) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("prognamecache_lookup: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (prognamecache_lookup) */


/* local subroutines */


static int prognamecache_namecache(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->namecache == NULL) {
	    const int	msize = sizeof(NAMECACHE) ;
	    void	*p ;
	    if ((rs = uc_malloc(msize,&p)) >= 0) {
	        NAMECACHE	*ncp = (NAMECACHE *) p ;
	        const int	to = (5*60) ;
	        pip->namecache = p ;
	        if ((rs = namecache_start(ncp,VARUSERNAME,0,to)) >= 0) {
	            const char	*up = getourenv(pip->envv,VARUSERNAME) ;
	            if (up != NULL) {
		        const char	*cp = getourenv(pip->envv,VARFULLNAME) ;
		        if ((cp == NULL) || (cp[0] == '\0'))
		            cp = getourenv(pip->envv,VARNAME) ;
		        if ((cp != NULL) && (cp[0] != '\0')) {
	        	    rs = namecache_add(ncp,up,cp,-1) ;
		        }
		    }
		    if (rs < 0)
		        namecache_finish(ncp) ;
	        } /* end if (namecache) */
		if (rs < 0) {
		    uc_free(p) ;
		    pip->namecache = NULL ;
		}
	    } /* end if (memory-allocation) */
	} /* end if (needed-construction) */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("prognamecache_namecache: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (prognamecache_namecache) */


