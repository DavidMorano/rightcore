/* main */

/* general program front end */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The program was written from scratch to do what the previous program by
	the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is the front-end of the EMA program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
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
#include	<nulstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"ema_config.h"
#include	"ema_local.h"
#include	"defs.h"


/* local defines */

#define	OURLINELEN	(MAILMSGLINELEN-4)


/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	progfile(PROGINFO *,PARAMOPT *,bfile *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procargs(PROGINFO *,ARGINFO *,BITS *,PARAMOPT *,
			cchar *,cchar *,cchar *) ;

static int	procmsgfile(PROGINFO *,PARAMOPT *,void *,const char *,int) ;


/* local variables */

static const char	*aknames[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"info",
	"sn",
	"af",
	"ef",
	"of",
	"if",
	NULL
} ;

enum aknames {
	akname_root,
	akname_version,
	akname_verbose,
	akname_tmpdir,
	akname_help,
	akname_option,
	akname_info,
	akname_sn,
	akname_af,
	akname_ef,
	akname_of,
	akname_if,
	akname_overlast
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char	*akonames[] = {
	"expand",
	"noexpand",
	"flat",
	"list",
	"info",
	NULL
} ;

enum akonames {
	akoname_expand,
	akoname_noexpand,
	akoname_flat,
	akoname_list,
	akoname_info,
	akoname_overlast
} ;

static const char	*subparts[] = {
	"address",
	"route",
	"comment",
	"original",
	"best",
	"any",
	NULL
} ;

enum subparts {
	subpart_address,
	subpart_route,
	subpart_comment,
	subpart_original,
	subpart_best,
	subpart_any,
	subpart_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	ARGINFO		ainfo ;
	CMD_LOCAL	localstate, *lsp = &localstate ;
	BITS		pargs ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		i ;
	int		cl ;
	int		count = 0 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_recipients = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}

#endif /* CF_DEBUG */

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
	proginfo_setbanner(pip,cp) ;

	pip->lsp = lsp ;
	memset(lsp,0,sizeof(CMD_LOCAL)) ;

/* early things to initialize */

	pip->verboselevel = 1 ;

	lsp->f.expand = TRUE ;		/* default is to expand mail-groups */

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
	        const int ach = MKCHAR(argp[1]) ;

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

	            if ((kwi = matostr(aknames,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case akname_version:
	                    f_version = TRUE ;
	                    break ;

/* verbose */
	                case akname_verbose:
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
	                case akname_tmpdir:
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

/* get a program root */
	                case akname_root:
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

/* want help ! file */
	                case akname_help:
	                    f_help = TRUE ;
	                    break ;

/* program search-name */
	                case akname_sn:
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

/* argument file-file */
	                case akname_af:
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
	                case akname_ef:
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
	                case akname_of:
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
	                case akname_if:
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

/* the user specified some options */
	                case akname_option:
	                    cp = NULL ;
	                    cl = -1 ;
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
	                        rs = paramopt_loadu(&aparams,cp,cl) ;
	                    }
	                    break ;

	                case akname_info:
	                    lsp->f.info = TRUE ;
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

/* get a program root */
	                    case 'R':
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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'h':
	                        cp = NULL ;
	                        cl = -1 ;
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
	                            const char	*po = PO_HEADER ;
	                            rs = paramopt_loads(&aparams,po,cp,cl) ;
	                        }
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

	                    case 'n':
	                        lsp->f.count = TRUE ;
	                        break ;

/* specify all recipient headers */
	                    case 'r':
	                        f_recipients = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                f_recipients = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* specifiy a subpart of the address to extract */
	                    case 's':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                PARAMOPT	*pop = &aparams ;
	                                const char	*po = PO_SUBPART ;
	                                rs = paramopt_loads(pop,po,argp,argl) ;
	                            }
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

	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->linelen = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
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
	debugprintf("main: after args rs=%d\n",rs) ;
#endif

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"dwca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
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

/* check a few more things */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* user specified help only */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* other initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* process the keyed arguments */

	if (f_recipients)
	    rs = paramopt_loads(&aparams,PO_HEADER,"to,cc,bcc",-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: procopts() \n") ;
#endif

	if (rs >= 0)
	    rs = procopts(pip,&akopts) ;

	if ((rs >= 0) && (pip->linelen == 0)) {
	    cp = getourenv(envv,VARLINELEN) ;
	    if (cp != NULL) {
	        if ((rs = optvalue(cp,-1)) >= 0) {
	            pip->linelen = rs ;
	        }
	    }
	}

	if (pip->linelen == 0) pip->linelen = OURLINELEN ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2) && (rs >= 0))
	    debugprintf("main: AKO expand=%u\n",lsp->f.expand) ;
#endif /* CF_DEBUG */

/* check arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    PARAMOPT_CUR	c ;
	    const char	*ccp ;
	    debugprintf("main: parameter keys are:\n") ;
	    paramopt_curbegin(&aparams,&c) ;
	    while (paramopt_enumkeys(&aparams,&c,&ccp) >= 0) {
	        debugprintf("main: key=%s\n",ccp) ;
	    }
	    paramopt_curend(&aparams,&c) ;
	    debugprintf("main: headers are:\n") ;
	    paramopt_curbegin(&aparams,&c) ;
	    while (paramopt_enumvalues(&aparams,PO_HEADER,&c,&ccp) >= 0)
	        debugprintf("main: header=%s\n",ccp) ;
	    paramopt_curend(&aparams,&c) ;
	    debugprintf("main: subparts are:\n") ;
	    paramopt_curbegin(&aparams,&c) ;
	    while (paramopt_enumvalues(&aparams,PO_SUBPART,&c,&ccp) >= 0)
	        debugprintf("main: header=%s\n",ccp) ;
	    paramopt_curend(&aparams,&c) ;
	} /* end block */
#endif /* CF_DEBUG */

/* set the default header to choose if one has not been selected */

	if (rs >= 0) {
	    if (paramopt_havekey(&aparams,PO_HEADER) == SR_NOTFOUND) {
	        rs = paramopt_load(&aparams,PO_HEADER,"to",2) ;
	    }
	}

/* what about the address subpart selection criteria */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: about to set subpart criteria\n") ;
#endif

	if (rs >= 0) {
	    PARAMOPT_CUR	c ;
	    int			spc = 0 ;
	    int			vl ;
	    const char		*vp ;

	    if ((rs = paramopt_curbegin(&aparams,&c)) >= 0) {
	        while (rs >= 0) {

	            vl = paramopt_enumvalues(&aparams,PO_SUBPART,&c,&vp) ;
	            if (vl == SR_NOTFOUND) break ;
	            rs = vl ;
	            if (rs < 0) break ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("main: lookup subpart=%s\n",cp) ;
#endif

	            if ((i = matostr(subparts,1,vp,vl)) >= 0) {
	                switch (i) {
	                case subpart_address:
	                    lsp->af.address = TRUE ;
	                    spc += 1 ;
	                    break ;
	                case subpart_route:
	                    lsp->af.route = TRUE ;
	                    spc += 1 ;
	                    break ;
	                case subpart_comment:
	                    lsp->af.comment = TRUE ;
	                    spc += 1 ;
	                    break ;
	                case subpart_original:
	                    lsp->af.original = TRUE ;
	                    spc += 1 ;
	                    break ;
	                case subpart_best:
	                    lsp->af.best = TRUE ;
	                    spc += 1 ;
	                    break ;
	                case subpart_any:
	                    lsp->af.any = TRUE ;
	                    spc += 1 ;
	                    break ;
	                } /* end switch */
	            } else {
	                rs = SR_INVALID ;
	                f_usage = TRUE ;
	                bprintf(pip->efp,
	                    "%s: unrecognized EMA subpart=%s\n",
	                    pip->progname,vp) ;
	            } /* end if */
	        } /* end while */
	        paramopt_curend(&aparams,&c) ;
	    } /* end if */

	    if (spc <= 0)
	        lsp->af.best = TRUE ;

	} /* end if (ok) */

/* continue */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    const char	*ofn = ofname ;
	    const char	*ifn = ifname ;
	    const char	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,&aparams,ofn,ifn,afn) ;
	} /* end if (ok) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid usage (%d)\n",
	                pip->progname,rs) ;
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

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting rs=%d ex=%u\n",rs,ex) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = NULL ;
	    paramopt_finish(&aparams) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = NULL ;
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


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<msgfile(s)>] [-af <afile>] [-h <header(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-info] [-s <subpart(s)>] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program options */
static int procopts(PROGINFO *pip,KEYOPT *akp)
{
	CMD_LOCAL	*lsp = pip->lsp ;
	int		rs = SR_OK ;
	int		c = 0 ;
	const char	*cp ;

	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(akp,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	cur ;
	    if ((rs = keyopt_curbegin(akp,&cur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(akp,&cur,&kp)) >= 0) {

	            if ((oi = matostr(akonames,1,kp,kl)) >= 0) {

	                vl = keyopt_fetch(akp,kp,NULL,&vp) ;

	                switch (oi) {

	                case akoname_noexpand:
	                case akoname_list:
	                    lsp->f.expand = FALSE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lsp->f.expand = (rs > 0) ;
	                    }
	                    break ;

	                case akoname_expand:
	                case akoname_flat:
	                    lsp->f.expand = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lsp->f.expand = (rs > 0) ;
	                    }
	                    break ;

	                case akoname_info:
	                    lsp->f.info = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        lsp->f.info = (rs > 0) ;
	                    }
	                    break ;

	                } /* end switch */

	                c += 1 ;
	            } /* end if (matostr) */

	            if (rs < 0) break ;
	        } /* end while */

	        keyopt_curend(akp,&cur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procargs(pip,aip,bop,app,ofname,ifname,afname)
PROGINFO	*pip ;
ARGINFO		*aip ;
BITS		*bop ;
PARAMOPT	*app ;
const char	*ofname ;
const char	*ifname ;
const char	*afname ;
{
	CMD_LOCAL	*lsp = pip->lsp ;
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		count = 0 ;

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    int		f_input = TRUE ;
	    int		cl ;
	    const char	*cp ;
	    pip->ofp = ofp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                f_input = FALSE ;
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	                    rs = procmsgfile(pip,app,ofp,cp,-1) ;
	                    count += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for (looping through mail message files) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        f_input = FALSE ;
	        if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	        if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procmsgfile(pip,app,ofp,cp,cl) ;
	                        count += rs ;
	                    }
	                }

	                if (rs < 0) break ;
	            } /* end while (reading lines) */

	            bclose(afp) ;
	        } else {
	            if (! pip->f.quiet) {
	                bprintf(pip->efp,
	                    "%s: inaccessible arglist (%d)\n",
	                    pip->progname,rs) ;
	                bprintf(pip->efp,"\targfile=%s\n",afname) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && f_input) {

	        if ((ifname == NULL) || (ifname[0] == '\0')) ifname = "-" ;

	        cp = ifname ;
	        pan += 1 ;
	        rs = progfile(pip,app,ofp,cp) ;
	        count += rs ;

	        if (rs < 0) {
	            if (strcmp(cp,"-") == 0) cp = "*STDIN*" ;
	            if (! pip->f.quiet) {
	                bprintf(pip->efp,
	                    "%s: problem (%d) file=%s\n",
	                    pip->progname,rs,cp) ;
	            }
	        }

	    } /* end if */

	    if ((rs >= 0) && lsp->f.count)
	        bprintf(ofp,"%u\n",count) ;

	    pip->ofp = NULL ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    bprintf(pip->efp,"%s: inaccessible output (%d)\n",
	        pip->progname,rs) ;
	}

	if ((rs >= 0) && (pip->debuglevel > 0)) {
	    bprintf(pip->efp,"%s: count=%u\n",
	        pip->progname,count) ;
	}

	return (rs >= 0) ? count : rs ;
}
/* end subroutine (procargs) */


static int procmsgfile(pip,app,ofp,fp,fl)
PROGINFO	*pip ;
PARAMOPT	*app ;
void		*ofp ;
const char	*fp ;
int		fl ;
{
	NULSTR		f ;
	int		rs ;
	int		c = 0 ;
	const char	*fname ;
	if ((rs = nulstr_start(&f,fp,fl,&fname)) >= 0) {
	    rs = progfile(pip,app,ofp,fname) ;
	    c = rs ;
	    nulstr_finish(&f) ;
	} /* end if (nulstr) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procmsgfile) */


