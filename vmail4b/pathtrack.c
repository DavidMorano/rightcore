/* pathtrack */

/* track paths (for finding programs) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_DEBUGFORK	0		/* debug-fork */
#define	CF_LOGID	1		/* use a special LOGID */
#define	CF_SETRUID	1		/* use 'setreuid(2)' */
#define	CF_SETEUID	0		/* already done in 'main()' */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was borrowed and modified from previous generic
        front-end 'main' subroutines!

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Prepare to do some servicing.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	getserial(const char *) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG 
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	progexports(struct proginfo *,const char *) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static const char	*prlibs[] = {
	"lib",
	NULL
} ;


/* exported subroutines */



/* static subroutines */


static int loadpath(pip,plp,varname,prdirs,defpath)
struct proginfo	*pip ;
vecstr		*plp ;
const char	*varname ;
const char	**prdirs ;
const char	*defpath ;
{
	VECSTR	*elp = &pip->exports ;

	int	rs = SR_OK ;
	int	c = 0 ;

	const char	*pp ;

/* system-administrative environment */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progprocess/loadpath: exports> \n") ;
	    progexports(pip,"loadpath0") ;
	}
#endif

	if ((rs = vecstr_search(elp,varname,vstrkeycmp,&pp)) >= 0) {
	    const char	*tp ;

	    if ((tp = strchr(pp,'=')) != NULL) {
	        rs = loadpathcomp(pip,plp,(tp + 1)) ;
	        c += rs ;
	    }

/* our program root */

	    if (rs >= 0) {
	        rs = loadpathpr(pip,plp,prdirs) ;
	        c += rs ;
	    }

/* system-default path */

	    if ((rs >= 0) && (defpath != NULL)) {
	        rs = loadpathcomp(pip,plp,defpath) ;
	        c += rs ;
	    }

/* process environment */

	    if ((rs >= 0) && ((tp = getenv(varname)) != NULL)) {
	        rs = loadpathcomp(pip,plp,tp) ;
	        c += rs ;
	    }

	} /* end if (search-found) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("progprocess/loadpath: ret exports> \n") ;
	    progexports(pip,"loadpath1") ;
	}
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progprocess/loadpath: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


static int loadpathpr(pip,plp,prdirs)
struct proginfo	*pip ;
vecstr		*plp ;
const char	**prdirs ;
{
	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;


	for (i = 0 ; prdirs[i] != NULL ; i += 1) {
	    rs = loadpathprdir(pip,plp,prdirs[i]) ;
	    c += rs ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathpr) */


static int loadpathprdir(pip,plp,bname)
struct proginfo	*pip ;
vecstr		*plp ;
const char	bname[] ;
{
	int	rs = SR_OK ;
	int	pl ;
	int	c = 0 ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if ((rs = mkpath2(tmpfname,pip->pr,bname)) >=0) {
	    pl = rs ;
	    rs = vecstr_adduniq(plp,tmpfname,pl) ;
	    if (rs < INT_MAX) c += 1 ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathprdir) */


static int loadpathcomp(pip,plp,pp)
struct proginfo	*pip ;
vecstr		*plp ;
const char	*pp ;
{
	int	rs = SR_OK ;
	int	c = 0 ;

	const char	*tp ;


	while ((tp = strpbrk(pp,":;")) != NULL) {
	    rs = loadpather(pip,plp,pp,(tp - pp)) ;
	    pp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {
	    rs = loadpather(pip,plp,pp,-1) ;
	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpathcomp) */


static int loadpather(pip,plp,pbuf,plen)
struct proginfo	*pip ;
vecstr		*plp ;
const char	pbuf[] ;
int		plen ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	pl ;
	int	c = 0 ;

	char	tmpfname[MAXPATHLEN + 1] ;


	if ((pl = pathclean(tmpfname,pbuf,plen)) > 0) {

	    rs1 = vecstr_findn(plp,tmpfname,pl) ;
	    if (rs1 == SR_NOTFOUND) {
	        c += 1 ;
	        rs = vecstr_add(plp,tmpfname,pl) ;
	    }

	} /* end if (pathclean) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpather) */


