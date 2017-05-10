/* main */

/* front-end subroutine for the RMER program */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable */
#define	CF_SORTENTRIES	0		/* sort entries */
#define	CF_MSGDISCARD	0		/* use message-discard mode */


/* revision history:

	= 1998-06-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ rmer [-af <afile>] [<file(s)>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"rmermsg.h"
#include	"msgbuf.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	BUFLEN		4096

#ifndef	TO_READ
#define	TO_READ		4
#endif

#define	MAXDELAY	(365 * 24 * 3600)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct entry {
	const char	*fname ;
	time_t		ti_expire ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *) ;
static int	process(PROGINFO *,int) ;
static int	procafiles(PROGINFO *,vecobj *) ;
static int	procin(PROGINFO *,vecobj *,int) ;
static int	procin_file(PROGINFO *,vecobj *,struct rmermsg_fname *) ;
static int	procoff(PROGINFO *,vecobj *) ;
static int	procthem(PROGINFO *,vecobj *) ;
static int	filecheck(PROGINFO *,const char *,time_t) ;
static int	findmin(PROGINFO *,vecobj *) ;
static int	vecobj_finishfiles(vecobj *) ;

#if	CF_SORTENTRIES
static int	cmpentry(struct entry **,struct entry **) ;
#endif

static int	entry_start(struct entry *,const char *,time_t) ;
static int	entry_finish(struct entry *) ;

#ifdef	COMMENT
static int	filedel(PROGINFO *,const char *) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"sn",
	"ef",
	"if",
	"of",
	"af",
	"md",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_sn,
	argopt_ef,
	argopt_if,
	argopt_of,
	argopt_af,
	argopt_md,
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
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int	rs, rs1 ;
	int	v ;
	int	fd_in = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_version = FALSE ;
	int	f_inclose = FALSE ;

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
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

#if	CF_DEBUG || CF_DEBUGS
	cp = getenv(VARDEBUGFNAME) ;
	bprintf(pip->efp,"%s: debugfname=%s\n",
	    pip->progname,cp) ;
#endif

	pip->verboselevel = 1 ;
	pip->pagesize = getpagesize() ;

	pip->daytime = time(NULL) ;

/* key options */

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

/* input file */
	                case argopt_if:
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

/* set message-discard mode */
	                case argopt_md:
			    pip->f.msgdiscard = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
				    pip->f.msgdiscard = (rs > 0) ;
				}
	                    } 
			    break ;

/* help */
	                case argopt_help:
	                    f_help = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

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

/* quiet */
	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* timeout */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                pip->to_file = v ;
				    }
				} else
	                            rs = SR_INVALID ;
	                        break ;

/* verbose (level) */
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

/* program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

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

	if (f_usage || f_version || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* check some arguments */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: progdname=%s\n",
	        pip->progname,pip->progdname) ;
	}

/* other initialization */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (pip->to_file <= 0)
	   pip->to_file = TO_FILE ;

	pip->euid = geteuid() ;
	pip->uid = getuid() ;

	if (pip->euid != pip->uid) {
	    u_setreuid(pip->euid,-1) ;
	}

	if ((rs = vecstr_start(&pip->afiles,0,0)) >= 0) {

/* do some argument processing */

	if (rs >= 0) {
	    ARGINFO	*aip = &ainfo ;
	    BITS	*bop = &pargs ;
	    cchar	*afn = afname ;
	    rs = procargs(pip,aip,bop,afn) ;
	}

/* open the input */

	fd_in = FD_STDIN ;
	if ((rs >= 0) && (ifname != NULL) && (ifname[0] != '\0')) {
	    int		ch = MKCHAR(ifname[0]) ;
	    u_close(fd_in) ;
	    if (isdigitlatin(ch)) {
	        rs = cfdeci(ifname,-1,&fd_in) ;
	    } else {
		f_inclose = TRUE ;
	        rs = uc_open(ifname,O_RDONLY,0666) ;
	    }
	    fd_in = rs ;
	}

	if (rs >= 0) {

	    if (pip->f.msgdiscard) {
		uc_msgdiscard(fd_in) ;
	     }

	    rs = process(pip,fd_in) ;

	    if (f_inclose) {
	        u_close(fd_in) ;
		fd_in = -1 ;
	    }

	} /* end if (input) */

	    rs1 = vecstr_finish(&pip->afiles) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecstr) */

/* done */
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

/* bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument (%d)\n",
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
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-t <timeout>] [-if <infile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		pan = 0 ;
	int		c = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*cp ;

	if (rs >= 0) {
	    int		ai ;
	    int		f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                if (cp[0] != '\0') {
	                    pan += 1 ;
	    		    rs = vecstr_adduniq(&pip->afiles,cp,-1) ;
			    if (rs < INT_MAX) c += 1 ;
	                }
	            }

	            if (rs < 0) break ;
	        } /* end for */
	} /* end if (ok) */

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;

	    if (afn[0] == '-') afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
		int		len ; 
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

		    if (len > 0) {
		    pan += 1 ;
		    rs = vecstr_adduniq(&pip->afiles,lbuf,len) ;
			    if (rs < INT_MAX) c += 1 ;
		    }

		    if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
		if (rs >= 0) rs = rs1 ;
	    } else {
		    fmt = "%s: inaccessible argument-list (%d)\n" ;
	            bprintf(pip->efp,fmt,pn,rs) ;
	            bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (argument list file) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procargs) */


static int process(PROGINFO *pip,int fd)
{
	vecobj		flist ;
	int		rs ;
	int		size ;
	int		opts ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: fd=%d\n",fd) ;
#endif

#ifdef	COMMENT		/* not necessary */
	opts = VECOBJ_OCOMPACT ;
#else
	opts = 0 ;
#endif

	size = sizeof(struct entry) ;
	if ((rs = vecobj_start(&flist,size,10,opts)) >= 0) {

	    rs = procafiles(pip,&flist) ;
	    c += rs ;

	    if ((rs >= 0) && (c == 0)) {
	       rs = procin(pip,&flist,fd) ;
	       c += rs ;
	    }

	    if (rs >= 0) {
	        pip->c_files = c ;
	        if (pip->debuglevel > 0)
	            bprintf(pip->efp,"%s: files=%u\n", pip->progname,c) ;
	    }

	    if ((rs >= 0) && (c > 0)) {
	        rs = procoff(pip,&flist) ;
	    }

	    vecobj_finishfiles(&flist) ;

	    vecobj_finish(&flist) ;
	} /* end if (vecobj) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/process: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


static int procafiles(pip,flp)
PROGINFO	*pip ;
vecobj		*flp ;
{
	struct entry	e ;

	time_t		ti_e ;

	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	const char	*cp ;

	for (i = 0 ; vecstr_get(&pip->afiles,i,&cp) >= 0 ; i += 1) {
	    if (cp != NULL) {
	        ti_e = (pip->daytime + pip->to_file) ;
	        if ((rs = entry_start(&e,cp,ti_e)) >= 0) {
		    c += 1 ;
	            rs = vecobj_add(flp,&e) ;
	            if (rs < 0)
	                entry_finish(&e) ;
	        }
	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procafiles) */


static int procin(PROGINFO *pip,vecobj *flp,int fd)
{
	struct rmermsg_fname	m0 ;
	struct rmermsg_unknown	mu ;
	MSGBUF		mb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		msgtype ;
	int		m0_size = sizeof(struct rmermsg_fname) ;
	int		size ;
	int		mlen ;
	int		to = TO_READ ;
	int		c = 0 ;

	size = MAX(m0_size,pip->pagesize) ;
	if ((rs = msgbuf_start(&mb,fd,size,to)) >= 0) {
	    int		ml ;
	    cchar	*mp ;

	    while ((rs = msgbuf_read(&mb,&mp)) > 0) {
	        ml = rs ;

	    msgtype = MKCHAR(mp[0]) ;
	    switch (msgtype) {

	    case rmermsgtype_fname:
	        mlen = rmermsg_fname(&m0,1,(char *) mp,ml) ;
	        if (mlen > 0) {
	            rs = procin_file(pip,flp,&m0) ;
	            c += rs ;
	        }
	        break ;

	    default:
	        mlen = rmermsg_unknown(&mu,1,(char *) mp,ml) ;
	        if (mlen > 0)
	            mlen = mu.msglen ;
	        break ;

	    } /* end switch */

	    if (rs >= 0) {
	        rs = msgbuf_adv(&mb,mlen) ;
	    }

	    if (rs < 0) break ;
	} /* end while */

	    rs1 = msgbuf_finish(&mb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (msgbuf) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procin) */


static int procin_file(pip,flp,mp)
PROGINFO	*pip ;
vecobj		*flp ;
struct rmermsg_fname	*mp ;
{
	struct entry	e ;
	time_t		ti_e ;
	int		rs = SR_OK ;
	int		delay ;
	int		c = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procin_file: f=%s d=%u\n",
	        mp->fname,mp->delay) ;
#endif

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: R file=%s (%u)\n",
	        pip->progname,mp->fname,mp->delay) ;
	}

	if (mp->fname[0] != '\0') {
	    delay = MIN(mp->delay,MAXDELAY) ;
	    ti_e = (pip->daytime + delay) ;
	    if ((rs = entry_start(&e,mp->fname,ti_e)) >= 0) {
	        c += 1 ;
	        rs = vecobj_add(flp,&e) ;
	        if (rs < 0)
	            entry_finish(&e) ;
	    }
	} /* end if (non-empty) */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procin_file) */


static int procoff(PROGINFO *pip,vecobj *flp)
{
	int		rs ;

	bflush(pip->efp) ;

	if ((rs = uc_fork()) == 0) {
	    int		ex = EX_OK ;

	    u_setsid() ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: processing\n",pip->progname) ;

	    rs = procthem(pip,flp) ;

	    vecobj_finishfiles(flp) ;

	    vecobj_finish(flp) ;

	    if (pip->debuglevel > 0)
	        bprintf(pip->efp,"%s: exiting files=%u (%d)\n",
	            pip->progname,pip->c_files,rs) ;

	    bclose(pip->efp) ;

	    if (rs < 0)
	        ex = EX_DATAERR ;

	    uc_exit(ex) ;

	} /* end if (child) */

	return rs ;
}
/* end subroutine (procoff) */


static int procthem(PROGINFO *pip,vecobj *flp)
{
	struct entry	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		delay ;
	int		c = 0 ;

#if	CF_SORTENTRIES /* the file scanning algorithm doesn't use this */
	rs = vecobj_sort(flp,cmpentry) ;
	if (rs < 0)
	    goto ret0 ;
#endif /* CF_SORTENTRIES */

	while ((c = vecobj_count(flp)) > 0) {

	    delay = findmin(pip,flp) ;

	    if (delay > 0) {
	        if (delay > 5) delay = 5 ;
	        uc_safesleep(delay) ;
	    }

	    pip->daytime = time(NULL) ;

	    for (i = 0 ; vecobj_get(flp,i,&ep) >= 0 ; i += 1) {
	        if (ep != NULL) {
	            if ((rs1 = filecheck(pip,ep->fname,ep->ti_expire)) >= 0) {
	                c += 1 ;
	                vecobj_del(flp,i) ;
	            }
		}
	    } /* end for */

	} /* end while */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procthem) */


static int filecheck(PROGINFO *pip,cchar *fname,time_t ti_e)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs1 = u_stat(fname,&sb)) >= 0) {
	    if (pip->daytime >= ti_e) {
		f = TRUE ;
	        rs1 = u_unlink(fname) ;
	    }
	} else
	    f = TRUE ;

	if (f && (pip->debuglevel > 0)) {
	    bprintf(pip->efp,"%s: D file=%s (%d)\n",
	        pip->progname, fname,rs1) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (filecheck) */


#ifdef	COMMENT

static int filedel(pip,fname)
PROGINFO	*pip ;
const char	fname[] ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = u_unlink(fname) ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: D file=%s (%d)\n",
	        pip->progname, fname,rs1) ;
	}

	return rs ;
}
/* end subroutine (filedel) */

#endif /* COMMENT */


static int findmin(pip,flp)
PROGINFO	*pip ;
vecobj		*flp ;
{
	struct entry	*ep ;
	int		rs = SR_OK ;
	int		i ;
	int		min = INT_MAX ;

	for (i = 0 ; vecobj_get(flp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        int	v = MAX((ep->ti_expire - pip->daytime),0) ;
	        if (v < min) min = v ;
	        if (min <= 0) {
	            min = 0 ;
	            break ;
	        }
	    }
	} /* end for */

	return (rs >= 0) ? min : rs ;
}
/* end subroutine (findmin) */


static int vecobj_finishfiles(vecobj *flp)
{
	struct entry	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(flp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = entry_finish(ep) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (vecobj_finishfiles) */


static int entry_start(struct entry *ep,cchar *fname,time_t ti_e)
{
	int		rs ;
	const char	*cp ;

	if (ep == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	ep->ti_expire = ti_e ;
	ep->fname = NULL ;
	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    ep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(struct entry *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->fname != NULL) {
	    rs1 = uc_free(ep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


#if	CF_SORTENTRIES

static int cmpentry(f1pp,f2pp)
struct entry	**f1pp, **f2pp ;
{

	if ((f1pp == NULL) && (f2pp == NULL))
	    return 0 ;

	if (f1pp == NULL)
	    return 1 ;

	if (f2pp == NULL)
	    return -1 ;

	return ((*f1pp)->ti_expire - (*f2pp)->ti_expire) ;
}
/* end subroutine (cmpentry) */

#endif /* CF_SORTENTRIES */


