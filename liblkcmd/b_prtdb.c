/* b_prtdb */

/* the front-end of the PRTDB program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LPGET	1		/* test LPGET */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This program was written from scratch to facilitate printer
	configuration (for PRT and PRTFMT).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is the front-end for a program that accesses several
        possible databases in order to find the value for specified keys. We use
        the PRINTER environment variable to find the default printer to lookup
        keys for when an explicit printer is not specified.


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
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<char.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<ids.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>
#include	<paramopt.h>
#include	<getax.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_prtdb.h"
#include	"defs.h"
#include	"pdb.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#ifndef	KBUFLEN
#define	KBUFLEN		MAXNAMELEN
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		MAXNAMELEN
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snwcpy(char *,int,cchar *,int) ;
extern int	snwcpyuc(char *,int,cchar *,int) ;
extern int	sncpy1(char *,int,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getprogpath(IDS *,vecstr *,char *,cchar *,int) ;
extern int	prgetprogpath(cchar *,char *,cchar *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		db:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PDB		db ;
	PROGINFO	*pip ;
	cchar		*printer ;
	cchar		*un ;		/* user name */
	cchar		*udname ;	/* sser home directory */
	cchar		*utilname ;
	cchar		*dbfname ;
	cchar		*prog_lpget ;
	char		unbuf[USERNAMELEN+1] ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,void *,cchar *,int) ;
static int	procspec(PROGINFO *,void *,cchar *,int) ;

static int	lpgetout(PROGINFO *,char *,int,cchar *,cchar *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_userinfo(LOCINFO *) ;
static int	locinfo_findlpget(LOCINFO *) ;
static int	locinfo_getprinter(LOCINFO *) ;


/* local variables */

static const char	*progmodes[] = {
	"prtdb",
	NULL
} ;

enum progmodes {
	progmode_prtdb,
	progmode_overlast
} ;

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"pm",
	"sn",
	"af",
	"ef",
	"of",
	"un",
	"db",
	"set",
	"option",
	"follow",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_un,
	argopt_db,
	argopt_set,
	argopt_option,
	argopt_follow,
	argopt_overlast
} ;

static const PIVARS	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const MAPEX	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_INVALID, EX_USAGE },
	{ 0, 0 }
} ;

static const uchar	aterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int b_prtdb(int argc,cchar *argv[],void *contextp)
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
/* end subroutine (b_prtdb) */


int p_prtdb(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_prtdb) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar **argv,cchar **envv,void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	PARAMOPT	aparams ;
	BITS		pargs ;
	SHIO		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs ;
	int		rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pm = NULL ;
	const char	*sn = NULL ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->lip = &li ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

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

/* do we have a keyword match or should we assume only key letters? */

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

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pm = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pm = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* search name */
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

/* utility name */
	                case argopt_un:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->utilname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->utilname = argp ;
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

/* the user specified some progopts */
	                case argopt_option:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            PARAMOPT	*pop = &aparams ;
	                            cchar	*po = PO_OPTION ;
	                            rs = paramopt_loads(pop,po,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* the user specified some progopts */
	                case argopt_set:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            PARAMOPT	*pop = &aparams ;
	                            rs = paramopt_loadu(pop,argp,argl) ;
				}
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* argument files */
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

/* printer database */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lip->dbfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                lip->dbfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

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

/* printer destination */
	                    case 'd':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    lip->printer  = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* username to use for DB query */
	                    case 'u':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            lip->un = argp ;
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

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	debugprintf("b_prtdb: while-out rs=%d\n",rs) ;
#endif

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(pip->efp,SHIO_CSETBUFLINE,TRUE) ;
	} else if (isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    shio_printf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get our program mode */

	if (pm == NULL) pm = pip->progname ;

	if ((pip->progmode = matstr(progmodes,pm,-1)) >= 0) {
	    if (pip->debuglevel > 0) {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: pm=%s (%u)\n" ;
	        shio_printf(pip->efp,fmt,pn,pm,pip->progmode) ;
	    }
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid program-mode (%s)\n" ;
	    shio_printf(pip->efp,fmt,pn,pm) ;
	    ex = EX_USAGE ;
	    rs = SR_INVALID ;
	}

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
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

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

/* check a few more things */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* check the utility */

	if (lip->utilname == NULL) lip->utilname = getourenv(envv,VARUTILNAME) ;
	if (lip->utilname == NULL) lip->utilname = DEFUTILITY ;

/* try to find a DB file */

	if (lip->dbfname == NULL) lip->dbfname = getourenv(envv,VARDBFNAME) ;
	if (lip->dbfname == NULL) lip->dbfname = PDBFNAME ;

	if (rs >= 0) {
	    if ((rs = locinfo_userinfo(lip)) >= 0) {
	        if ((rs = locinfo_findlpget(lip)) >= 0) {
	            rs = locinfo_getprinter(lip) ;
	        }
	    }
	}

/* get a default printer if necessary */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: printer=>%s<\n",lip->printer) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: printer=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,lip->printer) ;
	}

/* open the printer-default database */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: ud=%s\n",lip->udname) ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    PDB		*pdp = &lip->db ;
	    cchar	*pr = pip->pr ;
	    cchar	*dbfname = lip->dbfname ;
	    cchar	*utilname = lip->utilname ;
	    cchar	*ud = lip->udname ;
	    if ((rs = pdb_open(pdp,pr,ud,utilname,dbfname)) >= 0) {
	        lip->open.db = TRUE ;
	        {
	            ARGINFO	*aip = &ainfo ;
	            BITS	*bop = &pargs ;
	            cchar	*ofn = ofname ;
	            cchar	*afn = afname ;
	            rs = process(pip,aip,bop,ofn,afn) ;
	        }
	        lip->open.db = FALSE ;
	        rs1 = pdb_close(pdp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: inaccessible printer database (%d)\n" ;
	        ex = EX_UNAVAILABLE ;
	        shio_printf(pip->efp,fmt,pn,rs) ;
	    } /* end if (pdm) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    shio_close(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
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
/* end subroutine (main) */


static int usage(PROGINFO *pip)
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s <key(s)> [...] [-af {<afile>|-}]\n",
	    rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-u <username>] [-db <dbfile>]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-un <utility>] [-d <printer>]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	SHIO		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if ((ofn == NULL) || (ofn[0] == '\0')) ofn = BFILE_STDOUT ;
	if ((rs = shio_open(ofp,ofn,"wct",0644)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        cchar	**argv = aip->argv ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	            if (f) {
	                cp = argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procspec(pip,ofp,cp,-1) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end for (positional arguments) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        SHIO	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	        if ((rs = shio_open(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = shio_readline(&afile,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procspecs(pip,ofp,cp,cl) ;
	                        wlen += rs ;
	                    }
	                }

	                if (rs >= 0) rs = lib_sigterm() ;
	                if (rs >= 0) rs = lib_sigintr() ;
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = shio_close(&afile) ;
	            if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                fmt = "%s: inaccessible argument-list (%d)\n" ;
	                shio_printf(pip->efp,fmt,pn,rs) ;
	                shio_printf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0) && (afn == NULL)) {
	        shio_printf(pip->efp,"%s: no keys specified\n",pn) ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    fmt = "%s: ofn=%s\n" ;
	    shio_printf(pip->efp,fmt,pn,ofn) ;
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("b_prtdb/process: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procspecs(PROGINFO *pip,void *ofp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procspec(pip,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspecs) */


static int procspec(PROGINFO *pip,void *ofp,cchar *kp,int kl)
{
	LOCINFO		*lip = pip->lip ;
	const int	klen = KBUFLEN ;
	int		rs ;
	int		vl = 0 ;
	char		kbuf[KBUFLEN+1] ;

	if (kp == NULL) return SR_FAULT ;

	if ((rs = snwcpy(kbuf,klen,kp,kl)) >= 0) {
	    const int	rsn = SR_NOTFOUND ;
	    const int	vlen = VBUFLEN ;
	    const char	*pp ;
	    char	vbuf[VBUFLEN + 1] ;

#if	CF_DEBUG
	debugprintf("progkey: ent key=%s\n",kbuf) ;
#endif

/* access the front database (local and system) */

	if ((vl == 0) && lip->open.db) {
	    PDB		*pdp = &lip->db ;
	    cchar	*printer = lip->printer ;

	    pp = (strcmp(printer,DEFPRINTER) == 0) ? "default" : printer ;

#if	CF_DEBUG
	debugprintf("progkey: pdb-fetch printer=%s k=%s\n",pp,kbuf) ;
#endif

	    if ((rs = pdb_fetch(pdp,vbuf,vlen,pp,kbuf)) >= 0) {
	        vl = rs ;
	    } else if (rs == rsn) {
	        rs = SR_OK ;
	        vl = 0 ;
	    }

	} /* end if (PDB) */

/* access the backend database (system) */

#if	CF_LPGET
	if ((rs >= 0) && (vl == 0)) {
	    cchar	*pt = lip->printer ;
	    rs = lpgetout(pip,vbuf,vlen,pt,kbuf) ;
	    vl = rs ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        debugprintf("progkey: lpgetout() rs=%d\n",rs) ;
	        debugprintf("progkey: v=>%t<\n",vbuf,strlinelen(vbuf,vl,40)) ;
	    }
#endif
	}
#endif /* CF_LPGET */

/* print out result */

	   if (rs >= 0) {
	       rs = shio_printf(ofp,"%t\n",vbuf,vl) ;
	   }

	} /* end if (snwcpy) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progkey: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (procspec) */


static int lpgetout(PROGINFO *pip,char *vbuf,int vlen,cchar *pt,cchar *key)
{
	LOCINFO		*lip = pip->lip ;
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		cl ;
	int		ml ;
	int		i = 0 ;
	int		vl = 0 ;
	cchar		*cp ;
	cchar		*progfname ;
	cchar		*av[6] ;
	char		progname[MAXNAMELEN + 1] ;

	if (pt == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("lpgetout: printer=%s key=%s\n",pt,key) ;
#endif

	progfname = lip->prog_lpget ;
	if ((cl = sfbasename(progfname,-1,&cp)) > 0) {
	    ml = MIN(MAXNAMELEN,cl) ;
	    snwcpyuc(progname,MAXNAMELEN,cp,ml) ;
	} else {
	    snwcpyuc(progname,MAXNAMELEN,progfname,-1) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("lpgetout: progfname=%s\n",progfname) ;
	    debugprintf("lpgetout: progname=%s\n",progname) ;
	}
#endif

/* prepare arguments */

	av[i++] = progname ;
	av[i++] = "-k" ;
	av[i++] = key ;
	av[i++] = pt ;
	av[i] = NULL ;

	if ((rs = bopenprog(ofp,progfname,"r",av,pip->envv)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		f_gotit ;
	    int		f_first ;
	    int		len ;
	    const char	*tp ;
	    char	lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("lpgetout: opened-prog rs=%d\n",rs) ;
#endif

	    f_gotit = FALSE ;
	    f_first = TRUE ;
	    while ((rs = breadline(ofp,lbuf,llen)) > 0) {
	        len = rs ;

	        if ((tp = strnchr(lbuf,len,':')) == NULL)
	            continue ;

	        if (strncmp(lbuf,pt,(tp - lbuf)) == 0) {

	            cl = (lbuf + len - (tp + 1)) ;
	            cp = (tp + 1) ;
	            while ((cl > 0) && CHAR_ISWHITE(cp[cl - 1])) {
	                cl -= 1 ;
	            }

	            f_gotit = TRUE ;
	            break ;
	        }

	    } /* end while (readling lines) */

	    if ((rs >= 0) && f_gotit) {

	        while (f_first ||
	            ((rs = breadline(ofp,lbuf,llen)) > 0)) {

	            if (! f_first) {

	                len = rs ;
	                if (lbuf[len - 1] == '\n')
	                    len -= 1 ;

	                cl = sfshrink(lbuf,len,&cp) ;

	            } else {
	                f_first = FALSE ;
	            }

	            tp = strnpbrk(cp,cl,"=-") ;

	            if ((tp != NULL) && (tp[0] == '=')) {

	                vl = (cp + cl) - (tp + 1) ;

	                rs = (vl <= vlen) ? SR_OK : SR_OVERFLOW ;

	                if (rs >= 0)
	                    rs = strwcpy(vbuf,(tp + 1),MIN(vlen,vl)) - vbuf ;

	                break ;
	            }

	        } /* end while */

	        if (rs == 0) rs = SR_NOTFOUND ;

	    } /* end if (extracting value) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (reading child output) */

#if	CF_DEBUG
	debugprintf("lpgetout: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (lpgetout) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

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


static int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
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


static int locinfo_userinfo(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		hl = 0 ;
	if (lip->un != NULL) {
	    const int	hlen = MAXPATHLEN ;
	    char	hbuf[MAXPATHLEN+1] ;
	    if ((rs = getuserhome(hbuf,hlen,lip->un)) >= 0) {
	        cchar	**vpp = &lip->udname ;
	        rs = locinfo_setentry(lip,vpp,hbuf,rs) ;
	        hl = rs ;
	    }
	} else {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    char		*pwbuf ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
	            const int	unlen = USERNAMELEN ;
	            if ((rs = sncpy1(lip->unbuf,unlen,pw.pw_name)) >= 0) {
	                cchar	**vpp = &lip->udname ;
	                rs = locinfo_setentry(lip,vpp,pw.pw_dir,-1) ;
	                hl = rs ;
	            }
	        } /* end if (getpwusername) */
	        uc_free(pwbuf) ;
	    } /* end if (m-a-f) */
	}
	return (rs >= 0) ? hl : rs ;
}
/* end subroutine (locinfo_userinfo) */


/* get the path to the LPGET program */
static int locinfo_findlpget(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->prog_lpget == NULL) {
	    cchar	*cp ;
	    if ((cp = getourenv(pip->envv,VARLPGET)) != NULL) {
	        cchar	**vpp = &lip->prog_lpget ;
	        rs = locinfo_setentry(lip,vpp,cp,-1) ;
	    } else {
	        cchar	*pr = pip->pr ;
	        cchar	*pn = PROG_LPGET ;
	        char	tbuf[MAXPATHLEN+1] ;
	        if ((rs = prgetprogpath(pr,tbuf,pn,-1)) >= 0) {
	            cchar	**vpp = &lip->prog_lpget ;
	            rs = locinfo_setentry(lip,vpp,tbuf,rs) ;
	        }
	    }
	}
	return rs ;
}
/* end subroutine (locinfo_findlpget) */


static int locinfo_getprinter(LOCINFO *lip)
{
	PROGINFO	*pip = lip->pip ;
	int		rs = SR_OK ;
	if (lip->printer == NULL) {
	    lip->printer = getourenv(pip->envv,VARPRINTER) ;
	}
	if (lip->printer == NULL) {
	    lip->printer = getourenv(pip->envv,VARLPDEST) ;
	}
	if (lip->printer == NULL) {
	    PROGINFO	*pip = lip->pip ;
	    const int	vlen = VBUFLEN ;
	    cchar	*def = DEFPRINTER ;
	    cchar	*key = DEFPRINTERKEY ;
	    char	vbuf[VBUFLEN+1] ;
	    if ((rs = lpgetout(pip,vbuf,vlen,def,key)) >= 0) {
	        cchar	**vpp = &lip->printer ;
	        rs = locinfo_setentry(lip,vpp,vbuf,rs) ;
	    }
	} /* end if (trying to get default printer) */
	return rs ;
}
/* end subroutine (locinfo_getprinter) */


