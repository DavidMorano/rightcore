/* proginfo_setpiv */

/* utility for KSH built-in commands */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is used for acquiring the program-root for a program.

	Synopsis:

	int proginfo_setpiv(pip,pr,vars)
	PROGINFO	*pip ;
	cchar		pr[] ;
	struct pivars	*vars ;

	Arguments:

	pip		program-information pointer
	pr		program root
	vars		array of program parameters

	Returns:

	<0	error
	>=0	length of PR


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getrootdname(char *,int,cchar *,cchar *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isNotPresent(int) ;

extern int	proginfo_getenv(PROGINFO *,cchar *,int,cchar **) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthex(cchar *,int,cchar *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	proginfo_setpiver(PROGINFO *,IDS *,cchar *,struct pivars *) ;

static int	sfrootdirname(cchar *,int,cchar **) ;

static int	dircheck(IDS *,cchar *) ;
static int	isNotGoodDir(int) ;


/* local variables */


/* exported subroutines */


int proginfo_setpiv(PROGINFO *pip,cchar *pr,struct pivars *vars)
{
	IDS		id ;
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("proginfo_setpiv: ent\n") ;
#endif

	if ((rs = ids_load(&id)) >= 0) {
	    rs = proginfo_setpiver(pip,&id,pr,vars) ;
	    ids_release(&id) ;
	} /* end if (ids) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("proginfo_setpiv: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proginfo_setpiv) */


/* local subroutines */


int proginfo_setpiver(PROGINFO *pip,IDS *idp,cchar *pr,struct pivars *vars)
{
	const int	plen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pl = -1 ;
	int		prlen = 0 ;
	cchar		*cp = NULL ;
	char		rdn[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("proginfo_setpiv: pr=%s\n",pr) ;
	    if (vars != NULL) {
	        debugprintf("proginfo_setpiv: vpr1=%s\n",vars->vpr1) ;
	        debugprintf("proginfo_setpiv: vpr2=%s\n",vars->vpr2) ;
	        debugprintf("proginfo_setpiv: vpr3=%s\n",vars->vpr3) ;
	        debugprintf("proginfo_setpiv: pr=%s\n",vars->pr) ;
	        debugprintf("proginfo_setpiv: vprname=%s\n",vars->vprname) ;
	    }
	}
#endif /* CF_DEBUG */

	if (pr == NULL) {
	    int	i ;

	    pl = -1 ;
	    rs1 = SR_NOTFOUND ;
	    for (i = 1 ; (rs >= 0) && isNotGoodDir(rs1) && (i <= 3) ; i += 1) {
	        cchar	*var  = NULL ;
	        switch (i) {
	        case 1:
	            var = vars->vpr1 ;
	            break ;
	        case 2:
	            var = vars->vpr2 ;
	            break ;
	        case 3:
	            var = vars->vpr3 ;
	            break ;
	        } /* end switch */
	        if ((var != NULL) && (var[0] != '\0')) {
	            if ((rs1 = proginfo_getenv(pip,var,-1,&cp)) >= 0) {
	                rs1 = dircheck(idp,cp) ;
	                if (! isNotGoodDir(rs1)) rs = rs1 ;
	            }
	        }
	    } /* end for */

	    if (rs1 >= 0) pr = cp ;
	} /* end if (straight out variables) */

	if ((rs >= 0) && (pr == NULL)) {
	    char	nn[MAXNAMELEN + 1] ;
	    char	dn[MAXHOSTNAMELEN + 1] ;

	    if ((rs1 = getnodedomain(nn,dn)) >= 0) {
	        cchar	**vpp = &pip->nodename ;
	        if ((rs = proginfo_setentry(pip,vpp,nn,-1)) >= 0) {
	            cchar	**vpp = &pip->domainname ;
	            if ((rs = proginfo_setentry(pip,vpp,dn,-1)) >= 0) {
	                rs1 = getrootdname(rdn,plen,vars->vprname,dn) ;
	                pl = rs1 ;
	            }
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("proginfo_setpiv: "
	                "getrootdname() rs=%d rootdname=%s\n",
	                rs1,rdn) ;
#endif

	        if ((rs >= 0) && (rs1 > 0))
	            pr = rdn ;

	    } /* end if (getnodedomain) */

	} /* end if (guess program root from domain name) */

/* try to see if a path was given at invocation */

	if ((rs >= 0) && (pr == NULL)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("proginfo_setpiv: progename=%s\n",pip->progename) ;
	        debugprintf("proginfo_setpiv: progdname=%s\n",pip->progdname) ;
	    }
#endif

	    if (pip->progdname == NULL)
	        rs = proginfo_progdname(pip) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("proginfo_setpiv: _progdname() "
	            "rs=%d progdname=%s\n",
	            rs,pip->progdname) ;
#endif

	    if ((rs >= 0) && (pip->progdname != NULL)) {
	        cchar	*cp ;
	        pl = sfrootdirname(pip->progdname,-1,&cp) ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            debugprintf("proginfo_setpiv: sfrootdirname() pl=%d\n",pl) ;
	            debugprintf("proginfo_setpiv: p=>%t<\n",cp,pl) ;
	        }
#endif

	        if (pl > 0) pr = cp ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("proginfo_setpiv: sfrootdirname() "
	            "rs=%d pr=%s\n",rs,pr) ;
#endif

	} /* end if (set program-root from program d-name) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proginfo_setpiv: mid pr=%s\n",pr) ;
#endif

/* default is a fixed string (from the initialization variables) */

	if ((rs >= 0) && (pr == NULL)) {
	    pr = vars->pr ;
	    pl = -1 ;
	}

/* enter it in if we have found it */

	if ((rs >= 0) && (pr != NULL)) {
	    rs = proginfo_setprogroot(pip,pr,pl) ;
	    prlen = rs ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("proginfo_setpiv: ret rs=%d prlen=%u pr=%s\n",
	        rs,prlen,pip->pr) ;
#endif

	return (rs >= 0) ? prlen : rs ;
}
/* end subroutine (proginfo_setpiver) */


static int sfrootdirname(cchar *dp,int dl,cchar **rpp)
{
	int		bl ;
	int		sl = -1 ;
	int		f ;
	cchar		*sp = NULL ;
	cchar		*bp ;

	if (rpp != NULL) *rpp = NULL ;

#if	CF_DEBUGS
	debugprintf("sfrootdirname: d=%t\n",dp,dl) ;
#endif

	bl = sfbasename(dp,dl,&bp) ;

#if	CF_DEBUGS
	debugprintf("sfrootdirname: b=%t\n",bp,bl) ;
#endif

	f = ((bl == 3) && (strncmp(bp,"bin",bl) == 0)) ;

	if (! f) {
	    f = ((bl == 4) && (strncmp(bp,"sbin",bl) == 0)) ;
	}

#if	CF_DEBUGS
	debugprintf("sfrootdirname: f=%u\n",f) ;
#endif

	if (f) {
	    sl = sfdirname(dp,dl,&sp) ;

#if	CF_DEBUGS
	    debugprintf("sfrootdirname: pr=%t\n",sp,sl) ;
#endif
	    if ((sl >= 0) && (rpp != NULL)) *rpp = sp ;
	}

#if	CF_DEBUGS
	debugprintf("sfrootdirname: ret sl=%d\n",sl) ;
#endif
	return sl ;
}
/* end subroutine (sfrootdirname) */


static int dircheck(IDS *idp,cchar *dname)
{
	struct ustat	sb ;
	int		rs ;
	if ((rs = u_stat(dname,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        const int	am = (R_OK|X_OK) ;
	        rs = sperm(idp,&sb,am) ;
	    } else {
	        rs = SR_NOTDIR ;
	    }
	}
	return rs ;
}
/* end subroutine (dircheck) */


static int isNotGoodDir(int rs)
{
	int		f = FALSE ;
	f = f || isNotPresent(rs) ;
	f = f || (rs == SR_NOTDIR) ;
	return f ;
}
/* end if (isNotGoodDir) */


