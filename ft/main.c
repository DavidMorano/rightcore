/* main */

/* File Transfer (FT) program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1995-05-01, David A­D­ Morano

	This code module was written originally.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<bfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<field.h>
#include	<pcsconf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matostr(const char **,int,const char *,int) ;

extern char	*strbasename() ;
extern char	*timestrjul() ;
extern char	*malloc_str() ;


/* local forward references */


/* external variables */


/* global variables */

struct global		g ;


/* local data */

/* define command option words */

static char *argopts[] = {
	"ROOT",
	"DEBUG"
	"VERSION",
	"TMPDIR",
	NULL
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_TMPDIR		3



/* options specified in the 'FTOPTS' environment variable */

static char *ftopts[] = {
	"fastscan",
	NULL,
} ;


#define	FTOPT_FASTSCAN	0


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;

	struct sigaction	ss ;

	sigset_t	signalmask ;

	struct ustat	sb ;

	struct tm	ts, *timep ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	ngi, i, j ;
	int	fd, rs ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	l, oi ;
	int	err_fd ;

	char	*argp, *aop, *akp, *avp ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	fname_buf[MAXPATHLEN + 1] ;
	char	buf[BUFSIZE + 1] ;
	char	*cp ;
	char	argpresent[MAXARGGROUPS + 1] ;


	g.progname = strbasename(argv[0]) ;

	bopen(efp,BFILE_STDERR,"wca",0644) ;

	if (((cp = getenv(ERRORFDVAR)) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	g.pcs = NULL ;
	g.efp = efp ;

/* early things to initialize */

	g.ofp = ofp ;
	g.ifp = ifp ;
	g.ufp = ufp ;

	g.f.debug = FALSE ;
	g.f.verbose = FALSE ;
	g.f.exit = FALSE ;
	g.f.old = FALSE ;
	g.f.all = FALSE ;
	g.f.every = FALSE ;
	g.f.reverse = FALSE ;
	g.f.newprogram = FALSE ;
	g.f.interactive = TRUE ;
	g.f.mailbox = FALSE ;
	g.f.catchup = FALSE ;
	g.f.nopage = FALSE ;
	g.f.combine = FALSE ;
	g.f.query = FALSE ;
	g.f.newmessages = FALSE ;
	g.f.description = FALSE ;

#if	defined(SYSV)
	g.f.sysv_ct = TRUE ;
#else
	g.f.sysv_ct = FALSE ;
#endif

	g.f.sysv_rt = FALSE ;
	if (access("/usr/sbin",R_OK) >= 0) g.f.sysv_rt = TRUE ;

	if (g.progname[0] == 'n') g.f.newprogram = TRUE ;


/* get user profile information */

	if ((rs = userinfo(&u,userbuf,USERINFO_LEN,NULL)) < 0) 
		goto baduser ;

	g.up = &u ;

#if	CF_DEBUGS
	debugprintf("main: user=%s homedir=%s\n",
	    u.username,u.homedname) ;
#endif


/* get the current time-of-day */

	(void) time(&g.daytime) ;

#ifdef	SYSV
	timep = (struct tm *) localtime_r((time_t *) &g.daytime,&ts) ;
#else
	timep = (struct tm *) localtime((time_t *) &g.daytime) ;
#endif


/* are we running with some sort of prefix to our program name? */

	strcpy(buf,g.progname) ;

	if ((cp = strchr(buf,'.')) != NULL) *cp = '\0' ;

	g.prefix = "" ;
	cp = buf ;
	if (strncmp(buf,"nbb",3) == 0)
	    g.prefix = "n" ;

	else if (strncmp(buf,"obb",3) == 0)
	    g.prefix = "o" ;

	else if (strncmp(buf,"bb",2) != 0)
	    cp = "BB" ;

	g.progname = malloc_str(cp) ;


/* initialize some stuff before command line argument processing */

	g.newsdir = NULL ;
	g.prog_editor = PROG_EDITOR ;
	g.prog_mailer = PROG_MAILER ;
	g.prog_metamail = PROG_METAMAIL ;
	g.prog_metamail = NULL ;
	g.progmode = -1 ;
	g.debuglevel = 0 ;
	g.termlines = -1 ;
	g.termtype = NULL ;
	if ((cp = getenv("TERM")) != NULL) g.termtype = cp ;


/* process program arguments */

	rs = SR_OK ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_done) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program root */
	                case ARGOPT_ROOT:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.pcs = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.pcs = argp ;

	                    }

	                    break ;

	                case ARGOPT_TERM:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.termtype = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.termtype = argp ;

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

/* "date" */
	                case ARGOPT_DATE:
	                    if (g.progmode < 0) {

	                        g.progmode = PM_HEADER ;
	                        g.header = HI_DATE ;

	                    }

	                    break ;

/* "from" */
	                case ARGOPT_FROM:
	                    if (g.progmode < 0) {

	                        g.progmode = PM_HEADER ;
	                        g.header = HI_FROM ;

	                    }
	                    break ;

/* "subject" and the old "titles" */
	                case ARGOPT_TITLES:
	                case ARGOPT_SUBJECT:
	                    if (g.progmode < 0) {

	                        g.progmode = PM_HEADER ;
	                        g.header = HI_SUBJECT ;

	                    }
	                    break ;

/* "old_titles" */
	                case ARGOPT_OLDTITLES:
	                    if (f_optplus) {

	                        if (g.progmode < 0) {

	                            g.progmode = PM_HEADER ;
	                            g.f.all = TRUE ;

	                        }

	                    } else
	                        g.f.old = TRUE ;

	                    break ;

/* "count" */
	                case ARGOPT_COUNT:
	                    if (g.progmode < 0)
	                        g.progmode = PM_COUNT ;

	                    else if ((g.progmode == PM_NAMES) &&
	                        g.f.description)
	                        g.progmode = PM_COUNT ;

	                    break ;

/* "query" */
	                case ARGOPT_QUERY:
	                    if (g.progmode < 0)
	                        g.progmode = PM_COUNT ;

	                    g.f.query = TRUE ;
	                    break ;

/* "all_bulletins" */
	                case ARGOPT_ALL:
	                    g.f.all = TRUE ;
	                    break ;

/* "names" */
/* "boards" */
	                case ARGOPT_NAMES:
	                case ARGOPT_BOARDS:
	                case ARGOPT_NEWSGROUPS:
	                    if (g.progmode < 0)
	                        g.progmode = PM_NAMES ;

	                    break ;

/* "every_board" */
	                case ARGOPT_EVERY:
	                    g.f.every = TRUE ;
	                    break ;

/* "reverse" */
	                case ARGOPT_REVERSE:
	                    g.f.reverse = TRUE ;
	                    break ;

/* "interactive" */
	                case ARGOPT_INTERACTIVE:
	                    g.f.interactive = FALSE ;
	                    if (f_optplus)
	                        g.f.interactive = TRUE ;

	                    break ;

/* "nopage" */
	                case ARGOPT_NOPAGE:
	                    g.f.nopage = FALSE ;
	                    if (f_optplus)
	                        g.f.nopage = TRUE ;

	                    break ;

/* mailbox option for BBR (and smart users) */
	                case ARGOPT_MAILBOX:
	                    g.progmode = PM_READ ;
	                    g.f.mailbox = TRUE ;
	                    break ;

/* catchup option (really quite similar to the "mailbox" option) */
	                case ARGOPT_CATCHUP:
	                    g.progmode = PM_READ ;
	                    g.f.catchup = TRUE ;
	                    break ;

/* subscription changes */
	                case ARGOPT_SUBSCRIBE:
	                    g.progmode = PM_SUBSCRIPTION ;
	                    g.f.subscribe = TRUE ;
	                    break ;

/* subscription changes */
	                case ARGOPT_UNSUBSCRIBE:
	                    g.progmode = PM_SUBSCRIPTION ;
	                    g.f.subscribe = FALSE ;
	                    break ;

/* print out newsgroup descriptions */
	                case ARGOPT_DESCRIPTION:
	                    if (g.progmode < 0)
	                        g.progmode = PM_NAMES ;

	                    g.f.description = TRUE ;
	                    break ;

/* editor program */
	                case ARGOPT_EDITOR:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.prog_editor = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.prog_editor = argp ;

	                    }

	                    break ;

/* mailer program */
	                case ARGOPT_MAILER:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.prog_mailer = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.prog_mailer = argp ;

	                    }

	                    break ;

/* metamail program */
	                case ARGOPT_METAMAIL:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.prog_metamail = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.prog_metamail = argp ;

	                    }

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    f_done = TRUE ;
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'v':
	                        g.f.verbose = TRUE ;
	                        break ;

	                    case 'D':
	                        g.f.debug = TRUE ;
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl, &g.debuglevel) < 0)
	                                goto badargvalue ;

	                        }

	                        break ;

/* number of terminal lines */
	                    case 'L':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if ((rs = cfdec(avp,avl,&g.termlines))
	                                != OK) goto badargvalue ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) if ((rs = 
	                                cfdec(argp,argl,&g.termlines))
	                                != OK) goto badargvalue ;

	                        }

	                        break ;

/* combine command output and regular message output to standard output */
	                    case 'C':
	                        g.f.combine = TRUE ;
	                        break ;

/* editor program */
	                    case 'E':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.prog_editor = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.prog_editor = argp ;

	                        }

	                        break ;

/* mailer program */
	                    case 'M':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.prog_mailer = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.prog_mailer = argp ;

	                        }

	                        break ;

/* new */
	                    case 'n':
	                        g.f.every = TRUE ;
	                        break ;

/* subject */
	                    case 's':
	                        g.f.every = TRUE ;
	                        break ;

/* count */
	                    case 'c':
	                        g.f.every = TRUE ;
	                        break ;

/* all */
	                    case 'a':
	                        g.f.all = TRUE ;
	                        break ;

/* old */
	                    case 'o':
	                        g.f.old = TRUE ;
	                        break ;

/* every */
	                    case 'e':
	                        g.f.every = TRUE ;
	                        break ;

/* reverse */
	                    case 'r':
	                        g.f.reverse = TRUE ;
	                        break ;

/* user's newsgroup list file */
	                    case 'u':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) ufname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ufname = argp ;

	                        }

	                        break ;

/* alternate newsgroup spool area */
	                    case 'N':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.newsdir = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.newsdir = argp ;

	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_OK ;
	                        bprintf(efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
			    if (rs < 0)
				break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGGROUPS) {

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


/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (g.f.debug) {

	    bprintf(g.efp,
	        "%s: debugging turned on to level %d\n",
	        g.progname,g.debuglevel) ;

	    bcontrol(g.efp,BC_LINEBUF,0) ;

	    bflush(g.efp) ;

	    logfile_printf(&g.lh,"debuglevel=%u\n",g.debuglevel) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s/%s\n",
	        g.progname,
	        VERSION,(g.f.sysv_ct ? "SYSV" : "BSD")) ;

	    bprintf(efp,"%s: m=%s os=%s u=%s\n",
	        g.progname,u.nodename,(g.f.sysv_rt ? "SYSV" : "BSD"),
	        u.username) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto done ;


/* some more general initialization */


/* get some program information */

	if (g.pcs == NULL) {

	    if ((g.pcs = getenv("PCS")) == NULL)
	        g.pcs = PCS ;

	}


/* help file */

	sprintf(fname_buf,"%s/%s",g.pcs,HELPFILE) ;

	g.helpfile = malloc_str(fname_buf) ;

/* log file */

	sprintf(fname_buf,"%s/%s",g.pcs,LOGFILE) ;

	g.logfile = malloc_str(fname_buf) ;

/* user list file */

	sprintf(fname_buf,"%s/%s",g.pcs,USERFILE) ;

	g.userfile = malloc_str(fname_buf) ;


/* make some log entries */

#if	CF_DEBUGS
	debugprintf("main: about to do 'logfile::open'\n") ;
#endif

	if ((rs = logfile_open(&g.lh,g.logfile,0,0666,u.logid)) < 0)
	    bprintf(g.efp,"%s: could not open the log file (rs %d)\n",
	        g.progname,rs) ;

	buf[0] = '\0' ;
	if ((u.name != NULL) && (u.name[0] != '\0'))
	    sprintf(buf,"(%s)",u.name) ;

	else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	    sprintf(buf,"(%s)",u.gecosname) ;

	else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	    sprintf(buf,"(%s)",u.fullname) ;

	else if (u.mailname != NULL)
	    sprintf(buf,"(%s)",u.mailname) ;

#if	CF_DEBUGS
	debugprintf("main: about to do 'logfile_printf'\n") ;
#endif

	logfile_printf(&g.lh,"%02d%02d%02d %02d%02d:%02d %-14s %s/%s\n",
	    timep->tm_year,
	    timep->tm_mon + 1,
	    timep->tm_mday,
	    timep->tm_hour,
	    timep->tm_min,
	    timep->tm_sec,
	    g.progname,
	    VERSION,(g.f.sysv_ct ? "SYSV" : "BSD")) ;

	logfile_printf(&g.lh,"os=%s %s!%s %s\n",
	    (g.f.sysv_rt ? "SYSV" : "BSD"),u.nodename,u.username,buf) ;

/* write user's mail address (roughly as we have it) into the user list file */

	f_listopen = TRUE ;
	if ((rs = bopen(ufp,g.userfile,"wa",0666)) < 0) {

	    if ((rs = bopen(ufp,g.userfile,"wca",0666)) >= 0)
	        bcontrol(ufp,BC_CHMOD,0666) ;

	    else
	        f_listopen = FALSE ;

	}

	if (f_listopen) {

	    bprintf(ufp,"%s!%s\n",u.nodename,u.username) ;

	    bclose(ufp) ;

	} else {

	    logfile_printf(&g.lh,"could not open the user_list file (rs %d)\n",
	        rs) ;

	}



	rs = pcsconf(g.pcs,NULL,&p,NULL,NULL,pcsconfbuf,PCSCONF_LEN) ;

	if (rs >= 0) {

	    if (p.mailhost != NULL)
	        g.mailhost = p.mailhost ;

	    if (p.fromnode != NULL)
	        g.fromnode = p.fromnode ;

	} else {

	    g.mailhost = MAILHOST ;
	    g.fromnode = MAILHOST ;

	}



/* HERE */

/* find the user's default newsgroup list file */

	cp = mkufname(u.homedname,ufname,fname_buf) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: cp=%s\n",cp) ;
#endif

	if ((cp == NULL) &&
	    ((ufname = getenv("BBNEWSRC")) != NULL)) {

	    cp = mkufname(u.homedname,ufname,fname_buf) ;

	}

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: cp=%s\n",cp) ;
#endif

	if (cp == NULL) {

	    cp = mkufname(u.homedname,DEFNEWSRC1,fname_buf) ;

	    if ((cp != NULL) && (access(cp,R_OK) < 0))
	        cp = NULL ;

	}

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: cp=%s\n",cp) ;
#endif

	if (cp == NULL) {

	    cp = mkufname(u.homedname,DEFNEWSRC2,fname_buf) ;

	    if ((cp != NULL) && (access(cp,R_OK) < 0))
	        cp = NULL ;

	}

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: cp=%s\n",cp) ;
#endif

	if (cp == NULL) {

	    cp = mkufname(u.homedname,DEFNEWSRC1,fname_buf) ;

	}

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: cp=%s\n",cp) ;
#endif

	if (cp == NULL) goto baduser ;

	g.ufname = malloc_str(cp) ;

	if (g.f.debug)
	    bprintf(g.efp,
	        "%s: user newsgroup list file \"%s\"\n",
	        g.progname,g.ufname) ;


/* newsgroup spool area */

#if	CF_DEBUG
	if (g.debuglevel > 2) debugprintf(
	    "main: newsdir 1 %s\n",g.newsdir) ;
#endif

	if (g.newsdir == NULL) {

	    if ((g.newsdir = getenv("BBDIR")) != NULL) {

	        if ((stat(g.newsdir,&sb) != 0) || (! S_ISDIR(sb.st_mode)))
	            g.newsdir = NULL ;

	    }

	}

	if (g.newsdir == NULL)
	    g.newsdir = getenv("BBNEWSDIR") ;

#if	CF_DEBUG
	if (g.debuglevel > 2) debugprintf(
	    "main: newsdir 2 %s\n",g.newsdir) ;
#endif

	if (g.newsdir == NULL) {

	    sprintf(fname_buf,"%s/%s",g.pcs,NEWSDIR) ;

	    g.newsdir = malloc_str(fname_buf) ;

	}

#if	CF_DEBUG
	if (g.debuglevel > 2) debugprintf(
	    "main: newsdir 3 %s\n",g.newsdir) ;
#endif

#ifdef	COMMENT
	if (g.debuglevel > 1) bprintf(g.efp,
	    "%s: newsgroup directory \"%s\"\n",
	    g.progname,g.newsdir) ;
#endif

	if ((stat(g.newsdir,&sb) != 0) || (! S_ISDIR(sb.st_mode)))
	    goto badnewsdir ;

	if ((access(g.newsdir,X_OK) != 0) || (access(g.newsdir,R_OK) != 0))
	    goto badnewsdir ;

	if (g.f.debug)
	    bprintf(g.efp,
	        "%s: newsgroup spool directory path \"%s\"\n",
	        g.progname,g.newsdir) ;


/* establish if we are running our output on a terminal or not ! */

	g.f.terminal = isatty(1) ;

	if ((g.debuglevel > 1) && g.f.terminal)
	    bprintf(g.efp,
	        "%s: output is on a temrinal\n",g.progname) ;

/* number of terminal lines */

	if (g.termlines < 0) {

	    if ((cp = getenv("LINES")) != NULL) {

	        if (cfdec(cp,strlen(cp),&g.termlines) < 0)
	            g.termlines = -1 ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: TL env %d\n",g.termlines) ;
#endif

	    }
	}

/* get the number of lines from the terminal driver, if present */

	if (g.f.terminal) {

/* get the number of lines from the driver, if it knows */

	    if (ioctl(1, TIOCGWINSZ, &ws) >= 0) {

	        if ((ws.ws_row > 0) && (g.termlines < 0))
	            g.termlines = ws.ws_row ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: TL driver %d\n",g.termlines) ;
#endif

	    }

/* setup the window change signal handler */

	    g.f_winchange = FALSE ;
	    (void) sigemptyset(&signalmask) ;

	    ss.sa_handler = int_winchange ;
	    ss.sa_mask = signalmask ;
	    ss.sa_flags = 0 ;
	    sigaction(SIGWINCH,&ss,NULL) ;

	} /* end if (we are on a terminal) */

/* OK, there were no other ways to get the number of lines */

	if (g.termlines < 0)
	    g.termlines = 24 ;

/* check if it was an OK number of terminal lines */

	if (g.termlines < 4) goto badtermlines ;

	if (g.f.debug)
	    bprintf(g.efp,
	        "%s: terminal lines %d\n",
	        g.progname,g.termlines) ;


	if ((rs = bopen(ofp,BFILE_STDOUT,"wct",0644)) < 0)
	    goto badoutopen ;


/* open a file to be used for command output, interactive or not */

	g.cfp = g.ofp ;


/* get some startup flags if there are any */

	g.f.extrascan = TRUE ;

#if	CF_DEBUG
	if (g.debuglevel > 1) debugprintf(
	    "main: checking for options\n") ;
#endif

	fieldterms(fterms,0,":") ;

	if ((cp = getenv("FTOPTS")) != NULL) {

#if	CF_DEBUG
	    if (g.debuglevel > 1) debugprintf(
	        "main: got some options >%s<\n",cp) ;
#endif

	    field_start(fbp,cp,-1) ;

	    while (fbp->rlen > 0) {

#if	CF_DEBUG
	        if (g.debuglevel > 1) debugprintf(
	            "main: top of options loop\n") ;
#endif

	        if ((l = field_get(fbp,fterms)) > 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 1) debugprintf(
	                "main: non-empty field\n") ;
#endif

	            f_optoff = FALSE ;
	            if ((fbp->fp[0] == '-') || 
	                (fbp->fp[0] == '+')) {

	                if (fbp->fp[0] == '-')
	                    f_optoff = TRUE ;

	                fbp->fp += 1 ;
	                fbp->flen -= 1 ;
	            }

	            if ((oi = matstr(bbopts,
	                fbp->fp,fbp->flen)) >= 0) {

#if	CF_DEBUG
	                if (g.debuglevel > 1) debugprintf(
	                    "main: valid option, oi=%d\n",oi) ;
#endif

	                switch (oi) {

	                case FTOPT_FASTSCAN:
	                    g.f.extrascan = FALSE ;

	                    if ((g.debuglevel > 1) || (g.f.verbose))
	                        bprintf(g.efp,
	                            "%s: FASTSCAN on\n",
	                            g.progname) ;

	                    logfile_printf(&g.lh,"FASTSCAN\n") ;

	                } /* end switch */

	            } /* end if (got a match) */

	        } /* end if (non-zero length field) */

	    } /* end while */

	    field_finish(fbp) ;

	} /* end if (getting startup flags) */


/* find the BBPOST program to use for posting later */

	if (g.prog_bbpost == NULL) {

	    if (g.f.newprogram) {

	        sprintf(buf,"n%s",PROG_BBPOST) ;

	        if (getfiledirs(NULL,buf,"x",NULL) >= 0)
	            g.prog_bbpost = malloc_str(buf) ;

	    }

	    if (g.prog_bbpost == NULL) {

	        if (getfiledirs(NULL,PROG_BBPOST,"x",NULL) >= 0)
	            g.prog_bbpost = PROG_BBPOST ;

	    }

	} /* end if */


/* initialize some stuff */

	if (g.progmode < 0) g.progmode = PM_READ ;

	if (g.f.debug)
	    bprintf(g.efp,
	        "%s: program mode %d\n",
	        g.progname,g.progmode) ;


	bull_bds = NULL ;
	user_bds = NULL ;
	namespace = NULL ;

	if (initialize(g.newsdir,bb_bds,bb_opts) < 0) goto badret ;

/* initialize bulletin board structure */

	if (initbulls() < 0) goto badret ;

	if ((rs = get_bds(g.newsdir,bb_bds)) < 0) goto badret ;

#if	CF_DEBUGS
	debugprintf("main: found newsgroups\n") ;
#endif

#if	CF_DEBUGS
	debugprintf("main: after mid-exit \n") ;
#endif


/* finally perform some processing */

	if (g.progmode == PM_READ) {

	    if ((rs = bopen(ifp,BFILE_STDIN,"r",0644)) < 0)
	        goto badinopen ;

	}


	if (npa > 2) {

#if	CF_DEBUGS
	    debugprintf("main: got user specified newsgroups\n") ;
#endif

	    if (g.f.debug) bprintf(g.efp,
	        "%s: processing user specified newsgroups\n",
	        g.progname) ;

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) ||
	            (argv[i][0] == '-') || (argv[i][0] == '+'))
	            continue ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf(
	                "main: processing newsgroup \"%s\"\n",
	                argv[i]) ;
#endif

	        f_found = FALSE ;
	        for (j = 0 ; j < nuboards ; j += 1) {

	            dsp = user_bds[j].dsp ;
	            if (bbcmp(argv[i],dsp->name) == 0) {

	                f_found = TRUE ;
	                rs = process(&shownp,g.progmode,j) ;

	                if (rs == EMIT_QUIT) break ;

	            }

	        } /* end for (inner) */

	        if (! f_found) {

	            bprintf(g.efp,
	                "%s: newsgroup \"%s\" is not accessible\n",
	                g.progname,argv[i]) ;

	        }

	        if (rs == EMIT_QUIT) break ;

	    } /* end for (outer) */

	} else {

#if	CF_DEBUGS
	    debugprintf("main: handling all newsgroups\n") ;
#endif

	    if (g.f.debug) bprintf(g.efp,
	        "%s: processing all subscribed newsgroups\n",
	        g.progname) ;

	    if (g.progmode != PM_SUBSCRIPTION) {

	        for (ngi = 0 ; ngi < nuboards ; ngi += 1) {

	            dsp = user_bds[ngi].dsp ;

#if	CF_DEBUG
	            if (g.f.debug) {

	                debugprintf("main: at bull \"%s\"\n",
	                    dsp->name) ;

	            }
#endif

	            if (dsp->f.show || g.f.every) {

#if	CF_DEBUG
	                if (g.f.debug)
	                    debugprintf("main: about to call process for \"%s\"\n",
	                        dsp->name) ;
#endif

	                rs = process(&shownp,g.progmode,ngi) ;

	                if (rs == EMIT_QUIT) break ;

	            } /* end if (show) */

	        } /* end for */

	    } else {

	        bprintf(g.efp,
	            "%s: subscription changes to ALL newsgroups",
	            g.progname) ;

	        bprintf(g.efp," is too dangerous !! (not allowed)\n") ;

	    }

	} /* end if (of just printing out articles as usual) */


#if	CF_DEBUGS
	debugprintf("main: program exiting OK\n") ;
#endif

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: exiting\n") ;
#endif

/* good return from program */
goodret:

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: exiting at 'goodret', efp=%08X\n",g.efp) ;
#else
	if (g.f.debug)
	    bprintf(g.efp,"%s: program exiting\n",
	        g.progname) ;
#endif

done:
	bclose(g.efp) ;

	return OK ;

/* come here for a bad return from the program */
badret:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	bclose(g.efp) ;

	return BAD ;

/* program usage */
usage:
	bprintf(g.efp,
	    "%s: USAGE> %s source [source(s) ..] destination",
	    g.progname,g.progname) ;

	bprintf(g.efp,
	    "\t[-vr]\n") ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badoutopen:
	bprintf(g.efp,"%s: could not open output (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badinopen:
	bprintf(g.efp,"%s: could not open standard input (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

baduser:
	bprintf(g.efp,"%s: could not get user information (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

baduserfile:
	bprintf(g.efp,
	    "%s: unreadable user's newsgroup list file \"%s\"\n",
	    g.progname) ;

	goto badret ;

badnewsdir:
	bprintf(g.efp,
	    "%s: could not access the newsgroup spool directory (errno=%d)\n",
	    g.progname,errno) ;

	goto badret ;

badtermlines:
	bprintf(g.efp,
	    "%s: not enough lines on the terminal to be useable, min=4\n",
	    g.progname) ;

	goto badret ;

}
/* end subroutine (main) */



