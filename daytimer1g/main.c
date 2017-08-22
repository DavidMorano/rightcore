/* main */

/* main subroutine for the day timer program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_EXPERIMENT	1		/* ? */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


	= 1990-02-10, David A­D­ Morano

	This subroutine was modified to not write out anything
	to standard output if the access time of the associated
	terminal has not been changed in 10 minutes.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/************************************************************************

	Synopsis:

	$ daytimer [[mailfile] offint] [-o options] [-sV] [-t timeout]


*************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mkdev.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<char.h>
#include	<ascii.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<logfile.h>
#include	<lfm.h>
#include	<mallocstuff.h>
#include	<userinfo.h>
#include	<localmisc.h>

#include	"mailfiles.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	LINENAMELEN
#define	LINENAMELEN	MAXPATHLEN
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	termdevice(char *,int,int) ;
extern int	bufprintf(char *,int,cchar *,...) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	process(struct proginfo *,struct userinfo *,int,
			time_t,time_t,int,MAILFILES *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern const char	daytimer_makedate[] ;


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;
static int	mklinename(char *,const char *) ;
static int	procopts(struct proginfo *,KEYOPT *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"LOG",
	"MAKEDATE",
	"sn",
	"af",
	"ef",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_log,
	argopt_makedate,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;

static const char *progopts[] = {
	"offint",
	"logextra",
	NULL
} ;

enum progopts {
	progopt_offint,
	progopt_logextra,
	progopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	USERINFO	u ;
	MAILFILES	mfs ;
	KEYOPT		akopts ;
	bfile		errfile ;
	time_t		daytime = time(NULL) ;

	int	argr, argl, aol, avl, akl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	len, cl ;
	int	v ;
	int	refresh = DEF_REFRESH ;
	int	fd_termdev ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_statdisplay ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[NARGPRESENT] ;
	char	buf[BUFLEN + 1] ;
	char	userinfobuf[USERINFO_LEN + 1] ;
	char	linename[MAXNAMELEN + 1] ;
	char	termdevfname[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	lockfname[MAXPATHLEN + 1] ;
	char	pidfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*piddname = PIDDNAME ;
	const char	*lockdname = LOCKDNAME ;
	const char	*afname = NULL;
	const char	*efname = NULL;
	const char	*hfname = HELPFNAME ;
	const char	*lfname = NULL ;
	const char	*termarg = NULL ;
	const char	*maildname = NULL ;
	const char	*mailfname = NULL ;
	const char	*mailpath = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	pip->pidfname = pidfname ;

	pip->verboselevel = 1 ;

	pip->to_blank = -1 ;
	pip->offint = -1 ;

	pip->f.quiet = FALSE ;
	pip->f.lockfile = TRUE ;
	pip->f.pidfile = TRUE ;
	pip->f.lockopen = FALSE ;
	pip->f.pidopen = FALSE ;

	f_statdisplay = FALSE ;
	f_help = FALSE ;

/* process program arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	if (rs >= 0) {
	    rs = keyopt_loads(&akopts,cp,-1) ;
	}

	for (ai = 0 ; ai < NARGPRESENT ; ai += 1)  {
		argpresent[ai] = 0 ;
	}

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

		    rs = cfdecti((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

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

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* program root */
	                    case argopt_root:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) 
					pr = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					pr = argp ;
	                        }
	                        break ;

/* debug level */
	                    case argopt_debug:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
					rs = optvalue(avp,avl) ;
	                                pip->debuglevel = rs ;
				    }
	                        }
	                        break ;

	                    case argopt_version:
				    f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
			 	    }
	                        }
	                        break ;

/* help file */
	                    case argopt_help:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) 
					hfname = avp ;
	                        }
	                        f_help  = TRUE ;
	                        break ;

/* log file */
	                    case argopt_log:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) 
					lfname = avp ;
	                        }
	                        break ;

/* display the time this program was last "made" */
	                    case argopt_makedate:
	                        f_makedate = TRUE ;
	                        break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* argument-list file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            afname = argp ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

			    default:
				rs = SR_INVALID ;
				break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {
			        const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
					if (avl) {
	                                    rs = optvalue(avp,avl) ;
					    pip->debuglevel = rs ;
				        }
	                            }
	                            break ;

	                        case 'V':
				    f_makedate = f_version ;
	                            f_version = TRUE ;
	                            break ;

/* terminal device */
	                        case 'd':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                termarg = argp ;
	                            break ;

/* set the lock file directory */
	                        case 'l':
	                            pip->f.lockfile = TRUE ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
	                                if (avl)
	                                    lockdname = avp ;
	                            }
	                            break ;

/* the mail path */
	                        case 'm':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                mailpath = argp ;
	                            break ;

/* time offset */
	                        case 'o':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					rs = keyopt_loads(&akopts,argp,argl) ;
				    }
	                            break ;

/* observe PID file */
	                        case 'p':
	                            pip->f.pidfile = TRUE ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
	                                if (avl)
	                                    piddname = avp ;
	                            }
	                            break ;

	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

	                        case 's':
	                            f_statdisplay = TRUE ;
	                            break ;

/* set the display refresh interval */
	                        case 'r':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = cfdecti(argp,argl,&refresh) ;
	                            break ;

/* time out for screen blanking */
	                        case 't':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					rs = cfdecti(argp,argl,&v) ;
				        pip->to_blank = v ;
				    }
	                            break ;

	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
	                                if (avl) {
	                                    rs = optvalue(avp,avl) ;
	                                    pip->verboselevel = rs ;
				        }
	                            }
	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        default:
	                            rs = SR_INVALID ;
				    break ;

	                        } /* end switch */
	                        akp += 1 ;

				if (rs < 0) break ;
	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX) break ;
	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,hfname) ;

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* processing the invocation arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	            switch (pan) {

	            case 0:
	                mailfname = argv[ai] ;
	                break ;

	            case 1:
	                if (argv[ai][0] != '\0') {

				pip->final.offint = TRUE ;
	                    rs = cfdeci(argv[ai],-1,&pip->offint) ;

	                }

	                break ;

	            } /* end switch */

	            pan += 1 ;
		    if (rs < 0)
			break ;

	    } /* end for (loading positional arguments) */

	if (rs < 0)
		goto badarg ;

/* process the program options */

	rs = procopts(pip,&akopts) ;
	if (rs < 0)
		goto badarg ;

/* get a terminal device name */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: termarg=%s\n",termarg) ;
#endif /* CF_DEBUG */

	fd_termdev = FD_STDOUT ;
	if (termarg != NULL) {

		if (strncmp(termarg,"/dev",4) == 0)
			mkpath1(termdevfname,termarg) ;

		else
			mkpath2(termdevfname,"/dev",termarg) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: termdev=%s\n",termdevfname) ;
#endif /* CF_DEBUG */

		mklinename(linename,termdevfname) ;

	    rs = u_open(termdevfname,(O_RDWR | O_NOCTTY),0666) ;
	    fd_termdev = rs ;

	} else {

	        rs = termdevice(termdevfname,MAXPATHLEN, FD_STDOUT) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: termdevice() rs=%d\n",rs) ;
	        debugprintf("main: termdev=%s\n",termdevfname) ;
	}
#endif /* CF_DEBUG */

		mklinename(linename,termdevfname) ;

	} /* end if */

	if (rs < 0)
		goto badnoterm ;

/* is it a termainl? */

	if (! isatty(fd_termdev))
	    goto badnotterm ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: linename=%s\n", linename) ;
#endif /* CF_DEBUG */

/* check some optional parameters */

	if ((pip->offint <= 0) && (argvalue > 0))
		pip->offint = argvalue ;

	if (pip->offint < 0)
		pip->offint = 0 ;

/* who are we? */

	rs = userinfo(&u,userinfobuf,USERINFO_LEN,NULL) ;

	if (rs < 0)
	    goto baduser ;

	pip->pid = u.pid ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: username=%s linename=%s\n",
	        pip->progname,u.username,linename) ;

	pip->sid = u_getsid(0) ;

/* other initialization */

	pip->loglen = LOGSIZE ;
	pip->pollint = POLLINT ;
	pip->mailint = MAILINT ;
	pip->to_lock = TO_LOCK ;
	if (pip->to_blank <= 0)
		pip->to_blank = TO_BLANK ;

/* do we have a log file? */

	if (lfname == NULL) lfname = getenv(VARLOGFILE) ;
	if (lfname == NULL) lfname = LOGFNAME ;

	if (lfname[0] != '/') {
	    len = mkpath2(buf, pip->pr,lfname) ;
	    lfname = mallocstrw(buf,len) ;
	}

/* make a log entry */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: logfile=%s\n",
	        lfname) ;
#endif

	rs = logfile_open(&pip->lh,lfname,0,0666,u.logid) ;

	if (rs >= 0) {

		pip->f.log = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main: about to do 'logfile_printf'\n") ;
#endif

	    logfile_printf(&pip->lh,"%s %-14s %s/%s\n",
	        timestr_log(daytime,timebuf),
	        pip->progname,
	        VERSION,(u.f.sysv_ct ? "SYSV" : "BSD")) ;

	    logfile_printf(&pip->lh,"ostype=%s os=%s(%s)\n",
	        (u.f.sysv_rt ? "SYSV" : "BSD"),
		u.sysname,u.release) ;

	    buf[0] = '\0' ;
	    if ((u.name != NULL) && (u.name[0] != '\0'))
	        sprintf(buf,"(%s)",u.name) ;

	    else if ((u.gecosname != NULL) && (u.gecosname[0] != '\0'))
	        sprintf(buf,"(%s)",u.gecosname) ;

	    else if ((u.fullname != NULL) && (u.fullname[0] != '\0'))
	        sprintf(buf,"(%s)",u.fullname) ;

	    else if (u.mailname != NULL)
	        sprintf(buf,"(%s)",u.mailname) ;

	    logfile_printf(&pip->lh,"%s!%s %s\n",
		u.nodename,u.username,buf) ;

	} else {

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp, "%s: logfile=%s\n",pip->progname,lfname) ;
	        bprintf(pip->efp,
	            "%s: could not open the log file (%d)\n",
	            pip->progname,rs) ;
	    }

	} /* end if (opened a log) */

/* before we go too far, are we supposed to observe a lock file? */

	lockfname[0] = '\0' ;
	if (pip->f.lockfile) {
	    mode_t	umask_old ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: trying to create a lock file\n") ;
#endif

/* create the LOCK directory if necessary */

	    if (lockdname[0] == '/') {
	        cl = mkpath1(tmpfname, lockdname) ;
	    } else
	        cl = mkpath2(tmpfname, u.homedname, lockdname) ;

	    while ((cl > 0) && (tmpfname[cl - 1] == '/')) 
		cl -= 1 ;

	    tmpfname[cl] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: lockdname=%s\n", tmpfname) ;
#endif

	    umask_old = umask(0000) ;

	    rs = mkdirs(tmpfname,0777) ;

	    (void) umask(umask_old) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: mkdirs() rs=%d\n",rs) ;
#endif

	    if (rs < 0) {

	        logfile_printf(&pip->lh,
			"could not create lock file directory (%d)\n",rs) ;

	        goto badlockdir ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: lockfile trying file\n") ;
#endif

	    mkpath2(lockfname,tmpfname,linename) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: lockfname=%s\n",lockfname) ;
#endif

	    rs = lfm_start(&pip->lk,
			lockfname,LFM_TRECORD,pip->to_lock,NULL,
			u.nodename,u.username,pip->banner) ;

	    if (rs < 0) {

	        logfile_printf(&pip->lh,
			"could not capture the lockfile (%d)\n",
	            rs) ;

	        goto badlock1 ;
	    }

	    pip->f.lockopen = TRUE ;
	    lfm_printf(&pip->lk,"%-14s %s\n",
	        pip->progname,
	        VERSION) ;

	    lfm_printf(&pip->lk,"termdev=%s\n",termdevfname) ;

	    lfm_printf(&pip->lk,"linename=%s\n",linename) ;

	    lfm_flush(&pip->lk) ;

	} /* end if (capturing lock file) */

/* before we go too far, are we the only one on this PID mutex? */

	pidfname[0] = '\0' ;
	if (pip->f.pidfile) {

	    mode_t	umask_old ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: trying to create a PID file\n") ;
#endif

/* create the PID directory if necessary */

	    if (piddname[0] == '/')
	        cl = mkpath1(pidfname, piddname) ;

	    else
	        cl = mkpath2(pidfname, u.homedname, piddname) ;

	    while ((cl > 0) && (pidfname[cl - 1] == '/')) 
		cl -= 1 ;

	    pidfname[cl] = '\0' ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: piddname=%s\n", pidfname) ;
#endif

	    umask_old = umask(0000) ;

	    rs = mkdirs(pidfname,0777) ;

	    (void) umask(umask_old) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: mkdirs() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto badpiddir ;

	    bufprintf((pidfname + cl),(MAXPATHLEN - cl),"/%s",
	        linename) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: pidfname=%s\n",pidfname) ;
#endif

	    if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: pidfname=%s\n",
			pip->progname,pidfname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: ldm_init()\n") ;
#endif

	    rs = lfm_start(&pip->pidlock,pidfname,LFM_TRECORD,pip->to_lock,
			NULL,u.nodename,u.username,BANNER) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: lfm_start() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto badpidopen ;

	    pip->f.pidopen = TRUE ;
	    lfm_printf(&pip->pidlock,"%-14s %s\n",
	        pip->progname,
	        VERSION) ;

	    lfm_printf(&pip->pidlock,"termdev=%s\n",termdevfname) ;

	    lfm_printf(&pip->pidlock,"linename=%s\n",linename) ;

	    lfm_flush(&pip->pidlock) ;

	} /* end if (we have a mutex PID file) */

/* do the main thing */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {

	    debugprintf("main: calling 'daytimer' f_lock=%d f_pid=%d\n",
	        pip->f.lockfile,pip->f.pidfile) ;

	    debugprintf("main: refresh=%d to_blank=%d\n",
	        refresh,pip->to_blank) ;

	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {

		bprintf(pip->efp,"%s: to_blank=%u\n",
			pip->progname,pip->to_blank) ;

	    bprintf(pip->efp,"%s: PID=%u SID=%u\n",
		pip->progname,(uint) pip->pid,(uint) pip->sid) ;

	}

	if (pip->f.log) {

		logfile_printf(&pip->lh,"PID=%u SID=%u",
			(uint) pip->pid,(uint) pip->sid) ;

		logfile_printf(&pip->lh,"linename=%s to=%u",
			linename,pip->to_blank) ;

	    logfile_printf(&pip->lh,"lockfile=%s\n",
		lockfname) ;

	}

	mailfiles_init(&mfs) ;

#if	CF_EXPERIMENT

	if (mailfname != NULL) {

	    if (mailfname[0] != '-')
		mailfiles_add(&mfs,mailfname,-1) ;

	} else if ((cp = getenv("MAILPATH")) != NULL) {

		mailfiles_addpath(&mfs,cp,-1) ;

	} else if ((cp = getenv("MAIL")) != NULL)
		mailfiles_add(&mfs,cp,-1) ;

#endif /* CF_EXPERIMENT */

	rs = mailfiles_count(&mfs) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	    debugprintf("main: 1 mailfiles count=%d\n",rs) ;
#endif

	if (rs == 0) {

	    cl = -1 ;
	    if ((mailfname = getenv("MAIL")) == NULL) {

	        if ((maildname = getenv("MAILDIR")) == NULL)
	            maildname = MAILDNAME ;

	        mailfname = tmpfname ;
		cl = mkpath2(tmpfname,maildname,u.username) ;

	    } /* end if */

		mailfiles_add(&mfs,mailfname,cl) ;

	} /* end if (trying to get a MAIL file to monitor) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {

		MAILFILES_ENT	*ep ;


		rs = mailfiles_count(&mfs) ;

		debugprintf("main: 2 mailfiles count=%d\n",rs) ;

		for (i = 0 ; mailfiles_get(&mfs,i,&ep) >= 0 ; i += 1) {

			if (ep == NULL) continue ;

			debugprintf("main: mf=%s\n",ep->mailfname) ;

		}

	}
#endif /* CF_DEBUG */

/* do it */

	rs = process(pip,&u,fd_termdev,pip->offint,
		refresh,f_statdisplay,&mfs) ;

	mailfiles_free(&mfs) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


#ifdef	COMMENT
	logfile_printf(&pip->lh,"exiting (%d) ex=%u\n",rs,ex) ;
#endif

/* close off and get out ! */
done:
ret5:
	if (pip->f.pidopen)
	    lfm_finish(&pip->pidlock) ;

	if (pip->f.lockopen)
	    lfm_finish(&pip->lk) ;

	if (lockfname[0] != '\0')
		u_unlink(lockfname) ;

	if (pip->f.log)
	    logfile_close(&pip->lh) ;

ret4:
	if (fd_termdev >= 0)
		u_close(fd_termdev) ;

retearly:
ret3:

ret2:
ret1:
	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

ret0:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
		pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

badnoterm:
	ex = EX_CANTCREAT ;
	    bprintf(pip->efp,
	        "%s: no or invalid terminal device (%d)\n",
	        pip->progname,rs) ;

	goto retearly ;

badnotterm:
	ex = EX_DATAERR ;
	    bprintf(pip->efp,
	        "%s: target is not a terminal\n",
	        pip->progname) ;

	goto retearly ;

baduser:
	ex = EX_NOUSER ;
	if (! pip->f.quiet)
	    bprintf(pip->efp,
	        "%s: could not get user information (%d)\n",
	        pip->progname,rs) ;

	goto ret5 ;

badlockdir:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,
	        "%s: could not create the necessary lock directory (%d)\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badlock1:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,
	        "%s: could not create or capture the lock file (%d)\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badpiddir:
	if (! pip->f.quiet)
	    bprintf(pip->efp,
	        "%s: could not create the PID directory (%d)\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badpidopen:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,
	        "%s: could not create the PID file (%d)\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badpidlock:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,
	        "%s: could not lock the PID file (%d)\n",
	        pip->progname,rs) ;

	ex = EX_DATAERR ;
	goto badret ;

badret:
	goto done ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [mailfile [<toff>]] [-sV] [-t <to>] [-<toff>]\n",
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;
	    
	rs = bprintf(pip->efp, "  [-d terminal_device]\n",pn) ;
	wlen += rs ;

	return rs ;
}
/* end subroutines (usage) */


static int mklinename(linename,termdevfname)
char		linename[] ;
const char	termdevfname[] ;
{
	int		i ;
	const char	*sp, *tp ;

	sp = termdevfname ;
	while (*sp && (*sp == '/'))
		sp += 1 ;

/* remove the device directory prefix part */

	if ((tp = strchr(sp,'/')) != NULL)
		sp = (tp + 1) ;

	for (i = 0 ; *sp && (i < MAXNAMELEN) ; i += 1) {
		linename[i] = (*sp != '/') ? *sp : '_' ;
		sp += 1 ;
	}

	linename[i] = '\0' ;
	return i ;
}
/* end subroutine (mklinename) */


/* process program options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	int		rs = SR_OK ;
	const char	*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
		int		val ;
	        int		oi ;
	        int		kl, vl ;
	        const char	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(progopts,3,kp,kl)) >= 0) {

	            switch (oi) {

	            case progopt_offint:
	                if ((vl > 0) && (cfdeci(vp,vl,&val) >= 0))
	                    pip->offint = val ;
	                break ;

	            case progopt_logextra:
			pip->f.logextra = TRUE ;
	                if ((vl > 0) && (cfdeci(vp,vl,&val) >= 0))
	                    pip->f.logextra = (val > 0) ;
	                break ;

	            } /* end switch */

		    if (rs < 0) break ;
		} /* end if (matostr) */

	    } /* end while (keys) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (procopts) */


