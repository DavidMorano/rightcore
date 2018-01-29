/* b_look */

/* this is the LOOK program (for looking up words in a dictionary) */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_CHAR		1		/* use 'char(3dam)' */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a knock-off of the famous LOOK program.  It looks up strings
	(which should be prefixes to words) in a sorted word list (contains
	in a file).

	A note from the BSD (I think) version of this program:
 	|
	| The man page said that TABs and SPACEs participate in -d comparisons.
	| In fact, they were ignored.  This implements historic practice, not
	| the manual page.


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
#include	<sys/mman.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<vecobj.h>
#include	<char.h>
#include	<naturalwords.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_look.h"
#include	"defs.h"
#include	"lookword.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#undef	MAXSTRLEN
#define	MAXSTRLEN	256		/* GNU man-page say this */

#ifndef	WORDF
#define	WORDF		"share/dict/words"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	nleadcasestr(const char *,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	strnndictcmp(const char *,int,const char *,int) ;
extern int	isalnumlatin(int) ;
extern int	isdigitlatin(int) ;
extern int	isdict(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */

extern char	**environ ;		/* definition required by AT&T AST */


/* local structures */

struct locinfo_flags {
	uint		sort ;
	uint		w ;
	uint		f ;
	uint		d ;
	uint		q ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, final, changed ;
	PROGINFO	*pip ;
} ;

struct word {
	const char	*wp ;
	int		wl ;
} ;


/* forward references */

static int	mainsub(int,cchar **,cchar **,void *) ;

static int	usage(PROGINFO *) ;

static int	procsort(PROGINFO *,void *,const char *) ;
static int	procsortread(PROGINFO *,vecobj *,const char *,int) ;
static int	procsortout(PROGINFO *,void *,vecobj *) ;

static int	procsearch(PROGINFO *,void *,cchar *,cchar *) ;

static int	wordcmp(const void *,const void *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"ef",
	"of",
	"if",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_ef,
	argopt_of,
	argopt_if,
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

static const char	*prnames[] = {
	"NCMP",
	"LOCAL",
	"GNU",
	"EXTRA",
	NULL
} ;

static const char	*wordfiles[] = {
	"/usr/add-on/ncmp/share/dict/words",
	"/usr/add-on/local/share/dict/words",
	"/usr/add-on/gnu/share/dict/words",
	"/usr/share/lib/dict/words",
	"/usr/share/dict/words",
	"/usr/dict/words",
	NULL
} ;


/* exported subroutines */


int b_look(int argc,cchar *argv[],void *contextp)
{
	int		rs ;
	int		rs1 ;
	int		ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp,NULL)) >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = mainsub(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_look) */


int p_look(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	return mainsub(argc,argv,envv,contextp) ;
}
/* end subroutine (p_look) */


/* local subroutines */


/* ARGSUSED */
static int mainsub(int argc,cchar *argv[],cchar *envv[],void *contextp)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	BITS		pargs ;
	SHIO		errfile ;
	SHIO		ofile, *ofp = &ofile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
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
	const char	*file ;
	const char	*cp ;

	char		tmpfname[MAXPATHLEN + 1] ;
	char		string[MAXSTRLEN + 1] ;


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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

	pip->lip = lip ;
	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	file = NULL ;

	string[0] = '\0' ;

	if (rs >= 0) rs = bits_start(&pargs,0) ;
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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

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

/* output file name */
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

	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            cp = avp ;
	                    } else {
	                        if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            cp = argp ;
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

/* quiet mode */
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* dictionary order */
	                    case 'd':
	                        lip->final.d = TRUE ;
	                        lip->f.d = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.d = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* fold-case */
	                    case 'f':
	                        lip->final.f = TRUE ;
	                        lip->f.f = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.f = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

/* sort mode */
	                    case 's':
	                        lip->f.sort = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.sort = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* whole word */
	                    case 'w':
	                        lip->f.w = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                lip->f.w = (rs > 0) ;
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

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getourenv(envv,VAREFNAME) ;
	if (efname == NULL) efname = STDERRFNAME ;
	if ((rs1 = shio_open(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    shio_control(pip->efp,SHIO_CSETBUFLINE,TRUE) ;
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
	    shio_printf(pip->efp,"%s: version %s\n",
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

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

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

/* more initialization */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

/* process */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
	        cp = argv[ai] ;
		switch (pan) {
	        case 0:
	            if (! lip->final.d) lip->f.d = TRUE ;
	            if (! lip->final.f) lip->f.f = TRUE ;
	            strwcpy(string,cp,MAXSTRLEN) ;
	            break ;
	        case 1:
	            file = cp ;
	            break ;
	        } /* end switch */
	        pan += 1 ;
	    }

	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: prefix=%s\n",string) ;
	    debugprintf("main: file=%s\n",file) ;
	}
#endif

/* find a file if we don't have one already */

	if (file == NULL) {

	    rs = SR_NOENT ;
	    if ((cp = getourenv(envv,VARWORDS)) != NULL) {
	        file = cp ;
	        rs = perm(cp,-1,-1,NULL,R_OK) ;
	    }

	    if (isNotPresent(rs)) {

	        for (i = 0 ; prnames[i] != NULL ; i += 1) {
	            if ((cp = getourenv(envv,prnames[i])) != NULL) {
	                file = tmpfname ;
	                mkpath2(tmpfname,cp,WORDF) ;
	                rs = perm(tmpfname,-1,-1,NULL,R_OK) ;
		    }
	            if (rs >= 0) break ;
	        } /* end for */

	    } /* end if (moderately tough measures) */

	    if (isNotPresent(rs)) {

	        for (i = 0 ; wordfiles[i] != NULL ; i += 1) {

	            rs = perm(wordfiles[i],-1,-1,NULL,R_OK) ;
	            file = (char *) wordfiles[i] ;

	            if (rs >= 0) break ;
	        } /* end for */

	    } /* end if (tough measures) */

	    if (isNotPresent(rs))
	        file = WORDSFNAME ;

	} /* end if (finding file) */

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: dict=%s\n",pip->progname,file) ;
	    shio_printf(pip->efp, "%s: fold=%u dictorder=%u\n",
	        pip->progname,lip->f.f,lip->f.d) ;
	}

/* continue */

#if	CF_DEBUGS
	debugprintf("main: file=%s f_dict=%u f_fold=%u\n",
	    file,lip->f.f,lip->f.f) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: dict=%s\n", file) ;
	    debugprintf("main: f_sort=%u f_dict=%u f_fold=%u\n",
	        lip->f.sort,lip->f.f,lip->f.f) ;
	}
#endif

/* print out a sorted list */

	if (rs >= 0) {

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {

	    if (lip->f.sort) {
	        rs = procsort(pip,ofp,file) ;
	    } else {
	        rs = procsearch(pip,ofp,file,string) ;
	    }

	    rs1 = shio_close(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (output-open) */

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
	} else if ((rs == 0) && (ex == EX_OK)) {
	    ex = 1 ;
	}

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

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: bad or invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;
}
/* end subroutine (mainsub) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-df] [-w] <string> [<file>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-of <ofile>] [-s]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procsort(PROGINFO *pip,void *ofp,cchar fname[])
{
	VECOBJ		wlist ;
	const int	n = 250000 ;
	int		rs ;
	int		size ;
	int		wlen = 0 ;

	if (pip == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procsort: ent fname=%s\n",fname) ;
#endif

	if (fname[0] == '-') fname = "/dev/stdin" ;

	size = sizeof(struct word) ;
	if ((rs = vecobj_start(&wlist,size,n,0)) >= 0) {
	    const mode_t	operms = 0666 ;
	    const int		oflags = O_RDONLY ;
	    if ((rs = uc_open(fname,oflags,operms)) >= 0) {
	        struct ustat	sb ;
	        int	fd = rs ;
	        if (((rs = u_fstat(fd,&sb)) >= 0) && S_ISREG(sb.st_mode)) {
	            const size_t	fs = (size_t) (sb.st_size & LONG_MAX) ;
	            size_t 		ps = getpagesize() ;
	            if (fs > 0) {
	                size_t		ms = MAX(fs,ps) ;
	                const int	mp = PROT_READ ;
	                const int	mf = MAP_SHARED ;
	                void		*md ;
	                if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	                    int		madv = MADV_SEQUENTIAL ;
	                    int		mdl = ms ;
	                    const char	*mdp = md ;
	                    const caddr_t	ma = (const caddr_t) md ;

	                    if ((rs = uc_madvise(ma,ms,madv)) >= 0) {
	                        rs = procsortread(pip,&wlist,mdp,mdl) ;
	                        if (rs >= 0) {
	                            madv = MADV_RANDOM ;
	                            rs = uc_madvise(ma,ms,madv) ;
	                        }
	                    } /* end if (memory-advice) */

	                    if (rs >= 0)
	                        rs = vecobj_sort(&wlist,wordcmp) ;

	                    if (rs >= 0) {
	                        rs = procsortout(pip,ofp,&wlist) ;
	                        wlen += rs ;
	                    }

	                    u_munmap(md,ms) ;
	                } /* end if (map file) */
	            } /* end if (non-zero-length) */
	        } else {
	            if (rs >= 0) rs = SR_NOTSUP ;
	        }
	        u_close(fd) ;
	    } /* end if (file-open) */
	    vecobj_finish(&wlist) ;
	} /* end if (word-list) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procsort: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsort) */


/* read all of the words in */
static int procsortread(PROGINFO *pip,vecobj *wlp,cchar *mdp,int mdl)
{
	struct word	w ;
	int		rs = SR_OK ;
	int		len ;
	int		ll ;
	int		wl ;
	const char	*tp ;
	const char	*lp ;
	const char	*wp ;

	while ((tp = strnchr(mdp,mdl,'\n')) != NULL) {
	    len = ((tp + 1) - mdp) ;

	    lp = mdp ;
	    ll = (len - 1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procsort: w=>%t<\n",lp,ll) ;
#endif

	    if ((tp = strnchr(lp,ll,'#')) != NULL)
	        ll = (tp - lp) ;

	    if ((wl = sfshrink(lp,ll,&wp)) > 0) {
	        w.wp = wp ;
	        w.wl = wl ;
	        rs = vecobj_add(wlp,&w) ;
	    }

	    mdp += len ;
	    mdl -= len ;

	    if (rs < 0) break ;
	} /* end while */

	return rs ;
}
/* end subroutine (procsortread) */


static int procsortout(PROGINFO *pip,void *ofp,vecobj *wlp)
{
	struct word	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		wlen = 0 ;

	for (i = 0 ; vecobj_get(wlp,i,&ep) >= 0 ; i += 1) {
	    if (ep == NULL) continue ;
	    rs = shio_print(ofp,ep->wp,ep->wl) ;
	    wlen += rs ;
	    if (rs >= 0) rs = lib_sigterm() ;
	    if (rs >= 0) rs = lib_sigintr() ;
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procsortout) */


static int procsearch(PROGINFO *pip,void *ofp,cchar *dfname,cchar *string)
{
	LOCINFO		*lip = pip->lip ;
	LOOKWORD	lw ;
	int		rs ;
	int		rs1 ;
	int		lo = 0 ;
	int		c = 0 ;

	if (lip->f.d) lo |= LOOKWORD_ODICT ;
	if (lip->f.f) lo |= LOOKWORD_OFOLD ;
	if (lip->f.w) lo |= LOOKWORD_OWORD ;
	if ((rs = lookword_open(&lw,dfname,lo)) >= 0) {
	    LOOKWORD_CUR	cur ;
	    if ((rs = lookword_curbegin(&lw,&cur)) >= 0) {
		if ((rs = lookword_lookup(&lw,&cur,string)) > 0) {
		    const int	rlen = NATURALWORDLEN ;
		    char	rbuf[NATURALWORDLEN+1] ;
		    while ((rs = lookword_read(&lw,&cur,rbuf,rlen)) > 0) {
			c += 1 ;
			if (pip->verboselevel > 0) {
			    rs = shio_print(ofp,rbuf,rs) ;
			}
			if (rs < 0) break ;
		    } /* end while */
		} /* end if (lookword_lookup) */
		rs1 = lookword_curend(&lw,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (lookword-cur) */
	    rs1 = lookword_close(&lw) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (lookword) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsearch) */


static int wordcmp(const void *v1p,const void *v2p)
{
	struct word	*e1p, **e1pp = (struct word **) v1p ;
	struct word	*e2p, **e2pp = (struct word **) v2p ;
	int		rc = 0 ;

	if (*e1pp == NULL) {
	    rc = 1 ;
	} else {
	    if (*e2pp == NULL) {
	        rc = -1 ;
	    } else {
	        e1p = *e1pp ;
	        e2p = *e2pp ;
	        rc = strnndictcmp(e1p->wp,e1p->wl,e2p->wp,e2p->wl) ;
	    }
	}

	return rc ;
}
/* end subroutine (wordcmp) */


