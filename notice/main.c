/* main (NOTICE) */

/* program to send a notice to a user's terminal */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGN	1		/* run-time debugging */
#define	CF_LOCSET	0		/* |locinfo_setentry()| */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine forms the front-end of a program that sends a notice to
	a user's terminal.  It is similar to 'write(1)' but more general.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<vecint.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<ascii.h>
#include	<sbuf.h>
#include	<tmpx.h>
#include	<sesnotes.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#undef	BUFLEN
#define	BUFLEN		((10 * 1024) + MAXHOSTNAMELEN)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2028
#endif

#ifndef	DEVDNAME
#define	DEVDNAME	"/dev"
#endif

#define	O_TERM		(O_WRONLY | O_NOCTTY | O_NDELAY)

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	dialticotsordnls(const char *,int,const char *,int,int) ;
extern int	getnodename(char *,int) ;
extern int	getusername(char *,int,uid_t) ;
extern int	mklogid(char *,int,const char *,int,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	tmpx_getuserterms(TMPX *,VECSTR *,const char *) ;
extern int	tmpx_getsessions(TMPX *,VECINT *,const char *) ;
extern int	isprintlatin(uint) ;
extern int	iseol(int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	proglog_begin(PROGINFO *,USERINFO *) ;
extern int	proglog_end(PROGINFO *) ;
extern int	proglog_print(PROGINFO *,cchar *,int) ;
extern int	proglog_printf(PROGINFO *,cchar *,...) ;
extern int	proglog_flush(PROGINFO *) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		names:1 ;
	uint		ut:1 ;
	uint		outfile:1 ;
	uint		sn:1 ;
	uint		nterms:1 ;
	uint		to:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	vecstr		names ;
	TMPX		ut ;
	SESNOTES	sn ;
	PROGINFO	*pip ;
	void		*ofp ;
	const char	*nodename ;
	pid_t		pid ;
	int		to ;
	int		nterms ;
	int		count ;
	char		logid[LOGFILE_LOGIDLEN+1] ;
	char		username[USERNAMELEN+1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,
			cchar *,cchar *,cchar *,int) ;
static int	procnames(PROGINFO *,const char *,int,const char *,int) ;
static int	procname(PROGINFO *, const char *,int,const char *,int) ;
static int	procgather(PROGINFO *,char *,int,const char *) ;

static int	procnotify(PROGINFO *,const char *,int,
			const char *,int) ;
static int	procnotifier(PROGINFO *,const char *,int,
			const char *,int) ;
static int	procnotesessions(PROGINFO *,const char *,int,
			const char *,int) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	noteone(PROGINFO *,const char *,const char *,int) ;
static int	writenotice(PROGINFO *,int,const char *,int) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_already(LOCINFO *,const char *,int) ;
static int	locinfo_snbegin(LOCINFO *) ;
static int	locinfo_snsend(LOCINFO *,const char *,int,pid_t) ;
static int	locinfo_snend(LOCINFO *) ;

#if	CF_LOCSET
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCSET */


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"lf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_lf,
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

static const char *akonames[] = {
	"nterms",
	"to",
	NULL
} ;

enum akonames {
	akoname_nterms,
	akoname_to,
	akoname_overlast
} ;

static const char	eol[] = "\033[K\r\n" ;

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


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
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
	proginfo_setbanner(pip,cp) ;

/* early initialization */

	pip->verboselevel = 1 ;
	pip->f.logprog = TRUE ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

	                case argopt_help:
	                    f_help = TRUE ;
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

/* argument file */
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

/* log file */
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

	                    case 'L':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->lfname = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
				} else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'a':
	                        pip->f.all = TRUE ;
	                        break ;

	                    case 'b':
	                        pip->f.biffonly = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.biffonly = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'l':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->lfname = argp ;
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

/* number of terminals */
	                    case 'n':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            lip->nterms = rs ;
	                        }
				} else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 'r':
	                        pip->f.ringbell = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.ringbell = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 't':
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecti(argp,argl,&v) ;
	                            lip->to = v ;
	                        }
				} else
	                            rs = SR_INVALID ;
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

	        } /* end if (digit or not) */

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
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get some program information */

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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	rs = procopts(pip,&akopts) ;

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking arguments\n") ;
#endif

	if ((rs >= 0) && (lip->nterms < 0) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&v) ;
	    lip->nterms = MAX(v,0) ;
	}

	if ((rs >= 0) && (lip->nterms < 0)) {
	    if ((cp = getourenv(envv,VARNTERMS)) != NULL) {
	        rs = cfdeci(cp,-1,&v) ;
	        lip->nterms = MAX(v,0) ;
	    }
	}

	if (rs >= 0) {
	    if (pip->f.all) {
	        lip->nterms = NALLTERMS ;
	    } else {
	        if (lip->nterms < 0) lip->nterms = NDEFTERMS ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: rs=%d numterms=%u\n",rs,lip->nterms) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: nterms=%d\n",pip->progname,lip->nterms) ;
	}

/* OK, do the output */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = proglog_begin(pip,&u)) >= 0) {
	                if ((rs = proguserlist_begin(pip)) >= 0) {
			    {
			        ARGINFO	*aip = &ainfo ;
			        BITS	*bop = &pargs ;
	   		        cchar	*ofn = ofname ;
			        cchar	*afn = afname ;
			        cchar	*ifn = ifname ;
			        rs = process(pip,aip,bop,ofn,afn,ifn) ;
			    }
			    rs1 = proguserlist_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
		        } /* end if (proguserlist) */
		        rs1 = proglog_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
		    } /* end if (proglog) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt ;
	        fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if */

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    bprintf(lip->ofp,"%s: users=%u notified=%u\n",
	        pip->progname,pan,lip->count) ;
	}

	if ((rs >= 0) && (pan > 0) && (lip->count <= 0)) {
	    ex = EX_DATAERR ;
	    if ((pip->debuglevel > 0) && (lip->count == 0)) {
	        bprintf(pip->efp,"%s: could not notify any users\n",
	            pip->progname) ;
	    }
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	}

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
	    rs1 = keyopt_finish(&akopts) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = bits_finish(&pargs) ;
	if (rs >= 0) rs = rs1 ;

badpargs:
	locinfo_finish(lip) ;

badlocstart:
	rs1 = proginfo_finish(pip) ;
	if (rs >= 0) rs = rs1 ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mi[12] ;
	    uint	mo ;
	    uint	mdiff ;
	    uc_mallout(&mo) ;
	    mdiff = (mo-mo_start) ;
	    debugprintf("main: final mallout=%u\n",mdiff) ;
	    if (mdiff > 0) {
		UCMALLREG_CUR	cur ;
		UCMALLREG_REG	reg ;
		const int	size = (10*sizeof(uint)) ;
		const char	*ids = "main" ;
		uc_mallinfo(mi,size) ;
	        debugprintf("main: MIoutnum=%u\n",mi[ucmallreg_outnum]) ;
	        debugprintf("main: MIoutnummax=%u\n",mi[ucmallreg_outnummax]) ;
	        debugprintf("main: MIoutsize=%u\n",mi[ucmallreg_outsize]) ;
	        debugprintf("main: MIoutsizemax=%u\n",
			mi[ucmallreg_outsizemax]) ;
	        debugprintf("main: MIused=%u\n",mi[ucmallreg_used]) ;
	        debugprintf("main: MIusedmax=%u\n",mi[ucmallreg_usedmax]) ;
	        debugprintf("main: MIunder=%u\n",mi[ucmallreg_under]) ;
	        debugprintf("main: MIover=%u\n",mi[ucmallreg_over]) ;
	        debugprintf("main: MInotalloc=%u\n",mi[ucmallreg_notalloc]) ;
	        debugprintf("main: MInotfree=%u\n",mi[ucmallreg_notfree]) ;
		ucmallreg_curbegin(&cur) ;
		while (ucmallreg_enum(&cur,&reg) >= 0) {
	            debugprintf("main: MIreg.addr=%p\n",reg.addr) ;
	            debugprintf("main: MIreg.size=%u\n",reg.size) ;
		    debugprinthexblock(ids,80,reg.addr,reg.size) ;
		}
		ucmallreg_curend(&cur) ;
	    }
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad usage stuff */
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

	fmt = "%s: USAGE> %s [<user>[=<n>] ...] [-af <afile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-r] [-a] [-n <num>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

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

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                switch (oi) {
	                case akoname_nterms:
	                    if (! lip->final.nterms) {
	                        lip->have.nterms = TRUE ;
	                        lip->final.nterms = TRUE ;
	                        if (vl > 0) {
				    rs = optvalue(vp,vl) ;
				    lip->nterms = rs ;
				}
	                    }
	                    break ;
	                case akoname_to:
	                    if (! lip->final.to) {
	                        lip->have.to = TRUE ;
	                        lip->final.to = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecti(vp,vl,&v) ;
	                            lip->to = v ;
	                        }
	                    }
	                    break ;
	                default:
	                    rs = SR_INVALID ;
	                    break ;
	                } /* end switch */

	                c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,
		cchar *ofn,cchar *afn,cchar *ifn)
{
	const int	mlen = BUFLEN ;
	int		rs ;
	char		mbuf[BUFLEN+1] = { 0 } ;
	if ((rs = procgather(pip,mbuf,mlen,ifn)) >= 0) {
	    rs = procargs(pip,aip,bop,ofn,afn,mbuf,rs) ;
	} /* end if (procgather) */
	return rs ;
}
/* end subroutine (process) */


static int procgather(PROGINFO *pip,char mbuf[],int mlen,cchar ifname[])
{
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		rs1 ;
	int		bl = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procgather: ent ifname=%s\n",ifname) ;
#endif

	mbuf[0] = '\0' ;
	if ((ifname == NULL) || (ifname[0] == '\0') || (ifname[0] == '-'))
	    ifname = BFILE_STDIN ;

	if ((rs = bopen(ifp,ifname,"r",0666)) >= 0) {
	    SBUF	b ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    if ((rs = sbuf_start(&b,mbuf,mlen)) >= 0) {
	        int	i ;
	        int	f ;
		int	ch ;
	        int	f_eol = FALSE ;

	        while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	            len = rs ;

	            for (i = 0 ; i < len ; i += 1) {

	                f_eol = (lbuf[i] == '\n') ;
	                if ((i == 80) && (! f_eol))
	                    sbuf_char(&b,'\n') ;

			ch = MKCHAR(lbuf[i]) ;
	                f = isprintlatin(ch) ;

	                f = f || f_eol ;
	                f = f || (lbuf[i] == '\t') ;
	                f = f || (lbuf[i] == CH_BEL) ;

	                if (f) {
	                    sbuf_char(&b,lbuf[i]) ;
	                } else
	                    sbuf_char(&b,' ') ;

			rs = sbuf_getlen(&b) ;
			if (rs < 0) break ;
	            } /* end for */

		    if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (len > 0) && (! f_eol))
	            sbuf_char(&b,'\n') ;

	        bl = sbuf_finish(&b) ;
	        if (rs >= 0) rs = bl ;
	    } /* end if (sbuf) */

	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (input-file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procgather: ret rs=%d bl=%u\n",rs,bl) ;
#endif

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (procgather) */


static int procargs(pip,aip,bop,ofn,afn,mbuf,mlen)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
const char	*ofn ;
const char	*afn ;
const char	*mbuf ;
int		mlen ;
{
	LOCINFO		*lip = pip->lip ;
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    int		cl ;
	    const char	*cp ;
	    lip->ofp = ofp ;

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
	            	    rs = procname(pip,mbuf,mlen,cp,-1) ;
		    	    c += rs ;
			}
		    }
	            if (rs < 0) break ;
	        } /* end for (looping through requested) */
	    } /* end if (ok) */

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
				rs = procnames(pip,mbuf,mlen,cp,cl) ;
				c += rs ;
			    }
			} /* end if (sfskipwhite) */

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            bclose(afp) ;
	        } else {
	            if (! pip->f.quiet) {
			fmt = "%s: inaccesssible argument list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {

	        cp = pip->username ;
	        pan += 1 ;
	        rs = procname(pip,mbuf,mlen,cp,-1) ;
	        c += rs ;

	    } /* end if */

	    if ((rs >= 0) && (pip->verboselevel >= 2) && lip->f.outfile) {
	        bprintf(lip->ofp,"users=%d notified=%d\n",pan,lip->count) ;
	    } /* end if */

	    lip->ofp = NULL ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int procnames(PROGINFO *pip,cchar *mbuf,int mlen,cchar *ap,int al)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,ap,al)) >= 0) {
	    int		fl ;
	    const char	*fp ;
	    const uchar	*at = aterms ;
	    while ((fl = field_get(&fsb,at,&fp)) >= 0) {
		if (fl > 0) {
		    rs = procname(pip,mbuf,mlen,fp,fl) ;
		    c += rs ;
		}
		if (fsb.term == '#') break ;
		if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnames) */


static int procname(PROGINFO *pip,cchar *mbuf,int mlen,cchar *np,int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		c = 0 ;

	if (nl < 0) nl = strlen(np) ;

	if ((rs = procnotify(pip,mbuf,mlen,np,nl)) >= 0) {
	    c += rs ;
	        if (rs > 0) lip->count += rs ;

	        if ((pip->debuglevel > 0) && (rs == 0)) {
	            bprintf(pip->efp,
	                "%s: could not notify userspec >%t<\n",
	                pip->progname,np,nl) ;
		}

	        if ((pip->debuglevel >= 2) && (rs >= 0)) {
	            bprintf(pip->efp,
	                "%s: userspec >%t< notified=%u\n",
	                pip->progname,np,nl,rs) ;
		}

	} /* end if (procnotify) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procname) */


static int procnotify(PROGINFO *pip,cchar mbuf[],int mlen,cchar up[],int ul)
{
	LOCINFO		*lip = pip->lip ;
	const int	ulen = USERNAMELEN ;
	int		rs = SR_OK ;
	int		cl ;
	int		nterms = 0 ;
	int		c = 0 ;
	const char	*pn = pip->progname ;
	const char	*tp ;
	const char	*cp ;
	char		ubuf[USERNAMELEN + 1] ;

	if (mbuf == NULL) return SR_FAULT ;
	if (up == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("procnotify: ent us=%t\n",up,ul) ;
	    debugprintf("procnotify: mlen=%d mbuf=%t\n",
	        mlen,mbuf,strlinelen(mbuf,mlen,45)) ;
	}
#endif

	if (up[0] == '\0') return SR_INVALID ;

/* parse the username specification into the real username and count */

	if ((tp = strnchr(up,ul,'=')) != NULL) {
	    cl = sfshrink(up,(tp-up),&cp) ;
	    rs = cfdeci((tp+1),-1,&nterms) ;
	} else {
	    nterms = lip->nterms ;
	    cl = sfshrink(up,ul,&cp) ;
	} /* end if */

	if (cl > 0) {
	    int		ul ;
	    const char	*up ;
	    if (cp[0] == '-') {
		up = pip->username ;
		ul = strlen(up) ;
	    } else {
		up = ubuf ;
	        ul = strdcpy1w(ubuf,ulen,cp,cl) - ubuf ;
	    }

	if (pip->open.logprog)
	    proglog_printf(pip,"recip=%t",up,ul) ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: recip=%t\n",pn,up,ul) ;

	    if ((rs = locinfo_already(lip,up,ul)) == 0) {
	        if (rs >= 0) {
	            rs = procnotifier(pip,mbuf,mlen,up,nterms) ;
		    c += rs ;
	        } /* end if (ok) */
	        if (rs >= 0) {
	            rs = procnotesessions(pip,mbuf,mlen,up,nterms) ;
		    c += rs ;
		} /* end if (ok) */
	    } /* end if (not-already) */

	} else
	    rs = SR_INVALID ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procnotify) */


static int procnotifier(PROGINFO *pip,cchar mbuf[],int mlen,cchar un[],int nt)
{
	LOCINFO		*lip = pip->lip ;
	VECSTR		uterms ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("notify: un=%s nt=%d\n",un,nt) ;
#endif

/* get the terminals that this user is logged in on */

	if ((rs = vecstr_start(&uterms,10,0)) >= 0) {
	    TMPX	*txp = &lip->ut ;
	    if ((rs = tmpx_getuserterms(txp,&uterms,un)) >= 0) {
		int		i ;
		const char	*cp ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("notify: tmpx_getuserterms() rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            for (i = 0 ; vecstr_get(&uterms,i,&cp) >= 0 ; i += 1) {
	                if (cp != NULL)
	                debugprintf("notify: line=%s\n",cp) ;
	            }
	        }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("notify: top-of-loop\n") ;
#endif

	        n = 0 ;
	        for (i = 0 ; vecstr_get(&uterms,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("notify: termdev=%s\n",cp) ;
#endif

	                rs = noteone(pip,cp,mbuf,mlen) ;
			n += rs ;

	                if (pip->open.logprog) {
	                    fmt = "  line=%s (%d)" ;
	                    proglog_printf(pip,fmt,cp,rs) ;
			}

			if (pip->debuglevel > 0) {
	    		    fmt = "%s:   line=%s (%d)\n" ;
	    		    bprintf(pip->efp,fmt,pn,cp,rs) ;
			}

	                if ((nt > 0) && (n >= nt)) break ;
	            if (rs < 0) break ;
	        } /* end for (writing them) */

	        if (pip->open.logprog)
	            proglog_printf(pip,"  terms-sent=%u",n) ;

		if (pip->debuglevel > 0) {
		    bprintf(pip->efp,"%s:  terms-sent=%u\n",pn,n) ;
		}

	    } /* end if (tmpx_getuserterms) */
	    rs1 = vecstr_finish(&uterms) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("notify: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procnotifier) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	LOCINFO		*lip = pip->lip ;
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
	    const char	*nn = pip->nodename ;
	    const char	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        const char	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	if (rs >= 0) {
	    rs = locinfo_snbegin(lip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procuserinfo_begin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;

	rs1 = locinfo_snend(lip) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


static int noteone(PROGINFO *pip,cchar termfname[],cchar mbuf[],int mlen)
{
	const int	oflags = (O_WRONLY | O_NOCTTY) ;
	int		rs ;
	int		n = 0 ;

	if ((rs = uc_open(termfname,oflags,0666)) >= 0) {
	    struct ustat	sb ;
	    const int		fd = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("notify: got one termdev=%s fd=%d\n",
	            termfname,fd) ;
#endif

	    if ((rs = u_fstat(fd,&sb)) >= 0) {
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("notify: mode=\\o%08o\n",sb.st_mode) ;
#endif
		if (sb.st_mode & S_IWGRP) {
	            if ((! pip->f.biffonly) || (sb.st_mode & S_IXUSR)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("notify: writenotice()\n") ;
#endif

	                if ((rs = writenotice(pip,fd,mbuf,mlen)) >= 0) {
	                    n += 1 ;
	                    if ((rs >= 0) && (pip->verboselevel >= 2)) {
			        LOCINFO	*lip = pip->lip ;
			        if (pip->f.outfile) {
			            bfile	*ofp = lip->ofp ;
			            if (ofp != NULL) {
				        const char	*tf = termfname ;
	                                rs = bprintf(ofp,"%s\n",tf) ;
			            }
			        }
	                    } /* end if (verbose) */
	                } /* end if (successful write) */

	            } /* end if (go-ahead) */
	 	} /* end if (group-writable) */
	    } /* end if (stat) */

	    u_close(fd) ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (noteone) */


static int writenotice(pip,fd_termdev,tbuf,tlen)
PROGINFO	*pip ;
int		fd_termdev ;
const char	tbuf[] ;
int		tlen ;
{
	SBUF		out ;
	const int	mlen = BUFLEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		mbuf[BUFLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("writenotice: fd_termdev=%d tlen=%d \n",
	        fd_termdev,tlen) ;
	    debugprintf("writenotice: tlen=%d tbuf=%t\n",
	        tlen,tbuf,strlinelen(tbuf,tlen,45)) ;
	}
#endif

	if ((rs = sbuf_start(&out,mbuf,mlen)) >= 0) {
	    int		sl, cl ;
	    const char	*sp, *cp ;
	    const char	*tp ;

/* form the notice to write out */

	    if (pip->f.ringbell)
	        sbuf_char(&out,CH_BELL) ;

	    sbuf_char(&out,'\r') ;

	    sp = tbuf ;
	    sl = tlen ;
	    while ((tp = strnchr(sp,sl,'\n')) != NULL) {

	        cp = sp ;
	        cl = (tp - sp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("writenotice: line=>%t<\n",cp,cl) ;
#endif

	        sbuf_strw(&out,cp,cl) ;

	        rs = sbuf_strw(&out,(char *) eol,5) ;

	        sl -= ((tp + 1) - sp) ;
	        sp = (tp + 1) ;

	    } /* end while */

	    if ((rs >= 0) && (sl > 0)) {

	        cp = sp ;
	        cl = sl ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("writenotice: line=>%t<\n",cp,cl) ;
#endif

	        sbuf_strw(&out,cp,cl) ;

	        sbuf_strw(&out,(char *) eol,5) ;

	    } /* end if (some residue remaining) */

	    wlen = sbuf_getlen(&out) ;
	    if (rs >= 0) rs = wlen ;

/* write the notice out */

	    if ((rs >= 0) && (wlen > 0))
	        rs = u_write(fd_termdev,mbuf,wlen) ;

	    rs1 = sbuf_finish(&out) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (writenotice) */


/* ARGSUSED */
static int procnotesessions(pip,mbuf,mlen,un,nt)
PROGINFO	*pip ;
const char	mbuf[] ;
int		mlen ;
const char	un[] ;
int		nt ;
{
	LOCINFO		*lip = pip->lip ;
	VECINT		sessions ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;
	const char	*pn = pip->progname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procnotesessions: ent un=%s nt=%d\n",un,nt) ;
#endif

/* get the terminals that this user is logged in on */

	if ((rs = vecint_start(&sessions,10,0)) >= 0) {
	    TMPX	*txp = &lip->ut ;
	    if ((rs = tmpx_getsessions(txp,&sessions,un)) >= 0) {
		int	i ;
		int	v ;

	        n = 0 ;
	        for (i = 0 ; vecint_getval(&sessions,i,&v) >= 0 ; i += 1) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procnotesessions: ses=%u\n",v) ;
#endif
		    if (v > 0) {
			pid_t	sid = (pid_t) v ;
	                rs = locinfo_snsend(lip,mbuf,mlen,sid) ;
			n += rs ;
		    }
	            if (rs < 0) break ;
	        } /* end for (sending them) */

	        if (pip->open.logprog)
	            proglog_printf(pip,"  sessions-sent=%u",n) ;

		if (pip->debuglevel > 0) {
		    bprintf(pip->efp,"%s:  sessions-sent=%u\n",pn,n) ;
		}

	    } /* end if (tmpx_getsessions) */
	    rs1 = vecint_finish(&sessions) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecint) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procnotesessions: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procnotesessions) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;
	lip->to = -1 ;
	lip->nterms = -1 ;
	lip->pid = getpid() ;

	if ((rs = tmpx_open(&lip->ut,NULL,O_RDONLY)) >= 0) {
	    lip->open.ut = TRUE ;
	    if ((rs = vecstr_start(&lip->names,10,0)) >= 0) {
	        lip->open.names = TRUE ;
	    }
	    if (rs < 0) {
	        lip->open.ut = FALSE ;
		tmpx_close(&lip->ut) ;
	    }
	} /* end if (tmpx_start) */

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

	if (lip->open.names) {
	    lip->open.names = FALSE ;
	    rs1 = vecstr_finish(&lip->names) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (lip->open.ut) {
	    lip->open.ut = FALSE ;
	    rs1 = tmpx_close(&lip->ut) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSET
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar vp[],int vl)
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
#endif /* CF_LOCSET */


static int locinfo_already(LOCINFO *lip,const char *np,int nl)
{
	int		rs ;

	if ((rs = vecstr_adduniq(&lip->names,np,nl)) == INT_MAX) {
	    rs = 1 ;
	} else
	    rs = 0 ;

	return rs ;
}
/* end subroutine (locinfo_already) */


static int locinfo_snbegin(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (! lip->open.sn) {
	    PROGINFO	*pip = lip->pip ;
	    if ((rs = sesnotes_open(&lip->sn,pip->username)) >= 0) {
		lip->open.sn = TRUE ;
	    }
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/locinfo_snbegin: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_snbegin) */


static int locinfo_snsend(LOCINFO *lip,cchar *mbuf,int mlen,pid_t sid)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_snsend: ent sid=%d\n",sid) ;
#endif
	if (lip->open.sn) {
	    while (mlen && iseol(MKCHAR(mbuf[mlen-1]))) mlen -= 1 ;
	    rs = sesnotes_sendgen(&lip->sn,mbuf,mlen,sid) ;
	} else
	    rs = SR_BUGCHECK ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/locinfo_snsend: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (locinfo_snsend) */


static int locinfo_snend(LOCINFO *lip)
{
	int		rs = SR_OK ;
	if (lip->open.sn) {
	    lip->open.sn = FALSE ;
	    rs = sesnotes_close(&lip->sn) ;
	}
	return rs ;
}
/* end subroutine (locinfo_snend) */


