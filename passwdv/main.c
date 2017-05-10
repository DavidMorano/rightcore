/* main */

/* main module for the 'passwdv' program */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is the front-end subroutine for the PASSWDV (password
	verification) program.


*******************************************************************************/

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	<vsystem.h>
#include	<paramopt.h>
#include	<bits.h>
#include	<bfile.h>
#include	<ids.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"pwfile.h"
#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	isinteractive() ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	process(PROGINFO *,PARAMOPT *,PWFILE *,bfile *,cchar *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strshrink(char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"option",
	"af",
	"ef",
	"pf",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_option,
	argopt_af,
	argopt_ef,
	argopt_pf,
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

static const char	*procopts[] = {
	"seven",
	NULL
} ;

enum procopts {
	procopt_seven,
	procopt_overlast
} ;

static const char	*po_option = PO_OPTION ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	BITS		pargs ;
	PARAMOPT	aparams ;
	PWFILE		pf, *pfp = NULL ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		pan = 0 ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;
	int		f_passed = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*pwfname = NULL ;
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

	pip->verboselevel = 1 ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = paramopt_start(&aparams) ;
	pip->f.aparams = (rs >= 0) ;

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some options */
	                case argopt_option:
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
	                        const char	*po = po_option ;
	                        rs = paramopt_loads(pop,po,cp,cl) ;
	                    }
	                    break ;

/* search name */
	                case argopt_sn:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            searchname = argp ;
	                    } else
	                        rs = SR_INVALID ;
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

/* PASSWD file */
	                case argopt_pf:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pwfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pwfname = argp ;
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

/* alternate PASSWD file */
	                    case 'p':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pwfname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* update */
	                    case 'u':
	                        pip->f.update = TRUE ;
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

	        } /* end if (had an argument-option) */

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
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
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

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (isinteractive() < 0) {
	    rs = SR_INVALID ;
	    ex = EX_DATAERR ;
	    bprintf(pip->efp,
	        "%s: this program can only be executed interactively\n",
	        pip->progname,rs) ;
	    goto retearly ;
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* options? */

	{
	    PARAMOPT_CUR	cur ;
	    cchar		*po = PO_OPTION ;
	    if ((rs = paramopt_curbegin(&aparams,&cur)) >= 0) {
	        while (paramopt_enumvalues(&aparams,po,&cur,&cp) >= 0) {
	            if (cp != NULL) {
	                if ((i = matostr(procopts,2,cp,-1)) >= 0) {
	                    switch (i) {
	                    case procopt_seven:
	                        pip->f.sevenbit = TRUE ;
	                        break ;
	                    } /* end switch */
		        }
		    }
	        } /* end while (option processing) */
	        paramopt_curend(&aparams,&cur) ;
	    } /* end if (paramopt-cur) */
	} /* end block */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: sevenbit=%d\n",pip->f.sevenbit) ;
#endif

	pfp = NULL ;
	if (pwfname != NULL) {

	    rs = perm(pwfname,-1,-1,NULL,R_OK) ;

	    if (rs >= 0)
	        rs = pwfile_open(&pf,pwfname) ;

	    if (rs < 0) {
	        ex = EX_SOFTWARE ;
	        bprintf(pip->efp,"%s: could not open PASSWORD file (%d)\n",
	            pip->progname,rs) ;
	        goto badpwfile ;
	    }

	    pfp = &pf ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("main: opened PWFILE \"%s\" (rs %d)\n",
	            pwfname,rs) ;
	        {
	            PWFILE_CUR	cur ;
	            PWFILE_ENT	pe ;
	            char	pebuf[PWFILE_RECLEN + 1] ;

	            pwfile_curbegin(pfp,&cur) ;
	            debugprintf("main: &pe %08lX pebuf %08lX\n",&pe,pebuf) ;
	            while (pwfile_enum(pfp,&cur,
	                &pe,pebuf,PWFILE_RECLEN) >= 0) {
	                debugprintf("username=%s\n",pe.username) ;
	                debugprintf("password=%s\n",pe.password) ;
	                debugprintf("gecos=>%s<\n",pe.gecos) ;
	                debugprintf("realname=>%s<\n",pe.realname) ;
	                debugprintf("account=%s\n",pe.account) ;
	                debugprintf("office=>%s<\n",pe.office) ;
	                debugprintf("hphone=>%s<\n",pe.hphone) ;
	            } /* end while */
	            pwfile_curend(pfp,&cur) ;
	        } /* end block */
	    } /* end if */
#endif /* CF_DEBUG */

	} /* end if (specified password file) */

/* get ready */

	if (! pip->f.nooutput) {
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0664) ;
	    if (rs < 0) {
	        ex = EX_CANTCREAT ;
	        bprintf(pip->efp,"%s: output unavailable (%d)\n",
	            pip->progname,rs) ;
	        goto badoutopen ;
	    }
	}

/* OK, we do it */

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = process(pip,&aparams,pfp,ofp,cp) ;
	    f_passed = (rs > 0) ;

	    if ((rs >= 0) && (! pip->f.nooutput)) {
	        rs = bprintf(ofp,"%svalid\n",(! f_passed) ? "in" : "") ;
	    }

	    if (rs < 0) break ;
	} /* end for (looping through arguments) */

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

	            cp = strshrink(lbuf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = process(pip,&aparams,pfp,ofp,cp) ;
	            f_passed = (rs > 0) ;

	            if ((rs >= 0) && (! pip->f.nooutput)) {
	                rs = bprintf(ofp,"%svalid\n",(! f_passed) ? "in" : "") ;
		    }

	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        bclose(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            bprintf(pip->efp,
	                "%s: argument list-file unavailable (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",
	                pip->progname,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list */

	if ((rs >= 0) && (pan <= 0)) {

	    cp = "-" ;
	    rs = process(pip,&aparams,pfp,ofp,cp) ;
	    f_passed = (rs > 0) ;

	    if ((rs >= 0) && (! pip->f.nooutput)) {
	        rs = bprintf(ofp,"%svalid\n",(! f_passed) ? "in" : "") ;
	    }

	} /* end if */

	if (! pip->f.nooutput) {
	    bclose(ofp) ;
	}

badoutopen:
bad1:
	if ((pwfname != NULL) && (pfp != NULL)) {
	    pwfile_close(pfp) ;
	}

/* good return from program */
badpwfile:
retgood:
done:
	if (rs >= 0) {
	    ex = (f_passed) ? EX_OK : EX_DATAERR ;
	} else
	    ex = EX_DATAERR ;

/* we are out of here */
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

	if (pip->open.aparams) {
	    pip->open.aparams = FALSE ;
	    paramopt_finish(&aparams) ;
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


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [<user(s)> ...] [-u] [-pf <passwdfile>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


