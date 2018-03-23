/* main (passwdv) */
/* lang=C++11 */

/* main module for the 'passwdv' program */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the front-end subroutine for the PASSWDV (password verification)
        program.


*******************************************************************************/

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<termios.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<paramopt.h>
#include	<bits.h>
#include	<bfile.h>
#include	<ids.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"pwfile.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern "C" int	sfshrink(cchar *,int,cchar **) ;
extern "C" int	matstr(cchar **,cchar *,int) ;
extern "C" int	matostr(cchar **,int,cchar *,int) ;
extern "C" int	cfdeci(cchar *,int,int *) ;
extern "C" int	optbool(cchar *,int) ;
extern "C" int	optvalue(cchar *,int) ;
extern "C" int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern "C" int	sperm(IDS *,struct ustat *,int) ;
extern "C" int	isinteractive() ;
extern "C" int	isdigitlatin(int) ;
extern "C" int	isNotPresent(int) ;
extern "C" int	isFailOpen(int) ;

extern "C" int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern "C" int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;
extern "C" int	process(PROGINFO *,PWFILE *,bfile *,cchar *) ;

extern "C" cchar	*getourenv(cchar **,cchar *) ;


/* external variables */


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		pf:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PWFILE		pf ;
	PROGINFO	*pip ;
	bfile		*ofp ;
	char		ubuf[USERNAMELEN+1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procgather(PROGINFO *,PARAMOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procone(PROGINFO *,cchar *) ;

static int	procout_begin(PROGINFO *,bfile *,cchar *) ;
static int	procout_end(PROGINFO *,bfile *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
static int	locinfo_pwbegin(LOCINFO *,cchar *) ;
static int	locinfo_pwend(LOCINFO *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"option",
	"af",
	"ef",
	"of",
	"pf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_option,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_pf,
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
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;

static const char	*aknopts[] = {
	"seven",
	NULL
} ;

enum aknopts {
	aknopt_seven,
	aknopt_overlast
} ;

static const char	*po_option = PO_OPTION ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	PARAMOPT	aparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_passed = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*pwfname = NULL ;
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

/* early things to initialize */

	pip->verboselevel = 1 ;

/* process program arguments */

	pip->lip = lip ;
	if (rs >= 0) rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = paramopt_start(&aparams) ;
	pip->f.aparams = (rs >= 0) ;

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

/* program-root */
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

/* the user specified some options */
	                case argopt_option:
	                    cp = NULL ;
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
	                        PARAMOPT	*pop = &aparams ;
	                        cchar		*po = po_option ;
	                        rs = paramopt_loads(pop,po,cp,cl) ;
	                    }
	                    break ;

/* search name */
	                case argopt_sn:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    } else
	                        rs = SR_INVALID ;
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

/* output file-name */
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

/* PASSWD file */
	                case argopt_pf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pwfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pwfname = argp ;
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

/* alternate PASSWD file */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pwfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* update */
	                    case 'u':
	                        pip->f.update = TRUE ;
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

	        } /* end if (had an argument-option) */

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
	    bfile	*efp = (bfile *) pip->efp ;
	    bprintf(efp,"%s: version %s\n",pip->progname,VERSION) ;
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
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {
	    bfile	*efp = (bfile *) pip->efp ;
	    bprintf(efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* options? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: sevenbit=%d\n",pip->f.sevenbit) ;
#endif

	pfp = NULL ;
	if (pwfname != NULL) {

	    rs = perm(pwfname,-1,-1,NULL,R_OK) ;

	    if (rs >= 0)
	        rs = pwfile_open(&pf,pwfname) ;

	    if (rs < 0) {
	        bfile	*efp = (bfile *) pip->efp ;
	        ex = EX_SOFTWARE ;
	        bprintf(efp,"%s: could not open PASSWORD file (%d)\n",
	            pip->progname,rs) ;
	        goto badpwfile ;
	    }

	    pfp = &pf ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	            PWFILE_CUR	cur ;
	            PWFILE_ENT	pe ;
	    const int	pwlen = PWFILE_RECLEN ;
	            char	pwbuf[PWFILE_RECLEN + 1] ;
		cchar	*fmt = "main: opened PWFILE >%s< rs=%d\n" ;
	        debugprintf(fmt,pwfname,rs) ;
	            pwfile_curbegin(pfp,&cur) ;
	            debugprintf("main: &pe %08lX pwbuf %08lX\n",&pe,pwbuf) ;
	            while (pwfile_enum(pfp,&cur,
	                &pe,pwbuf,pwlen) >= 0) {
	                debugprintf("username=%s\n",pe.username) ;
	                debugprintf("password=%s\n",pe.password) ;
	                debugprintf("gecos=>%s<\n",pe.gecos) ;
	                debugprintf("realname=>%s<\n",pe.realname) ;
	                debugprintf("account=%s\n",pe.account) ;
	                debugprintf("office=>%s<\n",pe.office) ;
	                debugprintf("hphone=>%s<\n",pe.hphone) ;
	            } /* end while */
	            pwfile_curend(pfp,&cur) ;
	    } /* end if */
#endif /* CF_DEBUG */

	} /* end if (specified password file) */

/* get ready */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    if ((rs = isinteractive()) > 0) {
	        if ((rs = procgather(pip,&aparams)) >= 0) {
		    bfile	ofile ;
		    cchar	*ofn = ofname ;
		    if ((rs = procout_begin(pip,&ofile,ofn)) >= 0) {
		        {
			    ARGINFO	*aip = &ainfo ;
			    BITS	*bop = &pargs ;
			    rs = procargs(pip,aip,bop,afname) ;
		        }
		        rs1 = procout_end(pip,&ofile) ;
		        if (rs >= 0) rs = rs1 ;
		    } else {
	                ex = EX_CANTCREAT ;
		        fmt = "%s: output unavailable (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: ofile=%s\n",pn,ofname) ;
		    } /* end if (procout) */
	        } else if (rs == 0) {
	            fmt = "%s: program must be executed interactively\n" ;
	            rs = SR_INVALID ;
	            ex = EX_DATAERR ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	        }
	    } /* end if (procgather) */
	} else if (ex == EX_OK) {
	    bfile	*efp = (bfile *) pip->efp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    bprintf(efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

/* finished */

	if (rs >= 0) {
	    ex = (f_passed) ? EX_OK : EX_DATAERR ;
	} else {
	    ex = EX_DATAERR ;
	}

/* we are out of here */
retearly:
	if (pip->debuglevel > 0) {
	    bfile	*efp = (bfile *) pip->efp ;
	    bprintf(efp,"%s: exiting ex=%u (%d)\n",pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    bfile	*efp = (bfile *) pip->efp ;
	    pip->open.errfile = FALSE ;
	    bclose(efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
	}

	bits_finish(&pargs) ;

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


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<user(s)> ...] [-u] [-pf <passwdfile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procgather(PROGINFO *pip,PARAMOPT *pop)
{
	PARAMOPT_CUR	cur ;
	int		rs ;
	int		rs1 ;
	cchar		*po = PO_OPTION ;
	if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {
	    int		i ;
	    cchar	*cp ;
	    while ((rs1 = paramopt_enumvalues(pop,po,&cur,&cp)) >= 0) {
	            if (cp != NULL) {
	                if ((i = matostr(aknopts,2,cp,-1)) >= 0) {
	                    switch (i) {
	                    case aknopt_seven:
	                        pip->f.sevenbit = TRUE ;
	                        break ;
	                    } /* end switch */
		        }
		    }
	    } /* end while (option processing) */
	    if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;
	    rs1 = paramopt_curend(pop,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramopt-cur) */
	return rs ;
} 
/* end subroutine (procgather) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*cp ;
	cchar		*fmt ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {
	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
		    if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = procone(pip,cp) ;
		    }
	        }
	        if (rs < 0) break ;
	    } /* end for (looping through arguments) */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

		    if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
			if (cp[0] != '#') {
			    lbuf[(cp+cl)-lbuf] = '\0' ;
	            	    pan += 1 ;
	            	    rs = procone(pip,cp) ;
			}
		   } /* end if (sfshrink) */

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
		    fmt = "%s: argument list-file unavailable (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list */

	if ((rs >= 0) && (pan == 0)) {
	    cp = "-" ;
	    rs = procone(pip,cp) ;
	} /* end if */

	return rs ;
}
/* end subroutine (procargs) */


static int procone(PROGINFO *pip,cchar *un)
{
	int		rs ;
	if ((rs = process(pip,un)) >= 0) {
	    int	f_passed = (rs > 0) ;
	    if ((rs >= 0) && (! pip->f.nooutput)) {
	    rs = bprintf(ofp,"%svalid\n",(! f_passed) ? "in" : "") ;
	    }
	}
	return rs ;
}
/* end subroutine (procone) */


static int procout_begin(PROGINFO *pip,cchar *ofn)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	if (! pip->f.nooutput) {
	    const int	oflen = sizeof(bfile) ;
	    bfile	*ofbuf ;
	    if ((rs = uc_malloc(oflen+1),&ofbuf)) >= 0) {
	        if ((ofn == NULL) || (ofn[0] == '\0')) ofn = BFILE_STDOUT ;
	        if ((rs = bopen(ofbuf,ofn,"dwct",0664)) >= 0) {
		    lip->ofp = ofbuf ;
	        }
		if (rs < 0) {
		    uc_free(ofbuf) ;
		    pip->ofp = NULL ;
		}
	    } /* end if (m-a) */
	}
	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip)
{
	LOCINFO		*lip = (LOCINFO *) pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	if ((! pip->f.nooutput) && (lip->ofp != NULL)) {
	    rs1 = bclose(lip->ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = uc_free(lip->ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->ofp = NULL ;
	}
	return rs ;
}
/* end subroutine (procout_end) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (lip == NULL) return SR_FAULT ;

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

#if	CF_PERCACHE /* register |ourfini()| for mod-unload */
	if (lip->f.percache) {
	    if ((rs1 = percache_finireg(&pc)) > 0) { /* need registration? */
	        rs1 = uc_atexit(ourfini) ;
	    }
	    if (rs >= 0) rs = rs1 ;
	}
#endif /* CF_PERCACHE */

	if ((lip->fname != NULL) && lip->f.allocfname) {
	    rs1 = uc_free(lip->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    lip->fname = NULL ;
	    lip->f.allocfname = FALSE ;
	}

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


static int locinfo_pwbegin(LOCINFO *lip,cchar *pfn)
{
	int		rs = SR_OK ;

	return rs ;
}
/* end subroutine (locinfo_pwbegin) */


static int locinfo_pwend(LOCINFO *lip)
{
	int		rs = SR_OK ;

	return rs ;
}
/* end subroutine (locinfo_pwend) */


