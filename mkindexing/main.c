/* main */

/* main subroutine for the MKINDEXING system */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocation */
#define	CF_LATIN	1		/* treat Latin as characters */
#define	CF_COLON	0		/* treat colon as character */


/* revision history:

	= 1999-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the 'main' module for the MKKEY program.  This module processes
	the program invocation arguments and performs some preprocessing steps
	before any actual input files are scanned.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<userinfo.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<char.h>
#include	<varsub.h>
#include	<hdb.h>
#include	<field.h>
#include	<ascii.h>
#include	<eigendb.h>
#include	<upt.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"proglog.h"
#include	"proguserlist.h"
#include	"progeigen.h"
#include	"progids.h"


/* local defines */

#define	PINFO	struct pinfo


/* external subroutines */

extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getnprocessors(cchar **,int) ;
extern int	vecstr_adduniq(vecstr *,cchar *,int) ;
extern int	makedate_get(cchar *,cchar **) ;
extern int	isprintlatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;

extern int	progkey(PROGINFO *,ARGINFO *,const uchar *,
			cchar *,cchar *,cchar *) ;
extern int	proginv(PROGINFO *,ARGINFO *,cchar *) ;
extern int	progquery(PROGINFO *,ARGINFO *,const uchar *,cchar *,cchar *) ;
extern int	progtagprint(PROGINFO *,ARGINFO *,cchar *, cchar *, cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */

extern cchar	mkindexing_makedate[] ;


/* local structures */

struct pinfo {
	cchar		*delim ;
	cchar		*ignchars ;
	cchar		*basedname ;
	cchar		*outfmt ;
	cchar		*dbname ;
	uchar		terms[32] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,cchar *) ;
static int	process(PROGINFO *,ARGINFO *,PINFO *,cchar *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	procmode_begin(PROGINFO *,ARGINFO *,cchar *) ;
static int	procmode_end(PROGINFO *) ;

static int	procdef_begin(PROGINFO *) ;
static int	procdef_end(PROGINFO *) ;

static int	arginfo_start(PROGINFO *,ARGINFO *,int,cchar **) ;
static int	arginfo_finish(PROGINFO *,ARGINFO *) ;
static int	arginfo_argmark(ARGINFO *,int) ;
static int	arginfo_arg(PROGINFO *,ARGINFO *,cchar *,int) ;

static int	loadncpus(PROGINFO *) ;

static int	pinfo_mkterms(PINFO *) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static cchar *argopts[] = {
	"ROOT",
	"CONFIG",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	"lf",
	"db",
	"basedir",
	"tablen",
	"tl",
	"sdn",
	"sfn",
	"dn",
	"lang",
	"npar",
	"iacc",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_config,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_lf,
	argopt_db,
	argopt_basedir,
	argopt_tablen,
	argopt_tl,
	argopt_sdn,
	argopt_sfn,
	argopt_dn,
	argopt_lang,
	argopt_npar,
	argopt_iacc,
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

static cchar	*progmodes[] = {
	"mkkey",
	"mkinv",
	"mkquery",
	"mktagprint",
	NULL
} ;

enum progmodes {
	progmode_mkkey,
	progmode_mkinv,
	progmode_mkquery,
	progmode_mktagprint,
	progmode_overlast
} ;

static cchar *akonames[] = {
	"bible",
	"nofile",
	"par",
	"unique",
	"audit",
	"sendparams",
	"linelen",
	"indent",
	"tablen",
	"tl",
	"keyfold",
	"nodebug",
	"iacc",
	NULL
} ;

enum akonames {
	akoname_bible,
	akoname_nofile,
	akoname_par,
	akoname_uniq,
	akoname_audit,
	akoname_sendparams,
	akoname_linelen,
	akoname_indent,
	akoname_tablen,
	akoname_tl,
	akoname_keyfold,
	akoname_nodebug,
	akoname_iacc,
	akoname_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo, *aip = &ainfo ;
	KEYOPT		akopts ;
	PINFO		pinfo ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai ;
	int		rs, rs1 ;
	int		v ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_makedate = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*varsearch = VARSEARCHNAME ;
	cchar		*pmspec = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*efname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*cfname = NULL ;
	cchar		*hfname = NULL ;
	cchar		*sdn = NULL ;
	cchar		*sfn = NULL ;
	cchar		*cp ;

	if_exit = 0 ;
	if_int = 0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) != NULL) {
	    rs1 = debugopen(cp) ;
	    debugprintf("main: starting FD=%d\n",rs1) ;
	}
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	memset(&pinfo,0,sizeof(PINFO)) ;
	pinfo.ignchars = IGNORECHARS ;
	pinfo.delim = DEFDELIM ;

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->minwordlen = -2 ;
	pip->maxwordlen = -2 ;
	pip->eigenwords = -2 ;
	pip->maxkeys = -2 ;
	pip->f.optsendparams = OPT_SENDPARAMS ;
	pip->f.logprog = OPT_LOGPROG ;

/* arguments */

	rs = arginfo_start(pip,aip,argc,argv) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badargstart ;
	}

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

#if	CF_DEBUGS
	debugprintf("main: keyopt_start() rs=%d\n",rs) ;
#endif

	aip->ai_max = 0 ;
	aip->ai_pos = 0 ;
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

	            aip->ai_pos = ai ;
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

/* configuration file */
	                case argopt_config:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

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
	                    f_makedate = f_version ;
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

/* print out the help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pmspec = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pmspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

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

	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pinfo.dbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pinfo.dbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_basedir:
	                case argopt_dn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pinfo.basedname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pinfo.basedname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_sdn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sdn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sdn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_sfn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sfn = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                sfn = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_tablen:
	                case argopt_tl:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecmfi(argp,argl,&v) ;
	                            pip->tablen = v ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_lang:
	                    if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->eigenlang = argp ;
	                    } else
	                            rs = SR_INVALID ;
	                    break ;

	                case argopt_npar:
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
					pip->npar = rs ;
				    }
	                        } else
	                            rs = SR_INVALID ;
	                    break ;

	                case argopt_iacc:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
				    rs = optbool(avp,avl) ;
	                            pip->f.iacc = rs ;
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
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* configuration file */
	                    case 'C':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                cfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

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

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        pip->quietlevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->quietlevel = rs ;
	                            }
	                        }
	                        break ;

/* version */
	                    case 'V':
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

/* append to the key file */
	                    case 'a':
	                        pip->f.append = TRUE ;
	                        break ;

	                    case 'b':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pinfo.basedname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* common words file (eigenfile) */
	                    case 'c':
	                    case 'e':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->eigenfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* entry delimiter string */
	                    case 'd':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            pinfo.delim = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* output format */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            pinfo.outfmt = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'i':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pinfo.ignchars = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* maximum number of keys written out */
	                    case 'k':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->maxkeys = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* minimum word length */
	                    case 'l':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->minwordlen = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* maximum word length */
	                    case 'm':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->maxwordlen = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* number of eigenwords to consider */
	                    case 'n':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->eigenwords = rs ;
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

/* prefix match */
	                    case 'p':
	                        pip->f.prefix = TRUE ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* remove labels */
	                    case 's':
	                        pip->f.removelabel = TRUE ;
	                        break ;

/* index whole files */
	                    case 'w':
	                        pip->f.wholefile = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.wholefile = (rs > 0) ;
	                            }
	                        }
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

	        rs = arginfo_argmark(aip,ai) ;
	        ainfo.ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    aip->ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	debugprintf("main: args-out rs=%d\n",rs) ;
#endif

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"dwca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
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
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	    if (f_makedate) {
	        cl = makedate_get(mkindexing_makedate,&cp) ;
	        bprintf(pip->efp,"%s: makedate %t\n", pip->progname,cp,cl) ;
	    }
	} /* end if */

/* get our program mode */

	if (pmspec == NULL) pmspec = pip->progname ;
	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if ((pip->progmode < 0) && (sn != NULL))
	    pip->progmode = matstr(progmodes,sn,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

	if (pip->progmode < 0)
	    pip->progmode = progmode_mkkey ;

/* this renaming of the "search-name" is really up to individual tastes! */

	if (sn == NULL) sn = getenv(varsearch) ;
	if (sn == NULL) sn = progmodes[pip->progmode] ;

/* get our program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,varsearch,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main: pr=%s\n", pip->pr) ;
	    debugprintf("main: sn=%s\n", pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* print out the help file if requested */

	if (f_help) {
	    if ((hfname == NULL) || (hfname[0] == '\0')) hfname = HELPFNAME ;
	    printhelp(NULL,pip->pr,pip->searchname,hfname) ;
	} /* end if (hfname) */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* other option processing */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: progmode=%s(%u)\n",
	        progmodes[pip->progmode],pip->progmode) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pm=%s (%u)\n",
	        pip->progname,progmodes[pip->progmode],pip->progmode) ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pinfo.dbname == NULL) pinfo.dbname = getenv(VARDBNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

#if	CF_DEBUG || CF_DEBUGS
	if (pip->f.nodebug) debugclose() ;
	debugprintf("main: pn=%s nodebug=%u\n",pip->progname,pip->f.nodebug) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    const int	f_sp = pip->f.optsendparams ;
	    bprintf(pip->efp,"%s: cfname=%s\n",pn,cfname) ;
	    bprintf(pip->efp,"%s: sendparams=%u\n",pn,f_sp);
	}

	if ((rs >= 0) && (pip->npar == 0)) {
	    if ((cp = getourenv(envv,VARNPAR)) != NULL) {
		rs = optvalue(cp,-1) ;
		pip->npar = rs ;
	    }
	}

	if ((rs >= 0) && (pip->npar == 0)) {
	    rs = loadncpus(pip) ;
	    pip->npar = (rs+1) ;
	}

	if (rs >= 0) {
	    uptsetconcurrency(pip->npar) ; /* for library objects */
	}

/* line length */

	if ((rs >= 0) && (pip->linelen == 0)) {
	    cp = argval ;
	    if (cp == NULL) cp = getenv(VARLINELEN) ;
	    if (cp == NULL) cp = getenv(VARCOLUMNS) ;
	    if (cp != NULL) {
	        rs = optvalue(cp,-1) ;
	        pip->linelen = rs ;
	    }
	    if (pip->linelen == 0) pip->linelen = COLUMNS ;
	} /* end if (line-fold length) */

/* table length */

	if ((rs >= 0) && (pip->tablen <= 0)) {
	    if ((cp = getenv(VARTABLEN)) != NULL) {
	        rs = cfdecmfi(cp,-1,&v) ;
	        pip->tablen = v ;
	    }
	}

	pinfo_mkterms(&pinfo) ;

	if ((afname != NULL) && (afname[0] != '\0'))
	    aip->afname = afname ;

	if (pip->minwordlen < -1)
	    pip->minwordlen = MINWORDLEN ;

	if (pip->maxwordlen < -1)
	    pip->maxwordlen = MAXWORDLEN ;

	if (pip->maxkeys < -1)
	    pip->maxkeys = MAXKEYS ;

	if (pip->eigenwords < 0)
	    pip->eigenwords = MAXEIGENWORDS ;

	if (rs >= 0) {
	    const int	tlen = MAXPATHLEN ;
	    int		tl = -1 ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if (sdn == NULL) {
	        sdn = tbuf ;
	        rs = proginfo_getpwd(pip,tbuf,tlen) ;
		tl = rs ;
	    }
	    if ((rs >= 0) && (sdn != NULL)) {
	        cchar	**vpp = &pip->sdn ;
	        rs = proginfo_setentry(pip,vpp,sdn,tl) ;
	    }
	} /* end if (ok) */

	if ((rs >= 0) && (sfn != NULL)) {
	    cchar	**vpp = &pip->sfn ;
	    rs = proginfo_setentry(pip,vpp,sfn,-1) ;
	}

	if ((rs >= 0) && (pinfo.basedname != NULL)) {
	    cchar	**vpp = &pip->basedname ;
	    rs = proginfo_setentry(pip,vpp,pinfo.basedname,-1) ;
	}

/* continue */

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
		    if ((rs = proglog_begin(pip,&u)) >= 0) {
			if ((rs = proguserlist_begin(pip)) >= 0) {
			    PINFO	*inp = &pinfo ;
			    cchar	*ofn = ofname ;
			    cchar	*afn = afname ;
			    if ((rs = procmode_begin(pip,aip,afn)) >= 0) {
				if ((rs = procdef_begin(pip)) >= 0) {
				    {
				        rs = process(pip,aip,inp,ofn) ;
				    }
				    rs1 = procdef_end(pip) ;
				    if (rs >= 0) rs = rs1 ;
				}
				rs1 = procmode_end(pip) ;
				if (rs >= 0) rs = rs1 ;
			    } /* end if (procmode) */
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
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
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
	    pip->f.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: arginfo_finish() \n") ;
#endif

	rs1 = arginfo_finish(pip,aip) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: arginfo_finish() rs=%d\n",rs1) ;
#endif

badargstart:
	proginfo_finish(pip) ;

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
	        cchar	*ids = "main" ;
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

/* bad argument usage */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


int progexit(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip == NULL) return SR_FAULT ;
	if (if_exit) rs = SR_INTR ;
	return rs ;
}
/* end subroutine (progexit) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-d <delim>] [<file(s)>] [-af <afile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-C <conf>] [-l <minwordlen>] [-m <maxwordlen>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-n <eigenwords>] [-asw] [-c <eigenfile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-i <ignorechars>] [-k <nkeys>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        int	v ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                c += 1 ;
	                switch (oi) {
	                case akoname_bible:
	                    if (! pip->final.optbible) {
	                        pip->final.optbible = TRUE ;
	                        pip->have.optbible = TRUE ;
	                        pip->f.optbible = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.optbible = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_nofile:
	                    if (! pip->final.optnofile) {
	                        pip->final.optnofile = TRUE ;
	                        pip->have.optnofile = TRUE ;
	                        pip->f.optnofile = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.optnofile = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_keyfold:
	                    if (! pip->final.keyfold) {
	                        pip->final.keyfold = TRUE ;
	                        pip->have.keyfold = TRUE ;
	                        pip->f.keyfold = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.keyfold = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_par:
	                    if (! pip->final.optpar) {
	                        pip->final.optpar = TRUE ;
	                        pip->have.optpar = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            pip->npar = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_tablen:
	                case akoname_tl:
	                    if (! pip->final.tablen) {
	                        pip->final.tablen = TRUE ;
	                        pip->have.tablen = TRUE ;
	                        if (vl > 0) {
	                            rs = cfdecmfi(vp,vl,&v) ;
	                            pip->tablen = v ;
	                        }
	                    }
	                    break ;
	                case akoname_linelen:
	                    if (! pip->final.linelen) {
	                        pip->final.linelen = TRUE ;
	                        pip->have.linelen = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            pip->linelen = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_indent:
	                    if (! pip->final.indent) {
	                        pip->final.indent = TRUE ;
	                        pip->have.indent = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            pip->indent = rs ;
	                        }
	                    }
	                    break ;
	                case akoname_uniq:
	                    if (! pip->final.optuniq) {
	                        pip->final.optuniq = TRUE ;
	                        pip->have.optuniq = TRUE ;
	                        pip->f.optuniq = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.optuniq = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_audit:
	                    if (! pip->final.optaudit) {
	                        pip->final.optaudit = TRUE ;
	                        pip->have.optaudit = TRUE ;
	                        pip->f.optaudit = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.optaudit = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_sendparams:
	                    if (! pip->final.optsendparams) {
	                        pip->final.optsendparams = TRUE ;
	                        pip->have.optsendparams = TRUE ;
	                        pip->f.optsendparams = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.optsendparams = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_nodebug:
	                    if (! pip->final.nodebug) {
	                        pip->final.nodebug = TRUE ;
	                        pip->have.nodebug = TRUE ;
	                        pip->f.nodebug = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.nodebug = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_iacc:
	                    if (! pip->final.iacc) {
	                        pip->final.iacc = TRUE ;
	                        pip->have.iacc = TRUE ;
	                        pip->f.iacc = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.iacc = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	            } /* end if (got a match) */

	            if (rs < 0) break ;
	        } /* end while */

	        rs1 = keyopt_curend(kop,&kcur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(PROGINFO *pip,ARGINFO *aip,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	cchar		*cp ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (rs >= 0) {
	    BITS	*bop = &aip->pargs ;
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {
	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = aip->argv[ai] ;
		    if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = arginfo_arg(pip,aip,cp,-1) ;
		    }
		}
	        if (rs < 0) break ;
	    } /* end for */
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
	                	rs = arginfo_arg(pip,aip,cp,cl) ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        }

	    } /* end if (argument-list) */

	    if ((rs >= 0) && (pan == 0)) {

	        cp = "-" ;
	        pan += 1 ;
	        rs = arginfo_arg(pip,aip,cp,-1) ;

	    } /* end if */

	return (rs >= 0) ? pan : rs ;
} 
/* end subroutine (procargs) */


static int process(PROGINFO *pip,ARGINFO *aip,PINFO *inp,cchar *ofn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	    switch (pip->progmode) {
	    case progmode_mkkey:
		if ((rs = progeigen_begin(pip)) >= 0) {
		    {
		        const uchar	*terms = inp->terms ;
		        cchar		*delim = inp->delim ;
		        cchar		*ign = inp->ignchars ;
	                rs = progkey(pip,aip,terms,delim,ign,ofn) ;
		    }
		    rs1 = progeigen_end(pip) ;
		    if (rs >= 0) rs = rs1 ;
		} /* end if (progeigen) */
	        break ;
	    case progmode_mkinv:
	        rs = proginv(pip,aip,inp->dbname) ;
	        break ;
	    case progmode_mkquery:
	        rs = progquery(pip,aip,inp->terms,inp->dbname,ofn) ;
	        if ((rs == SR_NOENT) && (pip->quietlevel == 0)) {
		    cchar	*fmt = "%s: could not open database (%d)\n" ;
		    cchar	*pn = pip->progname ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	  	}
	        break ;
	    case progmode_mktagprint:
	        rs = progtagprint(pip,aip,inp->basedname,inp->outfmt,ofn) ;
	        break ;
	    } /* end switch */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/process: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (process) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->logid = uip->logid ;

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


static int procdef_begin(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if (pip->npar == 0) {
	    if ((rs = getnprocessors(pip->envv,0)) == 0) rs = 1 ;
	    pip->npar = rs ;
	}
	return rs ;
}
/* end subroutine (procdef_begin) */


static int procdef_end(PROGINFO *pip)
{
	if (pip == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (procdef_end) */


static int procmode_begin(PROGINFO *pip,ARGINFO *aip,cchar *afn)
{
	int		rs = SR_OK ;
	switch (pip->progmode) {
	case progmode_mkkey:
	    pip->f.args = FALSE ;
	    pip->f.eigendb = TRUE ;
	    break ;
	case progmode_mkinv:
	    pip->f.args = TRUE ;
	    pip->f.eigendb = FALSE ;
	    break ;
	case progmode_mkquery:
	    pip->f.args = FALSE ;
	    pip->f.eigendb = FALSE ;
	    break ;
	case progmode_mktagprint:
	    pip->f.args = FALSE ;
	    pip->f.eigendb = FALSE ;
	    break ;
	} /* end switch */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmode: mid0 rs=%d\n",rs) ;
#endif
	if ((rs >= 0) && pip->f.args) {
	    rs = procargs(pip,aip,afn) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmode: mid1 rs=%d\n",rs) ;
#endif
	if ((rs >= 0) && (pip->f.eigendb || pip->f.ids)) {
	    rs = progids_begin(pip) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmode: mid2 rs=%d\n",rs) ;
#endif
	if ((rs >= 0) && pip->f.eigendb) {
	    rs = progeigen_begin(pip) ;
	}
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procmode: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroiutine (procmode_begin) */


static int procmode_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->open.eigendb) {
	    rs1 = progeigen_end(pip) ;
	    if (rs >= 0)  rs = rs1 ;
	}
	if (pip->open.ids) {
	    rs1 = progids_end(pip) ;
	    if (rs >= 0)  rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (procmode_end) */


static int arginfo_start(PROGINFO *pip,ARGINFO *aip,int argc,cchar **argv)
{
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
	if (aip == NULL) return SR_FAULT ;

	memset(aip,0,sizeof(ARGINFO)) ;
	aip->argc = argc ;
	aip->argv = argv ;

	if ((rs = vecstr_start(&aip->args,10,0)) >= 0) {
	    rs = bits_start(&aip->pargs,1) ;
	    if (rs < 0)
	        vecstr_finish(&aip->args) ;
	}

	return rs ;
}
/* end subroutine (arginfo_start) */


static int arginfo_finish(PROGINFO *pip,ARGINFO *aip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;
	if (aip == NULL) return SR_FAULT ;

	rs1 = vecstr_finish(&aip->args) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = bits_finish(&aip->pargs) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (arginfo_finish) */


static int arginfo_argmark(ARGINFO *aip,int ai)
{
	return bits_set(&aip->pargs,ai) ;
}
/* end subroutine (arginfo_argmark) */


static int arginfo_arg(PROGINFO *pip,ARGINFO *aip,cchar *sp,int sl)
{
	int		rs ;

	if (pip == NULL) return SR_FAULT ;
	if (aip == NULL) return SR_FAULT ;

	rs = vecstr_adduniq(&aip->args,sp,sl) ;

	return rs ;
}
/* end subroutine (arginfo_arg) */


static int loadncpus(PROGINFO *pip)
{
	int		rs = getnprocessors(pip->envv,0) ;
	if (pip == NULL) return SR_FAULT ;
	pip->ncpu = rs ;
	return rs ;
}
/* end subroutine (loadncpus) */


static int pinfo_mkterms(PINFO *inp)
{
	uchar		*terms = inp->terms ;
	int		rs = SR_OK ;
	int		i ;

	for (i = 0 ; i < 32 ; i += 1) {
	    terms[i] = 0xFF ;
	}

	BACLR(terms,'_') ;
	BACLR(terms,'-') ;

	for (i = 'a' ; i <= 'z' ; i += 1)
	    BACLR(terms,i) ;

	for (i = 'A' ; i <= 'Z' ; i += 1)
	    BACLR(terms,i) ;

	for (i = '0' ; i <= '9' ; i += 1)
	    BACLR(terms,i) ;

#if	CF_LATIN
	for (i = 0xC0 ; i < 0x100 ; i += 1) {
	    BACLR(terms,i) ;
	}
	BACLR(terms,0xB5) ;		/* the "micro" character (mu) */
#endif /* CF_LATIN */

/* notice! we allow a single quote (apostrophe) in order to get possessives */

	BACLR(terms,CH_SQUOTE) ;

#if	CF_COLON
	BACLR(terms,':') ;
#endif

	return rs ;
}
/* end subroutine (pinfo_mkterms) */


