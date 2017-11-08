/* main (varsub) */

/* variable-substituting */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_AUDIT	1		/* ¿some kind of audit */


/* revision history:

	- 1991-11-01, David A­D­ Morano
	This was a total rewrite from scratch of the old SATSIM filter
	program.  This should be quite a bit faster for such a simple function
	(although Rick Meis was a cleavor guy when we wrote his version).

	- 1995-10-01, David A­D­ Morano
	This was adapted for use as a general macro variable expansion filter.
	No real program changes have been made (including the comments below).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program converts extension and circuit pack locations in SATSIM
	scripts (.ap files) to model specific locations at script run time.  It
	reads the model file (dr06_CP.s for instance) to determine what
	extension perfix and carrier-slot location to replace in the file.

	Synopsis:

	$ varsub [-s <var>=<value>] [-f <filter_file>] [<input_file>]


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
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<field.h>
#include	<varsub.h>
#include	<nulstr.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	KEYLEN		MAXPATHLEN

#define	SUBOPTS		struct subopts


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	varsub_loadfile(VARSUB *,const char *) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct subopts {
	uint		blanks:1 ;
	uint		badnokey:1 ;
	uint		paren:1 ;
	uint		brace:1 ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,
			VARSUB *,cchar *,cchar *,cchar *) ;

static int	procfiles(PROGINFO *,VARSUB *,bfile *,const char *,int) ;
static int	procfile(PROGINFO *,VARSUB *,bfile *,const char *,int) ;
static int	getkeychars(PROGINFO *,SUBOPTS *,const char *) ;
static int	loadsubstrs(PROGINFO *,VARSUB *,VECSTR *) ;
static int	vecstr_addpair(VECSTR *,const char *,int) ;

#ifdef	COMMENT
static int	getconfig() ;
static int	ourfreeze(PROGINFO *,bfile *,char *,char *) ;
static int	fullpath(const char *,char *,int) ;

static void	mkfzname(const char *,int,const char *) ;
#endif /* COMMENT */

#if	CF_DEBUG 
static int	debugsubs(PROGINFO *,VARSUB *) ;
#endif


/* local variables */

static cchar	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	"db",
	"env",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_db,
	argopt_env,
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

static cchar	*progmodes[] = {
	"varsub",
	NULL
} ;

enum progmodes {
	progmode_varsub,
	progmode_overlast
} ;

static cchar	*akonames[] = {
	"badnokey",
	"blanks",
	NULL
} ;

enum akopts {
	akoname_badnokey,
	akoname_blanks,
	akoname_overlast
} ;

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


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	SUBOPTS		sopts ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	VECSTR		substrs ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		opts ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_env = FALSE ;
	int		f_remove = TRUE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*pmspec = NULL ;
	const char	*keychars = NULL ;
	const char	*ffname = NULL ;
	const char	*cp ;
	char		fzfname[MAXPATHLEN + 1] ;
	char		fbuf[FBUFLEN + 1] ;
	char		keychar[4] ;

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

/* continue with initialization */

	pip->verboselevel = 1 ;

	memset(&sopts,0,sizeof(SUBOPTS)) ;

	fzfname[0] = '\0' ;
	fbuf[0] = '\0' ;
	keychar[0] = '\0' ;

/* the default is to allow for blank substitution */

	sopts.blanks = TRUE ;

/* start processing the arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	if (rs >= 0) {
	    rs = vecstr_start(&substrs,2,0) ;
	    pip->open.substrs = (rs >= 0) ;
	}

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

/* database file */
	                case argopt_db:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ffname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ffname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
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

	                case argopt_env:
	                    f_env = TRUE ;
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
	                        if (f_optequal)
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'b':
	                        sopts.blanks = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                sopts.blanks = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* filter file name */
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ffname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* key characters */
	                    case 'k':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                keychars = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
					VECSTR	*ssp = &substrs ;
	                                rs = vecstr_addpair(ssp,argp,argl) ;
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

	                    case 'z':
	                        f_remove = FALSE ;
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

	if (f_usage)
	    usage(pip) ;

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
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
	} /* end if */

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

	if (pip->progmode < 0)
	    pip->progmode = progmode_varsub ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUG */

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (ofname == NULL) ofname = getenv(VAROFNAME) ;

	if (rs >= 0) {
	    rs = procopts(pip,&akopts) ;
	}

/* check the arguments */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

#ifdef	COMMENT
	if (ifname == NULL)
	    ifname = STDINFNAME) ;
#endif

/* program options */

#ifdef	COMMENT
	if (rs >= 0) {
	    KEYOPT_CUR	cur ;
	    uint	v ;

	    keyopt_curbegin(&akopts,&cur) ;

	    while ((sl = keyopt_enumkeys(&akopts,&cur,&sp)) >= 0) {
	        if (sp == NULL) continue ;

	        if ((kwi = matostr(akopts,2,sp,sl)) >= 0) {

	            rs1 = keyopt_fetch(&akopts,sp,NULL,&cp) ;
	            cl = rs1 ;

	            if (rs1 >= 0) {
	                switch (kwi) {
	                case akoname_badnokey:
	                    if (cl > 0) {
	                        rs1 = cfdecui(cp,cl,&v) ;
	                        if (rs1 >= 0)
	                            sopts.badnokey = ((v > 0) ? 1 : 0) ;
	                    } else
	                        sopts.badnokey = TRUE ;
	                    break ;
	                case akoname_blanks:
	                    if (cl > 0) {
	                        rs1 = cfdecui(cp,cl,&v) ;
	                        if (rs1 >= 0)
	                            sopts.blanks = ((v > 0) ? 1 : 0) ;
	                    } else
	                        sopts.blanks = TRUE ;
	                    break ;
	                } /* end switch */
	            } /* end if (got an option key) */

	        } /* end if (match) */

	    } /* end while (options keys) */

	    keyopt_curend(&akopts,&cur) ;
	} /* end if (akopts) */
#endif /* COMMENT */

/* key characters */

	if (rs >= 0) {
	    rs = 0 ;
	    if (keychars != NULL) {
	        rs = getkeychars(pip,&sopts,keychars) ;
	    } 
#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: keychars n=%d f_paren=%u f_brace=%u\n",
	            rs,sopts.paren,sopts.brace) ;
#endif
	    if (rs == 0) {
	        sopts.paren = TRUE ;
	        sopts.brace = TRUE ;
	    }
	}


/* find a DB filename */

	if (ffname == NULL)
	    ffname = getenv(VARMAP) ;

	if (ffname != NULL) {

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: filter file=%s\n",
	            pip->progname,ffname) ;
	    }

	    if (rs >= 0)
	        rs = u_access(ffname,R_OK) ;

	    if (rs < 0) {
	        ex = EX_BADFILTER ;
	        bprintf(pip->efp,"%s: filter file unavailable (%d)\n",
	            pip->progname,rs) ;
	    }

	} /* end if (filter file) */

/* check the freeze file */

#ifdef	COMMENT
	size = NFILTERS * sizeof(struct fzentry) ;
	memset(fe,0,size) ;

	rs = getconfig(pip,ffname,fe,fbuf,&nfe,&fbuflen,fzfname) ;

	switch (rs) {

	case GC_BADPATH:
	    bprintf(pip->efp,"%s: no full path of filter file\n",
	        pip->progname) ;
	    break ;

	case GC_TOOMANY:
	    bprintf(pip->efp,"%s: too many filter entries\n",
	        pip->progname) ;
	    break ;

	case GC_TOOMUCH:
	    bprintf(pip->efp,"%s: ran out of string space\n",
	        pip->progname) ;
	    break ;

	case GC_MAXTMP:
	    bprintf(pip->efp,"%s: maximum temporary file attempts\n",
	        pip->progname) ;
	    break ;

	} /* end switch */

	if (rs < 0)
	    goto badfreeze ;
#endif /* COMMENT */

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* substitution options */

	opts = 0 ;
	if (sopts.blanks) {
	    opts &= (~ VARSUB_MNOBLANK) ;
	} else {
	    opts |= VARSUB_MNOBLANK ;
	}

	if (sopts.badnokey)
	    opts |= VARSUB_MBADNOKEY ;

	if (sopts.paren)
	    opts |= VARSUB_MPAREN ;

	if (sopts.brace)
	    opts |= VARSUB_MBRACE ;


	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    VARSUB	subs ;
	    if ((rs = varsub_start(&subs,opts)) >= 0) {
	        if ((rs = loadsubstrs(pip,&subs,&substrs)) >= 0) {
	            if (ffname != NULL) {
	                rs = varsub_loadfile(&subs,ffname) ;
	            }
	            if (rs >= 0) {
	                if (f_env) {
	                    rs = varsub_addva(&subs,pip->envv) ;
	                }
	                if (rs >= 0) {
			    ARGINFO	*aip = &ainfo ;
			    BITS	*bop = &pargs ;
	                    cchar	*ofn = ofname ;
	                    cchar	*afn = afname ;
	                    cchar	*ifn = ifname ;
#if	CF_DEBUG
	                    debugsubs(pip,&subs) ;
#endif

	                    rs = procargs(pip,aip,bop,&subs,ofn,afn,ifn) ;

	                } /* end if (ok) */
	            } else {
	                ex = EX_DATAERR ;
	                bprintf(pip->efp,"%s: inaccesssible filter (%d)\n",
	                    pip->progname,rs) ;
	            }
	        } /* end if (loadsubstrs) */
	        rs1 = varsub_finish(&subs) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        ex = EX_OSERR ;
	        bprintf(pip->efp,"%s: usage (%d)\n",pip->progname,rs) ;
	    }
	} else {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} else {
	    ex = ((! sopts.blanks) && sopts.badnokey) ? EX_BADNOKEY : EX_OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: done rs=%d\n",rs) ;
#endif

	if (f_remove && (fzfname[0] != '\0')) {
	    u_unlink(fzfname) ;
	    fzfname[0] = '\0' ;
	}

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.substrs) {
	    pip->open.substrs = FALSE ;
	    vecstr_finish(&substrs) ;
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
	    debugprintf("b_shcat: final mallout=%u\n",(mo-mo_start)) ;
	    uc_mallset(0) ;
	}
#endif /* CF_DEBUGMALL */

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


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-db <map>] [<file(s)>] [-of <ofile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-s <var>=<value>] [-k <keychar>] [-z]\n" ;
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

	                c += 1 ;
	                switch (oi) {
	                case akoname_badnokey:
	                    if (! pip->final.badnokey) {
	                        pip->have.badnokey = TRUE ;
	                        pip->final.badnokey = TRUE ;
	                        pip->f.badnokey = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.badnokey = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                case akoname_blanks:
	                    if (! pip->final.blanks) {
	                        pip->have.blanks = TRUE ;
	                        pip->final.blanks = TRUE ;
	                        pip->f.blanks = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            pip->f.blanks = (rs > 0) ;
	                        }
	                    }
	                    break ;
	                } /* end switch */

	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(pip,aip,bop,vsp,ofn,afn,ifn)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
VARSUB		*vsp ;
const char	*ofn ;
const char	*afn ;
const char	*ifn ;
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp = NULL ;

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
	                    rs = procfile(pip,vsp,ofp,cp,-1) ;
	                    wlen += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for */

	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (afn[0] == '-') afn = BFILE_STDIN ;

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
				lbuf[(cp+cl)-lbuf] = '\0' ;
	                        pan += 1 ;
	                        rs = procfiles(pip,vsp,ofp,cp,cl) ;
	                        wlen += rs ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
			fmt = "%s: inaccessible argument-list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (ifn != NULL) && (ifn[0] != '\0')) {

	        cp = ifn ;
	        pan += 1 ;
	        rs = procfile(pip,vsp,ofp,cp,-1) ;
	        wlen += rs ;

	    } /* end if (processing standard input) */

	    if ((rs >= 0) && (pan == 0)) {

	        cp = STDINFNAME ;
	        pan += 1 ;
	        rs = procfile(pip,vsp,ofp,cp,-1) ;
	        wlen += rs ;

	    } /* end if (processing standard input) */

	    if ((rs < 0) && (cp != NULL)) {
	        fmt = "%s: processing error (%d) file=%s\n" ;
	        bprintf(pip->efp,fmt,pn,rs,cp) ;
	    }

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccesile output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procfiles(pip,vsp,ofp,lbuf,llen)
PROGINFO	*pip ;
VARSUB		*vsp ;
bfile		*ofp ;
const char	*lbuf ;
int		llen ;
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    const char	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procfile(pip,vsp,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfiles) */


static int procfile(pip,vsp,ofp,fp,fl)
PROGINFO	*pip ;
VARSUB		*vsp ;
bfile		*ofp ;
const char	*fp ;
int		fl ;
{
	NULSTR		n ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*fname ;

	if (pip == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: fname=%s\n",fname) ;
#endif

/* open the files to operate on */

	if ((fp == NULL) || (fp[0] == '\0') || (fp[0] == '-'))
	    fp = BFILE_STDIN ;

	if ((rs = nulstr_start(&n,fp,fl,&fname)) >= 0) {
	    bfile	infile, *ifp = &infile ;
	    if ((rs = bopen(ifp,fname,"r",0666)) >= 0) {
		{
	            rs = varsub_expandfile(vsp,ifp,ofp) ;
	            wlen += rs ;
		}
	        rs1 = bclose(ifp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (file) */
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


/* what are the key characters */
static int getkeychars(PROGINFO *pip,SUBOPTS *sop,cchar *keychars)
{
	int		n = 0 ;
	const char	*cp = keychars ;

	if (pip == NULL) return SR_FAULT ;

	if ((keychars != NULL) && (keychars[0] != '\0')) {
	    while (*cp != '\0') {

	        while (CHAR_ISWHITE(*cp))
	            cp += 1 ;

	        switch (*cp) {
	        case 'p':
	            sop->paren = TRUE ;
	            break ;
	        case 'b':
	            sop->brace = TRUE ;
	            break ;
	        } /* end switch */

	        while (*cp && (*cp != ',') && (! CHAR_ISWHITE(*cp))) {
	            cp += 1 ;
	        }

	        if (*cp == ',')
	            cp += 1 ;

	    } /* end while */
	    if (sop->paren) n += 1 ;
	    if (sop->brace) n += 1 ;
	} /* end if (non-empty) */

	return n ;
}
/* end subroutine (getkeychars) */


/* load up invocation-argument substitutions */
static int loadsubstrs(pip,subp,substrp)
PROGINFO	*pip ;
VARSUB		*subp ;
VECSTR		*substrp ;
{
	int		rs = SR_OK ;
	int		i ;
	int		kl, vl ;
	int		c = 0 ;
	const char	*tp, *sp ;
	const char	*kp, *vp ;

	if (pip == NULL) return SR_FAULT ;

	for (i = 0 ; vecstr_get(substrp,i,&sp) >= 0 ; i += 1) {
	    if (sp == NULL) continue ;

	    kp = sp ;
	    kl = -1 ;
	    vp = NULL ;
	    vl = 0 ;
	    if ((tp = strchr(sp,'=')) != NULL) {
	        kl = (tp - sp) ;
	        vp = (tp + 1) ;
	        vl = -1 ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: VS key=%t\n",kp,strnlen(kp,kl)) ;
	        if (vp != NULL)
	            debugprintf("main: VS val=>%t<\n",vp,strnlen(vp,vl)) ;
	    }
#endif

	    c += 1 ;
	    rs = varsub_add(subp,kp,kl,vp,vl) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: varsub_add() rs=%d\n",rs) ;
#endif

	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadsubstrs) */


static int vecstr_addpair(op,sp,sl)
VECSTR		*op ;
const char	sp[] ;
int		sl ;
{
	int		rs ;
	int		kl, vl ;
	const char	*kp, *vp ;
	const char	*tp ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0)
	    sl = strlen(sp) ;

	kp = sp ;
	kl = sl ;
	vp = NULL ;
	vl = 0 ;
	if ((tp = strnchr(sp,sl,'=')) != NULL) {
	    kl = (tp - sp) ;
	    vp = (tp + 1) ;
	    vl = ((sp + sl) - (tp + 1)) ;
	} /* end if (had a key-value separator) */

#if	CF_DEBUGS
	debugprintf("vecstr_addpair: key=%t\n",kp,strnlen(kp,kl)) ;
	if (vp != NULL)
	    debugprintf("vecstr_addpair: val=>%t<\n",vp,strnlen(vp,vl)) ;
#endif

	rs = vecstr_addkeyval(op,kp,kl,vp,vl) ;

#if	CF_DEBUGS
	debugprintf("vecstr_addpair: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (vecstr_addpair) */


#ifdef	COMMENT

/* check to see if we have a good freeze file */

/****
	The freeze file has the following layout :

	magic number
	number filter entries
	filter buffer pointer
	filter buffer length
	filter entry array block
	filter buffer

****/

static int getconfig(pip,ffname,fe,fbuf,nfep,fbuflenp,fzfname)
PROGINFO	*pip ;
char		ffname[] ;
struct fzentry	fe[] ;
char		fbuf[] ;
int		*nfep ;
int		*fbuflenp ;
char		fzfname[] ;
{
	bfile		filterfile, *ffp = &filterfile ;
	bfile		fzfile, *zfp = &fzfile ;
	struct ustat	fzstat ;
	struct ustat	fstat ;
	FIELD		fsb ;
	long		fzmagic ;
	int	rs ;
	int	i, c ;
	int	line, len ;
	int	size ;
	int	nfe = 0 ;
	int	fbuflen = 0, rflen ;

	char	pathbuf[MAXPATHLEN + 1] ;
	char	lbuf[LINEBUFLEN] ;
	char	*cp, *fbp = fbuf ;


	rs = u_stat(ffname,&fstat) ;
	if (rs < 0)
	    goto ret0 ;

	rs = fullpath(ffname,pathbuf,MAXPATHLEN) ;
	if (rs < 0)
	    goto badpath ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: pathbuf len=%u\n",
	        strlen(pathbuf)) ;
#endif

/* create the freeze file name */

	for (i = 0 ; i < MAXTMP ; i += 1) {
	    mkfzname(ffname,i,fzfname) ;
	    rs = ourfreeze(pip,zfp,fzfname,pathbuf) ;
	    if (rs >= 0) break ;
	} /* end for */

	if (i >= MAXTMP)
	    return GC_MAXTMP ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: freeze file=%s\n",
	        pip->progname,fzfname) ;
	}

/* if file doesn't exist, go create it */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 1\n") ;
#endif

	if (rs > 0)
	    goto create ;

/* check file modification dates */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 2\n") ;
#endif

	rs = u_stat(fzfname,&fzstat) ;
	if (rs < 0)
	    goto badfreeze ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 3\n") ;
#endif

	if (fstat.st_mtime > fzstat.st_mtime)
	    goto badfreeze ;

/* read in and check the data structure addresses and sizes */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 4\n") ;
#endif

	size = sizeof(int) ;
	rs = bread(zfp,&nfe,size) ;
	len = rs ;
	if (len < size)
	    goto badfreeze ;

	*nfep = nfe ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: nfe %d\n",nfe) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 5\n") ;
#endif

	if (nfe < 0)
	    goto badfreeze ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 6\n") ;
#endif

	size = sizeof(fbp) ;
	rs = bread(zfp,&fbp,size) ;
	len = rs ;
	if (len < size)
	    goto badfreeze ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 7\n") ;
#endif

	if (fbp != fbuf)
	    goto badfreeze ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 8\n") ;
#endif

	size = sizeof(int) ;
	rs = bread(zfp,&fbuflen,size) ;
	len = rs ;
	if (len < size)
	    goto badfreeze ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 9\n") ;
#endif

	if (nfe > NFILTERS) {

	    bclose(zfp) ;

	    return GC_TOOMANY ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    bprintf(pip->efp,"10\n") ;
#endif

	*fbuflenp = fbuflen ;
	if (fbuflen > FBUFLEN) {

	    bclose(zfp) ;

	    return GC_TOOMUCH ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 11\n") ;
#endif

	if (fbuflen < 0)
	    goto badfreeze ;

	len = nfe * sizeof(struct fzentry) ;

/* read in the freeze file data */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 12\n") ;
#endif

	if (bread(zfp,fe,len) < len)
	    goto badfreeze ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 13\n") ;
#endif

	if (bread(zfp,fbuf,fbuflen) < fbuflen)
	    goto badfreeze ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: 14\n") ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: filter substitutions\n",
	        pip->progname) ;

	    for (i = 0 ; i < nfe ; i += 1) {

	        bprintf(pip->efp,"%W\n-> %W\n",fe[i].kp,fe[i].klen,
	            fe[i].vp,fe[i].vlen) ;

	    } /* end for */

	} /* end if (debug print-out) */

	bclose(zfp) ;

	return OK ;

/* process the filter file and write out the freeze file */
create:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: creating a new freeze file\n",
	        pip->progname) ;

/* process the filter file first */

	if (bopen(ffp,ffname,"r",0666) < 0)
	    return BAD ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: open successful\n") ;
#endif

	nfe = 0 ;
	rflen = FBUFLEN ;
	fbp = fbuf ;
	line = 0 ;
	while ((rs = breadline(ffp,lbuf,LINEBUFLEN)) > 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("getconfig: read filter file line\n") ;
#endif

	    len = rs ;
	    line += 1 ;
	    if (len == 1)
	        continue ;		/* blank line */

	    if (lbuf[len - 1] != '\n') {

	        while ((c = bgetc(ffp)) >= 0)
	            if (c == '\n')
	                break ;

	        bprintf(pip->efp,
	            "%s: filter file line too long, ignoring\n",
	            pip->progname) ;

	        continue ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("getconfig: line=>%W<\n",lbuf,(len - 1)) ;
#endif

	    fsb.rlen = len - 1 ;
	    fsb.lp = lbuf ;
	    cp = fbp ;

/* parse out the key string */

	    field_get(&fsb,NULL) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("getconfig: flen=%d\n",fsb.flen) ;
	        if (fsb.flen >= 0)
	            debugprintf("getconfig: field=>%W<\n",fsb.fp,fsb.flen) ;
	    }
#endif

	    if (fsb.flen <= 0)
	        continue ;

	    if (nfe >= NFILTERS) {

	        bclose(ffp) ;

	        return GC_TOOMANY ;
	    }

	    if ((fsb.flen + 1) >= rflen)
	        goto toomuch ;

	    fe[nfe].kp = cp ;
	    fe[nfe].klen = fsb.flen ;
	    strncpy(cp,fsb.fp,fsb.flen) ;

	    cp += fsb.flen ;
	    *cp++ = '\0' ;
	    rflen -= (fsb.flen + 1) ;

/* try to parse out the substitution string */

	    field_get(&fsb,NULL) ;

	    if (fsb.flen < 0)
	        fsb.flen = 0 ;

	    if ((fsb.flen + 1) >= rflen)
	        goto toomuch ;

	    fe[nfe].vp = cp ;
	    fe[nfe].vlen = fsb.flen ;
	    strncpy(cp,fsb.fp,fsb.flen) ;

	    cp += fsb.flen ;
	    *cp++ = '\0' ;
	    rflen -= (fsb.flen + 1) ;
	    fbp = cp ;
	    nfe += 1 ;

	} /* end while */

	fbuflen = fbp - fbuf ;

#if	CF_AUDIT
	if (fbuflen != (FBUFLEN - rflen)) {

	    bprintf(pip->efp,
	        "%s: audit failure on \"filter buffer length\"\n",
	        pip->progname) ;

	}
#endif /* CF_AUDIT */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    fbp = fbuf ;
	    for (i = 0 ; i < nfe ; i += 1) {
	        debugprintf("getconfig: %s\n",fbp) ;
	        fbp += (strlen(fbp) + 1) ;
	        debugprintf("getconfig: -> %s\n",fbp) ;
	        fbp += (strlen(fbp) + 1) ;
	    }
	}
#endif /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: read whole filter file\n") ;
#endif

	bclose(ffp) ;

	*nfep = nfe ;
	*fbuflenp = fbuflen ;

/* try to write out a freeze file */

	u_unlink(fzfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: opening freeze file\n") ;
#endif

	if ((rs = bopen(zfp,fzfname,"rwct",0666)) < 0) {

	    bprintf(pip->efp,"%s: could not create a freeze file (%d)\n",
	        pip->progname,rs) ;

	    return OK ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: writing out magic\n") ;
#endif

	fzmagic = FZMAGIC ;
	bwrite(zfp,&fzmagic,sizeof(fzmagic)) ;

	bprintf(zfp,"%s\n",pathbuf) ;

/* write out the freeze file data */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: writing out freeze file data\n") ;
#endif

	bwrite(zfp,&nfe,sizeof(int)) ;

	fbp = fbuf ;
	bwrite(zfp,&fbp,sizeof(fbp)) ;

	bwrite(zfp,&fbuflen,sizeof(int)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: filter buffer length %d\n",fbuflen) ;
#endif

	bwrite(zfp,fe,sizeof(struct fzentry) * nfe) ;

	bwrite(zfp,fbuf,fbuflen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("getconfig: wrote out freeze file data\n") ;
#endif

done:
	bclose(zfp) ;

ret0:
	return rs ;

/* bad stuff */
badpath:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,
	        "%s: could not get full path of filter file\n",
	        pip->progname) ;

	return GC_BADPATH ;

badfreeze:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: bad freeze file\n",
	        pip->progname) ;

	bclose(zfp) ;

	goto create ;

toomuch:
	bclose(ffp) ;

	return GC_TOOMUCH ;
}
/* end subroutine (getconfig) */


/* get the full path to a file (for freeze same purposes) */
static int fullpath(filename,pathbuf,pathlen)
const char	filename[] ;
char		pathbuf[] ;
int		pathlen ;
{
	const int	flen = strlen(filename) ;
	int		plen ;

	if (filename[0] == '/') {

	    if ((flen + 1) >= pathlen)
	        return BAD ;

	    strcpy(pathbuf,filename) ;

	    return OK ;
	}

	if (getpwd(pathbuf,pathlen) < 0)
	    return BAD ;

	plen = strlen(pathbuf) ;

	if ((pathlen - plen) < (flen + 2))
	    return BAD ;

	strcpy((pathbuf + plen),"/") ;

	strcpy((pathbuf + plen + 1),filename) ;

	return OK ;
}
/* end subroutine (fullpath) */


/* make a freeze file */
static void mkfzname(ffname,seed,fzfname)
const char	*ffname ;
int		seed ;
const char	fzfname[] ;
{
	uint		num1 = 0 ;
	uint		num2 = 0 ;
	int		len, bnlen, i ;
	const char	*bn ;

	if ((bnlen = sfbasename(ffname,-1,&bn)) <= 0) {
	    bn = "junk" ;
	    bnlen = strlen(bn) ;
	}

	for (i = 0 ; i < ((bnlen < 6) ? bnlen : 6) ; i += 1) {
	    num1 = (num1 << 5) | (bn[i] & 0x1F) ;
	}

	if (bnlen > 6) {

	    for (i = 6 ; i < ((bnlen < 12) ? bnlen : 12) ; i += 1)
	        num2 = (num2 << 5) | (bn[i] & 0x1F) ;

	    num1 = (num1 ^ (num2 >> 16)) ;

	    switch (bnlen) {

	    case 14:
	        num2 = (num2 ^ bn[13]) ;

/* fall through to next case */
/* FALLTHROUGH */
	    case 13:
	        num1 = (num1 ^ bn[12]) ;
	        break ;

	    } /* end switch */

	} /* end if */

	num1 = (num1 ^ seed) ;
	len = sprintf(fzfname,"%s/%s%08X%04X",FZPATH,FZPREFIX,num1,num2) ;

	fzfname[len] = '\0' ;
}
/* end subroutine (mkfzname) */


/* is the specified freeze file "our" freeze file? */
static int ourfreeze(pip,zfp,fzfname,pathbuf)
PROGINFO	*pip ;
bfile		*zfp ;
const char	fzfname[], pathbuf[] ;
{
	long		fzmagic ;
	int		rs ;
	int		i, len ;
	char		buf[MAXPATHLEN], *bp, *cp ;

	rs = bopen(zfp,fzfname,"r",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: entered 'ourfreeze' - open RS (%d)\n",rs) ;
#endif

	if (rs < 0)
	    return 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 1\n") ;
#endif

	if (bread(zfp,&fzmagic,sizeof(fzmagic)) < sizeof(fzmagic))
	    goto bad ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 2\n") ;
#endif

	if (fzmagic != FZMAGIC)
	    goto bad ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("oufreeze: 3\n") ;
#endif

	rs = breadline(zfp,buf,MAXPATHLEN) ;
	len = rs ;
	if (rs < 0)
	    goto bad ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 4\n") ;
#endif

	if (len < 2)
	    goto bad ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 5\n") ;
#endif

	if (buf[len - 1] != '\n')
	    goto bad ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 6\n") ;
#endif

	len -= 1 ;
	bp = buf ;
	for (i = 0 ; i < len ; i += 1) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("ourfreeze: c=%c %02X\n",*bp,*bp) ;
#endif

	    if ((*bp != ' ') && (*bp != '\t'))
	        break ;

	    bp += 1 ;

	} /* end for */

	if (i >= len)
	    goto bad ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 7\n") ;
#endif

	cp = bp ;
	for ( ; i < len ; i += 1) {
	    if ((*cp == ' ') || (*cp == '\t')) break ;
	    cp += 1 ;
	} /* end for */


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 8\n") ;
#endif

	*cp = '\0' ;
	if (strcmp(bp,pathbuf) != 0)
	    goto bad ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("ourfreeze: 9\n") ;
#endif

	return OK ;

bad:
	bclose(zfp) ;

	return BAD ;
}
/* end subroutine (ourfreeze) */

#endif /* COMMENT */


#if	CF_DEBUG 
static int debugsubs(PROGINFO *pip,VARSUB *vsp)
{
	int	rs = SR_OK ;
	if (DEBUGLEVEL(2) && (rs >= 0)) {
	    VARSUB_CUR	scur ;
	    int		vl ;
	    const char	*kp, *vp ;
	    debugprintf("main: list-start\n") ;
	    varsub_curbegin(&subs,&scur) ;
	    while ((vl = varsub_enum(&subs,&scur,&kp,&vp)) >= 0) {
	        debugprintf("main: VS k=%s\n",kp) ;
	        if (vp != NULL)
	            debugprintf("main: VS v=>%t<\n",vp,vl) ;
	    }
	    varsub_curend(&subs,&scur) ;
	    debugprintf("main: list-end\n") ;
	}
	return SR_OK ;
}
/* end subroutine (debugsubs) */
#endif /* CF_DEBUG */


