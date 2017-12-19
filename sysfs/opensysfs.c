/* opensysfs */

/* open a channel (file-descriptor) to some system file or directory */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	We open a file or a directory to some system resource.

	Synopsis:

	int opensysfs(w,of,ttl)
	int		w ;
	int		of ;
	int		ttl ;

	Arguments:

	w		what resource
	of		open-flags
	ttl		time-to-live for the resource

	Returns:

	<0		error
	>=0		FD of dir-cache

	Notes:

	- open flags:
		O_NOTTY
		O_EXCL		
		O_SYNC
		O_NDELAY
		O_NONBLOCK
		O_CLOEXEC


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
#include	<endianstr.h>
#include	<ids.h>
#include	<vecpstr.h>
#include	<dirseen.h>
#include	<envhelp.h>
#include	<spawnproc.h>
#include	<localmisc.h>

#include	"opensysfs.h"


/* local defines */

#define	OPENSYSFS_PROGSYSFS	"sysfs"
#define	OPENSYSFS_PROGMKPWI	"mkpwi"

#define	SUFNAMELEN	USERNAMELEN
#define	ABUFLEN		USERNAMELEN

#define	REALNAMESUF	"pwi"

#define	MINPERMS	0664		/* minimum permissions on files */

#define	PASSWDFNAME	"/etc/passwd"
#define	GROUPFNAME	"/etc/group"
#define	PROJECTFNAME	"/etc/project"
#define	SHADOWFNAME	"/etc/shadow"
#define	USERATTRFNAME	"/etc/user_attr"


/* external subroutines */

extern int	snsd(char *,int,const char *,uint) ;
extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpyuc(char *,int,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfprogroot(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	strwcmp(const char *,const char *,int) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	dirseen_notseen(DIRSEEN *,struct ustat *,const char *,int) ;
extern int	dirseen_notadd(DIRSEEN *,struct ustat *,const char *,int) ;
extern int	isprintlatin(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern cchar	**environ ;


/* local structures */


/* forward references */

static int	mkrealpath(char *,int,cchar *,cchar *) ;
static int	opencfile(int,int,int) ;
static int	checkperms(const char *,struct ustat *,mode_t) ;

static int	findprog(IDS *,char *,cchar *) ;
static int	findprogbin(IDS *,DIRSEEN *,char *,cchar *,cchar *) ;

static int	runmkpwi(int,cchar *,int) ;
static int	runsysfs(int) ;


/* local variables */

static cchar	*dbfnames[] = { /* source database files */
	PASSWDFNAME,
	PASSWDFNAME,
	GROUPFNAME,
	PROJECTFNAME,
	PASSWDFNAME,
	GROUPFNAME,
	PROJECTFNAME,
	PASSWDFNAME,
	"/etc/shells",
	SHADOWFNAME,
	USERATTRFNAME,
	NULL
} ;

static cchar	*cfnames[] = {
	OPENSYSFS_FUSERHOMES,
	OPENSYSFS_FUSERNAMES,
	OPENSYSFS_FGROUPNAMES,
	OPENSYSFS_FPROJECTNAMES,
	OPENSYSFS_FPASSWD,
	OPENSYSFS_FGROUP,
	OPENSYSFS_FPROJECT,
	OPENSYSFS_FREALNAME,
	OPENSYSFS_FSHELLS,
	OPENSYSFS_FSHADOW,
	OPENSYSFS_FUSERATTR,
	NULL
} ;

static cchar	*prvars[] = {
	"EXTRA",
	"PREROOT",
	NULL
} ;

static cchar	*prdirs[] = {
	"/usr/extra",
	"/usr/preroot",
	NULL
} ;

static cchar	*prbins[] = {
	"sbin",
	"bin",
	NULL
} ;

static cchar	*envbads[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"RANDOM",
	"SECONDS",
	NULL
} ;


/* exported subroutines */


int opensysfs(int w,int of,int ttl)
{
	int		rs = SR_OK ;

	if (w < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("opensysfs/opencfile: w=%u ttl=%d\n",w,ttl) ;
#endif

	if (ttl < 0) ttl = OPENSYSFS_DEFTTL ;

	if ((of & O_ACCMODE) == O_RDONLY) {
	    switch (w) {
	    case OPENSYSFS_WUSERHOMES:
	    case OPENSYSFS_WUSERNAMES:
	    case OPENSYSFS_WPASSWD:
	        rs = opencfile(w,of,ttl) ;
	        break ;
	    case OPENSYSFS_WGROUPNAMES:
	    case OPENSYSFS_WGROUP:
	        rs = opencfile(w,of,ttl) ;
	        break ;
	    case OPENSYSFS_WPROJECTNAMES:
	    case OPENSYSFS_WPROJECT:
	        rs = opencfile(w,of,ttl) ;
	        break ;
	    case OPENSYSFS_WREALNAME:
	        rs = opencfile(w,of,ttl) ;
	        break ;
	    case OPENSYSFS_WSHELLS:
	        rs = opencfile(w,of,ttl) ;
	        break ;
	    case OPENSYSFS_WSHADOW:
	        rs = opencfile(w,of,ttl) ;
	        break ;
	    case OPENSYSFS_WUSERATTR:
	        rs = opencfile(w,of,ttl) ;
	        break ;
	    default:
	        rs = SR_INVALID ;
	        break ;
	    } /* end switch */
	} else {
	    rs = SR_BADF ;
	}

#if	CF_DEBUGS
	debugprintf("opensysfs: ret rs=%d \n",rs) ;
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (opensysfs) */


/* local subroutines */


static int opencfile(int w,int of,int ttl)
{
	int		rs ;
	int		fd = -1 ;
	cchar		*sdname = OPENSYSFS_SYSDNAME ;
	cchar		*gcname = cfnames[w] ;
	char		gfname[MAXPATHLEN+1] ;

#if	CF_DEBUGS
	debugprintf("opensysfs/opencfile: w=%u ttl=%d\n",w,ttl) ;
	debugprintf("opensysfs/opencfile: gcname=%s\n",gcname) ;
#endif

	if ((rs = mkrealpath(gfname,w,sdname,gcname)) > 0) {
	    struct ustat	sb ;
	    time_t		dt = 0 ;

#if	CF_DEBUGS
	    debugprintf("opensysfs/opencfile: gfname=%s\n",gfname) ;
#endif

	    if ((rs = u_stat(gfname,&sb)) >= 0) {
		mode_t	mm = MINPERMS ;
	        time_t	mt = sb.st_mtime ;

		switch (w) {
	        case OPENSYSFS_WSHADOW:
		    mm &= (~7) ;
		/* FALLTHROUGH */
		default:
	            rs = checkperms(gfname,&sb,mm) ;
		    break ;
		} /* end switch */

	        if ((rs >= 0) && (ttl >= 0)) {
	            switch (w) {
	            case OPENSYSFS_WUSERNAMES:
	            case OPENSYSFS_WGROUPNAMES:
	            case OPENSYSFS_WPROJECTNAMES:
	            case OPENSYSFS_WPASSWD:
	            case OPENSYSFS_WGROUP:
	            case OPENSYSFS_WPROJECT:
	            case OPENSYSFS_WREALNAME:
	            case OPENSYSFS_WSHELLS:
	            case OPENSYSFS_WSHADOW:
	            case OPENSYSFS_WUSERATTR:
	                if (dt == 0) dt = time(NULL) ;
	                if ((dt-mt) >= ttl) rs = SR_STALE ;
	                break ;
	            } /* end switch */
	        } /* end if (ttl) */

	        if ((rs >= 0) && (ttl >= 0)) {
	            switch (w) {
	            case OPENSYSFS_WUSERHOMES:
	                {
	                    const int	aw = OPENSYSFS_WUSERNAMES ;
	                    const char	*an ;
	                    char	tfname[MAXPATHLEN+1] ;
	                    an = cfnames[aw] ;
	                    if ((rs = mkpath2(tfname,sdname,an)) >= 0) {
	                        if ((rs = u_stat(tfname,&sb)) >= 0) {
	                            if (dt == 0) dt = time(NULL) ;
	                            if ((dt-sb.st_mtime) >= ttl) rs = SR_STALE ;
	                        } /* end if (stat) */
	                    } /* end if (mkpath) */
	                } /* end block */
	                break ;
	            } /* end switch */
	        } /* end if (alternate test) */

#if	CF_DEBUGS
	        debugprintf("opensysfs/opencfile: tar stat() rs=%d\n",rs) ;
#endif
	        if (rs >= 0) {
#if	CF_DEBUGS
	            debugprintf("opensysfs/opencfile: dbfname=%s\n",
	                dbfnames[w]) ;
#endif
	            if (strcmp(dbfnames[w],STDNULLFNAME) != 0) {
	                if ((rs = u_stat(dbfnames[w],&sb)) >= 0) {
	                    if (dt == 0) dt = time(NULL) ;
	                    if (sb.st_mtime > mt) rs = SR_STALE ;
	                }
	            } /* end if (not std-null) */
#if	CF_DEBUGS
	            debugprintf("opensysfs/opencfile: db stat() rs=%d\n",rs) ;
#endif
	        } /* end if (DB stat) */

	    } /* end if (stat) */

	    if ((rs == SR_NOENT) || (rs == SR_STALE)) {
	        const char	*tp ;
#if	CF_DEBUGS
	        debugprintf("opensysfs/opencfile: no-entry­stale rs=%d\n",
	            rs) ;
#endif
	        switch (w) {
	        case OPENSYSFS_WREALNAME:
	            if ((tp = strrchr(gfname,'.')) != NULL) {
	                rs = runmkpwi(w,gfname,(tp-gfname)) ;
	            }
	            break ;
	        default:
	            rs = runsysfs(w) ;
	            break ;
	        } /* end switch */
	    } /* end (not-found or stale) */

	    if (rs >= 0) {
	        if ((rs = u_open(gfname,of,0666)) >= 0) {
	            fd = rs ;
	            if (of & O_CLOEXEC) {
	                rs = uc_closeonexec(fd,TRUE) ;
		    }
	            if (rs < 0)
	                u_close(fd) ;
	        } /* end if (file-open) */
	    } /* end if (ok) */

	} /* end if (mkrealpath) */

#if	CF_DEBUGS
	{
	    struct ustat	sb ;
	    if (rs >= 0) u_fstat(rs,&sb) ;
	    debugprintf("opensysfs/opencfile: mode=\\o%08o\n",sb.st_mode) ;
	    debugprintf("opensysfs/opencfile: ret rs=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opencfile) */


static int mkrealpath(char *gfname,int w,cchar *sdname,cchar *gcname)
{
	int		rs = SR_OK ;

	switch (w) {
	case OPENSYSFS_WREALNAME:
	    {
	        const int	slen = SUFNAMELEN ;
	        const char	*suf = REALNAMESUF ;
	        char		sbuf[SUFNAMELEN+1] ;
	        char		cbuf[MAXNAMELEN+1] ;
	        if ((rs = sncpy2(sbuf,slen,suf,ENDIANSTR)) >= 0) {
	            if ((rs = snsds(cbuf,MAXNAMELEN,gcname,sbuf)) >= 0) {
	                rs = mkpath2w(gfname,sdname,cbuf,rs) ;
	            }
	        }
	    }
	    break ;
	default:
	    rs = mkpath2(gfname,sdname,gcname) ;
	    break ;
	} /* end switch */

	return rs ;
}
/* end subroutine (mkrealpath) */


static int checkperms(cchar *gfname,struct ustat *sbp,mode_t mm)
{
	int		rs = SR_OK ;

	if ((sbp->st_mode & mm) != mm) {
	    uid_t	uid = getuid() ;
	    if (sbp->st_uid == uid) {
	        mode_t	newm = (sbp->st_mode | mm) ;
	        rs = u_chmod(gfname,newm) ;
	    } else {
	        uc_unlink(gfname) ;
	        rs = SR_NOENT ;
	    }
	} /* end if (problem) */

	return rs ;
}
/* end subroutine (checkperms) */


/* ARGSUSED */
static int runmkpwi(int w,cchar *dbp,int dbl)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;
	const char	*pn = OPENSYSFS_PROGMKPWI ;

#if	CF_DEBUGS
	debugprintf("opensysfs/runmkpwi: ent db=%t\n",dbp,dbl) ;
#endif

	if ((rs = ids_load(&id)) >= 0) {
	    char	pfname[MAXPATHLEN+1] ;
	    if ((rs = findprog(&id,pfname,pn)) > 0) {
		const int	zlen = MAXNAMELEN ;
	        int		cs = 0 ;
	        const char	*av[3] ;
	        char		zbuf[MAXNAMELEN+1] ;

	        if ((rs = sncpyuc(zbuf,zlen,pn)) >= 0) {
	            ENVHELP	env ;
	            if ((rs = envhelp_start(&env,envbads,environ)) >= 0) {
	                cchar	**ev = NULL ;

	                if (rs >= 0) {
	                    cchar	*cp ;
	                    int		cl ;
	                    cchar	*evar = "MKPWI_PROGRAMROOT" ;
	                    if ((cl = sfprogroot(pfname,-1,&cp)) > 0) {
	                        rs = envhelp_envset(&env,evar,cp,cl) ;
	                    }
	                }

	                if (rs >= 0) {
	                    cchar	*evar = "MKPWI_DBNAME" ;
	                    rs = envhelp_envset(&env,evar,dbp,dbl) ;
	                }

	                if ((rs = envhelp_getvec(&env,&ev)) >= 0) {
	                    SPAWNPROC	ps ;

	                    av[0] = zbuf ;
	                    av[1] = NULL ;

	                    memset(&ps,0,sizeof(SPAWNPROC)) ;
	                    ps.disp[0] = SPAWNPROC_DCLOSE ;
	                    ps.disp[1] = SPAWNPROC_DCLOSE ;
	                    ps.disp[2] = SPAWNPROC_DNULL ;
	                    ps.opts |= SPAWNPROC_OIGNINTR ;
	                    ps.opts |= SPAWNPROC_OSETSID ;

	                    if ((rs = spawnproc(&ps,pfname,av,ev)) >= 0) {
	                        pid_t	pid = rs ;
	                        rs = u_waitpid(pid,&cs,0) ;
	                    } /* end if (spawned and waited for) */

	                } /* end if (envhelp-get) */

	                rs1 = envhelp_finish(&env) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (envhelp) */
	        } /* end if (sncpyuc) */

	    } /* end if (findprog) */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

#if	CF_DEBUGS
	debugprintf("opensysfs/runmkpwi: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (runmkpwi) */


static int runsysfs(int w)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;
	const char	*pn = OPENSYSFS_PROGSYSFS ;

	if ((rs = ids_load(&id)) >= 0) {
	    char	pfname[MAXPATHLEN+1] ;
	    if ((rs = findprog(&id,pfname,pn)) > 0) {
	        SPAWNPROC	ps ;
	        const int	alen = ABUFLEN ;
	        int		cs = 0 ;
	        const char	*av[3] ;
	        char		zbuf[MAXNAMELEN+1] ;
	        char		abuf[ABUFLEN+1] ;

	        if ((rs = sncpyuc(zbuf,MAXNAMELEN,pn)) >= 0) {
	            const char	**ev = NULL ;
	            if ((rs = ctdeci(abuf,alen,w)) >= 0) {

	                av[0] = zbuf ;
	                av[1] = abuf ;
	                av[2] = NULL ;

	                memset(&ps,0,sizeof(SPAWNPROC)) ;
	                ps.disp[0] = SPAWNPROC_DCLOSE ;
	                ps.disp[1] = SPAWNPROC_DCLOSE ;
	                ps.disp[2] = SPAWNPROC_DNULL ;
	                ps.opts |= SPAWNPROC_OIGNINTR ;
	                ps.opts |= SPAWNPROC_OSETSID ;

	                if ((rs = spawnproc(&ps,pfname,av,ev)) >= 0) {
	                    pid_t	pid = rs ;
	                    rs = u_waitpid(pid,&cs,0) ;
	                } /* end if (spawned and waited for) */

	            } /* end if (argument-preparation) */
	        } /* end if (sncpyuc) */

	    } /* end if (findprog) */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

	return rs ;
}
/* end subroutine (runsysfs) */


static int findprog(IDS *idp,char *pfname,cchar *pn)
{
	DIRSEEN		bhist, *blp = &bhist ;
	int		rs ;
	int		rs1 ;
	int		pl = 0 ;

	pfname[0] = '\0' ;
	if ((rs = dirseen_start(blp)) >= 0) {
	    VECPSTR	dhist ;
	    if ((rs = vecpstr_start(&dhist,4,0,0)) >= 0) {
	        const int	rsn = SR_NOTFOUND ;
	        int		i ;
	        int		f = FALSE ;
	        const char	*pr ;

	        for (i = 0 ; (rs >= 0) && (prvars[i] != NULL) ; i += 1) {
	            if ((pr = getenv(prvars[i])) != NULL) {
	                if ((rs = vecpstr_already(&dhist,pr,-1)) == rsn) {
	                    if ((rs = findprogbin(idp,blp,pfname,pr,pn)) > 0) {
	                        pl = rs ;
	                        f = TRUE ;
	                    }
	                    if ((rs >= 0) && (! f)) {
	                        rs = vecpstr_add(&dhist,pr,-1) ;
	                    }
	                }
	            } /* end if */
	            if (f) break ;
	            if (rs < 0) break ;
	        } /* end for */

	        if ((rs >= 0) && (! f)) {
	            for (i = 0 ; (rs >= 0) && (prdirs[i] != NULL) ; i += 1) {
	                pr = prdirs[i] ;
	                if ((rs = vecpstr_already(&dhist,pr,-1)) == rsn) {
	                    if ((rs = findprogbin(idp,blp,pfname,pr,pn)) > 0) {
	                        pl = rs ;
	                        f = TRUE ;
	                    }
	                    if ((rs >= 0) && (! f))
	                        vecpstr_add(&dhist,pr,-1) ;
	                } /* end if (not already) */
	                if (rs > 0) break ;
	            } /* end for */
	        } /* end if */

	        rs1 = vecpstr_finish(&dhist) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecpstr) */
	    rs1 = dirseen_finish(blp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dirseen) */

	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findprog) */


static int findprogbin(IDS *idp,DIRSEEN *dsp,char *pfname,cchar *pr,cchar *pn)
{
	struct ustat	sb ;
	int		rs ;
	int		pl = 0 ;
	int		f = FALSE ;
	if ((rs = u_stat(pr,&sb)) >= 0) {
	    if (S_ISDIR(sb.st_mode)) {
	        int		i ;
	        int		dl ;
	        char		bindname[MAXPATHLEN+1] ;
	        for (i = 0 ; (rs >= 0) && (prbins[i] != NULL) ; i += 1) {
	            if ((rs = mkpath2(bindname,pr,prbins[i])) >= 0) {
	                dl = rs ;
	                if ((rs = dirseen_notseen(dsp,&sb,bindname,rs)) > 0) {
	                    if ((rs = mkpath2(pfname,bindname,pn)) >= 0) {
	                        struct ustat	psb ;
	                        pl = rs ;
	                        if ((rs = u_stat(pfname,&psb)) >= 0) {
	                            if ((rs = sperm(idp,&psb,X_OK)) >= 0) {
	                                f = S_ISREG(psb.st_mode) ;
	                            }
	                        }
	                        if ((rs < 0) && isNotPresent(rs)) rs = SR_OK ;
	                        if ((rs >= 0) && (! f)) {
	                            rs = dirseen_notadd(dsp,&sb,bindname,dl) ;
	                        }
	                    } /* end if (mkpath) */
	                } /* end if (dirseen-notseen) */
	            } /* end if (mkpath) */
	        } /* end for */
	    } /* end if (was a directory) */
	} /* end if (stat) */
	if ((rs >= 0) && (! f)) pl = 0 ;
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (findprogbin) */


