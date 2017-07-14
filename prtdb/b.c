/* main */

/* the front-end of the PRTDB program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_GETPROGPATH	1		/* use 'getprogpath(3dam)' */
#define	CF_LPGET	1		/* test LPGET */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is the front-end for a program that accesses several
        possible databases in order to find the value for specified keys. We use
        the PRINTER environment variable to find the default printer to lookup
        keys for when an explicit printer is not specified.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>

#include	<vsystem.h>
#include	<getbufsize.h>
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

#include	"config.h"
#include	"defs.h"
#include	"pdb.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#ifndef	PRINTERLEN
#define	PRINTERLEN	MAXNAMELEN
#endif

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	MAXNAMELEN
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		MAXNAMELEN
#endif

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
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
extern int	getprogpath(IDS *,vecstr *,char *,const char *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	prgetprogpath(const char *,char *,const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

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


/* forward references */

static int	usage(PROGINFO *) ;

static int process(PROGIFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int procspecs(PROGINFO *,void *,cchar *,int) ;
static int procspec(PROGINFO *,void *,cchar *,int) ;

static int	loadpath(PROGINFO *,vecstr *,const char *) ;
extern int	lpgetout(PROGINFO *,cchar *,char *,int,cchar *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_userinfo(LOCINFO *) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		pdb:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	pdb		pdb ;
	PROGINFO	*pip ;
	cchar		*printer ;
	cchar		*un ;		/* user name */
	cchar		*udname ;	/* sser home directory */
	cchar		*utilname ;
	cchar		*dbfname ;
	char		unbuf[USERNAMELEN+1] ;
} ;


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

static const char	*progopts[] = {
	"follow",
	"nofollow",
	NULL
} ;

enum progopts {
	progopt_follow,
	progopt_nofollow,
	progopt_overlast
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
	{ SR_ACCESS, EX_ACCESS },
	{ SR_REMOTE, EX_FORWARDED },
	{ SR_NOSPC, EX_NOSPACE },
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


/* ARGSUSED */
static int mainsub(int argc,cchar **argv,cchar **envv,void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	PARAMOPT	aparams ;
	BITS		pargs ;
	bfile		errfile ;

	const int	hlen = MAXPATHLEN ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs, rs1 ;
	int		opts ;
	int		cl ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*plpget = PROG_LPGET ;
	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pmspec = NULL ;
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

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	ids_load(&pip->ids) ;

/* early things to initialize */

	pip->ofp = ofp ;

	pip->progmode = -1 ;
	pip->verboselevel = 1 ;

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
	                    if (argl)
	                        rs = paramopt_loadu(&aparams,argp,argl) ;
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

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
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

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif

	if (pip->progmode < 0)
	    pip->progmode = progmode_prtdb ;

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* check if we have a username for the query */


/* check the utility */

	if (lip->utilname == NULL) lip->utilname = getenv(VARUTILNAME) ;
	if (lip->utilname == NULL) lip->utilname = DEFUTILITY ;

/* try to find a DB file */

	if (lip->dbfname == NULL) lip->dbfname = getenv(VARDBFNAME) ;
	if (lip->dbfname == NULL) lip->dbfname = PDBFNAME ;

/* get the path to the LPGET program */

	if ((rs >= 0) && (pip->prog_lpget == NULL)) {
	    if ((cp = getenv(VARLPGET)) != NULL) {
		cchar	**vpp = &pip->prog_lpget ;
	        rs = proginfo_setentry(pip,vpp,cp,-1) ;
	    }
	}

	if ((rs >= 0) && (pip->prog_lpget == NULL)) {

	    opts = VECSTR_OORDERED ;
	    rs = vecstr_start(&pip->path,10,opts) ;
	    pip->f.path = (rs >= 0) ? TRUE : FALSE ;
	    if (rs < 0)
	        goto badinitpath ;

	    if ((cp = getenv(VARPATH)) != NULL)
	        rs = loadpath(pip,&pip->path,cp) ;

	    cp = tmpfname ;
	    mkpath2(tmpfname,pip->pr,"bin") ;

	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

	    cp = tmpfname ;
	    mkpath2(tmpfname,pip->pr,"sbin") ;

	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

	    cp = "/usr/bin" ;
	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

	    cp = "/usr/sbin" ;
	    if (vecstr_find(&pip->path,cp) < 0)
	        vecstr_add(&pip->path,cp,-1) ;

#if	CF_GETPROGPATH
	    rs = getprogpath(&pip->ids,&pip->path,tmpfname,plpget,-1) ;
#else
	    rs = prgetprogpath(pip->pr,tmpfname,plpget,-1) ;
#endif /* CF_GETPROGPATH */

	    cl = rs ;
	    if (rs >= 0)
	        proginfo_setentry(pip,&pip->prog_lpget,tmpfname,cl) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: getprogpath() rs=%d\n",rs) ;
	        debugprintf("main: prog_lpget=%s\n",pip->prog_lpget) ;
	    }
#endif

	} /* end if (trying to find a LPGET program) */

	if ((rs >= 0) && (pip->prog_lpget == NULL))
	    rs = SR_NOENT ;

	if (rs >= 0)
	    rs = u_stat(pip->prog_lpget,&sb) ;

	if (rs < 0) {
	    ex = EX_NOPROG ;
	    bprintf(pip->efp,"%s: LPGET program not found (%d)\n",
	        pip->progname,rs) ;
	    goto badnoprog ;
	}

/* get a default printer if necessary */

	if (printer[0] == '\0') {

	    if ((cp = getenv(VARPRINTER)) != NULL) {
	        rs = sncpy1(printer,PRINTERLEN,cp) ;
	    } else if ((cp = getenv(VARLPDEST)) != NULL) {
	        rs = sncpy1(printer,PRINTERLEN,cp) ;
	    }

	} /* end if */

	if ((rs >= 0) && (printer[0] == '\0')) {

	    rs = lpgetout(pip,DEFPRINTER,printer,PRINTERLEN,DEFPRINTERKEY) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: lpgetout() rs=%d\n",rs) ;
	        debugprintf("main: lpgetout() printer=%s\n",printer) ;
	    }
#endif

	    if (rs < 0)
	        printer[0] = '\0' ;

	} /* end if (trying to get default printer) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: printer=>%s<\n",printer) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: printer=%s\n",pip->progname,printer) ;
	}

/* open the printer-default database */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: ur=%s\n",lip->ur) ;
#endif

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    USTAT	sb ;
	    if ((rs = pdb_open(&pip->db,pip->pr,ur,utilname,dbfname)) >= 0) {
		pip->f.pdbopen = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: pdb_open() rs=%d\n",rs) ;
#endif

	if (pip->debuglevel > 0) {
	    cp = (rs >= 0) ? "" : "not " ;
	    bprintf(pip->efp,"%s: default PRT DB %sfound\n",
	        pip->progname,cp) ;
	}


		rs = process(pip,aip,ofn,afn) ;



badopenout:
	if (pip->f.pdbopen) {
	    pip->f.pdbopen = FALSE ;
	    pdb_close(&pip->db) ;
	}

	} /* end if */

badnoprog:
	if (pip->f.path) {
	    pip->f.path = FALSE ;
	    vecstr_finish(&pip->path) ;
	}

done:
badinitpath:
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
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
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

	ids_release(&pip->ids) ;

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s <key(s)> [...] [-af {<afile>|-}]\n",
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-u <username>] [-db <dbfile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-un <utility>] [-d <printer>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	if (ofn == NULL) || (ofn[0] == '\0')) ofn = BFILE_STDOUT ;
	if ((rs = bopen(ofp,ofn,"wct",0644)) >= 0) {
	    int	pan = 0 ;
	int		cl ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	for (ai = 1 ; ai < aip->argc ; ai += 1) {

	    f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
		if (cp[0] != '\0') {
	    	    pan += 1 ;
	    	    rs = progkey(pip,pip->printer,cp,-1) ;
		}
	    }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	    if (rs < 0) break ;
	} /* end for (positional arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int	len ;
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(&afile,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procspecs(pip,ofp,cp,cl) ;
	                        c += rs ;
	                    }
	                }

	            if (rs >= 0) rs = lib_sigterm() ;
	            if (rs >= 0) rs = lib_sigintr() ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(&afile) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0) && (afn == NULL)) {
	    bprintf(pip->efp,"%s: no keys specified\n",pn) ;
	}

		rs1 = bclose(ofp) ;
		if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    fmt = "%s: ofn=%s\n" ;
	    bprintf(pip->efp,fmt,pn,ofn) ;
	} /* end if */

	if ((rs > 0) && (pip->debuglevel > 0)) {
	    bprintf(pip->efp,"%s: keys processed=%u\n",pn,pan) ;
	}


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
	const int	vlen = VBUFLEN ;
	int		rs ;
	int		vl = 0 ;
	const char	*pp ;
	char		keyname[KEYBUFLEN+1] ;
	char		vbuf[VBUFLEN + 1] ;

	if (printer == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	rs = snwcpy(keyname,KEYBUFLEN,kp,kl) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUG
	debugprintf("progkey: printer=%s keyname=%s\n",printer,keyname) ;
#endif

/* access the front database (local and system) */

	if ((vl == 0) && pip->f.pdbopen) {

	    pp = (strcmp(printer,DEFPRINTER) == 0) ? "default" : printer ;

	    rs = pdb_fetch(&pip->db,pp,keyname,vbuf,vlen) ;
	    vl = rs ;

#if	CF_DEBUG
	debugprintf("progkey: pdb_fetch() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTFOUND) {
		rs = SR_OK ;
		vl = 0 ;
	    }

	} /* end if (PDB) */

/* access the backend database (system) */

#if	CF_LPGET
	if ((rs >= 0) && (vl == 0)) {

	    pp = (strcmp(printer,"default") == 0) ? DEFPRINTER : printer ;

	    rs = lpgetout(pip,pp,vbuf,vlen,keyname) ;
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
	    rs = bprintf(pip->ofp,"%t\n",vbuf,vl) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progkey: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (procspec) */


static int loadpath(PROGINFO *pip,vecstr *lp,cchar *pp)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		pathlen ;
	int		c = 0 ;
	const char	*tp ;
	char		pathdname[MAXPATHLEN + 1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	    debugprintf("main/loadpath: entered\n") ;
#endif

	while ((tp = strchr(pp,':')) != NULL) {

	    pathlen = pathclean(pathdname,pp,(tp - pp)) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main/loadpath: pathname=%t\n",pathdname,pathlen) ;
#endif

	    if ((rs = vecstr_findn(lp,pathdname,pathlen)) == rsn) {
	        c += 1 ;
	        rs = vecstr_add(lp,pathdname,pathlen) ;
	    }

	    pp = (tp + 1) ;
	    if (rs < 0) break ;
	} /* end while */

	if ((rs >= 0) && (pp[0] != '\0')) {

	    pathlen = pathclean(pathdname,pp,-1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("main/loadpath: pathname=%t\n",pathdname,pathlen) ;
#endif

	    if ((rs = vecstr_findn(lp,pathdname,pathlen)) == rsn) {
	        c += 1 ;
	        rs = vecstr_add(lp,pathdname,pathlen) ;
	    }

	} /* end if (trailing one) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadpath) */


static int lpgetout(PROGINFO *pip,cchar *printer,
		char *vbuf,int vlen,cchar *key)
{
	bfile		ofile, *ofp = &ofile  ;
	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;
	int	len, cl, ml ;
	int	vl = 0 ;
	int	f_gotit ;
	int	f_first ;

	const char	*progfname = pip->prog_lpget ;
	const char	*cp, *tp ;
	const char	*av[6] ;

	char	progname[MAXNAMELEN + 1] ;
	char	lbuf[LINEBUFLEN + 1] ;


	if (printer == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("lpgetout: printer=%s key=%s\n",printer,key) ;
	debugprintf("lpgetout: progfname=%s\n",progfname) ;
	}
#endif

	if ((cl = sfbasename(progfname,-1,&cp)) > 0) {
		ml = MIN(MAXNAMELEN,cl) ;
		strwcpy(progname,cp,ml) ;
	} else
		sncpy1(progname,MAXNAMELEN,progfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("lpgetout: progname=%s\n",progname) ;
#endif

/* prepare arguments */

	i = 0 ;
	av[i++] = progname ;
	av[i++] = "-k" ;
	av[i++] = key ;
	av[i++] = printer ;
	av[i] = NULL ;

	if ((rs = bopenprog(ofp,progfname,"r",av,pip->envv)) >= 0) {

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

	        if (strncmp(lbuf,printer,(tp - lbuf)) == 0) {

		    cl = (lbuf + len - (tp + 1)) ;
		    cp = (tp + 1) ;
			while ((cl > 0) && isspace(cp[cl - 1]))
				cl -= 1 ;

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

		    } else
			f_first = FALSE ;

	            tp = strnpbrk(cp,cl,"=-") ;

		    if ((tp != NULL) && (tp[0] == '=')) {

	                vl = (cp + cl) - (tp + 1) ;

	                rs = (vl <= vlen) ? SR_OK : SR_OVERFLOW ;

	                if (rs >= 0)
	                    rs = strwcpy(vbuf,(tp + 1),MIN(vlen,vl)) - vbuf ;

	                break ;
	            }

	        } /* end while */

		if (rs == 0)
			rs = SR_NOTFOUND ;

	    } /* end if (extracting value) */

	    bclose(ofp) ;
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
	lip->f.poll = TRUE ;

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


static int locinfo_userinfo(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		ul = 0 ;
	if (lip->un != NULL) {
	    const int	ulen = MAXPATHLEN ;
	    char	ubuf[MAXPATHLEN+1] ;
	    if ((rs = getuserhome(ubuf,ulen,-1)) >= 0) {
		cchar	**cpp = *lip->udname ;
		ul = rs ;
		rs = locinfo_setentry(lip,vpp,ubuf,rs) ;
	    }
	} else {
	    const int	pwlen = getbufsize(getbufsize_pw) ;
	    char	*pwbuf ;
	    if ((rs = uc_malloc((pwlen),&pwbuf)) >= 0) {
	        if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) {
		    const int	unlen = USERNAMELEN ;
		    if ((rs = sncpy1(lip->unbuf,unlen,pw.pw_name)) >= 0) {
			cchar	**cpp = *lip->udname ;
			rs = locinfo_setentry(lip,vpp,pw.pw_home,-1) ;
		    }
	        } /* end if (getpwusername) */
		uc_free(pwbuf) ;
	    } /* end if (m-a-f) */
	}
	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (locinfo_userinfo) */


