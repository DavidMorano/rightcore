/* main */

/* check summing program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This subroutine was written for Rightcore Network Services (RNS). The
        program handles very large (greater than 1Gbyte) files without any
        fudging problems.

	= 2017-02-01, David A­D­ Morano
        Refactored somewhat.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        Compute a checksum (POSIX 'cksum' style) on the data passing from input
        to output.

	Synospsis:

	$ cksumpass -s <ansfile> [<input>] > <outfile>


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<cksum.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		1

#define	DEFRECLEN	((BLOCKSIZE * 126) * 10)
#define	MAXRECLEN	((BLOCKSIZE * 128) * 10)


/* external subroutines */

extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	procerrstat(PROGINFO *) ;
static int	procout(PROGINFO *,cchar *) ;
static int	procreduce(PROGINFO *) ;
static int	procdivide(PROGINFO *,int) ;


/* local variables */

static cchar	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"option",
	"set",
	"follow",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
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


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	CKSUM		sum ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	uint		sv ;
	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		rs = SR_OK ;
	int		i, len, npa ;
	int		ofd = 1 ;
	int		ifd ;
	int		trec_t, trec_f, trec_p ;
	int		bytes, blocks ;
	int		maxnrec = MAXNREC ;
	int		reclen = DEFRECLEN ;
	int		nfile = 0 ;
	int		ex = EX_INFO ;
	int		f_version = FALSE ;
	int		f_verbose = FALSE ;
	int		f_usage = FALSE ;
	int		f_ignore = FALSE ;
	int		f_ignorezero = FALSE ;
	int		f_zero ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	cchar		*pr = NULL ;
	cchar		*sn = NULL ;
	cchar		*infname = NULL ;
	cchar		*sumfname = NULL ;
	cchar		*blockbuf = NULL ;
	cchar		*reclenp = NULL ;
	cchar		*cp ;

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

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;

/* process program arguments */

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
	        const int ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            argval = (argp+1) ;

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

	            if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

	                switch (kwi) {

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

/* the user specified some key options */
	                case argopt_option:
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
	                    const int	kc = MKCHAR(*aop) ;

	                    switch (kc) {

	                    case 'V':
	                        f_version = TRUE ;
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

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* optional division */
	                    case 'd':
	                        pip->f.divide = TRUE ;
	                        break ;

/* ignore mode */
	                    case 'i':
	                        pip->f.ignore = TRUE ;
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

/* reduce domain variable */
	                    case 'c':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
	                                pip->compressor = rs ;
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

	        } /* end if (digits or key options) */

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

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
	debugprintf("main: npa=%d\n",npa) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp, "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;
	    bcontrol(pip->efp,BC_LINEBUF,0) ;
	    bflush(pip->efp) ;
	}

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	}

/* get the program root */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: pr=%s\n",pip->pr) ;
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

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* OK, we do it */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    cchar	*ofn = ofname ;
	    cchar	*afn = afname ;
	    if ((rs = procargs(pip,aip,bop,afn)) >= 0) {
	        const int	nfiles = rs ;
	        if ((rs = procerrstat(pip)) >= 0) {
	            if ((rs = procreduce(pip)) >= 0) {
	                if ((rs = procdivide(pip,nfiles)) >= 0) {
	                    rs = procout(pip,ofn) ;
	                }
	            } /* end if (procreduce) */
	        } /* end if (procerrstat) */
	    } /* end if (procargs) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	}

	if (pip->pairs != NULL) {
	    uc_free(pip->pairs) ;
	    pip->pairs = NULL ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        if (! pip->f.quiet) {
	            cchar	*fmt = "%s: invalid usage (%d)\n" ;
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

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
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
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

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

badarg:
	{
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;
	    ex = EX_USAGE ;
	    fmt = "%s: invalid argument specified (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	}
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<file(s)> ...] [-af <afile>] [-d]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-c <factor>] [-d]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nfiles = 0 ;
	int		pan = 0 ;
	int		cl ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	    cchar	**argv = aip->argv ;
	    for (ai = 1 ; ai < aip->argc ; ai += 1) {

	        f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	        f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            cp = argv[ai] ;
	            if (cp[0] != '\0') {
	                pan += 1 ;
	                rs = progfile(pip,cp) ;
	                nfiles += 1 ;
	            }
	        }

	        if (rs < 0) break ;
	    } /* end for (looping through requested circuits) */
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
	                    lbuf[((cp-lbuf)+cl)] = '\0' ;
	                    rs = progfile(pip,cp) ;
	                    nfiles += 1 ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        fmt = "%s: inaccessible argument-list (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	        bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    }

	} /* end if (processing file argument file list) */

	return (rs >= 0) ? nfiles : rs ;
}
/* end subroutine (procargs) */



	                if (maxnrec < 0)
				maxnrec = MAXNREC ;

	            } else {

	                aop = argp ;
	                aol = argl ;
	                while (--aol) {

	                    akp += 1 ;
	                    switch ((int) *aop) {

	                    case 'D':
	                        pip->debuglevel = 2 ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* record length */
	                    case 'b':
	                    case 'r':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[i++] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl > 0) reclenp = argp ;

	                        break ;

	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* file to receive the cksum answer in */
			case 's':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[i++] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) sumfname = argp ;

	                        break ;

	                    case 'v':
	                        f_verbose = TRUE ;
	                        break ;

	                    case 'z':
	                        f_ignorezero = TRUE ;
	                        break ;

	                    default:
	                        bprintf(pip->efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } ; /* end switch */

	                } /* end while */

	            } /* end if */

	        } else {

	            npa += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (npa < NPARG) {

	            switch (npa) {

	            case 0:
	                if (argl > 0) infname = argp ;

	                break ;

	            default:
	                break ;
	            }

	            npa += 1 ;

	        } else {

			ex = EX_USAGE ;
			f_usage = TRUE ;
	            bprintf(pip->efp,"%s: extra arguments specified\n",
	                pip->progname) ;

	        }

	    } /* end if */

	} /* end while (arguments) */


/* done w/ arguments, now handle miscellaneous */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	    goto retearly ;
	}

	if (f_usage) 
		goto usage ;


/* check arguments */

	if (reclenp == NULL) {

	    reclen = DEFRECLEN ;
	    if (pip->debuglevel > 0)
		bprintf(pip->efp,
	        "%s: no record length given, using default %d\n",
	        pip->progname,reclen) ;

	} else {
		int	mf ;

#ifdef	COMMENT
	    l = strlen(reclenp) ;

	    mf = 1 ;
	    if (l > 0) {

	        if (reclenp[l - 1] == 'b') {

	            mf = BLOCKSIZE ;
	            reclenp[l-- - 1] = '\0' ;

	        } else if (reclenp[l - 1] == 'k') {

	            mf = 1024 ;
	            reclenp[l-- - 1] = '\0' ;

	        } else if (tolower(reclenp[l - 1]) == 'm') {

	            mf = 1024 * 1024 ;
	            reclenp[l-- - 1] = '\0' ;

	        }

	    }

	    if (pip->debuglevel > 0) 
		bprintf(pip->efp, "%s: record string \"%s\"\n",
	        pip->progname,reclenp) ;

	    if ((rs = cfdec(reclenp,l,&reclen)) < 0)
	        goto badarg ;

	    reclen = reclen * mf ;
#else
	    if ((rs = cfdecmfi(reclenp,-1,&reclen)) < 0)
	        goto badarg ;

#endif /* COMMENT */

	} /* end if */

	if (reclen > MAXRECLEN) {

	    reclen = MAXRECLEN ;
		if (! pip->f.quiet)
	    bprintf(pip->efp,
		"%s: record length is too large - reduced to %d\n",
	        pip->progname,reclen) ;

	}

	if ((sumfname != NULL) && (sumfname[0] != '\0'))
		bopen(ofp,sumfname,"wct",0666) ;


	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: running with record size %d\n",
	    pip->progname,reclen) ;


/* allocate the buffer for the date from the tape */

	if ((blockbuf = valloc(reclen)) == NULL)
		goto badalloc ;


/* open files */

	if (infname != NULL) {
	    if ((ifd = u_open(infname,O_RDONLY,0666)) < 0)
	        goto badinfile ;
	} else 
	    ifd = 0 ;


/* finally go through the loops */

	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: about to enter while loop\n",
	    pip->progname) ;

	f_zero = FALSE ;
	trec_f = trec_p = 0 ;
	bytes = blocks = 0 ;


	cksum_start(&sum) ;

	while ((rs = u_read(ifd,blockbuf,reclen)) > 0) {
	    len = rs ;

	    if ((rs = cksum_accum(&sum,blockbuf,len)) >= 0) {
		rs = u_write(ofd,blockbuf,len) ;
	    }

		if (len == reclen) {
			trec_f += 1 ;
		} else {
			trec_p += 1 ;
		}

		bytes += len ;
		if (bytes >= BLOCKSIZE) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("main: block increment, bytes=%d\n",bytes) ;
#endif

			blocks += (bytes / BLOCKSIZE) ;
			bytes = bytes % BLOCKSIZE ;

		}

	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
		debugprintf("main: out of loop\n") ;
#endif

	trec_t = trec_f + trec_p ;

	bprintf(ofp,"records\t\tP=%ld F=%ld T=%ld (record size=%db)\n",
	    trec_p,trec_f,trec_t,
		(reclen / BLOCKSIZE)) ;

	bprintf(ofp,"UNIX blocks\t%d remaining bytes %d\n",
	    blocks,bytes) ;

	cksum_getsum(&sum,&sv) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
		rs = cksum_getlen(&sum,&len) ;
		debugprintf("main: rs=%d len=%u\n",rs,len) ;
	}
#endif

	bprintf(ofp,"cksum\t\t\\x%08x (%u)\n",sv,sv) ;

	bprintf(ofp,"size\t\t%d Mibytes %d bytes\n",
	    (blocks / 2048),
	    (((blocks % 2048) * BLOCKSIZE) + bytes)) ;

	bclose(ofp) ;


	cksum_finish(&sum) ;


/* finish off */

	u_close(ifd) ;

	u_close(ofd) ;

done:
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

/* usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [-s ansfile] [infile(s) ...]",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp," [-V]\n") ;

	goto badret ;

/* bad arguments */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: bad argument given (rs %d)\n",
	    pip->progname,rs) ;

	goto ret1 ;

/* other bad */
badalloc:
	bprintf(pip->efp,"%s: could not allocate buffer memory\n",
	    pip->progname) ;

	goto badret ;

badinfile:
	bprintf(pip->efp,"%s: cannot open the input file (rs %d)\n",
	    pip->progname,ifd) ;

	goto badret ;

badoutopen:
	bprintf(pip->efp,"%s: cannot open the output file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto ret1 ;

}
/* end subroutine (main) */


/* local subroutines */


