/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUG	0		/* switchable print-outs */
#define	CF_DEBUGWHOOPEN	0
#define	CF_GETEXECNAME	1		/* user 'getexecname()' */
#define	CF_DEFPATH	1		/* export a default PATH */
#define	CF_LOGNAME	1		/* give our LOGNAME to daemons */
#define	CF_SRVTAB	0		/* server table */
#define	CF_ACCTAB	0		/* access table */
#define	CF_PATHFILE	0		/* PATH file handling */
#define	CF_SETRUID	1		/* set real UID to EUID */
#define	CF_CHECKONC	0		/* check ONC */


/* revision history:

	= 1999-09-01, David A­D­ Morano

	This subroutine was borrowed and modified from previous generic
	front-end 'main' subroutines!


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forms the front-end part of a generic PCS type of
        program. This front-end is used in a variety of PCS programs.

        This subroutine was originally part of the Personal Communications
        Services (PCS) package but can also be used independently from it.
        Historically, this was developed as part of an effort to maintain high
        function (and reliable) email communications in the face of increasingly
        draconian security restrictions imposed on the computers in the DEFINITY
        development organization.


*****************************************************************************/


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
#include	<baops.h>
#include	<bfile.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<storebuf.h>
#include	<pcsconf.h>
#include	<ids.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"srvtab.h"
#include	"acctab.h"

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

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
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	getserial(const char *) ;
extern int	getgid_name(cchar *) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	bopenroot(bfile *,const char *,const char *,
			char *,const char *,int) ;
extern int	isNotPresent(int) ;

extern int	varsub_addva(), varsub_subbuf(), varsub_merge() ;
extern int	expander(struct proginfo *,char *,int,char *,int) ;
extern int	procfileenv(char *,char *,vecstr *) ;
extern int	procfilepath(char *,char *,vecstr *) ;

extern int	process(struct proginfo *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct errmsg {
	int	rs ;
	int	ex ;
	char	*msgstr ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	procfile(struct proginfo *,int (*)(char *,char *,vecstr *),
			const char *,vecstr *,char *,vecstr *) ;
static int	getsets(struct proginfo *,vecstr *,struct configinfo *) ;
static int	matme(char *,char *,char **,char **) ;
static int	checkstamp(struct proginfo *,const char *,int) ;
static int	geterrmsg(int) ;


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
	"newsdir",
	"timestamp",
	"mincheck",
	NULL
} ;

enum configkeys {
	configkey_newsdir,
	configkey_timestamp,
	configkey_mincheck,
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

static const struct errmsg	errmsgs[] = {
	{ SR_PROTO, EX_PROTOCOL, "protocol error" },
	{ SR_NONET, EX_NOHOST, "node not in cluster" },
	{ SR_NOENT, EX_NOPROG, "entry not found" },
	{ SR_NOTDIR, EX_UNAVAILABLE, "directory not found" },
	{ SR_HOSTUNREACH, EX_NOHOST, "node is unreachable" },
	{ SR_TIMEDOUT, EX_NOHOST, "connection attempt timed out" },
	{ SR_HOSTDOWN, EX_NOHOST, "node is down" },
	{ SR_CONNREFUSED, EX_TEMPFAIL, "connection was refused" },
	{ SR_NOMEM, EX_TEMPFAIL, "memory was low" },
	{ SR_NOTAVAIL, EX_TEMPFAIL, "node resource not available" },
	{ SR_ACCESS, EX_NOPERM, "no permission" },
	{ SR_OK, EX_OK, "" },
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	struct ustat		sb ;
	struct proginfo		pi, *pip = &pi ;
	struct userinfo		u ;
	struct configfile	cf ;
	struct pcsconf		pc ;
	struct group	ge ;
	bfile		errfile ;
	bfile		logfile ;
	bfile		pidfile ;
	vecstr		defines, unsets ;
	VECSTR		svars ;
	varsub		vsh_e, vsh_d ;
	SRVTAB_ENT	*srvp ;

	time_t	daytime = 0 ;

	int	argr, argl, aol, akl, avl ;
	int	ai, maxai, pan, npa, kwi ;
	int	argvalue = -1 ;
	int	rs, rs1, len, e, i ;
	int	mincheck = -2 ;
	int	loglen = -1 ;
	int	sl, sl2 ;
	int	ex = EX_INFO ;
	int	fd_debug = -1 ;
	int	logfile_type = -1 ;
	int	srvtab_type = -1 ;
	int	acctab_type = -1 ;
	int	path_type = -1 ;
	int	c_rootdirs = 0 ;
	int	c_dirs = 0 ;
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
	int	f_schedvar = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 2] ;
	char	buf2[BUFLEN + 2] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	newsdname[MAXPATHLEN + 2] ;
	char	stampdname[MAXPATHLEN + 2] ;
	char	tmpfname[MAXPATHLEN + 2] ;
	char	pathfname[MAXPATHLEN + 2] ;
	char	srvfname[MAXPATHLEN + 2] ;
	char	accfname[MAXPATHLEN + 2] ;
	char	pidfname[MAXPATHLEN + 2] ;
	char	lockfname[MAXPATHLEN + 2] ;
	char	logfname[MAXPATHLEN + 2] ;
	char	stampfname[MAXPATHLEN + 2] ;
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
	pip->verboselevel = 1 ;
	pip->quietlevel = 0 ;

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


	stampdname[0] = '\0' ;
	newsdname[0] = '\0' ;
	pathfname[0] = '\0' ;
	srvfname[0] = '\0' ;
	pidfname[0] = '\0' ;
	lockfname[0] = '\0' ;
	logfname[0] = '\0' ;
	stampfname[0] = '\0' ;
	accfname[0] = '\0' ;


	rs = SR_OK ;

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	ai = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	            if (isdigit(argp[1])) {

	                if ((argl - 1) > 0)
	                    rs = cfdecti((argp + 1),(argl - 1),
				&pip->interval) ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

	                    case argopt_tmpdir:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->tmpdname = avp ;

	                        } else {

	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
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
					rs = SR_INVALID ;

	                        break ;

/* verbose mode */
	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {
 
	                                    rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

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

	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
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

	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
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

	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
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
				rs = SR_INVALID ;
	                        bprintf(pip->efp,
					"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {
 
	                                    rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

					}
	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                configfname = argp ;

	                            break ;

/* news directory */
	                        case 'N':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                strwcpy(newsdname,argp,argl) ;

	                            break ;

/* mutex lock PID file */
	                        case 'P':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
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

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdecti(avp,avl,
	                                    	&pip->runtime) ;

					}
	                            }

	                            break ;

/* force a run even if our minimum time between executions is not expired */
	                        case 'f':
	                            mincheck = 0 ;
	                            break ;

/* minimum check time */
	                        case 'm':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdecti(argp,argl,&mincheck) ;

	                            }

	                            break ;

/* quiet mode */
	                        case 'q':
					pip->quietlevel = 1 ;
	                            pip->f.quiet = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

	                                    rs = cfdecti(avp,avl,
	                                    	&pip->quietlevel) ;

					}
	                            }

	                            break ;

/* verbose mode */
	                        case 'v':
					pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {
 
	                                    rs = cfdeci(avp,avl,
	                                    	&pip->verboselevel) ;

					}
	                            }

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
				break ;

	                        default:
	                            rs = SR_INVALID ;
	                            bprintf(pip->efp,
					"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
					break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	    } else {

	        if (ai < MAXARGINDEX) {

	            BASET(argpresent,ai) ;
	            maxai = ai ;
	            npa += 1 ;

	            pip->f.named = TRUE ;

	        } else {

	            if (! f_extra) {

			rs = SR_INVALID ;
	                f_extra = TRUE ;
	                bprintf(pip->efp,"%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0)
		goto badarg ;

	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: finished parsing arguments\n",
	        pip->progname) ;

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

/* miscellaneous */

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGWHOOPEN
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

/* get some host/user information (offensive to ONC secure operations!) */

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

	pip->gid_pcs = -1 ;

	pip->uid = u.uid ;
	pip->euid = geteuid() ;

	pip->gid = u.gid ;
	pip->egid = getegid() ;


	rs = getgr_gid(&ge,buf,BUFLEN,pip->gid) ;

	if (rs < 0) {

	    cp = buf ;
	    bufprintf(buf,BUFLEN,"GID%d",(int) pip->gid) ;

	} else
	    cp = ge.gr_name ;

	pip->groupname = mallocstr(cp) ;


/* root secure ? */

	rs = securefile(pip->pr,pip->euid,pip->egid) ;

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

	rs1 = vecstr_start(&svars,6,0) ;
	f_schedvar = (rs1 >= 0) ;

	vecstr_envset(&svars,"p",pip->pr,-1) ;

	vecstr_envset(&svars,"e","etc",-1) ;

	vecstr_envset(&svars,"n",pip->searchname,-1) ;

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
			rs = securefile(configfname,pip->euid,pip->egid) ;
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

			vecstr_envset(&svars,"p",pip->pr,-1) ;

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
	            debugprintf("main: processed CF logfilename=%s\n",
			logfname) ;
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
		vecstr_envset(&svars,"p",pip->pr,-1) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

/* before we go too far, are we the only one on this PID mutex ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 pidfname=%s\n",pidfname) ;
#endif

	if ((pidfname[0] == '\0') || (pidfname[0] == '-'))
	        strcpy(pidfname,PIDFNAME) ;

	if (pidfname[0] != '/') {
		rs1 = mkpath2(tmpfname,pip->pr,pidfname) ;
		if (rs1 > 0)
			strcpy(pidfname,tmpfname) ;
	}

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 pidfname=%s\n",pidfname) ;
#endif

	    pip->pidfname = pidfname ;

/* before proceeding too much further, what does PCSCONF say ? */

	{
	    struct configinfo	ci ;
	    vecstr	sets ;

	    vecstr_start(&sets,10,0) ;

	    rs = pcsconf(pip->pr,NULL,&pc,&sets,NULL,pcsconfbuf,PCSCONF_LEN) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: pcsconf() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

/* get any program-specific options */

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

	            if (ci.newsdname[0] != '\0')
	                strcpy(newsdname,ci.newsdname) ;

	            if (ci.stampfname[0] != '\0')
	                strcpy(stampfname,ci.stampfname) ;

		} /* end if (handling sets) */

/* get any general options */

		if ((pc.pcslogin != NULL) && (pc.pcslogin[0] != '\0')) {
		    cchar	*un = pc.pcslogin ;
		    if ((rs = getgid_name(un)) >= 0) {
			pip->gid_pcs = pw.pw_gid ;
		    } else if (isNotPresent(rs)) {
			pip->gid_pcs = getgid() ;
			rs = SR_OK ;
		    }
		}

	    } /* end if (got PCS configuration settings) */

	    vecstr_finish(&sets) ;
	} /* end block (PCS configuration) */

/* can we get out right now? */

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

	} /* end if (debugging) */

	if (mincheck == -1)
	        goto mininterval ;


/* what about a news directory ? */

	if (newsdname[0] == '\0')
	    strcpy(newsdname,NEWSDNAME) ;

	if (newsdname[0] != '/') {
	    rs = mkpath2(tmpfname, pip->pr,newsdname) ;
	    if (rs > 0)
	    	strcpy(newsdname,tmpfname) ;
	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: newsdir=%s\n",
		pip->progname,newsdname) ;


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
	    sl = mkpath2(tmpfname, stampdname,stampfname) ;
	    if (sl > 0)
	        strcpy(stampfname,tmpfname) ;
	}

	if (stampfname[0] != '/') {
	    sl = mkpath2(tmpfname, pip->pr,stampfname) ;
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

	    if (daytime == 0)
	        daytime = time(NULL) ;

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
	if (DEBUGLEVEL(2))
	    debugprintf("main: serialfile=%s\n",cp) ;
#endif

	rs = getserial(cp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: getserial() rs=%d\n",rs) ;
#endif

	if (rs < 0)
		rs = pip->pid ;

	pip->serial = rs ;
	(void) storebuf_dec(buf,BUFLEN,0,pip->serial) ;

	pip->logid = mallocstr(buf) ;

	buf[0] = '\0' ;
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

	        daytime = time(NULL) ;

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


/* log some stuff */

	if (pip->f.log) {

	if (pip->f.daemon || pip->f.named)
	    logfile_printf(&pip->lh,"conf=%s\n",configfname) ;

	    logfile_printf(&pip->lh,
	        "pr=%s\n",pip->pr) ;

	}


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

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGWHOOPEN && 0
	    if (pip->debuglevel > 1) {

	        debugprintf("main: after daemon backgrounding\n") ;

	        d_whoopen("3") ;

	    }
#endif /* CF_DEBUG */

		pip->pid = getpid() ;

	    logfile_printf(&pip->lh,"backgrounded pid=%d\n",pip->pid) ;

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
	(void) storebuf_dec(buf,BUFLEN,0,pip->pid) ;

	logfile_setid(&pip->lh,buf) ;
#endif /* COMMENT */


/* before we go too far, are we the only one on this PID mutex ? */

	if (mincheck != 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: have PID file ?\n") ;
#endif

	    if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {
		int	devmajor, devminor ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: pidfname=%s\n",pip->pidfname) ;
#endif

		rs1 = lfm_start(&pip->pidlock,
			pip->pidfname,LFM_TRECORD, TO_PIDLOCK,NULL,
			u.nodename,u.username,BANNER) ;

		if (rs1 < 0)
	            goto badpidfile1 ;

	        if (userbuf[0] != '\0')
	            lfm_printf(&pip->pidlock,
			"host=%s.%s user=%s pid=%d\n",
	                u.nodename,u.domainname,
	                u.username,
	                pip->pid) ;

	        else
	            lfm_printf(&pip->pidlock,"host=%s.%s pid=%d\n",
	                u.nodename,u.domainname,
	                pip->pid) ;

/* we leave the file open as our mutex lock ! */

	        logfile_printf(&pip->lh,"pidfile=%s\n",pip->pidfname) ;

	        logfile_printf(&pip->lh,"PID mutex captured\n") ;

		pip->f.pidlock = TRUE ;
		lfm_flush(&pip->pidlock) ;

	    } /* end if (we have a mutex PID file) */

	} /* end if (non-forced mode) */


#if	CF_SRVTAB

/* find and open the server table file */

	rs = SR_NOEXIST ;
	if (srvfname[0] == '\0') {

	    srvtab_type = GETFNAME_TYPEROOT ;
	    strcpy(srvfname,SRVFNAME) ;

	    sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, srvfname,R_OK) ;

	    if (sl < 0)
	        sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, srvfname,R_OK) ;

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

			rs = securefile(srvfname,pip->euid,pip->egid) ;

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

#endif /* CF_SRVTAB */


#if	CF_ACCTAB

/* find and open the Access Table file if we have one */

	rs = SR_NOEXIST ;
	if (accfname[0] == '\0') {

	    acctab_type = GETFNAME_TYPEROOT ;
	    strcpy(accfname,ACCFNAME) ;

	    sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, accfname,R_OK) ;

#ifdef	COMMENT /* security problem */
	    if (sl < 0)
	        sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, accfname,R_OK) ;
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

			rs = securefile(accfname,pip->euid,pip->egid) ;

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


#if	CF_PATHFILE

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

			rs = securefile(pathfname,pip->euid,pip->egid) ;

			pip->f.secure_path = (rs > 0) ;

		}

	    }

	} else
	    rs = SR_NOEXIST ;

/* add an extra default to the search path */

#ifdef	COMMENT
	vecstr_add(&pip->path,"/bin",4) ;
#endif

#endif /* CF_PATHFILE */

/* clean up some stuff we will no longer need */

	if (f_schedvar) {
	    f_schedvar = FALSE ;
	    schedvar_finish(&svars) ;
	}

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


	pan = 0 ;
	if (npa > 0) {

		for (ai = 1 ; ai <= maxai ; ai += 1) {

		if (BATST(argpresent,ai) && (argv[ai] != NULL)) {

			pan += 1 ;
			c_rootdirs += 1 ;
			rs1 = process(pip,argv[ai]) ;

			if (rs1 >= 0) {

				c_dirs += rs1 ;
				if (pip->debuglevel > 0)
				bprintf(pip->efp,"%s: rd=%s c=%u\n",
					pip->progname,argv[ai],rs1) ;

			} else if (pip->debuglevel > 0) {

				e = geterrmsg(rs1) ;

				cp = (e >= 0) ? errmsgs[e].msgstr : "" ;
				bprintf(pip->efp,"%s: %s (%d)\n",
					pip->progname,cp,rs1) ;

			}

		}

		} /* end for */

	} /* end if (positional arguments) */

	if ((pan == 0) && (newsdname[0] != '\0')) {

		pan += 1 ;
			c_rootdirs += 1 ;
		rs1 = process(pip,newsdname) ;

			if (rs1 >= 0) {

				c_dirs += rs1 ;
				if (pip->debuglevel > 0)
				bprintf(pip->efp,"%s: rd=%s c=%u\n",
					pip->progname,argv[ai],rs1) ;

			} else if (pip->debuglevel > 0) {

				e = geterrmsg(rs1) ;

				cp = (e >= 0) ? errmsgs[e].msgstr : "" ;
				bprintf(pip->efp,"%s: %s (%d)\n",
					pip->progname,cp,rs1) ;

			}

	}

	if ((pan == 0) && ((cp = getenv(NEWSDIRVAR)) != NULL)) {

		pan += 1 ;
			c_rootdirs += 1 ;
		rs1 = process(pip,cp) ;

			if (rs1 >= 0) {

				c_dirs += rs1 ;
				if (pip->debuglevel > 0)
				bprintf(pip->efp,"%s: rd=%s c=%u\n",
					pip->progname,argv[ai],rs1) ;

			} else if (pip->debuglevel > 0) {

				e = geterrmsg(rs1) ;

				cp = (e >= 0) ? errmsgs[e].msgstr : "" ;
				bprintf(pip->efp,"%s: %s (%d)\n",
					pip->progname,cp,rs1) ;

			}

	}


	if (pip->f.pidlock)
		lfm_finish(&pip->pidlock) ;

	if (pip->debuglevel > 0) {

		bprintf(pip->efp,"%s: rootdirs=%u dirs=%u\n",
			pip->progname,
			c_rootdirs,c_dirs) ;

	}


/* close the daemon stuff */
daemonret2:

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: daemonret2\n") ;
#endif

#if	CF_ACCTAB
	if (pip->f.acctab)
	    (void) acctab_close(&pip->atab) ;
#endif

#if	CF_SRVTAB

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	debugprintf("main: srvtab_close()\n") ;
#endif

	(void) srvtab_close(&pip->stab) ;

#endif /* CF_SRCTAB */

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

/* usage */
usage:
	usage(pip) ;

	goto retearly ;

help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

mininterval:
	schedvar_finish(&svars) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: minimum check interval not expired\n",
	        pip->progname) ;

	ex = EX_OK ;
	goto bad3 ;

badworking:
	(void) schedvar_finish(&svars) ;

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
	ex = EX_UNAVAILABLE ;
	bprintf(pip->efp,
	    "%s: configfile=%s\n",
	    pip->progname,configfname) ;

	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badpidopen:
	(void) schedvar_finish(&svars) ;

	bprintf(pip->efp,
	    "%s: could not open the PID file (rs=%d)\n",
	    pip->progname,rs) ;

	bprintf(pip->efp, "%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	ex = EX_DATAERR ;
	goto bad2 ;

badpidlock:
	(void) schedvar_finish(&svars) ;

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
	ex = EX_OSERR ;
	(void) schedvar_finish(&svars) ;

	logfile_printf(&pip->lh,
	    "could not fork (rs=%d)\n",rs) ;

	goto bad4 ;

badpidfile1:
	ex = EX_OSERR ;
	(void) schedvar_finish(&svars) ;

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
	ex = EX_OSERR ;
	(void) schedvar_finish(&svars) ;

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



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;


	bprintf(pip->efp,
	    "%s: USAGE> %s [newsdir(s) ...] [-C conf] [-f] \n",
	    pip->progname,pip->progname) ;

	rs = bprintf(pip->efp,"%s: \t[-v] [-interval] [-m mincheck] [-D[=n]]\n",
		pip->progname) ;

	return rs ;
}
/* end subroutine (usage) */


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
	    debugprintf("main/procfile: 1 fname=%s\n",fname) ;
#endif

	sl = permsched(sched2,svp, tmpfname,MAXPATHLEN, fname,R_OK ) ;

	if (sl < 0)
	    sl = permsched(sched3,svp,
	        tmpfname,MAXPATHLEN, fname,R_OK) ;

	if (sl > 0)
	        fname = tmpfname ;

#if	CF_DEBUG
	if (pip->debuglevel > 2)
	    debugprintf("main/procfile: 2 fname=%s\n",fname) ;
#endif

	if (sl >= 0)
	    rs = (*func)(pr,fname,elp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 2) {
		int	i ;
		char	*cp ;
	        debugprintf("main/procfile: dumping exports from fname=%s\n",
			fname) ;
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

	memset(cip,0,sizeof(struct configinfo))  ;

	cip->mincheck = -2 ;

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

		case configkey_newsdir:
	            cl = sfshrink(vp,-1,&cp2) ;

	            if ((cl > 0) && (cl < MAXPATHLEN))
	                strwcpy(cip->newsdname,cp2,cl) ;

			break ;

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
	char	*cp2, *cp3 ;


	if ((cp2 = strchr(ts,'=')) == NULL)
	    return -1 ;

	if (vpp != NULL)
	    *vpp = cp2 + 1 ;

	if ((cp3 = strchr(ts,':')) == NULL)
	    return -1 ;

	if (cp3 > cp2)
	    return -1 ;

	if (kpp != NULL)
	    *kpp = cp3 + 1 ;

	if (strncmp(ts,key,(cp3 - ts)) != 0)
	    return -1 ;

	return (cp2 - (cp3 + 1)) ;
}
/* end subroutine (matme) */


/* check the program time stamp file for need of processing or not */
static int checkstamp(pip,stampfname,mintime)
struct proginfo	*pip ;
const char	stampfname[] ;
int		mintime ;
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


static int geterrmsg(e)
int	e ;
{
	int	i ;


	for (i = 0 ; errmsgs[i].rs != 0 ; i += 1) {

		if (errmsgs[i].rs == e)
			break ;

	}

	return (errmsgs[i].rs != 0) ? i : -1 ;
}
/* end subroutine (geterrmsg) */



