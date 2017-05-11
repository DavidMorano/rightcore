/* main */

/* main subroutine for the REFERM program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1994-09-01, David A�D� Morano

	This program was originally written.


*/

/* Copyright � 1994 David A�D� Morano.  All rights reserved. */

/*****************************************************************************

	Handles references in documents.


*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<varsub.h>
#include	<hdb.h>
#include	<field.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"bdb.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	BUFLEN
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	var_init(), var_subbuf(), var_merge() ;
extern int	expander() ;
extern int	bdbinit(), bdbfree(), bdbloads(), bdbcount() ;
extern int	process() ;

extern char	*strbasename(), *strshrink() ;
extern char	*timestr_log() ;
extern char	*malloc_str(), *malloc_sbuf() ;


/* externals variables */

extern char	makedate[] ;


/* forward references */

static int	bibexists() ;

static char	*filereadable() ;

static void	eigendb_add(), eigendb_close() ;


/* local global variabes */

struct global		g ;


/* local structures */

/* define command option words */

static char *argopts[] = {
	"ROOT",
	"CONFIG",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	NULL,
} ;

#define	ARGOPT_ROOT	0
#define	ARGOPT_CONFIG	1
#define	ARGOPT_VERSION	2
#define	ARGOPT_VERBOSE	3
#define	ARGOPT_TMPDIR	4
#define	ARGOPT_LOGFILE	5
#define	ARGOPT_HELP	6


/* local variables */


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct global		*gp = &g ;

	struct configfile	cf ;

	struct bdb	bdbl ;

	struct bdb_cur	cur ;

	VARSUB		varsub	vsh_e, vsh_d ;

	HDB		eigendb ;

	USERINFO	u ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		argfile, *afp = &argfile ;
	bfile		eigenfile, *ifp = &eigenfile ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j, k, l ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	len, srs, rs ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_append = FALSE ;
	int	loglen = -1 ;
	int	c, l2 ;
	int	err_fd ;
	int	argnum ;

	uchar	terms[32] ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp, *linebuf = buf ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*configfname = NULL ;
	const char	*logfname = NULL ;
	const char	*helpfname = NULL ;
	const char	*afname = NULL ;
	const char	*eigenfname = NULL ;
	const char	*ofname = NULL ;
	const char	*ignorechars = IGNORECHARS ;
	const char	*delimiter = " " ;
	const char	*cp ;


#if	CF_DEBUG
	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;
#endif

#if	CF_DEBUGS
	debugprintf("main: started\n") ;
#endif

	g.version = VERSION ;
	g.progname = strbasename(argv[0]) ;

	g.f.stderror = TRUE ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	else
	    g.f.stderror = FALSE ;

#ifdef	COMMENT
	bprintf(efp,"%s: started\n",g.progname) ;

	bflush(efp) ;
#endif


/* initialize */

	g.f.verbose = FALSE ;
	g.f.log = FALSE ;
	g.f.quiet = FALSE ;

	g.debuglevel = 0 ;
	g.minwordlen = -2 ;
	g.maxwordlen = -2 ;
	g.eigenwords = -2 ;
	g.keys = -2 ;

	g.programroot = NULL ;
	helpfname = NULL ;
	eigenfname = NULL ;

/* early initialization processing */

	g.tmpdir = getenv("TMPDIR") ;


/* start parsing the arguments */

	bdbinit(&bdbl) ;

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
	                    (cfdeci(argp + 1,argl - 1,&argnum) < 0))
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

/* print out the help */
	                    case ARGOPT_HELP:
	                        f_help = TRUE ;
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

/* append to the key file */
	                        case 'a':
	                            f_append = TRUE ;
	                            break ;

/* common words file (eigenfile) */
	                        case 'c':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) eigenfname = argp ;

	                            break ;

/* entry delimiter string */
	                        case 'd':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

/* we want to store the pointer to zero length string if given */
	                            delimiter = argp ;

	                            break ;

	                        case 'f':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                            afname = argp ;

	                            break ;

	                        case 'i':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ignorechars = argp ;

	                            break ;

	                        case 'k':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&g.keys) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'l':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&g.minwordlen) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'm':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&g.maxwordlen) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'n':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&g.eigenwords) < 0))
	                                goto badargvalue ;

	                            break ;

/* specify the bibliographical databases */
	                        case 'p':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                bdbloads(&bdbl,argp,argl) ;

	                            break ;

/* output file */
	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ofname = argp ;

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


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif


	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	    bprintf(efp,"%s: makedate %s\n",
	        g.progname,makedate) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

	if (g.debuglevel > 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 0)
	        debugprintf("main: debuglevel %d\n",g.debuglevel) ;
#endif

	    bprintf(efp,"%s: debug level %d\n",
	        g.progname,g.debuglevel) ;

	    bflush(efp) ;

	}


/* get our program root */

	if (g.programroot == NULL)
	    g.programroot = getenv(VARPROGRAMROOT1) ;

	if (g.programroot == NULL)
	    g.programroot = getenv(VARPROGRAMROOT2) ;

	if (g.programroot == NULL)
	    g.programroot = getenv(VARPROGRAMROOT3) ;

	if (g.programroot == NULL)
	    g.programroot = PROGRAMROOT ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: initial programroot=%s\n",g.programroot) ;
#endif

/* get some host/user information */

	if ((rs = userinfo(&u,userbuf,USERINFO_LEN,NULL)) < 0) {

	    if ((g.f.verbose || (g.debuglevel > 0)) && (! g.f.quiet))
	        bprintf(efp,"%s: could not get user information\n",
	            g.progname) ;

	}

	g.nodename = u.nodename ;
	g.domainname = u.domainname ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking for configuration file\n") ;
#endif

/* search locally */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR1,CONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR1,CONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR2,CONFIGFILE1) ;

	if (configfname == NULL) {

	    configfname = CONFIGFILE1 ;
	    if (u_access(configfname,R_OK) < 0)
	        configfname = NULL ;

	}

	if (configfname == NULL) {

	    configfname = CONFIGFILE2 ;
	    if (u_access(configfname,R_OK) < 0)
	        configfname = NULL ;

	}

/* search in our program root area */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        g.programroot,ETCDIR1,CONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        g.programroot,ETCDIR1,CONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        g.programroot,ETCDIR2,CONFIGFILE1) ;


/* read in the configuration file if we have one */

	if (u_access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: we have a configuration file \"%s\"\n",
	        configfname) ;
#endif

	    if ((rs = configinit(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: varsubinit d\n") ;
#endif

	    varsubinit(&vsh_d) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: varsubinit e\n") ;
#endif

	    varsubinit(&vsh_e) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: var_init\n") ;
#endif

	    var_init(&vsh_e,envv) ;


	    if ((cf.root != NULL) && (g.programroot == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF programroot=%s\n",
	                cf.root) ;
#endif

	        if (((l = var_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

	            g.programroot = malloc_sbuf(buf2,l2) ;

	        }

	        cf.root = NULL ;

	    } /* end if (programroot) */

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((l = var_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

	            logfname = malloc_sbuf(buf2,l2) ;

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",logfname) ;
#endif

	        cf.logfname = NULL ;

	    } /* end if (configuration file log filename) */

	    if ((cf.helpfname != NULL) && (helpfname == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF helpfname=%s\n",cf.helpfname) ;
#endif

	        if (((l = var_subbuf(&vsh_d,&vsh_e,cf.helpfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: helpfname subed and expanded\n") ;
#endif

	            helpfname = malloc_sbuf(buf2,l2) ;

	        }

	        cf.helpfname = NULL ;

	    } /* end if (helpfname) */

	    if ((cf.eigenfname != NULL) && (eigenfname == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF eigenfile=%s\n",cf.eigenfname) ;
#endif

	        if (((l = var_subbuf(&vsh_d,&vsh_e,cf.eigenfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

	            eigenfname = malloc_sbuf(buf2,l2) ;

	        }

	        cf.eigenfname = NULL ;

	    } /* end if (eigenfname) */

	    if ((cf.tmpdir != NULL) && (g.tmpdir == NULL)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: CF tmpdir=%s\n",cf.tmpdir) ;
#endif

	        if (((l = var_subbuf(&vsh_d,&vsh_e,cf.tmpdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

	            g.tmpdir = malloc_sbuf(buf2,l2) ;

	        }

	        cf.tmpdir = NULL ;

	    } /* end if (tmpdir) */


#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

	    if ((cf.maxwordlen >= 0) && (g.maxwordlen < 0))
	        g.maxwordlen = cf.loglen ;

	    if ((cf.minwordlen >= 0) && (g.minwordlen < 0))
	        g.minwordlen = cf.minwordlen ;

	    if ((cf.keys >= 0) && (g.keys < 0))
	        g.keys = cf.keys ;


#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: processing CF freeing data structures\n") ;
#endif

	    varsubfree(&vsh_d) ;

	    varsubfree(&vsh_e) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: processing CF free\n") ;
#endif


	    configfree(&cf) ;

	} /* end if (accessed the configuration file) */


#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: finished with any configfile stuff\n") ;
#endif


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: final programroot=%s\n",g.programroot) ;
#endif



/* check program parameters */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking program parameters\n") ;
#endif


	if ((g.tmpdir == NULL) || (g.tmpdir[0] == '\0'))
	    g.tmpdir = TMPDIR ;


/* clean up the EigenWord dictionary path as we may have it */

	if ((eigenfname == NULL) || (eigenfname[0] == '\0'))
	    eigenfname = EIGENFNAME ;

	rs = u_access(eigenfname,R_OK) ;

	if ((rs < 0) && (eigenfname[0] != '/')) {

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	        g.programroot,eigenfname) ;

	    if ((rs = u_access(tmpfname,R_OK)) >= 0)
	        eigenfname = malloc_str(tmpfname) ;

	} /* end if */

	if (rs < 0)
	    eigenfname = NULL ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: eigenfname=%s\n",eigenfname) ;
#endif

	if (g.f.verbose || (g.debuglevel > 0))
	    bprintf(efp,"%s: eigenfname=%s\n", g.progname,eigenfname) ;


#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: progname=%s\n", g.progname) ;
#endif


/* the database(s) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: default BIB\n") ;
#endif

	if (bdbcount(&bdbl) <= 0) {

	    if ((cp = getenv("BIBLIOGRAPHY")) != NULL)
	        bdbloads(&bdbl,cp,-1) ;

	    else if ((cp = getenv("REFER")) != NULL)
	        bdbloads(&bdbl,cp,-1) ;

	}

/* are there databases which are not there ? */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: progname=%s\n", g.progname) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: are the BIBs all there\n") ;
#endif

	bdbnullcursor(&cur) ;

	while (bdbget(&bdbl,&cur,&cp) >= 0) {

	    if (cp == NULL) continue ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: top_loop progname=%s\n", g.progname) ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: BIB exists >%s<\n",cp) ;
#endif

	    if (! bibexists(cp)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: BIB not exist progname=%s\n", g.progname) ;
#endif

	        if (! g.f.quiet)
	            bprintf(g.efp,
	                "%s: bibliography \"%s\" not found\n",
	                g.progname,cp) ;

	        bdbdelcursor(&bdbl,&cur) ;

	    } /* end if (deleting non-existent BIBs) */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: bot_loop progname=%s\n", g.progname) ;
#endif

	} /* end for */


#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: progname=%s\n", g.progname) ;
#endif


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: activity log ?\n") ;
#endif


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs = BAD ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = LOGFILE ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs = logfile_open(&g.lh,logfname,0,0666,u.logid) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if ((rs < 0) && (logfname[0] != '/')) {

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	        g.programroot,logfname) ;

	    rs = logfile_open(&g.lh,tmpfname,0,0666,u.logid) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	if (rs >= 0) {

	    g.f.log = TRUE ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0) loglen = LOGSIZE ;

	    logfile_checksize(&g.lh,loglen) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we checked its length\n") ;
#endif

	    time(&daytime) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: making log entry\n") ;
#endif

	    logfile_printf(&g.lh,"%s %s\n",
	        timestr_log(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&g.lh,"%-14s %s/%s\n",
	        g.progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;


	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    logfile_printf(&g.lh,"os=%s pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",u.pid) ;

	    logfile_printf(&g.lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;


	    if (eigenfname != NULL)
	        logfile_printf(&g.lh,"eigenfile=%s\n",
	            eigenfname) ;

	} /* end if (we have a log file or not) */


#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: progname=%s\n", g.progname) ;
#endif


/* print out the helpfname if requested */

	if (f_help) {

	    bfile	helpfile ;


#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: -1 helpfname=%s\n",helpfname) ;
#endif

	    rs = BAD ;
	    if ((helpfname == NULL) || (helpfname[0] == '\0'))
	        helpfname = HELPFILE ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: 0 helpfname=%s\n",helpfname) ;
#endif

	    if ((helpfname[0] == '/') || (u_access(helpfname,R_OK) >= 0))
	        rs = bopen(&helpfile,helpfname,"r",0666) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: 1 helpfname=%s rs=%d\n",helpfname,rs) ;
#endif

	    if ((rs < 0) && (helpfname[0] != '/')) {

	        bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	            g.programroot,helpfname) ;

	        rs = bopen(&helpfile,tmpfname,"r",0666) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: 2 helpfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	    } /* end if (we tried to open another log file) */

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: reading helpfname\n") ;
#endif

	        bcopyblock(&helpfile,efp,-1) ;

	        bclose(&helpfile) ;

	        bflush(efp) ;

	    } /* end if */

	    goto done ;

	} /* end if (helpfname) */


/* some final initialization */

	if (g.minwordlen < -1)
	    g.minwordlen = MINWORDLEN ;

	if (g.maxwordlen < -1)
	    g.maxwordlen = MAXWORDLEN ;

	if (g.eigenwords < -1)
	    g.eigenwords = EIGENWORDS ;

	if (g.keys < -1)
	    g.keys = KEYS ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf( "main: delimiter=>%s<\n",delimiter) ;
#endif

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: progname=%s\n", g.progname) ;
#endif



/* field terminators */

	for (i = 0 ; i < 32 ; i += 1)
	    terms[i] = 0xFF ;

	BACLR(terms,'_') ;

	for (c = 'a' ; c <= 'z' ; c += 1)
	    BACLR(terms,c) ;

	for (c = 'A' ; c <= 'Z' ; c += 1)
	    BACLR(terms,c) ;

	for (c = '0' ; c <= '9' ; c += 1)
	    BACLR(terms,c) ;



/* open the output key file */

	strcpy(buf,"wc") ;

	if (f_append)
	    strcat(buf,"a") ;

	else
	    strcat(buf,"t") ;

	if ((ofname == NULL) || (ofname[0] == '\0')) {

	    strcat(buf,"d") ;

	    rs = bopen(ofp,BFILE_STDOUT,buf,0666) ;

	} else
	    rs = bopen(ofp,ofname,buf,0666) ;

	if (rs < 0) goto badoutopen ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf( "main: opened output file rs=%d\n",rs) ;
#endif



/* open the eigenwords file and read them in to a DB */

	if (g.eigenwords == 0)
	    l2 = 10 ;

	else if (g.eigenwords < 0)
	    l2 = EIGENWORDS ;

	else
	    l2 = g.eigenwords ;

	rs = hdbinit(&eigendb,l2,NULL) ;

	if (rs < 0) goto badeigen ;


	if ((eigenfname != NULL) && (g.eigenwords != 0) &&
	    (bopen(ifp,eigenfname,"r",0666) >= 0)) {

	    i = 0 ;
	    while ((len = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {
	        int	fl ;

	        if (linebuf[len - 1] == '\n') len -= 1 ;

	        fl = sfshrink(linebuf,len,&cp) ;

	        if (fl >= g.minwordlen) {

	            eigendb_add(&eigendb,cp,fl) ;

	            if (g.eigenwords > 0) {

	                i += 1 ;
	                if (i >= g.eigenwords) break ;

	            }

	        } /* end if (met minimum word length restriction) */

	    } /* end while */

	    bclose(ifp) ;

	} else
	    g.eigenwords = 0 ;


#if	CF_DEBUG
	if (gp->debuglevel > 1) {

	    debugprintf("main: BDB enumerated keys\n") ;

	bdbnullcursor(&cur) ;

	while (bdbget(&bdbl,&cur,&cp) >= 0) {

		if (cp == NULL) continue ;

	    debugprintf("main: db=%s\n",cp) ;

	} /* end for */

	}
#endif /* CF_DEBUG */


/* process the positional arguments */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf( "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i) && (strlen(argv[i]) > 0)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "main: progname=%s\n", g.progname) ;
#endif

	        if (g.f.verbose || (g.debuglevel > 0))
	            bprintf(efp,"%s: 1 processing input file \"%s\"\n",
	                g.progname,argv[i]) ;


	        if ((rs = process(&g,&bdbl,ofp, argv[i])) < 0) {

	            cp = argv[i] ;
	            if (*cp == '-') cp = "*stdinput*" ;

	            bprintf(efp,"%s: error processing input file, rs=%d\n",
	                g.progname,rs) ;

	            bprintf(efp,"%s: errored file=%s\n",
	                g.progname,cp) ;

	        }

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: done processing argument files\n") ;
#endif


/* process any files in the argument filename list file */

	if (afname != NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: we have an argument file list\n") ;
#endif

	    if ((strcmp(afname,"-") == 0) || (afname[0] == '\0'))
	        rs = bopen(afp,BFILE_STDIN,"dr",0666) ;

	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processing the argument file list\n") ;
#endif

	        while ((len = breadline(afp,buf,BUFLEN)) > 0) {

	            if (buf[len - 1] == '\n') len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            if ((rs = process(&g,&bdbl,ofp, cp)) < 0) {

	                if (*cp == '-') cp = "*stdinput*" ;

	                bprintf(efp,
	                    "%s: error processing input file, rs=%d\n",
	                    g.progname,rs) ;

	                bprintf(efp,"%s: errored file=%s\n",
	                    g.progname,cp) ;

	            } /* end if */

	            pan += 1 ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: done processing the argument file list\n") ;
#endif

	    } else {

	        if (! g.f.quiet) {

	            bprintf(efp,
	                "%s: could not open the argument list file\n",
	                g.progname) ;

	            bprintf(efp,"\trs=%d argfile=%s\n",rs,afname) ;

	        }

	    }

	} else if (pan == 0) {

	    if ((rs = process(&g,&bdbl,ofp, "-")) < 0) {

	        cp = "*stdinput*" ;
	        bprintf(efp,"%s: error processing input file, rs=%d\n",
	            g.progname,rs) ;

	        bprintf(efp,"%s: errored file=%s\n",
	            g.progname,cp) ;

	    }

	    pan += 1 ;

	} /* end if (file list arguments or not) */


/* close out the various databases */


/* close out the eigenwords database */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: closing eigenwords DB\n") ;
#endif

	eigendb_close(&eigendb) ;


/* close the output file */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: closing output file\n") ;
#endif

	bclose(ofp) ;


	if (g.f.verbose || (g.debuglevel > 0))
	    bprintf(efp,"%s: %d files processed\n",
	        g.progname,pan) ;


/* we are done */
done:

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: program finishing\n") ;
#endif

	if ((configfname != NULL) && (configfname[0] != '\0'))
	    configfree(&cf) ;

	logfile_close(&g.lh) ;

	bclose(efp) ;

	return srs ;

earlyret:
	bclose(efp) ;

	return OK ;

/* error types of returns */
badret:
	bdbfree(&bdbl) ;

	bclose(efp) ;

	return BAD ;

/* USAGE> grope-dict [-C conf] [file(s) ...] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-C conf] [-d delim] [file(s) ...] [-?vV]\n",
	    g.progname,g.progname) ;

	bprintf(efp,"\t[-l minwordlen] [-m maxwordlen] [-HELP] [-D[=n]]\n") ;

	bprintf(efp,"\t[-n nkeys] [-asw]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badconfig:
	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    g.progname,rs,cf.badline) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument(s) given\n",
	    g.progname) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output (key) file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badeigen:
	bclose(ofp) ;

	bprintf(efp,"%s: could not initialize the eigen word database\n",
	    g.progname) ;

	goto badret ;
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

	if (access(tmpfname,R_OK) >= 0)
	    return tmpfname ;

	return NULL ;
}
/* end if (filereadable) */



static void eigendb_add(dbp,s,len)
HDB_HEAD	*dbp ;
char		s[] ;
int		len ;
{
	HDB_DATUM	key, value, dumbvalue ;


	if (len < 0) len = strlen(s) ;

	key.len = len ;
	key.buf = s ;			/* prepare for check */

	value.len = 0 ;
	value.buf = NULL ;

/* if it is already present, we're done, return */

	if (hdbfetch(dbp,NULL,key,&dumbvalue) >= 0)
	    return ;

	key.buf = malloc_sbuf(s,len) ;	/* for real */

	hdbstore(dbp,key,value) ;

	return ;
}

static void eigendb_close(dbp)
HDB_HEAD	*dbp ;
{
	HDB_CUR	keycursor ;
	HDB_DATUM	key, value ;


	hdbnullcursor(&keycursor) ;

	while (hdbenum(dbp,&keycursor,&key,&value) >= 0)
	    if (key.buf != NULL) free(key.buf) ;

	hdbfree(dbp) ;

	return ;
}



static char	*suffix[] = {
	".ia",
	".i",
	NULL,
} ;

static int bibexists(s)
char	s[] ;
{
	char	buf[MAXPATHLEN + 1] ;
	char	**spp = suffix ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("bibexists: entered\n") ;
#endif

	if (u_access(s,R_OK) >= 0)
	    return TRUE ;

	while (*spp != NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("bibexists: top loop >%s<\n",*spp) ;
#endif

	    bufprintf(buf,MAXPATHLEN,"%s%s",
	        s,*spp) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("bibexists: buf=%s\n",buf) ;
#endif

	    if (u_access(buf,R_OK) >= 0)
	        return TRUE ;

	    spp += 1 ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "bibexists: bl progname=%s\n", g.progname) ;
#endif

	} /* end while */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf( "bibexists: ol progname=%s\n", g.progname) ;
#endif

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("bibexists: exiting\n") ;
#endif

	return FALSE ;
}
/* end subroutine (bibexists) */


