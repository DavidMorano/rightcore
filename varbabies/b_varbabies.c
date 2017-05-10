/* b_varbabies */

/* SHELL built-in to for specialized DB management */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ varbabies <date(s)>


*****************************************************************************/


#include	<envstandards.h>	/* must be first to configure */

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
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"varbabies_config.h"
#include	"defs.h"
#include	"cvtdater.h"
#include	"babycalc.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	CVTBUFLEN	100

#ifndef	VARRANDOM
#define	VARRANDOM	"RANDOM"
#endif


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		cvtdater : 1 ;
	uint		babycalc : 1 ;
	uint		init : 1 ;
	uint		nocache : 1 ;
	uint		quiet : 1 ;
	uint		intref : 1 ;
} ;

struct locinfo {
	const char	*dbname ;
	struct locinfo_flags	have, f, changed, final, open ;
	struct proginfo	*pip ;
	CVTDATER	cvt ;
	BABYCALC	bc ;
	int		intref ;
} ;


/* forward references */

static void	sighand_int(int) ;

static int	usage(struct proginfo *) ;

static int	procopts(struct proginfo *,KEYOPT *) ;
static int	procinfo(struct proginfo *,void *) ;
static int	procspec(struct proginfo *,void *, const char *) ;

static int	locinfo_start(struct locinfo *,struct proginfo *) ;
static int	locinfo_dbname(struct locinfo *,const char *) ;
static int	locinfo_flags(struct locinfo *,int,int) ;
static int	locinfo_lookinfo(struct locinfo *,BABYCALC_INFO *) ;
static int	locinfo_lookup(struct locinfo *,const char *,int,uint *) ;
static int	locinfo_finish(struct locinfo *) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

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
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	SIGQUIT,
	0
} ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"af",
	"of",
	"db",
	"nocache",
	"info",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_af,
	argopt_of,
	argopt_db,
	argopt_nocache,
	argopt_info,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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

static const char *akonames[] = {
	"quiet",
	"refint",
	NULL
} ;

enum akonames {
	akoname_quiet,
	akoname_intref,
	akoname_overlast
} ;

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* persistent local variables (special class of local variables) */


/* exported subroutines */


int b_varbabies(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	struct proginfo	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;

	SIGMAN	sm ;

	SHIO	errfile ;
	SHIO	outfile, *ofp = &outfile ;

	KEYOPT	akopts ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	n, size, len ;
	int	i, j ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_init = FALSE ;
	int	f_nocache = FALSE ;
	int	f_info = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*dbname = NULL ;
	const char	*cp ;


	if (contextp != NULL) lib_initenviron() ;

	if_exit = 0 ;
	if_int = 0 ;

	rs = sigman_start(&sm,argv[0],
		sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("b_varbabies: starting\n") ;
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,environ,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) == NULL)
	    cp = STDERRFNAME ;

	rs1 = shio_open(&errfile,cp,"wca",0666) ;

	if (rs1 >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CLINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

	pip->daytime = time(NULL) ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* start parsing the arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

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

	        if (isdigit(argp[1])) {

	            argval = (argp + 1) ;

	        } else if (argp[1] == '-') {

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

/* keyword match or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                case argopt_root:
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
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                    }

	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* argument file */
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

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            ofname = argp ;

	                    }

	                    break ;

/* DB file */
	                case argopt_db:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            dbname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            dbname = argp ;

	                    }

	                    break ;

	                case argopt_nocache:
			    f_nocache = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
				    f_nocache = (rs > 0) ;
				}

			    }

			    break ;

	                case argopt_info:
			    f_info = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
				    f_info = (rs > 0) ;
				}

			    }

			    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
			    int kc = (*akp & 0xff) ;

	                    switch (kc) {

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* special initialization for persistent cache */
			    case 'i':
				f_init = TRUE ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

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
	                    if (rs < 0)
	                        break ;

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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,NULL) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_varbabies: f_help=%u\n",f_help) ;
#endif

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	rs = procopts(pip,&akopts) ;
	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* initialization */

	if (dbname == NULL)
	    dbname = getenv(VARDBNAME) ;

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: dbname=%s\n",
		pip->progname,dbname) ;

	locinfo_dbname(lip,dbname) ;

	locinfo_flags(lip,f_init,f_nocache) ;

/* OK, we finally do our thing */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	rs = shio_open(ofp,ofname,"wct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

/* asked for information? */

	if (f_info) {
	    rs = procinfo(pip,ofp) ;
	}

/* go through the loops */

	pan = 0 ;

	if (rs >= 0) {
	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procspec(pip,ofp,cp) ;
	    if (rs < 0)
	        break ;

	    if (if_int || if_exit)
	        break ;

	} /* end for (handling positional arguments) */
	} /* end if */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    SHIO	argfile, *afp = &argfile ;


	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

	    rs = shio_open(afp,afname,"r",0666) ;

	    if (rs >= 0) {

	        FIELD	fsb ;

	        int	ml ;
	        int	fl ;

	        const char	*fp ;

	        char	linebuf[LINEBUFLEN + 1] ;
	        char	name[MAXNAMELEN + 1] ;


	        while ((rs = shio_readline(afp,linebuf,LINEBUFLEN)) > 0) {
	            len = rs ;

	            if (linebuf[len - 1] == '\n') len -= 1 ;
	            linebuf[len] = '\0' ;

	            if ((rs = field_start(&fsb,linebuf,len)) >= 0) {

	                while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	                    if (fl == 0) continue ;

	                    ml = MIN(fl,MAXNAMELEN) ;
	                    strwcpy(name,fp,ml) ;

	                    pan += 1 ;
	                    rs = procspec(pip,ofp,name) ;
	                    if (rs < 0)
	                        break ;

	                    if (fsb.term == '#')
	                        break ;

	                    if (if_int || if_exit)
	                        break ;

	                } /* end while */

	                field_finish(&fsb) ;
	            } /* end if (field) */

	            if (if_int || if_exit)
	                break ;

	        } /* end while (reading lines) */

	        shio_close(afp) ;
	    } else {

	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,
	                "%s: inaccessible argument list file (%d)\n",
	                pip->progname,rs) ;
	            shio_printf(pip->efp,"%s: argfile=%s\n",
	                pip->progname,afname) ;
	        }

	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0) && (! f_info)) {

	    cp = "" ;
	    pan += 1 ;
	    rs = procspec(pip,ofp,cp) ;

	} /* end if */

	shio_close(ofp) ;

/* finish */
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {

	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet)
	            shio_printf(pip->efp,"%s: invalid query (%d)\n",
	                pip->progname,rs) ;
	        break ;

	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;

	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;

	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;

	    } /* end switch */
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

/* early return thing */
badoutopen:
retearly:
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_varbabies: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	locinfo_finish(lip) ;

badlocstart:
	shio_close(pip->efp) ;

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sigman_finish(&sm) ;

ret0:
	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{


	if_int = TRUE ;
	if_exit = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = shio_printf(pip->efp,
	    "%s: USAGE> %s [<date(s)> ...] [-af <argfile>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:  [-db <dbname>] [-Q] [-D] [-v[=n]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->lip ;

	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	oi ;
	int	kl, vl ;
	int	c = 0 ;

	const char	*kp, *vp ;
	const char	*cp ;


	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs < 0)
	    goto ret0 ;

/* process program options */

	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

/* get the first value for this key */

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

/* do we support this option? */

	    if ((oi = matostr(akonames,2,kp,kl)) >= 0) {
	        uint	uv ;

	        switch (oi) {

	        case akoname_quiet:
	            if (! pip->final.quiet) {
	                pip->have.quiet = TRUE ;
	                pip->final.quiet = TRUE ;
	                pip->f.quiet = TRUE ;
	                if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                    pip->f.quiet = (uv > 0) ? 1 : 0 ;
	            }
	            break ;

	        case akoname_intref:
	            if (! lip->final.intref) {
	                lip->have.intref = TRUE ;
	                lip->final.intref = TRUE ;
	                lip->f.intref = TRUE ;
	                if (vl > 0) {
			    rs = cfdecui(vp,vl,&uv) ;
	                    lip->intref = uv ;
			}
	            }
	            break ;

	        } /* end switch */

	        c += 1 ;

	        } /* end if (valid option) */

		if (rs < 0) break ;

	    } /* end while (looping through key options) */

	    keyopt_curend(kop,&kcur) ;
	} /* end if (key-options) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


/* process an information request */
static int procinfo(pip,ofp)
struct proginfo	*pip ;
void		*ofp ;
{
	struct locinfo	*lip = pip->lip ;

	BABYCALC_INFO	bi ;

	time_t	ti ;
	time_t	t = pip->daytime ;

	int	rs = SR_OK ;
	int	wlen = 0 ;

	char	timebuf[TIMEBUFLEN + 1] ;


	if (ofp == NULL)
	    goto ret0 ;

	rs = locinfo_lookinfo(lip,&bi) ;

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    if (rs >= 0) {
	        ti = bi.wtime ;
	        t = (t & (~ UINT_MAX)) | (((time_t) ti) & UINT_MAX) ;
	        rs = shio_printf(ofp,"wtime=%s\n",timestr_log(t,timebuf)) ;
	        wlen += rs ;
	    }
	    if (rs >= 0) {
	        ti = bi.atime ;
	        t = (t & (~ UINT_MAX)) | (((time_t) ti) & UINT_MAX) ;
	        rs = shio_printf(ofp,"atime=%s\n",timestr_log(t,timebuf)) ;
	        wlen += rs ;
	    }
	    if (rs >= 0) {
	        rs = shio_printf(ofp,"acount=%u\n",bi.acount) ;
	        wlen += rs ;
	    }
	} /* end if */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procinfo) */


/* process a specification name */
static int procspec(pip,ofp,name)
struct proginfo	*pip ;
void		*ofp ;
const char	name[] ;
{
	struct locinfo	*lip = pip->lip ;

	uint	count = 0 ;

	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (name == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies/procspec: name=%s\n",name) ;
#endif

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: reqdate=%s\n",pip->progname,name) ;

	rs = locinfo_lookup(lip,name,-1,&count) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies/procspec: locinfo_lookup() rs=%d c=%u\n",
		rs,count) ;
#endif

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	    rs = shio_printf(ofp,"%u\n",count) ;
	    wlen += rs ;
	} /* end if */

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: c=%u (%d)\n",
		pip->progname,count,rs) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies/procspec: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int locinfo_start(lip,pip)
struct locinfo	*lip ;
struct proginfo	*pip ;
{


	if (lip == NULL)
	    return SR_FAULT ;

	memset(lip,0,sizeof(struct locinfo)) ;

	lip->pip = pip ;
	return 0 ;
}
/* end subroutine (locinfo_start) */


static int locinfo_dbname(lip,dbname)
struct locinfo	*lip ;
const char	*dbname ;
{


	if (lip == NULL)
	    return SR_FAULT ;

	lip->dbname = (char *) dbname ;
	return 0 ;
}
/* end subroutine (locinfo_dbname) */


static int locinfo_flags(lip,f_init,f_nocache)
struct locinfo	*lip ;
int		f_init ;
int		f_nocache ;
{


	if (lip == NULL)
	    return SR_FAULT ;

	lip->f.init = f_init ;
	lip->f.nocache = f_nocache ;
	return 0 ;
}
/* end subroutine (locinfo_flags) */


static int locinfo_lookinfo(lip,bip)
struct locinfo	*lip ;
BABYCALC_INFO	*bip ;
{
	struct proginfo	*pip ;

	int	rs = SR_OK ;


	if (lip == NULL)
	    return SR_FAULT ;

	if (bip == NULL)
	    return SR_FAULT ;

	pip = lip->pip ;
	if (! lip->f.babycalc) {
	    lip->f.babycalc = TRUE ;
	    rs = babycalc_open(&lip->bc,pip->pr,lip->dbname) ;
	    lip->open.babycalc = (rs >= 0) ;
	    if (pip->debuglevel > 0)
		shio_printf(pip->efp,"%s: DB-open (%d)\n",
		    pip->progname,rs) ;
	} /* end if */

	if (rs >= 0)
	    rs = babycalc_info(&lip->bc,bip) ;

	return rs ;
}
/* end subroutine (locinfo_info) */


static int locinfo_lookup(lip,dbuf,dlen,rp)
struct locinfo	*lip ;
const char	dbuf[] ;
int		dlen ;
uint		*rp ;
{
	struct proginfo	*pip ;

	time_t	rd = 0 ;

	int	rs = SR_OK ;


#if	CF_DEBUGS
	    debugprintf("b_varbabies/locinfo_lookup: enter\n") ;
#endif

	if (lip == NULL)
	    return SR_FAULT ;

	if (dbuf == NULL)
	    return SR_FAULT ;

	if (dlen < 0)
	    dlen = strlen(dbuf) ;

	pip = lip->pip ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies/locinfo_lookup: name=%t\n",
		dbuf,strlinelen(dbuf,dlen,50)) ;
#endif

	if (! lip->f.babycalc) {
	    lip->f.babycalc = TRUE ;
	    rs = babycalc_open(&lip->bc,pip->pr,lip->dbname) ;
	    lip->open.babycalc = (rs >= 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies/locinfo_lookup: babycalc_open() "
			"rs=%d\n",rs) ;
#endif

	    if (pip->debuglevel > 0)
		shio_printf(pip->efp,"%s: DB-open (%d)\n",
		    pip->progname,rs) ;

	} /* end if */

	if ((rs >= 0) && (! lip->f.cvtdater)) {
	    lip->f.cvtdater = TRUE ;
	    rs = cvtdater_start(&lip->cvt,pip->daytime) ;
	    lip->open.cvtdater = (rs >= 0) ;
	}

	rd = pip->daytime ;
	if ((rs >= 0) && (dlen > 0) && (dbuf[0] != '-')) {

	    rs = cvtdater_load(&lip->cvt,&rd,dbuf,dlen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies/locinfo_lookup: cvtdater_load() rs=%d\n",
		rs) ;
#endif

	}

	if (rs >= 0) {

	    rs = babycalc_lookup(&lip->bc,rd,rp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_varbabies/locinfo_lookup: babycalc_lookup() "
		"rs=%d\n",rs) ;
#endif

	} /* end if */

	if ((rs < 0) && (rp != NULL))
	    *rp = 0 ;

ret0:
	return rs ;
}
/* end subroutine (locinfo_lookup) */


static int locinfo_finish(lip)
struct locinfo	*lip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL)
	    return SR_FAULT ;

	if (lip->open.cvtdater) {
	    lip->open.cvtdater = FALSE ;
	    rs1 = cvtdater_finish(&lip->cvt) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.babycalc) {
	    lip->open.babycalc = FALSE ;
	    rs1 = babycalc_close(&lip->bc) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */



