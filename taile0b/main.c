/* main */

/* fairly generic front-end */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_SESSION	1		/* track our process session */
#define	CF_ISPROC	1		/* use 'isproc(3dam)' */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The program was written from scratch to do what the previous program by
	the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program is like 'tail(1)' but has several enhancements over that
	program.  This program can track multiple files simultaneously and can
	also be put into the backgroun more safely than 'tail(1)' can be.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<vechand.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"filewatch.h"
#include	"strfilter.h"

#include	"config.h"
#include	"defs.h"


/* local defines */

#undef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	isproc(pid_t) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	istrack(PROGINFO *) ;

static void	int_exit(int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static cchar	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDNAME",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"ssf",
	"sxf",
	"ppm",
	"tpid",
	"wait",
	"fold",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_ssf,
	argopt_sxf,
	argopt_ppm,
	argopt_tpid,
	argopt_wait,
	argopt_fold,
	argopt_overlast
} ;

static cchar	*progopts[] = {
	"wait",
	"stdin",
	"own",
	"fold",
	"clean",
	"ssf",
	"sxf",
	"linelen",
	"indent",
	NULL
} ;

enum progopts {
	progopt_wait,
	progopt_stdin,
	progopt_own,
	progopt_fold,
	progopt_clean,
	progopt_ssf,
	progopt_sxf,
	progopt_linelen,
	progopt_indent,
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
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar *argv[],cchar *envv[])
{
	struct sigaction	sigs ;
	PROGINFO	pi, *pip = &pi ;
	KEYOPT		akopts ;
	PARAMOPT	aparams ;
	BITS		pargs ;
	FILEWATCH	*wp ;
	STRFILTER	sf, *sfp = NULL ;
	sigset_t	signalmask ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	vecstr		files ;
	vechand		watchers ;
	uint		uiw ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		narg = 0 ;
	int		rs, rs1 ;
	int		n, i ;
	int		size, len ;
	int		cl ;
	int		opts, cutoff = 0 ;
	int		interval = -1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_used = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*afname = NULL ;
	cchar		*ofname = NULL ;
	cchar		*efname = NULL ;
	cchar		*cp ;


	if_int = 0 ;
	if_exit = 0 ;

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

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->f.quiet = FALSE ;
	pip->f.carriage = FALSE ;
	pip->f.background = FALSE ;
	pip->f.usestdin = TRUE ;	/* TRUE */
	pip->f.clean = TRUE ;		/* TRUE */

/* other early initialization */

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

	if (rs >= 0) {
	    rs = vecstr_start(&files,10,0) ;
	    pip->open.files = (rs >= 0) ;
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

/* want help ! file */
	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* search-name */
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

	                case argopt_ssf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->final.ssfile = TRUE ;
	                            pip->have.ssfile = TRUE ;
	                            pip->ssfname = avp ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->final.ssfile = TRUE ;
	                                pip->have.ssfile = TRUE ;
	                                pip->ssfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_sxf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            pip->final.sxfile = TRUE ;
	                            pip->have.sxfile = TRUE ;
	                            pip->sxfname = avp ;
	                        }
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->final.sxfile = TRUE ;
	                                pip->have.sxfile = TRUE ;
	                                pip->sxfname = argp ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* post-processing module(s) */
	                case argopt_ppm:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = paramopt_loads(&aparams,PO_PPM,
	                                argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* track PID */
	                case argopt_tpid:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecui(argp,argl,&uiw) ;
	                            pip->pid_track = (pid_t) uiw ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

	                case argopt_wait:
	                    pip->final.wait= TRUE ;
	                    pip->have.wait = TRUE ;
	                    pip->f.wait = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.wait = (rs > 0) ;
	                        }
	                    }
	                    break ;

	                case argopt_fold:
	                    pip->final.fold = TRUE ;
	                    pip->have.fold = TRUE ;
	                    pip->f.fold = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optvalue(avp,avl) ;
	                            pip->linelen = rs ;
	                            if (pip->linelen > 0) {
	                                pip->have.linelen = TRUE ;
	                            } else {
	                                pip->f.fold = FALSE ;
	                            }
	                        }
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
	                    const int	kc = (*akp & 0xff) ;

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

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'b':
	                        pip->f.background = TRUE ;
	                        break ;

	                    case 'c':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                cutoff = rs ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'r':
	                        pip->f.carriage = TRUE ;
	                        break ;

/* poll interval */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&interval) ;
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

/* line width */
	                    case 'w':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pip->have.linelen = TRUE ;
	                                pip->final.linelen = TRUE ;
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

	        } /* end if (digits as argument or not) */

	    } else {

	        rs = bits_set(&pargs,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

	if (efname == NULL) efname = getenv(VARERRORFNAME) ;
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isFailOpen(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif /* CF_DEBUG */

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

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* process the program options */

#ifdef	COMMENT
	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;
#endif /* COMMENT */

	if (argval != NULL) {
	    rs = cfdeci(argval,-1,&argvalue) ;
	}
	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	rs1 = (DEFPRECISION + 2) ;
	if ((pip->linelen < rs1) && (argvalue >= rs1)) {
	    pip->have.linelen = TRUE ;
	    pip->final.linelen = TRUE ;
	    pip->linelen = argvalue ;
	}

/* load up the environment options */

	rs = procopts(pip,&akopts) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* other arguments */

	if (cutoff < 0)
	    cutoff = 0 ;

	if (interval < 1)
	    interval = argvalue ;

	if (interval < 1)
	    interval = DEFINTERVAL ;

	rs1 = (DEFPRECISION + 2) ;
	if (pip->linelen < rs1) {
	    cp = getenv(VARLINELEN) ;

	    if (cp == NULL)
	        cp = getenv(VARCOLUMNS) ;

	    if (cp != NULL) {
	        if ((cfdeci(cp,-1,&n) >= 0) && (n >= rs1)) {
	            pip->have.linelen = TRUE ;
	            pip->final.linelen = TRUE ;
	            pip->linelen = n ;
	        }
	    }
	}

	if (pip->linelen < rs1)
	    pip->linelen = COLUMNS ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: linelen=%d\n",pip->linelen) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: track pid=%u\n",(uint) pip->pid_track) ;
#endif

	if (pip->pid_track == 0) {
	    if ((cp = getenv(VARTRACKPID)) != NULL) {
	        if (cfdecui(cp,-1,&uiw) >= 0) {
	            pip->pid_track = (pid_t) uiw ;
	        }
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: cutoff=%d interval=%d\n",cutoff,interval) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pollint=%u\n",
	        pip->progname,interval) ;

/* gather up the files to watch */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    if (argv[ai][0] == '\0') continue ;

	    cp = argv[ai] ;
	    narg += 1 ;
	    rs = vecstr_add(&files,cp,-1) ;

	    if (rs < 0) break ;
	} /* end for (looping through requested circuits) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	argfile, *afp = &argfile ;

	    if (afname[0] == '-') {
	        afname = BFILE_STDIN ;
	        f_used = TRUE ;
	    }

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            cp = lbuf ;
	            cl = len ;

	            if (cp[0] == '\0') continue ;

	            narg += 1 ;
	            rs = vecstr_add(&files,cp,cl) ;

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } /* end if */

	} /* end if (argument file) */

	if ((rs >= 0) && ((cp = getenv(VARFILE)) != NULL)) {
	    narg += 1 ;
	    rs = vecstr_add(&files,cp,-1) ;
	} /* end if */

	if ((rs >= 0) && (! f_used) && pip->f.usestdin && (narg == 0)) {
	    narg += 1 ;
	    rs = vecstr_add(&files,STDINFNAME,-1) ;
	} /* end if */

	if ((rs < 0) || (narg <= 0))
	    goto badnofiles ;

/* any filter files? */

	if (pip->have.ssfile || pip->have.sxfile) {
	    rs = strfilter_start(&sf,pip->ssfname,pip->sxfname) ;
	    if (rs < 0)
	        goto badfilterinit ;
	    sfp = &sf ;
	} /* end if (string selection-exclusion files) */

/* go into output phase */

	if ((ofname == NULL) || (ofname[0] == '\0')) ofname = BFILE_STDOUT ;

	rs = bopen(ofp,ofname,"wct",0666) ;
	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: output file unavailable (=%d)\n",
	        pip->progname,rs) ;
	    goto badoutopen ;
	}

	bcontrol(ofp,BC_LINEBUF,0) ;

/* initialize the file-watch structures */

	opts = 0 ;
	rs = vechand_start(&watchers,10,opts) ;
	pip->open.watchers = (rs >= 0) ;
	if (rs >= 0) {

	    opts = 0 ;
	    if (pip->f.carriage)
	        opts |= FILEWATCH_MCARRIAGE ;

	    rs = SR_OK ;
	    for (i = 0 ; vecstr_get(&files,i,&cp) >= 0 ; i += 1) {
	        FILEWATCH_ARGS	fa ;
	        if (cp == NULL) continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: file=%s\n",cp) ;
#endif

	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: file=%s\n",
	                pip->progname,cp) ;

/* allocate a new file-watcher */

	        size = sizeof(FILEWATCH) ;
	        rs = uc_malloc(size,&wp) ;
	        if (rs < 0)
	            break ;

	        rs = vechand_add(&watchers,wp) ;
	        if (rs < 0) {
	            uc_free(wp) ;
	            break ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("main: filewatch_start() file=%s\n",cp) ;
#endif

	        memset(&fa,0,sizeof(FILEWATCH_ARGS)) ;
	        fa.interval = interval ;
	        fa.cut = cutoff ;
	        fa.opts = opts ;
	        fa.columns = pip->linelen ;
	        fa.indent = pip->indent ;

	        rs = filewatch_start(wp,&fa,sfp,cp) ;
	        if (rs < 0)
	            break ;

	    } /* end for */

	    if (rs < 0) {

	        if (! pip->f.quiet)
	            bprintf(pip->efp,"%s: could not open file=%s\n",
	                pip->progname,cp) ;

	        for (i = 0 ; vechand_get(&watchers,i,&wp) >= 0 ; i += 1) {
	            if (wp != NULL) {
	                filewatch_finish(wp) ;
	                uc_free(wp) ;
	                vechand_del(&watchers,i--) ;
		    }
	        } /* end for */

	    } /* end if (could not open a file) */

	    pip->open.files = FALSE ;
	    vecstr_finish(&files) ;
	} /* end block */

/* OK, go into watch phase */

	if_exit = 0 ;
	if (rs >= 0) {

	    rs1 = u_getsid(0) ;
	    if (rs1 >= 0)
	        pip->pid_session = rs1 ;

	    rs = 0 ;
	    if (pip->f.background) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            debugprintf("main: PID=%d\n",getpid()) ;
	            debugprintf("main: parent ID=%d\n",getppid()) ;
	            debugprintf("main: group leader ID=%d\n",getpgrp()) ;
	            debugprintf("main: session ID=%d\n",getsid(0)) ;
	        }
#endif

	        rs = fork() ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4)) {
	            sleep(1) ;
	            debugprintf("main: PID=%d\n",getpid()) ;
	            debugprintf("main: parent ID=%d\n",getppid()) ;
	            debugprintf("main: group leader ID=%d\n",getpgrp()) ;
	            debugprintf("main: session ID=%d\n",getsid(0)) ;
	        }
#endif

	    } /* end if (background) */

	} /* end if */

	if (rs == 0) {
	    int	count = 0 ;
	    int	nfiles ;

	    uc_sigsetempty(&signalmask) ;

	    sigs.sa_handler = int_exit ;
	    sigs.sa_mask = signalmask ;
	    sigs.sa_flags = 0 ;
	    sigaction(SIGTERM,&sigs,NULL) ;

	    uc_sigsetempty(&signalmask) ;

	    sigs.sa_handler = int_exit ;
	    sigs.sa_mask = signalmask ;
	    sigs.sa_flags = 0 ;
	    sigaction(SIGHUP,&sigs,NULL) ;

	    uc_sigsetempty(&signalmask) ;

	    sigs.sa_handler = int_exit ;
	    sigs.sa_mask = signalmask ;
	    sigs.sa_flags = 0 ;
	    sigaction(SIGINT,&sigs,NULL) ;

	    uc_sigsetempty(&signalmask) ;

	    sigs.sa_handler = int_exit ;
	    sigs.sa_mask = signalmask ;
	    sigs.sa_flags = 0 ;
	    sigaction(SIGPIPE,&sigs,NULL) ;

	    while ((rs >= 0) && (! if_exit)) {

	        for (i = 0 ; i < interval ; i += 1) {
	            sleep(1) ;
	            if (if_exit) break ;
	        } /* end for */

	        if (if_exit)
	            break ;

	        if ((count % 3) == 0) {

/* is our session leader still around? */

#if	CF_SESSION
	            if ((pip->pid_session > 0) && (! isproc(pip->pid_session)))
	                break ;
#endif /* CF_SESSION */

	            if (! istrack(pip))
	                break ;

	        } /* end if (check for being orphaned from session) */

	        pip->daytime = time(NULL) ;

	        nfiles = 0 ;
	        for (i = 0 ; vechand_get(&watchers,i,&wp) >= 0 ; i += 1) {
	            if (wp == NULL) continue ;

	            rs = filewatch_check(wp,pip->daytime,ofp) ;

	            if ((rs < 0) && (rs != SR_NOENT))
	                break ;

	            if ((rs == SR_NOENT) && (! pip->f.wait)) {
	                filewatch_finish(wp) ;
	                vechand_del(&watchers,i) ;
	                uc_free(wp) ;
	            } else {
	                nfiles += 1 ;
	            }

	            rs = SR_OK ;

	        } /* end for */

	        if ((! pip->f.wait) && (nfiles <= 0)) {
	            if_exit = TRUE ;
	            break ;
	        }

	        count += 1 ;

	    } /* end while (semi-infinite loop) */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: out-of-loop rs=%d if_exit=%d\n",
	            rs,if_exit) ;
#endif

	} /* end if (child process) */

/* get out and close everything */

	for (i = 0 ; vechand_get(&watchers,i,&wp) >= 0 ; i += 1) {
	    if (wp != NULL) {
	        filewatch_finish(wp) ;
	        uc_free(wp) ;
	        vechand_del(&watchers,i--) ;
	    }
	} /* end for */

	if (pip->open.watchers) {
	    pip->open.watchers = FALSE ;
	    vechand_finish(&watchers) ;
	}

	bclose(ofp) ;

badoutopen:
	if (sfp != NULL) {
	    strfilter_finish(sfp) ;
	    sfp = NULL ;
	}

badfilterinit:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,"%s: invalid query (%d)\n",
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
	} else if (if_exit) {
	    ex = EX_TERM ;
	} else if (if_int)
	    ex = EX_INTR ;

/* we are out of here */
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
	    pip->open.errfile = TRUE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.files) {
	    pip->open.files = FALSE ;
	    vecstr_finish(&files) ;
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

badnofiles:
	ex = EX_USAGE ;
	if (! pip->f.quiet)
	    bprintf(pip->efp,"%s: no files were specified\n",
	        pip->progname) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


/* ARGSUSED */
static void int_exit(int sn)
{

	if_exit = TRUE ;
}
/* end subroutine (int_exit) */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if (rs >= 0) {
	    fmt = "%s: USAGE> %s [<file(s)> ...] [-t <interval>] [-r] [-b]\n" ;
	    rs = bprintf(pip->efp,fmt,pn,pn) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    fmt = "%s:  [-c <cutcols>] [-fold[=<linelen>]] [-wait]\n" ;
	    rs = bprintf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	if (rs >= 0) {
	    fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	    rs = bprintf(pip->efp,fmt,pn) ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		n = 0 ;
	cchar		*cp ;

	if ((cp = getenv(VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	cur ;
	    if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {
	        uint	uv ;
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&cur,&kp)) >= 0) {

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            oi = matostr(progopts,2,kp,kl) ;

	            switch (oi) {
	            case progopt_wait:
	                if (! pip->final.wait) {
	                    n += 1 ;
	                    pip->have.wait = TRUE ;
	                    pip->f.wait = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.wait = (rs > 0) ;
	                    }
	                }
	                break ;
	            case progopt_stdin:
	                if (! pip->final.usestdin) {
	                    n += 1 ;
	                    pip->have.usestdin = TRUE ;
	                    pip->f.usestdin = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.usestdin = (rs > 0) ;
	                    }
	                }
	                break ;
	            case progopt_own:
	                if (! pip->final.useown) {
	                    n += 1 ;
	                    pip->have.useown = TRUE ;
	                    pip->f.useown = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.useown = (rs > 0) ;
	                    }
	                }
	                break ;
	            case progopt_fold:
	                if (! pip->final.fold) {
	                    n += 1 ;
	                    pip->have.fold = TRUE ;
	                    pip->f.fold = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.fold = (rs > 0) ;
	                    }
	                }
	                break ;
	            case progopt_clean:
	                if (! pip->final.clean) {
	                    n += 1 ;
	                    pip->have.clean = TRUE ;
	                    pip->f.clean = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.clean = (rs > 0) ;
	                    }
	                }
	                break ;
	            case progopt_ssf:
	                if (! pip->final.ssfile) {
	                    n += 1 ;
	                    pip->have.ssfile = TRUE ;
	                    if (vl > 0) {
	                        cchar	**vpp = &pip->ssfname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                }
	                break ;
	            case progopt_sxf:
	                if (! pip->final.sxfile) {
	                    n += 1 ;
	                    pip->have.sxfile = TRUE ;
	                    if (vl > 0) {
	                        cchar	**vpp = &pip->sxfname ;
	                        rs = proginfo_setentry(pip,vpp,vp,vl) ;
	                    }
	                }
	                break ;
	            case progopt_linelen:
	                if (! pip->final.linelen) {
	                    n += 1 ;
	                    pip->have.linelen = TRUE ;
	                    pip->final.linelen = TRUE ;
	                    pip->f.linelen = TRUE ;
	                    if (vl > 0) {
	                        rs = cfdecui(vp,vl,&uv) ;
	                        pip->linelen = uv ;
	                    }
	                }
	                break ;
	            case progopt_indent:
	                if (! pip->final.indent) {
	                    n += 1 ;
	                    pip->have.indent = TRUE ;
	                    pip->final.indent = TRUE ;
	                    pip->f.indent = TRUE ;
	                    pip->indent = 8 ;
	                    if (vl > 0) {
	                        rs = cfdecui(vp,vl,&uv) ;
	                        pip->indent = uv ;
	                    }
	                }
	                break ;
	            } /* end switch */

	            if (rs < 0) break ;
	        } /* end while (looping over keys) */

	        keyopt_curend(kop,&cur) ;
	    } /* end if (keyopt-cur) */
	} /* end if (ok) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procopts) */


static int istrack(PROGINFO *pip)
{
	int		f_track = TRUE ;

	if (pip->pid_track > 0) {

#if	CF_ISPROC
	    f_track = isproc(pip->pid_track) ;
#else /* CF_ISPROC */
	    {
	        int	rs1 = u_kill(pip->pid_track,0) ;
	        f_track = (rs1 >= 0) ;
	    }
#endif /* CF_ISPROC */

	} /* end if (not-zero) */

	return f_track ;
}
/* end subroutine (istrack) */


