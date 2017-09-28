/* main (MAINTUTMP) */

/* perform some maintenance on the UTMPX database */


#define	CF_DEBUGS	0		/* run-time debugging */
#define	CF_DEBUG	0		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_SYSLEN	0		/* use UTMPX 'syslen' */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for small programs.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<sighand.h>
#include	<bfile.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<paramopt.h>
#include	<mapstrint.h>
#include	<tmpx.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	makedate_date(const char *,const char **) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern const char	utmpmaint_makedate[] ;


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		list:1 ;
	uint		maint:1 ;
	uint		hdr:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	int		to ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,
			MAPSTRINT *,cchar *,cchar *,cchar *) ;
static int	procname(PROGINFO *,MAPSTRINT *,const char *,int) ;
static int	procmaint(PROGINFO *,const char *) ;
static int	procmaintbake(PROGINFO *,void *,size_t,int) ;
static int	proclist(PROGINFO *,bfile *,MAPSTRINT *,const char *) ;
static int	procuser(PROGINFO *,MAPSTRINT *,cchar *,int,int *) ;

static void	main_sighand(int,siginfo_t *,void *) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_intr ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"db",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_db,
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

static const char	*akonames[] = {
	"print",
	"hdr",
	NULL
} ;

enum akonames {
	akoname_print,
	akoname_hdr,
	akoname_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	SIGHAND		sm ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	apam ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs ;
	int		rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_makedate = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*dbfname = NULL ;
	const char	*fmt ;
	const char	*cp ;


	if_exit = 0 ;
	if_intr = 0 ;

	rs = sighand_start(&sm,sigblocks,sigignores,sigints,main_sighand) ;
	if (rs < 0) goto badsighand ;

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

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&apam) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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

/* version */
	                case argopt_version:
	                    f_makedate = f_version ;
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose */
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

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->tmpdname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* database file */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                dbfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* argument-list file */
	                case argopt_af:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            afname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                efname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* get an output file name other than using STDOUT! */
	                case argopt_of:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
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

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

	                    case 'h':
	                        lip->final.hdr = TRUE ;
	                        lip->have.hdr = TRUE ;
	                        lip->f.hdr = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.hdr = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* maintenance */
	                    case 'm':
	                        lip->f.maint = TRUE ;
	                        break ;

/* list */
	                    case 'l':
	                        lip->f.list = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					KEYOPT	*kop = &akopts ;
	                                rs = keyopt_loads(kop,argp,argl) ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* print out */
	                    case 'p':
	                        pip->final.print = TRUE ;
	                        pip->have.print = TRUE ;
	                        pip->f.print = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.print = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* other things */
	                    case 's':
	                        cp = NULL ;
	                        cl = -1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                cp = avp ;
	                                cl = avl ;
	                            }
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl) {
	                                    cp = argp ;
	                                    cl = argl ;
	                                }
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        if ((rs >= 0) && (cp != NULL)) {
				    PARAMOPT	*pop = &apam ;
	                            cchar	*po = PO_OPTION ;
	                            rs = paramopt_loads(pop,po,cp,cl) ;
	                        }
	                        break ;

/* verbose output */
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
	                        f_usage = TRUE ;
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
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
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: version %s\n",pn,VERSION) ;
	    if (f_makedate) {
	        cl = makedate_date(utmpmaint_makedate,&cp) ;
	        bprintf(pip->efp,"%s: makedate %t\n",pn,cp,cl) ;
	    }
	} /* end if */

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: f_maint=%u\n",pn,lip->f.maint) ;
	    bprintf(pip->efp,"%s: f_list=%u\n",pn,lip->f.list) ;
	}

/* remaining initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (dbfname == NULL) dbfname = getenv(VARDB) ;
	if (dbfname == NULL) dbfname = getenv(VARDBFNAME) ;

	if ((pip->debuglevel > 0) && (dbfname != NULL)) {
	    bprintf(pip->efp,"%s: dbfile=%s\n",pip->progname,dbfname) ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    MAPSTRINT	names ;
	    if ((rs = mapstrint_start(&names,10)) >= 0) {

	        if (lip->f.maint) {
	            rs = procmaint(pip,dbfname) ;
	        }

	        if ((rs >= 0) && (pip->f.print || lip->f.list)) {
	            ARGINFO	*aip = &ainfo ;
	            BITS	*bop = &pargs ;
	            cchar	*dfn = dbfname ;
	            cchar	*ofn = ofname ;
	            cchar	*afn = afname ;
	            rs = procargs(pip,aip,bop,&names,dfn,ofn,afn) ;
	        } /* end if (print or list) */

	        rs1 = mapstrint_finish(&names) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        ex = EX_OSERR ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    fmt = NULL ;
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOINPUT ;
	        fmt = "%s: database unavailable (%d)\n" ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        fmt = "%s: processing error (%d)\n" ;
	        break ;
	    } /* end switch */
	    if (! pip->f.quiet) {
	        if (fmt != NULL)
	            bprintf(pip->efp,fmt,pip->progname,rs) ;
	    }
	} else if (rs >= 0) {
	    if (if_exit) {
	        ex = EX_TERM ;
	    } else if (if_intr) {
	        ex = EX_INTR ;
	    }
	} /* end if (error) */

/* we are out of here */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d) \n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&apam) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sighand_finish(&sm) ;

badsighand:
	return ex ;

/* bad stuff comes here */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


/* ARGSUSED */
static void main_sighand(int sn,siginfo_t *sip,void *vcp)
{
	switch (sn) {
	case SIGINT:
	    if_intr = TRUE ;
	    break ;
	case SIGKILL:
	    if_exit = TRUE ;
	    break ;
	default:
	    if_exit = TRUE ;
	    break ;
	} /* end switch */
}
/* end subroutine (main_sighand) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-db <dbfile>] [-m] [-l] [<name>[=<type>]]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;
	lip->f.hdr = OPT_HDR ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(&lip->stores,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(&lip->stores,oi) ;
	    }
	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_print:
	                    if (! pip->final.print) {
	                        pip->have.print = TRUE ;
	                        pip->final.print = TRUE ;
	                        pip->f.print = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.print = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_hdr:
	                    if (! lip->final.hdr) {
	                        lip->have.hdr = TRUE ;
	                        lip->final.hdr = TRUE ;
	                        lip->f.hdr = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.hdr = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (cursor) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(pip,aip,bop,nlp,dbfn,ofn,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
MAPSTRINT	*nlp ;
const char	*dbfn ;
const char	*ofn ;
const char	*afn ;
{
	LOCINFO		*lip = pip->lip ;
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: opening output file=%s\n",ofn) ;
#endif

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procname(pip,nlp,cp,-1) ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

/* process any names in the argument filename list file */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	        if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procname(pip,nlp,cp,cl) ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                fmt = "%s: inaccessible argument list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list */

/* process regular requests */

	    if ((rs >= 0) && lip->f.list) {
	        rs = proclist(pip,ofp,nlp,dbfn) ;
	        c += rs ;
	    }

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: output unavailable (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procname(PROGINFO *pip,MAPSTRINT *nlp,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		v = -1 ;
	int		cl ;
	const char	*tp ;
	const char	*cp ;

	if (pip == NULL) return SR_FAULT ;

	if (nl < 0) nl = strlen(np) ;

	if ((tp = strnchr(np,nl,'=')) != NULL) {
	    nl = (tp-np) ;
	    cp = (tp+1) ;
	    cl = ((np+nl) - (tp+1)) ;
	    rs = cfdeci(cp,cl,&v) ;
	    if (v < 0) v = 0 ;
	}

	if (rs >= 0) {
	    rs = mapstrint_add(nlp,np,nl,v) ;
	}

	return rs ;
}
/* end subroutine (procname) */


static int procmaint(PROGINFO *pip,cchar *fn)
{
	const mode_t	om = 0666 ;
	const int	of = O_RDWR ;
	const int	ps = getpagesize() ;
	int		rs ;

	if (fn == NULL) fn = "/var/adm/utmpx" ;

	if ((rs = uc_open(fn,of,om)) >= 0) {
	    const int	fd = rs ;
	    if ((rs = uc_fsize(fd)) >= 0) {
	        void	*md ;
	        size_t	ms = MAX(rs,ps) ;
	        int	mp = (PROT_READ | PROT_WRITE) ;
	        int	mf = MAP_SHARED ;
	        int	fs = rs ;
	        if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	            const caddr_t	ma = md ;
	            const int		madv = MADV_SEQUENTIAL ;
	            if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
	                if ((rs = procmaintbake(pip,md,ms,fs)) > 0) {
#if	CF_DEBUG
	                    if (DEBUGLEVEL(3))
	                        debugprintf("main/procmaint: trunc o=%u\n",rs) ;
#endif
	                    rs = uc_ftruncate(fd,rs) ;
	                }
	            } /* end if (advise) */
	            u_munmap(md,ms) ;
	        } /* end if (mmap) */
	    } /* end if (file-size) */
	    u_close(fd) ;
	} /* end if (file-open) */

	return rs ;
}
/* end subroutine (procmaint) */


/* ARGSUSED */
static int procmaintbake(PROGINFO *pip,void *md,size_t ms,int fs)
{
	TMPX_ENT	*wup = NULL ;
	TMPX_ENT	*lup = NULL ;
	TMPX_ENT	*up ;
	const int	es = sizeof(TMPX_ENT) ;
	int		rs = SR_OK ;
	cchar		*mp, *mep ;

	mp = (cchar *) md ;
	mep = (char *) md ;
	while (mep < (mp+fs)) {
	    up = (TMPX_ENT *) mep ;

	    switch (up->ut_type) {
	    case TMPX_TLOGINPROC:
	        {
	            const pid_t	sid = up->ut_pid ;
	            if ((rs = u_kill(sid,0)) == SR_SRCH) {
			rs = SR_OK ;
	                if (strcmp(up->ut_line,"/dev/console") == 0) {
	                    if (wup == NULL) wup = up ;
	                    up = NULL ;
	                } else if (strncmp(up->ut_id,"PM",2) == 0) {
	                    if (wup == NULL) wup = up ;
	                    up = NULL ;
	                } else if (up->ut_line[0] == '\0') {
	                    if (wup == NULL) wup = up ;
	                    up = NULL ;
	                }
	            } else if (rs == SR_PERM) {
	                rs = SR_OK ;
	            } /* end if (no-process) */
	        } /* end block */
	        break ;
	    case TMPX_TDEADPROC:
	        if (strncmp(up->ut_line,"pts",3) == 0) {
	            if (wup == NULL) wup = up ;
	            up = NULL ;
	        } else if (strncmp(up->ut_id,"PM",2) == 0) {
	            if (wup == NULL) wup = up ;
	            up = NULL ;
	        }
	        break ;
	    case TMPX_TEMPTY:
	        up = NULL ;
	        break ;
	    } /* end switch */
	    if (wup != NULL) {
	        if (up != NULL) {
	            lup = wup ;
	            *wup++ = *up ;
	        }
	    } else {
	        if (up != NULL) lup = up ;
	    }

	    mep += es ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (lup != NULL)) {
	    offset_t	feo ;
	    cchar	*mlp = (cchar *) (lup+1) ;
	    feo = (mlp-mp) ;
	    if (feo < fs) {
	        rs = (int) (feo & INT_MAX) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (procmaintbake) */


static int proclist(PROGINFO *pip,bfile *ofp,MAPSTRINT *nlp,cchar *dbfn)
{
	TMPX		ut ;
	TMPX_CUR	ucur ;
	TMPX_ENT	ue, *up = &ue ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	int		f_restrict = FALSE ;
	char		timebuf[TIMEBUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclist: ufn=%s\n",dbfn) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: ufn=%s\n",pip->progname,dbfn) ;
	}

	if (pip->verboselevel > 1) {
	    rs1 = bprintf(ofp,"ufn=%s\n",dbfn) ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/proclist: bprintf() rs=%d\n",rs1) ;
#endif
	}

	if ((rs = mapstrint_count(nlp)) > 0) {
	    f_restrict = TRUE ;
	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: restrict=%u\n",pip->progname,rs) ;
	    }
	}

	if (rs >= 0) {
	    if ((rs = tmpx_open(&ut,dbfn,O_RDONLY)) >= 0) {
	        if ((rs = tmpx_curbegin(&ut,&ucur)) >= 0) {
	            cchar	*fmt ;
	            fmt = "T   ID %-12s %-12s %6s SN EX TIME\n" ;

	            rs = bprintf(ofp,fmt,"USER","LINE","SID") ;
	            fmt = "%1u %4t %-12t %-12t %6u %2d %2d %s\n" ;

	            while (rs >= 0) {
			int	v = -1 ;
	                int	f = TRUE ;
	                rs1 = tmpx_enum(&ut,&ucur,up) ;
	                if (rs1 == SR_NOTFOUND) break ;
	                rs = rs1 ;

	                if ((rs >= 0) && f_restrict) {
	                    const int	nl = strnlen(up->ut_user,TMPX_LUSER) ;
	                    cchar	*np = up->ut_user ;
			    f = FALSE ;
			    if ((rs = procuser(pip,nlp,np,nl,&v)) > 0) {
				f = ((v < 0) || (up->ut_type == v)) ;
			    }
	                } /* end if (restriction) */

	                if ((rs >= 0) && f) {
	                    c += 1 ;
	                    timestr_log(up->ut_tv.tv_sec,timebuf) ;
	                    rs = bprintf(ofp,fmt,
	                        up->ut_type,
	                        up->ut_id,strnlen(up->ut_id,TMPX_LID),
	                        up->ut_user,strnlen(up->ut_user,TMPX_LUSER),
	                        up->ut_line,strnlen(up->ut_line,TMPX_LLINE),
	                        up->ut_pid,
	                        up->ut_session,
	                        up->ut_exit.e_exit,
	                        timebuf) ;

#if	CF_SYSLEN
	                    bprintf(ofp, "sl=%u host=%t\n",
	                        up->ut_syslen,
	                        up->ut_host,strnlen(up->ut_host,TMPX_LHOST)) ;
#endif
	                } /* end if */

	            } /* end while */

	            rs1 = tmpx_curend(&ut,&ucur) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (cursor) */
	        rs1 = tmpx_close(&ut) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (tmpx) */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclist: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclist) */


static int procuser(PROGINFO *pip,MAPSTRINT *nlp,cchar *np,int nl,int *vp)
{
	const int	nrs = SR_NOTFOUND ;
	int		rs ;
	int		f = TRUE ;

	if ((rs = mapstrint_already(nlp,np,nl)) == nrs) {
	    rs = SR_OK ;
	    f = FALSE ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
		debugprintf("main/proclist: already\n") ;
#endif
	} else if ((rs = mapstrint_fetch(nlp,np,nl,NULL,vp)) >= 0) {
	     f = TRUE ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (procuser) */


