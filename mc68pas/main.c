/* main (MC68PAS) */

/* PCS Configuration */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 1991-09-10, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This program is used either by programs or a user to retrieve
	the current PCS configuration settings from a PCS configuration
	file in the PCS distribution directory tree.

	Environment variables:

		M68TOOLS		root of program files

	Synopsis:

	$ mc68pas [-ROOT program_root] [-C conf] infile [outfile] [-V?]


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	USERINFO_LEN	(2 * 1024)
#define	CONFBUFLEN	(9 * 256)
#define	LINELEN		256


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	quoted(), expand() ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */


/* forwards */


/* local global variabes */


/* local structures */

/* define command option words */

static char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"CONFIG",
	"LOGFILE",
	NULL,
} ;

#define	ARGOPT_VERSION	0
#define	ARGOPT_VERBOSE	1
#define	ARGOPT_ROOT	2
#define	ARGOPT_CONFIG	3
#define	ARGOPT_LOGFILE	4

/* define the configuration keywords */
static char *confopts[] = {
	"mailhost",
	"mailnode",
	"maildomain",
	"uucphost",
	"userhost",
	"relay",
	"gateway",
	NULL,
} ;

#define	CONFOPT_MAILHOST	0
#define	CONFOPT_MAILNODE	1
#define	CONFOPT_MAILDOMAIN	2
#define	CONFOPT_UUCPHOST	3
#define	CONFOPT_USERHOST	4
#define	CONFOPT_RELAY		5
#define	CONFOPT_GATEWAY		6


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct ustat		sb ;

	struct proginfo		g, *pip = &g ;

	USERINFO	u ;

	bfile		errfile, *efp = &errfile ;
	bfile		infile , *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	time_t	daytime ;

	int	argr, argl, aol, akl, avl ;
	int	maxai, pan, npa, kwi, i, j, k, l ;
	int	argnum ;
	int	es = ES_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	len, srs, rs ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_quote = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	confbuf[CONFBUFLEN + 1] ;
	char	linebuf[LINELEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*configfname = NULL ;
	const char	*logfname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*filename = "stdin.s" ;
	const char	*cp, *cp1, *cp2, *cp3 ;


	if (u_fstat(3,&sb) >= 0) 
		debugsetfd(3) ;

	if (bopen(efp,BFILE_ERR,"wca",0666) < 0) 
		return BAD ;

	memset(pip,0,sizeof(struct proginfo)) ;

	g.progname = strbasename(argv[0]) ;

/* initialize */

	g.f.quiet = FALSE ;
	g.f.verbose = FALSE ;

	g.debuglevel = 0 ;
	if ((g.programroot = getenv("M68TOOLS")) == NULL)
		g.programroot = M68TOOLS ;

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
	                    (cfdec(argp + 1,argl - 1,&argnum) < 0))
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

/* do we have a keyword match or should we assume only key letters? */

#if	CF_DEBUGS
	                debugprintf("main: about to check for key word match\n") ;
#endif

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("main: got an option keyword, kwi=%d\n",
	                        kwi) ;
#endif

	                    switch (kwi) {

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

/* the real file name */
	                        case 'f':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) filename = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            g.f.quiet = TRUE ;
	                            break ;

/* CPP preprocessing has been done */
	                        case 'p':
	                            g.f.cpp = TRUE ;
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


	if (g.debuglevel > 0) bprintf(efp,
	    "%s: finished parsing arguments\n",
	    g.progname) ;

	if (f_version) bprintf(efp,"%s: version %s\n",
	    g.progname,VERSION) ;

	if (f_usage) goto usage ;

	if (f_version) goto earlyret ;

	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debug level %d\n",
	        g.progname,g.debuglevel) ;

/* load the positional arguments */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (g.debuglevel > 0) debugprintf(
	            "main: got a positional argument i=%d pan=%d arg=%s\n",
	            i,pan,argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            ifname = (char *) argv[i] ;
	            break ;

	        case 1:
	            ofname = (char *) argv[i] ;
	            break ;

	        default:
	            bprintf(efp,
	                "%s: extra positional arguments ignored\n",
	                g.progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */


#ifdef	COMMENT

/* process the configuration file if we have one */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "main: checking for configuration file \n") ;
#endif

	if (access(configfname,R_OK) >= 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 0) debugprintf(
	        "main: we have a configuration file\n") ;
#endif

	    if ((rs = configinit(&cf,configfname)) < 0) goto badconfig ;

#if	CF_DEBUG
	    if (g.debuglevel > 0) debugprintf(
	        "main: we have a good configuration file\n") ;
#endif

	    if ((cf.directory != NULL) && (g.directory == NULL)) {

	        g.directory = cf.directory ;
	        cf.directory = NULL ;

	    }

	    if ((cf.interrupt != NULL) && (g.interrupt == NULL)) {

	        g.interrupt = cf.interrupt ;
	        cf.interrupt = NULL ;

	    }

	    if ((cf.workdir != NULL) && (g.workdir == NULL)) {

	        g.workdir = cf.workdir ;
	        cf.workdir = NULL ;

	    }

	    if ((cf.srvtab != NULL) && (g.srvtab == NULL)) {

	        g.srvtab = cf.srvtab ;
	        cf.srvtab = NULL ;

	    }

	    if ((cf.pidfile != NULL) && (g.pidfile == NULL)) {

	        g.pidfile = cf.pidfile ;
	        cf.pidfile = NULL ;

	    }

	    if ((cf.logfname != NULL) && (logfname == NULL)) {

	        logfname = cf.logfname ;
	        cf.logfname = NULL ;

	    }

	    if ((cf.polltime > 1) && (g.polltime < 1))
	        g.polltime = cf.polltime ;

	    for (i = 0 ; (rs = vecstrget(&cf.exports,i,&sp)) >= 0 ; i += 1)
	        vecstradd(&g.exports,sp,-1) ;

	    for (i = 0 ; (rs = vecstrget(&cf.paths,i,&sp)) >= 0 ; i += 1)
	        vecstradd(&g.paths,sp,-1) ;

	    configfree(&cf) ;

	} /* end if */

#endif /* COMMENT */


	g.pid = getpid() ;

/* do we have an activity log file? */

	userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	mkpath2(tmpfname,g.programroot,LOGFILE) ;

	rs = logfile_open(&g.lh,tmpfname,0,0666,u.logid) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	debugprintf("main: log file rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

	    time(&daytime) ;

	    logfile_printf(&g.lh,"%s %-14s %s/%s\n",
		timestr_log(daytime,timebuf),
	        g.progname,
	        VERSION,(u.f.sysv_ct) ? "SYSV" : "BSD") ;

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

		logfile_printf(&g.lh,"source=%s\n",
		((filename != NULL) && (filename[0] != '\0')) ?
		filename : "stdin.s") ;

	} /* end if (making log entries) */


/* open the necessary files */

	if ((ifname != NULL) && (ifname[0] != '\0'))
	    rs = bopen(ifp,ifname,"r",0666) ;

	else
	    rs = bopen(ifp,BFILE_STDIN,"rd",0666) ;

	if (rs < 0) goto badinopen ;


	if ((ofname != NULL) && (ofname[0] != '\0'))
	    rs = bopen(ifp,ofname,"r",0666) ;

	else
	    bopen(ofp,BFILE_STDOUT,"rd",0666) ;

	if (rs < 0) goto badoutopen ;



/* go for it ! */

	bprintf(ofp,"\tfile \"%s\"\n",filename) ;

	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	    if (linebuf[len - 1] == '\n') len -= 1 ;

	    linebuf[len] = '\0' ;

	    for (i = 0 ; i < len ; i += 1) {

	        switch (linebuf[i]) {

	        case '\'':
	            if (! f_quote)
	                i += 1 ;

	            break ;

	        case '"':
	            if (! f_quote)
	                f_quote = TRUE ;

	            else
	                f_quote = FALSE ;

	            break ;

	        case ';':
	            if (! f_quote)
	                len = i ;

	            break ;

	        case '#':
	            if (! f_quote) {

	                len = i ;
	                if (i == 0) {

	                    cp1 = linebuf + 1 ;
	                    while (CHAR_ISWHITE(*cp1)) cp1 += 1 ;

	                    cp2 = cp1 ;
	                    while (*cp2 && (! CHAR_ISWHITE(*cp2)))
	                        cp2 += 1 ;

	                    if (*cp2 != '\0') *cp2++ = '\0' ;

	                    while (CHAR_ISWHITE(*cp2)) cp2 += 1 ;

	                    if (*cp2 == '"') cp2 += 1 ;

	                    cp3 = cp2 ;
	                    while (*cp3 && (! CHAR_ISWHITE(*cp3)) &&
	                        (*cp3 != '"'))
	                        cp3 += 1 ;

	                    *cp3 = '\0' ;

#if	CF_DEBUG
	                    if (g.debuglevel > 0)
	                        debugprintf("main: cp2=%s\n",cp2) ;
#endif
	                    if (((cp = strrchr(cp2,'.')) != NULL) &&
	                        (cp[1] == 'c') && (cp[2] == '\0'))
	                        cp2 = filename ;

	                    else if ((cp2[0] == '.') &&
	                        (cp2[1] == '/'))
	                        cp2 += 2 ;

#ifdef	COMMENT
	                    bprintf(ofp,"\tfile \"%s\"\n",
	                        cp2) ;
#endif

	                    bprintf(ofp,"\tln %s\n",
	                        cp1) ;

	                    len = -1 ;

	                } /* end if (CPP directive) */

	            } /* end if (not inside of a quoted string) */

	            break ;

	        } /* end switch */

	    } /* end for */

	    if (len >= 0) {

	        linebuf[len++] = '\n' ;
	        bwrite(ofp,linebuf,len) ;

	    }

	} /* end while (reading lines) */


/* we are done */

#if	CF_DEBUG
	if (g.debuglevel < 0)
	    bprintf(efp,"%s: program finishing\n",
	        g.progname) ;
#endif

	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

earlyret:
	bclose(efp) ;

	return OK ;

/* error types of returns */
badret:
	bclose(efp) ;

	return BAD ;

/* USAGE> dwd [-C conf] [-polltime] [directory_path] [srvtab] [-V?] */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-C conf] infile [outfile] [-?v] ",
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

#ifdef	COMMENT

badconfig:
	bprintf(efp,
	    "%s: error (rs %d) in configuration file starting at line %d\n",
		    g.progname,rs,cf.badline) ;

		goto badret ;

	#endif

	badarg:
		bprintf(efp,"%s: bad argument(s) given\n",
		    g.progname) ;

		goto badret ;

	badinopen:
		bprintf(efp,"%s: could not open the input file (rs %d)\n",
		    g.progname,rs) ;

		goto badret ;

	badoutopen:
		bprintf(efp,"%s: could not open the output file (rs %d)\n",
		    g.progname,rs) ;

		goto badret ;

	}
	/* end subroutine (main) */



