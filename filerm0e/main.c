/* main */

/* part of 'filerm' -- remove files which meet a specified criteria */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	The program was written from scratch.

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
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<randomvar.h>
#include	<egs.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"removename.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern uint	hashelf(const char *,int) ;

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getrand(char *,int) ;
extern int	progname(PROGINFO *,const char *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	mkrn(PROGINFO *) ;


/* external variables */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"HELP",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"APPLEDOUBLE",
	"LINKS",
	"CORES",
	"name",
	"burn",
	"sn",
	"af",
	"of",
	"ef",
	"follow",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_help,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_appledouble,
	argopt_links,
	argopt_cores,
	argopt_name,
	argopt_burn,
	argopt_sn,
	argopt_af,
	argopt_of,
	argopt_ef,
	argopt_follow,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	randomvar	rv ;
	BITS		pargs ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

#if	(CF_DEBUG || CF_DEBUGS) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		v ;
	int		c_args = 0 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_outopen = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*efname = NULL ;
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
	proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->ofp = ofp ;

	pip->verboselevel = 1 ;
	pip->bcount = -1 ;

/* initialize a structure to hold names to match on */

	rs = vecstr_start(&pip->names,10,0) ;
	if (rs < 0) goto badnamestart ;

	if (rs >= 0) rs = bits_start(&pargs,1) ;
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

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
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

/* remove the dangling AppleDouble files */
	                case argopt_appledouble:
	                    pip->f.appledouble = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* remove the dangling symbolic links */
	                case argopt_links:
	                    pip->f.links = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* remove the core files */
	                case argopt_cores:
	                    pip->f.cores = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* search names */
	                case argopt_name:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            VECSTR	*nlp = &pip->names ;
	                            rs = vecstr_add(nlp,argp,argl) ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* burn the file before removal */
	                case argopt_burn:
	                    pip->f.burn = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

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

/* remove all files! */
	                    case 'a':
	                        pip->f.all = TRUE ;
	                        break ;

/* burn them as we go */
	                    case 'b':
	                        pip->f.burn = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                pip->bcount = v ;
	                            }
	                        }
	                        break ;

/* continue on error */
	                    case 'c':
	                    case 'f':
	                        pip->f.force = TRUE ;
	                        break ;

/* do NOT actually perform the act */
	                    case 'n':
	                        pip->f.no = TRUE ;
	                        break ;

/* print out the bad links */
	                    case 'p':
	                        pip->f.print = TRUE ;
	                        break ;

/* quiet STDOUT */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* recursive */
	                    case 'r':
	                        pip->f.recursive = TRUE ;
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

	                    case 'z':
	                        pip->f.zargs = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.zargs = (rs > 0) ;
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

	        } /* end if (digits or options) */

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
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

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

/* need help? */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* temporary stuff */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	pip->rn_opts = 0 ;
	pip->rn_opts |= (pip->f.follow ? REMOVENAME_MFOLLOW : 0) ;
	pip->rn_opts |= (pip->f.burn ? REMOVENAME_MBURN : 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: rn_opts=%04x\n",pip->rn_opts) ;
#endif

/* are we instructed to "burn" the files? */

	pip->rvp = NULL ;
	if (pip->f.burn) {

	    if ((pip->bcount < 0) && (argval != NULL)) {
	        rs = cfdeci(argval,-1,&v) ;
	        pip->bcount = v ;
	    }

	    if (pip->bcount < 1)
	        pip->bcount = 1 ;

	    if (rs >= 0) {
	        uint	srn ;
	        rs = mkrn(pip) ;
	        srn = rs ;
	        if (rs >= 0) {
	            pip->rvp = &rv ;
	            rs = randomvar_start(&rv,FALSE,srn) ;
	        }
	    }

/* add some extra noise */

	    if (rs >= 0) {
	        char	noise[10+1] ;
	        if ((rs = getrand(noise,(10+1))) >= 0) {
	            rs = randomvar_addnoise(&rv,noise,rs) ;
		}
	    }

	} /* end if (burn mode) */

/* OK, we do it */

	if ((rs >= 0) && ((pip->f.print || (pip->verboselevel > 0)))) {

	    if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	        ofname = BFILE_STDOUT ;

	    rs = bopen(ofp,ofname,"wct",0666) ;
	    f_outopen = (rs >= 0) ;
	    if (rs < 0) {
	        ex = EX_CANTCREAT ;
	        bprintf(pip->efp,"%s: could not open output (%d)\n",
	            pip->progname,rs) ;
	        goto badoutopen ;
	    }

	} /* end if (output) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: about to loop\n") ;
#endif

	if (rs >= 0) {
	    for (ai = 1 ; ai < argc ; ai += 1) {

	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (! f) continue ;

	        cp = argv[ai] ;
	        c_args += 1 ;
	        rs = progname(pip,cp) ;
	        if (rs < 0) {
	            bprintf(pip->efp,
	                "%s: processing error (%d) in file=%s\n",
	                pip->progname,rs,cp) ;
	            break ;
	        }

	    } /* end for (looping through requested circuits) */
	} /* end if */

/* process any files in the argument filename list file */

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

	            cp = lbuf ;
	            if (cp[0] == '\0') continue ;

	            c_args += 1 ;
	            rs = progname(pip,cp) ;
	            if (rs < 0) {
	                bprintf(pip->efp,
	                    "%s: processing error (%d) in file=%s\n",
	                    pip->progname,rs,cp) ;
	                break ;
	            }

	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {

	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: could not open argument list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: \targfile=%s\n",
	                pip->progname,afname) ;
	        }

	    } /* end if */

	} /* end if (processing file argument file list */

	if (pip->verboselevel >= 2) {

	    bprintf(ofp,"files processed %u\n",pip->c_processed) ;

	    if (pip->f.no) {
	        bprintf(ofp,"files removed   %u (would have been)\n",
	            pip->c_removed) ;
	    } else
	        bprintf(ofp,"files removed   %u\n",pip->c_removed) ;

	} /* end if */

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: files processed=%u\n",
	        pip->progname,pip->c_processed) ;

	    if (pip->f.no) {
	        bprintf(pip->efp,"%s: files removed=%u (would have been)\n",
	            pip->progname,pip->c_removed) ;

	    } else
	        bprintf(pip->efp,"%s: files removed=%u\n",
	            pip->progname,pip->c_removed) ;

	} /* end if */

	if (f_outopen) {
	    f_outopen = FALSE ;
	    bclose(ofp) ;
	}

badoutopen:
	if (pip->f.burn && (pip->rvp != NULL)) {
	    randomvar_finish(&rv) ;
	    pip->rvp = NULL ;
	}

	if ((rs >= 0) && (! pip->f.zargs) && (c_args <= 0)) {
	    rs = SR_INVALID ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: no files or directories were specified\n",
	        pip->progname) ;
	}

done:
	if ((rs < 0) && (ex == EX_OK))
	    ex = mapex(mapexs,rs) ;

/* good return from program */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

	if (pip->efp != NULL) {
	    pip->open.errfile = TRUE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	vecstr_finish(&pip->names) ;

badnamestart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("b_la: final mallout=%u\n",(mo_finish-mo_start)) ;
	}
#endif /* CF_DEBUGMALL */

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [<dir(s)> ...] [-s <search> [-s <search>]]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-APPLEDOUBLE] [-LINKS] [-CORES] [-af <argfile>] [-a]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-b[=<count>]] [-f] [-n]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int mkrn(PROGINFO *pip)
{
	EGS		e ;
	uint		rn = 0 ;
	int		rs = SR_OK ;
	int		i ;
	int		v ;
	const char	**envv = pip->envv ;

#ifdef	COMMENT
	if ((cp = getenv(VARRANDOM)) != NULL) {
	    if (cfdeci(cp,-1,&v) >= 0)
	        rn ^= v ;
	}
#endif

/* pop in our environment also! */

	for (i = 0 ; envv[i] != NULL ; i += 1)
	    rn ^= hashelf(envv[i],-1) ;

/* get some "entropy" and mix it in */

	if (egs_open(&e,ENTFNAME) >= 0) {

	    egs_read(&e,(char *) &v,sizeof(int)) ;
	    rn += v ;

	    egs_close(&e) ;
	} /* end if (was able to get some "entropy") */

	rn = (rn & INT_MAX) ;
	return (rs >= 0) ? rn : rs ;
}
/* end subroutine (mkrn) */


