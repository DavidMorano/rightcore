/* main (mroff) */

/* MiniRoff program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1887-09-01, David A­D­ Morano

	This program was originally written.


	- 1998-02-20, David A­D­ Morano

	This program has been enhanced to provide for an arbitrary (user
	defined) header on the print-outs.  Not having a user defined
	header on print-outs has proven to be dissasterous in the past.


*/

/* Copyright © 1987,1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This program will read the input file and format it into
	TROFF constant width font style input.


*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<varsub.h>
#include	<getxusername.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"configfile.h"
#include	"mroff.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	USERINFO_LEN	(2 * 1024)

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	getpwd(char *,int) ;
extern int	getnodedomain(char *,char *) ;

extern int	expander() ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forward references */

static int	usage(struct proginfo *) ;

static char	*filereadable() ;


/* local structures */

static const char *argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"LOGFILE",
	"CONFIG",
	NULL,
} ;


#define	ARGOPT_ROOT	0
#define	ARGOPT_TMPDIR	1
#define	ARGOPT_VERSION	2
#define	ARGOPT_VERBOSE	3
#define	ARGOPT_LOGFILE	4
#define	ARGOPT_CONFIG	5


/* local variables */






int main(argc,argv,envp)
int	argc ;
char	*argv[] ;
char	*envp[] ;
{
	struct proginfo	g, *pip = &g ;

	struct ustat	sb ;

	struct group	ge, *gp ;

	USERINFO	u ;

	CONFIGFILE	cf ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;

	vecstr		defines, exports ;

	VARSUB		vsh_e, vsh_d ;

	struct mroff	ro ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	maxai, pan, npa ;
	int	argvalue = -1 ;
	int	rs, rs1 ;
	int	i, j, k, l ;
	int	len, srs ;
	int	loglen = -1 ;
	int	l2 ;
	int	pointlen = 0, ps = 0, vs = 0 ;
	int	pagenum = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_extra = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	nodename[NODENAMELEN + 1], domainname[MAXHOSTNAMELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	pwd[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*configfname = NULL ;
	const char	*logfname = NULL ;
	const char	*pointstring = NULL ;
	const char	*ofname = NULL ;
	const char	*fontname  = NULL ;
	const char	*cp ;


	memset(pip,0,sizeof(struct proginfo)) ;

	g.f.stderror = TRUE ;
	g.efp = efp ;
	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0)
	    g.f.stderror = FALSE ;

	g.version = VERSION ;
	g.progname = strbasename(argv[0]) ;

	if (fstat(3,&sb) >= 0)
	    debugsetfd(3) ;


/* we want to open up some files so that the first few FD slots are FULL !! */

	for (i = 0 ; i < 3 ; i += 1) {

	    if (fstat(i,&sb) != 0) 
		u_open("/dev/null",O_RDWR,0666) ;

	}


/* general initialization */

	g.tmpdname = DEFTMPDNAME ;

/* local initialization */

	fontname = DEFFONTNAME ;


/* MROFF specific initialization */

	ro.f.concatenate = FALSE ;
	ro.f.reference = FALSE ;
	ro.f.headers = TRUE ;
	ro.coffset = 0 ;
	ro.xoffset = 0 ;
	ro.yoffset = 0 ;
	ro.maxlines = DEFMAXLINES ;
	ro.blanklines = DEFBLANKLINES ;
	ro.headerstring = NULL ;
	ro.footerstring = NULL ;


/* start parsing the arguments */

	rs = SR_OK ;
	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

	                    rs = cfdeci(argp + 1,argl - 1,&ro.maxlines) ;

	            } else {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {
	                f_optequal = TRUE ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	            } else {
			avp = NULL ;
	                avl = 0 ;
	                akl = aol ;
	            }

/* do we have a keyword match or should we assume only key letters ? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

	                    case ARGOPT_TMPDIR:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.tmpdname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.tmpdname = argp ;

	                        }

	                        break ;

/* version */
	                    case ARGOPT_VERSION:
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
	                            if (avl) g.pr = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.pr = argp ;

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
				rs = SR_INVALID ;
				f_usage = TRUE ;
	                        bprintf(efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) configfname = argp ;

	                            break ;

/* debug */
	                        case 'D':
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl > 0)
	                                    rs = cfdec(avp,avl,&g.debuglevel) ;

	                            }

	                            break ;

/* quiet mode */
	                        case 'Q':
	                            g.f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* blank lines at the top of a page */
	                        case 'b':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdec(argp,argl,&ro.blanklines) ;

	                            break ;

/* concatenate mode */
	                        case 'c':
	                            g.f.quiet = TRUE ;
	                            break ;

/* font name */
	                        case 'f':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) fontname = argp ;

	                            break ;

/* request that there be no page headers added */
	                        case 'n':
	                            ro.f.headers = FALSE ;
	                            break ;

/* output filename */
	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ofname = argp ;

	                            break ;

/* point size */
	                        case 'p':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {
	                                pointstring = argp ;
	                                pointlen = argl ;
	                            }

	                            break ;

				case 'q':
				    pip->verboselevel = 0 ;
				    break ;

/* reference mode */
	                        case 'r':
	                            ro.f.reference = TRUE ;
	                            break ;

/* vertical spacing */
	                        case 'v':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
					rs = cfdec(argp,argl,&vs) ;

	                            break ;

	                        case 'x':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdec(argp,argl,&ro.xoffset) ;

	                            break ;

	                        case 'y':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdec(argp,argl,&ro.yoffset) ;

	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        default:
				    rs = SR_INVALID ;
				    f_usage = TRUE ;
	                            bprintf(efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
				    break ;

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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
		debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (g.debuglevel > 1)
	    bprintf(efp,"%s: debuglevel=%u\n",
	        g.progname,g.debuglevel) ;

	if (f_version) 
		bprintf(efp,"%s: version %s\n",
	    g.progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;

/* get our program root */

	if (g.pr == NULL)
	    g.pr = getenv(VARPROGRAMROOT) ;

	if (g.pr == NULL)
	    g.pr = getenv("PROGRAMROOT") ;

	if (g.pr == NULL)
	    g.pr = PROGRAMROOT ;

/* prepare to store configuration variable lists */

	rs = vecstr_start(&defines,10,VECSTR_PNOHOLES) ;

	if (rs >= 0) {
		rs = vecstr_start(&exports,10,VECSTR_PNOHOLES) ;
		if (rs < 0)
	    	vecstr_finish(&defines) ;
	}

	if (rs < 0) {
		ex = EX_OSERR ;
		goto retearly ;
	}

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (g.debuglevel > 1) 
		debugprintf("main: checking for configuration file\n") ;
#endif

/* search locally */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        NULL,DEFCONFIGDIR1,DEFCONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,DEFCONFIGDIR1,DEFCONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,DEFCONFIGDIR2,DEFCONFIGFILE1) ;

	if (configfname == NULL) {

	    configfname = DEFCONFIGFILE1 ;
	    if (access(configfname,R_OK) < 0)
	        configfname = DEFCONFIGFILE2 ;

	}

/* search in our program root area */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        g.pr,DEFCONFIGDIR1,DEFCONFIGFILE1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        g.pr,DEFCONFIGDIR1,DEFCONFIGFILE2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        g.pr,DEFCONFIGDIR2,DEFCONFIGFILE1) ;


/* read in the configuration file if we have one */

	if (access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: we have a configuration file \"%s\"\n",
	        configfname) ;
#endif

	    if ((rs = configfile_start(&cf,configfname)) < 0) 
		goto badconfig ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) 
		debugprintf("main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (g.debuglevel > 1) 
		debugprintf("main: varsub_start d\n") ;
#endif

	    varsub_start(&vsh_d,0) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) 
		debugprintf("main: varsub_start e\n") ;
#endif

	    varsub_start(&vsh_e,0) ;


#if	CF_DEBUG
	    if (g.debuglevel > 1) {
	        debugprintf("main: 0 for\n") ;
	        varsub_dumpfd(&vsh_e,-1) ;
	    }
#endif /* CF_DEBUG */


	    for (i = 0 ; vecstr_get(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (g.debuglevel > 1) 
			debugprintf("main: 0 top\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1) 
			debugprintf("main: 0 about to merge\n") ;
#endif

	            varsub_merge(&vsh_d,&defines,buf2,l2) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) 
			debugprintf("main: 0 out of merge\n") ;
#endif

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1) 
			debugprintf("main: 0 bot\n") ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: done w/ defines\n") ;
#endif


	    for (i = 0 ; vecstr_get(&cf.exports,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "main: 1 about to sub> %s\n",cp) ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 1 about to merge> %W\n",buf,l2) ;
#endif

	            varsub_merge(NULL,&exports,buf2,l2) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: 1 done merging\n") ;
#endif

#if	CF_DEBUG && 0
	            if (g.debuglevel > 1) {
	                debugprintf("varsub_merge: VSA_D so far \n") ;
	                varsub_dumpfd(&vsh_d,-1) ;
	                debugprintf("varsub_merge: VSA_E so far \n") ;
	                varsub_dumpfd(&vsh_e,-1) ;
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

	    if ((cf.workdir != NULL) && (g.workdname == NULL)) {

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,cf.workdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

	            g.workdname = mallocstrw(buf2,l2) ;

	        }

	        cf.workdir = NULL ;

	    }

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

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

	            logfname = mallocstrw(buf2,l2) ;

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",
			logfname) ;
#endif

	        cf.logfname = NULL ;

	    } /* end if (configuration file log filename) */


	    if ((cf.root != NULL) && (g.pr == NULL)) {

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(&g,buf,l,buf2,BUFLEN)) > 0)) {

	            g.pr = mallocstrw(buf2,l2) ;

	        }

	        cf.root = NULL ;

	    }

	    if ((cf.loglen >= 0) && (loglen < 0)) {

	        loglen = cf.loglen ;

	    }

	    varsub_finish(&vsh_d) ;

	    varsub_finish(&vsh_e) ;

	    configfile_finish(&cf) ;

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

/* check program parameters */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking program parameters\n") ;
#endif

	if (g.workdname == NULL) {
	    g.workdname = DEFWORKDNAME ;

	} else if (g.workdname[0] == '\0')
	    g.workdname = "." ;

	if ((g.tmpdname == NULL) || (g.tmpdname[0] == '\0')) {

	    if ((cp = getenv("TMPDIR")) != NULL) {
	        g.tmpdname = cp ;

	    } else
	        g.tmpdname = DEFTMPDNAME ;

	} /* end if (tmpdir) */


/* can we access the working directory ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: access working directory \"%s\"\n",g.workdname) ;
#endif

	if ((access(g.workdname,X_OK) < 0) ||
	    (access(g.workdname,R_OK) < 0)) goto badworking ;


/* try to get some user information */

	userbuf[0] = '\0' ;
	rs =  userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	g.nodename = u.nodename ;
	g.domainname = u.domainname ;
	g.username = u.username ;

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

	    getusername(buf,BUFLEN,-1) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: goto username\n") ;
#endif

	    g.username = mallocstr(buf) ;

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

#if	defined(GETGRXXXR) && GETGRXXXR
	if ((rs = getgrgid_r(g.gid,&ge,buf,BUFLEN,&gp)) != 0)
		rs = (- rs) ;

	if (rs < 0)
		gp = NULL ;
#else
	gp = getgrgid(g.gid) ;
#endif

	if (gp == NULL) {

	    cp = buf ;
	    sprintf(buf,"GUEST-%d",(int) g.gid) ;

	} else
	    cp = gp->gr_name ;

	g.groupname = mallocstr(cp) ;



/* do we have an activity log file ? */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif


	sprintf(buf,"%s%d",g.nodename,g.pid) ;

	g.logid = mallocstr(buf) ;

	logfile_setid(&g.lh,g.logid) ;


	rs = BAD ;
	if ((logfname != NULL) && (logfname[0] != '\0'))
	    rs = logfile_open(&g.lh,logfname,0,0666,g.logid) ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs < 0) {

	    mkpath2(tmpfname,
		 g.pr,DEFLOGFILE) ;

	    rs = BAD ;
	    if (access(tmpfname,W_OK) >= 0)
	        rs = logfile_open(&g.lh,tmpfname,0,0666,g.logid) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: we do %shave a logfile\n",
	        ((rs < 0) ? "not " : "")) ;
#endif



	if (rs >= 0) {

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

/* prepare to make a log entry */

	    daytime = time(NULL) ;

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

	} /* end if (making  first log entry) */




	if (userbuf[0] != '\0')
	    u.pid = g.pid ;



	if (userbuf[0] != '\0') {

		struct utsname	un ;


	    (void) u_time(&daytime) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

		(void) u_uname(&un) ;

	    logfile_printf(&g.lh,"ostype=%s os=%s(%s) pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",
		un.sysname,un.release,
		g.pid) ;

	    logfile_printf(&g.lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	} /* end if (making log entries) */



/* we start ! */

#if	CF_DEBUG
	if (g.debuglevel > 1) 
		debugprintf("main: starting \n") ;
#endif



/* how many lines per page */

	if (ro.maxlines < 1) ro.maxlines = DEFMAXLINES ;

	if (ro.maxlines > MAXLINES) ro.maxlines = MAXLINES ;

/* establish an offset if any */

	if (ro.coffset < 0)
	    ro.coffset = 0 ;

	else if (ro.coffset > LINELEN)
	    ro.coffset = LINELEN ;


/* establish working point size and vertical spacing */

	if (pointstring != NULL) {

	    i = substring(pointstring,pointlen,".") ;

	    if (i < 0)
	        i = substring(pointstring,pointlen,"/") ;

	    if (i >= 0) {

	        if (i > 0) 
		    rs = cfdec(pointstring,i,&ps) ;

	        if ((rs >= 0) && ((pointlen - i) > 1))
	            rs = cfdec(pointstring + i + 1,pointlen - i - 1,&vs) ;

	    } else {

	        rs = cfdec(pointstring,pointlen,&ps) ;

	    }

	} /* end if (handling the 'pointstring') */

	if (rs < 0)
	    goto badarg ;

	if (ps < 2) ps = 6 ;

	if (vs == 0)
	    vs = ps + 2 ;

	else if (vs < ps)
	    vs = ps + 1 ;

	if (g.debuglevel > 0) bprintf(efp,
	    "%s: ps %ld - vs %ld\n",g.progname,ps,vs) ;


/* try to open the output file */

	if (ofname == NULL) 
	rs = bopen(ofp,ofname,"wca",0666) ;

	else
	rs = bopen(ofp,BFILE_STDOUT,"dwca",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: outfile file unavailable (%d)\n",
		pip->progname,rs) ;

	    goto badoutopen ;
	}

/* perform initialization processing */

	if (ro.f.headers)
		ro.maxlines -= 2 ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	debugprintf("main: maxlines %d\n",ro.maxlines) ;
#endif


/* perform initialization processing */

	bprintf(ofp,".nf\n") ;

	bprintf(ofp,".fp 1 %s\n.ft %s\n",fontname,fontname) ;

/* change to running point size */

	bprintf(ofp,".ps 6\n") ;

	bprintf(ofp,".vs 8\n") ;



/* perform the MROFF print-out function on our file arguments */

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	if (npa > 0) {

	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "main: got a positional argument i=%d pan=%d arg=%s\n",
	            i,pan,argv[i]) ;
#endif


	        rs = mroff(pip,&ro,&pagenum,argv[i],pan,ofp) ;


	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */

	} else {

	        rs = mroff(pip,&ro,&pagenum,"-",pan,ofp) ;

	}


	if (g.f.verbose || (g.debuglevel > 0))
	    bprintf(g.efp,"%s: pages printed %d\n",
	        pagenum) ;


/* we are done */
done:

#if	CF_DEBUG
	if (g.debuglevel < 0)
	    bprintf(efp,"%s: program finishing\n",
	        g.progname) ;
#endif

	if ((configfname != NULL) && (configfname[0] != '\0'))
	    configfile_finish(&cf) ;

	bclose(ofp) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

badoutopen:
retearly:
	bclose(g.efp) ;

	return ex ;

/* USAGE> mroff [-C conf] [-polltime] [directory_path] [srvtab] [-V?] */
usage:
	usage(pip) ;

	goto retearly ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: invalid argument specified (%d)\n",
	    g.progname,rs) ;

	usage(pip) ;

	goto retearly ;

badargnum:
badargextra:
badargvalue:
	ex = EX_USAGE ;
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto retearly ;

badworking:
	ex = EX_SOFTWARE ;
	bprintf(efp,"%s: could not access the working directory \"%s\"\n",
	    g.progname,g.workdname) ;

	goto retearly ;

badconfig:
	ex = EX_SOFTWARE ;
	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
	    g.progname,rs,cf.badline) ;

	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen =  0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-C conf] [file [...]] ",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,"[-D[=n]]\n") ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


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



