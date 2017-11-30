/* main */

/* test program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	1		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* 'uc_mallout(3uc)' */
#define	CF_DEBUGMINFO	0		/* debugmallxxx() */
#define	CF_DEBUGSTFIND	0		/* debug 'strtabfind()' */
#define	CF_STARTSMALL	1		/* use a small start-size for STRTAB */
#define	CF_COUNTS	1		/* counts of items */
#define	CF_VECSTR	0		/* VECSTR handling */
#define	CF_HDB		1		/* HDB handling */
#define	CF_STRTAB	1		/* turn ON STRTAB */
#define	CF_STRTABREAD	1		/* turn ON STRTAB reading */
#define	CF_STRTABHAVE	1		/* turn ON STRTAB having */
#define	CF_STRTABCMP	1		/* turn ON STRTAB comparing */
#define	CF_STRTABMK	1		/* turn ON STRTAB making */
#define	CF_STRTABREC	1		/* turn ON STRTAB recording */
#define	CF_STRTABIND	1		/* turn ON STRTAB indexing */
#define	CF_STRTABLOOK	1		/* turn ON STRTAB lookups */
#define	CF_WFIELD	0		/* write out field strings */
#define	CF_LIBMALLOC	0		/* include 'mallinfo(3malloc)' */
#define	CF_HASHSKIP	1		/* use hash-index skipping */
#define	CF_HASHLINK	1		/* use hash-index linking */


/* revision history:

	= 1999-03-01, David A­D­ Morano

	The base argument processing was grabbed from another program
	(from way back).  The basic test is new for the STRTAB object.


*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ teststrtab.x <infile>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/time.h>		/* for 'gethrvtime(3c)' */
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<math.h>

#if	CF_LIBMALLOC
#include	<malloc.h>
#endif

#include	<vsystem.h>
#include	<bits.h>
#include	<filemap.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<hdb.h>
#include	<density.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"strtab.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#define	BUFLEN		1024

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	MAXFILESIZE	(200 * 1024 * 1024)
#define	NSTRINGS	250000
#define	NSKIP		10


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	isprintlatin(int) ;
extern int	randlc(int) ;

extern int	field_word(FIELD *,const uchar *,const char **) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

extern int	strtabfind(const char *,int (*)[3],int,int,const char *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct ustats {
	int	t, vs, strs, stab ;
} ;

struct mallstate {
	uint	memout ;
#if	CF_LIBMALLOC
	struct mallinfo	mi ;
#endif
} ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	process(struct proginfo *,bfile *,const char *) ;
static int	mkindhdb(struct proginfo *,bfile *,const char *,
			int (*)[3],int,int,HDB *) ;

static int	strtablook(struct proginfo *,bfile *,const char *,
			int (*)[3],int,int,FILEMAP *) ;

static int	isweirdo(const char *,int) ;
static int	ismatkey(const char *,const char *,int) ;

static int	hashindex(uint,int) ;

static int	debugmallmark(struct mallstate *) ;
static int	debugmallstat(struct mallstate *,const char *) ;
static int	debugmallinfo(struct mallstate *,const char *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"df",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_df,
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


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct mallstate	ms_prog ;

	struct proginfo	pi, *pip = &pi ;

	BITS		pargs ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	uint	mo_start = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	v ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getenv(VARDEBUGFNAME)) == NULL) {
	    if ((cp = getenv(VARDEBUGFD1)) == NULL)
	        cp = getenv(VARDEBUGFD2) ;
	}
	if (cp != NULL)
	    debugopen(cp) ;
	debugprintf("main: starting\n") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

#if	CF_DEBUGS && CF_DEBUGMINFO
	debugmallinfo(&ms_prog,"PROG begin") ;
	debugmallmark(&ms_prog) ;
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	ai = 0 ;
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
	    if ((argl > 0) && (f_optminus || f_optplus)) {

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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
	                    break ;

/* argument files */
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

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
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

/* division-factor */
	                case argopt_df:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdeci(avp,avl,&v) ;
				    pip->df = v ;
				}
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdeci(argp,argl,&v) ;
				    pip->df = v ;
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* unbuffered mode */
	                    case 'u':
	                        pip->f.unbuf = TRUE ;
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
	                    if (rs < 0)
	                        break ;

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
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

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

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* check some arguments */

	if (pip->df == 0)
	    pip->df = 6 ;

/* open the output file */

	if (ofname != NULL) {
	    rs = bopen(ofp,ofname,"wct",0666) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open output file (%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

	if (pip->f.unbuf)
	    bcontrol(ofp,BC_LINEBUF,0) ;

/* get filename argument */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = process(pip,ofp,cp) ;

	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int	len ;
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cp = lbuf ;
	            if (cp[0] == '\0') continue ;

	            pan += 1 ;
	            rs = process(pip,ofp,cp) ;

	            if (rs < 0) {
	                bprintf(pip->efp,
	                    "%s: error (%d) in file=%s\n",
	                    pip->progname,rs,cp) ;
	                break ;
	            }

	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {

	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: \tafile=%s\n",
	                pip->progname,afname) ;
	        }

	    } /* end if */

	} /* end if (processing file argument file list) */

	bclose(ofp) ;

/* we are done */
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* early return thing */
badoutopen:
retearly:
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	CF_DEBUGS && CF_DEBUGMINFO
	debugmallstat(&ms_prog,"PROG end") ;
	debugmallinfo(&ms_prog,"PROG end") ;
#endif /* CF_DEBUGS */

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("main: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
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
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<file(s)>|-]\n",
	    pn,pn) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n",
	    pn) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(pip,ofp,ifname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	ifname[] ;
{
	struct ustats	s ;

	struct ustat	sb ;

	struct mallstate	ms_sub ;
	struct mallstate	ms_hdb ;
	struct mallstate	ms ;

	FILEMAP	imf ;

	STRTAB	stab ;

	HDB	strs ;

	FIELD	fsb ;

	int	rs, rs1, rs2, rs3 ;
	int	nstrings ;
	int	c, i ;
	int	sl ;
	int	ll, fl ;
	int	size, stsize, itsize, itlen ;
	int	nskip = NSKIP ;

	const char	*lp, *fp ;
	const char	*sp, *cp ;

	char	*tab = NULL ;


	if (ifname == NULL)
	    return SR_FAULT ;

	if ((ifname[0] == '\0') || (ifname[0] == '-'))
	    return SR_INVALID ;

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2)) {
	    debugmallmark(&ms_sub) ;
	    debugmallinfo(&ms_sub,"SUB begin") ;
	}
#endif /* CF_DEBUG */

	memset(&s,0,sizeof(struct ustats)) ;

#if	CF_WFIELD
	rs = bopen(&wf,"fs","wct",0666) ;
	if (rs < 0) goto ret0 ;
#endif

	if ((ifname != NULL) && (ifname[0] != '\0')) {
	    rs = u_stat(ifname,&sb) ;
	} else
	    rs = u_fstat(FD_STDIN,&sb) ;

	if (rs < 0) {
	    bprintf(pip->efp,"%s: input unavailable (%d)\n",
	        pip->progname,rs) ;
	    goto ret0 ;
	}

	nstrings = (sb.st_size / pip->df) + 1000 ;

/* initialization phase */

	bprintf(ofp,"INIT\n") ;

#if	CF_STRTAB

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2))
	    debugmallinfo(&ms_sub,"strtab_start") ;
#endif /* CF_DEBUG */

#if	CF_STARTSMALL
	size = nstrings ;
#else
	size = (nstrings * 5) ;
#endif

	rs = strtab_start(&stab,size) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: strtab_start() rs=%d\n",rs) ;
#endif /* CF_DEBUG */

	if (rs < 0)
	    goto ret0 ;

#endif /* CF_STRTAB */

#if	CF_VECSTR

	rs = vecstr_start(&sdb,nstrings,0) ;
	if (rs < 0)
	    goto ret1a ;

#endif /* CF_VECSTR */

	bprintf(ofp,"INIT rs=%d\n",rs) ;

/* reading phase */

	bprintf(ofp,"READING\n") ;

	rs = filemap_open(&imf,ifname,O_RDONLY,MAXFILESIZE) ;
	if (rs < 0)
	    goto ret1b ;

#if	CF_HDB

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2)) {
	    debugmallmark(&ms_hdb) ;
	    debugmallinfo(&ms_hdb,"hdb_start") ;
	}
#endif /* CF_DEBUG */

	rs = hdb_start(&strs,nstrings,TRUE,NULL,NULL) ;
	if (rs < 0)
	    goto ret2 ;

#endif /* CF_HDB */

	c = 0 ;
	while ((rs = filemap_getline(&imf,&lp)) > 0) {
	    ll = rs ;

	    if (lp[ll - 1] == '\n') ll -= 1 ;
	    

#if	CF_DEBUG && 0
	    if (DEBUGLEVEL(5))
	        debugprintf("process: line=>%t<\n",lp,ll) ;
#endif

	    if (strnchr(lp,ll,'&') != NULL) continue ;

	    if ((rs = field_start(&fsb,lp,ll)) >= 0) {

	        while ((fl = field_word(&fsb,NULL,&fp)) > 0) {

	            c += 1 ;
	            s.t += 1 ;

#if	CF_WFIELD
	            bprintf(&wf,"%t\n",fp,fl) ;
#endif

#if	CF_VECSTR
	            rs1 = vecstr_findn(&sdb,fp,fl) ;
	            if (rs1 == SR_NOTFOUND) {

	                s.vs += 1 ;
	                rs = vecstr_add(&sdb,fp,fl) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2) && (rs < 0))
	                    debugprintf("process: vecstr_add() rs=%d\n",
	                        rs) ;
#endif

	            }
#endif /* CF_VECSTR */

#if	CF_HDB
	            if (rs >= 0) {
	                HDB_DATUM	key, value ;

	                key.buf = fp ;
	                key.len = fl ;

	                rs1 = hdb_fetch(&strs,key,NULL,&value) ;
	                if (rs1 == SR_NOTFOUND) {

	                    s.strs += 1 ;

	                    value.len = key.len ;
	                    value.buf = key.buf ;
	                    rs = hdb_store(&strs,key,value) ;

	                }
	            }
#endif /* CF_HDB */

#if	CF_STRTAB

#if	CF_DEBUG && 0
	            if ((rs >= 0) && DEBUGLEVEL(2)) {
	                if (isweirdo(fp,fl)) {
	                    debugprintf("process: weirdo string read\n") ;
	                    rs = SR_BADFMT ;
	                }
	            }
#endif /* CF_DEBUG */

	            if (rs >= 0) {

#if	CF_STRTABHAVE
	                rs1 = strtab_already(&stab,fp,fl) ;
#else
	                rs1 = SR_NOTFOUND ;
#endif /* CF_STRTABHAVE */

	                if (rs1 == SR_NOTFOUND) {

	                    s.stab += 1 ;
#if	CF_STRTABREAD
	                    rs = strtab_add(&stab,fp,fl) ;
#endif

	                }
	            }

#if	CF_DEBUG && 0
	            if (DEBUGLEVEL(2) && (rs < 0)) 
	                debugprintf("process: strtab_add() rs=%d\n",rs) ;
#endif

#endif /* CF_STRTAB */

	            if (rs < 0) break ;
	        } /* end while (words) */

	        field_finish(&fsb) ;
	    } /* end if (field handling) */

	    if (rs < 0) break ;
	} /* end while (reading lines) */

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2))
	    debugmallinfo(&ms_sub,"READING end") ;
#endif /* CF_DEBUG */

	bprintf(ofp,"READING rs=%d c=%u\n",rs,c) ;

/* print out counts */

#if	CF_COUNTS
	if (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGMINFO
	    if (DEBUGLEVEL(2)) {
	        debugmallmark(&ms) ;
	        debugmallinfo(&ms,"COUNT start") ;
	    }
#endif /* CF_DEBUG */

	    bprintf(ofp,"COUNTS\n") ;

#if	CF_VECSTR
	    rs1 = vecstr_count(&sdb) ;
#else
	    rs1 = 0 ;
#endif /* CF_VECSTR */

#if	CF_HDB
	    rs2 = hdb_count(&strs) ;
#else
	    rs2 = 0 ;
#endif

#if	CF_STRTAB
	    rs3 = strtab_count(&stab) ;
#else
	    rs3 = 0 ;
#endif /* CF_STRTAB */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("process: t=%u vs=%u strs=%u stab=%u\n",
	            s.t,s.vs,s.strs,s.stab) ;
	        debugprintf("process: count=%u:%u:%u\n",rs1,rs2,rs3) ;
	    }
#endif

	    bprintf(ofp,"COUNTS rs=%d c1=%u c2=%u c3=%u\n",
	        rs,rs1,rs2,rs3) ;

#if	CF_DEBUGS && CF_DEBUGMINFO
	    if (DEBUGLEVEL(2)) {
	        debugmallinfo(&ms,"COUNTS end") ;
	        debugmallstat(&ms,"COUNTS end") ;
	    }
#endif /* CF_DEBUG */

	} /* end if (counts) */
#endif /* CF_COUNTS */

/* compare various string representations */

#if	CF_STRTAB && CF_STRTABCMP && CF_VECSTR

	if (rs >= 0) {

#if	CF_DEBUGS && CF_DEBUGMINFO
	    if (DEBUGLEVEL(2)) {
	        debugmallmark(&ms) ;
	        debugmallinfo(&ms,"CMP start") ;
	    }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: CMP\n") ;
#endif

	    bprintf(ofp,"CMP\n") ;

	    c = 0 ;
	    for (i = 0 ; vecstr_get(&sdb,i,&sp) >= 0 ; i += 1) {

	        if (sp == NULL) continue ;

	        c += 1 ;
	        rs = strtab_already(&stab,sp,-1) ;

	        if (rs < 0)
	            break ;

	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: CMP rs=%d c=%u\n",rs,c) ;
#endif

	    bprintf(ofp,"CMP rs=%d c=%u\n",rs,c) ;

#if	CF_DEBUGS && CF_DEBUGMINFO
	    if (DEBUGLEVEL(2)) {
	        debugmallinfo(&ms_sub,"CMP end") ;
	        debugmallstat(&ms_sub,"CMP end") ;
	    }
#endif /* CF_DEBUG */

	} /* end if (comparing string representations) */

#endif /* CF_STRTABCMP */

/* create (make) a string table */

#if	CF_STRTAB && CF_STRTABMK

	if (rs >= 0) {

	    bprintf(ofp,"MKSTR\n") ;

	    rs = strtab_strsize(&stab) ;

	    stsize = rs ;
	    bprintf(ofp,"MKSTR stsize=%u\n",stsize) ;

	    if (rs >= 0)
	        rs = uc_malloc(stsize,&tab) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: stsize=%u\n",stsize) ;
#endif

	        rs = strtab_strmk(&stab,tab,stsize) ;

#if	CF_VECSTR

	        if (rs >= 0) {

	            int	si ;


	            bprintf(ofp,"MKSTR VECSTR\n") ;

	            c = 0 ;
	            for (i = 0 ; vecstr_get(&sdb,i,&sp) >= 0 ; i += 1) {

	                if (sp == NULL) continue ;

	                rs = strtab_already(&stab,sp,-1) ;

	                si = rs ;
	                if (rs < 0)
	                    break ;

	                cp = (tab + si) ;
	                if ((si == 0) || (strcmp(cp,sp) != 0)) {
	                    rs = SR_NOTFOUND ;
	                    break ;
	                }

	                c += 1 ;

	            } /* end for */

	            bprintf(ofp,"MKSTR VECSTR rs=%d c=%u\n",rs,c) ;

	        } /* end if (checking new string table) */

#endif /* CF_VECSTR */

#if	CF_HDB

	        if (rs >= 0) {
	            HDB_CUR	cur ;
	            HDB_DATUM	key, value ;

	            int	si ;


#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process: MKSTR HDB\n") ;
#endif

	            bprintf(ofp,"MKSTR HDB\n") ;

	            c = 0 ;
	            hdb_curbegin(&strs,&cur) ;

	            while (hdb_enum(&strs,&cur,&key,&value) >= 0) {

	                sp = (char *) key.buf ;
	                sl = key.len ;

#if	CF_DEBUG && 0
	                if (DEBUGLEVEL(3))
	                    debugprintf("process: MKSTR sp=%t\n",sp,sl) ;
#endif

	                rs = strtab_already(&stab,sp,sl) ;

#if	CF_DEBUG && 0
	                if (DEBUGLEVEL(3))
	                    debugprintf("process: strtab_already() rs=%d\n",
	                        rs) ;
#endif

	                si = rs ;
	                if (rs < 0)
	                    break ;

#if	CF_DEBUG && 0
	                if (DEBUGLEVEL(3))
	                    debugprintf("process: MKSTR si=%u\n",si) ;
#endif

	                cp = (tab + si) ;
	                if ((si == 0) || (strncmp(cp,sp,sl) != 0) ||
	                    (cp[sl] != '\0')) {
	                    rs = SR_NOTFOUND ;
	                    break ;
	                }

	                c += 1 ;

	            } /* end while */

	            hdb_curend(&strs,&cur) ;

	            bprintf(ofp,"MKSTR HDB rs=%d c=%u\n",rs,c) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("process: MKSTR HDB rs=%d c=%u\n",rs,c) ;
#endif

	        } /* end if (HDB verification) */

#endif /* CF_HDB */

	    } /* end if (allocation) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: MKSTR rs=%d\n",rs) ;
#endif

	    bprintf(ofp,"MKSTR rs=%d c=%u\n",rs,c) ;

	} /* end if (making a string table) */

#endif /* CF_STRTABMK */

/* record the string table */

#if	CF_STRTAB && CF_STRTABREC && CF_STRTABMK

	if (rs >= 0) {

	    int	recsize, reclen ;
	    int	si ;
	    int	*rec = NULL ;


#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGMALL
	    if (DEBUGLEVEL(2))
	        debugmallmark(&ms) ;
#endif /* CF_DEBUG */

	    bprintf(ofp,"MKREC\n") ;

	    rs = strtab_count(&stab) ;
	    reclen = (rs + 1) ;		/* ZERO-th entry is NUL-string */

	    bprintf(ofp,"MKREC reclen=%u\n",reclen) ;

	    if (rs >= 0) {
	        rs = strtab_recsize(&stab) ;
	        recsize = rs ;
	    }

	    bprintf(ofp,"MKREC recsize=%u\n",recsize) ;

	    if (rs >= 0)
	        rs = uc_malloc(recsize,&rec) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            debugprintf("process: reclen=%u\n",reclen) ;
	            debugprintf("process: recsize=%u\n",recsize) ;
	        }
#endif

	        rs = strtab_recmk(&stab,rec,recsize) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: strtab_recmk() rs=%d\n",rs) ;
#endif

#if	CF_VECSTR

	        if (rs >= 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process: MKREC VECSTR\n") ;
#endif

	            bprintf(ofp,"MKREC VECSTR\n") ;

	            si = -1 ;
	            c = 0 ;
	            for (i = 1 ; (i < reclen) &&
	                ((si = rec[i]) >= 0) ; i += 1) {

	                cp = (tab + si) ;
	                if (si == 0) {
	                    rs = SR_INVALID ;
	                    break ;
	                }

	                rs = vecstr_find(&sdb,cp) ;
	                if (rs < 0)
	                    break ;

	                c += 1 ;

	            } /* end for */

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process: MKREC VECSTR rs=%d c=%u\n",rs,c) ;
#endif

	            bprintf(ofp,"MKREC VECSTR rs=%d c=%u\n",rs,c) ;

	        } /* end if (VECSTR verification) */

#endif /* CF_VECSTR */

#if	CF_HDB

	        if (rs >= 0) {

	            HDB_DATUM	key ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process: MKREC HDB\n") ;
#endif

	            bprintf(ofp,"MKREC HDB\n") ;

	            si = -1 ;
	            c = 0 ;
	            for (i = 1 ; (i < reclen) &&
	                ((si = rec[i]) >= 0) ; i += 1) {

	                cp = (tab + si) ;
	                if (si == 0) {
	                    rs = SR_INVALID ;
	                    break ;
	                }

	                key.buf = cp ;
	                key.len = strlen(cp) ;

	                rs = hdb_fetch(&strs,key,NULL,NULL) ;
	                if (rs < 0)
	                    break ;

	                c += 1 ;

	            } /* end for */

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process: MKREC HDB rs=%d c=%u\n",rs,c) ;
#endif

	            bprintf(ofp,"MKREC HDB rs=%d c=%u\n",rs,c) ;

	        } /* end if (HDB verification) */

#endif /* CF_HDB */

	        if (rec != NULL)
	            uc_free(rec) ;

	    } /* end if (allocated string-record table) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: MKREC rs=%d\n",rs) ;
#endif

	    bprintf(ofp,"MKREC rs=%d\n",rs) ;

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGMALL
	    if (DEBUGLEVEL(2))
	        debugmallstat(&ms,"MKREC") ;
#endif

	} /* end if (recording a string table) */

#endif /* CF_STRBREC */

/* index the string table */

#if	CF_STRTAB && CF_STRTABIND && CF_STRTABMK

	if (rs >= 0) {
	    int		(*it)[3] = NULL ;

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGMALL
	    if (DEBUGLEVEL(2))
	        debugmallmark(&ms) ;
#endif /* CF_DEBUG */

	    bprintf(ofp,"MKIND\n") ;

	    rs = strtab_indsize(&stab) ;
	    itsize = rs ;

	    bprintf(ofp,"MKIND itsize=%u\n",itsize) ;

	    if (rs >= 0)
	        rs = uc_malloc(itsize,&it) ;

	    if (rs >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: itsize=%u\n",itsize) ;
#endif

	        rs1 = strtab_indlen(&stab) ;
	        itlen = rs1 ;

	        bprintf(ofp,"MKIND itlen=%u\n",itlen) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: itlen=%u\n",itlen) ;
#endif

	        rs = strtab_indmk(&stab,it,itsize,NSKIP) ;

#if	CF_DEBUG && 0
	        if (DEBUGLEVEL(2)) {
	            debugprintf("process: strtab_indmk() rs=%d\n",rs) ;
	            debugprintf("process:    II    SI    THASH    NI\n") ;
		    for (i = 0 ; i < itlen ; i += 1) {
	            debugprintf("process: %5u %5u %8x %5u\n",
			i,it[i][0],it[i][1],it[i][2]) ;
		    }
		}
#endif

	        bprintf(ofp,"MKIND sc=%u\n",rs) ;

/* verify all strings are present */

#if	CF_VECSTR

	        if (rs >= 0) {

			int	j ;


#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process: MKIND VECSTR\n") ;
#endif

	            bprintf(ofp,"MKIND VECSTR\n") ;

	            c = 0 ;
	            for (i = 0 ; vecstr_get(&sdb,i,&sp) >= 0 ; i += 1) {

	                if (sp == NULL) continue ;

			sl = strlen(sp) ;

			j = strtabfind(tab,it,itlen,nskip,sp,sl) ;
	                if (j < 0) {

#if	CF_DEBUG && 0
	                    if (DEBUGLEVEL(2))
	                        debugprintf("process: notfound sp=%s\n",
	                            sp) ;
#endif

	                    rs = SR_NOTFOUND ;
	                    break ;
	                }

	                c += 1 ;

	            } /* end for */

	            bprintf(ofp,"MKIND VECSTR rs=%d c=%u\n",rs,c) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(2))
	                debugprintf("process: MKIND VECSTR rs=%d c=%u i=%u\n",
	                    rs,c,i) ;
#endif

	        } /* end if (VECSTR verification) */

#endif /* CF_VECSTR */

#if	CF_HDB

	        if (rs >= 0)
	            rs = mkindhdb(pip,ofp,tab,it,itlen,nskip,&strs) ;

#endif /* CF_HDB */

#if	CF_STRTABLOOK
		if (rs >= 0)
		    rs = strtablook(pip,ofp,tab,it,itlen,nskip,&imf) ;
#endif

	        if (it != NULL)
	            uc_free(it) ;

	    } /* end if (allocation) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: MKIND rs=%d\n",rs) ;
#endif

	    bprintf(ofp,"MKIND rs=%d\n",rs) ;

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGMALL
	    if (DEBUGLEVEL(2))
	        debugmallstat(&ms,"MKIND") ;
#endif

	} /* end if (making an index table) */

#endif /* CF_STRTABIND */

	if (tab != NULL) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("process: free(tab)\n") ;
#endif

	    uc_free(tab) ;

#if	CF_DEBUGS && CF_DEBUGMINFO
	    if (DEBUGLEVEL(2))
	        debugmallinfo(&ms_sub,"free TAB") ;
#endif /* CF_DEBUG */

	} /* end if (freeing table) */

ret4:

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2))
	    debugmallinfo(&ms_sub,"ret4") ;
#endif /* CF_DEBUG */

#if	CF_HDB

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2))
	    debugmallinfo(&ms_sub,"ret3") ;
#endif /* CF_DEBUG */

	hdb_finish(&strs) ;

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGMALL
	if (DEBUGLEVEL(2)) {
	    debugmallstat(&ms_hdb,"hdb_finish") ;
	    debugmallinfo(&ms_hdb,"hdb_finish") ;
	}
#endif /* CF_DEBUG */

#endif /* CF_HDB */

ret2:
	filemap_close(&imf) ;

ret1b:

#if	CF_VECSTR
#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2))
	    debugmallinfo(&ms_sub,"ret2") ;
#endif /* CF_DEBUG */

	vecstr_finish(&sdb) ;

#endif /* CF_VECSTR */

ret1a:

#if	CF_STRTAB

	strtab_finish(&stab) ;

#if	CF_DEBUGS && CF_DEBUGMINFO
	    if (DEBUGLEVEL(2))
	    debugmallinfo(&ms_sub,"strtab_finish") ;
#endif /* CF_DEBUG */

#endif /* CF_STRTAB */

ret1:

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2))
	    debugmallinfo(&ms_sub,"ret1") ;
#endif /* CF_DEBUG */

#if	CF_WFIELD
	bclose(&wf) ;
#endif

ret0:

#if	CF_DEBUGS && CF_DEBUGMINFO
	if (DEBUGLEVEL(2)) {
	    debugmallinfo(&ms_sub,"ret0") ;
	    debugmallstat(&ms_sub,"SUB end") ;
	}
#endif /* CF_DEBUG */

	return rs ;
}
/* end subroutine (process) */


#if	CF_HDB

static int mkindhdb(pip,ofp,tab,it,itlen,nskip,strp)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*tab ;
int		(*it)[3] ;
int		itlen ;
int		nskip ;
HDB		*strp ;
{
	DENSITY		d ;
	DENSITY_STATS	s ;

	HDB_CUR		cur ;
	HDB_DATUM	key, value ;

	hrtime_t	ht_start, ht_end ;

	uint	tmsecs, secs, msecs ;

	int	rs = SR_OK ;
	int	j ;
	int	sl ;
	int	sc = 0 ;

	const char	*sp ;

	char	timebuf[TIMEBUFLEN + 1] ;


	if (pip == NULL) return SR_FAULT ;

	bprintf(ofp,"MKIND HDB\n") ;

	rs = density_start(&d,itlen) ;
	if (rs < 0)
	    goto ret0 ;

	ht_start = gethrvtime() ;

	hdb_curbegin(strp,&cur) ;

	while (hdb_enum(strp,&cur,&key,&value) >= 0) {

	    sp = (char *) key.buf ;
	    sl = key.len ;

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(3))
	    debugprintf("process/mkindhdb: s>%t<\n",sp,sl) ;
#endif

	    j = strtabfind(tab,it,itlen,nskip,sp,sl) ;

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(3))
	    debugprintf("process/mkindhdb: strtabfind() j=%d\n",j) ;
#endif

	    sc += (j + 1) ;
	    if (j < 0) {
	        rs = SR_NOTFOUND ;
	        break ;
	    }

	    density_update(&d,(j + 1)) ;

	} /* end while */

	hdb_curend(strp,&cur) ;

	ht_end = gethrvtime() ;

	if (rs >= 0) {

	    tmsecs = (ht_end - ht_start) / 1000000 ;
	    msecs = (tmsecs % 1000) ;
	    secs = (tmsecs / 1000) ;
	    bprintf(ofp,"MKIND HDB elapsed=%s.%03u\n",
	        timestr_elapsed((time_t) secs,timebuf),msecs) ;

	    if (density_stats(&d,&s) >= 0) {

	        bprintf(ofp,"MKIND HDB sc=%u max=%u mean=%9.1f sd=%9.1f\n",
	            sc,s.max,s.mean,sqrt(s.var)) ;

	    } /* end if (density statistics) */

	} /* end if */

	density_finish(&d) ;

	bprintf(ofp,"MKIND HDB rs=%d sc=%u\n",rs,sc) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("process/mkindhdb: ret rs=%d sc=%u\n",
	        rs,sc) ;
#endif

	return (rs >= 0) ? sc : rs ;
}
/* end subroutine (mkindhdb) */

#endif /* CF_HDB */


#if	CF_STRTABLOOK

static int strtablook(pip,ofp,tab,it,itlen,nskip,fbp)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*tab ;
int		(*it)[3] ;
int		itlen ;
int		nskip ;
FILEMAP		*fbp ;
{
	FIELD		fsb ;

	hrtime_t	ht_start, ht_end ;

	int	rs = SR_OK ;
	int	ll, fl ;
	int	j ;
	int	sc = 0 ;

	const char	*lp, *fp ;


	if (pip == NULL) return SR_FAULT ;

	bprintf(ofp,"MKIND LOOK\n") ;

	rs = filemap_rewind(fbp) ;
	if (rs < 0)
	    goto ret1 ;

	ht_start = gethrvtime() ;

	while ((rs = filemap_getline(fbp,&lp)) > 0) {
	    ll = rs ;

	    if (lp[ll - 1] == '\n') ll -= 1 ;

	    if (strnchr(lp,ll,'&') != NULL) continue ;

	    if ((rs = field_start(&fsb,lp,ll)) >= 0) {

	        while ((fl = field_word(&fsb,NULL,&fp)) > 0) {

	            j = strtabfind(tab,it,itlen,nskip,fp,fl) ;

#if	CF_DEBUG && 0
	if (DEBUGLEVEL(3))
	    debugprintf("process/mkindhdb: strtabfind() j=%d\n",j) ;
#endif

	            sc += (j + 1) ;
	            if (j < 0) {
		        bprintf(ofp,"MKIND LOOK notfound=>%t<\n",fp,fl) ;
	                rs = SR_NOTFOUND ;
	                break ;
	            }

		    if (rs < 0)
			break ;

		} /* end while */

		field_finish(&fsb) ;
	    } /* end if (field) */

	    if (rs < 0) break ;
	} /* end while */

	ht_end = gethrvtime() ;

	if (rs >= 0) {
	    uint	tmsecs, msecs, secs ;
	    char	timebuf[TIMEBUFLEN + 1] ;

	    tmsecs = (ht_end - ht_start) / 1000000 ;
	    msecs = (tmsecs % 1000) ;
	    secs = (tmsecs / 1000) ;
	    bprintf(ofp,"MKIND LOOK elapsed=%s.%03u\n",
	        timestr_elapsed((time_t) secs,timebuf),msecs) ;

	} /* end if */

ret1:
	bprintf(ofp,"MKIND LOOK rs=%d sc=%u\n",rs,sc) ;

ret0:
	return (rs >= 0) ? sc : rs ;
}
/* end subroutine (strtablook) */

#endif /* CF_STRTABLOOK */


#if	CF_HASHLINK

#else /* CF_HASHLINK */

static int strtabfind(tab,it,itlen,nskip,sp,sl)
const char	tab[] ;
int		(*it)[3] ;
int		itlen ;
int		nskip ;
const char	*sp ;
int		sl ;
{
	uint	khash, nhash ;
	uint	chash ;

	int	hi ;
	int	nmax ;
	int	si = -1 ;
	int	j = 0 ;
	int	f = FALSE ;

	const char	*cp ;


	if (pip == NULL) return SR_FAULT ;

	nmax = itlen + nskip ;
	khash = hashelf(sp,sl) ;

	chash = khash & INT_MAX ;
	nhash = khash ;
	hi = hashindex(nhash,itlen) ;

#if	CF_DEBUGS && CF_DEBUGSTFIND
	debugprintf("strtabfind: il=%u ss>%t< shash=%08x hi=%u\n",
		itlen,sp,sl,khash,hi) ;
#endif

	for (j = 0 ; 
	    (j < nmax) && ((si = it[hi][0]) > 0) ;
	    j += 1) {

#if	CF_DEBUGS && CF_DEBUGSTFIND
	        cp = (tab + si) ;
	debugprintf("strtabfind: j=%u si=%u ts>%s< thash=%08x\n",
		j,si,cp,it[hi][1]) ;
#endif

	    f = ((it[hi][1] & INT_MAX) == chash) ;
	    if (f) {
	        cp = (tab + si) ;
	        f = (sp[0] == cp[0]) && ismatkey(cp,sp,sl) ;
	    }

	    if (f)
	        break ;

	    if ((it[hi][1] & (~ INT_MAX)) == 0)
		break ;

	    nhash = hashagain(nhash,j,nskip) ;

	    hi = hashindex(nhash,itlen) ;

#if	CF_DEBUGS && CF_DEBUGSTFIND
	debugprintf("strtabfind: next i=%u nhash=%08x hi=%u\n",
		j,nhash,hi) ;
#endif

	} /* end for */

#if	CF_DEBUGS && CF_DEBUGSTFIND
	debugprintf("strtabfind: ret f=%u j=%u\n",f,j) ;
#endif

	return (f) ? j : -1 ;
}
/* end subroutine (strtabfind) */

#endif /* CF_HASHLINK */


/* calculate the next hash from a given one */
static int hashindex(i,n)
uint	i ;
int	n ;
{
	int	hi ;


	hi = MODP2(i,n) ;

	if (hi == 0)
	    hi = 1 ;

	return hi ;
}
/* end subroutine (hashindex) */


/* check for weirdoness */
static int isweirdo(s,slen)
const char	s[] ;
int		slen ;
{
	int		i ;
	int		f = TRUE ;

	for (i = 0 ; (i < slen) && s[i] ; i += 1) {
	    const int	ch = MKCHAR(s[i]) ;
	    f = isprintlatin(ch) ;
	    if (!f) break ;
	} /* end for */

	return f ;
}
/* end subroutine (isweirdo) */


static int ismatkey(key,kp,kl)
const char	key[] ;
const char	kp[] ;
int		kl ;
{
	int	m ;
	int	f ;


	f = (key[0] == kp[0]) ;
	if (f) {
		m = nleadstr(key,kp,kl) ;
		f = (m == kl) && (key[m] == '\0') ;
	}

	return f ;
}
/* end subroutine (ismatkey) */


static int debugmallinfo(msp,name)
struct mallstate	*msp ;
const char		name[] ;
{


#if	CF_LIBMALLOC
	{
	    struct mallinfo	mi ;

	    debugprintf("PROG: mallinfo %s\n", name) ;

	    mi = mallinfo() ;

	    debugprintf("PROG: in-use small    %lu\n",
	        mi.usmblks) ;
	    debugprintf("PROG: in-use large    %lu\n",
	        mi.uordblks) ;
	    debugprintf("PROG: in-use total    %lu\n",
	        (mi.usmblks + mi.uordblks)) ;

	}
#endif /* CF_LIBMALLOC */

#if	CF_DEBUGMALL && CF_DEBUGS
	{
		uint	info[10] ;
		int	rs1 ;
		int	size = 10 * sizeof(uint) ;
		int	diff ;


	rs1 = uc_mallinfo(info,size) ;
	if (rs1 >= 0) {
	debugprintf("PROG: mallinfo %s used=%u\n",name,info[0]) ;
	debugprintf("PROG: mallinfo %s usedmax=%u\n",name,info[1]) ;
	debugprintf("PROG: mallinfo %s out=%u\n",name,info[2]) ;
	debugprintf("PROG: mallinfo %s under=%u\n",name,info[3]) ;
	debugprintf("PROG: mallinfo %s over=%u\n",name,info[4]) ;
	debugprintf("PROG: mallinfo %s nofreed=%u\n",name,info[5]) ;
	diff = (info[2] - msp->memout) ;
	debugprintf("PROG: mallinfo %s outdiff=%u\n",name,diff) ;
	} else
	debugprintf("PROG: mallinfo %s (%d)\n",name,rs1) ;
	}
#endif /* CF_DEBUGMALL */

	return 0 ;
}
/* end subroutine (debugmallinfo) */


static int debugmallmark(msp)
struct mallstate	*msp ;
{


#if	CF_LIBMALLOC
	msp->mi = mallinfo() ;
#endif /* CF_LIBMALLOC */

#if	CF_DEBUGMALL && CF_DEBUGS
	{
	    uint	memout ;
	    uc_mallout(&memout) ;
	    msp->memout = memout ;
	}
#endif /* CF_DEBUGMALL */

	return 0 ;
}
/* end subroutine (debugmallmark) */


static int debugmallstat(msp,name)
struct mallstate	*msp ;
const char		name[] ;
{
	uint	ts, te ;

	int	diff ;


#if	CF_LIBMALLOC
	{
	    struct mallinfo	mi ;
	    struct mallinfo	*mip = &msp->mi ;


	    mi = mallinfo() ;

	    ts = (mip->usmblks + mip->uordblks) ;
	    te = (mi.usmblks + mi.uordblks) ;
	    diff = (te - ts) ;

	    debugprintf("PROG: libmalloc %s outdiff=%u\n",
	        name,diff) ;

	}
#endif /* CF_LIBMALLOC */

#if	CF_DEBUGMALL && CF_DEBUGS
	uc_mallout(&te) ;
	ts = msp->memout ;
	diff = (te - ts) ;
	debugprintf("PROG: mallout %s memout=%u diff=%u\n",
	    name,te,diff) ;
#endif /* CF_DEBUGMALL */

	return 0 ;
}
/* end subroutine (debugmallstat) */


