/* main */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_NOSEND	1


/************************************************************************
 *									
	= 94-01-06, David A­D­ Morano 

	This subroutine was adopted from the 'main' subroutine of the
	old SENDMAIL program.

*
************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<errno.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<signal.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<logfile.h>
#include	<bfile.h>
#include	<char.h>
#include	<ascii.h>
#include	<userinfo.h>
#include	<pcsconf.h>
#include	<vecstr.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"prompt.h"
#include	"header.h"



/* local subroutine defines */

#define		MAXARGINDEX	100
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	bopencmd() ;

extern char	*getenv() ;
extern char	*strbasename() ;
extern char	*putheap() ;
extern char	*malloc_str(), *malloc_sbuf() ;
extern char	*timestr_log() ;


/* external data */

extern struct global	g ;

extern struct pcsconf	p ;

extern struct userinfo	u ;


/* global data */


/* forward references */

static int	popt_store() ;



/* local static data */

char	*argopts[] = {
	"ROOT",
	"TMPDIR",
	"TERM",
	"LINES",
	"EDITOR",
	"PAGER",
	"MAILER",
	"verify",			/* backwards compatibility */
	NULL
} ;

#define	ARGOPT_ROOT	0
#define	ARGOPT_TMPDIR	1
#define	ARGOPT_TERM	2
#define	ARGOPT_LINES	3
#define	ARGOPT_EDITOR	4
#define	ARGOPT_PAGER	5
#define	ARGOPT_MAILER	6
#define	ARGOPT_VERIFY	7

/* program options */

char	*progopts[] = {
	"verify",
	"filecopy",
	"edit",
	NULL
} ;

#define	PROGOPT_VERIFY		0
#define	PROGOPT_FILECOPY	1
#define	PROGOPT_EDIT		2





int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	msgfile, *mfp = &msgfile ;
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	*fpa[3] ;

	struct	stat	sb ;

	struct tm	*timep ;

	struct group	*gp ;

	struct address	*as_to = NULL ;
	struct address	*as_from = NULL ;
	struct address	*as_sender = NULL ;
	struct address	*as_replyto = NULL ;
	struct address	*as_cc = NULL ;
	struct address	*as_bcc = NULL ;

	vecstr	rl ;

	offset_t		offset ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int		len, rs, status ;
	int		wstatus ;
	int		i, l ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_done = FALSE ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int		f_send ;
	int		pid_child ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	pcsbuf[PCSCONF_LEN + 1] ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*options = NULL ;
	char		buf[BUFSIZE + 1], *bp ;
	char		linebuf[LINELEN + 1] ;
	char		*namep, *cp, *cp2 ;
	char		*realname ;


	if (fstat(3,&sb) >= 0)
	    debugsetfd(3) ;

	g.progname = "PCSMAIL" ;
	if ((argc >= 1) && ((l = strlen(argv[0])) > 0)) {

	    while (argv[0][l - 1] == '/') l -= 1 ;

	    argv[0][l] = '\0' ;
	    g.progname = strbasename(argv[0]) ;

	    strcpy(buf,g.progname) ;

	    bp = buf ;
	    if (*bp == 'n') bp += 1 ;

	}

/* open the error output */

	g.efp = efp ;
	if (bopen(g.efp,BFILE_STDERR,"wca",0666) < 0) return BAD ;


/* some early initialization */

	f_version = FALSE ;


	if ((g.pcs = getenv("PCS")) == NULL)
	    g.pcs = PCS ;

	g.debuglevel = 0 ;
	g.termtype = getenv("TERM") ;

	g.prog_editor = NULL ;
	g.prog_sendmail = PROG_SENDMAIL ;

	if ((g.prog_pager = getenv("PAGER")) == NULL)
	    g.prog_pager = PROG_PAGER ;


	g.arg_subject = NULL ;
	g.arg_from = NULL ;
	g.arg_msgfname = NULL ;
	g.arg_attfname = NULL ;
	g.arg_attctype = NULL ;

	g.pid = getpid() ;

	g.f.verbose = FALSE ;
	g.f.exit = FALSE ;
	g.f.edit = FALSE ;
	g.f.verify = FALSE ;

#if	CF_NOSEND
	g.f.nosend = TRUE ;
#else
	g.f.nosend = FALSE ;
#endif

#ifdef	SYSV
	g.f.sysv_ct = TRUE ;
#else
	g.f.sysv_ct = FALSE ;
#endif

	g.f.sysv_rt = FALSE ;
	if (access("/usr/sbin",R_OK) >= 0) g.f.sysv_rt = TRUE ;


	if ((cp = getenv("SMAILOPTS")) != NULL)
	    popt_store(&options,cp) ;


/* process program arguments */

#if	CF_DEBUGS
	debugprintf("main: about to loop through arguments\n") ;
#endif

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((! f_done) && (argr > 0)) {

#if	CF_DEBUGS
	    debugprintf("main: top of loop, argp=%s\n",argv[i + 1]) ;
#endif

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

#if	CF_DEBUGS
	            debugprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                debugprintf("main: got an option key w/ a value\n") ;
#endif

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword match or should we assume only key letters ? */

#if	CF_DEBUGS
	            debugprintf("main: about to check for a key word match\n") ;
#endif

	            if ((kwi = matstr(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("main: got an option keyword, kwi=%d\n",
	                    kwi) ;
#endif

	                switch (kwi) {

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

/* number of terminal lines */
	                case ARGOPT_LINES:
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

/* pager program */
	                case ARGOPT_PAGER:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.prog_pager = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.prog_pager = argp ;

	                    }

	                    break ;

/* mailer program */
	                case ARGOPT_MAILER:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.prog_sendmail = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.prog_sendmail = argp ;

	                    }

	                    break ;

/* the old "verify" option */
	                case ARGOPT_VERIFY:
	                    if (popt_store(&options,"-verify") < 0)
	                        goto badargstore ;

	                    break ;

/* default action and user specified help */
	                default:
	                    bprintf(efp,
	                        "%s: unknown argument keyword \"%s\"\n",
	                        g.progname,aop) ;

	                    f_usage = TRUE ;
	                    f_done = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *aop) {

	                    case 'V':
	                        f_version = TRUE ;
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

/* daemon mailer program */
	                    case 'M':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.prog_sendmail = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.prog_sendmail = argp ;

	                        }

	                        break ;

	                    case 'v':
	                        g.f.verbose = TRUE ;
	                        break ;

/* invocation line subject */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.arg_subject = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.arg_subject = argp ;

	                        }

	                        break ;

/* invocation line FROM address */
	                    case 'f':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.arg_from = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.arg_from = argp ;

	                        }

	                        break ;

/* invocation line message file */
	                    case 'm':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.arg_msgfname = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.arg_msgfname = argp ;

	                        }

	                        break ;

/* invocation line attachment file */
	                    case 'a':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.arg_attctype = avp ;

	                        }

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.arg_attfname = argp ;

	                        break ;

/* store away any supplied program options */
	                    case 'o':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {

	                                if (popt_store(&options,avp) < 0)
	                                    goto badargstore ;

	                            }

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                if (popt_store(&options,argp) < 0)
	                                    goto badargstore ;

	                            }

	                        }

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

#if	CF_DEBUGS
	debugprintf("main: done looping through arguments\n") ;
#endif

	if (g.debuglevel > 0) {

	    bprintf(g.efp,"%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;

#if	CF_DEBUGS
	    debugprintf("main: debuglevel %d\n",g.debuglevel) ;
#endif
	}

/* check arguments */



	g.ofp = ofp ;
	if ((rs = bopen(g.ofp,BFILE_STDOUT,"wct",0666)) < 0) goto badoutopen ;

	g.ifp = ifp ;
	if ((rs = bopen(g.ifp,BFILE_STDIN,"r",0666)) < 0) goto badinopen ;



/* continue with less critical things */


	standard = 1 ;
	verbose = 0 ;

	f_name = TRUE ;
	f_fullname = FALSE ;
	g.f.interactive = FALSE ;
	if (isatty(0)) g.f.interactive = TRUE ;


/* determine mode of execution */

	if (strncmp(bp,"rpcsmail",6) == 0)
	    ex_mode = EM_REMOTE ;

	else if (strncmp(bp,"pcsinfo",7) == 0)
	    ex_mode = EM_PCSINFO ;

	else if (strcmp(bp,"info") == 0) {

	    if (buf[0] != 'n') g.progname = "PCSINFO" ;

	    else g.progname = "NPCSINFO" ;

	    argv[0] = g.progname ;
	    ex_mode = EM_PCSINFO ;

	} else if (strcmp(bp,"pc") == 0)
	    ex_mode = EM_PC ;

	else if (strcmp(bp,"pcsmail") == 0)
	    ex_mode = EM_PCSMAIL ;

	else {

	    if (buf[0] == 'n') g.progname = "NPCSMAIL" ;

	    else g.progname = "PCSMAIL" ;

	    ex_mode = EM_PCSMAIL ;
	    argv[0] = g.progname ;
	}

/* get user information */

	userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

/* get the time and the broken out time structure */

	time(&g.daytime) ;

	timep = localtime(&g.daytime) ;

/* program logging */

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: logid=%s\n",u.logid) ;
#endif

	sprintf(buf,"%s/%s",g.pcs,LOGFILE) ;

/* open the log file */

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: logfile_open logfile=\"%s\"\n",buf) ;
#endif

	rs = logfile_open(&g.lh,buf,0,0666,u.logid) ;

#if	CF_DEBUG
	if (g.debuglevel > 0)
	    debugprintf("main: logfile_open return (rs %d)\n",rs) ;
#endif

/* make a log entry */

	buf[0] = '\0' ;
	if ((u.name != NULL) && (u.name[0] != '\0'))
	    sprintf(buf,"(%s)",u.name) ;

	else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	    sprintf(buf,"(%s)",u.gecosname) ;

	else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	    sprintf(buf,"(%s)",u.fullname) ;

	else if (u.mailname != NULL)
	    sprintf(buf,"(%s)",u.mailname) ;

	logfile_printf(&g.lh,"%s %-14s %s/%s\n",
	    timestr_log(g.daytime,timebuf),
	    g.progname,
	    VERSION,(u.f.sysv_ct ? "SYSV" : "BSD")) ;

	logfile_printf(&g.lh,"os=%s %s!%s %s\n",
	    (u.f.sysv_rt ? "SYSV" : "BSD"),u.nodename,u.username,buf) ;


/* create an ERROR log file */

	sprintf(buf,"%s/%s",g.pcs,ERRFILE) ;

	logfile_open(&g.eh,buf,0,0666,u.logid) ;


/* write the user file */

	sprintf(buf,"%s/%s",g.pcs,USERFILE) ;

	if (bopen_wall(tfp,buf) >= 0) {

	    bprintf(tfp,"%s!%s\n",u.nodename,u.username) ;

	    bclose(tfp) ;

	} /* end if */

/* make a "N"ew user file entry if we are running a "N"ew program version */

#ifdef	NUSERFILE
	sprintf(buf,"%s/%s",g.pcs,NUSERFILE) ;

	if (bopen_wall(tfp,buf) >= 0) {

	    bprintf(tfp,"%s!%s\n",u.nodename,u.username) ;

	    bclose(tfp) ;

	} /* end if */
#endif /* NUSERFILE */



	if (f_version) goto version ;

	if (g.f.exit) goto goodret ;



/* OK, we get the PCS site-wide configuration stuff */

	if (pcs_conf(g.pcs,NULL,&p,NULL,pcsbuf,PCSCONF_LEN) < 0)
	    logfile_printf(&g.lh, "could not get explicit PCS configuration\n") ;

	if (p.mailhost == NULL)
	    p.mailhost = MAILHOST ;

	if (p.uucpnode == NULL)
	    p.uucpnode = "" ;

	if (p.mailnode == NULL) {

	    if ((cp = strchr(p.mailhost,'.')) != NULL)
	        p.mailnode = cp + 1 ;

	    else
	        p.mailnode = "mailhost" ;

	}

	if (p.fromnode == NULL)
	    p.fromnode = p.mailhost ;

	if (p.maildomain == NULL)
	    p.maildomain = u.domainname ;

	if (p.maildomain == NULL)
	    p.maildomain = MAILDOMAIN ;

	if (p.orgdomain == NULL) {

	    if ((cp = strchr(u.domainname,'.')) != NULL)
	        p.orgdomain = cp ;

	    else
	        p.orgdomain = "lucent.com" ;

	}

	if (p.organization == NULL)
	    p.organization = getenv("ORGANIZATION") ;

	else
	    p.organization = ORGANIZATION ;



#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    debugprintf("main: ld=%s cd=%s md=%s\n",
	        u.domainname,p.orgdomain,p.maildomain) ;

	    debugprintf("main: mh=%s mn=%s\n",
	        p.mailhost,p.mailnode) ;

	}
#endif


	logfile_printf(&g.lh,"ld=%s cd=%s md=%s\n",
	    u.domainname,p.orgdomain,p.maildomain) ;

	logfile_printf(&g.lh,"mh=%s mn=%s\n",
	    p.mailhost,p.mailnode) ;



/* initialize some other common stuff only needed for mail operations */


/* get the 'mail' group ID */

	g.gid_mail = MAILGROUP ;
	if ((gp = getgrnam("mail")) != NULL)
	    g.gid_mail = gp->gr_gid ;


/* get some programs */


/* try to determine the user's favorite editor program */

	if (g.prog_editor == NULL) {

	    if (((cp = getenv("ED")) != NULL) && (*cp != '\0'))
	        g.prog_editor = cp ;

	    else if (((cp = getenv("EDITOR")) != NULL) && (*cp != '\0'))
	        g.prog_editor = cp ;

	    else {

	        if (g.f.sysv_rt)
	            g.prog_editor = "/usr/bin/ed" ;

	        else
	            g.prog_editor = "ed" ;

	    }

	} /* end if (prog_editor) */

	logfile_printf(&g.lh,"editor=\"%s\"\n",g.prog_editor) ;


/* the daemon delivery program */

	g.prog_sendmail = PROG_SENDMAIL ;


/* PCS "content-length" adder */

	g.prog_pcscl = PROG_PCSCL ;
	cp = PROG_PCSCL ;
	if (cp[0] != '/') {

	    if (getfiledirs(NULL,PROG_PCSCL,"x",NULL) <= 0) {

	        sprintf(buf,"%s/%s",g.pcs,PROG_PCSCL) ;

	        g.prog_pcscl = malloc_str(buf) ;

	    }

	}


/* PCS "cleanup" program */

	g.prog_pcscleanup = PROG_PCSCLEANUP ;
	cp = PROG_PCSCLEANUP ;
	if (cp[0] != '/') {

	    if (getfiledirs(NULL,PROG_PCSCLEANUP,"x",NULL) <= 0) {

	        sprintf(buf,"%s/%s",g.pcs,PROG_PCSCLEANUP) ;

	        g.prog_pcscleanup = malloc_str(buf) ;

	    }

	}


/* cleanup work (failing parent and successful child execute inside) */

	if ((pid_child = uc_fork()) <= 0) {

	    if (pid_child == 0) {
	        for (i = 0 ; i < 4 ; i += 1) close(i) ;
	        setsid() ;
	    }

	    fpa[0] = NULL ;
	    fpa[1] = NULL ;
	    fpa[2] = NULL ;
	    if ((rs = bopencmd(fpa,g.prog_pcscleanup)) < 0) {

	        logfile_printf(&g.lh,
	            "could not run the cleanup program (rs %d)\n",rs) ;

	    }

/* the successful child (if any) exits here */

	    if (pid_child == 0) exit(0) ;

	} /* end if (cleanup code) */



/* get program options */

	getvars() ;



/* we continue with other initialization that is needed for mail operations */

/* get the standard UNIX envelope date string */

	date_envelope(g.daytime,buf) ;

	g.date_envelope = putheap(buf) ;

/* get the standard RFC 822 "DATE" header date string */

	date_header(g.daytime,buf) ;

	g.date_header = putheap(buf) ;

#if	CF_DEBUG
	if (g.f.debug)
	    debugprintf("main: env=\"%s\" header=\"%s\"\n",
	        g.date_envelope,
	        g.date_header) ;
#endif


/* create a message ID */

	if (u.domainname != NULL) {

	    sprintf(buf, "<%d.%02d%03d%02d%02d%02d@%s.%s>",
	        g.pid,
	        timep->tm_year,timep->tm_yday,
	        timep->tm_hour,timep->tm_min,timep->tm_sec,
	        u.nodename,u.domainname) ;

	} else {

	    sprintf(buf, "<%d.%02d%03d%02d%02d%02d@%s>",
	        g.pid,
	        timep->tm_year,timep->tm_yday,
	        timep->tm_hour,timep->tm_min,timep->tm_sec,
	        u.nodename) ;

	}

	g.messageid = putheap(buf) ;


/* OK, do it */

	if (g.debuglevel > 0)
	    g.f.nosend = TRUE ;

	if (g.f.debug || g.f.nosend) {

	    logfile_printf(&g.lh,"running in NOSEND mode\n") ;

	    bprintf(g.efp,
	        "%s: special note -- running in NOSEND mode\n",
	        g.progname) ;

	}


	vecstrinit(&rl,10,0) ;

	if (npa > 0) {

#if	CF_DEBUGS
	    debugprintf("main: got user specified email addresses\n") ;
#endif

	    if (g.f.debug) bprintf(g.efp,
	        "%s: processing user specified newsgroups\n",
	        g.progname) ;

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (! BATST(argpresent,i)) continue ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf(
	                "main: processing argument \"%s\"\n",
	                argv[i]) ;
#endif

/* handle the funny (backward compatibility) pseudo recipients first */

	        if ((strncmp(argv[i],"file=",5) == 0) ||
	            (strncmp(argv[i],"eappend=",8) == 0)) {

	            if (g.arg_msgfname == NULL)
	                g.arg_msgfname = argv[i] ;

	        } else if ((strncmp(argv[i],"dappend=",8) == 0) ||
	            (strncmp(argv[i],"append=",7) == 0)) {

	            if (g.arg_attfname == NULL)
	                g.arg_attfname = argv[i] ;

	        } else if (strncmp(argv[i],"re=",3) == 0) {

	            vecstradd(&rl,argv[i] + 3,-1) ;

	        } else if (strncmp(argv[i],"or=",3) != 0) {

/* handle as a regular recipient */

	            vecstradd(&rl,argv[i],-1) ;

	        } /* end if (special argument) */

	    } /* end for (outer) */

	} /* end if (processing recipients) */

#if	CF_DEBUG
	if (g.debuglevel > 0) {

	    for (i = 0 ; vecstrget(&rl,i,&cp) >= 0 ; i += 1) {

	        if (cp == NULL) continue ;

	        debugprintf("main: re> %s\n",cp) ;

	    }

	}

#endif /* CF_DEBUG */


/* process program invocation options */

#if	CF_DEBUG

	if ((g.debuglevel > 0) && (options != NULL)) {

	    while ((cp = strpbrk(options,",:")) != NULL) {

	        *cp++ = '\0' ;
	        debugprintf("main: opt> %s\n",options) ;

	        options = cp ;

	    }

	    if (options != NULL)
	        debugprintf("main: opt> %s\n",options) ;

	}

#endif /* CF_DEBUG */


#if	CF_DEBUG
	if (g.debuglevel > 0) {

		if (g.arg_msgfname != NULL)
			debugprintf("main: arg_msgfname=%s\n",g.arg_msgfname) ;

		if (g.arg_attfname != NULL)
			debugprintf("main: arg_attfname=%s\n",g.arg_attfname) ;

		if (g.arg_attctype != NULL)
			debugprintf("main: arg_attctype=%s\n",g.arg_attctype) ;

	}
#endif /* CF_DEBUG */


#ifdef	COMMENT
	rs = process(&rl) ;
#endif


	vecstrfree(&rl) ;


/* no one to send to Sighhh...! */

#ifdef	COMMENT

	if (tonames == 0) {

	    if (access(tempfile,R_OK) < 0) {

	        debugprintf("main: no tempfile here\n") ;

	        sprintf(syscom,"cp /dev/null %s",tempfile) ;

#if	CF_DEBUG
	        if (g.f.debug)
	            debugprintf("main: SYSTEM> %s\n",syscom) ;
#endif

	        system(syscom) ;

	    }

	    sprintf(syscom, "cat %s >> %s/%s\n", tempfile,
	        u.homedname, DEADFILE) ;

	    system(syscom) ;

	    unlink(tempfile) ;

	    if (isforward > 0) unlink(forwfile) ;

	    if (g.f.interactive) {

	        bprintf(g.ofp,"\nno recipients were specified\n") ;

	        bprintf(g.ofp,"mail saved in file '%s'\n",DEADFILE) ;

	    }

	    goto badret ;

	} /* end if (no recipients were specified) */

#endif /* COMMENT */


done:
goodret:
	bclose(g.ofp) ;

	bclose(g.efp) ;

	return OK ;

/* the bad stuff */
badret:
	bclose(g.ofp) ;

	bclose(g.efp) ;

	return BAD ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargstore:
	bprintf(efp,"%s: could not alloc memory\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badinopen:
	bprintf(g.efp,"%s: could not open the input (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badoutopen:
	bprintf(g.efp,"%s: could not open the output (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

version:
	bprintf(g.efp,"%s: version %s/%s\n",
	    g.progname,
	    VERSION,(g.f.sysv_ct ? "SYSV" : "BSD")) ;

	goto goodret ;
}
/* end subroutine (main) */


/* local subroutine to store program invocation options */
static int popt_store(opp,s)
char	**opp ;
char	s[] ;
{
	int	len, l ;
	int	rs ;

	char	*np ;


	len = 0 ;
	if (*opp != NULL)
	    len = strlen(*opp) ;

#if	CF_DEBUGS
	debugprintf("popt_store: options=%s\n",*opp) ;
#endif

	if ((s != NULL) && ((l = strlen(s)) > 0)) {

#if	CF_DEBUGS
	    debugprintf("popt_store: s=%s\n",s) ;
#endif

	    if (*opp != NULL)
	        np = (char *) realloc(*opp,len + l + 2) ;

	    else
	        np = (char *) malloc(l + 3) ;

#if	CF_DEBUGS
	    debugprintf("popt_store: realloced\n") ;
#endif

	    if (np != NULL) {

	        *opp = np ;
	        strcpy(np + len,", ") ;

	        strcpy(np + len + 2,s) ;

	        len += l + 2 ;

	    } else
	        len = -1 ;

	}

#if	CF_DEBUGS
	debugprintf("popt_store: exiting\n") ;
#endif

	return len ;
}
/* end subroutine (popt_store) */


/* process a single program option */
static void popt_process(o)
char	o[] ;
{
	char	*cp ;

	int	i, f_state = TRUE ;


	while (CHAR_ISWHITE(*o)) o += 1 ;

	if (o[0] == '-') {

	    f_state = FALSE ;
	    o += 1 ;

	} else if (o[0] == '+')
	    o += 1 ;

	if ((i = matstr(progopts,o)) >= 0) {

	    switch (i) {

	    case PROGOPT_VERIFY:
	        g.f.verify = f_state ;
	        break ;

	    case PROGOPT_EDIT:
	        g.f.edit = f_state ;
	        break ;

	    case PROGOPT_FILECOPY:
	        g.f.filecopy = f_state ;
	        break ;

	    } /* end switch */

	} /* end if */

}
/* end subroutine (popt_process) */



