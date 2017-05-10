/* main */

/* main subroutine for the MKKEY program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	1


/* revision history:

	= 1994-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This is the 'main' module for the 'mkkey' program.  This module
	processes the program invocation arguments and performs some
	preprocessing steps before any actual input files are scanned.

	The real work of processing the input files (one at a time) is
	performed by the 'process()' subroutine.


*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<char.h>
#include	<varsub.h>
#include	<hdb.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"eigendb.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	process() ;

extern int	varsub_load(), varsub_addvec(VARSUB *,VECSTR *) ;
extern int	varsub_subbuf(), varsub_merge() ;
extern int	expander() ;

extern char	*strbasename(), *strshrink() ;
extern char	*timestr_log() ;


/* externals variables */

extern char	makedate[] ;


/* forward references */

static char	*filereadable() ;


/* local global variabes */


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


int main(argc,argv,envp)
int	argc ;
char	*argv[] ;
char	*envp[] ;
{
	struct proginfo		g, *pip = &g ;

	USERINFO	u ;

	CONFIGFILE	cf ;

	EIGENDB		eigendb ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		argfile, *afp = &argfile ;
	bfile		eigenfile, *ifp = &eigenfile ;

	varsub	vsh_e, vsh_d ;

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
	int	len1, len2 ;
	int	err_fd ;
	int	argnum, c ;

	uchar	terms[32] ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp, *linebuf = buf ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	timebuf[32] ;
	char	tmpfname[MAXPATHLEN + 1] ;
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
	    esetfd(err_fd) ;
#endif


#if	CF_DEBUGS
	eprintf("main: started\n") ;
#endif

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;

	pip->f.stderror = TRUE ;
	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0) {
		bcontrol(efp,BC_LINEBUF,0) ;
	} else
	    pip->f.stderror = FALSE ;

#ifdef	COMMENT
	bprintf(efp,"%s: started\n",pip->progname) ;

	bflush(efp) ;
#endif


/* initialize */

	pip->f.verbose = FALSE ;
	pip->f.log = FALSE ;
	pip->f.removelabel = FALSE ;
	pip->f.wholefile = FALSE ;
	pip->f.quiet = FALSE ;

	pip->debuglevel = 0 ;
	pip->minwordlen = -2 ;
	pip->maxwordlen = -2 ;
	pip->eigenwords = -2 ;
	pip->keys = -2 ;

	pip->programroot = NULL ;
	eigenfname = NULL ;

/* early initialization processing */

	pip->tmpdir = getenv("TMPDIR") ;


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
	                    (cfdeci(argp + 1,argl - 1,&argnum) < 0))
	                    goto badarg ;

	            } else {

#if	CF_DEBUGS
	                eprintf("main: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    eprintf("main: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    eprintf("main: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    eprintf("main: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	                eprintf("main: about to check for a key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    eprintf("main: got an option keyword, kwi=%d\n",
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
	                            if (avl) pip->programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) pip->programroot = argp ;

	                        }

	                        break ;

	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) pip->tmpdir = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) pip->tmpdir = argp ;

	                        }

	                        break ;

/* version */
	                    case ARGOPT_VERSION:
#if	CF_DEBUGS
	                        eprintf("main: version key-word\n") ;
#endif
	                        f_version = TRUE ;
	                        if (f_optequal) goto badargextra ;

	                        break ;

/* verbose mode */
	                    case ARGOPT_VERBOSE:
	                        pip->f.verbose = TRUE ;
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
	                            pip->progname,akp) ;

	                        goto badarg ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    eprintf("main: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        eprintf("main: option key letters\n") ;
#endif

	                        switch (*akp) {

/* debug */
	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

#if	CF_DEBUGS
	                                eprintf(
	                                    "main: debug flag, avp=\"%W\"\n",
	                                    avp,avl) ;
#endif

	                                f_optequal = FALSE ;
	                                if ((avl > 0) &&
	                                    (cfdec(avp,avl,&pip->debuglevel) < 0))
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
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					eigenfname = argp ;

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

/* file names to process is in this named file */
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

/* maximum number of keys written out */
	                        case 'k':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&pip->keys) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'l':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&pip->minwordlen) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'm':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&pip->maxwordlen) < 0))
	                                goto badargvalue ;

	                            break ;

/* number of eigenwords to consider */
	                        case 'n':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl &&
	                                (cfdec(argp,argl,&pip->eigenwords) < 0))
	                                goto badargvalue ;

	                            break ;

/* output file name */
	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ofname = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* remove labels */
	                        case 's':
	                            pip->f.removelabel = TRUE ;
	                            break ;

/* index whole files */
	                        case 'w':
	                            pip->f.wholefile = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->f.verbose = TRUE ;
	                            break ;

	                        default:
	                            bprintf(efp,"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

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
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: finished parsing arguments\n") ;
#endif


	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

		if ((cp = strchr(makedate,C_RPAREN)) != NULL) {

			cp += 1 ;
			while (*cp && isspace(*cp))
				cp += 1 ;

		} else
			cp = makedate ;

	    bprintf(efp,"%s: makedate %s\n",
	        pip->progname,cp) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

	if (pip->debuglevel > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 0)
	        eprintf("main: debuglevel %d\n",pip->debuglevel) ;
#endif

	    bprintf(efp,"%s: debug level %d\n",
	        pip->progname,pip->debuglevel) ;

	    bflush(efp) ;

	}


/* get our program root */

	if (pip->programroot == NULL)
	    pip->programroot = getenv(PROGRAMROOTVAR1) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv(PROGRAMROOTVAR2) ;

	if (pip->programroot == NULL)
	    pip->programroot = getenv(PROGRAMROOTVAR3) ;

	if (pip->programroot == NULL)
	    pip->programroot = PROGRAMROOT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: initial programroot=%s\n",pip->programroot) ;
#endif


/* get some host/user information */

	if ((rs = userinfo(&u,userbuf,USERINFO_LEN,NULL)) < 0) {

	    if ((pip->f.verbose || (pip->debuglevel > 0)) && (! pip->f.quiet))
	        bprintf(efp,"%s: could not get user information\n",
	            pip->progname) ;

	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;


/* find a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		eprintf(
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
	        pip->programroot,ETCDIR1,CONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        pip->programroot,ETCDIR1,CONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        pip->programroot,ETCDIR2,CONFIGFILE1) ;


/* read in the configuration file if we have one */

	if (u_access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) eprintf(
	        "main: we have a configuration file \"%s\"\n",
	        configfname) ;
#endif

	    if ((rs = configfile_init(&cf,configfname)) < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		eprintf("main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		eprintf("main: varsubinit d\n") ;
#endif

	    varsub_init(&vsh_d,VARSUB_MBADNOKEY) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		eprintf("main: varsubinit e\n") ;
#endif

	    varsub_init(&vsh_e,0) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		eprintf("main: var_init\n") ;
#endif

	    varsub_load(&vsh_e,envp) ;


	    if ((cf.root != NULL) && (pip->programroot == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: CF programroot=%s\n",
	                cf.root) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(&g,buf,len1,buf2,BUFLEN)) > 0)) {

	            pip->programroot = mallocstrn(buf2,len2) ;

	        }

	        cf.root = NULL ;

	    } /* end if (programroot) */

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(&g,buf,len1,buf2,BUFLEN)) > 0)) {

	            logfname = mallocstrn(buf2,len2) ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: processed CF logfilename=%s\n",logfname) ;
#endif

	        cf.logfname = NULL ;

	    } /* end if (configuration file log filename) */

	    if ((cf.helpfname != NULL) && (helpfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: CF helpfname=%s\n",cf.helpfname) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.helpfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(&g,buf,len1,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("main: helpfname subed and expanded\n") ;
#endif

	            helpfname = mallocstrn(buf2,len2) ;

	        }

	        cf.helpfname = NULL ;

	    } /* end if (helpfname) */

	    if ((cf.eigenfname != NULL) && (eigenfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: CF eigenfile=%s\n",cf.eigenfname) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.eigenfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(&g,buf,len1,buf2,BUFLEN)) > 0)) {

	            eigenfname = mallocstrn(buf2,len2) ;

	        }

	        cf.eigenfname = NULL ;

	    } /* end if (eigenfname) */

	    if ((cf.tmpdir != NULL) && (pip->tmpdir == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: CF tmpdir=%s\n",cf.tmpdir) ;
#endif

	        if (((len1 = varsub_subbuf(&vsh_d,&vsh_e,cf.tmpdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((len2 = expander(&g,buf,len1,buf2,BUFLEN)) > 0)) {

	            pip->tmpdir = mallocstrn(buf2,len2) ;

	        }

	        cf.tmpdir = NULL ;

	    } /* end if (tmpdir) */


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

	    if ((cf.maxwordlen >= 0) && (pip->maxwordlen < 0))
	        pip->maxwordlen = cf.loglen ;

	    if ((cf.minwordlen >= 0) && (pip->minwordlen < 0))
	        pip->minwordlen = cf.minwordlen ;

	    if ((cf.keys >= 0) && (pip->keys < 0))
	        pip->keys = cf.keys ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: processing CF freeing data structures\n") ;
#endif

	    varsubfree(&vsh_d) ;

	    varsubfree(&vsh_e) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: processing CF free\n") ;
#endif


	    configfile_free(&cf) ;

	} /* end if (accessed the configuration file) */


#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		eprintf("main: finished with any configfile stuff\n") ;
#endif


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: final programroot=%s\n",pip->programroot) ;
#endif


/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		eprintf("main: checking program parameters\n") ;
#endif


	if ((pip->tmpdir == NULL) || (pip->tmpdir[0] == '\0'))
	    pip->tmpdir = TMPDIR ;


/* clean up the path to the eigen word file, if we have one */

	if ((eigenfname == NULL) || (eigenfname[0] == '\0'))
	    eigenfname = EIGENFNAME ;

	rs = u_access(eigenfname,R_OK) ;

	if ((rs < 0) && (eigenfname[0] != '/')) {

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	        pip->programroot,eigenfname) ;

	    if ((rs = u_access(tmpfname,R_OK)) >= 0)
	        eigenfname = mallocstr(tmpfname) ;

	} /* end if */

	if (rs < 0)
	    eigenfname = NULL ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: eigenfname=%s\n",eigenfname) ;
#endif

	if (pip->f.verbose || (pip->debuglevel > 0))
	    bprintf(efp,"%s: eigenfname=%s\n", pip->progname,eigenfname) ;


/* do we have an activity log file ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs = BAD ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = LOGFNAME ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,logfname,0,0666,u.logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if ((rs < 0) && (logfname[0] != '/')) {

	    bufprintf(tmpfname,MAXPATHLEN,"%s/%s",
	        pip->programroot,logfname) ;

	    rs = logfile_open(&pip->lh,tmpfname,0,0666,u.logid) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	if (rs >= 0) {

		struct utsname	un ;


	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we opened a logfile\n") ;
#endif

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0) 
		loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we checked its length\n") ;
#endif

	    (void) u_time(&daytime) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: making log entry\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %s\n",
	        timestr_log(daytime,timebuf),
	        BANNER) ;

	    logfile_printf(&pip->lh,"%-14s %s/%s\n",
	        pip->progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;


	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

		(void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",
		un.sysname,un.release,
		u.pid) ;


	    if (eigenfname != NULL)
	        logfile_printf(&pip->lh,"eigenfile=%s\n",
	            eigenfname) ;

	} /* end if (we have a log file or not) */


/* print out the help file if requested */

	if (f_help) {

	    if ((helpfname == NULL) || (helpfname[0] == '\0'))
	        helpfname = HELPFNAME ;

		printhelp(NULL,pip->programroot,SEARCHNAME,helpfname) ;

	    goto done ;

	} /* end if (helpfname) */


/* some final initialization */

	if (pip->minwordlen < -1)
	    pip->minwordlen = MINWORDLEN ;

	if (pip->maxwordlen < -1)
	    pip->maxwordlen = MAXWORDLEN ;

	if (pip->eigenwords < -1)
	    pip->eigenwords = EIGENWORDS ;

	if (pip->keys < -1)
	    pip->keys = KEYS ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    eprintf( "main: delimiter=>%s<\n",delimiter) ;

	    eprintf( "main: keys=%d\n",pip->keys) ;

	}
#endif /* CF_DEBUG */


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

	    rs = bopen(ofp,BIO_STDOUT,buf,0666) ;

	} else
	    rs = bopen(ofp,ofname,buf,0666) ;

	if (rs < 0) 
		goto badoutopen ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf( "main: opened output file rs=%d\n",rs) ;
#endif



/* open the eigenwords file and read them in to a DB */

	if (pip->eigenwords == 0)
	    len2 = 10 ;

	else if (pip->eigenwords < 0)
	    len2 = EIGENWORDS ;

	else
	    len2 = pip->eigenwords ;

	rs = eigendb_open(&eigendb,len2) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf( "main: opened eigen DB rs=%d\n",rs) ;
#endif

	if (rs < 0) 
		goto badeigen ;


	if ((eigenfname != NULL) && (pip->eigenwords != 0) &&
	    (bopen(ifp,eigenfname,"r",0666) >= 0)) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf( "main: opened eigenfile rs=%d\n",rs) ;
#endif

	    i = 0 ;
	    while ((len = bgetline(ifp,linebuf,LINEBUFLEN)) > 0) {

	        int	fl ;


	        if (linebuf[len - 1] == '\n') 
			len -= 1 ;

	        fl = sfshrink(linebuf,len,&cp) ;

	        if ((fl >= pip->minwordlen) && (*cp != '#')) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf( "main: eigendb_add() \n") ;
#endif

	            rs = eigendb_add(&eigendb,cp,fl) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf( "main: eigendb_add() rs=%d\n") ;
#endif

	            if (pip->eigenwords > 0) {

	                i += 1 ;
	                if (i >= pip->eigenwords) 
				break ;

	            } /* end if */

	        } /* end if (met minimum word length restriction) */

	    } /* end while */

	    bclose(ifp) ;

	} else
	    pip->eigenwords = 0 ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf( "main: finished reading eigenfile rs=%d\n",rs) ;
#endif


/* process the positional arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf( "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i) && (strlen(argv[i]) > 0)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf( "main: i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif

	        if (pip->f.verbose || (pip->debuglevel > 0))
	            bprintf(efp,"%s: processing input file \"%s\"\n",
	                pip->progname,argv[i]) ;


	        if ((rs = process(&g,ofp,
	            &eigendb,terms,delimiter,ignorechars,argv[i])) < 0) {

	            cp = argv[i] ;
	            if (*cp == '-') cp = "*stdinput*" ;

	            bprintf(efp,"%s: error processing input file, rs=%d\n",
	                pip->progname,rs) ;

	            bprintf(efp,"%s: errored file=%s\n",
	                pip->progname,cp) ;

	        }

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (processing positional arguments) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: done processing argument files\n") ;
#endif


/* process any files in the argument filename list file */

	if (afname != NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we have an argument file list\n") ;
#endif

	    if ((strcmp(afname,"-") == 0) || (afname[0] == '\0'))
	        rs = bopen(afp,BIO_STDIN,"dr",0666) ;

	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: processing the argument file list\n") ;
#endif

	        while ((len = bgetline(afp,buf,BUFLEN)) > 0) {

	            if (buf[len - 1] == '\n') len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            if ((rs = process(&g,ofp,
	                &eigendb,terms,delimiter,ignorechars,cp)) < 0) {

	                if (*cp == '-') cp = "*stdinput*" ;

	                bprintf(efp,
	                    "%s: error processing input file, rs=%d\n",
	                    pip->progname,rs) ;

	                bprintf(efp,"%s: errored file=%s\n",
	                    pip->progname,cp) ;

	            } /* end if */

	            pan += 1 ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: done processing the argument file list\n") ;
#endif

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(efp,"\trs=%d argfile=%s\n",rs,afname) ;

	        }

	    }

	} else if (pan == 0) {

	    if ((rs = process(&g,ofp,
	        &eigendb,terms,delimiter,ignorechars,"-")) < 0) {

	        cp = "*stdinput*" ;
	        bprintf(efp,"%s: error processing input file, rs=%d\n",
	            pip->progname,rs) ;

	        bprintf(efp,"%s: errored file=%s\n",
	            pip->progname,cp) ;

	    }

	    pan += 1 ;

	} /* end if (file list arguments or not) */


/* close out the eigenwords database */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: closing eigenwords DB\n") ;
#endif

	eigendb_close(&eigendb) ;


/* close the output file */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: closing output file\n") ;
#endif

	bclose(ofp) ;


	if (pip->f.verbose || (pip->debuglevel > 0))
	    bprintf(efp,"%s: %d files processed\n",
	        pip->progname,pan) ;


/* we are done */
done:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: program finishing\n") ;
#endif

	logfile_close(&pip->lh) ;

	bclose(efp) ;

	return srs ;

earlyret:
	bclose(efp) ;

	return OK ;

/* error types of returns */
badret:
	bclose(efp) ;

	return BAD ;

/* USAGE> grope-dict [-C conf] [file(s) ...] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-C conf] [-d delim] [file(s) ...] [-?vV]\n",
	    pip->progname,pip->progname) ;

	bprintf(efp,"\t[-l minwordlen] [-m maxwordlen] [-HELP] [-D[=n]]\n") ;

	bprintf(efp,"\t[-n eigenwords] [-asw] [-c eigenfile]\n") ;

	bprintf(efp,"\t[-i ignorechars] [-k nkeys]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badret ;

badconfig:
	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument(s) given\n",
	    pip->progname) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output (key) file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badeigen:
	bclose(ofp) ;

	bprintf(efp,"%s: could not initialize the eigen word database\n",
	    pip->progname) ;

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

	if (u_access(tmpfname,R_OK) >= 0)
	    return tmpfname ;

	return NULL ;
}
/* end subroutine (filereadable) */



