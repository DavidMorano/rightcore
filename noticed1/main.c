/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time */
#define	CF_ACCTAB	1


/* revision history:

	= 1999-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

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
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<varsub.h>
#include	<storebuf.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<pwfile.h>
#include	<userinfo.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"srvtab.h"
#include	"acctab.h"

#include	"builtin.h"
#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	sfkey(char *,int,char **) ;
extern int	vstrkeycmp(char **,char **) ;
extern int	getpwd(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	matstr(char *const *,char *,int) ;
extern int	matstr2(char *const *,char *,int) ;
extern int	getfname(char *,char *,int,char *) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;
extern int	varsub_load(), varsub_subbuf(), varsub_merge() ;
extern int	expander() ;
extern int	procfileenv(char *,char *,VECSTR *) ;
extern int	procfilepaths(char *,char *,VECSTR *) ;
extern int	watch(struct proginfo *,struct serverinfo *,
			varsub *,vecstr *,SRVTAB *,ACCTAB *,BUILTIN *) ;
extern int	isdigitlatin(int) ;

extern char	*strnchr(char *,int,int) ;
extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;

#if	CF_DEBUG
extern void	d_whoopen(char *) ;
#endif


/* externals variables */


/* forward references */

static int	checkspooldir() ;
static int	checkfiledir() ;
static int	procfile(struct proginfo *,int (*)(char *,char *,VECSTR *),
			char *,vecstr *,char *,VECSTR *) ;


/* global variables */


/* local structures */


/* local variables */

/* define command option words */

static char *const argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	"CONFIG",
	NULL
} ;

#define	ARGOPT_TMPDIR		0
#define	ARGOPT_VERSION		1
#define	ARGOPT_VERBOSE		2
#define	ARGOPT_ROOT		3
#define	ARGOPT_LOGFILE		4
#define	ARGOPT_CONFIG		5

/* configuration options */

static char	*const configopts[] = {
	"marktime",
	"defacc",
	"spooldir",
	NULL
} ;

#define	CONFIGOPTS_MARKTIME	0
#define	CONFIGOPTS_DEFACC	1
#define	CONFIGOPTS_SPOOLDIR	2

/* 'conf' for most regular programs */
static char	*const sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

/* non-'conf' ETC stuff for all regular programs */
static char	*const sched2[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	"%p/%n.%f",
	NULL
} ;

/* 'conf' and non-'conf' ETC stuff for local searching */
static char	*const sched3[] = {
	"%e/%n/%n.%f",
	"%e/%n/%f",
	"%e/%n.%f",
	"%e/%f",
	"%n.%f",
	"%f",
	NULL
} ;

static const unsigned char 	oterms[32] = {
	0x00, 0x0B, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[], *envv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		logfile ;
	bfile		pidfile ;

	struct ustat		sb ;

	struct proginfo		g, *pip = &g ;

	struct serverinfo	si ;

	struct userinfo		u ;

	CONFIGFILE		cf ;

	struct servent	se, *sep ;

	struct protoent	pe, *pep ;

	struct hostent	he, *hep ;

	struct group	ge ;

	vecstr		defines, unsets, exports ;
	VECSTR		svars ;

	varsub		tabsubs ;
	varsub		vsdefines, vsexports ;

	BUILTIN		bis ;

	PWFILE		passwd ;

	SRVTAB		sfile ;

	ACCTAB		atab ;

	SRVTAB_ENT	*srvp ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j, k ;
	int	ex = EX_USAGE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	len, rs ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	loglen = -1 ;
	int	iw, sl, sl2 ;
	int	logfile_type = -1 ;
	int	srvtab_type = -1 ;
	int	acctab_type = -1 ;
	int	passwd_type = -1 ;
	int	f_programroot = FALSE ;
	int	f_freeconfigfname = FALSE ;
	int	f_procfileenv = FALSE ;
	int	f_procfilepaths = FALSE ;
	int	f_reuse = FALSE ;
	int	fd_debug ;

	const char	*argp, *aop, *akp, *avp ;
	const char	argpresent[MAXARGGROUPS] ;
	const char	*programroot = NULL ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	srvfname[MAXPATHLEN + 1] ;
	char	accfname[MAXPATHLEN + 1] ;
	char	pidfname[MAXPATHLEN + 1] ;
	char	lockfname[MAXPATHLEN + 1] ;
	char	logfname[MAXPATHLEN + 1] ;
	char	passwdfname[MAXPATHLEN + 1] ;
	char	pwd[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	portbuf[10] ;
	const char	*pr = NULL ;
	const char	*configfname = NULL ;
	const char	*portspec = NULL ;
	const char	*cp ;


	if ((cp = getenv(VARDEBUGFD)) == NULL)
		cp = getenv("ERROR_FD") ;

	if ((cp != NULL) && (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


/* we want to open up some files so that the first few FD slots are FULL !! */

	if (u_fstat(FD_STDIN,&sb) < 0)
	    (void) u_open("/dev/null",O_RDONLY,0666) ;

	(void) memset(pip,0,sizeof(struct proginfo)) ;

	pip->f.fd_stdout = TRUE ;
	if (u_fstat(FD_STDOUT,&sb) < 0) {

	    pip->f.fd_stdout = FALSE ;
	    (void) u_open("/dev/null",O_WRONLY,0666) ;

	}

	pip->f.fd_stderr = TRUE ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) < 0) {

	    pip->f.fd_stderr = FALSE ;
	    (void) u_open("/dev/null",O_WRONLY,0666) ;

	} else
	    bcontrol(efp,BC_LINEBUF,0) ;

#ifdef	COMMENT
	pip->efp = efp ;
#endif
	pip->envv = envv ;
	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;

	pip->pid = getpid() ;

	pip->ppid = pip->pid ;


/* initialize */

	pip->f.quiet = FALSE ;
	pip->f.verbose = FALSE ;
	pip->f.log = FALSE ;
	pip->f.slog = FALSE ;
	pip->f.daemon = FALSE ;
	pip->f.acctab = FALSE ;

	pip->debuglevel = 0 ;
	pip->marktime = -1 ;

	pip->lfp = &logfile ;
	pip->efp = &errfile ;

	pip->programroot = NULL ;
	pip->username = NULL ;
	pip->groupname = NULL ;
	pip->pidfname = NULL ;
	pip->lockfname = NULL ;
	pip->tmpdname = NULL ;
	pip->workdir = NULL ;
	pip->defuser = NULL ;
	pip->defgroup = NULL ;
	pip->userpass = NULL ;
	pip->machpass = NULL ;
	pip->defacc = NULL ;
	pip->spooldname = NULL ;
	pip->prog_rmail = NULL ;
	pip->prog_sendmail = NULL ;


	srvfname[0] = '\0' ;
	accfname[0] = '\0' ;
	pidfname[0] = '\0' ;
	lockfname[0] = '\0' ;
	logfname[0] = '\0' ;
	passwdfname[0] = '\0' ;


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
		const int	ach = MKCHAR(argp[1]) ;

	        if (argl > 1) {

	            if (isdigitlatin(ach)) {
	                int	port ;

	                if (((argl - 1) > 0) && 
	                    (cfdeci(argp + 1,argl - 1,&port) < 0))
	                    goto badargval ;

	                portspec = portbuf ;
	                storebuf_dec(portbuf,9,0,port) ;

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

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

	                    switch (kwi) {

	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) pip->tmpdname = avp ;

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
	                    case ARGOPT_VERSION:
#if	CF_DEBUGS
	                        debugprintf("main: version key-word\n") ;
#endif
	                        f_version = TRUE ;
	                        if (f_optequal) 
					goto badargextra ;

	                        break ;

/* verbose mode */
	                    case ARGOPT_VERBOSE:
	                        pip->f.verbose = TRUE ;
	                        break ;

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->programroot = argp ;

	                        }

	                        break ;

/* configuration file */
	                    case ARGOPT_CONFIG:
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

/* log file */
	                    case ARGOPT_LOGFILE:
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
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                        goto badargnot ;

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

#if	CF_DEBUGS
	                                debugprintf(
	                                    "main: debug flag, avp=\"%W\"\n",
	                                    avp,avl) ;
#endif

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
	                                    (cfdeci(avp,avl,
	                                    &pip->debuglevel) < 0))
	                                    goto badargval ;

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
	                            break ;

/* TCP port to listen on */
	                        case 'p':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                portspec = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* reuse the bind addresses */
	                        case 'r':
	                            f_reuse = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->f.verbose = TRUE ;
	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        default:
				    ex = EX_USAGE ;
				    f_usage = TRUE ;
	                            bprintf(efp,"%s: unknown option - %c\n",
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

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (pip->debuglevel > 1)
	    bprintf(efp,"%s: finished parsing arguments\n",
	        pip->progname) ;


/* load the positional arguments */

	ex = EX_DATAERR ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1) 
		    debugprintf("main: got positional arg i=%d pan=%d arg=%s\n",
	            i,pan,argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            if ((strlen(argv[i]) > 0) && (argv[i][0] != '-'))
	                configfname = argv[i] ;

	            break ;

	        case 1:
	            if ((strlen(argv[i]) > 0) && (argv[i][0] != '-'))
	                portspec = argv[i] ;

	            break ;

	        default:
			ex = EX_USAGE ;
	                f_usage = TRUE ;
	            bprintf(efp,
	                "%s: extra positional arguments ignored\n",
	                pip->progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */


	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto earlyret ;

	if (pip->debuglevel > 1)
	    bprintf(efp,"%s: debug level %d\n",
	        pip->progname,pip->debuglevel) ;


/* miscellaneous */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf( "main: starting FDs\n") ;

	    d_whoopen("1") ;

	}
#endif


/* get our program root */

	if (pip->programroot == NULL) {

	    programroot = getenv(VARPROGRAMROOT1) ;

	    if (programroot == NULL)
	        programroot = getenv(VARPROGRAMROOT2) ;

	    if (programroot == NULL)
	        programroot = getenv(VARPROGRAMROOT3) ;

	    if (programroot == NULL)
	        programroot = PROGRAMROOT ;

	    pip->programroot = programroot ;

	} else {

	    f_programroot = TRUE ;
	    programroot = pip->programroot ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: programroot=%s\n",pip->programroot) ;
#endif


/* get some host/user information */

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


/* make the hostname */

	sl = bufprintf(buf,BUFLEN,"%s.%s",pip->nodename,pip->domainname) ;

	if (sl > 0)
		pip->hostname = mallocstrw(buf,sl) ;


/* PWD */

	rs = getpwd(buf,BUFLEN) ;

	if (rs > 0)
		pip->pwd = mallocstrw(buf,rs) ;


/* prepare to store configuration variable lists */

	if ((rs = vecstr_start(&defines,10,VECSTR_PORDERED)) < 0)
	    goto badlistinit ;

	if ((rs = vecstr_start(&unsets,10,VECSTR_PNOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    goto badlistinit ;
	}

	if ((rs = vecstr_start(&exports,10,VECSTR_PNOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    vecstr_finish(&unsets) ;

	    goto badlistinit ;
	}

/* create the values for the file schedule searching */

	vecstr_start(&svars,6,0) ;

	vecstr_envset(&svars,"p",pip->programroot) ;

	vecstr_envset(&svars,"e","etc") ;

	vecstr_envset(&svars,"n",SEARCHNAME) ;

/* load up some initial environment that everyone should have ! */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: about to do DEFINITFNAME=%s\n",DEFINITFNAME) ;
#endif

	procfileenv(pip->programroot,DEFINITFNAME,&exports) ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    debugprintf("main: checking for configuration file\n") ;

	    debugprintf("main: 0 CF=%s\n",configfname) ;

	}
#endif /* CF_DEBUG */

	rs = SR_NOEXIST ;
	if ((configfname == NULL) || (configfname[0] == '\0')) {

	    configfname = CONFFNAME ;

	    sl = permsched(sched1,&svars,
	        tmpfname,MAXPATHLEN, configfname,R_OK) ;
	    if (sl < 0) {

	        sl = permsched(sched3,&svars,
	        tmpfname,MAXPATHLEN, configfname,R_OK) ;
		if (sl > 0)
	            configfname = tmpfname ;

	    } else if (sl > 0)
	        configfname = tmpfname ;

	    rs = sl ;

	} else {

	    sl = getfname(pip->programroot,configfname,1,tmpfname) ;

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

	    FIELD	fsb ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: configuration file \"%s\"\n",
	            configfname) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: configfile=%s\n",
	            pip->progname,configfname) ;

	    if ((rs = configfile_start(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("main: we have a good configuration file\n") ;
	        debugprintf("main: initial programroot=%s\n",
	            pip->programroot) ;
	    }
#endif


/* we must set this mode to 'VARSUB_MBADNOKEY' so that a miss is noticed */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_start d\n") ;
#endif

	    varsub_start(&vsdefines,VARSUB_MBADNOKEY) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_start e\n") ;
#endif

	    varsub_start(&vsexports,0) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_addva\n") ;
#endif

	    varsub_load(&vsexports,envv) ;

#if	CF_DEBUG && F_VARSUBDUMP
	    if (pip->debuglevel > 1) {

	        debugprintf("main: 0 for\n") ;

	        varsub_dumpfd(&vsexports,-1) ;

	    }
#endif /* CF_DEBUG */


/* program root from configuration file */

	    if ((cf.root != NULL) && (! f_programroot)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF programroot\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            pip->programroot = mallocstrw(buf2,sl2) ;

	        }

#if	CF_DEBUGS
	        debugprintf("main: new programroot=%s\n",
	            pip->programroot) ;
#endif

	    } /* end if (configuration file program root) */


/* loop through the DEFINEd variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: 0 top, cp=%s\n",cp) ;
#endif

	        sl = varsub_subbuf(&vsdefines,&vsexports,cp,-1,buf,BUFLEN) ;

	        if ((sl > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 0 about to merge >%W<\n",
	                    buf2,sl2) ;
#endif

	            varsub_merge(&vsdefines,&defines,buf2,sl2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 0 out of merge\n") ;
#endif

	        } /* end if */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf( "main: 0 bot\n") ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ defines\n") ;
#endif


/* all of the rest of the configuration file stuff */

	    if ((cf.workdir != NULL) && (pip->workdir == NULL)) {

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.workdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            pip->workdir = mallocstrw(buf2,sl2) ;

	        }

	    } /* end if (config file working directory) */


	    if (pip->f.daemon && (cf.pidfname != NULL) && 
	        (pidfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF pidfile=%s\n",cf.pidfname) ;
#endif

	        if ((cf.pidfname[0] != '\0') && (cf.pidfname[0] != '-')) {

	            if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.pidfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((sl2 = expander(&g,buf,sl, buf2,BUFLEN)) > 0)) {

	                strwcpy(pidfname,buf2,sl2) ;

	            }

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: pidfname=%s\n",pidfname) ;
#endif

	    } /* end if (configuration file PIDFNAME) */


	    if ((cf.lockfname != NULL) && (lockfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF lockfile=%s\n",cf.lockfname) ;
#endif

	        if ((cf.lockfname[0] != '\0') && (cf.lockfname[0] != '-')) {

	            if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.lockfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((sl2 = expander(&g,buf,sl, buf2,BUFLEN)) > 0)) {

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

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(logfname,buf2,sl2) ;

	            if (strchr(logfname,'/') != NULL)
	                logfile_type = 1 ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",logfname) ;
#endif

	    } /* end if (configuration file log filename) */

	    if ((cf.port != NULL) && (portspec == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing port\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.port,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            portspec = portbuf ;
	            strwcpy(portbuf,buf2,sl2) ;

	        }

	    } /* end if (handling the configuration file port parameter) */

	    if ((cf.user != NULL) && (pip->defuser == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF user \n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.user,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            pip->defuser = mallocstrw(buf2,sl2) ;

	        }

	    }

	    if ((cf.group != NULL) && (pip->defgroup == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF group \n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.group,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            pip->defgroup = mallocstrw(buf2,sl2) ;

	        }

	    } /* end if */

	    if ((cf.userpass != NULL) && (pip->userpass == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF user password\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.userpass,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            pip->userpass = mallocstrw(buf2,sl2) ;

	        }

	    }

	    if ((cf.machpass != NULL) && (pip->machpass == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF machine password\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.machpass,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            pip->machpass = mallocstrw(buf2,sl2) ;

	        }

	    } /* end if */

	    if ((cf.srvtab != NULL) && (srvfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF processing SRVTAB \n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.srvtab,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(srvfname,buf2,sl2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: CF SRVTAB s&e srvtab=%s\n",
	                    srvfname) ;
#endif

	            srvtab_type = 0 ;
	            if (strchr(srvfname,'/') != NULL)
	                srvtab_type = 1 ;

	        }

	    } /* end if (srvtab) */

#if	CF_ACCTAB
	    if ((cf.acctab != NULL) && (accfname[0] == '\0')) {

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.acctab,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(accfname,buf2,sl2) ;

	            acctab_type = 0 ;
	            if (strchr(accfname,'/') != NULL)
	                acctab_type = 1 ;

	        }

	    } /* end if (acctab) */
#endif /* CF_ACCTAB */


	    if ((cf.sendmail != NULL) && (pip->prog_sendmail == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF sendmail\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.sendmail,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            pip->prog_sendmail = mallocstrw(buf2,sl2) ;

	        }

	    } /* end if (sendmail) */


/* what about an 'environ' file ? */

	    if (cf.envfname != NULL) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: 1 envfile=%s\n",cf.envfname) ;
#endif

	        procfileenv(pip->programroot,cf.envfname,&exports) ;

	    } else
	        procfile(pip,procfileenv,pip->programroot,&svars,
	            ENVFNAME,&exports) ;

	    f_procfileenv = TRUE ;


/* "do" any 'paths' file before we process the environment variables */

	    if (cf.pathfname != NULL)
	        procfilepaths(pip->programroot,cf.pathfname,&exports) ;

	    else
	        procfile(pip,procfilepaths,pip->programroot,&svars,
	            PATHSFNAME,&exports) ;

	    f_procfilepaths = TRUE ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

/* OK, now loop through any options that we have */

	    if (cf.options != NULL) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF options\n") ;
#endif

	        if (((sl = varsub_subbuf(&vsdefines,&vsexports,cf.options,
	            -1,buf,BUFLEN)) > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	        field_start(&fsb,buf2,sl2) ;

	        while (field_get(&fsb,oterms) >= 0) {

	            if (fsb.flen == 0) continue ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: CF option >%W<\n",
				fsb.fp,fsb.flen) ;
#endif

	            if ((sl = sfkey(fsb.fp,fsb.flen,&cp)) < 0) {

				cp = fsb.fp ;
				sl = fsb.flen ;
			}

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: CF option key >%W<\n",cp,sl) ;
#endif

	            if ((i = matstr3(configopts,cp,sl)) >= 0) {

	                char	*cp2 ;


	                cp2 = strnchr(cp + sl,(fsb.flen - sl),'=') ;

			sl2 = 0 ;
	                if (cp2 != NULL) {

	                    cp2 += 1 ;
			    sl2 = fsb.flen - (cp2 - fsb.fp) ;

			}

#if	CF_DEBUG
	                if (pip->debuglevel > 1) {
	                    debugprintf("main: CF option match i=%d\n",i) ;
	                    debugprintf("main: cp2=%W\n",cp2,sl2) ;
			}
#endif

	                switch (i) {

	                case CONFIGOPTS_MARKTIME:
	                    if ((pip->marktime <= 0) && (cp2 != NULL) &&
	                        (cfdecti(cp2,sl2,&iw) >= 0)) {

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("main: CF opt marktime=%d\n",
					iw) ;
#endif

	                        pip->marktime = iw ;

	                    } /* end if */

	                    break ;

	                case CONFIGOPTS_DEFACC:
	                    if (pip->defacc == NULL) {

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("main: CF opt defacc=%s\n",
					cp2) ;
#endif

	                            pip->defacc = "DEFAULT" ;
				if (cp2 != NULL)
				    pip->defacc = mallocstrw(cp2,sl2) ;

			    }

	                    break ;

	                case CONFIGOPTS_SPOOLDIR:
	                    if (pip->spooldname == NULL) {

#if	CF_DEBUG
	                        if (pip->debuglevel > 1)
	                            debugprintf("main: CF opt spooldir=%s\n",
					cp2) ;
#endif

				pip->spooldname = "" ;
				if (cp2 != NULL)
				    pip->spooldname = mallocstrw(cp2,sl2) ;

			    }

	                    break ;

	                } /* end switch */

	            } /* end if */

	        } /* end while */

	        field_finish(&fsb) ;

		} /* end if */

	    } /* end if (options) */


/* loop through the UNSETs in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.unsets,i,&cp) >= 0 ; i += 1) {

	        if (cp == NULL) continue ;

	        vecstr_add(&unsets,cp,-1) ;

	        rs = vecstr_finder(&exports,cp,vstrkeycmp,NULL) ;

	        if (rs >= 0)
	            vecstr_del(&exports,rs) ;

	    } /* end for */


/* loop through the EXPORTed variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.exports,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: 1 about to sub> %s\n",cp) ;
#endif

	        sl = varsub_subbuf(&vsdefines,&vsexports,cp,-1,buf,BUFLEN) ;

	        if ((sl > 0) &&
	            ((sl2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 1 about to merge> %W\n",buf,sl2) ;
#endif

	            varsub_merge(&vsdefines,&exports,buf2,sl2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 1 done merging\n") ;
#endif

#if	CF_DEBUG && F_VARSUBDUMP && 0
	            if (pip->debuglevel > 1) {

	                debugprintf("varsub_merge: VSA_D so far \n") ;

	                varsub_dumpfd(&vsdefines,-1) ;

	                debugprintf("varsub_merge: VSA_E so far \n") ;

	                varsub_dump(&vsexports,-1) ;

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


	    varsub_finish(&vsdefines) ;

	    varsub_finish(&vsexports) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF free\n") ;
#endif

	    configfile_finish(&cf) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        debugprintf("main: dumping programroot=%W\n",
	            pip->programroot,strnlen(pip->programroot,50)) ;

	        debugprintf("main: dumping defines\n") ;

	        for (i = 0 ; vecstr_get(&defines,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: define> %s\n",cp) ;

	        debugprintf("main: dumping exports\n") ;

	        for (i = 0 ; vecstr_get(&exports,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: export> %s\n",cp) ;

	    } /* end if (CF_DEBUG) */
#endif /* CF_DEBUG */


	} /* end if (accessed the configuration file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished with any configfile stuff\n") ;
#endif


	if (pip->programroot == NULL)
	    pip->programroot = programroot ;

	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: programroot=%s\n",
	        pip->progname,pip->programroot) ;

/* load up the new program root in the file search schedule variables */

	vecstr_envset(&svars,"p",pip->programroot) ;

/* before we go too far, are we the only one on this PID mutex ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 pidfname=%s\n",pidfname) ;
#endif

	if (pip->f.daemon && (pidfname[0] != '\0')) {

	    if (pidfname[0] == '-')
	        strcpy(pidfname,PIDFNAME) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 1 pidfname=%s\n",pidfname) ;
#endif

	rs = checkfiledir(pip,pidfname) ;

	if (rs < 0)
		goto badpidopen ;

	    rs = bopenroot(&pidfile,pip->programroot,pidfname,tmpfname,
	        "rwc",0664) ;

	    pip->pidfname = pidfname ;
	    if (rs < 0)
	        goto badpidopen ;

	    if (tmpfname[0] != '\0')
	        strcpy(pidfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 pidfname=%s\n",pidfname) ;
#endif

/* capture the lock (if we can) */

	    if ((rs = bcontrol(&pidfile,BC_LOCK,0)) < 0)
	        goto badpidlock ;

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

	    bprintf(&pidfile,"%d\n",pip->pid) ;

	    bclose(&pidfile) ;		/* this releases the lock */

	} /* end if (we have a mutex PID file) */


/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: checking program parameters\n") ;
#endif

	if (pip->workdir == NULL)
	    pip->workdir = WORKDIR ;

	else if (pip->workdir[0] == '\0')
	    pip->workdir = "." ;


	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    if ((pip->tmpdname = getenv("TMPDIR")) == NULL)
	        pip->tmpdname = TMPDIR ;

	} /* end if (tmpdir) */

	if ((pip->spooldname == NULL) || (pip->spooldname[0] == '\0'))
		pip->spooldname = SPOOLDNAME ;

	if (pip->spooldname[0] != '/') {

	    sl = bufprintf(tmpfname,MAXPATHLEN + 1,"%s/%s",
			pip->programroot,pip->spooldname) ;

	    if (sl >= 0)
		pip->spooldname = mallocstrw(tmpfname,sl) ;

	}

	rs = checkspooldir(pip) ;


/* what about the lock file name ? */

	if (lockfname[0] == '\0') {

	    strcpy(lockfname,LOCKFNAME) ;

	}

	if ((sl = getfname(pip->programroot,lockfname,1,tmpfname)) > 0)
	    strwcpy(lockfname,tmpfname,sl) ;

	pip->lockfname = lockfname ;


	if (pip->prog_sendmail == NULL)
	    pip->prog_sendmail = mallocstr(PROG_SENDMAIL) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: sendmail=%s\n",
	        pip->prog_sendmail) ;
#endif

	if (pip->marktime == 0)
	    pip->marktime = TI_MARKTIME ;


/* open the system report log file */

#ifdef	COMMENT
	pip->f.slog = FALSE ;
	if ((rs = bopen(pip->lfp,logfname,"wca",0664)) >= 0) {

	    pip->f.slog = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: system log rs=%d\n",rs) ;
#endif

	    u_time(&daytime) ;

	    bprintf(lfp,"%s: %s %s started\n",
	        pip->progname,timestr_lodaytime,timebuf),
	        BANNER) ;

	    bflush(pip->lfp) ;

	} /* end if (opening log file) */
#endif /* COMMENT */


/* can we access the working directory ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: access working directory \"%s\"\n",pip->workdir) ;
#endif

	if ((perm(pip->workdir,-1,-1,NULL,X_OK) < 0) || 
	    (perm(pip->workdir,-1,-1,NULL,R_OK) < 0))
	    goto badworking ;


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

/* get a serial number for logging purposes */

	cp = SERIALFNAME ;
	rs = SR_OK ;
	if (cp[0] != '/') {

		rs = bufprintf(buf,MAXPATHLEN,"%s/%s",pip->programroot,cp) ;

		if (rs > 0)
			cp = buf ;

	}

	rs = getserial(cp) ;

	pip->serial = (rs >= 0) ? rs : pip->pid ;

	(void) storebuf_dec(buf,BUFLEN,0,pip->serial) ;

	pip->logid = mallocstr(buf) ;


/* does the log file name need massaging ? */

	rs = SR_BAD ;
	if (logfname[0] == '\0') {

	    logfile_type = 1 ;
	    strcpy(logfname,LOGFNAME) ;

	}

	sl = getfname(pip->programroot,logfname,logfile_type,tmpfname) ;

	if (sl > 0)
	    strwcpy(logfname,tmpfname,sl) ;

	rs = logfile_open(&pip->lh,logfname,0,0666,pip->logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs >= 0) {

	    struct utsname	un ;


	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

	    pip->f.log = TRUE ;
	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: logfile=%s\n",pip->progname,logfname) ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
	        loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

	    u_time(&daytime) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    if (pip->f.daemon) {

	        logfile_printf(&pip->lh,"%s %s %s\n",
	            timestr_log(daytime,timebuf),
	            BANNER,
	            "started") ;

	        logfile_printf(&pip->lh,"%-14s %s/%s\n",
	            pip->progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    } else
	        logfile_printf(&pip->lh,"%s %-14s %s/%s\n",
	            timestr_log(daytime,timebuf),
	            pip->progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    (void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) domain=%s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
	        un.sysname,un.release,
	        pip->domainname) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: LF name=%s\n",buf) ;
#endif

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;


	logfile_printf(&pip->lh,"configfile=%s\n",configfname) ;

	logfile_printf(&pip->lh,"programroot=%s\n",pip->programroot) ;


	} /* end if (we have a log file or not) */


/* handle UID/GID stuff */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: continuing with some PID stuff\n") ;
#endif

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


/* find a PASSWD file if we can */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 passwdfname=%s\n",passwdfname) ;
#endif

	rs = SR_NOEXIST ;
	if (passwdfname[0] == '\0') {

	    passwd_type = 1 ;
	    strcpy(passwdfname,PASSWDFNAME) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0a passwdfname=%s\n",passwdfname) ;
#endif

	    if ((sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, passwdfname,R_OK)) < 0) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0b not found\n") ;
#endif

	        sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, passwdfname,R_OK) ;

#if	CF_DEBUG
	if ((pip->debuglevel > 1) && (sl < 0))
	    debugprintf("main: 0c not found\n") ;
#endif

	    } 

	    if (sl > 0)
	        strcpy(passwdfname,tmpfname) ;

	    rs = sl ;

	} else {

	    if (passwd_type < 0)
	        passwd_type = 0 ;

	    sl = getfname(pip->programroot,passwdfname,passwd_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(passwdfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 passwdfname=%s\n",passwdfname) ;
#endif

	if ((rs >= 0) || (perm(passwdfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: passwd=%s\n",pip->progname,passwdfname) ;

	    logfile_printf(&pip->lh,"passwd=%s\n",passwdfname) ;

	    if ((rs = pwfile_open(&passwd,passwdfname)) < 0) {

	        logfile_printf(&pip->lh,"couldn't open passwd file (%d)\n",rs) ;

	        bprintf(efp,"%s: passwd=%s\n",
	            pip->progname,passwdfname) ;

	        bprintf(efp,"%s: couldn't open passwd file (%d)\n",
	            pip->progname,rs) ;

	    } /* end if */

	} /* end if (accessing a 'passwd' file) */

	if (rs < 0) {

	    logfile_printf(&pip->lh,
		"could not open password file (%d) passwd=%s\n",
		rs,passwdfname) ;

		goto badpasswd ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 2) {

	    PWFILE_CUR	cur ;

	    PWENTRY		pe ;

	    char	pwentrybuf[PWENTRY_RECLEN + 1] ;


	    debugprintf("main: pwfile entries> \n") ;

	    pwfile_curbegin(&passwd,&cur) ;

	    while (TRUE) {

	        rs = pwfile_enum(&passwd,&cur,
	            &pe,pwentrybuf,PWENTRY_RECLEN) ;

	        if (rs < 0)
	            break ;

	    debugprintf("main: pwentry->name=%s\n",pe.username) ;
	    debugprintf("main: pwentry->password=%s\n",pe.password) ;

	    } /* end while */

	    pwfile_curend(&passwd,&cur) ;

	} /* end if */
#endif /* CF_DEBUG */


/* load up some environment and execution paths if we have not already */

	if (! f_procfileenv)
	    procfile(pip,procfileenv,pip->programroot,&svars,
	        ENVFNAME,&exports) ;


	if (! f_procfilepaths)
	    procfile(pip,procfilepaths,pip->programroot,&svars,
	        PATHSFNAME,&exports) ;


/* OK, start to put everything together */

	(void) memset(&si,0,sizeof(struct serverinfo)) ;


/* if we are a daemon program, try to bind our INET port ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: about to check if we are a daemon\n") ;
#endif

	if (pip->f.daemon) {

	    in_addr_t	addr ;

	    int	one = 1 ;


#if	CF_DEBUG && 0
	    if (pip->debuglevel > 1) {

	        debugprintf("main: daemon mode\n") ;

	        d_whoopen("2") ;
	    }
#endif /* CF_DEBUG */

/* look up some miscellaneous stuff in various databases */

	    if (portspec == NULL) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: default port \n") ;
#endif

	        sep = getservbyname(PORTNAME, "tcp") ;

	        if (sep != NULL)
	            portspec = PORTNAME ;

	        else
	            portspec = PORTNUM ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: done looking at port stuff so far\n") ;
#endif

	    } /* end if (no port specified) */


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to listen, portspec=%s\n",portspec) ;
#endif

	    rs = listentcp(NULL,portspec,f_reuse) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: listnetcp rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto badlisten ;

	    si.fd_listentcp = rs ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: fd_listentcp=%d\n",si.fd_listentcp) ;
#endif


	    logfile_printf(&pip->lh,"daemon listening on TCP port '%s'\n",
	        portspec) ;


/* background ourselves */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf( "main: become a daemon ?\n") ;
#endif

	    bflush(efp) ;

	    if (pip->debuglevel == 0) {

#ifdef	COMMENT
	        for (i = 0 ; i < 3 ; i += 1) {

	            u_close(i) ;

	            (void) u_open("/dev/null",O_RDONLY,0666) ;

	        } /* end for */
#endif /* COMMENT */

	        rs = uc_fork() ;

	        if (rs < 0) {
	            logfile_printf(&pip->lh,"could not fork daemon (%d)\n",rs) ;
	            uc_exit(EC_ERROR) ;
	        }

	        if (rs > 0)
	            uc_exit(EC_OK) ;

	        u_setsid() ;

	    } /* end if (backgrounding) */

#if	CF_DEBUG && 0
	    if (pip->debuglevel > 1) {

	        debugprintf("main: after daemon backgrounding\n") ;

	        d_whoopen("3") ;

	    }
#endif /* CF_DEBUG */

	} /* end if (daemon mode) */


/* we start ! */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: starting\n") ;
#endif

	pip->pid = getpid() ;

/* reload the userinfo structure with our new PID */

	if (userbuf[0] != '\0')
	    u.pid = pip->pid ;

#ifdef	COMMENT
	(void) storebuf_dec(buf,BUFLEN,0,pip->pid) ;

	logfile_setid(&pip->lh,buf) ;
#endif /* COMMENT */


/* before we go too far, are we the only one on this PID mutex ? */

	if (pip->f.daemon) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: more daemon stuff\n") ;
#endif

	    if (rs == 0)
	        logfile_printf(&pip->lh,"backgrounded pid=%d\n",pip->pid) ;

	    if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: we have a PIDFNAME=%s\n",pip->pidfname) ;
#endif

	        if ((rs = bopen(&pidfile,pip->pidfname,"rwc",0664)) < 0)
	            goto badpidfile1 ;

/* capture the lock (if we can) */

	        if ((rs = bcontrol(&pidfile,BC_LOCK,2)) < 0) {

	            bclose(&pidfile) ;

	            goto badpidfile2 ;
	        }

	        bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	        bseek(&pidfile,0L,SEEK_SET) ;

	        bprintf(&pidfile,"%d\n",pip->pid) ;

	        bprintf(&pidfile,"%s %s\n",
	            BANNER,timestr_log(daytime,timebuf)) ;

	        if (userbuf[0] != '\0')
	            bprintf(&pidfile,"host=%s.%s user=%s pid=%d\n",
	                u.nodename,u.domainname,
	                u.username,
	                pip->pid) ;

	        else
	            bprintf(&pidfile,"host=%s.%s pid=%d\n",
	                u.nodename,u.domainname,
	                pip->pid) ;

	        bflush(&pidfile) ;

/* we leave the file open as our mutex lock ! */

	        logfile_printf(&pip->lh,"pidfile=%s\n",pip->pidfname) ;

	        logfile_printf(&pip->lh,"PID mutex captured\n") ;

	        bcontrol(&pidfile,BC_STAT,&sb) ;

	        logfile_printf(&pip->lh,
	            "pidfile device=\\x%08lx inode=%ld\n",
	            sb.st_dev,sb.st_ino) ;

	        pip->pidfp = &pidfile ;

	    } /* end if (we have a mutex PID file) */

	} /* end if (daemon mode) */


/* find and open the server file */

	rs = SR_NOEXIST ;
	if (srvfname[0] == '\0') {

	    srvtab_type = 1 ;
	    strcpy(srvfname,SRVFNAME) ;

	    if ((sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, srvfname,R_OK)) < 0) {

	        if ((sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, srvfname,R_OK)) > 0)
	            strcpy(srvfname,tmpfname) ;

	    } else if (sl > 0)
	        strcpy(srvfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 0 srvfname=%s\n",srvfname) ;
#endif

	    rs = sl ;

	} else {

	    if (srvtab_type < 0)
	        srvtab_type = 0 ;

	    sl = getfname(pip->programroot,srvfname,srvtab_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(srvfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 rs=%d srvfname=%s\n",rs,srvfname) ;
#endif

	if ((rs >= 0) || (perm(srvfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: srvtab=%s\n",pip->progname,srvfname) ;

	    logfile_printf(&pip->lh,"srvtab=%s\n",srvfname) ;

	    if ((rs = srvtab_open(&sfile,srvfname,NULL)) < 0) {

	        logfile_printf(&pip->lh,"bad (%d) server file\n",rs) ;

	        bprintf(efp,"%s: srvtab=%s\n",
	            pip->progname,srvfname) ;

	        bprintf(efp,"%s: bad (%d) server file\n",
	            pip->progname,rs) ;

	    } /* end if */

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	    debugprintf("main: service entries>\n") ;

	        for (i = 0 ; srvtab_get(&sfile,i,&srvp) >= 0 ; i += 1) {

	            if (srvp == NULL) continue ;

	    debugprintf("main: got an entry i=%d\n",i) ;

	            if (srvp->service != NULL) {

	    debugprintf("main: non-NULL service name\n") ;

	                debugprintf("main: service=%s\n",srvp->service) ;

			}

	        } /* end for */

	    }
#endif /* CF_DEBUG */

	} /* end if (accessing a 'srvtab' file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: SRVTAB rs=%d\n",rs) ;
#endif


/* find and open the Access Table file if we have one */

#if	CF_ACCTAB
	rs = SR_NOEXIST ;
	if (accfname[0] == '\0') {

	    acctab_type = 1 ;
	    strcpy(accfname,ACCFNAME) ;

	    if ((sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, accfname,R_OK)) < 0) {

	        if ((sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, accfname,R_OK)) > 0)
	            strcpy(accfname,tmpfname) ;

	    } else if (sl > 0)
	        strcpy(accfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 0 accfname=%s\n",accfname) ;
#endif

	    rs = sl ;

	} else {

	    if (acctab_type < 0)
	        acctab_type = 0 ;

	    sl = getfname(pip->programroot,accfname,acctab_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(accfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 rs=%d accfname=%s\n",rs,accfname) ;
#endif

	pip->f.acctab = FALSE ;
	if ((rs >= 0) || (perm(accfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (pip->debuglevel > 0)
	        bprintf(efp,"%s: acctab=%s\n",pip->progname,accfname) ;

	    logfile_printf(&pip->lh,"acctab=%s\n",accfname) ;

	    if ((rs = acctab_open(&atab,accfname,NULL)) < 0) {

	        logfile_printf(&pip->lh,"bad (%d) access file\n",rs) ;

	        bprintf(efp,"%s: acctab=%s\n",
	            pip->progname,srvfname) ;

	        bprintf(efp,"%s: bad (%d) access file\n",
	            pip->progname,rs) ;

	    } else
	        pip->f.acctab = TRUE ;

	} /* end if (accessing a 'acctab' file) */

#if	CF_DEBUG
	if ((pip->debuglevel > 1) && pip->f.acctab) {

		ACCTAB_CUR	ac ;

		ACCTAB_ENT	*ep ;


		debugprintf("main: netgroup machine user password\n") ;

		acctab_curbegin(&atab,&ac) ;

		while (acctab_enum(&atab,&ac,&ep) >= 0) {

			if (ep == NULL) continue ;

		debugprintf("main: %-20s %-20s %-8s %-8s\n",
			ep->netgroup.std,ep->machine.std,
			ep->username.std,ep->password.std) ;

		}

		acctab_curend(&atab,&ac) ;

	}
#endif /* CF_DEBUG */

#endif /* CF_ACCTAB */

/* open the built-in servers */

	(void) builtin_init(&bis,pip,&si,&sfile) ;

/* set an environment variable for the program run mode */

#ifdef	COMMENT
	if ((rs = vecstr_finder(&exports,"RUNMODE",vstrkeycmp,&cp)) >= 0)
	    vecstr_del(&exports,rs) ;

	vecstr_add(&exports,"RUNMODE=tcpmux",-1) ;
#endif /* COMMENT */


	if (vecstr_finder(&exports,"HZ",vstrkeycmp,NULL) < 0) {

	    sl = bufprintf(tmpfname,MAXPATHLEN,"HZ=%ld",CLK_TCK) ;

	    vecstr_add(&exports,tmpfname,sl) ;

	}


	if (vecstr_finder(&exports,"PATH",vstrkeycmp,NULL) < 0) {

	    sl = bufprintf(tmpfname,MAXPATHLEN,"PATH=%s",DEFPATH) ;

	    if (sl > 0)
	    	vecstr_add(&exports,tmpfname,sl) ;

	}

/* clean up some stuff we will no longer need */

	vecstr_finish(&svars) ;

/* create the table substitutions for use later */

	varsub_start(&tabsubs,0) ;

/* load up the configuration define variables */

	varsub_addvec(&tabsubs,&defines) ;

/* load up the environment variables */

	varsub_addva(&tabsubs,pip->envv) ;


/* we are done initializing */

	if (pip->f.daemon && (userbuf[0] != '\0')) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: log entry\n") ;
#endif

	    u_time(&daytime) ;

	    logfile_printf(&pip->lh,"%s finished initializing\n",
	        timestr_log(daytime,timebuf)) ;

	} /* end if (making log entries) */


/* fill in some server information that we have so far */

	si.ssp = &tabsubs ;
	si.elp = &exports ;			/* exports */
	si.sfp = &sfile ;
	si.atp = &atab ;
	si.pfp = &passwd ;

	rs = watch(&g,&si,&tabsubs,&exports,&sfile,&atab,&bis) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: watch() rs=%d\n",rs) ;
#endif


	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


/* release the table substitutions */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing tabsubs \n") ;
#endif

	varsub_finish(&tabsubs) ;




/* close the daemon stuff */
daemonret2:
#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing builtin \n") ;
#endif

	(void) builtin_free(&bis) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing acctab \n") ;
#endif

	if (pip->f.acctab)
	    (void) acctab_close(&atab) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing srvtab \n") ;
#endif

	(void) srvtab_close(&sfile) ;


/* close some more (earlier) daemon stuff */
daemonret1:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing defines exports\n") ;
#endif

#ifdef	OPTIONAL
	vecstr_finish(&defines) ;

	vecstr_finish(&exports) ;
#endif /* OPTIONAL */


#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: program finishing, rs=%d\n",
	        pip->progname,rs) ;
#endif

ret5:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing passwd \n") ;
#endif

	pwfile_close(&passwd) ;

ret4:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing logfile \n") ;
#endif

	if (pip->f.log)
	    logfile_close(&pip->lh) ;

ret3:
	if (pip->f.slog)
	    bclose(pip->lfp) ;

ret2:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing configfile\n") ;
#endif

	if (f_freeconfigfname && (configfname != NULL))
	    free(configfname) ;

ret1:
earlyret:
	bclose(efp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting ex=%d\n",ex) ;
#endif

	return ex ;

/* error types of returns */
badret:
	ex = EX_DATAERR ;
	goto earlyret ;

/* USAGE> rexd [-C conf] [-p port] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [{conf|-} [port]] [-C conf] [-p port]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,"%s: \t[-?v] [-D[=n]]\n",
		pip->progname) ;

	goto earlyret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargnot:
	bprintf(efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

badarg:
	goto earlyret ;

badspool:

badlistinit:
	bprintf(efp,"%s: could not initialize list structures (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badsrv:
	bprintf(efp,"%s: bad service table file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badconfig:
	bprintf(efp,
	    "%s: configfile=%s\n",
	    pip->progname,configfname) ;

	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badpidopen:
	ex = EX_MUTEX ;
	bprintf(efp,
	    "%s: could not open the PID file (rs=%d)\n",
	    pip->progname,rs) ;

	bprintf(efp, "%s: pidfile=%s\n", pip->progname,pidfname) ;

	goto earlyret ;

badpidlock:
	ex = EX_MUTEX ;
	if (! pip->f.quiet) {

	    bprintf(efp,
	        "%s: could not lock the PID file (rs=%d)\n",
	        pip->progname,rs) ;

	    bprintf(efp, "%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	    while ((len = breadline(&pidfile,buf,BUFLEN)) > 0) {

	        bprintf(efp,"%s: pidfile> %W",
	            pip->progname,
	            buf,len) ;

	    }

	} /* end if */

	bclose(&pidfile) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	goto earlyret ;

badworking:
	ex = EX_OSFILE ;
	bprintf(efp,"%s: could not access the working directory \"%s\"\n",
	    pip->progname,pip->workdir) ;

	goto ret3 ;

badpasswd:
	ex = EX_OSFILE ;
	bprintf(efp,"%s: could not find a passwd file\n",
	    pip->progname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: no passwd file rs=%d\n",rs) ;
#endif

	goto ret4 ;

badport:
	bprintf(efp,
	    "%s: bad port specified\n",
	    pip->progname) ;

	goto daemonret1 ;

badlisten:
	ex = EX_DATAERR ;
	    bprintf(efp,
	        "%s: could not listen to our server socket (rs=%d)\n",
	        pip->progname,rs) ;

	goto daemonret1 ;

badpidfile1:
	ex = EX_MUTEX ;
	logfile_printf(&pip->lh,
	    "could not open the PID mutex file (rs=%d)\n",
	    rs) ;

	logfile_printf(&pip->lh, "pidfile=%s\n", pip->pidfname) ;

	goto daemonret1 ;

badpidfile2:
	ex = EX_MUTEX ;
	logfile_printf(&pip->lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    pip->pid) ;

	goto daemonret1 ;

}
/* end subroutine (main) */


/* local subroutines */


static int procfile(pip,func,pr,svp,fname,elp)
struct proginfo	*pip ;
int		(*func)(char *,char *,VECSTR *) ;
char		pr[] ;
vecstr		*svp ;
char		fname[] ;
VECSTR		*elp ;
{
	int	sl ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procfile: 1 fname=%s\n",fname) ;
#endif

	if ((sl = permsched(sched2,svp,
	    tmpfname,MAXPATHLEN, fname,R_OK)) < 0) {

	    if ((sl = permsched(sched3,svp,
	        tmpfname,MAXPATHLEN, fname,R_OK)) > 0)
	        fname = tmpfname ;

	} else if (sl > 0)
	    fname = tmpfname ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("procfile: 2 fname=%s\n",fname) ;
#endif

	if (sl >= 0)
	    return (*func)(pr,fname,elp) ;

	return SR_NOEXIST ;
}
/* end subroutine (procfile) */


/* check if the spool directory exists */
static int checkspooldir(pip)
struct proginfo	*pip ;
{
	struct ustat	sb ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("checkspooldir: entered\n") ;
#endif

	if ((pip->spooldname == NULL) || (pip->spooldname[0] == '\0'))
		return SR_INVALID ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("checkspooldir: spooldir=%s\n",
			pip->spooldname) ;
#endif

	    rs = u_stat(pip->spooldname,&sb) ;

	if (rs < 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("checkspooldir: creating spooldir, stat() rs=%d\n",
			rs) ;
#endif

	        rs = mkdirs(pip->spooldname,0775) ;

	        if (rs < 0) {

	            logfile_printf(&pip->lh,
	                "could not create spool directory (rs %d)\n",rs) ;

	            bprintf(pip->efp,
	                "%s: could not create spool directory (rs %d)\n",
	                pip->progname,rs) ;

	        }

	    } else if (! S_ISDIR(sb.st_mode))
		rs = SR_NOTDIR ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("checkspooldir: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkspooldir) */


/* check if the directory of a file is present */
static int checkfiledir(pip,fname)
struct proginfo	*pip ;
char		fname[] ;
{
	struct ustat	sb ;

	int	rs, sl ;

	char	dname[MAXPATHLEN + 2] ;
	char	*dn ;


	if ((fname == NULL) || (fname[0] == '\0'))
		return SR_INVALID ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("checkfiledir: fname=%s\n",
			fname) ;
#endif

	sl = sfdirname(fname,-1,&dn) ;

	if (sl <= 0)
		return SR_OK ;

	strwcpy(dname,dn,sl) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("checkfiledir: dname=%s\n",
			dname) ;
#endif

	    rs = u_stat(dname,&sb) ;

	if (rs < 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("checkfiledir: creating spooldir, stat() rs=%d\n",
			rs) ;
#endif

	        rs = mkdirs(dname,0775) ;

	        if (rs < 0) {

	            logfile_printf(&pip->lh,
	                "could not create directory (rs %d)\n",rs) ;

	            bprintf(pip->efp,
	                "%s: could not create directory (rs %d)\n",
	                pip->progname,rs) ;

	        }

	    } else if (! S_ISDIR(sb.st_mode))
		rs = SR_NOTDIR ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	        debugprintf("checkfiledir: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (checkfiledir) */



