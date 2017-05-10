/* main */

/* this is a generic "main" module for the WEBCOUNTER program */


#define	CF_DEBUGS	0		/* run-time debugging */
#define	CF_DEBUG	1		/* compile-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_WTMP		0		/* use WTMP */
#define	CF_SYSLEN	0		/* use UTMPX 'syslen' */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for small
	programs.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<paramopt.h>
#include	<mapstrint.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"tmpx.h"


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2w(char *,const char *,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	makedate_date(const char *,const char **) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern const char	testutmpx_makedate[] ;


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;

static int	procopts(struct proginfo *,KEYOPT *) ;
static int	procname(struct proginfo *,MAPSTRINT *,const char *) ;
static int	proclist(struct proginfo *,bfile *,const char *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"db",
	"qs",
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
	argopt_db,
	argopt_qs,
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

static const char	*akonames[] = {
	"print",
	"add",
	"inc",
	NULL
} ;

enum akonames {
	akoname_print,
	akoname_add,
	akoname_inc,
	akoname_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	BITS		pargs ;

	KEYOPT		akopts ;

	PARAMOPT	apam ;

	MAPSTRINT	names ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs ;
	int	rs1 ;
	int	i ;
	int	cl ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f_makedate = FALSE ;
	int	f_list = TRUE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*qs = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*dbfname = NULL ;
	const char	*basedname = NULL ;
	const char	*fmt ;
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

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;
	pip->f.add = TRUE ;
	pip->f.inc = TRUE ;

/* process program arguments */

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&apam) ;
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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_makedate = f_version ;
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
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->tmpdname = argp ;
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
	                            dbfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            dbfname = argp ;
	                    }
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

/* argument-list file */
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

/* get an output file name other than using STDOUT! */
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
	                        ofname = argp ;
	                    }
	                    break ;

/* quert string */
	                case argopt_qs:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            qs = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        qs = argp ;
	                    }
	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
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
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

/* add a counter to the DB if not already present */
	                    case 'a':
	                        pip->final.add = TRUE ;
	                        pip->have.add = TRUE ;
	                        pip->f.add = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.add = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'b':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        basedname = argp ;
	                        break ;

/* increment a counter */
	                    case 'i':
	                        pip->final.inc = TRUE ;
	                        pip->have.inc = TRUE ;
	                        pip->f.inc = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.inc = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* list */
	                    case 'l':
	                        f_list = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;
	                        break ;

/* print out counter values */
	                    case 'p':
	                        pip->final.print = TRUE ;
	                        pip->have.print = TRUE ;
	                        pip->f.print = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.print = (rs > 0) ;
	                            }
	                        }
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* other things */
	                    case 's':
				cp = NULL ;
				cl = -1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
					cp = avp ;
					cl = avl ;
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
	                                cp = argp ;
					cl = argl ;
				    }
	                        }
				if (cp != NULL) {
					const char	*po = PO_OPTION ;
	                                rs = paramopt_loads(&apam,po,cp,cl) ;
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
	                        f_usage = TRUE ;
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

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	    if (f_makedate) {
	        cl = makedate_date(testutmpx_makedate,&cp) ;
	        bprintf(pip->efp,"%s: makedate %t\n",
	            pip->progname,cp,cl) ;
	    }
	} /* end if */

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

/* check a few more things */

	rs = procopts(pip,&akopts) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: f_print=%u\n",
	        pip->progname,pip->f.print) ;
	    bprintf(pip->efp,"%s: f_add=%u\n",
	        pip->progname,pip->f.add) ;
	    bprintf(pip->efp,"%s: f_inc=%u\n",
	        pip->progname,pip->f.inc) ;
	    bprintf(pip->efp,"%s: f_list=%u\n",
	        pip->progname,f_list) ;
	}

/* remaining initialization */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (dbfname == NULL) dbfname = getenv(VARDB) ;
	if (dbfname == NULL) dbfname = getenv(VARDBFNAME) ;

#ifdef	COMMENT
	if (dbfname == NULL) {
	    ex = EX_UNAVAILABLE ;
	    goto baddb ;
	}
#endif /* COMMENT */

	if (basedname == NULL) basedname = getenv(VARBASEDNAME) ;

	if (qs == NULL) qs = getenv(VARQS) ;
	if (qs == NULL) qs = getenv(VARQUERYSTRING) ;

	if ((pip->debuglevel > 0) && (dbfname != NULL)) {
	    bprintf(pip->efp,"%s: dbfile=%s\n",
	        pip->progname,dbfname) ;
	}

	if ((pip->debuglevel > 0) && (basedname != NULL)) {
	    bprintf(pip->efp,"%s: basedir=%s\n",
	        pip->progname,basedname) ;
	}

	if ((pip->debuglevel > 0) && (qs != NULL)) {
	    bprintf(pip->efp,"%s: qs=>%s<\n",
	        pip->progname,qs) ;
	}

	rs = mapstrint_start(&names,10) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badnameinit ;
	}

/* open the output file (if we are also printing) */

	if (pip->f.print || f_list) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: opening output file=%s\n",ofname) ;
#endif

	    if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
		ofname = BFILE_STDOUT ;

	    rs = bopen(ofp,ofname,"wct",0666) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main/proclist: output bopen() rs=%d\n",rs) ;
#endif

	    if (rs < 0) {
	        ex = EX_CANTCREAT ;
	        bprintf(pip->efp,"%s: output unavailable (%d)\n",
	            pip->progname,rs) ;
	        goto badoutopen ;
	    }

	} /* end if (opening the output file) */

/* OK, we do it */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procname(pip,&names,cp) ;

	    if (rs < 0) break ;
	} /* end for */

/* process any names in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (strcmp(afname,"-") == 0) afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = procname(pip,&names,cp) ;

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list */

#ifdef	COMMENT
	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procname(pip,&names,cp) ;

	} /* end if (program invocation arguments) */
#endif /* COMMENT */

/* process regular requests */

	if ((rs >= 0) && f_list)
	    rs = proclist(pip,ofp,dbfname) ;

	if ((pip->f.print || f_list) && (ofp != NULL))
	    bclose(ofp) ;

badoutopen:
	mapstrint_finish(&names) ;

badnameinit:
baddb:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    fmt = NULL ;
	    switch (rs) {
	    case SR_NOENT:
	        ex = EX_NOINPUT ;
	        fmt = "%s: database unavailable (%d)\n" ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        fmt = "%s: processing error (%d)\n" ;
	        break ;
	    } /* end switch */
	    if (! pip->f.quiet) {
	        if (fmt != NULL)
	            bprintf(pip->efp,fmt, pip->progname,rs) ;
	    }
	} /* end if (error) */

/* we are out of here */
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d) \n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&apam) ;
	}

	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

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


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s [-db <dbfile>] [<name(s)> ...] [-af <afile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-a[=<b>]] [-i[=<b>]] [-p[=<b>]]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
struct proginfo	*pip ;
KEYOPT		*kop ;
{
	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	oi ;
	int	kl, vl ;
	int	c = 0 ;

	const char	*kp, *vp ;
	const char	*cp ;


	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	    if ((oi = matostr(akonames,2,kp,kl)) >= 0) {

	        switch (oi) {

	        case akoname_print:
	            if (! pip->final.print) {
	                pip->have.print = TRUE ;
	                pip->final.print = TRUE ;
	                pip->f.print = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    pip->f.print = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_add:
	            if (! pip->final.add) {
	                pip->have.add = TRUE ;
	                pip->final.add = TRUE ;
	                pip->f.add = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    pip->f.add = (rs > 0) ;
			}
	            }
	            break ;

	        case akoname_inc:
	            if (! pip->final.inc) {
	                pip->have.inc = TRUE ;
	                pip->final.inc = TRUE ;
	                pip->f.inc = TRUE ;
	                if (vl > 0) {
			    rs = optbool(vp,vl) ;
	                    pip->f.inc = (rs > 0) ;
			}
	            }
	            break ;

	        } /* end switch */

	        	c += 1 ;
	            } /* end if (valid option) */

		    if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (cursor) */
	} /* end if */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procname(pip,nlp,name)
struct proginfo	*pip ;
MAPSTRINT	*nlp ;
const char	name[] ;
{
	int	rs = SR_OK ;
	int	nlen ;
	int	v = -1 ;
	int	cl ;

	const char	*tp ;
	const char	*cp ;


	nlen = strnlen(name,MAXNAMELEN) ;

	if ((tp = strnchr(name,nlen,'=')) != NULL) {
	    nlen = (tp - name) ;
	    cp = (tp + 1) ;
	    cl = (name + nlen) - (tp + 1) ;
	    rs = cfdeci(cp,cl,&v) ;
	    if (v < 0) v = 0 ;
	}

	if (rs >= 0)
	    rs = mapstrint_add(nlp,name,nlen,v) ;

	return rs ;
}
/* end subroutine (procname) */


static int proclist(pip,ofp,dbfname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	dbfname[] ;
{
	TMPX		ut ;
	TMPX_CUR	ucur ;
	TMPX_ENT	ue, *up = &ue ;

	const uid_t	uid = getuid() ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	c = 0 ;

	const char	*cp ;

	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_WTMP
	cp = WTMPXFNAME ;
#else
	cp = UTMPXFNAME ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/proclist: ufname=%s\n",cp) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: ufname=%s\n",pip->progname,cp) ;

	if (pip->verboselevel > 1) {
	    int	rs1 = bprintf(ofp,"ufname=%s\n",cp) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/proclist: bprintf() rs=%d\n",rs1) ;
#endif
	}

	if ((rs = tmpx_open(&ut,cp,O_RDONLY)) >= 0) {
	    if ((rs = tmpx_curbegin(&ut,&ucur)) >= 0) {

	while (rs >= 0) {

	    rs1 = tmpx_enum(&ut,&ucur,up) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/proclist: tmpx_enum() rs1=%d\n",rs1) ;
#endif

	    if (rs1 == SR_NOTFOUND) break ;

	    rs = rs1 ;
	    c += 1 ;
	    if (rs >= 0) {
	            bprintf(ofp,
			"t=%u i=%-4t u=%-12t l=%-12t p=%6u e=%2d %s\n",
	            up->ut_type,
	            up->ut_id,strnlen(up->ut_id,TMPX_LID),
	            up->ut_user,strnlen(up->ut_user,TMPX_LUSER),
	            up->ut_line,strnlen(up->ut_line,TMPX_LLINE),
	            up->ut_pid,
	            up->ut_exit.e_exit,
		    timestr_logz(up->ut_tv.tv_sec,timebuf)) ;

#if	CF_SYSLEN
	            bprintf(ofp, "sl=%u host=%t\n",
			up->ut_syslen,
			up->ut_host,strnlen(up->ut_host,TMPX_LHOST)) ;
#endif
		}

	} /* end while */

	        tmpx_curend(&ut,&ucur) ;
	    } /* end if (cursor) */
	    tmpx_close(&ut) ;
	} /* end if (tmpx) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/proclist: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proclist) */



