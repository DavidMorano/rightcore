/* b_cal */

/* generic (more of less) front-end subroutine */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	The program was written from scratch to do what the previous program by
	the same name did.  This program (command) takes arguments in a variety
	of weirdo ways (much more so than the original UNIX® CAL(1) program
	did) so watch out for the mess.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is really not generic now that I have modified it!  This
	subroutine is now most of this program except for the fact that we
	'spawn(3dam)' the UNIX® '/usr/bin/cal' program!

	Implementation notes:

	Watch out for how those arguments work and interact with each other.
	They are a bitch.  This command should be compatible with the arguments
	that can be given to UNIX® CAL(1) so a good bit of care went into how
	arguments can be specified and interpreted (so as not to break
	compatibility).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tzfile.h>		/* for TM_YEAR_BASE */

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<spawnproc.h>
#include	<linefold.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_cal.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matcasestr(cchar **,cchar *,int) ;
extern int	matocasestr(cchar **,int,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	ctdeci(char *,int,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct calarger {
	cchar		*progcal ;
	cchar		*specyear ;
	cchar		*specmonth ;
	cchar		*pos1 ;
	cchar		*pos2 ;
	int		defmonth ;
	int		defyear ;
	int		month ;
	int		year ;
	int		type ;
	int		cols ;
	int		n ;
} ;

struct monyear {
	short		y, m ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procarger(PROGINFO *,struct calarger *) ;
static int	process(PROGINFO *,struct calarger *,cchar *,int) ;
static int	procexec(PROGINFO *,struct calarger *,int) ;
static int	procerrout(PROGINFO *,int,cchar *,int) ;
static int	procline(PROGINFO *,int,cchar *,int) ;

static int	getstuff(PROGINFO *,struct monyear *,cchar *) ;

static int	getdefyear(struct calarger *) ;

static int	whichmonth(cchar *,int) ;
static int	getval(cchar *,int) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"sn",
	"af",
	"ef",
	"of",
	"cp",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_option,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_cp,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const MAPEX	mapexs[] = {
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

static const char	*months[] = {
	"january", "february", "march", "april", "may", "june", 
	"july", "august", "september", "october", "november", "december",
	NULL
} ;

static const char	blanks[] = "        " ;


/* exported subroutines */


int b_cal(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    cchar	**envv = (cchar **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_cal) */


int p_cal(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_cal) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	struct calarger	ca ;
	BITS		pargs ;
	KEYOPT		akopts ;
	SHIO		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		cols = -1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*specyear = NULL ;
	cchar		*specmonth = NULL ;
	cchar		*progcal = NULL ;
	cchar		*pos1 = NULL ;
	cchar		*pos2 = NULL ;
	cchar		*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_cal: starting DFD=%d\n",rs) ;
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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	pip->verboselevel = 1 ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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

	            argval = (argp+1) ;

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
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* version */
	                case argopt_version:
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

	                case argopt_help:
	                    f_help = TRUE ;
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

/* the user specified some options */
	                case argopt_option:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
	                    } else
	                        rs = SR_INVALID ;
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

/* argument-list file name */
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
	                            if (argl)
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

/* error file name */
	                case argopt_cp:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            progcal = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                progcal = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                specmonth = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

/* case year */
	                    case 'y':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                specyear = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* help */
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

	        } /* end if */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter-word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_cal: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,
	        "%s: verboselevel=%u\n",pip->progname,pip->verboselevel) ;
	    bcontrol(pip->efp,BC_LINEBUF,0) ;
	    bflush(pip->efp) ;
	}

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cal: PRE sn=%s\n",sn) ;
#endif

/* find our program-root */

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
	    debugprintf("b_cal: pr=%s\n",pip->pr) ;
	    debugprintf("b_cal: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* user specified help only */

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	} /* end if */

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get the calendar program ('CAL(1)') file-path */

	{

	    if ((progcal == NULL) || (progcal[0] == '\0'))
	        progcal = getourenv(envv,VARPROGCAL) ;
	    if ((progcal == NULL) || (progcal[0] == '\0'))
	        progcal = PROG_CAL ;

	    if (pip->debuglevel > 0)
	        shio_printf(pip->efp,"%s: progcal=%s\n",pip->progname,progcal) ;

	} /* end block (calendar program) */

/* output columns (line-width) */

	{
	    if ((cp = getourenv(envv,VARCOLUMNS)) != NULL) {
	        rs = cfdeci(cp,-1,&cols) ;
	    }
	    if (cols < 0) cols = COLUMNS ;
	} /* end block (columns) */

/* OK, do the figuring out type work */

	if (rs >= 0) {
	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            switch (pan) {
	            case 0:
	                pos1 = cp ;
	                break ;
	            case 1:
	                pos2 = cp ;
	                break ;
	            default:
	                rs = SR_INVALID ;
	                break ;
	            } /* end switch */
	            pan += 1 ;
	        } /* end if */

	    } /* end for (looping through positional arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&argvalue) ;
	}
	if ((rs >= 0) && (argvalue < 1)) {
	    argvalue = 1 ;
	}

/* crunch on the arguments (if any), and then do it */

	memset(&ca,0,sizeof(struct calarger)) ;
	ca.cols = cols ;
	ca.progcal = progcal ;
	ca.specyear = specyear ;
	ca.specmonth = specmonth ;
	ca.pos1 = pos1 ;
	ca.pos2 = pos2 ;

	if (rs >= 0) {
	    if ((rs = procarger(pip,&ca)) >= 0) {
	        rs = process(pip,&ca,ofname,argvalue) ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} else if ((rs >= 0) && (ex == EX_OK)) {
	    if ((rs = lib_sigterm()) < 0) {
	        ex = EX_TERM ;
	    } else if ((rs = lib_sigintr()) < 0) {
	        ex = EX_INTR ;
	    }
	} /* end if */

/* we are out of here */
retearly:
	if ((pip->debuglevel > 0) && (pip->efp != NULL)) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n", 
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_cal: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	CF_DEBUGS || CF_DEBUG
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s { [<am>] | [[<am>] <y>] | [<y>/<am>] } [-<n>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procarger(PROGINFO *pip,struct calarger *cap)
{
	struct monyear	my, *myp = &my ;
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("b_cal: specyear=%s\n",cap->specyear) ;
	    debugprintf("b_cal: specmonth=%s\n",cap->specmonth) ;
	    debugprintf("b_cal: p1=%s\n",cap->pos1) ;
	    debugprintf("b_cal: p2=%s\n",cap->pos2) ;
	}
#endif

/* grind */

	if ((rs >= 0) && (cap->pos1 != NULL)) {

	    cap->type = 1 ;
	    if ((rs = getstuff(pip,myp,cap->pos1)) >= 0) {

	        if (cap->pos2 != NULL) {

	            cap->month = (myp->m >= 0) ? myp->m : myp->y ;
	            cap->type = 2 ;
	            rs = getstuff(pip,myp,cap->pos2) ;
	            cap->year = myp->y ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("b_cal: two pos rs=%d t=%u y=%d m=%d\n",
	                    rs,cap->type,cap->year,cap->month) ;
#endif

	        } else {

	            if (myp->y >= 0) cap->year = myp->y ;

	            if (myp->m >= 0) {
	                cap->type = 2 ;
	                cap->month = myp->m ;
	                if (myp->y < 0) {
	                    rs = getdefyear(cap) ;
	                    cap->year = cap->defyear ;
	                }
	            }

	        } /* end if */

	    } /* end if */

	} /* end if (positions) */

	if ((rs >= 0) && (cap->pos1 == NULL) && (cap->pos2 == NULL)) {
	    int	n ;

	    rs = getdefyear(cap) ;
	    n = rs ;
	    if (rs >= 0) {
	        cap->year = cap->defyear ;
	        cap->month = cap->defmonth ;
	    }
	    if (rs > 0) {
	        switch (n) {
	        case 0:
	            break ;
	        case 1:
	            cap->type = 1 ;
	            break ;
	        case 2:
	        case 3:
	            cap->type = 2 ;
	            break ;
	        } /* end switch */
	    } /* end if */

	} /* end if (no arguments -- use any defaults) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cal/procarger: y=%d m=%d\n",
	        cap->year,cap->month) ;
#endif

	if ((rs >= 0) && (cap->month > 0)) {
	    if (cap->month > 12) rs = SR_INVALID ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cal/procarger: ret rs=%d type=%u\n",rs,cap->type) ;
#endif

	return rs ;
}
/* end subroutine (procarger) */


static int process(PROGINFO *pip,struct calarger *cap,cchar *ofn,int ncals)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_cal/process: ofn=%s\n",ofn) ;
	    debugprintf("b_cal/process: ncals=%u\n",ncals) ;
	}
#endif

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {

	    if ((rs = opentmp(NULL,O_RDWR,0664)) >= 0) {
	        int	ofd = rs ;
	        int	i ;

	        for (i = 0 ; i < ncals ; i += 1) {

	            rs = procexec(pip,cap,ofd) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_cal/process: procexec() rs=%d\n",rs) ;
#endif

	            if (rs < 0) break ;

	            if (cap->type == 1) break ;

	            cap->month += 1 ;
	            cap->type = 2 ;
	            if (cap->month > 12) {
	                cap->month = 1 ;
	                cap->year += 1 ;
	            }

	        } /* end for (looping through calendars) */

	        if (rs >= 0) {
	            const int	llen = LINEBUFLEN ;
	            char	lbuf[LINEBUFLEN+1] ;
	            if ((rs = u_rewind(ofd)) >= 0) {
	                while ((rs = u_read(ofd,lbuf,llen)) > 0) {
	                    rs = shio_write(ofp,lbuf,rs) ;
	                    wlen += rs ;
	                    if (rs < 0) break ;
	                } /* end while */
	            } /* end if (rewind) */
	        } /* end if */

	        u_close(ofd) ;
	    } /* end if (opentmp) */

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    fmt = "%s: ofn=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,ofn) ;
	} /* end if (output) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_cal/process: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procexec(PROGINFO *pip,struct calarger *cap,int ofd)
{
	SPAWNPROC	ps ;
	pid_t		pid ;
	int		rs = SR_OK ;
	int		cs ;			/* child status */
	int		cai = 0 ;
	cchar		*progfname = cap->progcal ;
	cchar		*calargs[5] ;
	char		bufyear[DIGBUFLEN+1] ;
	char		bufmonth[DIGBUFLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_cal/procexec: progfname=%s\n",progfname) ;
#endif

	if (rs >= 0)
	    rs = ctdeci(bufyear,DIGBUFLEN,cap->year) ;

	if (rs >= 0)
	    rs = ctdeci(bufmonth,DIGBUFLEN,cap->month) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("b_cal/procexec: ctdec rs=%d\n",rs) ;
	    debugprintf("b_cal/procexec: type=%u\n",cap->type) ;
	}
#endif

	if (rs >= 0) {

	    calargs[cai++] = PROG_CALNAME ;
	    switch (cap->type) {
	    case 0:
	        break ;
	    case 1:
	        calargs[cai++] = bufyear ;
	        break ;
	    case 2:
	        calargs[cai++] = bufmonth ;
	        calargs[cai++] = bufyear ;
	        break ;
	    default:
	        rs = SR_NOANODE ;
	        break ;
	    } /* end switch */
	    calargs[cai] = NULL ;

	    if (rs >= 0) {

/* pop it */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            int	i ;
	            for (i = 0 ; calargs[i] != NULL ; i += 1)
	                debugprintf("b_cal: carg[%u]=%s\n",i,calargs[i]) ;
	        }
#endif /* CF_DEBUG */

	        if ((rs = opentmp(NULL,O_RDWR,0664)) >= 0) {
	            int		efd = rs ;
	            cchar	*es = "standard error" ;

	            memset(&ps,0,sizeof(SPAWNPROC)) ;
	            ps.disp[0] = SPAWNPROC_DCLOSE ;
	            ps.disp[1] = SPAWNPROC_DDUP ;
	            ps.disp[2] = SPAWNPROC_DDUP ;
	            ps.fd[0] = 0 ;
	            ps.fd[1] = ofd ;
	            ps.fd[2] = efd ;

	            rs = spawnproc(&ps,progfname,calargs,pip->envv) ;
	            pid = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("b_cal/procexec: spawnproc() rs=%d\n",rs) ;
#endif

	            if (rs >= 0) {
	                rs = u_waitpid(pid,&cs,0) ;
	            }

	            if (rs >= 0) {
	                rs = procerrout(pip,cap->cols,es,efd) ;
	            }

	            u_close(efd) ;
	        } /* end if (opentmp) */

	    } /* end if (ok) */

	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_cal/procexec: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procexec) */


static int procerrout(PROGINFO *pip,int cols,cchar s[],int ofd)
{
	FILEBUF		b ;
	int		rs = SR_OK ;
	int		rs1 = SR_OK ;
	int		wlen = 0 ;
	int		f_title = FALSE ;
	cchar		*pn = pip->progname ;

	if (pip->efp != NULL) {
	    if ((rs = filebuf_start(&b,ofd,0L,0,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        rs1 = SR_OK ;
	        while ((rs = filebuf_readline(&b,lbuf,llen,-1)) > 0) {
	            int	len = rs ;

	            if (lbuf[len - 1] == '\n') lbuf[--len] = '\0' ;

	            if ((! f_title) && (len > 0)) {
	                if ((s != NULL) && (s[0] != '\0')) {
	                    f_title = TRUE ;
	                    rs1 = shio_printf(pip->efp,"%s: %s>\n",pn,s) ;
	                    wlen += rs1 ;
	                }
	            }

	            if (rs1 >= 0) {
	                rs1 = procline(pip,cols,lbuf,len) ;
	                wlen += rs1 ;
	            }

	            if (rs1 < 0) break ;
	        } /* end while (reading lines) */
	        if (rs1 < 0) wlen = 0 ;

	        filebuf_finish(&b) ;
	    } /* end if (filebuf) */
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procerrout) */


static int procline(PROGINFO *pip,int cols,cchar lp[],int ll)
{
	SHIO		*fp = pip->efp ;
	const int	indent = 2 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		leadlen ;
	int		textlen ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;

	leadlen = (strlen(pn) + 4) ;
	textlen = (cols - leadlen) ;
	if (textlen >= 1) {
	    LINEFOLD	lf ;
	    if ((rs = linefold_start(&lf,textlen,indent,lp,ll)) >= 0) {
	        int	i ;
	        int	ind = 0 ;
	        int	cl ;
	        cchar	*cp ;

	        for (i = 0 ; (cl = linefold_get(&lf,i,&cp)) >= 0 ; i += 1) {
	            rs1 = shio_printf(fp,"%s: | %t%t\n",pn,blanks,ind,cp,cl) ;
	            wlen += rs1 ;
	            if (rs1 < 0) break ;
	            ind = indent ;
	        } /* end for */

	        linefold_finish(&lf) ;
	    } /* end if (linefold) */
	} /* end if (text-len) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


/* get the month and-or year out of a string argument */
static int getstuff(PROGINFO *pip,struct monyear *myp,cchar s[])
{
	int		rs = SR_OK ;
	int		v ;
	cchar		*tp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cal/getstuff: s=>%s<\n",s) ;
#endif

	if (pip == NULL) return SR_FAULT ; /* lint */
	myp->m = -1 ;
	myp->y = -1 ;
	if ((tp = strpbrk(s,"/-")) != NULL) {

	    if (tp[0] == '-') {

	        v = whichmonth(s,(tp-s)) ;
	        if (v >= 0) {
	            myp->m = v ;
	            rs = getval((tp+1),-1) ;
	            myp->y = rs ;
	        } else {
	            rs = getval(s,(tp-s)) ;
	            myp->y = rs ;
	            if (rs >= 0) {
	                v = whichmonth((tp+1),-1) ;
	                if (v >= 0) {
	                    myp->m = v ;
	                } else {
	                    rs = getval((tp+1),-1) ;
	                    myp->m = rs ;
	                }
	            }
	        }

	    } else {

	        rs = getval(s,(tp-s)) ;
	        myp->y = rs ;
	        if (rs >= 0) {
	            v = whichmonth((tp+1),-1) ;
	            if (v >= 0) {
	                myp->m = v ;
	            } else {
	                rs = getval((tp+1),-1) ;
	                myp->m = v ;
	            }
	        }

	    } /* end if */

	} else {

	    if ((v = whichmonth(s,-1)) >= 0) {
	        myp->m = v ;
	    } else {
	        rs = getval(s,-1) ;
	        myp->y = rs ;
	    }

	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_cal/getstuff: ret rs=%d y=%d m=%d\n",
	        rs,myp->y,myp->m) ;
#endif

	return rs ;
}
/* end subroutine (getstuff) */


static int getdefyear(struct calarger *cap)
{
	struct tm	ts ;
	const time_t	daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		v ;
	int		n = 0 ;

	if ((cap->defyear == 0) && (cap->defmonth == 0)) {

	    if ((rs = uc_localtime(&daytime,&ts)) >= 0) {
	        cap->defyear = (ts.tm_year + TM_YEAR_BASE) ;
	        cap->defmonth = (ts.tm_mon + 1) ;
	    }

	    if ((rs >= 0) && (cap->specyear != NULL)) {
	        rs = cfdeci(cap->specyear,-1,&v) ;
	        cap->defyear = v ;
	        n |= 0x01 ;
	    }

	    if ((rs >= 0) && (cap->specmonth != NULL)) {
	        v = whichmonth(cap->specmonth,-1) ;
	        if (v < 0)
	            rs = cfdeci(cap->specmonth,-1,&v) ;
	        cap->defmonth = v ;
	        n |= 0x02 ;
	    }

	    cap->n = n ;

	} else
	    n = cap->n ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getdefyear) */


static int whichmonth(cchar sp[],int sl)
{
	int		i = -1 ;
	int		cl ;
	cchar		*cp ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    i = matocasestr(months,2,cp,cl) ;
	}

	return ((i >= 0) ? (i + 1) : -1) ;
}
/* end subroutine (whichmonth) */


static int getval(cchar *sp,int sl)
{
	int		rs = SR_INVALID ;
	int		cl ;
	int		v = 0 ;
	cchar		*cp ;

	if ((cl = sfshrink(sp,sl,&cp)) > 0) {
	    rs = cfdeci(cp,cl,&v) ;
	}

	return (rs >= 0) ? v : rs ;
}
/* end subroutine (getval) */


