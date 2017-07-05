/* main */

/* program to switch programs */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGN	0		/* special debugging */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_PROCESN	0		/* need |procesn()| */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This subroutine was originally adapted from one of the programs used
	with the hardware CAD systems.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program takes its own invocation name and looks it up in a program
	map to find possible filepaths to an executable to execute.  If it
	doesn't find the program in the map, or if all of the program names
	listed in the map are not executable for some reason, then the program
	executable search PATH is searched.

	In all cases, some care is taken so as to not accidentally execute
	ourselves (again)!

	Synopsis:

	$ <name> <arguments_for_name>


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ids.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<varsub.h>
#include	<sbuf.h>
#include	<kvsfile.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	PROGINFO
#define	PROGINFO	PROGINFO
#endif

#define	NDF		"progswitch.deb"


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	sncpy2(char *,int,cchar *,cchar *) ;
extern int	sncpy3(char *,int,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	rmext(cchar *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	vecstr_envfile(vecstr *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnsub(cchar *,int,cchar *) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procfindprog(PROGINFO *,char *) ;
static int	procdevinfo(PROGINFO *) ;

static int	procfindfiles(PROGINFO *) ;
static int	procfindcooks(PROGINFO *,vecstr *) ;
static int	procfindsched(PROGINFO *,cchar **,vecstr *,cchar **,cchar *) ;
static int	procdefsadd(PROGINFO *,varsub *) ;
static int	procinfo(PROGINFO *,cchar *) ;

static int	procids_begin(PROGINFO *) ;
static int	procids_end(PROGINFO *) ;

static int	procfind_withmap(PROGINFO *,char *) ;
static int	procfind_withmapper(PROGINFO *,varsub *,char *) ;
static int	procfind_withpath(PROGINFO *,char *) ;

#if	CF_PROCESN
static int	procesn(PROGINFO *,char *,int) ;
#endif

static int	progmk(PROGINFO *,char *,int,cchar *,int,cchar *) ;
static int	progok(PROGINFO *,const char *) ;


/* local variables */

#ifdef	COMMENT
static const char	*roots[] = {
	    "HOME",
	    "LOCAL",
	    "AST",
	    "NCMP",
	    "GNU",
	    "TOOLS",
	    "XDIR",
	    NULL
} ;
#endif /* COMMENT */

static const struct pivars	initvars = {
	    VARPROGRAMROOT1,
	    VARPROGRAMROOT2,
	    VARPROGRAMROOT3,
	    PROGRAMROOT,
	    VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	    { SR_NOENT, EX_NOUSER },
	    { SR_AGAIN, EX_TEMPFAIL },
	    { SR_DEADLK, EX_TEMPFAIL },
	    { SR_NOLCK, EX_TEMPFAIL },
	    { SR_TXTBSY, EX_TEMPFAIL },
	    { SR_ACCESS, EX_NOPERM },
	    { SR_REMOTE, EX_PROTOCOL },
	    { SR_NOSPC, EX_TEMPFAIL },
	    { 0, 0 }} ;

static const char	*sched0[] = { /* map-directory file */
	    "%p/etc/%n/%n.%f",
	    "%p/etc/%n/%f",
	    "%p/etc/%n.%f",
	    NULL
} ;

static const char	*sched1[] = { /* "defs" file */
	    "%p/etc/%n/%n.%f",
	    "%p/etc/%n/%f",
	    "%p/etc/%n.%f",
	    "%p/etc/%f",
	    NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_OSERR ;

	const char	*pr = NULL ;
	const char	*progname = NULL ;
	const char	*cp ;
	char		tarname[MAXNAMELEN + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting x DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if (argc == 0) goto badprogstart ;

#if	CF_DEBUGS
	debugprintf("main: proginfo_start()\n") ;
#endif

	ex = EX_INFO ;
	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

#if	CF_DEBUGS
	debugprintf("main: getourenv()\n") ;
#endif

	if ((cp = getourenv(envv,VARDEBUGLEVEL)) != NULL) {
	    rs = optvalue(cp,-1) ;
	    pip->debuglevel = rs ;
	}

#if	CF_DEBUGS
	debugprintf("main: front2\n") ;
#endif

	if ((cp = getourenv(envv,VAREFNAME)) == NULL) cp = BFILE_STDERR ;

#if	CF_DEBUGS
	debugprintf("main: efname=%s\n",cp) ;
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if ((rs1 = bopen(&errfile,cp,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    debugprintf("main: stderr() rs=%d\n",rs) ;
	    fmt = "%s: starting debuglevel=%u\n" ;
	    bprintf(pip->efp,fmt,pn,pip->debuglevel) ;
	}
#endif

	if (rs >= 0) {
	    if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	    rs = proginfo_setbanner(pip,cp) ;
	}

/* initialize */

	pip->verboselevel = 1 ;
	pip->f.logprog = OPT_LOGPROG ;

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,NULL) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: setpiv() rs=%d\n",rs) ;
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: searchname=%s\n",pip->searchname) ;
	}
#endif

/* what is the name of this program */

	if (rs >= 0) {
	    int		pnlen ;
	    cchar	*argz = argv[0] ;
	    pip->tarname = argz ;
	    if ((pnlen = sfbasename(argz,-1,&progname)) > 0) {
	        if (progname[pnlen-1] != '\0') {
	            rs = mkpath1w(tarname,progname,pnlen) ;
	            pip->tarname = tarname ;
	        }
	    }
	}

	if (pip->lfname == NULL) pip->lfname = getourenv(envv,VARLFNAME) ;
	if (pip->lfname == NULL) pip->lfname = SEARCHNAME ;

	if (rs >= 0) {
	    USERINFO	u ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = proglog_begin(pip,&u)) >= 0) {
	            char	pbuf[MAXPATHLEN+1] ;
	            if ((rs = procfindprog(pip,pbuf)) > 0) {
	                const int	len = rs ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("main: exec rs=%d pbuf=%s\n",
					rs,pbuf) ;
#endif
	                if ((rs = procinfo(pip,pbuf)) >= 0) {
	                    if (len > 0) {

	                        bflush(pip->efp) ;
	                        {
	                            const char **eav = (const char **) argv ;
	                            const char **eev = (const char **) envv ;
	                            rs = u_execve(pbuf,eav,eev) ;
	                        }
	                        ex = EX_NOEXEC ;
	                        {
	                            fmt = "%s: could not exec prog=%s (%d)\n" ;
	                            bprintf(pip->efp,fmt,pn,pbuf,rs) ;
	                        }
	                    } else {
	                        fmt = "%s: could not find prog=%s (%d)\n" ;
	                        bprintf(pip->efp,fmt,pn,pip->tarname,rs) ;
	                        ex = EX_NOPROG ;
	                    }
	                } /* end if (procinfo) */
		    } else {
	                fmt = "%s: cannot find program (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                fmt = "cannot find program (%d)" ;
			proglog_printf(pip,fmt,rs) ;
	                ex = EX_NOPROG ;
	            } /* end if (procfindprog) */
	            rs1 = proglog_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (proglog) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
		fmt = "%s: cannot identify user (%d)\n" ;
		bprintf(pip->efp,fmt,pn,rs) ;
		ex = EX_NOUSER ;
	    }
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: error (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    ex = EX_OSERR ;
	} /* end if (ok) */

/* done */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: done ex=%u (%d)\n",ex,rs) ;
	}
#endif

	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_INTR:
	        ex = EX_INTR ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if */

/* early return thing */

	if (pip->efp != NULL) {
	    pip->f.errfile = TRUE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: early ex=%u (%d)\n",ex,rs) ;
	}
#endif

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int procfindprog(PROGINFO *pip,char *pbuf)
{
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
	if ((rs = procdevinfo(pip)) >= 0) {
	    if ((rs = procids_begin(pip)) >= 0) {
	        if ((rs = procfindfiles(pip)) >= 0) {
	            if ((rs = procfind_withmap(pip,pbuf)) >= 0) {
	                len = rs ;
	                if (rs == 0) {
	                    rs = procfind_withpath(pip,pbuf) ;
	                    len = rs ;
	                }
	            } /* end if (procfind_withmap) */
	        } /* end if (procfindfiles) */
	        rs1 = procids_end(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procids) */
	} /* end if (procdevinfo) */
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procfindprog) */


static int procdevinfo(PROGINFO *pip)
{
	const int	tlen = MAXPATHLEN ;
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = proginfo_getename(pip,tbuf,tlen)) >= 0) {
	    USTAT	sb ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procdevinfo: tbuf=%s\n",tbuf) ;
#endif
	    if ((rs = u_stat(tbuf,&sb)) >= 0) {
	        pip->dev = sb.st_dev ;
	        pip->ino = sb.st_ino ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procdevinfo: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end suborutine (procdevinfo) */


static int procfindfiles(PROGINFO *pip)
{
	VECSTR		sv ;
	int		rs ;
	int		rs1 ;
	if ((rs = vecstr_start(&sv,6,0)) >= 0) {
	    cchar	**vpp ;
	    cchar	**sa ;
	    cchar	*fn ;
	    if ((rs = procfindcooks(pip,&sv)) >= 0) {
	        sa = sched0 ;
	        fn = MAPFNAME ;
	        vpp = &pip->mfname ;
	        if ((rs = procfindsched(pip,vpp,&sv,sa,fn)) >= 0) {
	            sa = sched1 ;
	            fn = DEFSFNAME ;
	            vpp = &pip->dfname ;
	            rs = procfindsched(pip,vpp,&sv,sa,fn) ;
	        }
	    }
	    rs1 = vecstr_finish(&sv) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */
	return rs ;
}
/* end subroutine (procfindfiles) */


static int procfindcooks(PROGINFO *pip,vecstr *slp)
{
	int		rs ;
	int		c = 0 ;
	{
	    int		i ;
	    int		vl ;
	    cchar	*esn = SEARCHNAME ;
	    cchar	*keys = "pen" ;
	    cchar	*vp ;
	    char	kbuf[2] = { 0, 0  } ;
	    for (i = 0 ; keys[i] != '\0' ; i += 1) {
	        const int	kch = MKCHAR(keys[i]) ;
	        vl = -1 ;
	        vp = NULL ;
	        switch (kch) {
	        case 'p':
	            vp = pip->pr ;
	            break ;
	        case 'e':
	            vp = "etc" ;
	            break ;
	        case 'n':
	            vp = esn ;
	            break ;
	        } /* end switch */
	        if (vp != NULL) {
	            kbuf[0] = kch ;
	            c += 1 ;
	            rs = vecstr_envset(slp,kbuf,vp,vl) ;
	        }
	        if (rs < 0) break ;
	    } /* end for */
	} /* end block */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfindcooks) */


/* process Exec-Search-Name */
#if	CF_PROCESN
static int procesn(PROGINFO *pip,char *ebuf,int elen)
{
	int		rs = SR_OK ;
	cchar		*progename = pip->progename ;
	ebuf[0] = '\0' ;
	if (progename != NULL) {
	    if (strpbrk(progename,"/.") != NULL) {
	        int	bnl ;
	        cchar	*bnp ;
	        if ((bnl = sfbasename(progename,-1,&bnp)) > 0) {
	            if ((bnl = rmext(bnp,bnl)) > 0) {
	                rs = sncpy1w(ebuf,elen,bnp,bnl) ;
	            }
	        }
	    } else {
	        rs = sncpy1(ebuf,elen,pip->progename) ;
	    }
	}
	return rs ;
}
/* end subroutine (procesn) */
#endif /* CF_PROCESN */


static int procfindsched(PROGINFO *pip,cchar **vpp,vecstr *slp,cchar **sa,
		cchar *fn)
{
	const int	tlen = MAXPATHLEN ;
	const int	am = R_OK ;
	int		rs ;
	int		f = FALSE ;
	char		tbuf[MAXPATHLEN+1] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfindsched: ent fn=%s\n",fn) ;
	}
#endif
	tbuf[0] = '\0' ;
	if ((rs = permsched(sa,slp,tbuf,tlen,fn,am)) >= 0) {
	    rs = proginfo_setentry(pip,vpp,tbuf,rs) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfindsched: ret tbuf=%s\n",tbuf) ;
	    debugprintf("main/procfindsched: ret rs=%d f=%u\n",rs,f) ;
	}
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procfindsched) */


static int procids_begin(PROGINFO *pip)
{
	int		rs ;
	IDS		*idp = &pip->id ;
	if ((rs = ids_load(idp)) >= 0) {
	    pip->open.id = TRUE ;
	}
	return rs ;
}
/* end subroutine (procids_begin) */


static int procids_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.id) {
	    IDS		*idp = &pip->id ;
	    pip->open.id = FALSE ;
	    rs1 = ids_release(idp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procids_end) */


static int procfind_withmap(PROGINFO *pip,char *pbuf)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfind_withmap: ent\n") ;
	    debugprintf("main/procfind_withmap: mfname=%s\n",pip->mfname) ;
	}
#endif
	pbuf[0] = '\0' ;
	if (pip->mfname != NULL) {
	    USTAT	sb ;
	    if ((rs = uc_stat(pip->mfname,&sb)) >= 0) {
	        VARSUB		sdefs ;
	        const int	vo = VARSUB_ONOBLANK ;
	        if ((rs = varsub_start(&sdefs,vo)) >= 0) {
	            if ((rs = varsub_addvaquick(&sdefs,pip->envv)) >= 0) {
	                if ((rs = procdefsadd(pip,&sdefs)) >= 0) {
	                    rs = procfind_withmapper(pip,&sdefs,pbuf) ;
	                    len = rs ;
	                }
	            }
	            rs1 = varsub_finish(&sdefs) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (varsub-sdefs) */
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    } /* end if (kvsfile) */
	} /* end if (non-null) */
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfind_withmap: ret p=%s\n",pbuf) ;
	    debugprintf("main/procfind_withmap: ret rs=%d len=%u\n",rs,len) ;
	}
#endif
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procfind_withmap) */


static int procfind_withmapper(PROGINFO *pip,varsub *sdp,char *pbuf)
{
	KVSFILE		kf ;
	KVSFILE_CUR	cur ;
	int		rs ;
	int		rs1 ;
	int		len = 0 ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfind_withmapper: ent\n") ;
	}
#endif
#if	CF_DEBUGN
	nprintf(NDF,"main/procfind_withmapper: ent\n") ;
#endif
	pbuf[0] = '\0' ;
	if ((rs = kvsfile_open(&kf,20,pip->mfname)) >= 0) {
	    if ((rs = kvsfile_curbegin(&kf,&cur)) >= 0) {
	        const int	plen = MAXPATHLEN ;
	        const int	vlen = MAXPATHLEN ;
	        int		vl ;
	        cchar		*tarname = pip->tarname ;
	        char		vbuf[MAXPATHLEN+1] ;
#if	CF_DEBUGN
	nprintf(NDF,"main/procfind_withmapper: tarname=%s\n",tarname) ;
#endif
	        while (rs >= 0) {
	            vl = kvsfile_fetch(&kf,tarname,&cur,vbuf,vlen) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if (rs < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                debugprintf("main/wn: kvsfile_fetch() rs=%d name=%s\n",
	                    rs,vbuf) ;
	            }
#endif

#if	CF_DEBUGN
	            nprintf(NDF,"main/wn: kvsfile_fetch() rs=%d vbuf=%s\n",
	                    rs,vbuf) ;
#endif

	            if (rs >= 0) {
	                if ((rs = varsub_expand(sdp,pbuf,plen,vbuf,vl)) >= 0) {
	                    const int	pl = rs ;

#if	CF_DEBUG
			    if (DEBUGLEVEL(4)) {
	                        debugprintf("main/wm: rs=%d pbuf=%s\n",
				rs,pbuf) ;
	                    }
#endif
#if	CF_DEBUGN
	                    nprintf(NDF,"main/wn: rs=%d pbuf=%s\n",
				rs,pbuf) ;
#endif

	                    if (pl > 0) {
	                        if ((rs = progok(pip,pbuf)) > 0) {
	                            len = pl ;
	                        }
	                    }
	                } /* end if (varsub_expand) */
	            } /* end if (ok) */
	            if (len > 0) break ;
	        } /* end while */

	        rs1 = kvsfile_curend(&kf,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (kvsfile-cur) */
	    rs1 = kvsfile_close(&kf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (kbsfile) */
	if (rs >= 0) {
	    proglog_printf(pip,"with-mapper=%u\n",len) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfind_withmaper: ret p=%s\n",pbuf) ;
	    debugprintf("main/procfind_withmaper: ret rs=%d len=%u\n",rs,len) ;
	}
#endif
#if	CF_DEBUGN
	nprintf(NDF,"main/procfind_withmapper: rs=%d len=%u\n",rs,len) ;
#endif
	return (rs >= 0) ? len : rs ;
}
/* end sugroutine (procfind_withmapper) */


/* if we don't have a program file yet, search the execution PATH */
static int procfind_withpath(PROGINFO *pip,char *pbuf)
{
	const int	plen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		len = 0 ;
	cchar		*sp ;
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfind_withpath: ent\n") ;
	}
#endif
#if	CF_DEBUGN
	nprintf(NDF,"main/procfind_withpath: ent\n") ;
#endif
	if ((sp = getenv(VARPATH)) != NULL) {
	    int		cl ;
	    cchar	*cp ;
	    cchar	*tarname = pip->tarname ;
	    cchar	*tp ;

	    while ((tp = strchr(sp,':')) != NULL) {

	        cp = sp ;
	        cl = (tp - sp) ;
	        if ((rs = progmk(pip,pbuf,plen,cp,cl,tarname)) >= 0) {
	            const int	pl = rs ;
	            if ((rs = progok(pip,pbuf)) > 0) {
	                len = pl ;
	            }
	        }
	        sp = tp + 1 ;
	        if (len > 0) break ;
	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (len == 0) && (sp[0] != '\0')) {
	        if ((rs = progmk(pip,pbuf,plen,sp,-1,tarname)) >= 0) {
	            const int	pl = rs ;
	            if ((rs = progok(pip,pbuf)) > 0) {
	                len = pl ; /* marker */
	            }
	        }
	    }

	} /* end if (getenv-path) */
	if (rs >= 0) {
	    proglog_printf(pip,"with-path=%u\n",len) ;
	}
#if	CF_DEBUGN
	nprintf(NDF,"main/procfind_withpath: rs=%d len=%u\n",rs,len) ;
	nprintf(NDF,"main/procfind_withpath: ret p=%s\n",pbuf) ;
#endif
#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procfind_withpath: ret p=%s\n",pbuf) ;
	    debugprintf("main/procfind_withpath: ret rs=%d len=%u\n",rs,len) ;
	}
#endif
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (procfind_withpath) */


static int procdefsadd(PROGINFO *pip,varsub *sdp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	if (pip->dfname != NULL) {
	    const int	am = R_OK ;
	    if ((rs = perm(pip->dfname,-1,-1,NULL,am)) >= 0) {
	        vecstr	defs ;
	        if ((rs = vecstr_start(&defs,0,0)) >= 0) {
	            if ((rs = vecstr_envfile(&defs,pip->dfname)) >= 0) {
	                cchar	**va ;
	                if ((rs = vecstr_getvec(&defs,&va)) >= 0) {
	                    rs = varsub_addva(sdp,va) ;
	                    c = rs ;
	                }
	            }
	            rs1 = vecstr_finish(&defs) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (vecstr-defs) */
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	} /* end if (non-null) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procdefsadd) */


static int procinfo(PROGINFO *pip,cchar *pbuf)
{
	int		rs1 ;
	if ((rs1 = proglog_printf(pip,"prog=%s",pbuf)) >= 0) {
	    proglog_flush(pip) ;
	}
	return SR_OK ;
}
/* end subroutine (procinfo) */


static int progmk(PROGINFO *pip,char *rbuf,int rlen,cchar *pp,int pl,cchar *pn)
{
	SBUF		pbuf ;
	int		rs ;
	int		rl = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if (pl < 0) pl = strlen(pp) ;

	if ((rs = sbuf_start(&pbuf,rbuf,rlen)) >= 0) {

	    if (pl > 0) {
	        sbuf_strw(&pbuf,pp,pl) ;
	        if (pp[pl-1] != '/')
	            sbuf_char(&pbuf,'/') ;
	    }

	    sbuf_strw(&pbuf,pn,-1) ;

	    rl = sbuf_finish(&pbuf) ;
	    if (rs >= 0) rs = rl ;
	} /* end if */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (progmk) */


static int progok(PROGINFO *pip,cchar *progfname)
{
	struct ustat	sb ;
	int		rs ;
	int		f = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/progok: ent p=%s\n",progfname) ;
	}
#endif

	if ((rs = u_stat(progfname,&sb)) >= 0) {
	    if ((rs = sperm(&pip->id,&sb,X_OK)) >= 0) {
	        f = TRUE ;
	        if ((sb.st_dev == pip->dev) && (sb.st_ino == pip->ino)) {
	            f = FALSE ;
	        }
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/progok: ret rs=%d f=%u\n",rs,f) ;
	}
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (progok) */


