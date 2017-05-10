/* main */

/* part of the MSGCLEAN program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	progspec(PROGINFO *,PARAMOPT *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* global variables */

static volatile int	if_exit ;
static volatile int	if_int ;


/* local structures */

struct locinfo_flags {
	uint		header:1 ;
	uint		fmtlong:1 ;
	uint		fmtshort:1 ;
	uint		uniq:1 ;
	uint		users:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f ;
	LOCINFO_FL	open ;
	char		username[USERNAMELEN + 1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,
			PARAMOPT *,cchar *,cchar *) ;


/* external variables */


/* local variables */

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
	"option",
	"set",
	"follow",
	"nice",
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
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_nice,
	argopt_overlast
} ;

static const char	*progmodes[] = {
	"msgclean",
	NULL
} ;

enum progmodes {
	progmode_msgclean,
	progmode_overlast
} ;

static const char *akonames[] = {
	"ageint"
	"nice",
	"header",
	"long",
	"short",
	"uniq",
	"users",
	NULL
} ;

enum akonames {
	akoname_ageint,
	akoname_nice,
	akoname_header,
	akoname_long,
	akoname_short,
	akoname_uniq,
	akoname_users,
	akoname_overlast
} ;

static const char	*progopts[] = {
	"follow",
	"nofollow",
	"nostop",
	NULL
} ;

enum progopts {
	progopt_follow,
	progopt_nofollow,
	progopt_nostop,
	progopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs ;
	int		rs1 ;
	int		opts ;
	int		v ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*pmspec = NULL ;
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

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uc_mallset(1) ;
	uc_mallout(&mo_start) ;
#endif

	if_exit = 0 ;
	if_int = 0 ;

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->namelen = MAXNAMELEN ;
	pip->verboselevel = 1 ;
	pip->daytime = time(NULL) ;
	pip->progmode = -1 ;
	pip->intage = -1 ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
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

/* error file */
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

/* nice value */
	                case argopt_nice:
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
	                        rs = optvalue(cp,cl) ;
	                        pip->niceval = rs ;
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

/* follow symbolic links */
	                    case 'f':
	                        pip->f.follow = TRUE ;
	                        break ;

/* file name length restriction */
	                    case 'l':
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
	                            rs = optvalue(cp,cl) ;
	                            pip->namelen = rs ;
	                        }
	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* recurse down directories */
	                    case 'r':
	                        pip->f.recurse = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.recurse = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* require a suffix for file names */
	                    case 's':
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
	                            cchar	*po = PO_SUFFIX ;
	                            rs = paramopt_loads(pop,po,cp,cl) ;
	                        }
	                        break ;

/* age interval */
	                    case 't':
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
	                            rs = cfdecti(cp,cl,&v) ;
	                            pip->intage = v ;
	                        }
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

	        } /* end if (digits or progopts) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VAREFNAME) ;
	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
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

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

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
	} /* end if */

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
#endif /* CF_DEBUG */

	if (pip->progmode < 0)
	    pip->progmode = progmode_msgclean ;

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	if (pip->intage >= 0) {
	    pip->have.intage = TRUE ;
	    pip->final.intage = TRUE ;
	}

	rs = procopts(pip,&akopts) ;

/* check a few more things */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* what about the age interval */

	if ((rs >= 0) && (pip->intage < 0) && (argval != NULL)) {
	    rs = cfdecti(argval,-1,&v) ;
	    pip->intage = v ;
	}

	if ((rs >= 0) && (pip->intage < 0)) {
	    if ((cp = getenv(VARAGE)) != NULL) {
	        rs = cfdecti(cp,-1,&v) ;
	        pip->intage = v ;
	    }
	}

	if (pip->intage < 0)
	    pip->intage = TO_AGE ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: intage=%u\n",pip->progname,pip->intage) ;
	}

/* optionally 'nice' us */

	if ((rs >= 0) && (pip->niceval > 0)) {
	    u_nice(pip->niceval) ;
	}

	if (rs < 0) goto badarg ;

/* initialize suffix list */

	opts = 0 ;
	if ((rs = vecstr_start(&pip->suffixes,10,opts)) >= 0) {
	    PARAMOPT	*pop = &aparams ;
	    cchar	*po = PO_SUFFIX ;

	    if ((rs = paramopt_havekey(pop,po)) >= 0) {
	        PARAMOPT_CUR	cur ;
	        const char	*vp ;

	        if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {

	            while (paramopt_enumvalues(pop,po,&cur,&vp) >= 0) {
	                if (vp == NULL) continue ;

	                pip->f.suffix = TRUE ;
	                rs = vecstr_add(&pip->suffixes,vp,-1) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("main: suf> %s\n",vp) ;
#endif /* CF_DEBUG */

	                if (rs < 0) break ;
	            } /* end while */

	            paramopt_curend(pop,&cur) ;
	        } /* end if (cursor) */
	    } /* end if (suffix list) */

	    if (rs >= 0) {
	        const char	*po = PO_OPTION ;
	        if (paramopt_havekey(pop,po) >= 0) {
	            PARAMOPT_CUR	cur ;
	            const char		*vp ;

	            if ((rs = paramopt_curbegin(pop,&cur)) >= 0) {

	                while (paramopt_enumvalues(&aparams,po,&cur,&vp) >= 0) {
	                    if (vp == NULL) continue ;

	                    if ((kwi = matostr(progopts,1,vp,-1)) >= 0) {
	                        switch (kwi) {
	                        case progopt_follow:
	                            pip->f.follow = TRUE ;
	                            break ;
	                        case progopt_nofollow:
	                            pip->f.follow = FALSE ;
	                            break ;
	                        case progopt_nostop:
	                            pip->f.nostop = TRUE ;
	                            break ;
	                        } /* end switch */
	                    } /* end if (progopts) */

	                } /* end while */

	                paramopt_curend(pop,&cur) ;
	            } /* end if (cursor) */
	        } /* end if (progopts) */
	    } /* end if (ok) */

/* go */

	    memset(&ainfo,0,sizeof(ARGINFO)) ;
	    ainfo.argc = argc ;
	    ainfo.ai = ai ;
	    ainfo.argv = argv ;
	    ainfo.ai_max = ai_max ;
	    ainfo.ai_pos = ai_pos ;

	    if (rs >= 0) {
	        cchar	*ofn = ofname ;
	        cchar	*afn = afname ;
	        rs = procargs(pip,&ainfo,&pargs,&aparams,ofn,afn) ;
	    }

	    vecstr_finish(&pip->suffixes) ;
	} /* end if (suffixes) */

/* done */
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: files total=%u scanned=%u processed=%u\n",
	        pip->progname,pip->c_total, pip->c_scanned, pip->c_fixed) ;
	}

	if ((rs < 0) && (ex == EX_OK)) {
	    ex = EX_DATAERR ;
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
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
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
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


int progabort(PROGINFO *pip)
{
	if (pip == NULL) return SR_FAULT ;
	return if_exit ;
}
/* end subroutine (progabort) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<file(s)>] [<dir(s)>] [-s <suffix(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-t <age>] [-af {<afile>|-}] \n" ;
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
	LOCINFO		*lip = pip->lip ;
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

	            if ((oi = matostr(akonames,3,kp,kl)) >= 0) {

	                vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_ageint:
	                    if (! pip->have.intage) {
	                        pip->have.intage = TRUE ;
	                        pip->f.intage = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            pip->intage = rs ;
	                        }
	                    }
	                    break ;

	                case akoname_nice:
	                    if (! pip->have.niceval) {
	                        pip->have.niceval = TRUE ;
	                        pip->f.niceval = TRUE ;
	                        if (vl > 0) {
	                            rs = optvalue(vp,vl) ;
	                            pip->niceval = rs ;
	                        }
	                    }
	                    break ;

	                case akoname_header:
	                    if (! lip->have.header) {
	                        lip->have.header = TRUE ;
	                        lip->f.header = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.header = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_long:
	                    if (! lip->have.fmtlong) {
	                        lip->have.fmtlong = TRUE ;
	                        lip->f.fmtlong = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.fmtlong = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_short:
	                    if (! lip->have.fmtshort) {
	                        lip->have.fmtshort = TRUE ;
	                        lip->f.fmtshort = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.fmtshort = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_uniq:
	                    if (! lip->have.uniq) {
	                        lip->have.uniq = TRUE ;
	                        lip->f.uniq = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.uniq = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case akoname_users:
	                    if (! lip->have.users) {
	                        lip->have.users = TRUE ;
	                        lip->f.users = TRUE ;
	                        if (vl > 0) {
	                            rs = optbool(vp,vl) ;
	                            lip->f.users = (rs > 0) ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procopts: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,PARAMOPT *app,
cchar *ofn, cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		pan = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0644)) >= 0) {
	    cchar	*cp ;
	    pip->ofp = ofp ;

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
	                    rs = progspec(pip,app,cp) ;
	                }
	            }

	            if (rs < 0) {
	                fmt = "%s: processing error (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: file=%s\n",pn,cp) ;
	            }
	            if (rs < 0) break ;
	        } /* end for (looping through requested circuits) */
	    } /* end if (ok) */

/* process any files in the argument filename list file */

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

	                if (len > 0) {
	                    if (lbuf[0] != '#') {
	                        pan += 1 ;
	                        rs = progspec(pip,app,lbuf) ;
	                    }
	                }

	                if (rs < 0) {
	                    fmt = "%s: processing error (%d)\n" ;
	                    bprintf(pip->efp,fmt,pn,rs) ;
	                    bprintf(pip->efp,"%s: afile=%s\n",pn,lbuf) ;
	                }
	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            bclose(afp) ;
	        } else {
	            if (! pip->f.quiet) {
	                bprintf(pip->efp,
	                    "%s: inaccessible argument-list (%d)\n",
	                    pip->progname,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",
	                    pip->progname,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

/* print out options summary */

	    if ((rs >= 0) && (pip->verboselevel >= 2)) {
	        bprintf(pip->ofp,"files total=%u scanned=%u processed=%u\n",
	            pip->c_total, pip->c_scanned, pip->c_fixed) ;
	    } /* end if (verbose output) */

	    pip->ofp = NULL ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}

	if ((pan == 0) && (afn == NULL)) {
	    fmt = "%s: no files or directories specified\n" ;
	    bprintf(pip->efp,fmt,pn) ;
	}

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


