/* main */

/* program to exec a program with the named arg0 */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEBUGARGZ	0		/* debug ARGZ problems */
#define	CF_DEBUGENV	0		/* debug 'execve(2)' call */
#define	CF_DEBUGEXEC	0		/* debug 'execve(2)' call */
#define	CF_ISAEXEC	1		/* use 'isaexec(3c)' */
#define	CF_FINDXFILE	1		/* use 'getprogpath(3dam)' */
#define	CF_SOFAR	0		/* sofar() ? */
#define	CF_ISBADENV	0		/* use |isbadenv()| */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine is pretty much the whole little program here. This
        program really just allows for the changing of ARG[0] when a program is
        executed.

	Synopsis:

	$ execname <prog> [<arg0> [<arg1 ...>]]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<strpack.h>
#include	<dirlist.h>
#include	<ucmallreg.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"progreport.h"


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#ifndef	ENVBUFLEN
#define	ENVBUFLEN	(MAXPATHLEN + 40)
#endif

#undef	NDF
#define	NDF		"/tmp/execname.nd"

#define	PROGEXEC	struct progexec
#define	PROGEXEC_FL	struct progexec_flags


/* external subroutines */

extern int	snshellunder(char *,int,pid_t,cchar *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	strkeycmp(cchar *,cchar *) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matkeystr(cchar **,cchar *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	pathclean(char *,const char *,int) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,void *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	getprogpath(IDS *,VECSTR *,char *,const char *,int) ;
extern int	vecstr_adduniq(VECSTR *,const char *,int) ;
extern int	strpack_envstorer(STRPACK *,cchar *,int,cchar *,int,cchar **) ;
extern int	xfile(IDS *,cchar *) ;
extern int	hasprintbad(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugprinthex(cchar *,int,cchar *,int) ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct progexec_flags {
	uint		list:1 ;
	uint		store:1 ;
} ;

struct progexec {
	PROGEXEC_FL	f ;
	const char	*argz ;
	const char	*progfname ;
	const char	**av ;
	vechand		list ;
	strpack		store ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procexecbegin(PROGINFO *,ARGINFO *,const char *) ;
static int	procexecend(PROGINFO *) ;
static int	procexecbeginer(PROGINFO *,ARGINFO *,cchar *) ;
static int	procexecargz(PROGINFO *,ARGINFO *,const char *) ;
static int	procexecname(PROGINFO *,ARGINFO *,const char *) ;
static int	procexecgo(PROGINFO *,ARGINFO *) ;

static int	procexecvarspecials(PROGINFO *) ;
static int	procexecvarclean(PROGINFO *) ;
static int	procexecvarcleaner(PROGINFO *,const char *) ;
static int	procexecvarcleanmk(PROGINFO *,cchar *,int,cchar *,int) ;
static int	procexecaddenv(PROGINFO *,cchar *,int,cchar *,int) ;

static int	findxfile(IDS *,char *,const char *) ;

#if	CF_ISBADENV
static int	isbadenv(const char *) ;
static int	hasourbad(const char *,int) ;
#endif

#if	CF_DEBUGS && CF_SOFAR
static int	sofar(const char *,int) ;
#endif


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"caf",
	"sn",
	"af",
	"ef",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_option,
	argopt_caf,
	argopt_sn,
	argopt_af,
	argopt_ef,
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

static cchar	*envpaths[] = {
	"PATH",
	"LD_LIBRARY_PATH",
	"MANPATH",
	"CDPATH",
	"FPATH",
	"INCPATH",
	"TESTPATH",
	NULL
} ;

static const char	*envbads[] = {
	"_EF",
	"_A0",
	"_",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		npa = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_help = FALSE ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_exitargs = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*efname = NULL ;
	const char	*program = NULL ;
	const char	*cp ;


#if	CF_DEBUGARGZ
	nprintf(NDF,"main: argz=>%t<\n",
	    argv[0],strlinelen(argv[0],-1,40)) ;
#endif

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

/* initialize */

	if (rs >= 0) {
	    if ((cp = getourenv(envv,VARDEBUGLEVEL)) != NULL) {
	    	rs = optvalue(cp,-1) ;
		pip->debuglevel = rs ;
	    }
	}

	pip->verboselevel = 1 ;
	pip->pagesize = getpagesize() ;

/* continue with invocation arguments */

	npa = 0 ;			/* number of positional so far */
	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((! f_exitargs) && (rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
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

/* default action and user specified help */
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

#ifdef	COMMENT

/* alternate effective group */
	                    case 'g':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                egroup = argp ;
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
#endif /* COMMENT */

#ifdef	COMMENT
/* alternate effective user */
	                    case 'u':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                euser = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;
#endif /* COMMENT */

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

	        } /* end if (digits or options) */

	    } else {

	        switch (npa) {
	        case 0:
	            f_exitargs = TRUE ;
	            if (argl > 0) program = argp ;
	            break ;
	        default:
	            rs = SR_NOANODE ;
	            break ;
	        } /* end switch */
	        npa += 1 ;

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

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%d\n",pip->debuglevel) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: pr=%s\n",pip->pr) ;
	    debugprintf("main: sn=%s\n",pip->searchname) ;
	}
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* get into it */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: argr=%d\n",argr) ;
	    debugprintf("main: ai=%d\n",ai) ;
	    debugprintf("main: ai_pos=%d\n",ai_pos) ;
	    debugprintf("main: program=%s\n",program) ;
	}
#endif

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (program == NULL) {
	    bprintf(pip->efp,"%s: no program specified\n",pip->progname) ;
	    ex = EX_NOPROG ;
	    rs = SR_INVALID ;
	}

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.argr = argr ;
	ainfo.argv = argv ;
	ainfo.ai = ai ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procexecbegin(pip,&ainfo,program)) >= 0) {
	        if ((rs = procexecvarspecials(pip)) >= 0) {
	            if ((rs = procexecvarclean(pip)) >= 0) {
	                rs = procexecgo(pip,&ainfo) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("main: procexecgo() rs=%d\n",rs) ;
#endif
	            }
	        }
	        rs1 = procexecend(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procexec) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: out rs=%d\n",rs) ;
#endif

	ex = EX_NOEXEC ;

/* done */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: done ex=%u (%d)\n",ex,rs) ;
#endif

	if (rs < 0) {
	    mkreport(pip,argc,argv,rs) ;
	}

	if ((rs < 0) && (ex == EX_OK)) {
	    if (! pip->f.quiet) {
	        bprintf(pip->efp,"%s: could not process (%d)\n",
	            pip->progname,rs) ;
	    }
	    ex = mapex(mapexs,rs) ;
	} else {
	    if (rs == SR_NOENT) {
	        bprintf(pip->efp,
	            "%s: program not found (%d)\n",
	            pip->progname,rs) ;
	    } else {
	        bprintf(pip->efp,
	            "%s: program not executable (%d)\n",
	            pip->progname,rs) ;
	    }
	} /* end if (exitcode) */

retearly:
	bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

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

/* bad stuff comes here */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,
	    "%s: bad argument specified (%d)\n",
	    pip->progname,rs) ;
	mkreport(pip,argc,argv,rs) ;
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

	fmt = "%s: USAGE> %s <progpath> [{+|-|<arg0>} [<arg(s)> ...]]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procexecbegin(PROGINFO *pip,ARGINFO *aip,cchar *program)
{
	const int	size = sizeof(PROGEXEC) ;
	int		rs ;
	void		*p ;

	if (pip->progexec != NULL) return SR_NOANODE ;

	if ((rs = uc_malloc(size,&p)) >= 0) {
	    pip->progexec = p ;
	    rs = procexecbeginer(pip,aip,program) ;
	    if (rs < 0) {
	        uc_free(p) ;
	        pip->progexec = NULL ;
	    }
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (procexecbegin) */


static int procexecend(PROGINFO *pip)
{
	PROGEXEC	*pep ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip == NULL) return SR_FAULT ;
	if (pip->progexec == NULL) return SR_NOTOPEN ;

	pep = pip->progexec ;

	if (pep->av != NULL) {
	    rs1 = uc_free(pep->av) ;
	    if (rs >= 0) rs = rs1 ;
	    pep->av = NULL ;
	}

	if (pep->f.list) {
	    pep->f.list = FALSE ;
	    rs1 = vechand_finish(&pep->list) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (pep->f.store) {
	    pep->f.store = FALSE ;
	    rs1 = strpack_finish(&pep->store) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = uc_free(pip->progexec) ;
	if (rs >= 0) rs = rs1 ;
	pip->progexec = NULL ;

	return rs ;
}
/* end subroutine (procexecend) */


static int procexecargz(PROGINFO *pip,ARGINFO *aip,cchar *program)
{
	PROGEXEC	*pep = pip->progexec ;
	int		rs = SR_OK ;
	int		cl = 0 ;
	const char	*cp = NULL ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procexecargz: argr=%d\n",aip->argr) ;
	    debugprintf("main/procexecargz: program=%s\n",program) ;
	}
#endif

	if (aip->argr > 0) {
	    int		argl ;
	    cchar	*argp ;
	    aip->ai += 1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procexecargz: ai=%u\n",aip->ai) ;
#endif

	    argp = aip->argv[aip->ai] ;
	    argl = strlen(argp) ;
	    aip->argr -= 1 ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procexecargz: argp=>%s< argl=%u\n",
	            argp,argl) ;
#endif

	    cl = sfbasename(program,-1,&cp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procexecargz: c=>%t<\n",cp,cl) ;
#endif

	    if (strcmp(argp,"-") == 0) {
	        const int	size = (cl + 2) ;
	        char		*bp ;
	        if ((rs = uc_malloc(size,&bp)) >= 0) {
	            cchar	**vpp = &pep->argz ;
	            sncpy2(bp,(cl + 1),"-",cp) ;
	            rs = proginfo_setentry(pip,vpp,bp,(cl+1)) ;
	            uc_free(bp) ;
	        } /* end if (memory-allocation) */
	    } else if (strcmp(argp,"+") == 0) {
	        pep->argz = cp ;
	    } else if (argp[0] != '\0') {
	        pep->argz = argp ;
	    }

	} /* end if (had additional argument) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procexecargz: mid2 rs=%d az=%s\n",rs,pep->argz) ;
	}
#endif

	if (rs >= 0) {
	    if (pep->argz == NULL) {
	            if (program != NULL) {
		        cl = sfbasename(program,-1,&cp) ;
	                pep->argz = cp ;
		    }
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procexecargz: argz=>%s<\n",pep->argz) ;
	    debugprintf("main/procexecargz: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (procexecargz) */


static int procexecbeginer(PROGINFO *pip,ARGINFO *aip,cchar *program)
{
	PROGEXEC	*pep = pip->progexec ;
	const int	ssize = 1024 ;
	const int	vn = 150 ;
	const int	vo = VECHAND_OORDERED ;
	int		rs ;

	memset(pep,0,sizeof(PROGEXEC)) ;

	if ((rs = strpack_start(&pep->store,ssize)) >= 0) {
	    pep->f.store = TRUE ;
	    if ((rs = vechand_start(&pep->list,vn,vo)) >= 0) {
	        pep->f.list = TRUE ;
	        if ((rs = procexecargz(pip,aip,program)) >= 0) {
	            rs = procexecname(pip,aip,program) ;
	        }
	        if (rs < 0) {
	            pep->f.list = FALSE ;
	            vechand_finish(&pep->list) ;
	        }
	    } /* end if (vechand-start) */
	    if (rs < 0) {
	        pep->f.store = FALSE ;
	        strpack_finish(&pep->store) ;
	    }
	} /* end if (strpack-start) */

	return rs ;
}
/* end subroutine (procexecbeginer) */


static int procexecname(PROGINFO *pip,ARGINFO *aip,cchar *program)
{
	PROGEXEC	*pep = pip->progexec ;
	IDS		id ;
	int		rs ;
	int		rs1 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*progfname = program ;

	if (pip->debuglevel > 0) {
	    fmt = "%s: program=%s\n" ;
	    bprintf(pip->efp,fmt,pn,program) ;
	}

	if ((rs = ids_load(&id)) >= 0) {
	    char	tbuf[MAXPATHLEN+1] ;

	    if (program[0] != '/') {

	        if (strchr(program,'/') == NULL) {
	            progfname = tbuf ;

#if	CF_FINDXFILE
	            rs = findxfile(&id,tbuf,program) ;
#else
	            rs = findfilepath(NULL,tbuf,program,X_OK) ;
	            if (rs == 0)
	                rs = mkpath1(tbuf,program) ;
#endif /* CF_FINDXFILE */

	        } else {

	            if ((rs = proginfo_pwd(pip)) >= 0) {
	                progfname = tbuf ;
	                if ((rs = mkpath2(tbuf,pip->pwd,program)) >= 0) {
	                    rs = xfile(&id,program) ;
	                }
	            }

	        } /* end if */

	    } else {

	        if ((rs = mkpath1(tbuf,program)) >= 0) {
	            rs = xfile(&id,program) ;
	        }

	    } /* end if */

	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

	if (rs >= 0) {
	    cchar	**vpp = &pep->progfname ;
	    rs = proginfo_setentry(pip,vpp,progfname,-1) ;
	    if (pip->debuglevel > 0) {
	        fmt = "%s: progpath=%s\n" ;
	        bprintf(pip->efp,fmt,pn,progfname) ;
	    }
	}

	return rs ;
}
/* end subroutine (procexecname) */


static int procexecgo(PROGINFO *pip,ARGINFO *aip)
{
	PROGEXEC	*pep = pip->progexec ;
	int		rs ;
	const int	n = (aip->argr+1) ;

	if ((rs = bflush(pip->efp)) >= 0) {
	    const int	size = (n + 1) * sizeof(cchar **) ; 
	    cchar	**av ;
	    if ((rs = uc_malloc(size,&av)) >= 0) {
	        vechand		*elp = &pep->list ;
	        int		ai = aip->ai ;
	        cchar		**ev ;
	        int		i ;

	        av[0] = pep->argz ;
	        for (i = 1 ; i < n ; i += 1) av[i] = aip->argv[++ai] ;
	        av[i] = NULL ;

	        if ((rs = vechand_getvec(elp,&ev)) >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("main: progfname=%s\n",pep->progfname) ;
#if	CF_DEBUGENV
	                for (i = 0 ; ev[i] != NULL ; i += 1) {
	                    debugprintf("main: env[%d] >%t<\n",i,
	                        ev[i],strlinelen(ev[i],-1,50)) ;
		        }
#endif /* CF_DEBUGENV */
	                for (i = 0 ; av[i] != NULL ; i += 1) {
	                    debugprintf("main: arg[%d] >%t<\n",i,
	                        av[i],strlinelen(av[i],-1,50)) ;
		        }
	            }
#endif /* CF_DEBUG */

#if	CF_DEBUGEXEC && CF_DEBUG
	            rs = SR_OK ;
	            if (DEBUGLEVEL(3))
	                debugprintf("procexecgo: progfname=%s\n",
				pep->progfname) ;
#else /* CF_DEBUGEXEC */
#if	CF_ISAEXEC && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0) && \
	    defined(OSNUM) && (OSNUM >= 8)
	            rs = uc_isaexecve(pep->progfname,av,ev) ;
#else
	            rs = uc_execve(pep->progfname,av,ev) ;
#endif
#endif /* CF_DEBUGEXEC */

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main: u_execve() rs=%d\n",rs) ;
#endif

	        } /* end if (vecstr_getvec) */

	        uc_free(av) ;
	    } /* end if (memory-allocation) */
	} /* end if (bflush) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procexecgo: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procexecgo) */


static int procexecvarspecials(PROGINFO *pip)
{
	PROGEXEC	*pep = pip->progexec ;
	const pid_t	pid = getpid() ;
	int		rs ;
	const char	*pf ;

	pf = pep->progfname ;
	if ((rs = procexecaddenv(pip,"_EF",3,pf,-1)) >= 0) {
	    if ((rs = procexecaddenv(pip,"_A0",3,pep->argz,-1)) >= 0) {
	        const int	elen = ENVBUFLEN ;
	        char		ebuf[ENVBUFLEN+1] ;
	        if ((rs = snshellunder(ebuf,elen,pid,pf)) >= 0) {
	            rs = procexecaddenv(pip,"_",1,ebuf,rs) ;
	        }
	    }
	}

	return rs ;
}
/* end subroutine (procexecvarspecials) */


/* cleanup some environment vaiables */
static int procexecvarclean(PROGINFO *pip)
{
	PROGEXEC	*pep = pip->progexec ;
	vechand 	*elp ;
	int		rs = SR_OK ;
	int		i ;
	const char	**envv = pip->envv ;
	const char	*ep ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procexecvarclean: ent\n") ;
#endif

	if (pep == NULL) return SR_FAULT ;

	elp = &pep->list ;
	for (i = 0 ; envv[i] != NULL ; i += 1) {
	    ep = envv[i] ;
	    if (matkeystr(envbads,ep,-1) < 0) {
		cchar	*tp ;
#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("main/procexecvarclean: not-bad\n") ;
#endif
	        if ((tp = strchr(ep,'=')) != NULL) {
#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("main/procexecvarclean: k=%t\n",
			ep,(tp-ep)) ;
#endif
	            if (matkeystr(envpaths,ep,-1) >= 0) {
	                rs = procexecvarcleaner(pip,ep) ;
	            } else {
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/procexecvarclean: non-path\n") ;
#endif
	                rs = vechand_add(elp,ep) ;
#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("main/procexecvarclean: "
				"vechand_add() rs=%d\n",rs) ;
#endif
	            } /* end if */
	        }
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procexecvarclean: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procexecvarclean) */


static int procexecvarcleaner(PROGINFO *pip,const char *ep)
{
	const int	el = strlen(ep) ;
	int		rs = SR_OK ;
	int		vl = 0 ;
	int		kl ;
	const char	*tp ;
	const char	*vp = NULL ;
	const char	*kp = ep ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procexecvarcleaner: ent\n") ;
#endif

	kl = el ;
	if ((tp = strnchr(ep,el,'=')) != NULL) {
	    kl = (tp-ep) ;
	    vp = (tp+1) ;
	    vl = ((ep+el)-(tp+1)) ;
	}

	if (vp != NULL) {
	    if (vl > 0) {
	        rs = procexecvarcleanmk(pip,kp,kl,vp,vl) ;
	    } else {
	        rs = procexecaddenv(pip,kp,kl,vp,vl) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procexecvarcleaner: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procexecvarcleaner) */


static int procexecaddenv(PROGINFO *pip,cchar *kp,int kl,cchar *vp,int vl)
{
	PROGEXEC	*pep = pip->progexec ;
	strpack		*spp ;
	int		rs ;
	const char	*rp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/procexecaddenv: k=%t v=%t\n",kp,kl,vp,vl) ;
#endif

	spp = &pep->store ;
	if ((rs = strpack_envstorer(spp,kp,kl,vp,vl,&rp)) >= 0) {
	    vechand *elp = &pep->list ;
	    rs = vechand_add(elp,rp) ;
	}

	return rs ;
}
/* end subroutine (procexecaddenv) */


static int procexecvarcleanmk(PROGINFO *pip,cchar *kp,int kl,cchar *vp,int vl)
{
	DIRLIST		db ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("main/procexecvarcleanmk: ent\n") ;
	    debugprintf("main/procexecvarcleanmk: k=>%t<\n",kp,kl) ;
	    debugprintf("main/procexecvarcleanmk: v=>%t<\n",
	        vp,strlinelen(vp,vl,50)) ;
	}
#endif

	if ((rs = dirlist_start(&db)) >= 0) {
	    if ((rs = dirlist_adds(&db,vp,vl)) >= 0) {
	        c = rs ;

	        if ((rs = dirlist_joinsize(&db)) >= 0) {
	            const int	jlen = rs ;
	            char	*jbuf ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procexecvarcleanmk: size=%d\n",rs) ;
#endif

	            if ((rs = uc_malloc((jlen+1),&jbuf)) >= 0) {
	                if ((rs = dirlist_joinmk(&db,jbuf,jlen)) >= 0) {
	                    rs = procexecaddenv(pip,kp,kl,jbuf,rs) ;
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procexecvarcleanmk: "
				"procexecaddenv() rs=%d\n",rs) ;
#endif
	                } /* end if (dirlist-joinmk) */
	                rs1 = uc_free(jbuf) ;
			if (rs >= 0) rs = rs1 ;
	            } /* end if (memory-allocation) */
	        } /* end if (dirlist-joinsize) */

	    } /* end if (dirlist_adds) */
#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main/procexecvarcleanmk: "
			"dirlist_adds-out rs=%d\n",rs) ;
#endif

	    rs1 = dirlist_finish(&db) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dirlist) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procexecvarcleanmk: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procexecvarcleanmk) */


static int findxfile(IDS *idp,char *rbuf,cchar *pn)
{
	VECSTR		pl ;
	int		rs = SR_NOENT ;
	int		f_pwd = FALSE ;
	const char	*tp, *sp ;

	rbuf[0] = '\0' ;
	if ((sp = getenv(VARPATH)) != 0) {
	    if ((rs = vecstr_start(&pl,40,0)) >= 0) {

	        while ((tp = strchr(sp,':')) != NULL) {
	            if ((tp - sp) == 0) f_pwd = TRUE ;
	            rs = vecstr_adduniq(&pl,sp,(tp - sp)) ;
	            sp = (tp + 1) ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (sp[0] != '\0'))
	            rs = vecstr_adduniq(&pl,sp,-1) ;

	        if (rs >= 0)
	            rs = getprogpath(idp,&pl,rbuf,pn,-1) ;

	        if ((! f_pwd) && (rs == SR_NOENT)) {
	            if ((rs = xfile(idp,pn)) >= 0)
	                rs = mkpath1(rbuf,pn) ;
	        }

	        vecstr_finish(&pl) ;
	    } /* end if */
	} /* end if */

	return rs ;
}
/* end subroutine (findxfile) */


#if	CF_ISBADENV

static int isbadenv(cchar *envp)
{
	uint		a ;
	const int	ps = getpagesize() ;
	int		sl ;
	int		maxsl = (8 * MAXPATHLEN) ;
	int		f = FALSE ;

	a = (uint) envp ;
	f = (a < (2 * ps)) ;

	f = f || hasourbad(envp,40) ;

	if (! f) {
	    sl = strnlen(envp,maxsl) ;
	    f = (sl == maxsl) ;
	}

	if (! f)
	    f = hasourbad(envp,sl) ;

	return f ;
}
/* end subroutine (isbadenv) */


static int hasourbad(cchar *sp,int sl)
{
	uint		sch ;
	register int	f = TRUE ;

	if (sp != NULL) {
	    f = hasprintbad(sp,sl) ;
	    if (! f) {
	        register int	i ;
	        if (sl < 0) sl = strlen(sp) ;
	        for (i = 0 ; i < sl ; i += 1) {
	            sch = sp[i] & 0xff ;
	            f = (sch >= 128) ;
	            if (f) break ;
	        } /* end for */
	    } /* end if */
	} /* end if */

	return f ;
}
/* end subroutine (hadourbad) */

#endif /* CF_ISBADENV */


#if	CF_DEBUGS && CF_SOFAR
static int sofar(cchar *bp,int bl)
{
	int		cl ;
	const char	*tp ;
	if (bl < 0) bl = strlen(bp) ;
	while ((tp = strnpbrk(bp,bl,":;")) != NULL) {
	    cl = (tp - bp) ;
	    debugprintf("sofar: %t\n",bp,cl) ;
	    bl -= ((tp + 1) - bp) ;
	    bp = (tp + 1) ;
	} /* end while */
	if (bp[0] != '\0')
	    debugprintf("sofar: e %s\n",bp) ;
	return 0 ;
}
/* end subroutine (sofar) */
#endif /* CF_DEBUGS */


