/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_GETEXECNAME	1		/* user 'getexecname()' */
#define	CF_DEFPATH	1		/* export a default PATH */
#define	CF_LOGNAME	1		/* give our LOGNAME to daemons */
#define	CF_ACCTAB	1		/* access table */
#define	CF_SETRUID	1		/* set real UID to EUID */
#define	CF_CHECKONC	1		/* check ONC */


/* revision history:

	= 94/09/01, David A­D­ Morano

	This subroutine was borrowed and modified from previous generic
	front-end 'main' subroutines !

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms the front-end part of a generic PCS
	type of program.  This front-end is used in a variety of
	PCS programs.

	This subroutine was originally part of the Personal
	Communications Services (PCS) package but can also be used
	independently from it.  Historically, this was developed as
	part of an effort to maintain high function (and reliable)
	email communications in the face of increasingly draconian
	security restrictions imposed on the computers in the DEFINITY
	development organization.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/mkdev.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<varsub.h>
#include	<storebuf.h>
#include	<mallocstuff.h>
#include	<pcsconf.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<userinfo.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"srvtab.h"
#include	"acctab.h"
#include	"paramfile.h"

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	(NUMNAMES * MAXPATHLEN)
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	GETFNAME_TYPELOCAL
#define	GETFNAME_TYPELOCAL	0	/* search locally first */
#define	GETFNAME_TYPEROOT	1	/* search programroot area first */
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)		(pip->debuglevel >= (n))
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getserial(const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;

extern int	varsub_addva(), varsub_subbuf(), varsub_merge() ;
extern int	expander(struct proginfo *,char *,int,char *,int) ;
extern int	procfileenv(char *,char *,vecstr *) ;
extern int	procfilepath(char *,char *,vecstr *) ;

extern int	process(struct proginfo *,vecstr *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_logz(time_t,char *) ;

#if	CF_DEBUG
extern void	d_whoopen(char *) ;
#endif

#ifdef	DMALLOC
extern void	pcspoll_checker() ;
#endif


/* external variables */


/* local structures */


/* forward references */

static int	procfile(struct proginfo *,int (*)(char *,char *,vecstr *),
			const char *,vecstr *,char *,vecstr *) ;
static int	getsets(struct proginfo *,vecstr *,struct configinfo *) ;
static int	matme(char *,char *,char **,char **) ;
static int	checkstamp(struct proginfo *,char *,int) ;


/* local variables */

static const char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	"CONFIG",
	"HELP",
	NULL
} ;

enum argopts {
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_logfile,
	argopt_config,
	argopt_help,
	argopt_overlast
} ;

static const char	*configkeys[] = {
	"timestamp",
	"mincheck",
	"mbtab",
	"maildir",
	NULL
} ;

enum configkeys {
	configkey_timestamp,
	configkey_mincheck,
	configkey_mbtab,
	configkey_maildir,
	configkey_overlast
} ;

/* 'conf' for most regular programs */
static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

/* non-'conf' ETC stuff for all regular programs */
static const char	*sched2[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	"%p/%n.%f",
	NULL
} ;

/* non-'conf' ETC stuff for local searching */
static const char	*sched3[] = {
	"%e/%n/%n.%f",
	"%e/%n/%f",
	"%e/%n.%f",
	"%e/%f",
	"%n.%f",
	"%f",
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[], *envv[] ;
{
	struct ustat		sb ;

	struct proginfo		pi, *pip = &pi ;

	struct configfile	cf ;

	struct group	ge ;

	PCSCONF		pc ;

	USERINFO	u ;

	bfile		errfile ;
	bfile		logfile ;
	bfile		pidfile ;

	vecstr		defines, unsets ;
	VECSTR		svars ;

	varsub		vsh_e, vsh_d ;

	SRVTAB_ENT	*srvp ;

	time_t	daytime = time(NULL) ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	ex = EX_INFO ;
	int	rs, rs1, len ;
	int	mincheck = -2 ;
	int	loglen = -1 ;
	int	sl, sl2 ;
	int	fd_debug ;
	int	logfile_type = -1 ;
	int	mbtab_type = -1 ;
	int	srvtab_type = -1 ;
	int	acctab_type = -1 ;
	int	path_type = -1 ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_programroot = FALSE ;
	int	f_freeconfigfname = FALSE ;
	int	f_procfileenv = FALSE ;
	int	f_changedroot = FALSE ;
	int	f_checksecure ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 2] ;
	char	buf2[BUFLEN + 2] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 2] ;
	char	srvfname[MAXPATHLEN + 2] ;
	char	accfname[MAXPATHLEN + 2] ;
	char	mbfname[MAXPATHLEN + 2] ;
	char	pidfname[MAXPATHLEN + 2] ;
	char	lockfname[MAXPATHLEN + 2] ;
	char	logfname[MAXPATHLEN + 2] ;
	char	stampfname[MAXPATHLEN + 2] ;
	char	pathfname[MAXPATHLEN + 2] ;
	char	stampdname[MAXPATHLEN + 2] ;
	char	maildname[MAXPATHLEN + 2] ;
	char	pcsconfbuf[PCSCONF_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*pr = NULL ;
	char	*configfname = NULL ;
	char	*cp ;


	if ((cp = getenv(VARDEBUGFD1)) == NULL)
		cp = getenv(VARDEBUGFD2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


/* we want to open up some files so that the first few FD slots are FULL !! */

	if (u_fstat(FD_STDIN,&sb) < 0)
	    (void) u_open("/dev/null",O_RDONLY,0666) ;

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	pip->f.fd_stdout = TRUE ;

	if (u_fstat(FD_STDOUT,&sb) < 0) {
	    pip->f.fd_stdout = FALSE ;
	    (void) u_dup(FD_STDIN) ;
	}

	pip->f.fd_stderr = FALSE ;
	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->f.fd_stderr = TRUE ;
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	} else
	    (void) u_open("/dev/null",O_WRONLY,0666) ;

	pip->lfp = &logfile ;

	pip->pid = getpid() ;

	pip->ppid = pip->pid ;

/* initialize */

	pip->username = NULL ;
	pip->groupname = NULL ;
	pip->pidfname = NULL ;
	pip->lockfname = NULL ;
	pip->tmpdname = NULL ;
	pip->workdname = NULL ;
	pip->stampdname = NULL ;
	pip->defuser = NULL ;
	pip->defgroup = NULL ;
	pip->defacc = NULL ;
	pip->srvtab = NULL ;
	pip->acctab = NULL ;

	pip->prog_rmail = NULL ;
	pip->prog_sendmail = NULL ;

	pip->debuglevel = 0 ;
	pip->interval = -1 ;	/* program check interval */
	pip->runtime = 0 ;	/* regular mode requires '0' here */
	pip->maxjobs = MAXJOBS ;

	pip->f.quiet = FALSE ;
	pip->f.log = FALSE ;
	pip->f.slog = FALSE ;
	pip->f.daemon = FALSE ;
	pip->f.named = FALSE ;
	pip->f.srvtab = FALSE ;
	pip->f.acctab = FALSE ;
	pip->f.defacc = FALSE ;


	pathfname[0] = '\0' ;
	srvfname[0] = '\0' ;
	accfname[0] = '\0' ;
	pidfname[0] = '\0' ;
	lockfname[0] = '\0' ;
	logfname[0] = '\0' ;
	stampfname[0] = '\0' ;
	stampdname[0] = '\0' ;


/* start parsing the arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                if (((argl - 1) > 0) && 
	                    (cfdecti(argp + 1,argl - 1,&pip->interval) < 0))
	                    goto badarg ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	                debugprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

	                    case argopt_tmpdir:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->tmpdname = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->tmpdname = argp ;

	                        }

	                        break ;

/* version */
	                    case argopt_version:
	                        f_version = TRUE ;
	                        if (f_optequal) 
					goto badargextra ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {
 
	                                    rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

					if (rs < 0)
	                                    goto badargval ;

					}
	                            }

	                        break ;

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pr = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pr = argp ;

	                        }

	                        break ;

/* configuration file */
	                    case argopt_config:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					configfname = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					configfname = argp ;

	                        }

	                        break ;

/* help */
	                    case argopt_help:
	                        f_help = TRUE ;
				break ;

/* log file */
	                    case argopt_logfile:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                logfile_type = 0 ;
	                                strwcpy(logfname,avp,avl) ;

	                            }

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)  {

	                                logfile_type = 0 ;
	                                strwcpy(logfname,argp,argl) ;

	                            }
	                        }

	                        break ;

/* handle all keyword defaults */
	                    default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                        bprintf(pip->efp,
					"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("main: option key letters\n") ;
#endif

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {
 
	                                    rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

					if (rs < 0)
	                                    goto badargval ;

					}
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* configuration file */
	                        case 'C':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                configfname = argp ;

	                            break ;

/* mutex lock PID file */
	                        case 'P':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                strwcpy(pidfname,argp,argl) ;

	                            break ;

/* daemon mode */
	                        case 'd':
	                            pip->f.daemon = TRUE ;
	                            pip->runtime = -1 ;
	                            if (f_optequal) {

#if	CF_DEBUGS
	                                debugprintf("main: debug, avp=\"%W\"\n",
	                                    avp,avl) ;
#endif

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdecti(avp,avl,
	                                    	&pip->runtime) ;

					if (rs < 0)
	                                    goto badargval ;

					}
	                            }

	                            break ;

/* force a run even if our minimum time between executions is not expired */
	                        case 'f':
	                            mincheck = 0 ;
	                            break ;

/* minimum check time */
	                        case 'm':
	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdecti(argp,argl,&mincheck) ;

					if (rs < 0)
	                                    goto badargval ;

	                            }

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
					pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {
 
	                                    rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

					if (rs < 0)
	                                    goto badargval ;

					}
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
				break ;

	                        default:
	                            ex = EX_USAGE ;
	                            f_usage = TRUE ;
	                            bprintf(pip->efp,
					"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	                pip->f.named = TRUE ;

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	            pip->f.named = TRUE ;

	        } else {

	            if (! f_extra) {

			ex = EX_USAGE ;
	                f_extra = TRUE ;
	                bprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: finished parsing arguments\n",
	        pip->progname) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;


/* miscellaneous */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf( "main: starting FDs\n") ;

	    d_whoopen("1") ;

	}
#endif


/* set some stupid UNIX global variables */

	tzset() ;


/* get our program root */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	debugprintf("main: before pr=%s\n",
		pr) ;
#endif

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

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


/* help */

	if (f_help)
	    goto help ;


/* before we even try to find out our username, try to get our ONC key */

#if	CF_CHECKONC
	rs = checkonc(pip->pr,NULL,NULL,NULL) ;
	pip->f.onckey = (rs >= 0) ;
#endif

/* get some host/user information (offensive to ONC secure operations !) */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: UI name=%s\n",u.name) ;
#endif

	if (rs < 0) {

	    getnodedomain(nodename,domainname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: got node/domain\n") ;
#endif

	    pip->nodename = nodename ;
	    pip->domainname = domainname ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to get username\n") ;
#endif

	    getusername(buf,USERNAMELEN,-1) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: got username\n") ;
#endif

	    pip->username = mallocstr(buf) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ name stuff\n") ;
#endif

	} /* end if (got some user information or not) */


/* handle UID/GID stuff */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: continuing with some PID stuff\n") ;
#endif

	pip->uid = u.uid ;
	pip->euid = u_geteuid() ;

	pip->gid = u.gid ;
	pip->egid = u_getegid() ;


	rs = getgr_gid(&ge,buf,BUFLEN,pip->gid) ;

	if (rs < 0) {

	    cp = buf ;
	    bufprintf(buf,BUFLEN,"GID%d",(int) pip->gid) ;

	} else
	    cp = ge.gr_name ;

	pip->groupname = mallocstr(cp) ;


/* root secure ? */

	rs = checksecure(pip->pr,pip->euid) ;

	pip->f.secure_root = (rs > 0) ;


/* make the hostname */

	sl = snsds(buf,BUFLEN,
		pip->nodename,pip->domainname) ;

	if (sl > 0)
		pip->hostname = mallocstrw(buf,sl) ;


/* prepare to store configuration variable lists */

	vecstr_start(&pip->path,10,VECSTR_PNOHOLES) ;

	vecstr_start(&pip->exports,10,VECSTR_PNOHOLES) ;

	vecstr_start(&defines,10,VECSTR_PORDERED) ;

	vecstr_start(&unsets,10,VECSTR_PNOHOLES) ;


/* create the values for the file schedule searching */

	vecstr_start(&svars,6,0) ;

	vecstr_envset(&svars,"p",pip->pr) ;

	vecstr_envset(&svars,"e","etc") ;

	vecstr_envset(&svars,"n",pip->searchname) ;

/* load up some initial environment that everyone should have ! */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: about to do DEFINITFNAME=%s\n",DEFINITFNAME) ;
#endif

	procfileenv(pip->pr,DEFINITFNAME,&pip->exports) ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("main: checking for configuration file\n") ;
	    debugprintf("main: 0 CF=%s\n",configfname) ;
	}
#endif /* CF_DEBUG */

	f_checksecure = FALSE ;
	rs = SR_NOEXIST ;
	if ((configfname == NULL) || (configfname[0] == '\0')) {

	    configfname = CONFFNAME ;

	    pip->f.secure_conf = pip->f.secure_root ;
	    sl = permsched(sched1,&svars,
	        tmpfname,MAXPATHLEN, configfname,R_OK) ;

#ifdef	COMMENT
	    if (sl < 0) {

		pip->f.secure_conf = FALSE ;
		f_checksecure = TRUE ;
	        sl = permsched(sched3,&svars,
	        tmpfname,MAXPATHLEN, configfname,R_OK) ;

	    }
#endif /* COMMENT */

	    if (sl > 0)
	        configfname = tmpfname ;

	    rs = sl ;

	} else {

	    pip->f.secure_conf = FALSE ;
	    f_checksecure = TRUE ;
	    sl = getfname(pip->pr,configfname,1,tmpfname) ;

	    if (sl > 0)
	        configfname = tmpfname ;

	    rs = sl ;		/* cause an open failure later */

	} /* end if */

	if (configfname == tmpfname) {

	    f_freeconfigfname = TRUE ;
	    configfname = mallocstr(tmpfname) ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf("main: find rs=%d\n",sl) ;

	    debugprintf("main: 1 CF=%s\n",configfname) ;

	}
#endif /* CF_DEBUG */


/* read in the configuration file if we have one */

	if ((rs >= 0) || (perm(configfname,-1,-1,NULL,R_OK) >= 0)) {

		int	rs1 ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: configuration file \"%s\"\n",
	            configfname) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: configfile=%s\n",
	            pip->progname,configfname) ;

	    rs = configfile_start(&cf,configfname) ;

	    if (rs < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("main: we have a good configuration file\n") ;
	        debugprintf("main: initial pr=%s\n",
	            pip->pr) ;
	    }
#endif

		if (f_checksecure) {

			rs = checksecure(configfname,pip->euid) ;

			pip->f.secure_conf = (rs > 0) ;

		}


/* we must set this mode to 'VARSUB_MBADNOKEY' so that a miss is noticed */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_start d\n") ;
#endif

	    varsub_start(&vsh_d,VARSUB_MBADNOKEY) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_start e\n") ;
#endif

	    varsub_start(&vsh_e,0) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_addva\n") ;
#endif

	    varsub_addva(&vsh_e,envv) ;

#if	CF_DEBUG && F_VARSUBDUMP
	    if (pip->debuglevel > 1) {

	        debugprintf("main: 0 for\n") ;

	        varsub_dumpfd(&vsh_e,-1) ;

	    }
#endif /* CF_DEBUG */


/* program root from configuration file */

	    if ((cf.root != NULL) && (! f_programroot)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF programroot\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

		    f_changedroot = TRUE ;
	            proginfo_setprogroot(pip,buf2,sl2) ;

/* set it so we have something for the present cookies ! */

			vecstr_envset(&svars,"p",pip->pr) ;

	        }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("main: config_file pr=%s\n",
	            pip->pr) ;
#endif

	    } /* end if (configuration file program root) */


/* loop through the DEFINEd variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: 0 top, cp=%s\n",cp) ;
#endif

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 0 varsub_subbuf() sl=%d\n",sl) ;
#endif

	        if ((sl > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1) {
				int	lenr = sl2 ;
				int	lenm ;
				char	*cp2 = buf2 ;

			while (lenr > 0) {
				lenm = MIN(lenr,60) ;
	                debugprintf("main: 0 expand> %W\n",cp2,lenm) ;
				cp2 += lenm ;
				lenr -= lenm ;
			}
			}
#endif /* CF_DEBUG */

	            rs1 = varsub_merge(&vsh_d,&defines,buf2,sl2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 0 varsub_merge() rs1=%d\n",rs1) ;
#endif

	        } /* end if */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf( "main: 0 bot, sl=%d sl2=%d\n",sl,sl2) ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ defines\n") ;
#endif


/* all of the rest of the configuration file stuff */

	    if ((cf.workdir != NULL) && (pip->workdname == NULL)) {

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.workdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            pip->workdname = mallocstrw(buf2,sl2) ;

	        }

	    } /* end if (config file working directory) */


#if	CF_DEBUG
	        if (pip->debuglevel > 1) {
	            debugprintf("main: 0 pidfname=%s\n",pidfname) ;
	            debugprintf("main: CF pidfname=%s\n",cf.pidfname) ;
		}
#endif

	    if ((cf.pidfname != NULL) && (pidfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF 0 pidfile=%s\n",cf.pidfname) ;
#endif

	        if (cf.pidfname[0] != '\0') {

		if (cf.pidfname[0] != '-') {

	            sl = varsub_subbuf(&vsh_d,&vsh_e,cf.pidfname,
	                -1,buf,BUFLEN) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: CF 2 pidfile=%W\n",buf,sl) ;
#endif

	            if ((sl > 0) &&
	                ((sl2 = expander(pip,buf,sl, buf2,BUFLEN)) > 0)) {

	                strwcpy(pidfname,buf2,sl2) ;

	            }

		} else
	                strcpy(pidfname,"-") ;

	        } /* end if (CF pidfname) */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF 2 pidfname=%s\n",pidfname) ;
#endif

	    } /* end if (configuration file PIDFNAME) */


	    if ((cf.lockfname != NULL) && (lockfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF lockfile=%s\n",cf.lockfname) ;
#endif

	        if ((cf.lockfname[0] != '\0') && (cf.lockfname[0] != '-')) {

	            if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.lockfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((sl2 = expander(pip,buf,sl, buf2,BUFLEN)) > 0)) {

	                strwcpy(lockfname,buf2,sl2) ;

	            }

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: lockfile=%s\n",lockfname) ;
#endif

	    } /* end if (configuration file LOCKFNAME) */


#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        debugprintf("main: so far logfname=%s\n",logfname) ;

	        debugprintf("main: about to get config log filename\n") ;
	    }
#endif

	    if ((cf.logfname != NULL) && (logfile_type < 0)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(logfname,buf2,sl2) ;

	            if (strchr(logfname,'/') != NULL)
	                logfile_type = 1 ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",logfname) ;
#endif

	    } /* end if (configuration file log filename) */

	    if ((cf.interval != NULL) && (pip->interval < 0)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF interval\n") ;
#endif

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cf.interval,
	            -1,buf,BUFLEN) ;

		if ((sl > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            (void) cfdecti(buf2,sl2,&pip->interval) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: CF interval=%d\n",pip->interval) ;
#endif

	        }

	    } /* end if (interval) */

	    if (cf.maxjobs != NULL) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF maxjobs\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.maxjobs,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            (void) cfdeci(buf2,sl2,&pip->maxjobs) ;

	        }

	    } /* end if (maxjobs) */

	    if ((cf.user != NULL) && (pip->defuser == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF user \n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.user,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            pip->defuser = mallocstrw(buf2,sl2) ;

	        }

	    }

	    if ((cf.group != NULL) && (pip->defgroup == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF group \n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.group,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            pip->defgroup = mallocstrw(buf2,sl2) ;

	        }

	    } /* end if */

	    if ((cf.stampdir != NULL) && (stampdname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF processing STAMPDIR\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.stampdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(stampdname,buf2,sl2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: CF opt STAMPDIR=%s\n",
	                    stampdname) ;
#endif

	        }

	    } /* end if (stampdir) */

	    if ((cf.srvtab != NULL) && (srvfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF processing SRVTAB \n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.srvtab,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(srvfname,buf2,sl2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: CF SRVTAB s&e srvtab=%s\n",
	                    srvfname) ;
#endif

	            srvtab_type = GETFNAME_TYPELOCAL ;
	            if (strchr(srvfname,'/') != NULL)
	                srvtab_type = GETFNAME_TYPEROOT ;

			pip->f.secure_srvtab = pip->f.secure_conf ;

	        }

	    } /* end if (srvtab) */

#if	CF_ACCTAB
	    if ((cf.acctab != NULL) && (accfname[0] == '\0')) {

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.acctab,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(accfname,buf2,sl2) ;

	            acctab_type = GETFNAME_TYPELOCAL ;
	            if (strchr(accfname,'/') != NULL)
	                acctab_type = GETFNAME_TYPEROOT ;

			pip->f.secure_acctab = pip->f.secure_conf ;

	        }

	    } /* end if (acctab) */
#endif /* CF_ACCTAB */

	    if ((cf.sendmail != NULL) && (pip->prog_sendmail == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF sendmail\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.sendmail,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            pip->prog_sendmail = mallocstrw(buf2,sl2) ;

	        }

	    } /* end if (sendmail) */

/* what about an 'env' file ? */

	    if (cf.envfname != NULL) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: 1 envfile=%s\n",cf.envfname) ;
#endif

	        procfileenv(pip->pr,cf.envfname,&pip->exports) ;

	    } else
	        procfile(pip,procfileenv,pip->pr,&svars,
	            XENVFNAME,&pip->exports) ;

	    f_procfileenv = TRUE ;


/* "do" any 'paths' file before we process the environment variables */

	    if ((cf.pathfname != NULL) && (cf.pathfname[0] != '\0') &&
			(pathfname[0] == '\0')) {

	        if (((sl = varsub_subbuf(&vsh_d,&vsh_e,cf.pathfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

			strwcpy(pathfname,buf2,sl2) ;

		}
	}

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

/* loop through the UNSETs in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.unsets,i,&cp) >= 0 ; i += 1) {
	        if (cp == NULL) continue ;

	        vecstr_add(&unsets,cp,-1) ;

	        rs = vecstr_finder(&pip->exports,cp,vstrkeycmp,NULL) ;

	        if (rs >= 0)
	            vecstr_del(&pip->exports,rs) ;

	    } /* end for */

/* loop through the EXPORTed variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.exports,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: 1 about to sub> %s\n",cp) ;
#endif

	        sl = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN) ;

	        if ((sl > 0) &&
	            ((sl2 = expander(pip,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1) {
				int	lenr = sl2 ;
				int	lenm ;
				char	*cp2 = buf2 ;

			while (lenr > 0) {
				lenm = MIN(lenr,60) ;
	                debugprintf("main: 0 expand> %W\n",cp2,lenm) ;
				cp2 += lenm ;
				lenr -= lenm ;
			}
			}
#endif /* CF_DEBUG */

	            rs1 = varsub_merge(NULL,&pip->exports,buf2,sl2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 1 varsub_merge() rs1=%d\n",rs1) ;
#endif

#if	CF_DEBUG && F_VARSUBDUMP && 0
	            if (pip->debuglevel > 1) {

	                debugprintf("varsub_merge: VSA_D so far \n") ;

	                varsub_dumpfd(&vsh_d,-1) ;

	                debugprintf("varsub_merge: VSA_E so far \n") ;

	                varsub_dump(&vsh_e,-1) ;

	            } /* end if (debuglevel) */
#endif /* CF_DEBUG */

	        } /* end if (merging) */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: done subbing & merging\n") ;
#endif

	    } /* end for (exports) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ exports\n") ;
#endif


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF freeing data structures\n") ;
#endif


	    varsub_finish(&vsh_d) ;

	    varsub_finish(&vsh_e) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF free\n") ;
#endif

	    configfile_finish(&cf) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("main: dumping defines\n") ;
	        for (i = 0 ; vecstr_get(&defines,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: define> %W\n",cp,strnlen(cp,60)) ;
	        debugprintf("main: dumping exports\n") ;
	        for (i = 0 ; vecstr_get(&pip->exports,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: export> %W\n",cp,strnlen(cp,60)) ;
	    } /* end if (CF_DEBUG) */
#endif /* CF_DEBUG */

	} else
		pip->f.secure_conf = TRUE ;

/* end of accessing the configuration file */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		debugprintf("main: finished with any configfile stuff\n") ;
#endif

	if (f_changedroot)
		vecstr_envset(&svars,"p",pip->pr) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

/* before we go too far, are we the only one on this PID mutex ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 pidfname=%s\n",pidfname) ;
#endif

	if ((! pip->f.named) && (pidfname[0] != '\0')) {

	    if (pidfname[0] == '-')
	        strcpy(pidfname,PIDFNAME) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 pidfname=%s\n",pidfname) ;
#endif

	    pip->pidfname = pidfname ;
	    rs = bopenroot(&pidfile,pip->pr,pidfname,tmpfname,
	        "rwc",0664) ;

	    if (rs < 0)
	        goto badpidopen ;

		if (rs == SR_CREATED)
			bcontrol(&pidfile,BC_CHMOD,0666) ;

	    if (tmpfname[0] != '\0')
	        strcpy(pidfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 3 pidfname=%s\n",pidfname) ;
#endif

/* capture the lock (if we can) */

	    if ((rs = bcontrol(&pidfile,BC_LOCK,0)) < 0)
	        goto badpidlock ;

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main: PID mutex file, writing pid=%d\n",
		(int) pip->pid) ;
#endif

	    bprintf(&pidfile,"%d\n",(int) pip->pid) ;

	    bprintf(&pidfile,"%s!%s\n",pip->nodename,pip->username) ;

	    bclose(&pidfile) ;		/* this releases the lock */

	} /* end if (we have a mutex PID file) */


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 0 mincheck=%d\n",mincheck) ;
#endif

/* before proceeding too much further, what does PCSCONF say ? */

	{
	    struct configinfo	ci ;

	    vecstr	sets ;


	    vecstr_start(&sets,10,0) ;

	    cp = pip->pr ;
	    rs = pcsconf(cp,NULL,&pc,&sets,NULL,pcsconfbuf,PCSCONF_LEN) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: pcsconf() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        if (getsets(pip,&sets,&ci) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: sets.mincheck=%d\n",ci.mincheck) ;
#endif

	            if ((mincheck < 0) && (ci.mincheck >= -1)) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: setting mincheck\n") ;
#endif

	                mincheck = ci.mincheck ;
			}

	            if (ci.stampfname[0] != '\0')
	                strcpy(stampfname,ci.stampfname) ;

		    if (ci.mbfname[0] != '\0')
	                strcpy(mbfname,ci.mbfname) ;

			if ((ci.maildname[0] != '\0') && 
				(pip->maildname[0] == '\0'))
			strcpy(pip->maildname,ci.maildname) ;

	        } /* end if (got PCS configuration settings) */

	    } /* end if (got a PCSCONF) */

	    vecstr_finish(&sets) ;

	} /* end block (PCS configuration) */


/* can we get out right now ? */

	if ((pip->debuglevel > 0) && (! pip->f.daemon)) {

		if (mincheck < -1)
			cp = "default" ;

		else if (mincheck == -1)
			cp = "disabled" ;

		else if (mincheck == 0)
			cp = "forced" ;

		if (mincheck > 0)
			bprintf(pip->efp,"%s: mincheck=%d\n",
				pip->progname,mincheck) ;

		else
			bprintf(pip->efp,"%s: mincheck=%s\n",
				pip->progname,cp) ;

	} /* end if */

	if (mincheck == -1)
	        goto mininterval ;


/* first, do we have to make the timestamp directory ? */

	if (stampdname[0] == '\0')
	    strcpy(stampdname,STAMPDNAME) ;

	if (stampdname[0] != '/') {

	    rs = mkpath2(tmpfname, pip->pr,stampdname) ;

	    if (rs > 0)
	    	strcpy(stampdname,tmpfname) ;

	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: stampdir=%s\n",
	        pip->progname,stampdname) ;

	pip->stampdname = stampdname ;
	rs = perm(pip->stampdname,-1,-1,NULL,(R_OK | X_OK)) ;

	if (rs < 0) {

	    mode_t	oldmask ;


	    oldmask = umask(0000) ;

	    rs = mkdirs(pip->stampdname,0777) ;

	    umask(oldmask) ;

	} /* end if (making STAMPDNAME) */


/* what is our own stamp file name ? */

	if (stampfname[0] == '\0')
	    strcpy(stampfname,STAMPFNAME) ;

	if (strchr(stampfname,'/') == NULL) {

	    sl = mkpath2(tmpfname,
	        stampdname,stampfname) ;

	    if (sl > 0)
	        strcpy(stampfname,tmpfname) ;

	}

	if (stampfname[0] != '/') {

	    sl = mkpath2(tmpfname,
	        pip->pr,stampfname) ;

	    if (sl > 0)
	        strcpy(stampfname,tmpfname) ;

	} /* end if (non-rooted stamp file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: stampfname=%s\n",stampfname) ;
#endif


/* now check if we have been called within the minimun check period */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 mincheck=%d\n",mincheck) ;
#endif

	if (mincheck < -1)
	    mincheck = TI_MINCHECK ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 2 mincheck=%d\n",mincheck) ;
#endif

	if ((! pip->f.daemon) && (! pip->f.named) && (mincheck > 0)) {

	    if (checkstamp(pip,stampfname,mincheck) <= 0)
	        goto mininterval ;

	} /* end if (not running with named service) */


/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking program parameters\n") ;
#endif

	if (pip->f.named)
	    pip->f.daemon = FALSE ;


/* temporary directory */

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    if ((pip->tmpdname = getenv("TMPDIR")) == NULL)
	        pip->tmpdname = TMPDNAME ;

	} /* end if (tmpdname) */


/* can we access the working directory ? */

	if (pip->workdname == NULL)
	    pip->workdname = WORKDNAME ;

	else if (pip->workdname[0] == '\0')
	    pip->workdname = pip->tmpdname ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: access working directory=%s\n",pip->workdname) ;
#endif

	rs1 = perm(pip->workdname,-1,-1,NULL,(X_OK | R_OK)) ;

	if (rs1 < 0)
	    goto badworking ;


/* check the mailspool directory */

	if (pip->maildname[0] == '\0')
		strcpy(pip->maildname,MAILDNAME) ;


/* maximum number of jobs */

	if (pip->maxjobs <= 0)
	    pip->maxjobs = MAXJOBS ;


/* service poll check interval */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 interval=%d\n",pip->interval) ;
#endif

	if (pip->interval <= 0)
	    pip->interval = TI_POLLSVC ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 2 interval=%d\n",pip->interval) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: poll interval=%d\n",
		pip->progname,pip->interval) ;


/* other defaults */

	if (lockfname[0] == '\0')
	    strcpy(lockfname,LOCKFNAME) ;

	if ((sl = getfname(pip->pr,lockfname,1,tmpfname)) > 0)
	    strwcpy(lockfname,tmpfname,sl) ;

	pip->lockfname = lockfname ;


	if (pip->prog_sendmail == NULL)
	    pip->prog_sendmail = mallocstr(PROG_SENDMAIL) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: sendmail=%s\n",
	        pip->prog_sendmail) ;
#endif


/* open the system report log file */

#ifdef	COMMENT
	if ((rs = bopen(pip->lfp,logfname,"wca",0664)) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: system log rs=%d\n",rs) ;
#endif

	    bprintf(lfp,"%s: %s %s started\n",
	        pip->progname,timestr_lodaytime,timebuf),
	        BANNER) ;

	    bflush(pip->lfp) ;

	} /* end if (opening log file) */
#endif /* COMMENT */


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

/* get a serial number for logging purposes */

	cp = SERIALFNAME ;
	if (cp[0] != '/') {

		rs = mkpath2(buf, pip->pr,cp) ;

		if (rs > 0)
			cp = buf ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: serialfile=%s\n",cp) ;
#endif

	rs = getserial(cp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: getserial() rs=%d\n",rs) ;
#endif

	if (rs < 0)
		rs = pip->pid ;

	pip->serial = rs ;
	(void) storebuf_dec(buf,BUFLEN,0,pip->serial) ;

	pip->logid = mallocstr(buf) ;

	rs = SR_BAD ;
	if (logfname[0] == '\0') {

	    logfile_type = 1 ;
	    strcpy(logfname,LOGFNAME) ;

	}

	sl = getfname(pip->pr,logfname,logfile_type,tmpfname) ;

	if (sl > 0)
	    strwcpy(logfname,tmpfname,sl) ;

	rs = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

	    pip->f.log = TRUE ;
	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: logfile=%s\n",pip->progname,logfname) ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    if (pip->f.daemon || pip->f.named) {

	        logfile_printf(&pip->lh,"%s %s %s\n",
	            timestr_logz(daytime,timebuf),
	            BANNER,
	            "started") ;

	        logfile_printf(&pip->lh,"%-14s %s/%s\n",
	            pip->progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    } else
	        logfile_printf(&pip->lh,"%s %-14s %s/%s\n",
	            timestr_logz(daytime,timebuf),
	            pip->progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) domain=%s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
	        u.sysname,u.release,
	        pip->domainname) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        strcpy(buf,u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        strcpy(buf,u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        strcpy(buf,u.fullname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: LF name=%s\n",buf) ;
#endif

	    if (buf[0] != '\0')
	        logfile_printf(&pip->lh,"%s!%s (%s)\n",
	            u.nodename,u.username,buf) ;

	    else
	        logfile_printf(&pip->lh,"%s!%s\n",
	            u.nodename,u.username) ;

	} /* end if (we have a log file or not) */


	if (pip->f.daemon || pip->f.named)
	    logfile_printf(&pip->lh,"conf=%s\n",configfname) ;


	    logfile_printf(&pip->lh,
	        "pr=%s\n",pip->pr) ;


/* load up some environment if we have not already */

	if (! f_procfileenv)
	    procfile(pip,procfileenv,pip->pr,&svars,
	        XENVFNAME,&pip->exports) ;


/* load up an alternate execution search path if we didn't get one already */

#ifdef	COMMENT
	cp = (pathfname[0] != '\0') ? pathfname : PATHFNAME ;
	procfile(pip,procfilepath,pip->pr,&svars,
	            cp,&pip->path) ;

	if (vecstr_find(lp,"/bin") < 0)
		vecstr_add(&pip->path,"/bin",4) ;
#endif


/* write user's mail address (roughly as we have it) into the user list file */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	debugprintf("main: pcsuserfile() pr=%s\n",
		pip->pr) ;
#endif

	rs = pcsuserfile(pip->pr,USERFNAME,
			pip->nodename,pip->username,buf) ;

	if (rs < 0) {

	    logfile_printf(&pip->lh,
	        "could not access user list file (%d)\n",
	        rs) ;

	} else if (rs == 1)
	    logfile_printf(&pip->lh,
	        "created the user list file\n") ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: wrote user file, rs=%d\n",rs) ;
#endif


/* do some things if we are running in daemon mode */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: are we in daemon mode ?\n") ;
#endif

	if (pip->f.daemon) {

#if	CF_DEBUG && 0
	    if (pip->debuglevel > 1) {
	        debugprintf("main: daemon mode\n") ;
	    }
#endif /* CF_DEBUG */


/* background ourselves */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf( "main: become a daemon ?\n") ;
#endif

	    bflush(pip->efp) ;

	    if (pip->debuglevel == 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf( "main: becoming a daemon ?\n") ;
#endif

		if (pip->f.log)
		    logfile_flush(&pip->lh) ;

#ifdef	COMMENT
	        for (i = 0 ; i < 3 ; i += 1) {

	            u_close(i) ;

	            (void) u_open("/dev/null",O_RDONLY,0666) ;

	        } /* end for */
#endif /* COMMENT */

	        rs = uc_fork() ;

	        if (rs < 0)
			goto badfork ;

	        if (rs > 0) 
			uc_exit(EX_OK) ;

	        u_setsid() ;

	    } /* end if (backgrounding) */

#if	CF_DEBUG && 0
	    if (pip->debuglevel > 1) {

	        debugprintf("main: after daemon backgrounding\n") ;

	        d_whoopen("3") ;

	    }
#endif /* CF_DEBUG */

		pip->pid = getpid() ;

	    logfile_printf(&pip->lh,"backgrounded pid=%d\n",
		(int) pip->pid) ;

	} /* end if (daemon mode) */


/* we start ! */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: starting\n") ;
#endif

/* reload the userinfo structure with our new PID */

	if (userbuf[0] != '\0')
	    u.pid = pip->pid ;

#ifdef	COMMENT
	(void) storebuf_dec(buf,BUFLEN,0,(int) pip->pid) ;

	logfile_setid(&pip->lh,buf) ;
#endif /* COMMENT */


/* before we go too far, are we the only one on this PID mutex ? */

	if (! pip->f.named) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: have PID file ?\n") ;
#endif

	    if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {

		int	devmajor, devminor ;


#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: we have a PIDFNAME=%s\n",
			pip->pidfname) ;
#endif

	        rs1 = bopen(&pidfile,pip->pidfname,"rwc",0664) ;

		if (rs1 < 0)
	            goto badpidfile1 ;

/* capture the lock (if we can) */

	        if ((rs1 = bcontrol(&pidfile,BC_LOCK,0)) < 0) {

	            bclose(&pidfile) ;

	            goto badpidfile2 ;
	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: captured PID lock\n") ;
#endif

	        bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	        bseek(&pidfile,0L,SEEK_SET) ;

	        bprintf(&pidfile,"%d\n",(int) pip->pid) ;

	        bprintf(&pidfile,"%s!%s\n",pip->nodename,pip->username) ;

	        bprintf(&pidfile,"%s %s\n",
	            timestr_logz(daytime,timebuf),BANNER) ;

	        if (userbuf[0] != '\0')
	            bprintf(&pidfile,"host=%s.%s user=%s pid=%d\n",
	                u.nodename,u.domainname,
	                u.username,
	                (int) pip->pid) ;

	        else
	            bprintf(&pidfile,"host=%s.%s pid=%d\n",
	                u.nodename,u.domainname,
	                (int) pip->pid) ;

	        bflush(&pidfile) ;

/* we leave the file open as our mutex lock ! */

	        logfile_printf(&pip->lh,"pidfile=%s\n",pip->pidfname) ;

	        logfile_printf(&pip->lh,"PID mutex captured\n") ;

	        bcontrol(&pidfile,BC_STAT,&sb) ;

		devmajor = major(sb.st_dev) ;

		devminor = minor(sb.st_dev) ;

	        logfile_printf(&pip->lh,
		    "pidfile device=%u,%u inode=%lu\n",
	            devmajor,devminor,sb.st_ino) ;

	        pip->pidfp = &pidfile ;

	    } /* end if (we have a mutex PID file) */

	} /* end if (daemon mode) */


/* the mailbox table file */

	rs = SR_NOEXIST ;
	if (mbfname[0] == '\0') {

	    mbtab_type = GETFNAME_TYPEROOT ;
	    strcpy(mbfname,MBFNAME) ;

	    sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, mbfname,R_OK) ;

	    if (sl < 0)
	        sl = permsched(sched3,&svars,
	        tmpfname,MAXPATHLEN, mbfname,R_OK) ;

	    if (sl > 0)
	        strcpy(mbfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 0 mbfname=%s\n",mbfname) ;
#endif

	    rs = sl ;

	} else {

	    if (mbtab_type < 0)
	        mbtab_type = GETFNAME_TYPELOCAL ;

	    sl = getfname(pip->pr,mbfname,mbtab_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(mbfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 mbfname=%s\n",mbfname) ;
#endif

	if ((rs >= 0) || (perm(mbfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: mbtab=%s\n",pip->progname,mbfname) ;

	    if (pip->f.daemon || pip->f.named)
	        logfile_printf(&pip->lh,"mbtab=%s\n",mbfname) ;

	    rs = paramfile_open(&pip->mbtab,envv,mbfname) ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,"bad (%d) mailbox file\n",rs) ;

	        bprintf(pip->efp,"%s: mbtab=%s\n",
	            pip->progname,mbfname) ;

	        bprintf(pip->efp,"%s: bad (%d) mailbox file\n",
	            pip->progname,rs) ;

	    } else {

		pip->f.mbtab = TRUE ;
#ifdef	COMMENT
		pip->mbtab = mallocstr(mbfname) ;
#endif

		pip->f.secure_mbtab = pc.f.secure_root && pip->f.secure_conf ;
		if ((mbfname[0] != '/') || (! pip->f.secure_mbtab)) {

			rs = checksecure(mbfname,pip->euid) ;

			pip->f.secure_mbtab = (rs > 0) ;

		}

	    }

	} else
	    rs = SR_NOEXIST ;


/* find and open the server table file */

	rs = SR_NOEXIST ;
	if (srvfname[0] == '\0') {

	    srvtab_type = GETFNAME_TYPEROOT ;
	    strcpy(srvfname,SRVFNAME) ;

	    sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN,srvfname,R_OK) ;

	    if (sl < 0)
	        sl = permsched(sched3,&svars,
	        tmpfname,MAXPATHLEN,srvfname,R_OK) ;

	    if (sl > 0)
	        strcpy(srvfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 0 srvfname=%s\n",srvfname) ;
#endif

	    rs = sl ;

	} else {

	    if (srvtab_type < 0)
	        srvtab_type = GETFNAME_TYPELOCAL ;

	    sl = getfname(pip->pr,srvfname,srvtab_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(srvfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 srvfname=%s\n",srvfname) ;
#endif

	if ((rs >= 0) || (perm(srvfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: srvtab=%s\n",pip->progname,srvfname) ;

	    if (pip->f.daemon || pip->f.named)
	        logfile_printf(&pip->lh,"srvtab=%s\n",srvfname) ;

	    rs = srvtab_open(&pip->stab,srvfname,NULL) ;

		if (rs < 0) {

	        logfile_printf(&pip->lh,"bad (%d) server file\n",rs) ;

	        bprintf(pip->efp,"%s: srvtab=%s\n",
	            pip->progname,srvfname) ;

	        bprintf(pip->efp,"%s: bad (%d) server file\n",
	            pip->progname,rs) ;

	    } else {

		pip->f.srvtab = TRUE ;
		pip->srvtab = mallocstr(srvfname) ;

		pip->f.secure_srvtab = pc.f.secure_root && pip->f.secure_conf ;
		if ((srvfname[0] != '/') || (! pip->f.secure_srvtab)) {

			rs = checksecure(srvfname,pip->euid) ;

			pip->f.secure_srvtab = (rs > 0) ;

		}

	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        for (i = 0 ; srvtab_get(&pip->stab,i,&srvp) >= 0 ; i += 1) {

	            if (srvp == NULL) continue ;

	            if (srvp->service != NULL)
	                debugprintf("main: service=%s\n",srvp->service) ;

	        } /* end for */

	    }
#endif /* CF_DEBUG */

	} else
	    rs = SR_NOEXIST ;


/* find and open the Access Table file if we have one */

#if	CF_ACCTAB
	rs = SR_NOEXIST ;
	if (accfname[0] == '\0') {

	    acctab_type = GETFNAME_TYPEROOT ;
	    strcpy(accfname,ACCFNAME) ;

	    sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN,accfname,R_OK) ;

#ifdef	COMMENT /* security problem */
	    if (sl < 0)
	        sl = permsched(sched3,&svars,
	        tmpfname,MAXPATHLEN,accfname,R_OK) ;
#endif /* COMMENT */ 

	    if (sl > 0)
	        strcpy(accfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 0 accfname=%s\n",accfname) ;
#endif

	    rs = sl ;

	} else {

	    if (acctab_type < 0)
	        acctab_type = GETFNAME_TYPELOCAL ;

	    sl = getfname(pip->pr,accfname,acctab_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(accfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 accfname=%s\n",accfname) ;
#endif

	pip->f.acctab = FALSE ;
	if ((rs >= 0) || (perm(accfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: acctab=%s\n",pip->progname,accfname) ;

	    logfile_printf(&pip->lh,"acctab=%s\n",accfname) ;

	    rs = acctab_open(&pip->atab,accfname,NULL) ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,"bad (%d) access file\n",rs) ;

	        bprintf(pip->efp,"%s: acctab=%s\n",
	            pip->progname,srvfname) ;

	        bprintf(pip->efp,"%s: bad (%d) access file\n",
	            pip->progname,rs) ;

	    } else {

	        pip->f.acctab = TRUE ;
		pip->acctab = mallocstr(accfname) ;

		pip->f.secure_acctab = pc.f.secure_root && pip->f.secure_conf ;
		if ((accfname[0] != '/') || (! pip->f.secure_acctab)) {

			rs = checksecure(accfname,pip->euid) ;

			pip->f.secure_acctab = (rs > 0) ;

		}

	    }

	} /* end if (accessing a 'acctab' file) */

#if	CF_DEBUG
	if ((pip->debuglevel > 1) && pip->f.acctab) {

		ACCTAB_CUR	ac ;

		ACCTAB_ENT	*ep ;


		debugprintf("main: netgroup machine user password\n") ;

		acctab_curbegin(&pip->atab,&ac) ;

		while (acctab_enum(&pip->atab,&ac,&ep) >= 0) {

			if (ep == NULL) continue ;

		debugprintf("main: %-20s %-20s %-8s %-8s\n",
			ep->netgroup.std,ep->machine.std,
			ep->username.std,ep->password.std) ;

		}

		acctab_curend(&pip->atab,&ac) ;

	}
#endif /* CF_DEBUG */

#endif /* CF_ACCTAB */


/* find and open the path file */

	rs = SR_NOEXIST ;
	if (pathfname[0] == '\0') {

	    strcpy(pathfname,PATHFNAME) ;

	    sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, pathfname,R_OK) ;

	    if (sl > 0)
	        strcpy(pathfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel >= 3)
	        debugprintf("main: 0 pathfname=%s\n",pathfname) ;
#endif

	    rs = sl ;

	} else {

	        path_type = GETFNAME_TYPEROOT ;
	    sl = getfname(pip->pr,pathfname,path_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(pathfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	    if (pip->debuglevel >= 3)
	    debugprintf("main: 1 pathfname=%s\n",pathfname) ;
#endif

	if ((rs >= 0) || (perm(pathfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: path=%s\n",pip->progname,pathfname) ;

	    if (pip->f.daemon || pip->f.named)
	        logfile_printf(&pip->lh,"path=%s\n",pathfname) ;

	    rs = procfilepath(pip->pr,pathfname,&pip->path) ;

		if (rs < 0) {

	        logfile_printf(&pip->lh,"bad path file (%d)\n",rs) ;

	        bprintf(pip->efp,"%s: path=%s\n",
	            pip->progname,pathfname) ;

	        bprintf(pip->efp,"%s: bad server file (%d)\n",
	            pip->progname,rs) ;

	    } else {

		pip->f.path = TRUE ;
		pip->pathfname = mallocstr(pathfname) ;

		pip->f.secure_path = pc.f.secure_root && pip->f.secure_conf ;
		if ((pathfname[0] != '/') || (! pip->f.secure_path)) {

			rs = checksecure(pathfname,pip->euid) ;

			pip->f.secure_path = (rs > 0) ;

		}

	    }

	} else
	    rs = SR_NOEXIST ;

/* add an extra default to the search path */

#ifdef	COMMENT
	vecstr_add(&pip->path,"/bin",4) ;
#endif


/* clean up some stuff we will no longer need */

	vecstr_finish(&svars) ;


/* get out now if there is no service table */

	if (rs < 0) {

	    ex = SR_OK ;
	    goto daemonret1 ;
	}


/* security summary */

	pip->f.secure = pip->f.secure_root && pip->f.secure_conf &&
		pip->f.secure_srvtab && pip->f.secure_path ;

#if	CF_SETRUID
	if (pip->f.secure && (pip->uid != pip->euid))
		u_setreuid(pip->euid,-1) ;
#endif


/* set an environment variable for the program run mode */

#ifdef	COMMENT
	if ((rs = vecstr_finder(&pip->exports,"RUNMODE",vstrkeycmp,&cp)) >= 0)
	    vecstr_del(&pip->exports,rs) ;

	vecstr_add(&pip->exports,"RUNMODE=tcpmux",-1) ;
#endif /* COMMENT */


	if (vecstr_finder(&pip->exports,"HZ",vstrkeycmp,NULL) < 0) {

	    sl = bufprintf(tmpfname,MAXPATHLEN,"HZ=%ld",CLK_TCK) ;

	    vecstr_add(&pip->exports,tmpfname,sl) ;

	}


#if	CF_DEFPATH
	if (vecstr_finder(&pip->exports,"PATH",vstrkeycmp,NULL) < 0) {

	    sl = bufprintf(tmpfname,MAXPATHLEN,"PATH=%s",DEFPATH) ;

	    if (sl > 0)
	    	vecstr_add(&pip->exports,tmpfname,sl) ;

	}
#endif /* CF_DEFPATH */

#if	CF_LOGNAME
	if ((vecstr_finder(&pip->exports,"LOGNAME",vstrkeycmp,NULL) < 0) &&
		(u.username != NULL)) {

	    sl = bufprintf(tmpfname,MAXPATHLEN,"LOGNAME=%s",u.username) ;

	    if (sl > 0)
	    	vecstr_add(&pip->exports,tmpfname,sl) ;

	}
#endif /* LOGNAME */


/* enter phase II of the startup sequence ! */

	ex = EX_DATAERR ;

	if (pip->f.log)
	    logfile_printf("stampdname=%s\n",pip->stampdname) ;


/* create the table substitutions for use later */

	varsub_start(&pip->tabsubs,0) ;

/* load up the configuration define variables */

	varsub_addvec(&pip->tabsubs,&defines) ;

/* load up the environment variables */

	varsub_addva(&pip->tabsubs,pip->envv) ;


/* we are done initializing */

	if (pip->f.daemon && pip->f.log) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: finishing initialization\n") ;
#endif

	    logfile_printf(&pip->lh,"%s finished initializing\n",
	        timestr_logz(daytime,timebuf)) ;

	} /* end if (making log entries) */


/* fill in some server information that we have so far */

	{
	    vecstr	snames ;

	    int		count = 0 ;


	    vecstr_start(&snames,0,0) ;

/* load up all of the service names that we have so far */

	    rs = SR_OK ;
	    for (i = 1 ; i <= maxai ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1) {
	            debugprintf("main: arg[%d]=>%s<\n",i,argv[i]) ;
	        }
#endif

	        if ((! BATST(argpresent,i)) ||
	            (argv[i][0] == '\0')) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: arg scv=%s\n",argv[i]) ;
#endif

		mkpath2(tmpfname,pip->maildname,argv[i]) ;

		rs = u_stat(tmpfname,&sb) ;

		if ((rs >= 0) && (! S_ISDIR(sb.st_mode))) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: found arg svc=%s\n",argv[i]) ;
#endif

	            rs = vecstr_add(&snames,argv[i],-1) ;

	            if (rs < 0)
	                break ;

	            count += 1 ;

	        } /* end if */

	        pip->f.named = TRUE ;

	    } /* end for */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: go ? rs=%d f_named=%d count=%d\n",
	            rs,pip->f.named,count) ;
#endif

	    if (rs >= 0) {

	        if ((! pip->f.named) || (count > 0)) {

#if	DMALLOC
		dmalloc_track((dmalloc_track_t) pcspoll_checker) ;
#endif

	            rs = process(pip,&snames) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: process() rs=%d\n",rs) ;
#endif

			if (rs < 0) {

			if (pip->debuglevel > 0)
			bprintf(pip->efp,
				"%s: processing problem (%d)\n",
				pip->progname,rs) ;

			logfile_printf(&pip->lh,
				"processing problem (%d)\n",
				rs) ;

			} /* end if */
		}

	        if (rs >= 0)
	            ex = EX_OK ;

	    } /* end if */


#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: vecitem_finish() snames\n") ;
#endif

	    vecstr_finish(&snames) ;

	} /* end block */


/* release the table substitutions */
bad5:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: bad5\n") ;
#endif

	varsub_finish(&pip->tabsubs) ;


/* close the daemon stuff */
daemonret2:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: daemonret2\n") ;
#endif

	if (pip->f.acctab)
	    (void) acctab_close(&pip->atab) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: srvtab_close()\n") ;
#endif

	(void) srvtab_close(&pip->stab) ;

	(void) paramfile_close(&pip->mbtab) ;


/* close some more (earlier) daemon stuff */
daemonret1:

#ifdef	OPTIONAL
	vecstr_finish(&unsets) ;

	vecstr_finish(&defines) ;

	vecstr_finish(&pip->exports) ;

	vecstr_finish(&pip->path) ;
#endif /* OPTIONAL */


#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: program finishing\n",
	        pip->progname) ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: pidfname=%s\n",pidfname) ;
#endif

	if (pidfname[0] != '\0')
	    bclose(&pidfile) ;

bad4:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: bad4\n") ;
#endif

	if (pip->f.log)
	    logfile_close(&pip->lh) ;

bad3:
bad2:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: bad2\n") ;
#endif

	if (f_freeconfigfname && (configfname != NULL))
	    free(configfname) ;

bad1:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: bad1\n") ;
#endif

	if (pip->f.slog)
	    bclose(pip->lfp) ;

retearly:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: retearly\n") ;
#endif

	bclose(pip->efp) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: exiting ex=%d\n",ex) ;
#endif

#ifdef	DMALLOC
	dmalloc_shutdown() ;
#endif

	proginfo_finish(pip) ;

	return ex ;

/* USAGE> rexd [-C conf] [-p port] [-V?] */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [svc [svc ...]] [-C conf] [-d[=runtime]] [-f]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,"%s: \t[-v] [-interval] [-m mincheck] [-D[=n]]\n",
		pip->progname) ;

	goto retearly ;

help:
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	    ex = EX_INFO ;
	goto retearly ;

/* bad stuff */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(pip->efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(pip->efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto retearly ;

mininterval:
	(void) vecstr_finish(&svars) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: minimum check interval not expired\n",
	        pip->progname) ;

	ex = EX_OK ;
	goto bad3 ;

badworking:
	(void) vecstr_finish(&svars) ;

	bprintf(pip->efp,
		"%s: could not access the working directory=%s\n",
	    pip->progname,pip->workdname) ;

	ex = EX_DATAERR ;
	goto bad3 ;

badlistinit:
	bprintf(pip->efp,"%s: could not initialize list structures (%d)\n",
	    pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto bad1 ;

badsrv:
	bprintf(pip->efp,"%s: bad service table file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badnosrv:
	bprintf(pip->efp,"%s: no service table file specified\n",
	    pip->progname) ;

	goto badret ;

badconfig:
	bprintf(pip->efp,
	    "%s: configfile=%s\n",
	    pip->progname,configfname) ;

	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badpidopen:
	(void) vecstr_finish(&svars) ;

	bprintf(pip->efp,
	    "%s: could not open the PID file (rs=%d)\n",
	    pip->progname,rs) ;

	bprintf(pip->efp, "%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	ex = EX_DATAERR ;
	goto bad2 ;

badpidlock:
	(void) vecstr_finish(&svars) ;

	if ((pip->debuglevel > 0) || pip->f.daemon) {

	    bprintf(pip->efp,
	        "%s: could not lock the PID file (%d)\n",
	        pip->progname,rs) ;

	    bprintf(pip->efp, "%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	    while ((len = breadline(&pidfile,buf,BUFLEN)) > 0) {

	        bprintf(pip->efp,"%s: pidfile> %W",
	            pip->progname,
	            buf,len) ;

	    }

	} /* end if */

	bclose(&pidfile) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	ex = EX_DATAERR ;
	goto bad2 ;

badfork:
	(void) vecstr_finish(&svars) ;

	logfile_printf(&pip->lh,
	    "could not fork (rs=%d)\n",rs) ;

	goto bad4 ;

badpidfile1:
	(void) vecstr_finish(&svars) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: could NOT capture PID lock rs=%d\n",rs1) ;
#endif

	logfile_printf(&pip->lh,
	    "could not open the PID mutex file (rs=%d)\n",
	    rs1) ;

	logfile_printf(&pip->lh, "pidfile=%s\n", pip->pidfname) ;

	goto bad4 ;

badpidfile2:
	vecstr_finish(&svars) ;

	logfile_printf(&pip->lh,
	    "there was a daemon already on the mutex file\n") ;

	goto bad4 ;

/* error types of returns */
badret:
	if (pip->f.log)
	    logfile_close(&pip->lh) ;

	ex = EX_USAGE ;
	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



/* process an environment or paths-type file */
static int procfile(pip,func,pr,svp,fname,elp)
struct proginfo	*pip ;
int		(*func)(char *,char *,vecstr *) ;
const char	pr[] ;
VECSTR		*svp ;
char		fname[] ;
vecstr		*elp ;
{
	int	rs = SR_NOEXIST ;
	int	sl ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("procfile: 1 fname=%s\n",fname) ;
#endif

	sl = permsched(sched2,svp,
		tmpfname,MAXPATHLEN, fname,R_OK) ;

	if (sl < 0)
	    sl = permsched(sched3,svp,
		tmpfname,MAXPATHLEN, fname,R_OK) ;

	if (sl > 0)
	        fname = tmpfname ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("procfile: 2 fname=%s\n",fname) ;
#endif

	if (sl >= 0)
	    rs = (*func)(pr,fname,elp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2) {
		int	i ;
		char	*cp ;
	        debugprintf("procfile: dumping exports from fname=%s\n",fname) ;
	        for (i = 0 ; vecstr_get(elp,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: export> %s\n",cp) ;
	}
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (procfile) */


/* get any of my values from the main configuration */
static int getsets(pip,sp,cip)
struct proginfo		*pip ;
vecstr			*sp ;
struct configinfo	*cip ;
{
	int	i, kl, cl, iw ;

	char	*kp, *vp ;
	char	*cp, *cp2 ;


	if ((sp == NULL) || (cip == NULL))
	    return SR_FAULT ;

	cip->mincheck = -2 ;
	cip->stampfname[0] = '\0' ;
	cip->mbfname[0] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("pcspoll/getsets: have some\n") ;
#endif

	for (i = 0 ; vecstr_get(sp,i,&cp) >= 0 ; i += 1) {

	    if (cp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("pcspoll/getsets: PCSCONF set=>%s<\n",cp) ;
#endif

	    if ((kl = matme(pip->searchname,cp,&kp,&vp)) < 0)
	        continue ;

#if	CF_DEBUGS
	    debugprintf("pcspoll/getsets: PCSCONF my key=%s\n",kp) ;
#endif

	    i = matstr(configkeys,kp,kl) ;

	    if (i >= 0) {

	        switch (i) {

	        case configkey_timestamp:
	            cl = sfshrink(vp,-1,&cp2) ;

	            if ((cl > 0) && (cl < MAXPATHLEN))
	                strwcpy(cip->stampfname,cp2,cl) ;

	            break ;

	        case configkey_mincheck:
	            cl = sfshrink(vp,-1,&cp2) ;

	            if (cl > 0) {

	                if (cfdecti(cp2,cl,&iw) >= 0)
	                    cip->mincheck = iw ;

#if	CF_DEBUGS
	                debugprintf("pcspoll/getsets: mincheck=%d\n",
				cip->mincheck) ;
#endif

	            }

	            break ;

	        case configkey_mbtab:
	            cl = sfshrink(vp,-1,&cp2) ;

	            if ((cl > 0) && (cl < MAXPATHLEN))
	                strwcpy(cip->mbfname,cp2,cl) ;

	            break ;

	        case configkey_maildir:
	            cl = sfshrink(vp,-1,&cp2) ;

	            if ((cl > 0) && (cl < MAXPATHLEN))
	                strwcpy(cip->maildname,cp2,cl) ;

	            break ;

	        } /* end switch */

	    } /* end if */

	} /* end for */

	return SR_OK ;
}
/* end subroutine (getsets) */


/* does a key match my search name ? */
static int matme(key,ts,kpp,vpp)
char	key[] ;
char	ts[] ;
char	**kpp, **vpp ;
{
	int	kl ;

	char	*cp2, *cp3 ;


	if ((cp2 = strchr(ts,'=')) == NULL)
	    return -1 ;

	if (vpp != NULL)
	    *vpp = cp2 + 1 ;

	if ((cp3 = strchr(ts,':')) != NULL) {

	if (cp3 > cp2)
	    return -1 ;

	if (kpp != NULL)
	    *kpp = cp3 + 1 ;

	if (strncmp(ts,key,(cp3 - ts)) != 0)
	    return -1 ;

	kl = (cp2 - (cp3 + 1)) ;

	} else {

	if (kpp != NULL)
	    *kpp = ts ;

	kl = cp2 - ts ;

	}

	return kl ;
}
/* end subroutine (matme) */


/* check the program time stamp file for need of processing or not */
static int checkstamp(pip,stampfname,mintime)
struct proginfo	*pip ;
char	stampfname[] ;
int	mintime ;
{
	bfile	tsfile ;

	struct ustat	sb ;

	time_t	daytime ;

	mode_t	oldmask ;

	int	rs ;
	int	f_process = TRUE ;

	char	ofname[MAXPATHLEN + 2], *timebuf = ofname ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checkstamp: mintime=%d\n",mintime) ;
#endif

	oldmask = umask(000) ;

	rs = bopenroot(&tsfile,pip->pr,stampfname,ofname,
	    "wc",0666) ;

	umask(oldmask) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("checkstamp: bopenroot() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    if (rs != SR_CREATED) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("checkstamp: not created\n") ;
#endif

	        f_process = FALSE ;
	        daytime = time(NULL) ;

	        bcontrol(&tsfile,BC_STAT,&sb) ;

	        if (daytime > (sb.st_mtime + mintime))
	            f_process = TRUE ;

	    } /* end if */

	    bprintf(&tsfile,"%s\n",
	        timestr_logz(daytime,timebuf)) ;

	    bclose(&tsfile) ;

	} /* end if (opened) */

	return f_process ;
}
/* end subroutine (checkstamp) */


static int checkmailuser(pip,name)
struct proginfo	*pip ;
const char	name[] ;
{



}
/* end subroutine (checkmailuser) */



