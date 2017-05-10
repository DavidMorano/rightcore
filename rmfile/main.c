/* main */

/* program to unlink files or directories */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory allocation */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ rmfile [-d[=delay]] [-?DVdn] file | directory | {-f file|directory}

	Arguments:

	-?		print brief help
	-D		debug mode
	-V		print version and exit
	-n		no-delete mode
	-r		recursive
	-f		take the next argument as a file or directory to
			be deleted
	-d		request a nominal delay or a specific delay


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	progfile(PROGINFO *,int,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	usage(PROGINFO *) ;


/* external variables */

extern const char	makedate[] ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_help,
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
	PROGINFO	pi, *pip = &pi ;

	KEYOPT		akopts ;

	BITS		pargs ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	uint	mo_start = 0 ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	v ;
	int	delay = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f_z = FALSE ;
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

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

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
	                    f_version = TRUE ;
	                    if (f_optequal) 
				rs = SR_INVALID ;
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

/* default action and user specified help */
	                default:
			    rs = SR_INVALID ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

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

	                        case 'Q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* program-root */
	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                        break ;

/* specify a delay to the "unlink" operation */
	                    case 'd':
	                        delay = -1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
				    if (avl) {
	                                rs = cfdeci(avp,avl,&delay) ;
				    }
	                        } /* end if */
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

	                    case 'n':
	                        pip->f.nodelete = TRUE ;
	                        break ;

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
	                        f_z = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
				    if (avl) {
	                                rs = optbool(avp,avl) ;
					f_z = (rs > 0) ;
				    }
	                        } /* end if */
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

	        } /* end if (digits or progopts) */

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

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;
	    bcontrol(pip->efp,BC_LINEBUF,0) ;
	    bflush(pip->efp) ;
	}

	if (f_version) {
		bprintf(pip->efp,"%s: version %s\n",
			pip->progname,VERSION) ;
		bprintf(pip->efp,"%s: makedate %s\n",
			pip->progname,makedate) ;
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp, "%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp, "%s: sn=%s\n",pip->progname,pip->searchname) ;
	}

	if (f_usage) 
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* go */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	     ofname = BFILE_STDOUT ;

	rs = bopen(ofp,ofname,"wct",0644) ;
	if (rs < 0) {
	     ex = EX_CANTCREAT ;
	     bprintf(pip->efp,"%s: inaccessible output file (%d)\n",
	    	pip->progname,rs) ;
	     goto badoutopen ;
	}

/* OK, we do it */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = progfile(pip,delay,cp) ;

	        if (rs < 0) {
			bprintf(pip->efp,
			"%s: processing error (%d) in file=%s\n",
			pip->progname,rs,cp) ;
			break ;
		}

	} /* end for (looping through requested circuits) */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if ((afname == NULL) || (afname[0] == '-')) afname = BFILE_STDIN ;

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
	    	    rs = progfile(pip,delay,cp) ;

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
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: \targfile=%s\n",
	                pip->progname,afname) ;
	        }

	    } /* end if */

	} /* end if (processing file argument file list) */

badoutopen:
	bclose(ofp) ;

	if ((pan == 0) && (afname == NULL) && (! f_z)) {
	    rs = SR_INVALID ;
	}

done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {

	    case SR_NOENT:
		ex = EX_NOINPUT ;
		break ;

	    case SR_INVALID:
		ex = EX_USAGE ;
		break ;

	    default:
	        ex = mapex(mapexs,rs) ;
		break ;

	    } /* end switch */
	} /* end if (bad result) */

retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo_finish ;
	    uc_mallout(&mo_finish) ;
	    debugprintf("mail: final mallout=%u\n",(mo_finish-mo_start)) ;
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
	cchar		*fmt ;

	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<file(s)>] [<directories>] | "
		"[-f <file>|<directory>] \n",
	    pn,pn) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:  [-af <argfile>] [-d[=<delay>]] [-v] [-n] [-D] [-V]\n",
		pn) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



