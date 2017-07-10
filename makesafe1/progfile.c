/* progfile */

/* process a filename */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEFEXTS	0		/* define DEP-EXTS */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	The subroutine was written from scratch.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Check the corresponding C language file for newer dependencies.

	Implementation notes:

	Errors look like:

	"main.c", line 42: Can't find include file vsystem.h


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<spawnproc.h>
#include	<filebuf.h>
#include	<sbuf.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dirlist.h"


/* local defines */

#define	ARGBUFLEN	(MAXPATHLEN + 3)
#define	NDEPS		100		/* default values */

#define	CPPERR		struct cpperr


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkaltext(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfsub(const char *,int,const char *,const char **) ;
extern int	sfdequote(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;

extern int	progeprintf(PROGINFO *,const char *,...) ;
extern int	progout_printf(PROGINFO *,const char *,...) ;
extern int	progalready_lookup(PROGINFO *,cchar *,int,time_t *) ;

extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */

struct lstate {
	int		f_continue ;
} ;

struct cpperr {
	const char	*fname ;
	const char	*ifname ;
	int		line ;
} ;


/* forward references */

static int depscheck(PROGINFO *,const char *,time_t,char *) ;
static int depsget(PROGINFO *,VECSTR *,VECOBJ *,const char *) ;
static int proclines(PROGINFO *,VECSTR *,int) ;
static int procline(PROGINFO *,VECSTR *,struct lstate *,cchar *,int) ;
static int procerr(PROGINFO *,VECOBJ *,int) ;
static int procerrline(PROGINFO *,VECOBJ *,const char *,int) ;
static int mkdepname(char *,const char *) ;

static int cpperr_start(CPPERR *,int,const char *,int) ;
static int cpperr_ifname(CPPERR *,const char *,int) ;
static int cpperr_finish(CPPERR *) ;


/* global variables */


/* local variables */

static const char	errsub1[] = ", line " ;
static const char	errsub2[] = ": Can't find include file " ;

#if	CF_DEFEXTS
static const char	*depexts[] = {
	"c",
	"cc",
	"cp",
	"cpp",
	"c++",
	"C",
	NULL
} ;
#endif /* CF_DEFEXTS */


/* exported subroutines */


int progfile(PROGINFO *pip,cchar *name)
{
	struct ustat	sb ;
	time_t		mtime_o ;
	time_t		mtime_c ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_remove = FALSE ;
	const char	*depout = NULL ;
	char		depfname[MAXPATHLEN + 1] ;
	char		outfname[MAXPATHLEN + 1] ;

	if (name == NULL) return SR_FAULT ;

	if ((name[0] == '\0') || (name[0] == '-')) return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: name=%s\n",name) ;
#endif /* CF_DEBUG */

/* is the file there at all? */

	rs1 = u_stat(name,&sb) ;

	mtime_o = sb.st_mtime ;
	if ((rs1 < 0) || (! S_ISREG(sb.st_mode))) {
	    rs = SR_OK ;
	    goto done ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("progfile: mtime=%s\n",
	        timestr_log(mtime_o,timebuf)) ;
	}
#endif /* CF_DEBUG */

/* is there a dependency name? */

	rs = mkdepname(depfname,name) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("progfile: mkdepname() rs=%d\n",rs) ;
	    debugprintf("progfile: depfname=%s\n",depfname) ;
	}
#endif /* CF_DEBUG */

	if (rs == SR_INVALID) {
	    rs = SR_OK ;
	    goto done ;
	} else if (rs < 0)
	    goto done ;

/* does the dependency exist? (return if not) */

	rs1 = u_stat(depfname,&sb) ;
	mtime_c = sb.st_mtime ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: dep u_stat() rs=%d\n",rs1) ;
#endif

	if (rs1 < 0) goto done ; /* return if dependency does not exist */

/* check the C-language file against its object */

	if (mtime_c > mtime_o) {
	    depout = depfname ;
	    f_remove = TRUE ;
	}

/* check the dependencies of the C-language file */

	if (! f_remove) {

	    depout = outfname ;
	    rs = depscheck(pip,depfname,mtime_o,outfname) ;

	    f_remove = (rs > 0) ;
	    if ((rs >= 0) && f_remove && (outfname[0] != '\0')) {
	        if (strncmp(outfname,"./",2) == 0) {
	            depout = outfname + 2 ;
		}
	    } /* end if */

	} /* end if (checking dependencies) */

/* remove the file if necessary */

	if ((rs >= 0) && f_remove) {

	    if (pip->debuglevel > 0)
	        progeprintf(pip,"%s: removing=%s dep=%s\n",
	            pip->progname,name, 
	            ((depout != NULL) ? depout : "")) ;

	    if (pip->verboselevel > 0) {
	        if (pip->verboselevel == 2) {
	            progout_printf(pip,"%s\n",name) ;
	        } else if (pip->verboselevel > 2) {
	            progout_printf(pip,"%s %s\n",name,
	                ((depout != NULL) ? depout : "")) ;
		}
	    } /* end if */

	    if (! pip->f.nochange)
	        u_unlink(name) ;

	} /* end if (removing file) */

done:
	if (pip->debuglevel > 0)
	    progeprintf(pip,"%s: progfile (%d:%u)\n",
	        pip->progname,rs,f_remove) ;

ret0:
	return (rs >= 0) ? f_remove : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


/* check the dependencies agains the original (object) file */
static int depscheck(pip,depfname,mtime_o,outfname)
PROGINFO	*pip ;
const char	depfname[] ;
time_t		mtime_o ;
char		outfname[] ;
{
	struct ustat	sb ;
	VECSTR		deps ;
	VECOBJ		errs ;
	time_t		mtime ;
	int		rs, rs1 ;
	int		size ;
	int		i ;
	int		opts ;
	int		f_remove = FALSE ;
	const char	*cp ;

	if (outfname != NULL) outfname[0] = '\0' ;

/* prepare to get the dependencies */

	opts = 0 ;
	rs = vecstr_start(&deps,NDEPS,opts) ;
	if (rs < 0)
	    goto ret0 ;

	size = sizeof(CPPERR) ;
	rs = vecobj_start(&errs,size,10,0) ;
	if (rs < 0)
	    goto ret1 ;

	rs = depsget(pip,&deps,&errs,depfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: depsget() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    for (i = 0 ; vecstr_get(&deps,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;
	        debugprintf("progfile: depfname=%s\n",cp) ;
	    }
	}
#endif /* CF_DEBUG */

/* pop our file if any dependency is younger */

	for (i = 0 ; vecstr_get(&deps,i,&cp) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: dep=%s\n",cp) ;
#endif

	    if (pip->open.cache) {
	        rs1 = progalready_lookup(pip,cp,-1,&mtime) ;
	    } else {
	        rs1 = u_stat(cp,&sb) ;
	        mtime = sb.st_mtime ;
	    } /* end if */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        char	timebuf[TIMEBUFLEN + 1] ;
	        debugprintf("progfile: stat rs=%d mtime=%s\n",
	            rs1,timestr_log(mtime,timebuf)) ;
	    }
#endif

	    if ((rs1 >= 0) && (mtime > mtime_o)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("progfile: remove\n") ;
#endif

	        if (outfname != NULL)
	            mkpath1(outfname,cp) ;

	        f_remove = TRUE ;
	        break ;
	    }

	} /* end for */

/* print out any errors that there may be */

	if (! pip->f.quiet) {
	    CPPERR	*ep ;
	    for (i = 0 ; vecobj_get(&errs,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	        progeprintf(pip,"%s: dep=%s:%u missing inc=%s\n",
	            pip->progname,
	            ep->fname,ep->line,
	            ep->ifname) ;

		}
	    } /* end for */
	} /* end if (missing include files) */

/* free up and get out */
ret2:
	{
	    CPPERR	*ep ;
	    for (i = 0 ; vecobj_get(&errs,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	        cpperr_finish(ep) ;
		}
	    } /* end for */
	} /* end block */

	vecobj_finish(&errs) ;

ret1:
	vecstr_finish(&deps) ;

ret0:
	if (pip->debuglevel > 0) {
	    progeprintf(pip,"%s: depscheck (%d)\n",
	        pip->progname,rs) ;
	}

	return (rs >= 0) ? f_remove : rs ;
}
/* end subroutine (depscheck) */


/* get the dependencies for the given file */
static int depsget(pip,dp,errp,fname)
PROGINFO	*pip ;
VECSTR		*dp ;
VECOBJ		*errp ;
const char	fname[] ;
{
	SPAWNPROC	psa ;
	DIRLIST_CUR	cur ;
	VECSTR		args ;
	const mode_t	operms = 0664 ;

	int		rs ;
	int		cl ;
	int		opts ;
	int		oflags ;
	int		fd_out ;
	int		fd_err ;
	int		cstat ;
	const char	**av ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		outfname[MAXPATHLEN + 1] ;
	char		errfname[MAXPATHLEN + 1] ;
	char		argbuf[ARGBUFLEN + 1] ;

	errfname[0] = '\0' ;
	outfname[0] = '\0' ;
	opts = VECSTR_OCOMPACT ;
	rs = vecstr_start(&args,10,opts) ;
	if (rs < 0)
	    goto ret0 ;

/* get a basename for the zeroth program argument */

	cl = sfbasename(pip->prog_cpp,-1,&cp) ;

/* create the argument list for the CPP program */

	vecstr_add(&args,cp,cl) ;

	vecstr_add(&args,"-M",-1) ;

/* loop, adding include-directories to the argument list */

	strcpy(argbuf,"-I") ;

	if ((rs = dirlist_curbegin(&pip->incs,&cur)) >= 0) {
	    char	*ap = (argbuf+2) ;
	    int		al = MAXPATHLEN ;
	    while (rs >= 0) {
	        int	rs1 = dirlist_enum(&pip->incs,&cur,ap,al) ;
	        if (rs1 == SR_NOTFOUND) break ;
	        rs = rs1 ;

	        if (rs >= 0) {
	            rs = vecstr_add(&args,argbuf,-1) ;
		}

	    } /* end while */

	    dirlist_curend(&pip->incs,&cur) ;
	} /* end if (cursor) */

	if (rs < 0) goto ret1 ;

/* add the filename itself */

	rs = vecstr_add(&args,fname,-1) ;
	if (rs < 0)
	    goto ret1 ;

	oflags = O_RDWR ;

/* prepare STDERR for the program */

	mkpath2(tmpfname,pip->udname,"seXXXXXXXXXX") ;

	rs = opentmpfile(tmpfname,oflags,operms,errfname) ;
	fd_err = rs ;
	if (rs < 0)
	    goto ret1 ;

	if (errfname[0] != '\0') {
	    u_unlink(errfname) ;
	    errfname[0] = '\0' ;
	}

/* prepare STDOUT for the program */

	mkpath2(tmpfname,pip->udname,"soXXXXXXXXXX") ;

	rs = opentmpfile(tmpfname,oflags,operms,outfname) ;
	fd_out = rs ;
	if (rs < 0)
	    goto ret2 ;

	if (outfname[0] != '\0') {
	    u_unlink(outfname) ;
	    outfname[0] = '\0' ;
	}

/* start the program */

	vecstr_getvec(&args,&av) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    int	i ;
	    for (i = 0 ; av[i] != NULL ; i += 1)
	        debugprintf("depsget: arg%u=>%s<\n",i,av[i]) ;
	}
#endif

	memset(&psa,0,sizeof(SPAWNPROC)) ;
	psa.disp[0] = SPAWNPROC_DCLOSE ;
	psa.disp[1] = SPAWNPROC_DDUP ;
	psa.disp[2] = SPAWNPROC_DDUP ;
	psa.fd[1] = fd_out ;
	psa.fd[2] = fd_err ;

	if ((rs = spawnproc(&psa,pip->prog_cpp,av,pip->envv)) >= 0) {
	    const pid_t	pid = rs ;
	    while (rs == 0) {
	        rs = u_waitpid(pid,&cstat,WUNTRACED) ;
	        if (rs == SR_INTR) rs = SR_OK ;
	    }
	    if (rs >= 0) {
	        if ((rs = proclines(pip,dp,fd_out)) >= 0) {
	            rs = procerr(pip,errp,fd_err) ;
	        }
	    }
	} /* end if spawnproc) */

/* done */

	u_close(fd_out) ;

ret2:
	u_close(fd_err) ;

ret1:
	vecstr_finish(&args) ;

	{
	    char	*p ;
	    p = outfname ;
	    if (p[0] != '\0') {
	        u_unlink(p) ;
	        p[0] = '\0' ;
	    }
	    p = errfname ;
	    if (p[0] != '\0') {
	        u_unlink(p) ;
	        p[0] = '\0' ;
	    }
	}

ret0:
	if ((pip->debuglevel > 0) && (rs < 0))
	    progeprintf(pip,"%s: depsget (%d)\n",
	        pip->progname,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("depsget: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (depsget) */


/* process the lines that contain dependency names */
static int proclines(pip,dp,fd)
PROGINFO	*pip ;
VECSTR		*dp ;
int		fd ;
{
	FILEBUF		buf ;
	const int	to = pip->to_read ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    struct ustat	sb ;
	    int	rs1 ;
	    debugprintf("proclines: ent\n") ;
	    rs1 = u_fstat(fd,&sb) ;
	    debugprintf("proclines: u_fstat() rs=%d\n",rs1) ;
	    debugprintf("proclines: fsize=%u\n",sb.st_size) ;
	}
#endif

	if ((rs = u_rewind(fd)) >= 0) {
	    if ((rs = filebuf_start(&buf,fd,0L,BUFLEN,0)) >= 0) {
	        struct lstate	ls ;
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        memset(&ls,0,sizeof(struct lstate)) ;
	        while ((rs = filebuf_readline(&buf,lbuf,llen,to)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("proclines: line=>%t<\n",
	                    lbuf,
	                    ((lbuf[len - 1] == '\n') ? (len - 1) : len)) ;
#endif /* CF_DEBUG */

	            rs = procline(pip,dp,&ls,lbuf,len) ;
	            c += rs ;

	            if (rs < 0) break ;
	        } /* end while */

	        filebuf_finish(&buf) ;
	    } /* end if (filebuf) */
	} /* end if (rewind) */

	if ((pip->debuglevel > 0) && (rs < 0)) {
	    progeprintf(pip,"%s: proclines (%d)\n",
	        pip->progname,rs) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclines) */


/* process a dependecy line */
static int procline(pip,dp,lsp,lbuf,len)
PROGINFO	*pip ;
VECSTR		*dp ;
struct lstate	*lsp ;
const char	lbuf[] ;
int		len ;
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		f_continue = FALSE ;
	int		c = 0 ;
	const char	*tp, *sp, *cp ;

	if ((len > 1) && (lbuf[len - 1] == CH_BSLASH)) {
	    f_continue = TRUE ;
	    len -= 1 ;
	}

	sp = lbuf ;
	sl = len ;
	if (! lsp->f_continue) {
	    if ((tp = strnchr(sp,sl,':')) != NULL) {
	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;
	    }
	}

	while ((cl = nextfield(sp,sl,&cp)) > 0) {

	    if (cl && (vecstr_findn(dp,cp,cl) == SR_NOTFOUND)) {
	        c += 1 ;
	        rs = vecstr_add(dp,cp,cl) ;
	    }

	    sl -= ((cp + cl) - sp) ;
	    sp = (cp + cl) ;

	    if (rs < 0) break ;
	} /* end while */

	lsp->f_continue = f_continue ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procline) */


/* process the error output */
static int procerr(pip,errp,fd_err)
PROGINFO	*pip ;
VECOBJ		*errp ;
int		fd_err ;
{
	struct ustat	sb ;
	FILEBUF		buf ;
	const int	fsize = BUFLEN ;
	int		rs ;
	int		to = pip->to_read ;

	rs = u_fstat(fd_err,&sb) ;

	if ((rs >= 0) && (sb.st_size > 0)) {
	    rs = u_rewind(fd_err) ;
	}

	if ((rs >= 0) && (sb.st_size > 0)) {
	    if ((rs = filebuf_start(&buf,fd_err,0L,fsize,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("depsget: have some error stuff size=%u\n",
	                sb.st_size) ;
#endif

	        while (rs >= 0) {
	            rs = filebuf_readline(&buf,lbuf,llen,to) ;
	            len = rs ;
	            if (rs <= 0) break ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("depsget: line> %t\n",
	                    lbuf,len) ;
#endif

	            rs = procerrline(pip,errp,lbuf,len) ;

	        } /* end while */

/* print out the errors */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3)) {
	            int	i ;
	            CPPERR	*ep ;
	            debugprintf("depget: errors\n") ;
	            for (i = 0 ; vecobj_get(errp,i,&ep) >= 0 ; i += 1) {
	                if (ep == NULL) continue ;
	                debugprintf("depsget: fname=%s:%u inc=%s\n",
	                    ep->fname,ep->line,ep->ifname) ;
	            }
	        }
#endif /* CF_DEBUG */

	        filebuf_finish(&buf) ;
	    } /* end if (filebuf) */
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (procerr) */


/* process an error line */
static int procerrline(pip,errp,lbuf,len)
PROGINFO	*pip ;
VECOBJ		*errp ;
const char	lbuf[] ;
int		len ;
{
	int		rs = SR_OK ;
	int		cl, cl1, cl2 ;
	int		line ;
	const char	*cp ;
	const char	*cp1, *cp2 ;

	if ((cl2 = sfsub(lbuf,len,errsub2,&cp2)) >= 0) {
	    if ((cl1 = sfsub(lbuf,MIN(len,(cp2-lbuf)),errsub1,&cp1)) > 0) {
	        if ((cl = sfdequote(lbuf,(cp1-lbuf),&cp)) > 0) {
	            const int	dl = (cp2 - (cp1 + cl1)) ;
	            const char	*dp = (cp1 + cl1) ;
	            if ((rs = cfdeci(dp,dl,&line)) >= 0) {
			CPPERR	e ;
	                if ((rs = cpperr_start(&e,line,cp,cl)) >= 0) {
	                    int		sl = (len-((cp2+cl2)-lbuf)) ;
	                    const char	*sp = (cp2 + cl2) ;
	                    if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	                        if ((rs = cpperr_ifname(&e,cp,cl)) >= 0) {
	                            rs = vecobj_add(errp,&e) ;
	                        }
	                    }
	                    if (rs < 0)
	                        cpperr_finish(&e) ;
	                } /* end if (cpperr) */
	            } /* end if (cfdeci) */
	        } /* end if (sfdequote) */
	    } /* end if (sfsub) */
	} /* end if (sfsub) */

	return rs ;
}
/* end subroutine (procerrline) */


/* create the dependency name from the given name */
static int mkdepname(char *rbuf,cchar *name)
{
	SBUF		dep ;
	int		rs = SR_INVALID ;
	int		len = 0 ;
	const char	*tp ;

	if ((tp = strrchr(name,'.')) != NULL) {
	    if (tp[1] == 'o') {
	        const int	rlen = MAXPATHLEN ;
	        if ((rs = sbuf_start(&dep,rbuf,rlen)) >= 0) {

	            sbuf_strw(&dep,name,(tp - name)) ;

	            sbuf_char(&dep,'.') ;

	            sbuf_char(&dep,'c') ;

	            len = sbuf_finish(&dep) ;
	            if (rs >= 0) rs = len ;
	        } /* end if (SBUF) */
	    }
	} /* end if (non-null) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mkdepname) */


static int cpperr_start(CPPERR *ep,int line,cchar *fp,int fl)
{
	int		rs ;
	const char	*cp ;

	if (fp == NULL) return SR_FAULT ;

	ep->line = line ;
	if ((rs = uc_mallocstrw(fp,fl,&cp)) >= 0) {
	    ep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (cpperr_start) */


static int cpperr_ifname(CPPERR *ep,cchar *fp,int fl)
{
	int		rs ;
	const char	*cp ;

	if (fp == NULL) return SR_FAULT ;

	if ((rs = uc_mallocstrw(fp,fl,&cp)) >= 0) {
	    ep->ifname = cp ;
	}

	return rs ;
}
/* end subroutine (cpperr_ifname) */


static int cpperr_finish(CPPERR *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->fname != NULL) {
	    rs1 = uc_free(ep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->fname = NULL ;
	}

	if (ep->ifname != NULL) {
	    rs1 = uc_free(ep->ifname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->ifname = NULL ;
	}

	ep->line = 0 ;
	return rs ;
}
/* end subroutine (cpperr_finish) */


