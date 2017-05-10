/* main (TRANSFORM) */

/* transform the contents of a file to something else */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 1995-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a data transformation program.

	Synopsis:

	$ transform -d|-e -k key [-V?] [<inputfile> [<outputfile>]]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<time.h>
#include	<ftw.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<getxusername.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<varsub.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define		MAXARGINDEX	100
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef		BUFLEN
#define		BUFLEN		(4096 + REALNAMELEN)
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forward references */


/* local global variabes */


/* local structures */


/* local variables */

/* define command option words */

static const char *argopts[] = {
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"ROOT",
	"LOGFILE",
	"CONFIG",
	NULL
} ;


#define	ARGOPT_TMPDIR	0
#define	ARGOPT_VERSION	1
#define	ARGOPT_VERBOSE	2
#define	ARGOPT_ROOT	3
#define	ARGOPT_LOGFILE	4
#define	ARGOPT_CONFIG	5


/* exported subroutines */


int main(argc,argv,envp)
int		argc ;
const char	*argv[] ;
const char	*envp[] ;
{
	struct proginfo	g, *pip = &g ;
	struct ustat	sb ;
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	VARSUB		vsh_e, vsh_d ;
	vecstr		defines, exports ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j, k, l ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	len, srs, rs ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	s, proto ;
	int	port ;
	int	loglen ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	*cp ;
	char	tmpfname[MAXPATHLEN + 1] ;


	if (u_fstat(3,&sb) >= 0) 
		debugsetfd(3) ;


	(void) memset(pip,0,sizeof(struct proginfo)) ;

	g.f.stderror = TRUE ;
	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0)
	    g.f.stderror = FALSE ;

	g.progname = strbasename(argv[0]) ;


/* initialize */

	g.f.quiet = FALSE ;
	g.f.verbose = FALSE ;

	g.debuglevel = 0 ;

	port = -1 ;

	g.programroot = NULL ;
	g.tmpdir = DEFTMPDIR ;
	g.workdir = NULL ;


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
	                            if (avl) logfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) logfname = argp ;

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

	                            if (argl) g.pidfile = argp ;

	                            break ;

/* daemon mode */
	                        case 'd':
	                            g.f.daemon = TRUE ;
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
	            if (cfdeci(argv[i],-1,&port) < 0)
	                goto badargvalue ;

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

#if	CF_DEBUG && 0
	if (g.debuglevel > 1) {

	    debugprintf(
	        "main: miscellaneous \n") ;

	    whoopen() ;

	}
#endif



/* get our program root */

	if (g.programroot == NULL)
	    g.programroot = getenv(VARPROGRAMROOT) ;

	if (g.programroot == NULL)
	    g.programroot = getenv("PROGRAMROOT") ;

	if (g.programroot == NULL)
	    g.programroot = PROGRAMROOT ;


/* prepare to store configuration variable lists */

	if ((rs = vecstrinit(&defines,10,VSP_CONSERVE)) < 0) goto badjobinit ;

	if ((rs = vecstrinit(&exports,10,VSP_CONSERVE)) < 0) {

	    vecstrfree(&defines) ;

	    goto badjobinit ;
	}


/* find a configuration file if we have one */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking for configuration file\n") ;
#endif

/* search locally */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        NULL,"etc/rexd",DEFCONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,"etc/rexd",DEFCONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,"etc",DEFCONFIGFILE1) ;

	if (configfname == NULL) {

	    configfname = DEFCONFIGFILE1 ;
	    if (access(configfname,R_OK) < 0)
	        configfname = DEFCONFIGFILE2 ;

	}

/* search in our program root area */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        g.programroot,"etc/rexd",DEFCONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        g.programroot,"etc/rexd",DEFCONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        g.programroot,"etc",DEFCONFIGFILE1) ;


/* read in the configuration file if we have one */

	if (access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: we have a configuration file \"%s\"\n",
	        configfname) ;
#endif

	    if ((rs = configinit(&cf,configfname)) < 0) goto badconfig ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: varsubinit d\n") ;
#endif

	    varsubinit(&vsh_d,0) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: varsubinit e\n") ;
#endif

	    varsubinit(&vsh_e,0) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: var_init\n") ;
#endif

	    var_init(&vsh_e,envp) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf(
	            "main: 0 for\n") ;

	        varsubdump(&vsh_e) ;

	    }
#endif /* CF_DEBUG */


	    for (i = 0 ; vecstrget(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "main: 0 top\n") ;
#endif

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 0 about to merge\n") ;
#endif

	            var_merge(&vsh_d,&defines,buf,l) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 0 out of merge\n") ;
#endif

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "main: 0 bot\n") ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: done w/ defines\n") ;
#endif


	    for (i = 0 ; vecstrget(&cf.exports,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "main: 1 about to sub> %s\n",cp) ;
#endif

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 1 about to merge> %W\n",buf,l) ;
#endif

	            var_merge(NULL,&exports,buf,l) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 1 done merging\n") ;
#endif

#if	CF_DEBUG && 0
	            if (g.debuglevel > 1) {

	                debugprintf("var_merge: VSA_D so far \n") ;

	                varsubsump(&vsh_d) ;

	                debugprintf("var_merge: VSA_E so far \n") ;

	                varsubsump(&vsh_e) ;

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

	    if ((cf.workdir != NULL) && (g.workdir == NULL)) {

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cf.workdir,
	            -1,buf,BUFLEN)) > 0)
	            g.workdir = malloc_sbuf(buf,l) ;

	        cf.workdir = NULL ;

	    }

	    if ((cf.pidfile != NULL) && (g.pidfile == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF pidfile=%s\n",cf.pidfile) ;
#endif

	        if ((cf.pidfile[0] != '\0') && (cf.pidfile[0] != '-')) {

	            if ((l = var_subbuf(&vsh_d,&vsh_e,cf.pidfile,
	                -1,buf,BUFLEN)) > 0)
	                g.pidfile = malloc_sbuf(buf,l) ;

	        } else
	            g.pidfile = "" ;

	        cf.pidfile = NULL ;

	    } /* end if (configuration file PIDFILE) */

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("main: so far logfname=%s\n",logfname) ;

	        debugprintf("main: about to get configfile log filename\n") ;
	    }
#endif

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0)
	            logfname = malloc_sbuf(buf,l) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",logfname) ;
#endif

	        cf.logfname = NULL ;

	    } /* end if (configuration file log filename) */

	    if ((cf.port != NULL) && (port < 0)) {

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cf.port,
	            -1,buf,BUFLEN)) > 0)

	            if (isalpha(buf)) {

#ifdef	SYSV
	                sep = getservbyname_r(buf, "tcp",
	                    &se,buf,BUFLEN) ;
#else
	                sep = getservbyname(buf, "tcp") ;
#endif

	                if (sep == NULL) goto badconfigport ;

	                port = (int) ntohs(sep->s_port) ;

	            } else if (cfdeci(buf,l,&port) < 0)
	                goto badconfigport ;

	    } /* end if (handling the configuration file port parameter) */

	    cf.port = NULL ;


	    if ((cf.root != NULL) && (g.programroot == NULL)) {

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0)
	            g.programroot = malloc_sbuf(buf,l) ;

	        cf.root = NULL ;

	    }

	    if ((cf.userpass != NULL) && (g.userpass == NULL)) {

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cf.userpass,
	            -1,buf,BUFLEN)) > 0)
	            g.userpass = malloc_sbuf(buf,l) ;

	        cf.userpass = NULL ;

	    }

	    if ((cf.machpass != NULL) && (g.machpass == NULL)) {

	        if ((l = var_subbuf(&vsh_d,&vsh_e,cf.machpass,
	            -1,buf,BUFLEN)) > 0)
	            g.machpass = malloc_sbuf(buf,l) ;

	        cf.root = NULL ;

	    }

	    if ((cf.loglen >= 0) && (loglen < 0)) {

	        loglen = cf.loglen ;

	    }

	    varsubfree(&vsh_d) ;

	    varsubfree(&vsh_e) ;

	    configfree(&cf) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("main: dumping defines\n") ;

	        for (i = 0 ; vecstrget(&defines,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: define> %s\n",cp) ;

	        debugprintf("main: dumping exports\n") ;

	        for (i = 0 ; vecstrget(&exports,i,&cp) >= 0 ; i += 1)
	            debugprintf("main: export> %s\n",cp) ;

	    } /* end if (CF_DEBUG) */
#endif /* CF_DEBUG */

	} /* end if (accessed the configuration file) */


#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: finished with any configfile stuff\n") ;
#endif


/* before we go too far, are we the only one on this PID mutex ? */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: pidfile=%s\n",g.pidfile) ;
#endif

	if (g.pidfile != NULL) {

	    if ((g.pidfile[0] == '\0') || (g.pidfile[0] == '-'))
	        g.pidfile = DEFPIDFILE ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we have a PIDFILE=%s\n",g.pidfile) ;
#endif

	    if ((rs = bopen(&pidfile,g.pidfile,"rwc",0664)) < 0)
	        goto badpidopen ;

/* capture the lock (if we can) */

	    if ((rs = bcontrol(&pidfile,BC_LOCK,0)) < 0)
	        goto badpidlock ;

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

	    bprintf(&pidfile,"%d\n",getpid()) ;

	    bclose(&pidfile) ;

	} /* end if (we have a mutex PID file) */


/* check program parameters */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking program parameters\n") ;
#endif

	if (g.workdir == NULL)
	    g.workdir = DEFWORKDIR ;

	else if (g.workdir[0] == '\0')
	    g.workdir = "." ;

	if ((g.tmpdir == NULL) || (g.tmpdir[0] == '\0')) {

	    if ((cp = getenv("TMPDIR")) != NULL)
	        g.tmpdir = cp ;

	    else
	        g.tmpdir = DEFTMPDIR ;

	} /* end if (tmpdir) */



/* open the system report log file */

#ifdef	COMMENT
	g.f.log = FALSE ;
	if ((rs = bopen(g.lfp,logfname,"wca",0664)) >= 0) {

	    g.f.log = TRUE ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: system log rs=%d\n",rs) ;
#endif

	    daytime = time(0L) ;

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

	if ((access(g.workdir,X_OK) < 0) ||
	    (access(g.workdir,R_OK) < 0)) goto badworking ;


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs = BAD ;
	if ((logfname != NULL) && (logfname[0] != '\0'))
	    rs = logfile_open(&g.lh,logfname,0,0666,DEFLOGID) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs < 0) {

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	        g.programroot,DEFLOGFILE) ;

	    rs = BAD ;
	    if (access(tmpfname,W_OK) >= 0)
	        rs = logfile_open(&g.lh,tmpfname,0,0666,DEFLOGID) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	userbuf[0] = '\0' ;
	if (rs >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

/* we opened it, maintenance this log file if we have to */

	    logfile_checksize(&g.lh,loglen) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

/* prepare to make a log entry */

	    userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	    g.nodename = u.nodename ;
	    g.domainname = u.domainname ;
	    g.username = u.username ;

	    time(&daytime) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    rs = logfile_printf(&g.lh,"\n") ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: made log entry, rs=%d\n",rs) ;
#endif

	    logfile_printf(&g.lh,"\n") ;

	    logfile_printf(&g.lh,"%s %s started\n",
	        timestr_log(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&g.lh,"%-14s %s/%s\n",
	        g.progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

	} else {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we do not have a logfile\n") ;
#endif

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

	    getusername(buf,BUFLEN,-1) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: goto username\n") ;
#endif

	    g.username = malloc_str(buf) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: done w/ name stuff\n") ;
#endif

	} /* end if (we have a log file or not) */


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: continuing with some PID stuff\n") ;
#endif

	g.gid = getgid() ;

#ifdef	SYSV
	gp = (struct group *) getgrgid_r(g.gid,
	    &ge,buf,BUFLEN) ;
#else
	gp = getgrgid(g.gid) ;
#endif

	if (gp == NULL) {

	    cp = buf ;
	    sprintf(buf,"GUEST-%d",(int) g.gid) ;

	} else
	    cp = gp->gr_name ;

	g.groupname = malloc_str(cp) ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: about to check if we are a daemon\n") ;
#endif

/* if we are a daemon program, try to bind our INET port ? */

	if (g.f.daemon) {

	    long	addr ;


#if	CF_DEBUG && 0
	    if (g.debuglevel > 1) {

	        debugprintf("main: daemon program\n") ;

	        whoopen() ;
	    }
#endif

/* look up some miscellaneous stuff in various databases */

	    if (port < 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: looking at port stuff\n") ;
#endif

#ifdef	SYSV
	        sep = getservbyname_r("rex", "tcp",
	            &se,buf,BUFLEN) ;
#else
	        sep = getservbyname("rex", "tcp") ;
#endif

	        if (sep != NULL)
	            port = (int) ntohs(sep->s_port) ;

	        else
	            port = DEFPORT ;

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

#if	SYSV
	    pep = getprotobyname_r("tcp",
	        &pe,buf,BUFLEN) ;
#else
	    pep = getprotobyname("tcp") ;
#endif

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

	        close(s) ;

	        srs = rs ;
	        goto badbind ;
	    }

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: about to listen\n") ;
#endif

	    if ((rs = u_listen(s,10)) < 0) {

	        close(s) ;

	        goto badlisten ;
	    }


/* background ourselves */

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: become a daemon ?\n") ;
#endif

	    bclose(efp) ;

	    open("/dev/null",O_RDONLY,0666) ;

	    if (g.debuglevel == 0) {

	        for (i = 0 ; i < 3 ; i += 1) {

	            close(i) ;

	            open("/dev/null",O_RDONLY,0666) ;

	        }

	        rs = uc_fork() ;

	        if (rs < 0) {
	            logfile_printf(&g.lh,"cannot fork daemon (%d)\n",rs) ;
	            u_exit(BAD) ;
	        }

	        if (rs > 0) u_exit(OK) ;

	        setsid() ;

	    } /* end if (backgrounding) */

#if	CF_DEBUG && 0
	    if (g.debuglevel > 1)
	        whoopen() ;
#endif

	} /* end if (becoming a daemon) */


/* we start ! */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: starting \n") ;
#endif

	g.pid = getpid() ;

	if (userbuf[0] != '\0')
	    u.pid = g.pid ;


	sprintf(buf,"%d.%s",g.pid,DEFLOGID) ;

	g.logid = malloc_str(buf) ;

	logfile_setid(&g.lh,g.logid) ;

	if (g.f.daemon && (rs == 0))
	    logfile_printf(&g.lh,"backgrounded\n") ;


/* before we go too far, are we the only one on this PID mutex ? */

	if (g.f.daemon) {

	    if ((g.pidfile != NULL) && (g.pidfile[0] != '\0')) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: we have a PIDFILE=%s\n",g.pidfile) ;
#endif

	        if ((srs = bopen(&pidfile,g.pidfile,"rwc",0664)) < 0)
	            goto badpidfile2 ;

/* capture the lock (if we can) */

	        if ((srs = bcontrol(&pidfile,BC_LOCK,2)) < 0) {

	            bclose(&pidfile) ;

	            goto badpidfile3 ;
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

	        logfile_printf(&g.lh,"pidfile=%s\n",g.pidfile) ;

	        logfile_printf(&g.lh,"PID mutex captured\n") ;

	        bcontrol(&pidfile,BC_STAT,&sb) ;

	        logfile_printf(&g.lh,"pidfile device=%ld inode=%ld\n",
	            sb.st_dev,sb.st_ino) ;

	        g.pidfp = &pidfile ;

	    } /* end if (we have a mutex PID file) */

	} /* end if (daemon mode) */


	if (userbuf[0] != '\0') {

	    time(&daytime) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    logfile_printf(&g.lh,"os=%s pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",g.pid) ;

	    logfile_printf(&g.lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	    if (g.f.daemon)
	        logfile_printf(&g.lh,"%s finished initializing\n",
	            timestr_log(daytime,timebuf)) ;

	} /* end if (making log entries) */


	if (g.f.daemon) {

#if	CF_DEBUG && 0
	    if (g.debuglevel > 1) {

	        debugprintf(
	            "main: starting to watch for new jobs\n") ;

	        whoopen() ;
	    }
#endif

	    srs = watch(s,&exports) ;

	    close(s) ;

	} else
	    srs = rexecd(0,&exports) ;


/* we are done */
daemonret:

#if	CF_DEBUG
	if (g.debuglevel < 0)
	    bprintf(efp,"%s: program finishing\n",
	        g.progname) ;
#endif

	if ((configfname != NULL) && (configfname[0] != '\0'))
	    configfree(&cf) ;

	if (g.lfp != NULL) bclose(g.lfp) ;

	return srs ;

earlyret:
	bclose(efp) ;

	return OK ;

/* error types of returns */
badret:
	bclose(efp) ;

	return BAD ;

badretlog:
	bclose(g.lfp) ;

	goto badret ;

/* USAGE> dwd [-C conf] [-polltime] [directory_path] [srvtab] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-C conf] [-polltime] [directory] [srvtab] [-?v] ",
	    g.progname,g.progname) ;

	bprintf(efp,"[-D[=n]]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
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

badjobinit:
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

baddir:
	bprintf(efp,"%s: bad working directory specified - errno %d\n",
	    g.progname,rs) ;

	goto badret ;

badconfig:
	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    g.progname,rs,cf.badline) ;

	goto badret ;

badconfigport:
	bprintf(efp,
	    "%s: the port number in the configuration file was bad\n",
	    g.progname) ;

	goto badret ;

badpidopen:
	bprintf(efp,
	    "%s: could not open the PID file \"%s\" rs=%d\n",
	    g.progname,g.pidfile,rs) ;

	goto badret ;

badpidlock:
	if (! g.f.quiet) {

	    bprintf(efp,
	        "%s: could not lock the PID file \"%s\" rs=%d\n",
	        g.progname,g.pidfile,rs) ;

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

badarg:
	bprintf(efp,"%s: bad argument(s) given\n",
	    g.progname) ;

	goto badret ;

badsocket:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not create a socket to listen on (rs=%d)\n",
	        g.progname,srs) ;

	goto badret ;

badbind:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not bind to our port number (rs=%d)\n",
	        g.progname,srs) ;

	goto badret ;

badlisten:
	if (! g.f.quiet)
	    bprintf(efp,
	        "%s: could not listen to our server socket (rs=%d)\n",
	        g.progname,srs) ;

	goto badret ;

badpidfile2:
	logfile_printf(&g.lh,
	    "we could not open the PID mutext file \"%s\" ! (rs=%d)\n",
	    g.pid,srs) ;

	goto daemonret ;

badpidfile3:
	logfile_printf(&g.lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    g.pid) ;

	goto daemonret ;

}
/* end subroutine (main) */


/* is a file readable */
static char *filereadable(tmpfname,dir1,dir2,fname)
char	tmpfname[], dir1[], dir2[], fname[] ;
{


	if ((dir1 != NULL) && (dir2 != NULL))
	    sprintf(tmpfname,"%s/%s/%s",
	        dir1,dir2,fname) ;

	else if (dir1 != NULL)
	    sprintf(tmpfname,"%s/%s",
	        dir1,fname) ;

	else if (dir2 != NULL)
	    sprintf(tmpfname,"%s/%s",
	        dir2,fname) ;

	else
	    strcpy(tmpfname,fname) ;

	if (u_access(tmpfname,R_OK) >= 0)
	    return tmpfname ;

	return NULL ;
}
/* end subroutine (filereadable) */



