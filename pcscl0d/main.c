/* main */

/* main subroutine for the PCSCL program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1994-09-01, David A­D­ Morano

	This subroutine was originally written.


	= 1997-05-03, David A­D­ Morano

	This subroutine was enhanced to take multiple input files.


*/

/* Copyright © 1994,1997 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	Description:

	This is the main subroutine for the 'pcscl' program.  This is
	pretty much a front-end subroutine modified for PCSCL.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<sys/utsname.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<varsub.h>
#include	<hdb.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<localmisc.h>

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

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX(BUFLEN,2048)
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	expander(struct proginfo *,const char *,int,char *,int) ;
extern int	process() ;

extern char	*strbasename(char *), *strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forward references */

static char	*filereadable() ;


/* local structures */

static const char *argopts[] = {
	"ROOT",
	"CONFIG",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_config,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_of,
	argopt_overlast
} ;

static const char	*tmpdirs[] = {
	"/tmp",
	"/var/tmp",
	"/var/spool/uucppublic",
	".",
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo		pi, *pip = &pi ;

	struct userinfo		u ;

	struct configfile	cf ;

	varsub	vsh_e, vsh_d ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos, maxai ;
	int	argvalue = -1 ;
	int	pan, npa ;
	int	rs, i, len ;
	int	loglen = -1 ;
	int	vl, vl2 ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_append = TRUE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *linebuf = buf ;
	char	buf2[BUFLEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*configfname = NULL ;
	const char	*logfname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*helpfname = NULL ;
	const char	*postspec = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->version = VERSION ;
	pip->progname = strbasename(argv[0]) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    pip->f.stderror = TRUE ;
	    bcontrol(pip->efp,BC_LINEBUF,0) ;
	}

	pip->tmpdname = getenv("TMPDIR") ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->f.quiet = FALSE ;
	pip->f.log = FALSE ;
	pip->f.seekable = FALSE ;
	pip->f.wantenvelope = FALSE ;


/* early initialization processing */

	helpfname = NULL ;


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
	                    (cfdeci((argp + 1),(argl - 1),&argvalue) < 0))
	                    goto badargval ;

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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

	                    switch (kwi) {

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

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->pr = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->pr = argp ;

	                        }

	                        break ;

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

/* log file */
	                    case argopt_logfile:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					logfname = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					logfname = argp ;

	                        }

	                        break ;

/* output file */
	                    case argopt_of:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					ofname = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					ofname = argp ;

	                        }

	                        break ;

/* print out the help */
	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* handle all keyword defaults */
	                    default:
				ex = EX_USAGE ;
				f_usage = TRUE ;
	                        bprintf(pip->efp,
					"%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

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
	                                if (avl) {

	                                    rs = cfdec(avp,avl,
							&pip->debuglevel) ;

						if (rs < 0)
	                                    goto badargval ;

					}
	                            }

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

/* allow the specification of a mailer if there isn't any */
	                        case 'M':
	                        case 'm':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->header_mailer = argp ;

	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* specify an article ID */
	                        case 'a':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->header_article = argp ;

	                            break ;

/* force an envelope on the output */
	                        case 'e':
	                            pip->f.wantenvelope = TRUE ;
	                            break ;

/* the "from" address for the envelope, if specified, or whatever */
	                        case 'f':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->address_from = argp ;

	                            break ;

/* add a "newsgroups" header */
	                        case 'n':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->header_newsgroups = argp ;

	                            break ;

/* specified a file to read filename arguments from */
	                        case 'k':
	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					afname = argp ;

	                            break ;

/* supply a "received" post-mark */
	                        case 'p':
	                            pip->f.postmark = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
					postspec = avp ;

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

					if (rs < 0)
						goto badargval ;

				    }
	                        }

	                            break ;

/* print usage summary */
	                        case '?':
	                            f_usage = TRUE ;
					break ;

	                        default:
					f_usage = TRUE ;
					ex = EX_USAGE ;
	                            bprintf(pip->efp,
					"%s: unknown option - %c\n",
	                                pip->progname,*aop) ;

/* fall through to next case */

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
	                bprintf(pip->efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif


	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto retearly ;

	if (pip->debuglevel > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 0)
	        debugprintf("main: debuglevel %d\n",pip->debuglevel) ;
#endif

	    bprintf(pip->efp,"%s: debug level %d\n",
	        pip->progname,pip->debuglevel) ;

	    bflush(pip->efp) ;

	}


/* get our program root */

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT1) ;

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT2) ;

	if (pip->pr == NULL)
	    pip->pr = getenv(VARPROGRAMROOT3) ;

	if (pip->pr == NULL)
	    pip->pr = PROGRAMROOT ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: initial programroot=%s\n",pip->pr) ;
#endif


/* get some host/user information */

	if ((rs = userinfo(&u,userbuf,USERINFO_LEN,NULL)) < 0) {

	    if ((pip->debuglevel > 0) && (! pip->f.quiet))
	        bprintf(pip->efp,"%s: could not get user information\n",
	            pip->progname) ;

	}

	pip->up = &u ;
	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;


/* find a configuration file if we have one */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		debugprintf("main: checking for configuration file\n") ;
#endif

/* search locally */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR1,CONFIGFNAME1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR1,CONFIGFNAME2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        NULL,ETCDIR2,CONFIGFNAME1) ;

	if (configfname == NULL) {

	    configfname = CONFIGFNAME1 ;
	    if (u_access(configfname,R_OK) < 0)
	        configfname = NULL ;

	}

	if (configfname == NULL) {

	    configfname = CONFIGFNAME2 ;
	    if (u_access(configfname,R_OK) < 0)
	        configfname = NULL ;

	}

/* search in our program root area */

	if ((configfname == NULL) || (configfname[0] == '\0'))
	    configfname = filereadable(tmpfname,
	        pip->pr,ETCDIR1,CONFIGFNAME1) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        pip->pr,ETCDIR1,CONFIGFNAME2) ;

	if (configfname == NULL)
	    configfname = filereadable(tmpfname,
	        pip->pr,ETCDIR2,CONFIGFNAME1) ;


/* read in the configuration file if we have one */

	if (u_access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "main: we have a configuration file \"%s\"\n",
	        configfname) ;
#endif

	    rs = configfile_start(&cf,configfname) ;

	    if (rs < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		debugprintf("main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "main: varsub_start d\n") ;
#endif

	    varsub_start(&vsh_d,0) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		debugprintf("main: varsub_start e\n") ;
#endif

	    varsub_start(&vsh_e,0) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
		debugprintf("main: varsub_start\n") ;
#endif

	    varsub_addva(&vsh_e,(const char **) envv) ;


	    if ((cf.root != NULL) && (pip->pr == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF programroot=%s\n",
	                cf.root) ;
#endif

	        if (((vl = varsub_subbuf(&vsh_d,&vsh_e,cf.root,
	            -1,buf,BUFLEN)) > 0) &&
	            ((vl2 = expander(pip,buf,vl,buf2,BUFLEN)) > 0)) {

	            proginfo_setprogroot(pip,buf2,vl2) ;

	        }

	        cf.root = NULL ;

	    } /* end if (programroot) */

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((vl = varsub_subbuf(&vsh_d,&vsh_e,cf.logfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((vl2 = expander(pip,buf,vl,buf2,BUFLEN)) > 0)) {

	            logfname = mallocstrw(buf2,vl2) ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",logfname) ;
#endif

	        cf.logfname = NULL ;

	    } /* end if (configuration file log filename) */


	    if ((cf.helpfname != NULL) && (helpfname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF helpfname=%s\n",cf.helpfname) ;
#endif

	        if (((vl = varsub_subbuf(&vsh_d,&vsh_e,cf.helpfname,
	            -1,buf,BUFLEN)) > 0) &&
	            ((vl2 = expander(pip,buf,vl,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: helpfname subed and expanded\n") ;
#endif

	            helpfname = mallocstrw(buf2,vl2) ;

	        }

	        cf.helpfname = NULL ;

	    } /* end if (helpfname) */


	    if ((cf.tmpdir != NULL) && (pip->tmpdname == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF tmpdir=%s\n",cf.tmpdir) ;
#endif

	        if (((vl = varsub_subbuf(&vsh_d,&vsh_e,cf.tmpdir,
	            -1,buf,BUFLEN)) > 0) &&
	            ((vl2 = expander(pip,buf,vl,buf2,BUFLEN)) > 0)) {

	            pip->tmpdname = mallocstrw(buf2,vl2) ;

	        }

	        cf.tmpdir = NULL ;

	    } /* end if (tmpdir) */


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing CF loglen\n") ;
#endif

	    if ((cf.loglen >= 0) && (loglen < 0))
	        loglen = cf.loglen ;

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
	} /* end if (accessed the configuration file) */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
	    debugprintf("main: finished with any configfile stuff\n") ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: final programroot=%s\n",pip->pr) ;
#endif

/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		debugprintf("main: checking program parameters\n") ;
#endif


	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    for (i = 0 ; tmpdirs[i] != NULL ; i += 1) {

	        if (u_access(tmpdirs[i],W_OK | R_OK | X_OK) >= 0)
	            break ;

	    } /* end for */

	    if (tmpdirs[i] != NULL)
	        pip->tmpdname = (char *) tmpdirs[i] ;

	} /* end if */

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0'))
	    pip->tmpdname = TMPDNAME ;

/* do we have an activity log file? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs = BAD ;
	if ((logfname == NULL) || (logfname[0] == '\0'))
	    logfname = LOGFNAME ;

	if ((logfname[0] == '/') || (u_access(logfname,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,logfname,0,0666,u.logid) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if ((rs < 0) && (logfname[0] != '/')) {

	    mkpath2(tmpfname,pip->pr,logfname) ;

	    rs = logfile_open(&pip->lh,tmpfname,0,0666,u.logid) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 logfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

	if (rs >= 0) {

		struct utsname	un ;


	    pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we opened a logfile\n") ;
#endif

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

		(void) u_uname(&un) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s) pid=%d\n",
	        u.f.sysv_rt ? "SYSV" : "BSD",
		un.sysname,un.release,
		u.pid) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
	        u.nodename,u.username,buf) ;

	} /* end if (we have a log file or not) */

/* print out the helpfname if requested */

	if (f_help) {

		if ((helpfname == NULL) || (helpfname[0] == '\0'))
			helpfname = HELPFNAME ;

		printhelp(NULL,pip->pr,SEARCHNAME,helpfname) ;

	    goto done ;

	} /* end if (help file) */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: address_from=%s\n",pip->address_from) ;
#endif

/* some more initialization */


/* who did we get called by? */

	pip->r_transport = NULL ;
	pip->r_machine = NULL ;
	pip->r_user = NULL ;


/* were we told directly? */

	if (postspec != NULL) {

		int	sl ;

		char	*sp ;


		if ((cp = strchr(postspec,':')) != NULL) {

		sl = sfshrink(postspec,(cp - postspec),&sp) ;

		pip->r_transport = mallocstrw(sp,sl) ;

		if (cp[1] != '\0') {

		sl = sfshrink(cp + 1,-1,&sp) ;

		pip->r_machine = mallocstrw(sp,sl) ;

		}

		} else {

		sl = sfshrink(postspec,-1,&sp) ;

		pip->r_transport = mallocstrw(sp,sl) ;

		}

	} /* end if (protocol information) */

/* called by UUCP? */

	if ((cp = getenv("UU_MACHINE")) != NULL) {

		if (pip->r_transport != NULL)
	    pip->r_transport = "UUCP" ;

		if (pip->r_machine != NULL)
	    pip->r_machine = cp ;

	    if ((cp = getenv("UU_USER")) != NULL)
	        pip->r_user = cp ;

	} /* end if (UUCP transport detected) */


/* called by RCMD? */

	if ((cp = getenv("RCMD_MACHINE")) != NULL) {

		if (pip->r_transport != NULL)
	    pip->r_transport = "RCMD" ;

		if (pip->r_machine != NULL)
	    pip->r_machine = cp ;

	    if ((cp = getenv("RCMD_USER")) != NULL)
	        pip->r_user = cp ;

	} /* end if (RCMD transport detected) */


/* log where we came from */

	if (pip->r_transport != NULL) {

	    logfile_printf(&pip->lh,"r_transport=%s\n",
	        pip->r_transport) ;

		if (pip->r_machine != NULL)
	    logfile_printf(&pip->lh,"r_machine=%s\n",
	        pip->r_machine) ;

	    if (pip->r_user != NULL)
	        logfile_printf(&pip->lh,"r_user=%s\n",
	            pip->r_user) ;

	} /* end if */


/* create a default envelope "from" address if need be */

	if ((pip->address_from == NULL) && (pip->r_user != NULL))
	    pip->address_from = pip->r_user ;

	if (pip->address_from == NULL)
	    pip->address_from = u.username ;



/* open the output key file */

	strcpy(buf,"wc") ;

	if (f_append)
	    strcat(buf,"a") ;

	if ((ofname == NULL) || (ofname[0] == '\0')) {

	    strcat(buf,"d") ;

	    rs = bopen(ofp,BFILE_STDOUT,buf,0666) ;

	} else
	    rs = bopen(ofp,ofname,buf,0666) ;

	if (rs < 0)
		goto badoutopen ;

	if (bseek(ofp,0L,SEEK_CUR) >= 0)
	    pip->f.seekable = TRUE ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

	    int	tmpfd, o_flags ;


	    bcontrol(ofp,BC_GETFL,&o_flags) ;

	    debugprintf("main: o_flags=%08X\n",o_flags) ;

	    if (o_flags & O_APPEND)
	        debugprintf("main: 1 file in APPEND mode\n") ;

	    bcontrol(ofp,BC_FD,&tmpfd) ;

	    o_flags = u_fcntl(tmpfd,F_GETFL,0) ;

	    if (o_flags & O_APPEND)
	        debugprintf("main: 2 file in APPEND mode\n") ;

	    if (f_append) {

	        o_flags |= O_APPEND ;
	        u_fcntl(tmpfd,F_SETFL,o_flags) ;

	        if ((o_flags = u_fcntl(tmpfd,F_GETFL,0)) >= 0) {

	            if (o_flags & O_APPEND)
	                debugprintf("main: 3 file in APPEND more\n") ;

	        }

	    }

	} /* end if (debug) */
#endif /* CF_DEBUG */


/* capture a lock on the output file */

	rs = bcontrol(ofp,BC_LOCKWRITE,LOCKTIMEOUT) ;

	if (rs < 0)
	    goto badlock ;


/* if output file is seekable, check for a new-line as the last character */

	if (f_append && pip->f.seekable) {

		struct ustat	sb ;

		int		ch ;


		bcontrol(ofp,BC_STAT,&sb) ;

		if (sb.st_size > 0) {

		bseek(ofp,-1L,SEEK_END) ;

		if ((ch = bgetc(ofp)) != '\n')
			bputc(ofp,'\n') ;

		} /* end if (non-zero length already) */

	} /* end if (checking for new-line character) */




	pan = 0 ;

	if ((npa > 1) || (afname != NULL))
	    pip->f.wantenvelope = TRUE ;


/* process the positional arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 1) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i) && (strlen(argv[i]) > 0)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: processing input file \"%s\"\n",
	                pip->progname,argv[i]) ;


	        rs = process(pip,ofp, argv[i]) ;

		if (rs < 0) {

	            cp = argv[i] ;
	            if (*cp == '-') 
			cp = "*stdinput*" ;

	            bprintf(pip->efp,
				"%s: error processing input file, rs=%d\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: errored file=%s\n",
	                pip->progname,cp) ;

	        } /* end if (processing a file) */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (processing positional arguments) */


/* process any files in the argument filename list file */

	if ((afname != NULL) && (afname[0] != '\0')) {

	bfile		argfile, *afp = &argfile ;


	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {

	        while ((len = breadline(afp,linebuf,BUFLEN)) > 0) {

	            if (linebuf[len - 1] == '\n') 
			len -= 1 ;

	            linebuf[len] = '\0' ;
	            cp = strshrink(linebuf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            if ((rs = process(pip,ofp, cp)) < 0) {

	                if (*cp == '-') 
				cp = "*stdinput*" ;

	                bprintf(pip->efp,
	                    "%s: error processing input file, rs=%d\n",
	                    pip->progname,rs) ;

	                bprintf(pip->efp,"%s: errored file=%s\n",
	                    pip->progname,cp) ;

	            } /* end if (processing a file) */

	            pan += 1 ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

	    } else {

	        bprintf(pip->efp,
	            "%s: could not open the argument list file\n",
	            pip->progname) ;

	        bprintf(pip->efp,"\trs=%d argfile=%s\n",rs,afname) ;

	    }

	} 

	if ((rs >= 0) && (pan == 0)) {

/* process STANDARD INPUT if we are supposed to */

	    if ((rs = process(pip,ofp,"-")) < 0) {

	        cp = argv[i] ;
	        if (*cp == '-') 
			cp = "*stdinput*" ;

	        bprintf(pip->efp,"%s: error processing input file, rs=%d\n",
	            pip->progname,rs) ;

	        bprintf(pip->efp,"%s: errored file=%s\n",
	            pip->progname,cp) ;

	    } /* end if (processing a file) */

	    pan += 1 ;

	} /* end if */


/* close the output file */

	bclose(ofp) ;


	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: %d files processed\n",
	        pip->progname,pan) ;


/* we are done */
done:
ret5:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf(pip->efp,"main: program finishing\n") ;
#endif

ret4:
	if ((configfname != NULL) && (configfname[0] != '\0'))
	    configfile_finish(&cf) ;

ret3:
	if (pip->f.log)
	logfile_close(&pip->lh) ;

retearly:
ret1:
	bclose(pip->efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [-C conf] [file(s) ...] [-?vV]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,
		"\t[-e] [-f from_address] [-o mailbox] [-HELP] [-D[=n]]\n") ;

	bprintf(pip->efp,
		"\t[-M mailer] [-n newsgroups] [-a article-id]\n") ;

	goto retearly ;

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

/* all bad argument usage */
badarg:
	EX_USAGE ;
	goto badret ;

/* bad stuff */
badconfig:
	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badoutopen:
	bprintf(pip->efp,"%s: could not open output (key) file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badlock:
	bprintf(pip->efp,
	    "%s: could not capture lock on output file, (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto done ;

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



