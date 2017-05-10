/* main */

/* main part of the FILERENAME program */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* debug menory-allocations */
#define	CF_NOHANDLEDIR	1		/* do not process directories */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

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
#include	<keyopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<nulstr.h>
#include	<ascii.h>
#include	<wdt.h>
#include	<char.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif

#define	isuc(ch)	CHAR_ISUC(ch)


/* external subroutines */

extern int	snfilemode(char *,int,mode_t) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotAccess(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* local structures */


/* external variables */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procout_begin(PROGINFO *,bfile *,cchar *) ;
static int	procout_end(PROGINFO *,bfile *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;
static int	procspecs(PROGINFO *,void *,cchar *,int) ;
static int	procspec(PROGINFO *,void *,cchar *,int) ;
static int	procspecer(PROGINFO *,void *,cchar *) ;

static int	checkname(const char *, struct ustat *, PROGINFO *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
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
	argopt_verbose,
	argopt_tmpdir,
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

static cchar *progopts[] = {
	"cvtupper",
	NULL
} ;

enum progopts {
	progopt_cvtupper,
	progopt_overlast
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

static const int	badchars[] = {
	CH_SP, 0x60, CH_DEL, '\\', ';', ':', '&', '+', '-', 0
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
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
	int		rs ;
	int		rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
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

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret0 ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

/* early things to initialize */

	pip->verboselevel = 1 ;

	pip->namelen = MAXNAMELEN ;
	pip->suffixlen = -1 ;

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

#if	CF_DEBUGS
	    debugprintf("main: a=>%t<\n",argp,argl) ;
#endif

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

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* follow links */
	                    case 'f':
	                        pip->f.follow = TRUE ;
	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->namelen = rs ;
	                            }
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl) {
	                                    rs = optvalue(avp,avl) ;
	                                    pip->namelen = rs ;
	                                }
	                            } else
	                                rs = SR_INVALID ;
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

/* print something !! */
	                    case 'p':
	                        pip->f.print = TRUE ;
	                        break ;

/* require a suffix for file names */
	                    case 's':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->suffix = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    pip->suffix = argp ;
	                            } else
	                                rs = SR_INVALID ;
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
	} else if (! isNotPresent(rs)) {
	    if (rs >= 0) rs = rs1 ;
	}

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

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
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	pip->suffixlen = -1 ;
	if (pip->suffix != NULL)
	    pip->suffixlen = strlen(pip->suffix) ;

/* OK, we do it */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    if ((rs = procopts(pip,&akopts)) >= 0) {
	        cchar	*ofn = ofname ;
	        cchar	*afn = afname ;
	        if ((rs = process(pip,&ainfo,&pargs,ofn,afn)) >= 0) {
		    if (pip->debuglevel > 0) {
			cchar	*pn = pip->progname ;
			cchar	*fmt = "%s: nodes=%u\n" ;
			bprintf(pip->efp,fmt,pn,rs) ;
		    }
		}
	    }
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;
	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;
	    case SR_INTR:
	        ex = EX_INTR ;
	        break ;
	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;
	    } /* end switch */
	} /* end if (ok) */

/* good return from program */
retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting rs=%d ex=%d\n",rs,ex) ;
#endif

	if (pip->efp != NULL) {
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

ret0:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("b_wn: final mallout=%u\n",(mo-mo_start)) ;
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

	fmt = "%s: USAGE> %s [<directory(s)> ...] [-s <suffix>] [-fpv]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(PROGINFO *pip,KEYOPT *kop)
{
	int		rs = SR_OK ;
	int		c = 0 ;
	cchar		*cp ;

	if ((cp = getourenv(pip->envv,VAROPTS)) != NULL) {
	    rs = keyopt_loads(kop,cp,-1) ;
	}

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int	oi ;
	        int	kl, vl ;
	        cchar	*kp, *vp ;

	        while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	            vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(progopts,3,kp,kl)) >= 0) {

	                switch (oi) {
	                case progopt_cvtupper:
	                    pip->f.cvtupper = TRUE ;
	                    if (vl > 0) {
	                        rs = optbool(vp,vl) ;
	                        pip->f.cvtupper = (rs > 0) ;
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


static int process(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/process: ent\n") ;
#endif

	if ((rs = procout_begin(pip,ofp,ofn)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    cchar	*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("main/process: argc=%d\n",aip->argc) ;
#endif

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
	                    rs = procspec(pip,ofp,cp,-1) ;
	                    c += rs ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0)  afn = BFILE_STDIN ;

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
	                        pan += 1 ;
	                        rs = procspecs(pip,ofp,cp,cl) ;
	                        c += rs ;
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
	        } /* end if (afile) */

	    } /* end if (argument file) */

	    if ((rs >= 0) && (pan == 0)) {
	        rs = SR_INVALID ;
	        fmt = "%s: no files or directories specified\n" ;
	        bprintf(pip->efp,fmt,pn) ;
	    }

	    rs1 = procout_end(pip,ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (procout) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int procout_begin(PROGINFO *pip,bfile *ofp,cchar *ofn)
{
	int		rs = SR_OK ;
	if (pip->f.print || (pip->verboselevel > 0)) {
	    if ((ofn == NULL) || (ofn[0] == '\0')) ofn = BFILE_STDOUT ;
	    rs = bopen(ofp,ofn,"wct",0644) ;
	    pip->ofp = ofp ;
	}
	return rs ;
}
/* end subroutine (procout_begin) */


static int procout_end(PROGINFO *pip,bfile *ofp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (pip->f.print || (pip->verboselevel > 0)) {
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	    pip->ofp = NULL ;
	}
	return rs ;
}
/* end subroutine (procout_end) */


static int procspecs(PROGINFO *pip,void *ofp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = procspec(pip,ofp,fp,fl) ;
	            c += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspecs) */


static int procspec(PROGINFO *pip,void *ofp,cchar *np,int nl)
{
	NULSTR		n ;
	int		rs ;
	int		c = 0 ;
	cchar		*name ;
	if ((rs = nulstr_start(&n,np,nl,&name)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procspec: name=>%s<\n",name) ;
#endif
	    rs = procspecer(pip,ofp,name) ;
	    c = rs ;
	    nulstr_finish(&n) ;
	} /* end if (nulstr) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procspec) */


static int procspecer(PROGINFO *pip,void *ofp,cchar name[])
{
	struct ustat	sb ;
	int		rs ;

	if (name == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("main/procspecer: name=%s\n",name) ;
#endif

	if ((rs = u_lstat(name,&sb)) >= 0) {
	int		wopts ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    char	mbuf[TIMEBUFLEN+1] ;
	    snfilemode(mbuf,TIMEBUFLEN,sb.st_mode) ;
	    debugprintf("main/procspecer: name=%s mode=%0o\n",
	        name,sb.st_mode) ;
	    debugprintf("main/procspecer: om=%s\n",mbuf) ;
	}
#endif

	if (S_ISLNK(sb.st_mode)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procspecer: LINK\n") ;
#endif
	    if (pip->f.follow) {
		struct ustat	sb2 ;
	        if ((rs = u_stat(name,&sb2)) >= 0) {
		    if (S_ISDIR(sb2.st_mode)) {
	        	wopts = (pip->f.follow) ? WDT_MFOLLOW : 0 ;
	        	rs = wdt(name,wopts,checkname,pip) ;
		    } else {
	        	if ((rs = checkname(name,&sb2,pip)) >= 0) {
		    	    rs = 1 ;
			}
		    }
		} else if (isNotAccess(rs)) {
		    rs = SR_OK ;
		}
	    } else {
	        if ((rs = checkname(name,&sb,pip)) >= 0) {
		    rs = 1 ;
		}
	    }

	} else if (S_ISDIR(sb.st_mode)) {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procspecer: DIR\n") ;
#endif
	    wopts = (pip->f.follow) ? WDT_MFOLLOW : 0 ;
	    rs = wdt(name,wopts,checkname,pip) ;
	} else {
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procspecer: REG\n") ;
#endif
	    if ((rs = checkname(name,&sb,pip)) >= 0) {
		rs = 1 ;
	    }
	}

	} /* end if (u_lstat) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procspecer: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procspecer) */


static int checkname(cchar *name,struct ustat *sbp,PROGINFO *pip)
{
	struct ustat	sb2 ;
	int		rs = SR_OK ;
	int		i ;
	int		dirlen, len, nnlen ;
	int		f_changed = FALSE ;
	const char	*bnp ;
	char		newname[MAXPATHLEN + 1] ;
	char		*nnp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: ent name=%s\n",name) ;
#endif

	if (sbp->st_ctime == 0) 
		return 1 ;

	len = sfbasename(name,-1,&bnp) ;

	if ((len > 0) && (bnp[0] == '.')) 
		return 0 ;

/* if this is a file link, see if it is a directory */

	if (S_ISLNK(sbp->st_mode)) {
	    sbp = &sb2 ;
	    rs = u_stat(name,&sb2) ;
	}

/* if this is a directory or dangling link, skip it */

#if	CF_NOHANDLEDIR
	if ((rs < 0) || S_ISDIR(sbp->st_mode))
	    return 0 ;
#endif

/* convert this name (the basename) to the desired format */

	dirlen = bnp - name ;
	nnp = newname + dirlen ;
	if (*bnp == '-') {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: leading character is minus\n") ;
#endif

	    f_changed = TRUE ;
	    *nnp = 'Þ' ;
	    bnp += 1 ;
	    nnp += 1 ;

	}

	while (*bnp) {
	    int	uch ;

	    for (i = 0 ; badchars[i] != 0 ; i += 1) {
		if (*bnp == badchars[i]) break ;
	    } /* end for */

	    uch = MKCHAR(*bnp) ;
	    if (badchars[i] != 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: bad character=>%c<\n",badchars[i]) ;
#endif

		if (uch == '-') {
	            *nnp = 'þ' ;
		} else {
	            *nnp = 'Þ' ;
		}

	    } else if (pip->f.cvtupper && (isuc(uch))) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: upper case\n") ;
#endif

	        *nnp = tolower(uch) ;

	    } else
	        *nnp = *bnp ;

	    f_changed = f_changed || (*nnp != *bnp) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        if (f_changed)
	        debugprintf("checkname: weirdo change here=>%c|%c<\n",
		((int) *bnp),((int) *nnp)) ;
	    }
#endif /* CF_DEBUG */

	    bnp += 1 ;
	    nnp += 1 ;

	} /* end while */

	*nnp = '\0' ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: f_changed=%u\n",f_changed) ;
#endif

/* check if it has a suffix already */

	if ((pip->suffixlen > 0) && (strchr(newname + dirlen,'.') == NULL)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: no suffix\n") ;
#endif

	    nnlen = nnp - (newname + dirlen) ;
	    if ((nnlen + 1 + pip->suffixlen) > pip->namelen)
	        goto badfile ;

	    f_changed = TRUE ;
	    *nnp++ = '.' ;
	    strcpy(nnp,pip->suffix) ;

	} /* end if (adding a suffix) */

	if (! f_changed) {

	    f_changed = (strcmp((name + dirlen),(newname + dirlen)) != 0) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4) && f_changed)
	    debugprintf("checkname: extra weird thing\n") ;
#endif

	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("checkname: final f_changed=%d\n",f_changed) ;
#endif

/* get out if nothing changed */

	if (! f_changed) 
		return 0 ;

	    if (pip->f.print)
	        bprintf(pip->ofp,"%s\n",name) ;

	    memcpy(newname,name,dirlen) ;

	if (pip->f.nochange) {

	    if (pip->verboselevel > 0) {
	        bprintf(pip->ofp,"original=\"%s\"\n",name) ;
	        bprintf(pip->ofp,"newname=%s\n",newname) ;
	    }

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {
	        debugprintf("checkname: old=\"%s\"\n",name) ;
	        memcpy(newname,name,dirlen) ;
	        debugprintf("checkname: new=\"%s\"\n",newname) ;
	    }
#endif /* CF_DEBUG */

	} else {
	    if ((rs = u_rename(name,newname)) < 0) {
	        bprintf(pip->efp,
	            "%s: rename failed (%d) original=\"%s\"\n",
	            pip->progname,rs,name) ;
	    } else if (pip->verboselevel > 0) {
	        bprintf(pip->ofp,"changed=%s\n",
	            newname) ;
	    }
	}

	return 0 ;

/* we would have created a file name that was too long */
badfile:
	if (pip->f.nochange || (pip->verboselevel > 0)) {
	    bprintf(pip->efp,"%s: length limit original=\"%s\"\n",
	        pip->progname,name) ;
	}

	return 1 ;
}
/* end subroutine (checkname) */


