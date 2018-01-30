/* main (ecover) */

/* main subroutine (for the ECOVER program) */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_LOCSETENT	0		/* |locinfo_setentry()| */


/* revision history:

	= 1999-06-13, David A­D­ Morano
	This subroutine was originally written.

	= 2003-04-28, David A­D­ Morano
	This has been slightly refactored.

	= 2008-09-29, David A­D­ Morano
	I updated this to use PROGLOG.

*/

/* Copyright © 1999,2003,2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This is the main subroutine for the ECOVER program.

	Synopsis:

	$ ecover -d <infile> > <outfile>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<logfile.h>
#include	<userinfo.h>
#include	<baops.h>
#include	<varsub.h>
#include	<sbuf.h>
#include	<randomvar.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"ecmsg.h"
#include	"ecinfo.h"
#include	"config.h"
#include	"defs.h"
#include	"proglog.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	BUFLEN
#define	BUFLEN		((8 * 1024) + REALNAMELEN)
#endif

#ifndef	NOISEBUFLEN
#define	NOISEBUFLEN	100
#endif

#ifndef	FILEINFO
#define	FILEINFO	struct fileinfo
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern uint	hashelf(cchar *,int) ;

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	iceil(int,int) ;
extern int	ecmsg_loadfile(ECMSG *,cchar *) ;
extern int	isdigitlatin(int) ;
extern int	isalnumlatin(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	lightnoise(PROGINFO *,USERINFO *,cchar *) ;
extern int	encode(PROGINFO *,RANDOMVAR *,
			FILEINFO *,ECMSG *,int,int) ;
extern int	decode(PROGINFO *,RANDOMVAR *,
			FILEINFO *,ECMSG *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		audit:1 ;
	uint		extra:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	ECMSG		extra ;
	PROGINFO	*pip ;
	cchar		*jobid ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,cchar *,cchar *) ;
static int	procereport(PROGINFO *,FILEINFO *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	proclog_begin(PROGINFO *) ;
static int	proclog_end(PROGINFO *) ;

static int	procout_begin(PROGINFO *,cchar *) ;
static int	procout_end(PROGINFO *,int) ;

static int	procinput_begin(PROGINFO *,cchar *) ;
static int	procinput_end(PROGINFO *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_loadmsg(LOCINFO *,cchar *,int) ;
static int	locinfo_loadfile(LOCINFO *,cchar *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSETENT */


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"HELP",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"lf",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"mf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_help,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_lf,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_msgfile,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static cchar	*akonames[] = {
	"audit",
	"logsize",
	NULL
} ;

enum akonames {
	akoname_audit,
	akoname_logsize,
	akoname_overlast
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	KEYOPT		akopts ;
	bfile		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		size ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*helpfname = NULL ;
	const char	*cp ;
	char		argpresent[MAXARGGROUPS] ;

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
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;
	pip->f.logprog = TRUE ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

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

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pr = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

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

/* log file */
	                case argopt_logfile:
	                case argopt_lf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->lfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->lfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_msgfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->msgfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->msgfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* print out the help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* progaram search-name */
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

/* output file */
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
	                            if (argl)
	                                ofname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* input file */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ifname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* unscramble */
	                    case 'u':
	                    case 'd':
	                        pip->f.unscramble = TRUE ;
	                        break ;

/* scramble */
	                    case 's':
	                    case 'e':
	                        pip->f.unscramble = FALSE ;
	                        break ;

/* job-ID */
	                    case 'j':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                lip->jobid = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* message text */
	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = locinfo_loadmsg(lip,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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

/* print usage summary */
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

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
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

/* get our program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help) {
	    if ((helpfname == NULL) || (helpfname[0] == '\0')) {
	        helpfname = HELPFNAME ;
	    }
	    printhelp(NULL,pip->pr,pip->searchname,helpfname) ;
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;

	ex = EX_OK ;

/* defaults */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0')) {
	    pip->tmpdname = TMPDNAME ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: mode=%s\n", pip->progname,
	        ((pip->f.unscramble) ? "decode" : "encode")) ;
	}

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
	        switch (pan) {
	        case 0:
	            ifname = cp ;
	            break ;
	        } /* end switch */
	        pan += 1 ;
	    }

	    if (ifname != NULL) break ;
	} /* end for (processing positional arguments) */

	if (ifname == NULL) ifname = getourenv(envv,VARIFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

	if (pip->lfname == NULL) pip->lfname = getourenv(envv,VARLFNAME) ;

	if ((rs >= 0) && (pip->logsize == 0)) {
	    if ((cp = getourenv(envv,VARLOGSIZE)) != NULL) {
		int	v ;
		rs = cfdecmfi(cp,-1,&v) ;
		pip->logsize = v ;
	    }
	}

	if (lip->jobid == NULL) lip->jobid = getourenv(envv,VARJOBID) ;

/* initialize (calculate) the number of OPwords to store ECINFO */

	size = sizeof(ECINFO) ;
	pip->necinfo = iceil(size,sizeof(ULONG)) / sizeof(ULONG) ;
	if (pip->necinfo > (NOPWORDS/2)) pip->necinfo = (NOPWORDS/2) ;

/* continue */

	if (rs >= 0) {
	    USERINFO	u ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            if ((rs = proglog_begin(pip,&u)) >= 0) {
			if ((rs = proclog_begin(pip)) >= 0) {
	                    cchar	*ofn = ofname ;
	                    cchar	*ifn = ifname ;
	                    if ((rs = lightnoise(pip,&u,ofn)) >= 0) {
	                        if ((rs = process(pip,ofn,ifn)) > 0) {
	                            if (pip->debuglevel > 0) {
	                                fmt = "%s: process (%d)\n" ;
	                                bprintf(pip->efp,fmt,pn,rs) ;
	                            }
	                        } /* end if (process) */
	                    } /* end if (lightnoise) */
			    rs1 = proclog_end(pip) ;
			    if (rs >= 0) rs = rs1 ;
			} /* end if (proclog) */
	                rs1 = proglog_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (proglog) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* argument errors */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<file>] \n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s|-u] [-of <ofile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-names */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	v ;
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {
	                case akoname_audit:
	                    if (! lip->final.audit) {
	                        lip->have.audit = TRUE ;
	                        lip->final.audit = TRUE ;
	                        lip->f.audit = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.audit = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_logsize:
	                    if (! pip->final.logsize) {
	                        pip->have.logsize = TRUE ;
	                        pip->final.logsize = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecmfi(vp,vl,&v) ;
	                            pip->logsize = v ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } else
			rs = SR_INVALID ;

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        rs1 = keyopt_curend(kop,&kcur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(PROGINFO *pip,cchar *ofn,cchar *ifn)
{
	LOCINFO		*lip = pip->lip ;
	const uint	hv = pip->hv ;
	RANDOMVAR	rv ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if ((rs = randomvar_start(&rv,FALSE,hv)) >= 0) {
	    if ((rs = procout_begin(pip,ofn)) >= 0) {
	        const int	ofd = rs ;
	        if ((rs = procinput_begin(pip,ifn)) >= 0) {
	            ECMSG	*emp = &lip->extra ;
	            FILEINFO	fi ; /* result data */
	            const int	ifd = rs ;
	            if (pip->f.unscramble) {
	                rs = decode(pip,&rv,&fi,emp,ifd,ofd) ;
	            } else {
	                if (pip->msgfname != NULL) {
	                    cchar	*mfn = pip->msgfname ;
	                    rs = locinfo_loadfile(lip,mfn) ;
	                }
	                if (rs >= 0) {
	                    rs = encode(pip,&rv,&fi,emp,ifd,ofd) ;
	                }
	            } /* end if (encode or decode) */
	            if (rs < 0) {
	                procereport(pip,&fi,rs) ;
	            } else {
	                fmt = "mode=%c cksum=\\x%08x (%u)" ;
	                logfile_printf(&pip->lh,fmt,
	                    ((pip->f.unscramble) ? 'u' : 's'),
	                    fi.cksum,fi.cksum) ;
	            }
	            rs1 = procinput_end(pip,ifd) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procinput) */
	        rs1 = procout_end(pip,ofd) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        fmt = "%s: inaccessible output (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	        bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	    } /* end if (procout) */
	    rs1 = randomvar_finish(&rv) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (randomvar) */

	return rs ;
}
/* end subroutine (process) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int proclog_begin(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	if (lip->jobid != NULL) {
	    proglog_printf(pip,"jobid=%s",lip->jobid) ;
	}
	return rs ;
}
/* end subroutine (proclog_begin) */


static int proclog_end(PROGINFO *pip)
{
	if (pip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (proclog_end) */


static int procout_begin(PROGINFO *pip,cchar *ofn)
{
	int		rs = SR_OK ;
	int		ofd = FD_STDOUT ;
	if (pip == NULL) return SR_FAULT ;
	if ((ofn != NULL) && (ofn[0] != '\0')) {
	    const int	of = (O_CREAT | O_TRUNC | O_WRONLY) ;
	    rs = u_open(ofn,of,0666) ;
	    ofd = rs ;
	}
	return (rs >= 0) ? ofd : rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip,int ofd)
{
	int		rs = SR_OK ;
	if (pip->f.outfile && (ofd >= 0)) {
	    pip->f.outfile = FALSE ;
	    rs = u_close(ofd) ;
	}
	return rs ;
}
/* end subroutine (procinput_end) */


static int procinput_begin(PROGINFO *pip,cchar *ifn)
{
	int		rs = SR_OK ;
	int		ifd = FD_STDIN ;
	if ((ifn != NULL) && (ifn[0] != '\0') && (ifn[0] != '-')) {
	    const int	of = O_RDONLY ;
	    if ((rs = uc_open(ifn,of,0666)) >= 0) {
	        pip->f.infile = TRUE ;
	        ifd = rs ;
	    }
	}
	return (rs >= 0) ? ifd : rs ;
}
/* end subroutine (procinput_begin) */


static int procinput_end(PROGINFO *pip,int ifd)
{
	int		rs = SR_OK ;
	if (pip->f.infile && (ifd >= 0)) {
	    pip->f.infile = FALSE ;
	    rs = u_close(ifd) ;
	}
	return rs ;
}
/* end subroutine (procinput_end) */


static int procereport(PROGINFO *pip,FILEINFO *fip,int prs)
{
	int		rs = SR_OK ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	proglog_printf(pip,"mode=%c",
	    ((pip->f.unscramble) ? 'd' : 'e')) ;

	fmt = "sent cksum=\\x%08x (%u)" ;
	proglog_printf(pip,fmt,fip->cksum,fip->cksum) ;

	proglog_printf(pip,"failed (%d)",prs) ;

	if ((pip->debuglevel > 0) || (! pip->f.quiet)) {
	    fmt = "%s: failed (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,prs) ;
	}

	return rs ;
}
/* end subroutine (procereport) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	if ((rs = ecmsg_start(&lip->extra)) >= 0) {
	    lip->open.extra = TRUE ;
	}

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.extra) {
	    rs1 = ecmsg_finish(&lip->extra) ;
	    if (rs >= 0) rs = rs1 ;
	}

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
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    rs = vecstr_start(slp,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
	        oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


static int locinfo_loadmsg(LOCINFO *lip,cchar *mp,int ml)
{
	int		rs = SR_OK ;
	if (lip->open.extra) {
	    ECMSG	*emp = &lip->extra ;
	    rs = ecmsg_loadbuf(emp,mp,ml) ;
	}
	return rs ;
}
/* end subroutine (locinfo_loadmsg) */


static int locinfo_loadfile(LOCINFO *lip,cchar *mfn)
{
	ECMSG		*emp = &lip->extra ;
	int		rs ;
	if ((rs = ecmsg_already(emp)) == 0) {
	    rs = ecmsg_loadfile(emp,mfn) ;
	}
	return rs ;
}
/* end subroutine (locinfo_loadfile) */


