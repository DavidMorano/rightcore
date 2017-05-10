/* b_testdir */

/* this is a SHELL built-in test fragment */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_NPRINTF	1


/* revision history:

	= 1989-03-01, David A­D­ Morano

	This subroutine was originally written.  This whole program, LOGDIR, is
	needed for use on the Sun CAD machines because Sun doesn't support
	LOGDIR or LOGNAME at this time.  There was a previous program but it is
	lost and not as good as this one anyway.  This one handles NIS+ also.
	(The previous one didn't.)


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testdir


*******************************************************************************/


#include	<envstandards.h>

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
#include	<sys/wait.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<fsdir.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_testdir.h"
#include	"defs.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#ifdef	ALIASNAMELEN
#define	ABUFLEN		ALIASNAMELEN
#else
#define	ABUFLEN		64
#endif

#ifdef	MAILADDRLEN
#define	VBUFLEN		MAILADDRLEN
#else
#define	VBUFLEN		2048
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	OUTLINELEN
#define	OUTLINELEN	80
#endif

#define	OUTPADLEN	20
#define	DEBUGFNAME	"/tmp/testdir.deb"


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrcmpr() ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */


/* forward references */

int		p_testdir(int,const char **,const char **,void *) ;

static int	usage(PROGINFO *) ;


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"ROOT",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const char	*suffixes[] = {
	"",
	"*",
	"/",
	"@",
	"=",
	"|",
	"^",
	"?",
	NULL
} ;

enum suffixes {
	suffix_reg,
	suffix_exec,
	suffix_dir,
	suffix_link,
	suffix_sock,
	suffix_fifo,
	suffix_dev,
	suffix_unk,
	suffix_overlast

} ;

static const char	blanks[] = "                    " ;


/* exported subroutines */


int b_testdir(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
void		*contextp ;
{
	int	rs ;
	int	rs1 ;
	int	ex = EX_OK ;

	if ((rs = lib_kshbegin(contextp)) >= 0) {
	    const char	**envv = (const char **) environ ;
	    ex = p_testdir(argc,argv,envv,contextp) ;
	    rs1 = lib_kshend() ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ksh) */

	if ((rs < 0) && (ex == EX_OK)) ex = EX_DATAERR ;

	return ex ;
}
/* end subroutine (b_testdir) */


int p_testdir(argc,argv,envv,contextp)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
void		*contextp ;
{
	PROGINFO	pi, *pip = &pi ;

	BITS		pargs ;

	KEYOPT		akopts ;

	SHIO	errfile ;
	SHIO	outfile, *ofp = &outfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	npa ;
	int	rs, rs1 ;
	int	n, size, len, c ;
	int	i, j, k ;
	int	sl, cl, ci ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_timeorder = FALSE ;
	int	f_reverse = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*sp, *cp, *cp2 ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("b_testdir: starting DFD=%d\n",rs) ;
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
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;
	pip->f.quiet = FALSE ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,0) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	npa = 0 ;			/* number of positional so far */
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
		const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	                argval = (argp + 1) ;

	            } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* keyword match or only key letters? */

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

/* argument-list file */
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

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* all */
	                        case 'a':
	                            pip->f.all = TRUE ;
	                            break ;

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* reverse sort */
	                        case 'r':
	                            f_reverse = TRUE ;
	                            break ;

/* time order */
	                        case 't':
	                            f_timeorder = TRUE ;
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
	    shio_control(&errfile,SHIO_CSETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
		goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(1))
	    debugprintf("b_testdir: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

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
	rs1 = printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	rs1 = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getourenv(envv,VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* output */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-')) 
	    ofname = STDOUTFNAME ;

	rs = shio_open(ofp,ofname,"wct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    shio_printf(pip->efp,
	        "%s: could not open the output file (%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_testdir: done w/ opening output \n") ;
#endif

	{
	    FSDIR	dir ;
	    FSDIR_ENT	ds ;
	    int		si, clen, blen, fnl ;
	    char	*fnp ;

	    if ((rs = fsdir_open(&dir,".")) >= 0) {

	        while ((rs = fsdir_read(&dir,&ds)) > 0) {
	            fnl = rs ;
	            fnp = ds.name ;

	            if (fnp[0] == '.') {

	                if (fnp[1] == '.') {

	                    if (fnp[2] == '\0')
	                        continue ;

	                } else if (fnp[1] == '\0')
	                    continue ;

	                if (! pip->f.all)
	                    continue ;

	            } /* end if (entries to be ignored) */

#if	CF_NPRINTF
	nprintf(DEBUGFNAME,"b_testdir: fml=%u dn=>%t<\n",
		fnl,fnp,strnlen(fnp,fnl)) ;
	nprintf(DEBUGFNAME,"b_testdir: ino=%u\n",ds.ino) ;
	nprintf(DEBUGFNAME,"b_testdir: off=%u\n",ds.off) ;
	nprintf(DEBUGFNAME,"b_testdir: len=%u\n",ds.reclen) ;
#endif

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
			debugprintf("b_testdir: fnl=%u fnp=>%t<\n",
				fnl,fnp,strnlen(fnp,fnl)) ;
#endif

	            rs = shio_printf(ofp,"file=%s\n",fnp) ;

	            if (rs < 0) break ;
	        } /* end while */

	        fsdir_close(&dir) ;
	    } /* end if (opened directory) */

	} /* end block */

	shio_close(ofp) ;

badoutopen:
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* we are done */
retearly:
	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: program finishing\n",
	        pip->progname) ;
	}

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

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_testdir: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (b_testdir) */


/* local subroutines */


static int usage(pip)
PROGINFO	*pip ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<files(s)>] [-af <afile>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? slen : rs ;
}
/* end subroutine (usage) */


