/* main */

/* this is the main part of the SHADOWORDER program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	The program was written from scratch to do what the previous program by
	the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<paramopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecobj.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif

#ifndef	PASSWDFNAME
#define	PASSWDFNAME	"/etc/passwd"
#endif
#ifndef	SHADOWFNAME
#define	SHADOWFNAME	"/etc/shadow"
#endif

#define	USERWORD	struct userword


/* external subroutines */

extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	ctdeci(char *,int,int) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	strnncmp(const char *,int,const char *,int) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	hasallalnum(const char *,int) ;
extern int	hasalldig(const char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* local structures */

struct userword {
	const char	*up ;
	int		ul ;
	int		line ;
	offset_t	uo ;
} ;

enum pwfields {
	pwfield_name,
	pwfield_pass,
	pwfield_uid,
	pwfield_gid,
	pwfield_gecos,
	pwfield_home,
	pwfield_shell,
	pwfield_overlast
} ;

enum shfields {
	shfield_name,
	shfield_pass,
	shfield_f3,
	shfield_f4,
	shfield_f5,
	shfield_f6,
	shfield_f7,
	shfield_f8,
	shfield_f9,
	shfield_overlast
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	process(PROGINFO *,PARAMOPT *,bfile *,const char *) ;
static int	processmore(PROGINFO *,PARAMOPT *,bfile *,VECOBJ *,cchar *) ;

static int	procprintout(PROGINFO *,PARAMOPT *,bfile *,VECOBJ *,VECOBJ *) ;

static int	procfile(PROGINFO *,PARAMOPT *,VECOBJ *,cchar *) ;

static int	procpwverify(PROGINFO *,PARAMOPT *,bfile *,VECOBJ *) ;
static int	procpwverifier(PROGINFO *,bfile *,USERWORD *,int) ;

static int	procshverify(PROGINFO *,PARAMOPT *,VECOBJ *) ;
static int	procshverifier(PROGINFO *,USERWORD *,int) ;

static int	userword_start(USERWORD *,uint,int,const char *,int) ;
static int	userword_finish(USERWORD *) ;

static int	getnumfields(PROGINFO *,USERWORD *,int,cchar *,int) ;

static int	userwordcmp(const void *,const void *) ;


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
	"dd",
	"option",
	"set",
	"follow",
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
	argopt_dd,
	argopt_option,
	argopt_set,
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
	{ SR_INVALID, EX_USAGE },
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

static const char	*progopts[] = {
	"follow",
	"nofollow",
	NULL
} ;

enum progopts {
	progopt_follow,
	progopt_nofollow,
	progopt_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	BITS		pargs ;
	PARAMOPT	aparams ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		cl ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_usage = FALSE ;
	int		f_version = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*pmspec = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ddname = NULL ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;

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

	pip->ofp = ofp ;
	pip->namelen = MAXNAMELEN ;
	pip->verboselevel = 1 ;

/* process program arguments */

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

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

/* database directory */
	                case argopt_dd:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ddname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ddname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

/* the user specified some progopts */
	                case argopt_option:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            cchar	*po = PO_OPTION ;
	                            rs = paramopt_loads(&aparams,po,argp,argl) ;
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
	                        if (argl)
	                            rs = paramopt_loadu(&aparams,argp,argl) ;
	                    } else
	                        rs = SR_INVALID ;
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
	                        if (f_optequal)
	                            rs = SR_INVALID ;
	                        break ;

/* take input file arguments from STDIN */
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
	            		    rs = optvalue(argp,argl) ;
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
	                                PARAMOPT	*pop = &aparams ;
	                                cchar		*po = PO_OPTION ;
	                                rs = paramopt_loads(pop,po,argp,argl) ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'p':
	                        pip->f.print = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.print = (rs > 0) ;
	                            }
	                        }
	                        break ;


/* require a suffix for file names */
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
	if (efname == NULL) efname = BFILE_STDERR ;
	if ((rs1 = bopen(&errfile,efname,"wca",0666)) >= 0) {
	    pip->efp = &errfile ;
	    pip->open.errfile = TRUE ;
	    bcontrol(&errfile,BC_SETBUFLINE,TRUE) ;
	} else if (! isNotPresent(rs1)) {
	    if (rs >= 0) rs = rs1 ;
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

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

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

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check a few more things */

	if (afname == NULL) afname = getenv(VARAFNAME) ;

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (ddname == NULL) ddname = ETCDNAME ;

/* get ready */

	if ((rs = paramopt_havekey(&aparams,PO_SUFFIX)) > 0) {
	    pip->f.suffix = TRUE ;
	} /* end if */

	if ((rs = paramopt_havekey(&aparams,PO_OPTION)) > 0) {
	    PARAMOPT_CUR	cur ;
	    cchar		*po = PO_OPTION ;
	    if ((rs = paramopt_curbegin(&aparams,&cur)) >= 0) {
	    while (paramopt_enumvalues(&aparams,po,&cur,&cp) >= 0) {
	        if (cp == NULL) continue ;
	        if ((kwi = matostr(progopts,2,cp,-1)) >= 0) {
	            switch (kwi) {
	            case progopt_follow:
	                pip->f.follow = TRUE ;
	                break ;
	            case progopt_nofollow:
	                pip->f.follow = FALSE ;
	                break ;
	            } /* end switch */
	        } /* end if (progopts) */
	    } /* end while */

	    rs1 = paramopt_curend(&aparams,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	    } /* end if (paramopt-cur) */
	} /* end if (progopts) */

	if (pip->pwfname == NULL) pip->pwfname = PASSWDFNAME ;

	if (pip->shfname == NULL) pip->shfname = SHADOWFNAME ;

/* start processing */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,"wct",0644)) >= 0) {
            {
	        rs = process(pip,&aparams,ofp,ddname) ;
	    }
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: output unavailable (%d)\n",
	        pip->progname,rs) ;
	}

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
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

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-p]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-dd <dbdir>] [-o <opt(s)>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int process(PROGINFO *pip,PARAMOPT *app,bfile *ofp,cchar ddname[])
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	char		pwfname[MAXPATHLEN+1] ;
	char		shfname[MAXPATHLEN+1] ;

	if (rs >= 0)
	    rs = mkpath2(pwfname,ddname,PWFNAME) ;

	if (rs >= 0)
	    rs = mkpath2(shfname,ddname,SHFNAME) ;

	if (rs >= 0) {
	    VECOBJ	pf ;
	    const int	size = sizeof(USERWORD) ;
	    if ((rs = vecobj_start(&pf,size,10,0)) >= 0) {
	        USERWORD	*wp ;
	        int		i ;

	        rs = procfile(pip,app,&pf,pwfname) ;

	        if (rs >= 0)
	            rs = procpwverify(pip,app,ofp,&pf) ;

	        if (rs >= 0) {
	            rs = processmore(pip,app,ofp,&pf,shfname) ;
	            wlen = rs ;
	        }

	        for (i = 0 ; vecobj_get(&pf,i,&wp) >= 0 ; i += 1) {
	            if (wp != NULL) {
	                userword_finish(wp) ;
	            }
	        } /* end for */

	        rs1 = vecobj_finish(&pf) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (vecobj) */
	} /* end if (ok) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (process) */


static int procfile(PROGINFO *pip,PARAMOPT *app,VECOBJ *pfp,cchar *fname)
{
	bfile		fs ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

	if (fname == NULL) return SR_FAULT ;

	if ((rs = bopen(&fs,fname,"r",0666)) >= 0) {
	    USERWORD	w ;
	    uint	lo = 0 ;
	    const int	llen = LINEBUFLEN ;
	    int		line = 1 ;
	    int		ll ;
	    const char	*lp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(&fs,lbuf,llen)) > 0) {
	        const int	len = rs ;

	        lp = lbuf ;
	        ll = len ;
	        if (lp[ll-1] == '\n') ll -= 1 ;

	        if ((ll > 0) && (lp[0] != '#')) {

	            c += 1 ;
	            if ((rs = userword_start(&w,lo,line,lp,ll)) >= 0) {
	                rs = vecobj_add(pfp,&w) ;
	                if (rs < 0)
	                    userword_finish(&w) ;
	            }

	        } /* end if */

	        lo += len ;
	        line += 1 ;

	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = bclose(&fs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened-file) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procfile: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfile) */


static int procpwverify(PROGINFO *pip,PARAMOPT *app,bfile *ofp,VECOBJ *pfp)
{
	USERWORD	pwfields[pwfield_overlast + 1] ;
	USERWORD	*wp ;
	const int	npf = pwfield_overlast ;
	int		rs = SR_OK ;
	int		ul ;
	int		n ;
	int		i ;
	int		line = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procpwverify: ent\n") ;
#endif

	for (i = 0 ; vecobj_get(pfp,i,&wp) >= 0 ; i += 1) {
	    if (wp == NULL) continue ;

	    line = wp->line ;
	    rs = getnumfields(pip,pwfields,npf,wp->up,-1) ;
	    n = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procpwverify: getnumfields() rs=%d \n",rs) ;
#endif


	    if (rs >= 0) {
	        if (n != npf) rs = SR_BADFMT ;
	    }

	    if (rs >= 0) {
	        ul = pwfields[0].ul ;
	        if ((ul == 0) || (ul > LOGNAME_MAX)) rs = SR_BADFMT ;
	    }

	    if (rs >= 0)
	        rs = procpwverifier(pip,ofp,pwfields,n) ;

	    if (rs < 0) {
		int	ul = pwfields[0].ul ;
		cchar	*up = pwfields[0].up ;
		fmt = "%s: PASSWD error line=%u (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,line,rs) ;
		fmt = "%s: PASSWD rec-name=%t\n" ;
	        bprintf(pip->efp,fmt,pn,up,ul) ;
	    }

	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procpwverify: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procpwverify) */


static int procpwverifier(PROGINFO *pip,bfile *ofp,USERWORD *fa,int fn)
{
	int		rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procpwverifier: ent fn=%d\n",fn) ;
#endif

	if (fn >= pwfield_overlast) {
	    const int	glen = GROUPNAMELEN ;
	    int		i ;
	    int		gl = 0 ;
	    cchar	*pn = pip->progname ;
	    char	gbuf[GROUPNAMELEN+1] = { 0 } ;
	    for (i = 0 ; i < pwfield_overlast ; i += 1) {
	        switch (i) {
	        case pwfield_name:
	            if (! hasallalnum(fa[i].up,fa[i].ul)) rs = SR_BADFMT ;
	            break ;
	        case pwfield_uid:
	            if (! hasalldig(fa[i].up,fa[i].ul)) rs = SR_BADFMT ;
	            break ;
	        case pwfield_gid:
	            if (hasalldig(fa[i].up,fa[i].ul)) {
	                const int	ul = fa[i].ul ;
	                int		v ;
	                cchar		*up = fa[i].up ;
	                if ((rs = cfdeci(up,ul,&v)) >= 0) {
	                    const gid_t	gid = v ;
	                    rs = getgroupname(gbuf,glen,gid) ;
	                    gl = rs ;
	                }
	            } else {
	                rs = SR_BADFMT ;
	            }
	            break ;
	        case pwfield_home:
	            if (fa[i].ul == 0) rs = SR_BADFMT ;
	            break ;
	        } /* end switch */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procpwverifier: fi=%d rs=%d\n",i,rs) ;
#endif
	        if (rs < 0) break ;
	    } /* end for */
	    if (pip->debuglevel > 0) {
	        const int	ul = fa[0].ul ;
	        cchar		*up = fa[0].up ;
	        cchar		*fmt = "%s: u=%-8t g=%-8t\n" ;
	        bprintf(pip->efp,fmt,pn,up,ul,gbuf,gl) ;
	    }
	    if (pip->verboselevel > 1) {
	        const int	ul = fa[0].ul ;
	        cchar		*up = fa[0].up ;
	        cchar		*fmt = "u=%-8t g=%-8t\n" ;
	        bprintf(ofp,fmt,up,ul,gbuf,gl) ;
	    }
	} else
	    rs = SR_BADFMT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procpwverifier: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procpwverifier) */


static int processmore(pip,app,ofp,pfp,shfname)
PROGINFO	*pip ;
PARAMOPT	*app ;
bfile		*ofp ;
VECOBJ		*pfp ;
const char	shfname[] ;
{
	VECOBJ		sh ;
	const int	size = sizeof(USERWORD) ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/processmore: ent\n") ;
#endif

	if ((rs = vecobj_start(&sh,size,10,0)) >= 0) {
	    USERWORD	*wp ;
	    int		i ;

	    rs = procfile(pip,app,&sh,shfname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/processmore: procfile() rs=%d\n",rs) ;
#endif

	    if (rs >= 0)
	        rs = procshverify(pip,app,&sh) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/processmore: procverify() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = procprintout(pip,app,ofp,pfp,&sh) ;
	        wlen = rs ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/processmore: procprintout() rs=%d\n",rs) ;
#endif

	    for (i = 0 ; vecobj_get(&sh,i,&wp) >= 0 ; i += 1) {
	        if (wp != NULL) {
	            userword_finish(wp) ;
	        }
	    } /* end for */

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/processmore: finish-up rs=%d\n",rs) ;
#endif

	    rs1 = vecobj_finish(&sh) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecobj) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/proessmore: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (processmore) */


static int procshverify(PROGINFO *pip,PARAMOPT *app,VECOBJ *sfp)
{
	USERWORD	shfields[shfield_overlast + 1] ;
	USERWORD	*wp ;
	const int	nf = shfield_overlast ;
	int		rs = SR_OK ;
	int		ul ;
	int		n ;
	int		i ;
	int		line = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procshverify: ent nf=%u\n",nf) ;
#endif

	for (i = 0 ; vecobj_get(sfp,i,&wp) >= 0 ; i += 1) {
	    if (wp == NULL) continue ;

	    line = wp->line ;
	    rs = getnumfields(pip,shfields,nf,wp->up,-1) ;
	    n = rs ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procpf: getnumfields() rs=%d \n",rs) ;
#endif


	    if (rs >= 0) {
	        if (n != nf) rs = SR_BADFMT ;
	    }

	    if (rs >= 0) {
	        ul = shfields[0].ul ;
	        if ((ul == 0) || (ul > LOGNAME_MAX)) rs = SR_BADFMT ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procshverify: mid rs=%d \n",rs) ;
#endif

	    if (rs >= 0) {
	        rs = procshverifier(pip,shfields,n) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main/procshverify: fin rs=%d \n",rs) ;
#endif

	    if (rs < 0) break ;
	} /* end for */

	if (rs < 0) {
	    bprintf(pip->efp,"%s: SHADOW error line=%u\n",
	        pip->progname,line) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procshverify: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procpwverify) */


static int procshverifier(PROGINFO *pip,USERWORD *fa,int fn)
{
	int		rs = SR_OK ;

	if (fn >= shfield_overlast) {
	    int		i ;
	    for (i = 0 ; i < shfield_overlast ; i += 1) {

	        switch (i) {
	        case shfield_name:
	            if (! hasallalnum(fa[i].up,fa[i].ul)) rs = SR_BADFMT ;
	            break ;
	        case shfield_f3:
	            if ((fa[i].ul > 0) && (! hasalldig(fa[i].up,fa[i].ul)))
	                rs = SR_BADFMT ;
	            break ;
	        case shfield_f5:
	            if ((fa[i].ul > 0) && (! hasalldig(fa[i].up,fa[i].ul)))
	                rs = SR_BADFMT ;
	            break ;
	        case shfield_f6:
	            if ((fa[i].ul > 0) && (! hasalldig(fa[i].up,fa[i].ul)))
	                rs = SR_BADFMT ;
	            break ;
	        } /* end switch */

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main/procshverifier: fi=%u rs=%d \n",i,rs) ;
#endif

	        if (rs < 0) break ;
	    } /* end for */
	} else
	    rs = SR_BADFMT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procshverifier: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procshverifier) */


static int procprintout(pip,app,ofp,pfp,sfp)
PROGINFO	*pip ;
PARAMOPT	*app ;
bfile		*ofp ;
VECOBJ		*pfp ;
VECOBJ		*sfp ;
{
	USERWORD	*wpp, *wsp ;
	int		rs = SR_OK ;
	int		i ;
	int		line = 0 ;
	int		wlen = 0 ;
	int		f = TRUE ;

	for (i = 0 ; vecobj_get(pfp,i,&wpp) >= 0 ; i += 1) {
	    if (wpp != NULL) {

	    line = wpp->line ;
	    rs = vecobj_search(sfp,wpp,userwordcmp,&wsp) ;
	    f = (rs >= 0) ;

	    if ((rs >= 0) && pip->f.print) {
	        rs = bprintln(ofp,wsp->up,-1) ;
	        wlen += rs ;
	    }

	    }
	    if (rs < 0) break ;
	} /* end for */

	if ((rs < 0) && (! f)) {
	    bprintf(pip->efp,"%s: SHADOW entry missing at PASSWD line=%u\n",
	        pip->progname,line) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprintout) */


#ifdef	COMMENT

static int procpf(pip,app,pfp)
PROGINFO	*pip ;
PARAMOPT	*app ;
VECOBJ		*pfp ;
{
	struct ustat	sb ;
	size_t		msize ;
	uint		lo = 0 ;
	int		rs = SR_OK ;
	int		fd = -1 ;
	int		line = 1 ;
	int		mprot ;
	int		mflags ;
	int		ml ;
	int		ll ;
	int		cl ;
	int		len ;
	int		n = 0 ;
	const char	*pfname = PASSWDFNAME ;
	const char	*mp ;
	const char	*lp ;
	const char	*tp ;
	const char	*cp ;
	char		*mdata ;

	rs = u_open(pfname,O_RDONLY,0666) ;
	fd = rs ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_fstat(fd,&sb) ;
	if (rs < 0)
	    goto ret1 ;

	if (! S_ISREG(sb.st_mode)) {
	    rs = SR_NOTSUP ;
	    goto ret1 ;
	}

	if (sb.st_size == 0)
	    goto ret1 ;

	msize = sb.st_size ;
	mprot = PROT_READ ;
	mflags = MAP_SHARED ;
	rs = u_mmap(NULL,msize,mprot,mflags, fd,0L,&mdata) ;
	if (rs < 0)
	    goto ret1 ;

/* read all of the words in */

	mp = mdata ;
	ml = msize ;
	while ((tp = strnchr(mp,ml,'\n')) != NULL) {

	    len = ((tp + 1) - mp) ;

	    lp = mp ;
	    ll = (len - 1) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procpf: line=>%t<\n",lp,ll) ;
#endif

	    if ((tp = strnchr(lp,ll,'#')) != NULL)
	        ll = (tp - mp) ;

	    if (ll > 0) {
	        rs = procpfline(pip,pfp,lo,lp,ll) ;
	    } else
	        rs = SR_BADFMT ;

	    if (rs < 0) {
	        bprintf(pip->efp,"%s: PASSWD error line=%u\n",
	            pip->progname,line) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procpf: rs=%d line=%u\n",rs,line) ;
#endif

	    if (rs < 0)
	        break ;

	    line += 1 ;
	    lo += len ;

	    mp += len ;
	    ml -= len ;

	} /* end while */

ret2:
	if (mdata != NULL)
	    u_munmap(mdata,msize) ;

ret1:
	if (fd >= 0)
	    u_close(fd) ;

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procpf) */

#endif /* COMMENT */


static int userword_start(USERWORD *wp,uint lo,int line,cchar *sp,int sl)
{
	int		rs ;
	const char	*tp ;
	const char	*cp ;

	wp->up = NULL ;
	wp->ul = 0 ;
	wp->line = line ;
	wp->uo = lo ;
	if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	    wp->up = cp ;
	    if ((tp = strnchr(sp,sl,':')) != NULL) {
	        wp->ul = (tp-sp) ;
	    }
	}

	return rs ;
}
/* end subroutine (userword_start) */


static int userword_finish(USERWORD *wp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (wp->up != NULL) {
	    rs1 = uc_free(wp->up) ;
	    if (rs >= 0) rs = rs1 ;
	    wp->up = NULL ;
	}
	wp->ul = 0 ;

	return rs ;
}
/* end subroutine (userword_finish) */


static int getnumfields(PROGINFO *pip,USERWORD *fa,int nf,cchar *lp,int ll)
{
	int		rs = SR_OK ;
	int		sl ;
	int		fl ;
	int		n = 0 ;
	const char	*sp ;
	const char	*tp ;
	const char	*fp ;

	sp = lp ;
	sl = strnlen(lp,ll) ;

	while ((n < nf) && ((tp = strnchr(sp,sl,':')) != NULL)) {

	    fp = sp ;
	    fl = (tp - sp) ;

	    fa[n].up = fp ;
	    fa[n].ul = fl ;
	    n += 1 ;

	    sl -= ((tp+1)-sp) ;
	    sp = (tp+1) ;

	} /* end while */

	if ((sl >= 0) && (n < nf)) {
	    fa[n].up = sp ;
	    fa[n].ul = sl ;
	    n += 1 ;
	}

	fa[n].up = NULL ;
	fa[n].ul = 0 ;
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getnumfields) */


static int userwordcmp(const void *v1p,const void *v2p)
{
	USERWORD	*e1p, **e1pp = (USERWORD **) v1p ;
	USERWORD	*e2p, **e2pp = (USERWORD **) v2p ;
	int		rc = 0 ;

	if (*e1pp != NULL) {
	    if (*e2pp != NULL) {
	        e1p = *e1pp ;
	        e2p = *e2pp ;
	        rc = strnncmp(e1p->up,e1p->ul,e2p->up,e2p->ul) ;
	    } else
	        rc = -1 ;
	} else
	    rc = 1 ;

	return rc ;
}
/* end subroutine (userwordcmp) */


