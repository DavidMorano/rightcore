/* main */

/* generic (pretty much) front end program subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0


/* revision history:

	= 1994-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

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



*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<varsub.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<userinfo.h>
#include	<mallocstuff.h>
#include	<srvtab.h>
#include	<localmisc.h>

#include	"builtin.h"
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


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	bopenroot(bfile *,char *,char *,char *,char *,int) ;
extern int	var_load(), var_subbuf(), var_merge() ;
extern int	expander() ;
extern int	getportnum() ;
extern int	procfileenv(char *,char *,VECSTR *) ;
extern int	procfilepaths(char *,char *,VECSTR *) ;
extern int	watch(struct global *,
			int,varsub *,vecstr *,SRVTAB *,BUILTIN *) ;
extern int	watchone(struct global *,
			int,varsub *,vecstr *,SRVTAB *,BUILTIN *) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log() ;

#if	CF_DEBUG
extern void	whoopen() ;
#endif


/* externals variables */


/* forward references */

static int	procfile(struct global *,int (*)(char *,char *,VECSTR *),
			const char *,vecstr *,char *,VECSTR *) ;


/* local structures */

static const char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	"CONFIG",
	NULL,
} ;


#define	ARGOPT_TMPDIR	0
#define	ARGOPT_VERSION	1
#define	ARGOPT_VERBOSE	2
#define	ARGOPT_ROOT	3
#define	ARGOPT_LOGFILE	4
#define	ARGOPT_CONFIG	5



/* local variables */

/* 'conf' for most regular programs */
static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL,
} ;

/* non-'conf' ETC stuff for all regular programs */
static const char	*sched2[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	"%p/%n.%f",
	NULL,
} ;

/* 'conf' and non-'conf' ETC stuff for local searching */
static const char	*sched3[] = {
	"%e/%n/%n.%f",
	"%e/%n/%f",
	"%e/%n.%f",
	"%e/%f",
	"%n.%f",
	"%f",
	NULL,
} ;








int main(argc,argv,envv)
int	argc ;
char	*argv[], *envv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		logfile ;
	bfile		pidfile ;

	struct ustat		sb ;

	struct global		*gp = &g ;

	struct userinfo		u ;

	struct configfile	cf ;

	struct sockaddr_in	sa_server ;

	struct sockaddr_in	sa_from ;

	struct servent	se, *sep ;

	struct protoent	pe, *pep ;

	struct hostent	he, *hep ;

	struct group	ge ;

	VECSTR		defines, unsets, exports ;
	VECSTR		svars ;

	varsub		srvsubs ;
	varsub		vsh_e, vsh_d ;

	BUILTIN		bis ;

	SRVTAB		sfile ;

	SRVTAB_ENT	*srvp ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j, k ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	len, srs, rs ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	s, proto ;
	int	port = -1 ;
	int	loglen = -1 ;
	int	l2, sl ;
	int	logfile_type = -1 ;
	int	srvtab_type = -1 ;
	int	f_programroot = FALSE ;
	int	f_freeconfigfname = FALSE ;
	int	f_procfileenv = FALSE ;
	int	f_procfilepaths = FALSE ;
	int	err_fd ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*programroot = NULL ;
	char	buf[BUFLEN + 1] ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	srvfname[MAXPATHLEN + 1] ;
	char	pidfname[MAXPATHLEN + 1] ;
	char	lockfname[MAXPATHLEN + 1] ;
	char	logfname[MAXPATHLEN + 1] ;
	char	pwd[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*pr = NULL ;
	char	*configfname = NULL ;
	char	*portspec = NULL ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


/* we want to open up some files so that the first few FD slots are FULL !! */

	if (u_fstat(FD_STDIN,&sb) < 0)
	    (void) u_open("/dev/null",O_RDONLY,0666) ;

	g.f.fd_stdout = TRUE ;
	if (u_fstat(FD_STDOUT,&sb) < 0) {

	    g.f.fd_stdout = FALSE ;
	    (void) u_open("/dev/null",O_RDONLY,0666) ;

	}

	g.f.fd_stderr = TRUE ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) < 0) {

	    g.f.fd_stderr = FALSE ;
	    (void) u_open("/dev/null",O_WRONLY,0666) ;

	} else
	    bcontrol(efp,BC_LINEBUF,0) ;

#ifdef	COMMENT
	g.efp = efp ;
#endif
	g.envv = envv ;
	g.version = VERSION ;
	g.progname = strbasename(argv[0]) ;

	g.pid = getpid() ;

	g.ppid = g.pid ;

/* initialize */

	g.f.quiet = FALSE ;
	g.f.verbose = FALSE ;
	g.f.log = FALSE ;
	g.f.daemon = FALSE ;

	g.debuglevel = 0 ;

	g.lfp = &logfile ;

	port = -1 ;

	g.programroot = NULL ;
	g.username = NULL ;
	g.groupname = NULL ;
	g.pidfname = NULL ;
	g.lockfname = NULL ;
	g.tmpdir = NULL ;
	g.defuser = NULL ;
	g.defgroup = NULL ;
	g.userpass = NULL ;
	g.machpass = NULL ;
	g.prog_sendmail = NULL ;


	pidfname[0] = '\0' ;
	lockfname[0] = '\0' ;
	logfname[0] = '\0' ;
	srvfname[0] = '\0' ;


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
	                    (cfdeci(argp + 1,argl - 1,&port) < 0))
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

	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.tmpdir = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.tmpdir = argp ;

	                        }

	                        break ;

/* version */
	                    case ARGOPT_VERSION:
#if	CF_DEBUGS
	                        debugprintf("main: version key-word\n") ;
#endif
	                        f_version = TRUE ;
	                        if (f_optequal) goto badargextra ;

	                        break ;

/* verbose mode */
	                    case ARGOPT_VERBOSE:
	                        g.f.verbose = TRUE ;
	                        break ;

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.programroot = argp ;

	                        }

	                        break ;

/* configuration file */
	                    case ARGOPT_CONFIG:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) configfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) configfname = argp ;

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

	                            if (argr <= 0) goto badargnum ;

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
	                        bprintf(efp,"%s: option (%s) not supported\n",
	                            g.progname,akp) ;

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
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

#if	CF_DEBUGS
	                                debugprintf(
	                                    "main: debug flag, avp=\"%W\"\n",
	                                    avp,avl) ;
#endif

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
	                                    (cfdec(avp,avl,&g.debuglevel) < 0))
	                                    goto badargvalue ;

	                            }

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) configfname = argp ;

	                            break ;

/* mutex lock PID file */
	                        case 'P':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                strwcpy(pidfname,argp,argl) ;

	                            break ;

/* daemon mode */
	                        case 'd':
	                            g.f.daemon = TRUE ;
	                            break ;

/* TCP port to listen on */
	                        case 'p':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                portspec = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            g.f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            g.f.verbose = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                g.progname,*aop) ;

	                        case '?':
	                            f_usage = TRUE ;

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
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (g.debuglevel > 1) bprintf(efp,
	    "%s: finished parsing arguments\n",
	    g.progname) ;

	if (f_version) bprintf(efp,"%s: version %s\n",
	    g.progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

	if (g.debuglevel > 1)
	    bprintf(efp,"%s: debug level %d\n",
	        g.progname,g.debuglevel) ;


/* load the positional arguments */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "main: got a positional argument i=%d pan=%d arg=%s\n",
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
	            bprintf(efp,
	                "%s: extra positional arguments ignored\n",
	                g.progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */


/* miscellaneous */

#if	CF_DEBUG
	if (g.debuglevel > 1) {

	    debugprintf( "main: starting FDs\n") ;

	    whoopen() ;

	}
#endif


/* get our program root */

	if (g.programroot == NULL) {

	    programroot = getenv(VARPROGRAMROOT1) ;

	    if (programroot == NULL)
	        programroot = getenv(VARPROGRAMROOT2) ;

	    if (programroot == NULL)
	        programroot = getenv(VARPROGRAMROOT3) ;

	    if (programroot == NULL)
	        programroot = PROGRAMROOT ;

	    g.programroot = programroot ;

	} else {

	    f_programroot = TRUE ;
	    programroot = g.programroot ;

	}

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	g.nodename = u.nodename ;
	g.domainname = u.domainname ;
	g.username = u.username ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: UI name=%s\n",u.name) ;
#endif

	if (rs < 0) {

	    getnodedomain(nodename,domainname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: got node/domain\n") ;
#endif

	    g.nodename = nodename ;
	    g.domainname = domainname ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: about to get username\n") ;
#endif

	    getusername(buf,USERNAMELEN,-1) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: got username\n") ;
#endif

	    g.username = malloc_str(buf) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: done w/ name stuff\n") ;
#endif

	} /* end if (got some user information or not) */


/* PWD */

	(void) getpwd(buf,BUFLEN) ;

	g.pwd = malloc_str(buf) ;


/* prepare to store configuration variable lists */

	if ((rs = vecstr_start(&defines,10,VSP_ORDERED)) < 0)
	    goto badlistinit ;

	if ((rs = vecstr_start(&unsets,10,VSP_NOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    goto badlistinit ;
	}

	if ((rs = vecstr_start(&exports,10,VSP_NOHOLES)) < 0) {

	    vecstr_finish(&defines) ;

	    vecstr_finish(&unsets) ;

	    goto badlistinit ;
	}

/* create the values for the file schedule searching */

	vecstr_start(&svars,6,0) ;

	vecstr_envset(&svars,"p",g.programroot,-1) ;

	vecstr_envset(&svars,"e","etc",-1) ;

	vecstr_envset(&svars,"n",SEARCHNAME,-1) ;

/* load up some initial environment that everyone should have ! */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to do INITFILE=%s\n",INITFILE) ;
#endif

	procfileenv(g.programroot,INITFILE,&exports) ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (g.debuglevel > 1) {

	    debugprintf("main: checking for configuration file\n") ;

	    debugprintf("main: 0 CF=%s\n",configfname) ;

	}
#endif /* CF_DEBUG */

	rs = SR_NOEXIST ;
	if ((configfname == NULL) || (configfname[0] == '\0')) {

		configfname = CONFIGFILE ;

	        if ((sl = permsched(sched1,&svars,
	            tmpfname,MAXPATHLEN, configfname,R_OK)) < 0) {

	            if ((sl = permsched(sched3,&svars,
	                tmpfname,MAXPATHLEN, configfname,R_OK)) > 0)
	    		configfname = tmpfname ;

	        } else if (sl > 0)
	    		configfname = tmpfname ;

		rs = sl ;

	} else {

	    sl = getfname(g.programroot,configfname,1,tmpfname) ;

	    if (sl > 0)
	        configfname = tmpfname ;

		rs = sl ;		/* cause an open failure later */

	} /* end if */

	if (configfname == tmpfname) {

	    f_freeconfigfname = TRUE ;
	    configfname = malloc_str(tmpfname) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 1) {

	    debugprintf("main: find rs=%d\n",sl) ;

	    debugprintf("main: 1 CF=%s\n",configfname) ;

	}
#endif /* CF_DEBUG */

/* read in the configuration file if we have one */

	if ((rs >= 0) || (perm(configfname,-1,-1,NULL,R_OK) >= 0)) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: configuration file \"%s\"\n",
	            configfname) ;
#endif

	    if (g.f.verbose || (g.debuglevel > 0))
	        bprintf(efp,"%s: configfile=%s\n",
	            g.progname,configfname) ;

	    if ((rs = configinit(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: varsub_start d\n") ;
#endif

/* we must set this mode to 'VARSUBM_BADNOKEY' so that a miss is noticed */

	    varsub_start(&vsh_d,VARSUBM_BADNOKEY) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: varsub_start e\n") ;
#endif

	    varsub_start(&vsh_e,0) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
		debugprintf("main: var_loadenv\n") ;
#endif

	    var_load(&vsh_e,envv) ;

#if	CF_DEBUG && F_VARSUBDUMP
	    if (g.debuglevel > 1) {

	        debugprintf(
	            "main: 0 for\n") ;

	        varsubdump(&vsh_e) ;

	    }
#endif /* CF_DEBUG */


/* program root from configuration file */

	    if ((cf.root != NULL) && (! f_programroot)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processing CF programroot\n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            g.programroot = malloc_strn(buf2,l2) ;

	        }

	    } /* end if (configuration file program root) */


/* loop through the DEFINEd variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: 0 top, cp=%s\n",cp) ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 0 about to merge\n") ;
#endif

	            var_merge(&vsh_d,&defines,buf2,l2) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 0 out of merge\n") ;
#endif

	        } /* end if */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: 0 bot\n") ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: done w/ defines\n") ;
#endif


/* all of the rest of the configuration file stuff */

	    if ((cf.workdir != NULL) && (g.workdir == NULL)) {

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.workdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            g.workdir = malloc_strn(buf2,l2) ;

	        }

	    } /* end if (config file working directory) */


	    if (g.f.daemon && (cf.pidfile != NULL) && 
	        (pidfname[0] == '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF pidfile=%s\n",cf.pidfile) ;
#endif

	        if ((cf.pidfile[0] != '\0') && (cf.pidfile[0] != '-')) {

	            if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.pidfile,
	                -1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(&g,buf,sl, buf2,BUFLEN)) > 0)) {

	                strwcpy(pidfname,buf2,l2) ;

	            }

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: pidfname=%s\n",pidfname) ;
#endif

	    } /* end if (configuration file PIDFILE) */


	    if ((cf.lockfile != NULL) && (lockfname[0] == '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF lockfile=%s\n",cf.lockfile) ;
#endif

	        if ((cf.lockfile[0] != '\0') && (cf.lockfile[0] != '-')) {

	            if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.lockfile,
	                -1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(&g,buf,sl, buf2,BUFLEN)) > 0)) {

	                strwcpy(lockfname,buf2,l2) ;

	            }

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: lockfile=%s\n",lockfname) ;
#endif

	    } /* end if (configuration file LOCKFILE) */


#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("main: so far logfname=%s\n",logfname) ;

	        debugprintf("main: about to get config log filename\n") ;
	    }
#endif

	    if ((cf.logfname != NULL) && (logfile_type < 0)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(logfname,buf2,l2) ;

	            if (strchr(logfname,'/') != NULL)
	                logfile_type = 1 ;

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",logfname) ;
#endif

	    } /* end if (configuration file log filename) */

	    if ((cf.port != NULL) && (port < 0) && (portspec == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processing port\n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.port,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            if (isalpha(buf2[0])) {

	                sep = getservbyname(buf2, "tcp") ;

	                port = -1 ;
	                if (sep != NULL)
	                    port = (int) ntohs(sep->s_port) ;

	            } else if (cfdeci(buf2,l2,&port) < 0)
	                port = -1 ;

	        }

	    } /* end if (handling the configuration file port parameter) */

	    if ((cf.user != NULL) && (g.defuser == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF user \n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.user,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            g.defuser = malloc_strn(buf2,l2) ;

	        }

	    }

	    if ((cf.group != NULL) && (g.defgroup == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF group \n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.group,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            g.defgroup = malloc_strn(buf2,l2) ;

	        }

	    } /* end if */

	    if ((cf.userpass != NULL) && (g.userpass == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processing CF user password\n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.userpass,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            g.userpass = malloc_strn(buf2,l2) ;

	        }

	    }

	    if ((cf.machpass != NULL) && (g.machpass == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processing CF machine password\n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.machpass,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            g.machpass = malloc_strn(buf2,l2) ;

	        }

	    } /* end if */

	    if ((cf.srvtab != NULL) && (srvfname[0] == '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF processing SRVTAB \n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.srvtab,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

	            strwcpy(srvfname,buf2,l2) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: CF SRVTAB s&e srvtab=%s\n",
				srvfname) ;
#endif

	            srvtab_type = 0 ;
	            if (strchr(srvfname,'/') != NULL)
	                srvtab_type = 1 ;

	        }

	    } /* end if (srvtab) */

	    if ((cf.sendmail != NULL) && (g.prog_sendmail == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processing CF sendmail\n") ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cf.sendmail,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            g.prog_sendmail = malloc_strn(buf2,l2) ;

	        }

	    } /* end if (sendmail) */


/* what about an 'environ' file ? */

	    if (cf.envfile != NULL) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: 1 envfile=%s\n",cf.envfile) ;
#endif

	        procfileenv(g.programroot,cf.envfile,&exports) ;

	    } else
	        procfile(gp,procfileenv,g.programroot,&svars,
	            ENVFILE,&exports) ;

	    f_procfileenv = TRUE ;

/* "do" any 'paths' file before we process the environment variables */

	    if (cf.pathsfile != NULL) {
	        procfilepaths(g.programroot,cf.pathsfile,&exports) ;
	    } else
	        procfile(gp,procfilepaths,g.programroot,&svars,
	            PATHSFILE,&exports) ;

	    f_procfilepaths = TRUE ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

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
	        if (g.debuglevel > 1)
			debugprintf("main: 1 about to sub> %s\n",cp) ;
#endif

	        if (((sl = var_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,sl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
			debugprintf("main: 1 about to merge> %W\n",buf,l2) ;
#endif

	            var_merge(NULL,&exports,buf2,l2) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 1 done merging\n") ;
#endif

#if	CF_DEBUG && F_VARSUBDUMP && 0
	            if (g.debuglevel > 1) {
	                debugprintf("var_merge: VSA_D so far \n") ;
	                varsubdump(&vsh_d) ;
	                debugprintf("var_merge: VSA_E so far \n") ;
	                varsubdump(&vsh_e) ;
	            } /* end if (debuglevel) */
#endif /* CF_DEBUG */

	        } /* end if (merging) */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: done subbing & merging\n") ;
#endif

	    } /* end for (exports) */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: done w/ exports\n") ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: processing CF freeing data structures\n") ;
#endif

#ifdef	COMMENT
	    var_free(&vsh_e) ;		/* no such thing !! */
#endif /* COMMENT */

	    varsub_finish(&vsh_d) ;

	    varsub_finish(&vsh_e) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: processing CF free\n") ;
#endif

	    configfree(&cf) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {
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
	if (g.debuglevel > 1) debugprintf(
	    "main: finished with any configfile stuff\n") ;
#endif

	if (g.programroot == NULL)
	    g.programroot = programroot ;

	if (g.f.verbose || (g.debuglevel > 0))
	    bprintf(efp,"%s: programroot=%s\n",g.progname,g.programroot) ;

/* load up the new program root in the file search schedule variables */

	vecstr_envset(&svars,"p",g.programroot,-1) ;

/* load up some environment and execution paths if we have not already */

	if (! f_procfileenv)
	    procfile(gp,procfileenv,g.programroot,&svars,
	        ENVFILE,&exports) ;


	if (! f_procfilepaths)
	    procfile(gp,procfilepaths,g.programroot,&svars,
	        PATHSFILE,&exports) ;


/* before we go too far, are we the only one on this PID mutex ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 0 pidfname=%s\n",pidfname) ;
#endif

	if (g.f.daemon && (pidfname[0] != '\0')) {

	    if (pidfname[0] == '-')
	        strcpy(pidfname,PIDFILE) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: 1 pidfname=%s\n",pidfname) ;
#endif

	    rs = bopenroot(&pidfile,g.programroot,pidfname,tmpfname,
	        "rwc",0664) ;

	        g.pidfname = pidfname ;
		if (rs < 0) goto badpidopen ;

	        if (tmpfname[0] != '\0')
	            strcpy(pidfname,tmpfname) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: 2 pidfname=%s\n",pidfname) ;
#endif

/* capture the lock (if we can) */

	        if ((rs = bcontrol(&pidfile,BC_LOCK,0)) < 0)
	            goto badpidlock ;

	        bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	        bseek(&pidfile,0L,SEEK_SET) ;

	        bprintf(&pidfile,"%d\n",g.pid) ;

	        bclose(&pidfile) ;		/* this releases the lock */

	} /* end if (we have a mutex PID file) */


/* check program parameters */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking program parameters\n") ;
#endif

	if (g.workdir == NULL)
	    g.workdir = WORKDIR ;

	else if (g.workdir[0] == '\0')
	    g.workdir = "." ;


	if ((g.tmpdir == NULL) || (g.tmpdir[0] == '\0')) {

	    if ((g.tmpdir = getenv("TMPDIR")) == NULL)
	        g.tmpdir = TMPDIR ;

	} /* end if (tmpdir) */


	if (lockfname[0] == '\0') {

	    strcpy(lockfname,LOCKFILE) ;

	}

	if ((sl = getfname(g.programroot,lockfname,1,tmpfname)) > 0)
	    strwcpy(lockfname,tmpfname,sl) ;

	g.lockfname = lockfname ;


	if (g.prog_sendmail == NULL)
	    g.prog_sendmail = malloc_str(PROG_SENDMAIL) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: sendmail=%s\n",
	        g.prog_sendmail) ;
#endif


/* open the system report log file */

#ifdef	COMMENT
	g.f.log = FALSE ;
	if ((rs = bopen(g.lfp,logfname,"wca",0664)) >= 0) {

	    g.f.log = TRUE ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: system log rs=%d\n",rs) ;
#endif

	    (void) time(&daytime) ;

	    bprintf(lfp,"%s: %s %s started\n",
	        g.progname,timestr_lodaytime,timebuf),
	        BANNER) ;

	    bflush(g.lfp) ;

	} /* end if (opening log file) */
#endif /* COMMENT */


/* can we access the working directory ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: access working directory \"%s\"\n",g.workdir) ;
#endif

	if ((perm(g.workdir,-1,-1,NULL,X_OK) < 0) || 
	    (perm(g.workdir,-1,-1,NULL,R_OK) < 0))
	    goto badworking ;


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	(void) storebuf_dec(buf,BUFLEN,0,g.pid) ;

	gp->logid = malloc_str(buf) ;

	rs = SR_BAD ;
	if (logfname[0] == '\0') {

	    logfile_type = 1 ;
	    strcpy(logfname,LOGFNAME) ;

	}

	if ((sl = getfname(g.programroot,logfname,logfile_type,tmpfname)) > 0)
	    strwcpy(logfname,tmpfname,sl) ;

	rs = logfile_open(&gp->lh,logfname,0,0666,gp->logid) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

		g.f.log = TRUE ;
	    if (g.f.verbose || (g.debuglevel > 0))
	        bprintf(efp,"%s: logfile=%s\n",gp->progname,logfname) ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0)
		loglen = LOGSIZE ;

	    logfile_checksize(&g.lh,loglen) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

	    (void) time(&daytime) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    if (g.f.daemon) {

	        logfile_printf(&g.lh,"%s %s %s\n",
	            timestr_log(daytime,timebuf),
	            BANNER,
	            "started") ;

	        logfile_printf(&g.lh,"%-14s %s/%s\n",
	            g.progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	    } else
	        logfile_printf(&g.lh,"%s %-14s %s/%s\n",
	            timestr_log(daytime,timebuf),
	            g.progname,
	            VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;


	    logfile_printf(&g.lh,"os=%s domain=%s\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),g.domainname) ;


	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: LF name=%s\n",buf) ;
#endif

	    logfile_printf(&g.lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	} /* end if (we have a log file or not) */


/* handle UID/GID stuff */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: continuing with some PID stuff\n") ;
#endif

	g.uid = u.uid ;
	g.euid = geteuid() ;

	g.gid = u.gid ;
	g.egid = getegid() ;

	rs = getgr_gid(&ge,buf,BUFLEN,g.gid) ;

	if (rs < 0) {

	    cp = buf ;
	    bufprintf(buf,BUFLEN,"GID%d",(int) g.gid) ;

	} else
	    cp = ge.gr_name ;

	g.groupname = malloc_str(cp) ;


/* if we are a daemon program, try to bind our INET port ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to check if we are a daemon\n") ;
#endif

	if (g.f.daemon) {

	in_addr_t	addr ;

	    int	one = 1 ;


#if	CF_DEBUG && 0
	    if (g.debuglevel > 1) {

	        debugprintf("main: daemon mode\n") ;

	        whoopen() ;
	    }
#endif /* CF_DEBUG */

/* look up some miscellaneous stuff in various databases */

	    if (portspec != NULL) {

	        if (getportnum(portspec,&port) < 0)
	            goto badport ;

	    }

	    if (port < 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: default port \n") ;
#endif

	        sep = getservbyname(PORTNAME, "tcp") ;

	        if (sep != NULL)
	            port = (int) ntohs(sep->s_port) ;

	        else
	            port = PORT ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: done looking at port stuff so far\n") ;
#endif

	    } /* end if (no port specified) */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: about to get a protocol number\n") ;
#endif


/* get the protocol number */

	    pep = getprotobyname("tcp") ;

	    if (pep != NULL)
	        proto = pep->p_proto ;

	    else
	        proto = IPPROTO_TCP ;


#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: about to call 'socket'\n") ;
#endif

	    if ((s = u_socket(PF_INET, SOCK_STREAM, proto)) < 0)
	        goto badsocket ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: socket for daemon, s=%d\n",s) ;
#endif

	    (void) u_setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
	        (char *) &one, sizeof(int)) ;

	    (void) memset((char *) &sa_server, 0, sizeof(struct sockaddr)) ;

	    sa_server.sin_family = AF_INET ;
	    sa_server.sin_port = htons(port) ;
	    addr = htonl(INADDR_ANY) ;

	    (void) memcpy((char *) &sa_server.sin_addr, &addr, sizeof(long)) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: about to bind to port=%d\n",
	            port) ;
#endif

	    if ((rs = u_bind(s, (struct sockaddr *) &sa_server, 
	        sizeof(struct sockaddr))) < 0) {

	        u_close(s) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: bind rs=%d\n", rs) ;
#endif

	        srs = rs ;
	        goto badbind ;
	    }

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: about to listen\n") ;
#endif

	    if ((rs = u_listen(s,10)) < 0) {

	        u_close(s) ;

	        goto badlisten ;
	    }

	    logfile_printf(&gp->lh,"configfile=%s\n",configfname) ;

	    logfile_printf(&g.lh,"daemon listening on TCP port %d\n",port) ;


/* background ourselves */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf( "main: become a daemon ?\n") ;
#endif

	    bflush(efp) ;

	    if (g.debuglevel == 0) {

#ifdef	COMMENT
	        for (i = 0 ; i < 3 ; i += 1) {

	            u_close(i) ;

	            (void) u_open("/dev/null",O_RDONLY,0666) ;

	        } /* end for */
#endif /* COMMENT */

	        rs = uc_fork() ;

	        if (rs < 0) {
	            logfile_printf(&g.lh,"cannot fork daemon (%d)\n",rs) ;
	            u_exit(BAD) ;
	        }

	        if (rs > 0) u_exit(OK) ;

	        setsid() ;

	    } /* end if (backgrounding) */

#if	CF_DEBUG && 0
	    if (g.debuglevel > 1) {

	        debugprintf("main: after daemon backgrounding\n") ;

	        whoopen() ;

	    }
#endif /* CF_DEBUG */

	} else
	    s = FD_STDIN ;


/* we start ! */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: starting, s=%d\n",s) ;
#endif

	g.pid = getpid() ;

/* reload the userinfo structure with our new PID */

	if (userbuf[0] != '\0')
	    u.pid = g.pid ;

#ifdef	COMMENT
	(void) storebuf_dec(buf,BUFLEN,0,g.pid) ;

	logfile_setid(&g.lh,buf) ;
#endif /* COMMENT */


/* before we go too far, are we the only one on this PID mutex ? */

	if (g.f.daemon) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: more daemon stuff\n") ;
#endif

	    if (rs == 0)
	        logfile_printf(&g.lh,"backgrounded pid=%d\n",gp->pid) ;

	    if ((g.pidfname != NULL) && (g.pidfname[0] != '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: we have a PIDFILE=%s\n",g.pidfname) ;
#endif

	        if ((srs = bopen(&pidfile,g.pidfname,"rwc",0664)) < 0)
	            goto badpidfile1 ;

/* capture the lock (if we can) */

	        if ((srs = bcontrol(&pidfile,BC_LOCK,2)) < 0) {

	            bclose(&pidfile) ;

	            goto badpidfile2 ;
	        }

	        bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	        bseek(&pidfile,0L,SEEK_SET) ;

	        bprintf(&pidfile,"%d\n",g.pid) ;

	        bprintf(&pidfile,"%s %s\n",
	            BANNER,timestr_log(daytime,timebuf)) ;

	        if (userbuf[0] != '\0')
	            bprintf(&pidfile,"host=%s.%s user=%s pid=%d\n",
	                u.nodename,u.domainname,
	                u.username,
	                g.pid) ;

	        else
	            bprintf(&pidfile,"host=%s.%s pid=%d\n",
	                u.nodename,u.domainname,
	                g.pid) ;

	        bflush(&pidfile) ;

/* we leave the file open as our mutex lock ! */

	        logfile_printf(&g.lh,"pidfile=%s\n",g.pidfname) ;

	        logfile_printf(&g.lh,"PID mutex captured\n") ;

	        bcontrol(&pidfile,BC_STAT,&sb) ;

	        logfile_printf(&g.lh,"pidfile device=%ld inode=%ld\n",
	            sb.st_dev,sb.st_ino) ;

	        g.pidfp = &pidfile ;

	    } /* end if (we have a mutex PID file) */

	} /* end if (daemon mode) */


/* find and open the server file */

	rs = SR_NOEXIST ;
	if (srvfname[0] == '\0') {

	    srvtab_type = 1 ;
	    strcpy(srvfname,SRVFILE) ;

	    if ((sl = permsched(sched2,&svars,
	        tmpfname,MAXPATHLEN, srvfname,R_OK)) < 0) {

	        if ((sl = permsched(sched3,&svars,
	            tmpfname,MAXPATHLEN, srvfname,R_OK)) > 0)
	    		strcpy(srvfname,tmpfname) ;

	    } else if (sl > 0)
	    	 strcpy(srvfname,tmpfname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: 0 srvfname=%s\n",srvfname) ;
#endif

	    rs = sl ;

	} else {

	    if (srvtab_type < 0)
	        srvtab_type = 0 ;

	    sl = getfname(g.programroot,srvfname,srvtab_type,tmpfname) ;

	    if (sl > 0)
	        strwcpy(srvfname,tmpfname,sl) ;

	} /* end if */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 1 srvfname=%s\n",srvfname) ;
#endif

	if ((rs >= 0) || (perm(srvfname,-1,-1,NULL,R_OK) >= 0)) {

	    if (g.f.verbose || (g.debuglevel > 0))
	        bprintf(efp,"%s: srvtab=%s\n",g.progname,srvfname) ;

	    logfile_printf(&g.lh,"srvtab=%s\n",srvfname) ;

	    if ((rs = srvtab_open(&sfile,srvfname,NULL)) < 0) {

	        logfile_printf(&g.lh,"bad (%d) server file\n",rs) ;

	        bprintf(efp,"%s: srvfile=%s\n",
	            g.progname,srvfname) ;

	        bprintf(efp,"%s: bad (%d) server file\n",
	            g.progname,rs) ;

	    } /* end if */

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        for (i = 0 ; srvtab_enum(&sfile,i,&srvp) >= 0 ; i += 1) {

	            if (srvp == NULL) continue ;

	            if (srvp->service != NULL)
	                debugprintf("main: service=%s\n",srvp->service) ;

	        } /* end for */

	    }
#endif /* CF_DEBUG */

	} /* end if (accessing a 'srvtab' file) */


/* open the built-in servers */

	(void) builtin_init(&bis,&sfile) ;


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

		vecstr_add(&exports,tmpfname,sl) ;

	}

/* clean up some stuff we will no longer need */

	vecstr_finish(&svars) ;

/* create the srver table substitutions for use later */

	varsub_start(&srvsubs,0) ;

/* load up the configuration define variables */

	var_loadvec(&srvsubs,&defines) ;

/* load up the environment variables */

	var_loadenv(&srvsubs,gp->envv) ;


/* we are done initializing */

	if (userbuf[0] != '\0') {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: log entry\n") ;
#endif

	    (void) time(&daytime) ;

	    if (g.f.daemon)
	        logfile_printf(&g.lh,"%s finished initializing\n",
	            timestr_log(daytime,timebuf)) ;

	} /* end if (making log entries) */


	if (g.f.daemon) {

#if	CF_DEBUG && 0
	    if (g.debuglevel > 1) {

	        debugprintf(
	            "main: starting to watch for new jobs, s=%d\n",s) ;

	        whoopen() ;
	    }
#endif /* CF_DEBUG */

	    srs = watch(&g,s,&srvsubs,&exports,&sfile,&bis) ;

	} else {


#ifdef	COMMENT
#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: are we a socket on STDIN ?\n") ;
#endif

	    if (! isasocket(s))
	        goto badnotsocket ;
#endif /* COMMENT */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: calling 'handle'\n") ;
#endif

	    srs = watchone(&g,s,&srvsubs,&exports,&sfile,&bis) ;

	} /* end if */


/* release the server table substitutions */

	varsub_finish(&srvsubs) ;


/* close the daemon stuff */
daemonret2:
	(void) builtin_free(&bis) ;

	(void) srvtab_close(&sfile) ;

	u_close(s) ;

/* close some more (earlier) daemon stuff */
daemonret1:

#ifdef	OPTIONAL
	vecstr_finish(&defines) ;

	vecstr_finish(&exports) ;
#endif /* OPTIONAL */


#if	CF_DEBUG
	if (g.debuglevel > 0)
	    bprintf(efp,"%s: program finishing\n",
	        g.progname) ;
#endif

	if (f_freeconfigfname && (configfname != NULL))
	    free(configfname) ;

	if (g.f.log)
		logfile_close(&g.lh) ;

	if (g.lfp != NULL) bclose(g.lfp) ;

	bclose(efp) ;

	return srs ;

earlyret:
	bclose(efp) ;

	return OK ;

/* error types of returns */
badret:
	if (g.f.log)
		logfile_close(&g.lh) ;

	bclose(efp) ;

	return BAD ;

badretlog:
	bclose(g.lfp) ;

	goto badret ;

/* USAGE> rexd [-C conf] [-p port] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [{conf|-} [port]] [-C conf] [-p port] [-?v] ",
	    g.progname,g.progname) ;

	bprintf(efp,"[-D[=n]]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument(s) given\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badworking:
	bprintf(efp,"%s: could not access the working directory \"%s\"\n",
	    g.progname,g.workdir) ;

	goto badret ;

badqueue:
	bprintf(efp,"%s: could not process the queue directory\n",
	    g.progname) ;

	goto badret ;

badlistinit:
	bprintf(efp,"%s: could not initialize list structures (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badsrv:
	bprintf(efp,"%s: bad service table file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badnosrv:
	bprintf(efp,"%s: no service table file specified\n",
	    g.progname) ;

	goto badret ;

badconfig:
	bprintf(efp,
	    "%s: configfile=%s\n",
	    g.progname,configfname) ;

	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    g.progname,rs,cf.badline) ;

	goto badret ;

badpidopen:
	bprintf(efp,
	    "%s: could not open the PID file (rs=%d)\n",
	    g.progname,rs) ;

	bprintf(efp, "%s: pidfile=%s\n", g.progname,g.pidfname) ;

	goto badret ;

badpidlock:
	if (! g.f.quiet) {

	    bprintf(efp,
	        "%s: could not lock the PID file (rs=%d)\n",
	        g.progname,rs) ;

	    bprintf(efp, "%s: pidfile=%s\n", g.progname,g.pidfname) ;

	    while ((len = breadline(&pidfile,buf,BUFLEN)) > 0) {

	        bprintf(efp,"%s: pidfile> %W",
	            g.progname,
	            buf,len) ;

	    }

	    bclose(&pidfile) ;

	} /* end if */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	goto badret ;

badport:
	bprintf(efp,
	    "%s: bad port specified\n",
	    g.progname) ;

	goto daemonret1 ;

badsocket:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not create a socket to listen on (rs=%d)\n",
	        g.progname,srs) ;

	goto daemonret1 ;

badbind:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not bind to our port number (rs=%d)\n",
	        g.progname,srs) ;

	goto daemonret2 ;

badlisten:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not listen to our server socket (rs=%d)\n",
	        g.progname,srs) ;

	goto daemonret2 ;

badpidfile1:
	logfile_printf(&g.lh,
	    "could not open the PID mutex file (rs=%d)\n",
	    srs) ;

	logfile_printf(&g.lh, "pidfile=%s\n", g.pidfname) ;

	goto daemonret2 ;

badpidfile2:
	logfile_printf(&g.lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    g.pid) ;

	goto daemonret2 ;

badnotsocket:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: the standard input is NOT a socket\n",
	        g.progname) ;

	goto daemonret2 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int procfile(gp,func,pr,svp,fname,elp)
struct global	*gp ;
int		(*func)(char *,char *,VECSTR *) ;
const char	pr[] ;
VECSTR		*svp ;
char		fname[] ;
VECSTR		*elp ;
{
	int	sl ;

	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
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
	if (gp->debuglevel > 1)
	    debugprintf("procfile: 2 fname=%s\n",fname) ;
#endif

	if (sl >= 0)
	    return (*func)(pr,fname,elp) ;

	return SR_NOEXIST ;
}
/* end subroutine (procfile) */



