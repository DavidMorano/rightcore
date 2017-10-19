/* proginfo */

/* program information */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGN	0		/* non-switchable debug print-outs */
#define	CF_GETEXECNAME	1		/* use 'getexecname()' */


/* revision history:

	= 1998-03-17, David A­D­ Morano
	I enhanced this somewhat from my previous version.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This group of subroutines help find and set from variables for program
	start-up type functions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<shellunder.h>
#include	<localmisc.h>

#include	"defs.h"


/* local defines */

#ifndef	NODENAMELEN
#define	NODENAMELEN	256
#endif

#define	NOPROGNAME	"NP"

#ifndef	VAREXECFNAME
#define	VAREXECFNAME	"_EF"
#endif

#ifndef	VARUNDER
#define	VARUNDER	"_"
#endif

#define	NDF	"proginfo.deb"


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	rmext(cchar *,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	getnodename(char *,int) ;
extern int	getpwd(char *,int) ;
extern int	getev(cchar **,cchar *,int,cchar **) ;
extern int	hasprintbad(cchar *,int) ;
extern int	hasuc(cchar *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;
extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strwcpylc(char *,cchar *,int) ;
extern char	*strwcpyuc(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnrchr(cchar *,int,int) ;


/* forward references */

int		proginfo_setentry(PROGINFO *,cchar **,cchar *,int) ;
int		proginfo_setprogname(PROGINFO *,cchar *) ;
int		proginfo_setexecname(PROGINFO *,cchar *) ;
int		proginfo_getpwd(PROGINFO *,char *,int) ;
int		proginfo_pwd(PROGINFO *) ;
int		proginfo_progdname(PROGINFO *) ;
int		proginfo_progename(PROGINFO *) ;

static int	proginfo_setdefnames(PROGINFO *) ;

#ifdef	COMMENT
static int	proginfo_setdefdn(PROGINFO *) ;
static int	proginfo_setdefpn(PROGINFO *) ;
#endif

#if	CF_DEBUGN
static int	proginfo_storelists(PROGINFO *,cchar *) ;
#endif

static int	hasourbad(cchar *,int) ;


/* local variables */


/* exported subroutines */


int proginfo_start(PROGINFO *pip,cchar *envv[],cchar argz[],cchar ver[])
{
	int		rs ;
	int		opts ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("proginfo_start: argz=%s\n",argz) ;
#endif

	memset(pip,0,sizeof(PROGINFO)) ;

	pip->envv = (cchar **) envv ;

	opts = (VECSTR_OCONSERVE | VECSTR_OREUSE | VECSTR_OSWAP) ;
	if ((rs = vecstr_start(&pip->stores,10,opts)) >= 0) {
	    if ((rs = proginfo_pwd(pip)) >= 0) {
	        if (argz != NULL) {
	            rs = proginfo_setprogname(pip,argz) ;
	        }
	        if (rs >= 0) {
	            if ((rs = proginfo_setdefnames(pip)) >= 0) {
	                if (ver != NULL) {
	                    cchar	**vpp = &pip->version ;
	                    rs = proginfo_setentry(pip,vpp,ver,-1) ;
	                }
	            }
	        } /* end if (ok) */
	    }
	    if (rs < 0)
	        vecstr_finish(&pip->stores) ;
	} /* end if (vecstr-stores) */

#if	CF_DEBUGS
	debugprintf("proginfo_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proginfo_start) */


int proginfo_finish(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUGN
	proginfo_storelists(pip,"freeing") ;
#endif /* CF_DEBUGN */

	rs1 = vecstr_finish(&pip->stores) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGN
	nprintf(NDF,"proginfo_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proginfo_finish) */


/* set an entry */
int proginfo_setentry(PROGINFO *pip,cchar **epp,cchar *vp,int vl)
{
	vecstr		*vsp = &pip->stores ;
	int		rs = SR_OK ;
	int		oi = -1 ;
	int		len = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (*epp != NULL) {
	    oi = vecstr_findaddr(vsp,*epp) ;
	}
	if (vp != NULL) {
	    len = strnlen(vp,vl) ;
	    rs = vecstr_store(vsp,vp,len,epp) ;
	} else {
	    *epp = NULL ;
	}
	if ((rs >= 0) && (oi >= 0)) {
	    vecstr_del(vsp,oi) ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (proginfo_setentry) */


#if	CF_DEBUGN
int proginfo_storelists(PROGINFO *pip,cchar s[])
{
	VECSTR		*vsp = &pip->stores ;
	int		rs = SR_OK ;
	int		i ;
	cchar		*cp ;
	if (s != NULL) {
	    nprintf(NDF,"proginfo_storelists: s=>%s<\n",s) ;
	}
	nprintf(NDF,"proginfo_storelists: vi=%u\n",vsp->i) ;
	for (i = 0 ; (rs = vecstr_get(vsp,i,&cp)) >= 0 ; i += 1) {
	    if (cp != NULL) {
	        nprintf(NDF,"proginfo_storelists: s[%u]{%p}\n",i,cp) ;
	        nprintf(NDF,"proginfo_storelists: s[%u]=>%s<\n",i,cp) ;
	    }
	} /* end for */
	if (rs == SR_NOTFOUND) rs = SR_OK ;
	nprintf(NDF,"proginfo_storelists: ret rs=%d i=%u\n",rs,i) ;
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (proginfo_storelists) */
#endif /* CF_DEBUGN */


int proginfo_setversion(PROGINFO *pip,cchar *v)
{
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
	if (v == NULL) return SR_FAULT ;

	{
	    cchar	**vpp = &pip->version ;
	    rs = proginfo_setentry(pip,vpp,v,-1) ;
	}

	return rs ;
}
/* end subroutine (proginfo_setversion) */


int proginfo_setbanner(PROGINFO *pip,cchar *v)
{
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
	if (v == NULL) return SR_FAULT ;

	{
	    cchar	**vpp = &pip->banner ;
	    rs = proginfo_setentry(pip,vpp,v,-1) ;
	}

	return rs ;
}
/* end subroutine (proginfo_setbanner) */


int proginfo_setsearchname(PROGINFO *pip,cchar *var,cchar *value)
{
	int		rs = SR_OK ;
	int		cl = -1 ;
	cchar		*cp = value ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("proginfo_setsearchname: ent var=%s val=%s\n",var,value) ;
#endif

#if	CF_DEBUGN
	nprintf(NDF,"proginfo_setsearchname: value=%s\n",value) ;
#endif

	if ((cp == NULL) && (var != NULL)) {
	    cp = getourenv(pip->envv,var) ;
	    if (hasourbad(cp,-1) || ((cp != NULL) && (cp[0] == '\0')))
	        cp = NULL ;
	}

	if ((cp == NULL) && (pip->progname != NULL)) {
	    cchar	*tp ;
	    cp = pip->progname ;
	    if ((tp = strrchr(cp,'.')) != NULL) {
	        cl = (tp - cp) ;
	    }
	}

	if ((rs >= 0) && (cp != NULL)) {
	    char	searchname[MAXNAMELEN+1] ;
	    int		ml = MAXNAMELEN ;
	    if (hasuc(cp,cl)) {
	        if ((cl > 0) && (cl < ml)) ml = cl ;
	        cl = strwcpylc(searchname,cp,ml) - searchname ;
	        cp = searchname ;
	    }
	    {
	        cchar	**vpp = &pip->searchname ;
	        rs = proginfo_setentry(pip,vpp,cp,cl) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (proginfo_setsearchname) */


/* set program directory and program name (as might be possible) */
int proginfo_setprogname(PROGINFO *pip,cchar *ap)
{
	int		rs = SR_OK ;
	int		al ;
	int		dl, bl ;
	cchar		*en ;
	cchar		*dn ;
	cchar		*dp, *bp ;

#if	CF_DEBUGS
	debugprintf("proginfo_setprogname: ent ap=%s\n",ap) ;
#endif

	if (ap == NULL) return SR_FAULT ;

	en = pip->progename ;
	dn = pip->progdname ;
	al = strlen(ap) ;

#if	CF_DEBUGN
	nprintf(NDF,"proginfo_setprogname: argz=%s\n",ap) ;
#endif

	while ((al > 0) && (ap[al-1] == '/')) {
	    al -= 1 ;
	}

	if ((rs >= 0) && (pip->progename == NULL)) {
	    if (ap[0] == '/') {
	        cchar	**vpp = &pip->progename ;
	        rs = proginfo_setentry(pip,vpp,ap,-1) ;
	    }
	}

	bl = sfbasename(ap,al,&bp) ;

/* set a program dirname? */

	if (rs >= 0) {
	    if (((en == NULL) || (dn == NULL)) && 
	        ((dl = sfdirname(ap,al,&dp)) > 0)) {
	        int		f_parent = FALSE ;
	        int		f_pwd = FALSE ;
	        int		f = FALSE ;

#if	CF_DEBUGN
	        nprintf(NDF,"proginfo_setprogname: dirname=%t\n",dp,dl) ;
#endif

	        if (dp[0] == '.') {
	            f_pwd = (strcmp(dp,".") == 0) ;
	            if (! f_pwd) {
	                f_parent = (strcmp(dp,"..") == 0) ;
	            }
	            f = f_pwd || f_parent ;
	        } /* end if */

	        if (f) {
	            if (pip->pwd == NULL) {
	                rs = proginfo_pwd(pip) ;
	            }
	            if (rs >= 0) {
	                if (f_pwd) {
	                    dp = pip->pwd ;
	                    dl = pip->pwdlen ;
	                } else {
	                    dl = sfdirname(pip->pwd,pip->pwdlen,&dp) ;
	                }
	            } /* end if (PWD or parent) */
	        } /* end if */

	        if ((rs >= 0) && (pip->progdname == NULL)) {
	            cchar	**vpp = &pip->progdname ;
	            rs = proginfo_setentry(pip,vpp,dp,dl) ;
	        }

	        if ((rs >= 0) && (pip->progename == NULL)) {
	            if ((bp != NULL) && (bl > 0)) {
	                char	ename[MAXPATHLEN+1] ;
	                if ((rs = mkpath2w(ename,pip->progdname,bp,bl)) >= 0) {
	                    cchar	**vpp = &pip->progename ;
	                    rs = proginfo_setentry(pip,vpp,ename,-1) ;
	                }
	            } /* end if */
	        } /* end if */

	    } /* end if (have a dirname) */
	} /* end if (ok) */

/* set a program basename? */

	if (rs >= 0) {

	    if ((bp != NULL) && (bl > 0)) {
	        if ((bl = rmext(bp,bl)) == 0) {
	            bp = NOPROGNAME ;
	            bl = -1 ;
	        }
	    }

	    if ((bp != NULL) && (bl > 0)) {
	        if (hasourbad(bp,bl)) {
	            bp = NULL ;
	            bl = 0 ;
	        }
	    }

	    if ((bp != NULL) && (bl > 0) && (bp[0] == '-')) {
	        pip->f.progdash = TRUE ;
	        bp += 1 ;
	        bl -= 1 ;
	    }

#if	CF_DEBUGN
	    nprintf(NDF,"proginfo_setprogname: progname=%t\n",bp,bl) ;
#endif

	    if ((bp != NULL) && (bl != 0)) {
	        cchar	**vpp = &pip->progname ;
	        rs = proginfo_setentry(pip,vpp,bp,bl) ;
	    }

	} /* end if (basename) */

#if	CF_DEBUGS
	debugprintf("proginfo_setprogname: ent rs=%d\n",rs) ;
	debugprintf("proginfo_setprogname: pn=%s\n",pip->progname) ;
#endif

	return rs ;
}
/* end subroutine (proginfo_setprogname) */


/* set program root */
int proginfo_setprogroot(PROGINFO *pip,cchar *prp,int prl)
{
	int		rs = SR_OK ;
	char		tbuf[MAXPATHLEN + 1] ;

	if (prp == NULL) return SR_FAULT ;

	if (prl < 0) prl = strlen(prp) ;

	if (prp[0] != '/') {
	    if ((rs = proginfo_pwd(pip)) >= 0) {
	        rs = mkpath2w(tbuf,pip->pwd,prp,prl) ;
	        prl = rs ;
	        prp = tbuf ;
	    }
	}

	if (rs >= 0) {
	    cchar	**vpp = &pip->pr ;
	    rs = proginfo_setentry(pip,vpp,prp,prl) ;
	}

	return rs ;
}
/* end subroutine (proginfo_setprogroot) */


/* set the program execution filename */
int proginfo_setexecname(PROGINFO *pip,cchar *enp)
{
	int		rs = SR_OK ;

	if (enp == NULL) return SR_FAULT ;

	if (pip->progename == NULL) {
	    int		enl = strlen(enp) ;
	    while ((enl > 0) && (enp[enl-1] == '/')) enl -= 1 ;
	    if (enl > 0) {
	        cchar	**vpp = &pip->progename ;
	        rs = proginfo_setentry(pip,vpp,enp,enl) ;
	    }
	} else {
	    if (pip->progename != NULL) {
	        rs = strlen(pip->progename) ;
	    }
	}

	return rs ;
}
/* end subroutine (proginfo_setexecname) */


/* ensure (set) that the current PWD is set */
int proginfo_pwd(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		pwdlen = 0 ;

	if (pip->pwd == NULL) {
	    char	pwdname[MAXPATHLEN + 1] ;
	    if ((rs = getpwd(pwdname,MAXPATHLEN)) >= 0) {
	        cchar	**vpp = &pip->pwd ;
	        pwdlen = rs ;
	        pip->pwdlen = pwdlen ;
	        rs = proginfo_setentry(pip,vpp,pwdname,pwdlen) ;
	    }
	} else {
	    pwdlen = pip->pwdlen ;
	}

	return (rs >= 0) ? pwdlen : rs ;
}
/* end subroutine (proginfo_pwd) */


int proginfo_progdname(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->progdname == NULL) {

	    rs = proginfo_progename(pip) ;

#if	CF_DEBUGS
	    debugprintf("proginfo_progdname: progename=%s\n",pip->progename) ;
#endif

	    if ((rs >= 0) && (pip->progename != NULL)) {
	        int	dl ;
	        cchar	*dp ;
	        if ((dl = sfdirname(pip->progename,-1,&dp)) == 0) {
	            if (pip->pwd == NULL) rs = proginfo_pwd(pip) ;
	            if ((rs >= 0) && (pip->pwd != NULL)) {
	                dp = pip->pwd ;
	                dl = -1 ;
	            }
	        }
	        if ((rs >= 0) && (dl > 0)) {
	            cchar	**vpp = &pip->progdname ;
	            rs = proginfo_setentry(pip,vpp,dp,dl) ;
	        }
	    } /* end if */

	} else {
	    rs = strlen(pip->progdname) ;
	}

	return rs ;
}
/* end subroutine (proginfo_progdname) */


/* Set Default (program) Exec-Name */
int proginfo_progename(PROGINFO *pip)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("proginfo_progename: ent\n") ;
	debugprintf("proginfo_progename: pe=%s\n",pip->progename) ;
#endif

	if (pip->progename == NULL) {
	    SHELLUNDER	su ;
	    cchar	*efn = NULL ;

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0) && CF_GETEXECNAME
	    if ((rs >= 0) && (efn == NULL)) {
	        efn = getexecname() ;
#if	CF_DEBUGN
	        nprintf(NDF,"proginfo_progename: getexecname() wn=%s\n",
	            efn) ;
#endif
	    }
#endif /* SOLARIS */

	    if ((rs >= 0) && (efn == NULL) && (efn[0] != '\0')) {
	        efn = getourenv(pip->envv,VAREXECFNAME) ;
	    }

	    if ((rs >= 0) && (efn == NULL) && (efn[0] != '\0')) {
		cchar	*cp ;
	        if ((cp = getourenv(pip->envv,VARUNDER)) != NULL) {
		    if (shellunder(&su,cp) >= 0) {
	                efn = su.progename ;
		    }
	        }
	    }

	    if ((rs >= 0) && (efn != NULL)) {
	        char	ebuf[MAXPATHLEN+1] ;
	        if (efn[0] != '/') {
	            if (pip->pwd == NULL) rs = proginfo_pwd(pip) ;
	            if (rs >= 0) {
	                rs = mkpath2(ebuf,pip->pwd,efn) ;
	                efn = ebuf ;
	            }
	        }
	        if (rs >= 0) {
	            rs = proginfo_setexecname(pip,efn) ;
	        }
	    }

#if	CF_DEBUGN
	    nprintf(NDF,"proginfo_progename: mid rs=%d en=%s\n",
	        rs,pip->progename) ;
#endif

	} else {
	    if (pip->progename != NULL) {
	        rs = strlen(pip->progename) ;
	    }
	}

#if	CF_DEBUGN
	debugprintf(NDF,"proginfo_progename: ret rs=%d en=%s\n",
	    rs,pip->progename) ;
#endif

#if	CF_DEBUGS
	debugprintf("proginfo_progename: ret rs=%d\n",rs) ;
	debugprintf("proginfo_progename: ret en=%s\n",pip->progename) ;
#endif

	return rs ;
}
/* end subroutine (proginfo_progename) */


/* ensure (set) nodename */
int proginfo_nodename(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		nl = 0 ;

	if (pip->nodename == NULL) {
	    const int	nlen = NODENAMELEN ;
	    char	nbuf[NODENAMELEN + 1] ;
	    if ((rs = getnodename(nbuf,nlen)) >= 0) {
	        cchar	**vpp = &pip->nodename ;
	        nl = rs ;
	        rs = proginfo_setentry(pip,vpp,nbuf,nl) ;
	    }
	} else {
	    nl = strlen(pip->nodename) ;
	}

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (proginfo_nodename) */


int proginfo_getename(PROGINFO *pip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("proginfo_getename: ent pe=%s\n",pip->progename) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"proginfo_getename: ent pe=%s\n",pip->progename) ;
#endif

	if (pip == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if ((rs = proginfo_progename(pip)) >= 0) {
	    if (pip->progename != NULL) {
	        rs = sncpy1(rbuf,rlen,pip->progename) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("proginfo_getename: ret rs=%d\n",rs) ;
	debugprintf("proginfo_getename: ret pe=%s\n",pip->progename) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"proginfo_getename: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (proginfo_getename) */


/* get the PWD when it was first set */
int proginfo_getpwd(PROGINFO *pip,char *rbuf,int rlen)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	rbuf[0] = '\0' ;
	if (pip->pwd == NULL) {
	    rs = proginfo_pwd(pip) ;
	}

	if (rs >= 0) {
	    if (rlen >= pip->pwdlen) {
	        rs = sncpy1(rbuf,rlen,pip->pwd) ;
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	}

	return rs ;
}
/* end subroutine (proginfo_getpwd) */


int proginfo_getenv(PROGINFO *pip,cchar *np,int nl,cchar **rpp)
{
	int		rs = SR_NOTFOUND ;

	if (np == NULL) return SR_FAULT ;

	if (pip->envv != NULL) {
	    rs = getev(pip->envv,np,nl,rpp) ;
	}

	return rs ;
}
/* end subroutine (proginfo_getenv) */


/* local subroutines */


static int proginfo_setdefnames(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->progname == NULL) {
	    if ((rs = proginfo_progename(pip)) >= 0) {
	        if (pip->progename != NULL) {
	            rs = proginfo_setprogname(pip,pip->progename) ;
	        }
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (proginfo_setdefnames) */


#ifdef	COMMENT

/* Set Default (program) Directory-Name */
static int proginfo_setdefdn(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->progdname == NULL) {
	    int		cl ;
	    cchar	*cp ;
	    if ((rs >= 0) && (pip->progename != NULL)) {
	        if ((cl = sfdirname(pip->progename,-1,&cp)) > 0) {
	            cchar	**vpp = &pip->progdname ;
	            rs = proginfo_setentry(pip,vpp,cp,cl) ;
	        }
	    } /* end if */
	} /* end if */

	return rs ;
}
/* end subroutine (proginfo_setdefdn) */


/* Set Default Program-Name */
static int proginfo_setdefpn(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip->progname == NULL) {
	    int		cl ;
	    cchar	*cp ;
	    if (pip->progename != NULL) {
	        if ((cl = sfbasename(pip->progename,-1,&cp)) > 0) {
	            cl = rmext(cp,cl) ;
	            if (cl == 0) {
	                cp = NOPROGNAME ;
	                cl = -1 ;
	            }
	        }
	        if ((cp != NULL) && (cl != 0)) {
	            cchar	**vpp = &pip->progdname ;
	            rs = proginfo_setentry(pip,vpp,cp,cl) ;
	        }
	    } /* end if */
	} /* end if */

	return rs ;
}
/* end subroutine (proginfo_setdefpn) */

#endif /* COMMENT */


static int hasourbad(cchar *sp,int sl)
{
	int		f = TRUE ;

	if (sp != NULL) {
	    if (! (f = hasprintbad(sp,sl))) {
	        uint	sch ;
	        int	i ;
	        if (sl < 0) sl = strlen(sp) ;
	        for (i = 0 ; i < sl ; i += 1) {
	            sch = (sp[i] & 0xff) ;
	            f = (sch >= 128) ;
	            if (f) break ;
	        } /* end for */
	    } /* end if */
	} /* end if */

	return f ;
}
/* end subroutine (hadourbad) */


