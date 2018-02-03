/* main */

/* generic (pretty much) daemon front end subroutine */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine forms the front-end part of a generic PCS daemon type of
        program. This front-end is used in a variety of PCS daemons and other
        programs.

        This subroutine was originally part of the Personal Communications
        Services (PCS) package but can also be used independently from it.
        Historically, this was developed as part of an effort to maintain high
        function (and reliable) email communications in the face of increasingly
        draconian security restrictions imposed on the computers in the DEFINITY
        development organization.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

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
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bfile.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<varsub.h>
#include	<pcsconf.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>
#include	<getax.h>
#include	<userinfo.h>
#include	<localmisc.h>

#include	"configfile.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"dev/null"
#endif


/* external subroutines */

extern int	strkeycmp(const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	getfname(const char *,const char *,int,char *) ;
extern int	getportnum(cchar *,cchar *) ;
extern int	logfile_userinfo(LOGFILE *,USERINFO *,time_t,cchar *,cchar *) ;
extern int	bopenroot(bfile *,cchar *,cchar *,cchar *,char *,mode_t) ;
extern int	isalphalatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;

extern int	procfileenv(cchar *,cchar *,VECSTR *) ;
extern int	varsub_subbuf(), varsub_merge() ;
extern int	expander() ;
extern int	watch(PROGINFO *,int,VECSTR *) ;
extern int	handle(PROGINFO *,int,VECSTR *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* forward references */

static int	usage(PROGINFO *) ;
static int	procopts(PROGINFO *,char *,int) ;


/* local structures */


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"CONFIG",
	"TMPDIR",
	"LOGFILE",
	"sn",
	"af",
	"ef",
	"if",
	"oi",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_config,
	argopt_tmpdir,
	argopt_logfile,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_if,
	argopt_oi,
	argopt_overlast
} ;

static const PIVARS	initvars = {
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

/* 'conf' for most regular programs */
static const char	*sched1[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%n.%f",
	NULL
} ;

/* 'conf' and non-'conf' ETC stuff for local searching */
static const char	*sched3[] = {
	"%e/%n/%n.%f",
	"%e/%n/%f",
	"%e/%n.%f",
	"%e/%f",
	"%n.%f",
	"%f",
	NULL
} ;

static const char	*progopts[] = {
	"sender",
	"dmail",
	NULL
} ;

enum progopts {
	progopt_sender,
	progopt_dmail,
	progopt_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO		pi, *pip = &pi ;
	struct sockaddr_in	sa_server ;
	struct ustat		sb ;

	struct group	ge ;
	USERINFO	u ;
	KEYOPT		akopts ;
	VECSTR		schedvars ;
	CONFIGFILE	cf ;
	bfile		errfile ;
	bfile		logfile ;
	bfile		pidfile ;
	vecstr		defines, exports ;
	varsub		vsh_e, vsh_d ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		len, i, l ;
	int		s, proto ;
	int		port = -1 ;
	int		loglen = -1 ;
	int		l2, sl ;
	int		logfile_type = -1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_programroot = FALSE ;
	int		f_freeconfigfname = FALSE ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char		argpresent[MAXARGGROUPS] ;
	char		buf[BUFLEN + 1] ;
	char		buf2[BUFLEN + 1] ;
	char		userbuf[USERINFO_LEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		pidfname[MAXPATHLEN + 1] ;
	char		lockfname[MAXPATHLEN + 1] ;
	char		logfname[MAXPATHLEN + 1] ;
	char		xfnamebuf[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ifname = NULL ;
	const char	*configfname = NULL ;
	const char	*portspec = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialization */

	pip->lfp = &logfile ;

	pip->verboselevel = 1 ;
	pip->nrecips = NSEND ;

	pip->daytime = time(NULL) ;

	pip->pid = getpid() ;

	pip->ppid = pip->pid ;

	port = -1 ;

/* we want to open up some files so that the first few FD slots are FULL!! */

	{
	    const char	*nullfname = NULLFNAME ;
	    for (i = 0 ; i < 3 ; i += 1) {
	        if (u_fstat(i,&sb) < 0)
	            u_open(nullfname,O_RDWR,0666) ;
	    } /* end for */
	}

	pidfname[0] = '\0' ;
	lockfname[0] = '\0' ;
	logfname[0] = '\0' ;

/* start parsing the arguments */

	if (rs >= 0) {
	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;
	}

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

		    argval = (argp + 1) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {
	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;
	                } else {
	                    akl = aol ;
	                    avl = 0 ;
	                }

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

	                    case argopt_tmpdir:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->tmpdname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
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
					rs = SR_INVALID ;
	                        break ;

/* verbose mode */
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

	                    case argopt_help:
	                        f_help = TRUE ;
				break ;

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

/* configuration file */
	                    case argopt_config:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) 
					configfname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					configfname = argp ;
	                        }
	                        break ;

/* log file */
	                    case argopt_logfile:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                logfile_type = 0 ;
	                                strwcpy(logfname,avp,avl) ;
	                            }
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                logfile_type = 0 ;
	                                strwcpy(logfname,argp,argl) ;
	                            }
	                        }
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

/* argument list-file */
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

/* input file */
	                    case argopt_if:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
					ifname = avp ;
	                        } else {
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
					ifname = argp ;
	                        }
	                        break ;

/* ignore dots on input (default anyway!) */
			    case argopt_oi:
				pip->f.optin = TRUE ;
				break ;

/* handle all keyword defaults */
	                    default:
				rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */

	                } else {

	                    while (akl--) {
			        const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {

/* debug */
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

/* configuration file */
	                        case 'C':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					configfname = argp ;
	                            break ;

/* program-root */
	                        case 'R':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) 
					pr = argp ;
				    break ;

/* mutex lock PID file */
	                        case 'P':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                strwcpy(pidfname,argp,argl) ;
	                            break ;

/* quiet mode */
	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* daemon mode */
	                        case 'd':
	                            pip->f.daemon = TRUE ;
	                            break ;

/* ignore dots on input (default anyway!) */
	                        case 'i':
				    pip->f.optin = TRUE ;
	                            break ;

/* options */
	                        case 'o':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                            break ;

/* TCP port to listen on */
	                        case 'p':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                portspec = argp ;
	                            break ;

				case 'q':
	                            pip->verboselevel = 0 ;
	                            break ;

/* verbose mode */
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

	        if (ai >= MAXARGINDEX)
	            break ;

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

#if	CF_DEBUGS
		debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u verboselevel=%d\n",
	        pip->progname,
		pip->debuglevel,pip->verboselevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	} 

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	}

/* help */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* other initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

/* load the positional arguments */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	        switch (pan) {

	        case 0:
	            if ((strlen(argv[ai]) > 0) && (argv[ai][0] != '-'))
	                configfname = argv[ai] ;

	            break ;

	        case 1:
	            if ((strlen(argv[ai]) > 0) && (argv[ai][0] != '-'))
	                portspec = argv[ai] ;

	            break ;

	        default:
		    rs = SR_INVALID ;
	            bprintf(pip->efp,
	                "%s: extra positional arguments specified\n",
	                pip->progname) ;

	        } /* end switch */

	        pan += 1 ;

	    if (rs < 0) break ;
	} /* end for (loading positional arguments) */

	if (rs < 0)
		goto badarg ;


/* get the PCS configuration on this machine, needed for 'maildomain' */

#if	CF_DEBUGS
	debugprintf("main: pcsconf()\n") ;
#endif

#ifdef	COMMENT
	{
	vecstr		sets, extras ;


	vecstr_start(&sets,10,0) ;

	vecstr_start(&extras,10,0) ;

	rs1 = pcsconf(pip->pr,NULL,&cs,&sets,&extras,
	    pcsconfbuf,PCSCONF_LEN) ;

/* see if some optional parameters were for us in the PCS-wide configuration */

	(void) getpcsopts(gp,&sets) ;

	vecstr_finish(&extras) ;

	vecstr_finish(&sets) ;

	} /* end block */
#endif /* COMMENT */

/* did we have a port specified on the invocation */

	if ((rs >= 0) && (port < 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    port = rs ;
	}

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
	    ex = EX_NOUSER ;
	    goto baduser ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;
	pip->logid = u.logid ;

/* prepare to store configuration variable lists */

	if ((rs = vecstr_start(&defines,10,VECSTR_PORDERED)) < 0)
	    goto badlistinit ;

	if ((rs = vecstr_start(&exports,10,VECSTR_PORDERED)) < 0) {

	    vecstr_finish(&defines) ;

	    goto badlistinit ;
	}

/* create the values for the file schedule searching */

	pip->f.svars = TRUE ;
	vecstr_start(&schedvars,6,0) ;

	vecstr_envset(&schedvars,"p",pip->pr,-1) ;

	vecstr_envset(&schedvars,"e","etc",-1) ;

	vecstr_envset(&schedvars,"n",pip->searchname,-1) ;

/* load up some initial environment that everyone should have! */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("main: about to do INITFILE=%s\n",DEFINITFNAME) ;
#endif

	procfileenv(pip->pr,DEFINITFNAME,&exports) ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
		debugprintf("main: 0 configfile=%s\n",configfname) ;
#endif

/* search locally */

	rs = SR_NOEXIST ;
	if ((configfname == NULL) || (configfname[0] == '\0')) {

	    configfname = CONFIGFNAME ;

	    if ((sl = permsched(sched1,&schedvars,
	        tmpfname,MAXPATHLEN, configfname,R_OK)) < 0) {

	        if ((sl = permsched(sched3,&schedvars,
	            tmpfname,MAXPATHLEN, configfname,R_OK)) > 0)
	            configfname = tmpfname ;

	    } else if (sl > 0)
	        configfname = tmpfname ;

	    rs = sl ;

	} else {

	    sl = getfname(pip->pr,configfname,1,tmpfname) ;
	    if (sl > 0)
	        configfname = tmpfname ;

	} /* end if */

	if (configfname == tmpfname) {

	    f_freeconfigfname = TRUE ;
	    configfname = mallocstr(tmpfname) ;

	}

#if	CF_DEBUG
	if (pip->debuglevel >= 4) {
	    debugprintf("main: find rs=%d sl=%d\n",rs,sl) ;
	    debugprintf("main: 1 configfile=%s\n",configfname) ;
	}
#endif /* CF_DEBUG */


/* read in the configuration file if we have one */

	if ((rs >= 0) || (perm(configfname,-1,-1,NULL,R_OK) >= 0)) {

#if	CF_DEBUG
	    if (pip->debuglevel >= 4)
	        debugprintf("main: 2 configfile=%s\n", configfname) ;
#endif

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: configfile=%s\n",
	            pip->progname,configfname) ;

	    rs = configfile_start(&cf,configfname) ;
		if (rs < 0)
	        goto badconfig ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		debugprintf("main: we have a good configuration file\n") ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		debugprintf("main: arsub_start() d\n") ;
#endif

	    varsub_start(&vsh_d,VARSUB_MBADNOKEY) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_start() e\n") ;
#endif

	    varsub_start(&vsh_e,0) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: varsub_load environment_variables\n") ;
#endif

	    varsub_addva(&vsh_e,(const char **) envv) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("main: environment sub_array\n") ;
	        varsub_dumpfd(&vsh_e,-1) ;
	    }
#endif /* CF_DEBUG */


/* program root from configuration file */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: CF root=%s\n",cf.root) ;
#endif

	    if ((cf.root != NULL) && (! f_programroot)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF programroot\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.root,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

	            proginfo_setprogroot(pip,buf2,l2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: 1 programroot=%s\n",pip->pr) ;
#endif

	        }

	    } /* end if (configuration file program root) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 programroot=%s\n",pip->pr) ;
#endif


/* loop through the DEFINEd variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.defines,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf( "main: 0 top, cp=%s\n",cp) ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1) debugprintf(
	                "main: 0 about to merge\n") ;
#endif

	            varsub_merge(&vsh_d,&defines,buf2,l2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1) debugprintf(
	                "main: 0 out of merge\n") ;
#endif

	        } /* end if */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf( "main: 0 bot\n") ;
#endif

	    } /* end for (defines) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ defines\n") ;
#endif


/* loop through the EXPORTed variables in the configuration file */

	    for (i = 0 ; vecstr_get(&cf.exports,i,&cp) >= 0 ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1) debugprintf(
	            "main: 1 about to sub> %s\n",cp) ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,cp,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1) debugprintf(
	                "main: 1 about to merge> %W\n",buf,l2) ;
#endif

	            varsub_merge(NULL,&exports,buf2,l2) ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1) debugprintf(
	                "main: 1 done merging\n") ;
#endif

#if	CF_DEBUG && 0
	            if (pip->debuglevel > 1) {

	                debugprintf("varsub_merge: VSA_D so far \n") ;

	                varsub_dumpfd(&vsh_d,-1) ;

	                debugprintf("varsub_merge: VSA_E so far \n") ;

	                varsub_dumpfd(&vsh_e,-1) ;

	            } /* end if (debuglevel) */
#endif /* CF_DEBUG */

	        } /* end if (merging) */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: done subbing & merging\n") ;
#endif

	    } /* end for (exports) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: done w/ exports\n") ;
#endif


/* all of the rest of the configuration file stuff */

	    if ((cf.workdir != NULL) && (pip->workdname == NULL)) {

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.workdir,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

	            pip->workdname = mallocstrw(buf2,l2) ;

	        }

	    }

	    if (pip->f.daemon && (cf.pidfname != NULL) && 
	        (pidfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF pidfile=%s\n",cf.pidfname) ;
#endif

	        if ((cf.pidfname[0] != '\0') && (cf.pidfname[0] != '-')) {

	            if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	                cf.pidfname,-1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(pip,buf,l, buf2,BUFLEN)) > 0)) {

	                strwcpy(pidfname,buf2,l2) ;

	            }

	        }

	    } /* end if (configuration file PIDFILE) */


	    if ((cf.lockfname != NULL) && (lockfname[0] == '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF lockfile=%s\n",cf.lockfname) ;
#endif

	        if ((cf.lockfname[0] != '\0') && (cf.lockfname[0] != '-')) {

	            if (((l = varsub_subbuf(&vsh_d,&vsh_e,cf.lockfname,
	                -1,buf,BUFLEN)) > 0) &&
	                ((l2 = expander(pip,buf,l, buf2,BUFLEN)) > 0)) {

	                strwcpy(lockfname,buf2,l2) ;

	            }

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: lockfile=%s\n",lockfname) ;
#endif

	    } /* end if (configuration file LOCKFILE) */


#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        debugprintf("main: so far logfname=%s\n",logfname) ;

	        debugprintf("main: about to get config log filename\n") ;
	    }
#endif

	    if ((cf.logfname != NULL) && (logfile_type < 0)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: CF logfilename=%s\n",cf.logfname) ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.logfname,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

	            logfile_type = 1 ;
	            strwcpy(logfname,buf2,l2) ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processed CF logfilename=%s\n",
			logfname) ;
#endif

	    } /* end if (configuration file log filename) */

	    if ((cf.port != NULL) && (port < 0) && (portspec == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing port\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.port,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {
			const int	bch = MKCHAR(buf2[0]) ;

	            if (isalphalatin(bch)) {
			struct servent	*sep ;

	                sep = getservbyname(buf2, "tcp") ;

	                if (sep == NULL)
	                    goto badconfigport ;

	                port = (int) ntohs(sep->s_port) ;

	            } else if (cfdeci(buf2,l2,&port) < 0)
	                goto badconfigport ;

	        }

	    } /* end if (handling the configuration file port parameter) */

	    if ((cf.userpass != NULL) && (pip->userpass == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF user password\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.userpass,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

	            pip->userpass = mallocstrw(buf2,l2) ;

	        }

	    }

	    if ((cf.machpass != NULL) && (pip->machpass == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF machine password\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.machpass,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: machpass subed and expanded\n") ;
#endif

	            pip->machpass = mallocstrw(buf2,l2) ;

	        }

	    } /* end if */

	    if ((cf.sendmail != NULL) && (pip->prog_sendmail == NULL)) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF sendmail\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.sendmail,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: sendmail subed and expanded\n") ;
#endif

	            pip->prog_sendmail = mallocstrw(buf2,l2) ;

	        }

	    } /* end if (sendmail) */

	    if (cf.nrecips != NULL) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF nrecips\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.nrecips,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: nrecips subed and expanded\n") ;
#endif

	            if ((cfdeci(buf2,l2,&argvalue) >= 0) && (argvalue >= 1))
	                pip->nrecips = argvalue ;

	        }

	    } /* end if (nrecips) */

	    if (cf.options != NULL) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing CF options\n") ;
#endif

	        if (((l = varsub_subbuf(&vsh_d,&vsh_e,
	            cf.options,-1,buf,BUFLEN)) > 0) &&
	            ((l2 = expander(pip,buf,l,buf2,BUFLEN)) > 0)) {

	            procopts(pip,buf2,l2) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: options> sender=%d\n",pip->f.sender) ;
#endif

	        }

	    } /* end if (options) */


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

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
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
	if (pip->debuglevel > 1)
	    debugprintf("main: finished with any configfile stuff\n") ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	if (pip->f.svars) {
	    pip->f.svars = FALSE ;
	    vecstr_finish(&schedvars) ;
	}

/* before we go too far, are we the only one on this PID mutex? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 pidfname=%s\n",pidfname) ;
#endif

	if (pip->f.daemon && (pidfname[0] != '\0')) {

	    if (pidfname[0] == '-')
	        strcpy(pidfname,PIDFNAME) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 1 pidfname=%s\n",pidfname) ;
#endif

	    rs = bopenroot(&pidfile,pip->pr,pidfname,tmpfname,
	        "rwc",0664) ;

	    pip->pidfname = pidfname ;
	    if (rs < 0) 
		goto badpidopen ;

	    if (tmpfname[0] != '\0')
	        strcpy(pidfname,tmpfname) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: 2 pidfname=%s\n",pidfname) ;
#endif

/* capture the lock (if we can) */

	    if ((rs = bcontrol(&pidfile,BC_LOCK,0)) < 0)
	        goto badpidlock ;

	    bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	    bseek(&pidfile,0L,SEEK_SET) ;

	    bprintf(&pidfile,"%d\n",pip->pid) ;

	    bprintf(&pidfile,"%s!%s\n",pip->nodename,pip->username) ;

	    bclose(&pidfile) ;		/* this releases the lock */

	} /* end if (we have a mutex PID file) */

/* check program parameters */

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		debugprintf("main: checking program parameters\n") ;
#endif

	if (pip->workdname == NULL)
	    pip->workdname = WORKDNAME ;

	else if (pip->workdname[0] == '\0')
	    pip->workdname= "." ;

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {

	    if ((pip->tmpdname = getenv("TMPDIR")) == NULL)
	        pip->tmpdname = TMPDNAME ;

	} /* end if (tmpdir) */

/* lock file */

	if (lockfname[0] == '\0')
	    strcpy(lockfname,LOCKFNAME) ;

	sl = getfname(pip->pr,lockfname,1,tmpfname) ;

	if (sl > 0)
	    strwcpy(lockfname,tmpfname,sl) ;

	pip->lockfname = lockfname ;

/* what? */

	pip->xfname = xfnamebuf ;
	mkpath2(xfnamebuf, pip->tmpdname, "rmailerXXXXXXX") ;

/* MTA */

	if (pip->prog_sendmail == NULL)
	    pip->prog_sendmail = mallocstr(PROG_SENDMAIL) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: sendmail=%s\n",
	        pip->prog_sendmail) ;
#endif

/* can we access the working directory? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: access working directory=%s\n",
		pip->workdname) ;
#endif

	if ((u_access(pip->workdname,X_OK) < 0) || 
		(u_access(pip->workdname,R_OK) < 0))
	    goto badworking ;

/* do we have an activity log file? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 0 logfname=%s\n",logfname) ;
#endif

	rs = SR_BAD ;
	if (logfname[0] == '\0') {

	    logfile_type = 1 ;
	    mkpath1(logfname,LOGFNAME) ;

	}

	if ((sl = getfname(pip->pr,logfname,logfile_type,tmpfname)) > 0)
	    strwcpy(logfname,tmpfname,sl) ;

	rs = logfile_open(&pip->lh,logfname,0,0666,buf) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: 1 logfname=%s rs=%d\n",logfname,rs) ;
#endif

	if (rs >= 0) {
		pip->open.logprog = TRUE ;

/* we opened it, maintenance this log file if we have to */

	    if (loglen < 0) 
		loglen = LOGSIZE ;

	    logfile_checksize(&pip->lh,loglen) ;

	    logfile_userinfo(&pip->lh,&u,
		pip->daytime,pip->progname,pip->version) ;

	} /* end if (we have a log file or not) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: continuing with some PID stuff\n") ;
#endif

	pip->gid = getgid() ;

	if ((rs = getgr_gid(&ge,buf,BUFLEN,pip->gid)) >= 0) {
	    cp = ge.gr_name ;
	} else if (isNotPresent(rs)) {
	    cp = buf ;
	    snsd(buf,BUFLEN,"G",(uint) pip->gid) ;
	}

	pip->groupname = mallocstr(cp) ;

/* if we are a daemon program, try to bind our INET port? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: about to check if we are a daemon\n") ;
#endif

	if (pip->f.daemon) {
	    struct protoent	*pep ;
	    long		addr ;

/* look up some miscellaneous stuff in various databases */

	    if (portspec != NULL) {
		rs = getportnum("tcp",portspec) ;
		port = rs ;
	    }

	    if (port < 0) {
		struct servent	*sep ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: default port \n") ;
#endif

	        if ((sep = getservbyname(PORTNAME,"tcp")) != NULL) {
	            port = (int) ntohs(sep->s_port) ;
	        } else {
	            port = PORT ;
		}

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: done looking at port stuff so far\n") ;
#endif

	    } /* end if (no port specified) */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to get a protocol number\n") ;
#endif

/* get the protocol number */

	    if ((pep = getprotobyname("tcp")) != NULL) {
	        proto = pep->p_proto ;
	    } else {
	        proto = IPPROTO_TCP ;
	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to call 'socket'\n") ;
#endif

	    if ((s = u_socket(PF_INET, SOCK_STREAM, proto)) < 0)
	        goto badsocket ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: socket for daemon, s=%d\n",s) ;
#endif

	    memset(&sa_server, 0, sizeof(struct sockaddr)) ;

	    sa_server.sin_family = AF_INET ;
	    sa_server.sin_port = htons(port) ;
	    addr = htonl(INADDR_ANY) ;

	    memcpy((char *) &sa_server.sin_addr, &addr, sizeof(long)) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to bind to port=%d\n",
	            port) ;
#endif

	    if ((rs = u_bind(s, (struct sockaddr *) &sa_server, 
	        sizeof(struct sockaddr))) < 0) {

	        u_close(s) ;

	        ex = EX_TEMPFAIL ;
	        goto badbind ;
	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: about to listen\n") ;
#endif

	    rs = u_listen(s,10) ;
	    if (rs < 0) {

	        u_close(s) ;
		s = -1 ;

	        ex = EX_OSERR ;
	        goto badlisten ;
	    }

	    logfile_printf(&pip->lh,
		"daemon listening on TCP port %d\n",port) ;

/* background ourselves */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf( "main: become a daemon?\n") ;
#endif

	    bflush(pip->efp) ;

	    if (pip->debuglevel == 0) {

#ifdef	COMMENT
	        for (i = 0 ; i < 3 ; i += 1) {
	            u_close(i) ;
	            (void) u_open("/dev/null",O_RDONLY,0666) ;
	        } /* end for */
#endif /* COMMENT */

	        rs = uc_fork() ;
	        if (rs < 0) {
	            logfile_printf(&pip->lh,"cannot fork daemon (%d)\n",rs) ;
		    logfile_close(&pip->lh) ;
	            uc_exit(EX_OSERR) ;
	        }

	        if (rs > 0) 
			uc_exit(EX_OK) ;

	        u_setsid() ;
	    } /* end if (backgrounding) */

	} else if ((ifname != NULL) && (ifname[0] != '\0')) {

		s = uc_open(ifname,O_RDONLY,0666) ;

	} else
	    s = FD_STDIN ;

/* we start! */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: starting, s=%d\n",s) ;
#endif

	pip->pid = getpid() ;

/* reload the userinfo structure with our new PID */

	if (userbuf[0] != '\0')
	    u.pid = pip->pid ;

#ifdef	COMMENT
	(void) storebuf_dec(buf,BUFLEN,0,pip->pid) ;

	logfile_setid(&pip->lh,buf) ;
#endif /* COMMENT */

/* before we go too far, are we the only one on this PID mutex? */

	if (pip->f.daemon) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: more daemon stuff\n") ;
#endif

	    if (rs == 0)
	        logfile_printf(&pip->lh,"daemon pid=%d\n",pip->pid) ;

	    if ((pip->pidfname != NULL) && (pip->pidfname[0] != '\0')) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: we have a PIDFILE=%s\n",pip->pidfname) ;
#endif

	        if ((rs = bopen(&pidfile,pip->pidfname,"rwc",0664)) < 0)
	            goto badpidfile2 ;

/* capture the lock (if we can) */

	        if ((rs = bcontrol(&pidfile,BC_LOCK,2)) < 0) {

	            bclose(&pidfile) ;

	            goto badpidfile3 ;
	        }

	        bcontrol(&pidfile,BC_TRUNCATE,0L) ;

	        bseek(&pidfile,0L,SEEK_SET) ;

	        bprintf(&pidfile,"%d\n",pip->pid) ;

	        bprintf(&pidfile,"%s!%s\n",
			pip->username,pip->nodename) ;

	        bprintf(&pidfile,"%s %s\n",
	            BANNER,timestr_logz(pip->daytime,timebuf)) ;

	        if (userbuf[0] != '\0')
	            bprintf(&pidfile,"host=%s.%s user=%s pid=%d\n",
	                u.nodename,u.domainname,
	                u.username,
	                pip->pid) ;

	        else
	            bprintf(&pidfile,"host=%s.%s pid=%d\n",
	                u.nodename,u.domainname,
	                pip->pid) ;

	        bflush(&pidfile) ;

/* we leave the file open as our mutex lock! */

	        logfile_printf(&pip->lh,"pidfile=%s\n",pip->pidfname) ;

	        logfile_printf(&pip->lh,"PID mutex captured\n") ;

	        bcontrol(&pidfile,BC_STAT,&sb) ;

	        logfile_printf(&pip->lh,"pidfile device=%ld inode=%ld\n",
	            sb.st_dev,sb.st_ino) ;

	        pip->pidfp = &pidfile ;

	    } /* end if (we have a mutex PID file) */

	} /* end if (daemon mode) */

	if (userbuf[0] != '\0') {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: log entry\n") ;
#endif
	    pip->daytime = time(NULL) ;

	    if (pip->f.daemon)
	        logfile_printf(&pip->lh,"%s finished initializing\n",
	            timestr_logz(pip->daytime,timebuf)) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: log entry 2\n") ;
#endif

	} /* end if (making log entries) */

	if (pip->f.daemon) {

	    rs = watch(pip,s,&exports) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: watch() ret rs=%d\n",rs) ;
#endif

	} else {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: calling 'handle'\n") ;
#endif

	    rs = handle(pip,s,&exports) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: handle() ret rs=%d\n",rs) ;
#endif

	} /* end if */

	if (s >= 0) {
	    u_close(s) ;
	    s = -1 ;
	}

/* done */
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* we are done */
daemonret:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif /* CF_DEBUG */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: freeing configfname\n") ;
#endif /* CF_DEBUG */

	if (pip->open.logprog) {
	    pip->open.logprog = FALSE ;
	    logfile_close(&pip->lh) ;
	}

	if (f_freeconfigfname) {
	    f_freeconfigfname = FALSE ;
	    uc_free(configfname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: about to 'configfree'\n") ;
#endif /* CF_DEBUG */

	configfile_finish(&cf) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: past configfree, about to vecstr_finish\n") ;
#endif /* CF_DEBUG */

	vecstr_finish(&exports) ;

	vecstr_finish(&defines) ;

/* early return */
baduser:
retearly:
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex-%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_shcat: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* USAGE> rexd [-C conf] [-p port] [-V?] */
usage:
	usage(pip) ;

	goto retearly ;

help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

/* other bad stuff */
badworking:
	ex = EX_UNAVAILABLE ;
	bprintf(pip->efp,"%s: could not access the working directory\n",
	    pip->progname) ;

	bprintf(pip->efp,"%s: directory=%s\n",
	    pip->progname,pip->workdname) ;

	goto badret ;

badlistinit:
	bprintf(pip->efp,
		"%s: could not initialize list structures (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badconfig:
	bprintf(pip->efp,
	    "%s: error (%d) in configuration file starting at line %d\n",
	    pip->progname,rs,cf.badline) ;

	goto badret ;

badconfigport:
	bprintf(pip->efp,
	    "%s: the port number in the configuration file was bad\n",
	    pip->progname) ;

	goto badret ;

badpidopen:
	bprintf(pip->efp,
	    "%s: could not open the PID file (%d)\n",
	    pip->progname,rs) ;

	bprintf(pip->efp, "%s: pidfile=%s\n", 
		pip->progname,pip->pidfname) ;

	goto badret ;

badpidlock:
	if (! pip->f.quiet) {

	    bprintf(pip->efp,
	        "%s: could not lock the PID file (%d)\n",
	        pip->progname,rs) ;

	    bprintf(pip->efp, "%s: pidfile=%s\n", pip->progname,pip->pidfname) ;

	    while ((len = breadline(&pidfile,buf,BUFLEN)) > 0) {

	        bprintf(pip->efp,"%s: pidfile> %t",
	            pip->progname,
	            buf,len) ;

	    }

	    bclose(&pidfile) ;

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: PID lock failed, rs=%d\n",rs) ;
#endif

	goto badret ;

badsocket:
	if (! pip->f.quiet)
	    bprintf(pip->efp,
	        "%s: could not create a socket to listen on (%d)\n",
	        pip->progname,rs) ;

	goto badret ;

badbind:
	if (! pip->f.quiet)
	    bprintf(pip->efp,
	        "%s: could not bind to our port number (%d)\n",
	        pip->progname,rs) ;

	goto ret2 ;

badlisten:
	if (! pip->f.quiet)
	    bprintf(pip->efp,
	        "%s: could not listen to our server socket (%d)\n",
	        pip->progname,rs) ;

	goto ret2 ;

badpidfile2:
	logfile_printf(&pip->lh,
	    "could not open the PID mutext file (%d)\n",
	    rs) ;

	logfile_printf(&pip->lh, "pidfile=%s\n", pip->pidfname) ;

	goto daemonret ;

badpidfile3:
	logfile_printf(&pip->lh,
	    "there was a daemon already on the mutex file, PID=%d\n",
	    pip->pid) ;

	goto daemonret ;

/* general bad return */
badret:
	ex = EX_DATAERR ;
	goto ret2 ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [{<conf>|-} [<port>]] [-C <conf>] [-p <port>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program options */

static const uchar	oterms[32] = {
	0x00, 0x0B, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
} ;

static int procopts(PROGINFO *pip,char *buf,int buflen)
{
	FIELD		rawopts ;
	int		rs ;
	int		fl ;
	int		i ;
	int		v ;
	int		c = 0 ;
	const char	*fp ;
	const char	*tp ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main/procopts: options\n") ;
#endif

	if ((rs = field_start(&rawopts,buf,buflen)) >= 0) {

	while ((fl = field_get(&rawopts,oterms,&fp)) >= 0) {
	    int		klen, vlen ;
	    const char	*key, *val ;

	    if (fl == 0) continue ;

	    key = fp ;
	    klen = fl ;
	    val = NULL ;
	    vlen = 0 ;
	    if ((tp = strnchr(fp,fl,'=')) != NULL) {
	        klen = (tp - fp) ;
	        val = (tp + 1) ;
	        vlen = fl - (val - fp) ;
	    }

	    if ((i = matostr(progopts,2,key,klen)) >= 0) {

		c += 1 ;
	        switch (i) {
	        case progopt_sender:
	            pip->f.sender = TRUE ;
	            if (vlen > 0) {
	                if (cfdeci(val,vlen,&v) >= 0)
	                    pip->f.sender = (v != 0) ? 1 : 0 ;
	            }
		    break ;
		default:
		    rs = SR_NOANODE ;
		    break ;
	        } /* end switch */

	    } /* end if */

	    if (rs < 0) break ;
	} /* end while */

	field_finish(&rawopts) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


