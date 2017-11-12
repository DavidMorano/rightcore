/* main (BIBLESET) */

/* clean up the KJV bible as shipped from xxxx */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */
#define	CF_DEBUGMALL	1		/* debug memory allocations */
#define	CF_TROFFSETLL	0		/* set TROFF 'll' register */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This subroutine was adapted from someting else to be a simple front-end
	for this program.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ bibleset <file(s)> [-af <afile>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"progoff.h"
#include	"biblebook.h"
#include	"biblemeta.h"


/* local defines */

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#undef	HDRBUFLEN
#define	HDRBUFLEN	200

#ifndef	BIBLEMETA_LEN
#define	BIBLEMETA_LEN	100
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdecf(const char *,int,double *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	progoutbegin(PROGINFO *,bfile *) ;
extern int	progoutend(PROGINFO *,bfile *) ;
extern int	progfile(PROGINFO *,bfile *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct pvs {
	uint	ps ;
	uint	vs ;
} ;

struct vzlw {
	uint	width ;
	double	percent ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,
			cchar *,cchar *,cchar *,cchar *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,void *,cchar *) ;
static int	procout(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procpagetitle(PROGINFO *) ;

static int	loadpvs(PROGINFO *,const char *,int) ;
static int	loadvzlw(PROGINFO *,const char *,int) ;

static int	metawordsbegin(PROGINFO *) ;
static int	metawordsend(PROGINFO *) ;

#ifdef	COMMENT
static int	defvzlinewidth(PROGINFO *) ;
#endif

static char	*firstup(char *) ;
static char	*alldown(char *) ;


/* local variables */

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"ndb",
	"vdb",
	"wdb",
	"ps",
	"vs",
	"vzlw",
	"frontmatter",
	"backmatter",
	"ibz",
	"ff",
	"cover",
	"tc",
	"pt",
	"title",
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
	argopt_ndb,
	argopt_vdb,
	argopt_wdb,
	argopt_ps,
	argopt_vs,
	argopt_vzlw,
	argopt_frontmatter,
	argopt_backmatter,
	argopt_ibz,
	argopt_ff,
	argopt_cover,
	argopt_tc,
	argopt_pt,
	argopt_title,
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

static cchar	*akonames[] = {
	"hyphenate",
	"ha",
	"ps",
	"vs",
	"vzlw",
	"font",
	"back",
	"pagenums",
	"ibz",
	"tc",
	"pt",
	"title",
	NULL
} ;

enum akonames {
	akoname_hyphenate,
	akoname_ha,
	akoname_ps,
	akoname_vs,
	akoname_vzlw,
	akoname_front,
	akoname_back,
	akoname_pagenums,
	akoname_ibz,
	akoname_tc,
	akoname_pt,
	akoname_title,
	akoname_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		v ;
	int		wlen = 0 ;
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
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ndbname = NULL ;
	const char	*vdbname = NULL ;
	const char	*wdbname = NULL ;
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

/* initialize */

	pip->verboselevel = 1 ;
	pip->columns = 0 ;
	pip->linewidth = 0 ;
	pip->vzlw = 0 ;
	pip->vzlb = 0 ;

	pip->f.pagenums = FALSE ;
	pip->f.hyphenate = TRUE ;
	pip->f.ha = TRUE ;
	pip->f.ibz = OPT_IBZ ;

	if (pip->daytime == 0) pip->daytime = time(NULL) ;

/* start parsing the arguments */

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

/* argument filename */
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

/* bible-name DB name */
	                case argopt_ndb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ndbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ndbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* bible-meta DB name */
	                case argopt_wdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            wdbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                wdbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* bible-verse DB name */
	                case argopt_vdb:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            vdbname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                vdbname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* point-size */
	                case argopt_ps:
	                    cp = NULL ;
	                    cl = 0 ;
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
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        rs = loadpvs(pip,cp,cl) ;
	                    }
	                    break ;

/* vertical-spacing */
	                case argopt_vs:
	                    cp = NULL ;
	                    cl = 0 ;
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
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        pip->have.ps = TRUE ;
	                        pip->final.ps = TRUE ;
	                        rs = optvalue(cp,cl) ;
	                        pip->vs = rs ;
	                    }
	                    break ;

/* verse-zero line-width */
	                case argopt_vzlw:
	                    cp = NULL ;
	                    cl = 0 ;
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
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        rs = loadvzlw(pip,cp,cl) ;
	                    }
	                    break ;

/* front-matter */
	                case argopt_frontmatter:
	                    cp = NULL ;
	                    cl = 0 ;
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
	                    } /* end if */
	                    pip->have.frontmatter = TRUE ;
	                    pip->final.frontmatter = TRUE ;
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        pip->f.frontmatter = TRUE ;
	                        pip->frontfname = cp ;
	                    }
	                    break ;

/* back-matter */
	                case argopt_backmatter:
	                    cp = NULL ;
	                    cl = 0 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } /* end if */
	                    pip->have.backmatter = TRUE ;
	                    pip->final.backmatter = TRUE ;
	                    pip->f.backmatter = TRUE ;
	                    if ((cp != NULL) && (cl > 0)) {
	                        rs = optbool(cp,cl) ;
	                        pip->f.backmatter = (rs > 0) ;
	                    }
	                    break ;

/* ignore-book-zero */
	                case argopt_ibz:
	                    cp = NULL ;
	                    cl = 0 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } /* end if */
	                    pip->have.ibz = TRUE ;
	                    pip->final.ibz = TRUE ;
	                    pip->f.ibz = TRUE ;
	                    if ((cp != NULL) && (cl > 0)) {
	                        rs = optbool(cp,cl) ;
	                        pip->f.ibz = (rs > 0) ;
	                    }
	                    break ;

/* font-family */
	                case argopt_ff:
	                    cp = NULL ;
	                    cl = 0 ;
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
	                    } /* end if */
	                    pip->have.ff = TRUE ;
	                    pip->final.ff = TRUE ;
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        pip->ff = cp ;
	                    }
	                    break ;

/* cover EPS file */
	                case argopt_cover:
	                    cp = NULL ;
	                    cl = 0 ;
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
	                    } /* end if */
	                    pip->have.ff = TRUE ;
	                    pip->final.ff = TRUE ;
#if	CF_DEBUGS
	                    debugprintf("main: cover c=%t\n",cp,cl) ;
#endif
	                    if ((rs >= 0) && (cp != NULL) && (cl > 0)) {
	                        pip->coverfname = cp ;
	                    }
	                    break ;

/* table-contents */
	                case argopt_tc:
	                    cp = NULL ;
	                    cl = 0 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } /* end if */
	                    pip->have.tc = TRUE ;
	                    pip->final.tc = TRUE ;
	                    pip->f.tc = TRUE ;
	                    if ((cp != NULL) && (cl > 0)) {
	                        rs = optbool(cp,cl) ;
	                        pip->f.tc = (rs > 0) ;
	                    }
	                    break ;

/* title for pages */
	                case argopt_pt:
	                case argopt_title:
	                    cp = NULL ;
	                    cl = 0 ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            cp = avp ;
	                            cl = avl ;
	                        }
	                    } /* end if */
	                    pip->have.pagetitle = TRUE ;
	                    pip->final.pagetitle = TRUE ;
	                    pip->f.pagetitle = TRUE ;
	                    if ((cp != NULL) && (cl > 0)) {
	                        pip->pagetitle = cp ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    f_usage = TRUE ;
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

/* line-width */
	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->linewidth = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        f_usage = TRUE ;
	                        rs = SR_INVALID ;
	                        break ;

	                    } /* end switch */
	                    akp += 1 ;

	                    if (rs < 0) break ;
	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or options) */

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
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
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

/* program search name */

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

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if ((rs >= 0) && (pip->linewidth == 0) && (argval != NULL)) {
	    rs = cfdeci(argval,-1,&v) ;
	    pip->linewidth = v ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (ofname == NULL) ofname = getenv(VAROFNAME) ;

/* load up the environment options */

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* more initialization */

	if (pip->linewidth == 0)
	    pip->linewidth = LINEWIDTH ;

	if (pip->vzlw == 0)
	    pip->vzlw = VERSEZEROLEN ;

	if ((pip->vzlb == 0) || (pip->vzlb > pip->vzlw))
	    pip->vzlb = (pip->vzlw - 4) ;

	if (pip->ps == 0)
	    pip->ps = DEFPS ;

	if (pip->vs == 0)
	    pip->vs = (pip->ps + 2) ;

	if (! pip->have.hyphenate) {
	    pip->have.hyphenate = TRUE ;
	    pip->f.hyphenate = TRUE ;
	}

	if (! pip->have.ha) {
	    pip->have.ha = TRUE ;
	    pip->f.ha = TRUE ;
	}

/* if no font-family was specified on invocation -> assume "Times" */

	if ((rs >= 0) && ((pip->ff == NULL) || (pip->ff[0] == '\0'))) {
	    const char	**vpp = &pip->ff ;
	    rs = proginfo_setentry(pip,vpp,"T",-1) ;
	}

/* other */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* debug-print out some configuration values */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: lw=%u\n",
	        pip->progname,
	        pip->linewidth) ;
	    bprintf(pip->efp,"%s: vzlw=%u vzlb=%u\n",
	        pip->progname,
	        pip->vzlw,pip->vzlb) ;
	    bprintf(pip->efp,"%s: ps=%u vs=%u\n",
	        pip->progname,
	        pip->ps,pip->vs) ;
	} /* end if */

/* continue initialize */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: bible-book-name=%s\n",
	        pip->progname,ndbname) ;
	}

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    cchar	*ofn = ofname ;
	    cchar	*afn = ofname ;
	    rs = process(pip,&ainfo,&pargs,ndbname,wdbname,ofn,afn) ;
	    wlen += rs ;
	    if ((rs >= 0) && (pip->debuglevel > 0)) {
		cchar *fmt = "%s: data written=%d\n" ;
	        bprintf(pip->efp,fmt,pip->progname,wlen) ;
	    }
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    shio_printf(pip->efp,fmt,pn,rs) ;
	    ex = EX_USAGE ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            cchar	*fmt = "%s: invalid text encountered (%d)\n" ;
	            bprintf(pip->efp,fmt,pip->progname,rs) ;
	        }
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
	} /* end if */

/* early return thing */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u rs=%d\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    rs1 = bclose(pip->efp) ;
	    if (rs >= 0) rs = rs1 ;
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
	rs1 = proginfo_finish(pip) ;
	if (rs >= 0) rs = rs1 ;

badprogstart:
	if ((rs < 0) && (ex == EX_OK)) ex = EX_SOFTWARE ;

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


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<file(s)> ...]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_hyphenate:
	                    if (! pip->have.hyphenate) {
	                        pip->have.hyphenate = TRUE ;
	                        pip->final.hyphenate = TRUE ;
	                        pip->f.hyphenate = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.hyphenate = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_ha:
	                    if (! pip->have.ha) {
	                        pip->have.ha = TRUE ;
	                        pip->final.ha = TRUE ;
	                        pip->f.ha = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.ha = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_ps:
	                    if (vl > 0) {
	                        rs = loadpvs(pip,vp,vl) ;
			    }
	                    break ;

	                case akoname_vs:
	                    if (! pip->have.vs) {
	                        pip->have.vs = TRUE ;
	                        pip->final.vs = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            pip->f.vs = rs ;
	                        }
	                    }
	                    break ;

	                case akoname_vzlw:
	                    if (vl > 0) {
	                        rs = loadvzlw(pip,vp,vl) ;
			    }
	                    break ;

	                case akoname_front:
	                    if (! pip->have.frontmatter) {
	                        pip->have.frontmatter = TRUE ;
	                        pip->final.frontmatter = TRUE ;
	                        pip->f.frontmatter = TRUE ;
	                        if (vl > 0) {
	                            const char	**vpp = &pip->frontfname ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                            pip->f.frontmatter = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_back:
	                    if (! pip->have.backmatter) {
	                        pip->have.backmatter = TRUE ;
	                        pip->final.backmatter = TRUE ;
	                        pip->f.backmatter = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.backmatter = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_pagenums:
	                    if (! pip->have.pagenums) {
	                        pip->have.pagenums = TRUE ;
	                        pip->final.pagenums = TRUE ;
	                        pip->f.pagenums = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.pagenums = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_ibz:
	                    if (! pip->have.ibz) {
	                        pip->have.ibz = TRUE ;
	                        pip->final.ibz = TRUE ;
	                        pip->f.ibz = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.ibz = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_tc:
	                    if (! pip->have.tc) {
	                        pip->have.tc = TRUE ;
	                        pip->final.tc = TRUE ;
	                        pip->f.tc = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.tc = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_pt:
	                case akoname_title:
	                    if (! pip->have.pagetitle) {
	                        pip->have.pagetitle = TRUE ;
	                        pip->final.pagetitle = TRUE ;
	                        pip->f.pagetitle = TRUE ;
	                        if (vl > 0) {
	                            const char	**vpp = &pip->pagetitle ;
	                            rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                            pip->f.pagetitle = (rs > 0) ;
	                        }
	                    }
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


static int process(pip,aip,bop,ndb,wdb,ofn,afn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
cchar		*ndb ;
cchar		*wdb ;
cchar		*ofn ;
cchar		*afn ;
{
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = biblebook_open(&pip->bb,pip->pr,ndb)) >= 0) {
	    pip->open.biblebook = TRUE ;

	    if ((rs = biblemeta_open(&pip->bm,pip->pr,wdb)) >= 0) {
	        pip->open.biblemeta = TRUE ;

	        if ((rs = metawordsbegin(pip)) >= 0) {

		    if ((rs = procpagetitle(pip)) >= 0) {
	                rs = procout(pip,aip,bop,ofn,afn) ;
	                wlen += rs ;
	            }

	            rs1 = metawordsend(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (metawords) */

	        pip->open.biblemeta = FALSE ;
	        rs1 = biblemeta_close(&pip->bm) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        bprintf(pip->efp,"%s: bible meta-word DB unavailable (%d)\n",
	            pip->progname,rs) ;
	    }

	    pip->open.biblebook = FALSE ;
	    rs1 = biblebook_close(&pip->bb) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    bprintf(pip->efp,"%s: bible book-name DB unavailable (%d)\n",
	        pip->progname,rs) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procout(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {

	    if ((rs = progoffbegin(pip,ofp)) >= 0) {
	        wlen += rs ;
	        if ((rs = progoutbegin(pip,ofp)) >= 0) {
	            wlen += rs ;
	            if ((rs = procargs(pip,aip,bop,ofp,afn)) >= 0) {
	                const char	*fmt = "%s BIBLESET %s completed\n" ;
	                char		timebuf[TIMEBUFLEN+1] ;
	                timestr_logz(pip->daytime,timebuf) ;
	                rs = bprintf(ofp,fmt,pip->troff.linecomment,timebuf) ;
	                wlen += rs ;
	            }
	            rs1 = progoutend(pip,ofp) ;
	            if (rs >= 0) rs = rs1 ;
	            wlen += rs1 ;
	        } /* end if (progout) */
	        rs1 = progoffend(pip) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (procoff) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: output unavailable (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    bprintf(pip->efp,"%s: output bytes=%u\n",
	        pip->progname,wlen) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,void *ofp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		cl ;
	int		pan = 0 ;
	int		wlen = 0 ;
	const char	*cp ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

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
	                rs = progfile(pip,ofp,cp) ;
	                wlen += rs ;
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
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	                    pan += 1 ;
	                    lbuf[(cp-lbuf)+cl] = '\0' ;
	                    rs = progfile(pip,ofp,cp) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        if (! pip->f.quiet) {
		    fmt = "%s: inaccessible argument-file (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s:  afile=%s\n",pn,afn) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = progfile(pip,ofp,cp) ;
	    wlen += rs ;

	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procpagetitle(PROGINFO *pip)
{
	int		rs = SR_OK ;
	if ((pip->pagetitle == NULL) && (! pip->final.pagetitle)) {
	                const int	blen = BIBLEBOOK_LEN ;
	                char		bbuf[BIBLEBOOK_LEN + 1] ;
	                if ((rs = biblebook_get(&pip->bb,0,bbuf,blen)) >= 0) {
	                    const char	**vpp = &pip->pagetitle ;
	                    rs = proginfo_setentry(pip,vpp,bbuf,rs) ;
	                    pip->have.pagetitle = (rs >= 0) ;
	                }
	} /* end if */
	if (pip->pagetitle == NULL) pip->pagetitle = DEFTITLE ;
	return rs ;
}
/* end subroutine (procpagetitle) */


#ifdef	COMMENT
static int defvzlinewidth(PROGINFO *pip)
{
	double		flw ;
	int		rs = SR_OK ;
	int		w ;

	flw = pip->linewidth ;
	w = (flw * 0.80) ;

	return (rs >= 0) ? w : rs ;
}
/* end subroutine (defvzlinewidth) */
#endif /* COMMENT */


static int loadpvs(PROGINFO *pip,cchar *ap,int al)
{
	uint		ps = 0 ;
	uint		vs = 0 ;
	int		rs = SR_OK ;
	int		sl, cl ;
	const char	*tp ;
	const char	*sp, *cp ;

	if ((sl = sfshrink(ap,al,&sp)) > 0) {

	    cp = NULL ;
	    cl = -1 ;
	    if ((tp = strnchr(sp,sl,':')) != NULL) {

	        cp = (tp + 1) ;
	        cl = ((sp + sl) - (tp + 1)) ;
	        if (cl > 0)
	            rs = cfdecui(cp,cl,&vs) ;

	        sl = (tp - sp) ;

	    } /* end if */

	    if ((rs >= 0) && (sl > 0))
	        rs = cfdecui(sp,sl,&ps) ;

	    if (rs >= 0) {
	        if ((ps > 0) && (! pip->final.ps)) {
	            pip->final.ps = TRUE ;
	            pip->have.ps = TRUE ;
	            pip->ps = ps ;
	        }
	        if (! pip->final.vs) {
	            if ((vs == 0) && (ps > 0))
	                vs = (ps + 2) ;
	            if (vs > 0) {
	                pip->final.vs = TRUE ;
	                pip->have.vs = TRUE ;
	                pip->vs = vs ;
	            }
	        }
	    } /* end if */

	} /* end if */

	return rs ;
}
/* end subroutine (loadpvs) */


static int loadvzlw(PROGINFO *pip,cchar *ap,int al)
{
	int		rs = SR_OK ;
	int		sl ;
	const char	*sp ;

	if ((sl = sfshrink(ap,al,&sp)) > 0) {
	    double	percent = 0.0 ;
	    uint	vzlw = 0 ;
	    uint	vzlb = 0 ;
	    int		cl = -1 ;
	    const char	*tp ;
	    cchar	*cp = NULL ;

	    if ((tp = strnchr(sp,sl,':')) != NULL) {

	        cp = (tp + 1) ;
	        cl = ((sp + sl) - (tp + 1)) ;
	        if (cl > 0)
	            rs = cfdecf(cp,cl,&percent) ;

	        sl = (tp - sp) ;

	    } /* end if */

	    if ((rs >= 0) && (sl > 0))
	        rs = cfdecui(sp,sl,&vzlw) ;

	    if (rs >= 0) {

	        if (cp != NULL) {
	            vzlb = (uint) (((double) vzlw) * percent) ;
	        }

	        if ((vzlw > 0) && (! pip->final.vzlw)) {
	            pip->final.vzlw = TRUE ;
	            pip->have.vzlw = TRUE ;
	            pip->vzlw = vzlw ;
	        }
	        if ((vzlb > 0) && (! pip->final.vzlb)) {
	            pip->final.vzlb = TRUE ;
	            pip->have.vzlb = TRUE ;
	            pip->vzlb = vzlb ;
	        }

	    } /* end if */

	} /* end if (sfshrink) */

	return rs ;
}
/* end subroutine (loadvzlw) */


static int metawordsbegin(PROGINFO *pip)
{
	const int	wordlen = BIBLEMETA_LEN ;
	int		rs = SR_OK ;
	int		mi ;
	int		len ;
	const char	*cp ;
	char		wordbuf[BIBLEMETA_LEN + 1] ;
	char		*wp ;

	for (mi = 0 ; (rs >= 0) && (mi < biblemeta_overlast) ; mi += 1) {
	    if ((rs = biblemeta_get(&pip->bm,mi,wordbuf,wordlen)) >= 0) {
	        len = rs ;
	        if ((rs = uc_mallocstrw(wordbuf,len,&cp)) >= 0) {
	    	    wp = (char *) cp ;
	    	    switch (mi) {
	    	    case biblemeta_chapter:
			pip->word[word_chapter] = firstup(wp) ;
			break ;
	    	    case biblemeta_psalm:
	        	pip->word[word_psalm] = firstup(wp) ;
	        	break ;
	    	    case biblemeta_bookindex:
	        	pip->word[word_bookindex] = alldown(wp) ;
	        	break ;
	    	    case biblemeta_page:
	        	pip->word[word_page] = alldown(wp) ;
	        	break ;
	    	    case biblemeta_booktitle:
	        	pip->word[word_booktitle] = alldown(wp) ;
	                break ;
	            case biblemeta_thebookof:
	        	pip->word[word_thebookof] = firstup(wp) ;
	        	break ;
	    	    case biblemeta_book:
	        	pip->word[word_book] = alldown(wp) ;
	        	break ;
	    	    } /* end switch */
		} /* end if (m-a) */
	    } /* end if (fetch) */
	} /* end for */

	if (rs < 0) {
	    metawordsend(pip) ;
	    if (rs != SR_NOMEM)
	        bprintf(pip->efp,"%s: problem loading from metaword DB (%d)\n",
	            pip->progname,rs) ;
	}

	return rs ;
}
/* end subroutine (metawordsbegin) */


static int metawordsend(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		mi ;

	for (mi = 0 ; mi < word_overlast ; mi += 1) {
	    if (pip->word[mi] != NULL) {
	        rs1 = uc_free(pip->word[mi]) ;
	        if (rs >= 0) rs = rs1 ;
	        pip->word[mi] = NULL ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (metawordsend) */


static char *firstup(char *bp)
{
	if (*bp != '\0') {
	    int	nch ;
	    int	ch = (*bp & 0xff) ;
	    nch = CHAR_TOUC(ch) ;
	    if (ch != nch) *bp = nch ;
	    alldown(bp+1) ;
	}
	return bp ;
}
/* end subroutine (firstup) */


static char *alldown(char *bp)
{
	if (*bp != '\0') {
	    int		ch ;
	    int		nch ;
	    int		i ;
	    for (i = 0 ; bp[i] ; i += 1) {
	        ch = (bp[i] & 0xff) ;
	        nch = CHAR_TOLC(ch) ;
	        if (ch != nch) bp[i] = nch ;
	    } /* end for */
	}
	return bp ;
}
/* end subroutine (alldown) */


