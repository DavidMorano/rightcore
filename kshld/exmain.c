/* main */

/* program to switch programs */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 89/03/01, David Morano

	This subroutine was originally written.  This whole program,
	LOGDIR, is needed for use on the Sun CAD machines because Sun
	doesn't support LOGDIR or LOGNAME at this time.  There was a
	previous program but it is lost and not as good as this one
	anyway.  This one handles NIS+ also.  (The previous one
	didn't.) 


	= 98/06/01, David Morano

	I enhanced the program a little to print out some other user
	information besides the user's name and login home directory.


	= 99/03/01, David Morano

	I enhanced the program to also print out effective UID and
	effective GID.


*/


/**************************************************************************

	This program takes its own invocation name and looks it up
	in a program map to find possible filepaths to an executable
	to execute.  If it doesn't find the program in the map, or
	if all of the program names listed in the map are not executable
	for some reason, then the program executable search PATH is
	searched.

	In all cases, some care is taken so as to not accidentally
	execute ourselves (again) !

	Synopsis:

	$ <name> <arguments_for_name>


*****************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ids.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<sbuf.h>
#include	<kvsfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXPATHLEN + MAXNAMELEN + 1)
#endif

#ifndef	PWBUFLEN
#ifdef	PWENTRY_BUFLEN
#define	PWBUFLEN	PWENTRY_BUFLEN
#else
#define	PWBUFLEN	1024
#endif
#endif

#ifndef	GRBUFLEN
#define	GRBUFLEN	7296		/* from Solaris */
#endif

#ifndef	PJBUFLEN
#define	PJBUFLEN	(10 * 1024)
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	procxenv(vecstr *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

#define	A	(__STDC__ != 0) 
#define	B	defined(_POSIX_C_SOURCE) 
#define	C	defined(_XOPEN_SOURCE)

#if	(A != 0) || (B != 0) || (C != 0)
extern long	altzone ;
#endif


/* local structures */


/* forward references */

static int	progmk(struct proginfo *,const char *,int,const char *,
			char *,int) ;
static int	progok(struct proginfo *,const char *) ;


/* local variables */

#ifndef	COMMENT

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


static const char	*sched0[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n.%f",
	NULL
} ;

static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	NULL
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct ustat	sb ;

	VECSTR	sv ;

	KVSFILE	map ;

	vecstr	defs ;

	bfile	errfile ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	len, c, i, j, k ;
	int	sl, cl, ml ;
	int	pnlen, pfnlen ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_defsfile = FALSE ;
	int	f ;

	char	buf[BUFLEN + 1], *bp ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	defsfname[MAXPATHLEN + 1] ;
	char	progfname[MAXPATHLEN + 1] ;
	char	fname[MAXNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*progname = NULL ;
	const char	*tp, *sp, *cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFD1)) == NULL)
	    cp = getenv(VARDEBUGFD2) ;

	if (cp != NULL) {

	    if (cfdeci(cp,-1,&fd_debug) >= 0)
	        debugsetfd(fd_debug) ;

	    else
	        fd_debug = debugopen(cp) ;

	}
#endif /* CF_DEBUGS || CF_DEBUG */


	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;


	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {

	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	}

/* initialize */

	pip->f.quiet = FALSE ;


/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_rootexecname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

	}

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

#if	CF_DEBUGS
	debugprintf("main: pr=%s\n",pip->pr) ;
	debugprintf("main: searchname=%s\n",pip->searchname) ;
#endif

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getenv("TMPDIR") ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif /* COMMENT */

/* what is the name of this program */

	pnlen = sfbasename(argv[0],-1,&progname) ;

	if ((pnlen > 0) && (progname[pnlen - 1] != '\0')) {

	    ml = MIN(MAXNAMELEN,pnlen) ;
	    strwcpy(fname,progname,ml) ;

	    progname = fname ;
	}


/* get the device and inode of our own executable */

	rs1 = proginfo_getename(pip,tmpfname,MAXPATHLEN) ;

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	if (rs1 < 0) {

	    const char	*pp ;


	    pp = getexecname() ;

	    if (pp != NULL)
	        proginfo_rootexecname(pip,pp) ;

	}
#endif

	rs = proginfo_getename(pip,tmpfname,MAXPATHLEN) ;

	if (rs < 0)
	    goto badinit ;

	rs = u_stat(tmpfname,&sb) ;

	if (rs < 0)
	    goto badinit ;

	pip->dev = sb.st_dev ;
	pip->ino = sb.st_ino ;


	ids_load(&pip->id) ;


/* search for our "map" table */

	rs = vecstr_start(&sv,0,0) ;

	if (rs >= 0) {

	    vecstr_add(&sv,"p",pip->pr) ;

	    vecstr_add(&sv,"e","etc") ;

	    vecstr_add(&sv,"n",pip->searchname) ;

	    rs = permsched(sched0,&sv,
	        tmpfname,MAXPATHLEN, MAPFNAME,R_OK) ;

	    if (rs >= 0) {

	        rs1 = permsched(sched1,&sv,
			 defsfname,MAXPATHLEN, DEFSFNAME,R_OK) ;

	        f_defsfile = (rs1 >= 0) ;

	    }

	    vecstr_finish(&sv) ;
	} /* end block */

#if	CF_DEBUGS
	debugprintf("main: mapfname=%s\n",tmpfname) ;
	debugprintf("main: defsfname=%s\n",defsfname) ;
#endif

	if (rs > 0)
	    rs = kvsfile_open(&map,tmpfname,20) ;

#if	CF_DEBUGS
	debugprintf("main: kvsfile_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    KVSFILE_CURSOR	cur ;

	    VARSUB	subdefs ;
	    VARSUB	subenvs ;

	    vecstr	defs ;

	    int		opts = (VARSUB_MBADNOKEY | VARSUB_MBRACE) ;
	    int		f_defs = FALSE ;
	    int		f_subdefs = FALSE ;
	    int		f_subenvs = FALSE ;


	    rs = SR_NOENT ;
	    rs1 = varsub_init(&subdefs,opts) ;

	    if (rs1 >= 0) {

	        f_subdefs = TRUE ;
	        rs1 = vecstr_start(&defs,20,0) ;

	        f_defs = (rs1 >= 0) ;
	        if (f_defs && f_defsfile) {

	            procxenv(&defs,defsfname) ;

	            varsub_loadvec(&subdefs,&defs) ;

	        }

	    }

	    rs1 = varsub_init(&subenvs,opts) ;

	    f_subenvs = (rs1 >= 0) ;
	    if (f_subenvs)
	        varsub_loadenv(&subenvs,(const char **) envv) ;

	    if (f_subdefs || f_subenvs) {

/* search for a filepath */

	        kvsfile_curbegin(&map,&cur) ;

	        while (TRUE) {

	            rs = kvsfile_fetch(&map,progname,&cur,
	                tmpfname,MAXPATHLEN) ;

	            if (rs <= 0)
	                break ;

#if	CF_DEBUGS
	            debugprintf("main: kvsfile_fetch() rs=%d name=%s\n",
	                rs,tmpfname) ;
#endif

	            cl = rs ;
	            rs1 = SR_NOENT ;
	            if (f_subdefs)
	                rs1 = varsub_buf(&subdefs,tmpfname,cl,
	                    progfname,MAXPATHLEN) ;

#if	CF_DEBUGS
	            debugprintf("main: varsub_buf() rs=%d\n",rs1) ;
	            debugprintf("main: progfname=%s\n",progfname) ;
#endif

	            if ((rs1 < 0) && f_subenvs) {

	                rs1 = varsub_buf(&subenvs,tmpfname,cl,
	                    progfname,MAXPATHLEN) ;

#if	CF_DEBUGS
	                debugprintf("main: varsub_buf() rs=%d\n",rs1) ;
	                debugprintf("main: progfname=%s\n",progfname) ;
#endif

	            }

	            if (rs1 >= 0) {

#if	CF_DEBUGS
	                debugprintf("main: try prog filepath=%w\n",progfname,rs1) ;
#endif

	                rs = progok(pip,progfname) ;

	                if (rs >= 0)
	                    break ;

	            }

	        } /* end while */

	        kvsfile_curend(&map,&cur) ;

	    } /* end if (initialized subs) */

	    if (f_subenvs)
	        varsub_free(&subenvs) ;

	    if (f_defs)
	        vecstr_finish(&defs) ;

	    if (f_subdefs)
	        varsub_free(&subdefs) ;

	    kvsfile_close(&map) ;

	} /* end if (searched map table for entry) */

#if	CF_DEBUGS
	debugprintf("main: mid rs=%d\n",rs) ;
#endif

/* if we don't have a program file yet, search the execution PATH */

	if ((rs < 0) && ((sp = getenv("PATH")) != NULL)) {

#if	CF_DEBUGS
	    debugprintf("main: searching execution PATH\n") ;
#endif

	    while ((tp = strchr(sp,':')) != NULL) {

	        cp = sp ;
	        cl = (tp - sp) ;
	        rs = progmk(pip,cp,cl,progname,progfname,MAXPATHLEN) ;

	        if (rs >= 0)
	            rs = progok(pip,progfname) ;

	        if (rs >= 0)
	            break ;

	        sp = tp + 1 ;

	    } /* end while */

	    if ((rs < 0) && (sp[0] != '\0')) {

	        rs = progmk(pip,sp,-1,progname,progfname,MAXPATHLEN) ;

	        if (rs >= 0)
	            rs = progok(pip,progfname) ;

	    }

	} /* end if */


	ids_release(&pip->id) ;


	if (rs < 0)
	    goto badnoprog ;

#if	CF_DEBUGS
	debugprintf("main: progfname=%s\n",progfname) ;
#endif

	bclose(pip->efp) ;

	pip->efp = NULL ;

	rs = u_execve(progfname,argv,envv) ;

	ex = EX_NOEXEC ;

#ifdef	COMMENT
	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {

	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	}
#endif /* COMMENT */

/* early return thing */
retearly:
ret1:
	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* bad stuff */
badnoprog:
	ex = EX_NOPROG ;
	bprintf(pip->efp,"%s: could not find program=%s (%d)\n",
	    pip->progname,progname,rs) ;

	goto retearly ;

badinit:
	ex = EX_OSERR ;
	bprintf(pip->efp,"%s: could not initialize (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int progmk(pip,path,pathlen,progname,buf,buflen)
struct proginfo	*pip ;
const char	path[] ;
int		pathlen ;
const char	progname[] ;
char		buf[] ;
int		buflen ;
{
	SBUF	pbuf ;

	int	rs ;
	int	pl ;


	if (pathlen < 0)
	    pathlen = strlen(path) ;

	rs = sbuf_init(&pbuf,buf,buflen) ;

	if (rs < 0)
	    goto ret0 ;

	if (pathlen > 0) {

	    sbuf_strw(&pbuf,path,pathlen) ;

	    if (path[pathlen - 1] != '/')
	        sbuf_char(&pbuf,'/') ;

	}

	sbuf_strw(&pbuf,progname,-1) ;

	rs = sbuf_free(&pbuf) ;

	pl = rs ;

ret0:
	return (rs >= 0) ? pl : rs ;
}
/* end subroutine (progmk) */


static int progok(pip,progfname)
struct proginfo	*pip ;
const char	progfname[] ;
{
	struct ustat	sb ;

	int	rs ;


	rs = u_stat(progfname,&sb) ;

	if (rs >= 0)
	    rs = sperm(&pip->id,&sb,X_OK) ;

	if (rs >= 0) {

	    if ((sb.st_dev == pip->dev) &&
	        (sb.st_ino == pip->ino))
	        rs = SR_INVALID ;

	}

	return rs ;
}
/* end subroutine (progok) */



