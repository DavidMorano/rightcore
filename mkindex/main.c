/* main */

/* main subroutine for the MKINDEX program */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */


/* revision history:

	= 1994-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1994 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is the 'main' module for the 'mkkey' program. This module processes
        the program invocation arguments and performs some preprocessing steps
        before any actual input files are scanned.

        The real work of processing the input files (one at a time) is performed
        by the 'process()' subroutine.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<varsub.h>
#include	<hdb.h>
#include	<field.h>
#include	<ascii.h>
#include	<localmisc.h>
#include	<exitcodes.h>

#include	"config.h"
#include	"defs.h"
#include	"memfile.h"


/* local defines */

#ifndef	CMDBUFLEN
#define	CMDBUFLEN	(2 * 1024)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	HASHINFO	struct hashinfo


/* external subroutines */

extern uint	nextpowtwo(uint) ;

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecmfi(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	varsub_addva(VARSUB *,const char **) ;
extern int	varsub_addvec(VARSUB *,VECSTR *) ;
extern int	varsub_subbuf(), varsub_merge() ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	progfile(struct proginfo *,uint *,uint,
			MEMFILE *,bfile *,const char *) ;
extern int	postwrite(struct proginfo *, uint *,int,
			MEMFILE *, const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strshrink(char *) ;
extern char	*timestr_log(time_t,char *) ;


/* externals variables */

extern char	makedate[] ;


/* local structures */

struct hashinfo {
	struct proginfo	*pip ;
	uint		*hasha ;
	const char	*postfname ;
	MEMFILE		mf ;
	bfile		namefile ;
	int		hashn ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;

static int	procargs(struct proginfo *,struct arginfo *,BITS *,
			HASHINFO *,const char *,const char *) ;

static int	hashinfo_begin(HASHINFO *,struct proginfo *,int,const char *) ;
static int	hashinfo_end(HASHINFO *) ;
static int	hashinfo_post(HASHINFO *,const char *) ;

#ifdef	COMMENT
static int	mkfieldterms(uchar *) ;
#endif


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"CONFIG",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"LOGFILE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"idx",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_config,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_logfile,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_idx,
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
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	struct proginfo	pi, *pip = &pi ;
	struct arginfo	ainfo ;
	BITS		pargs ;
	USERINFO	u ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint	mo_start = 0 ;
#endif

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	rs ;
	int	rs1 ;
	int	v ;
	int	hashn = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_makedate = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_append = FALSE ;
	int	f_noinput = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	userbuf[USERINFO_LEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*lfname = NULL ;
	const char	*idxname = NULL ;
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

/* initialize */

	pip->pagesize = getpagesize() ;

	pip->tmpdname = getenv(VARTMPDNAME) ;
	pip->verboselevel = 1 ;

	pip->minwordlen = -2 ;
	pip->maxwordlen = -2 ;
	pip->eigenwords = -2 ;
	pip->keys = -2 ;

/* start parsing the arguments */

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
	        const int ch = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ch)) {

	            argval = (argp+1) ;

	        } else if (ch == '-') {

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

/* keyword match or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program root */
	                case argopt_root:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pr = argp ;
	                    }
	                    break ;

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

/* version */
	                case argopt_version:
	                    f_makedate = f_version ;
	                    f_version = TRUE ;
	                    break ;

/* verbose mode */
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

/* log file */
	                case argopt_logfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            lfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            lfname = argp ;
	                    }
	                    break ;

/* print out the help */
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

/* output file */
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

/* index-name */
	                case argopt_idx:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            idxname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            idxname = argp ;
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    ex = EX_USAGE ;
	                    break ;

	                } /* end switch */

	            } else {

	                while (akl--) {
			    const int	kc = MKCHAR(*akp) ;

	                    switch (kc) {

/* debug */
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

/* version */
	                    case 'V':
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

/* append to the key file */
	                    case 'a':
	                        f_append = TRUE ;
	                        break ;

/* file names to process is in this named file */
	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        afname = argp ;
	                        break ;

/* maximum number of keys written out */
	                    case 'k':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecmfi(argp,argl,&v) ;
	                            pip->keys = v ;
	                        }
	                        break ;

	                    case 'l':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = optvalue(argp,argl) ;
	                            pip->minwordlen = rs ;
	                        }
	                        break ;

	                    case 'm':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdeci(argp,argl,&v) ;
	                            pip->maxwordlen = v ;
	                        }
	                        break ;

	                    case 'n':
	                        f_noinput = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* remove labels */
	                    case 's':
	                        pip->f.removelabel = TRUE ;
	                        break ;

/* index whole files */
	                    case 'w':
	                        pip->f.wholefile = TRUE ;
	                        break ;

/* verbose mode */
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
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
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
	    debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	    if (f_makedate) {
	        if ((cp = strchr(makedate,CH_RPAREN)) != NULL) {
	            cp += 1 ;
	            while (*cp && isspace(*cp))
	                cp += 1 ;
	        } else
	            cp = makedate ;
	        bprintf(pip->efp,"%s: makedate %s\n",
	            pip->progname,cp) ;
	    }
	}

/* get the program root */

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

	if (f_help) {
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
	}

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* continue */

	pip->f.noinput = f_noinput ;
	pip->f.append = f_append ;

	memset(&ainfo,0,sizeof(struct arginfo)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

/* get some host/user information */

	rs = userinfo(&u,userbuf,USERINFO_LEN,NULL) ;

	if (rs < 0) {
	    ex = EX_NOUSER ;
	    if ((pip->debuglevel > 0) && (! pip->f.quiet))
	        bprintf(pip->efp,"%s: could not get user information\n",
	            pip->progname) ;
	    goto retearly ;
	}

	pip->nodename = u.nodename ;
	pip->domainname = u.domainname ;
	pip->username = u.username ;

/* find a configuration file if we have one */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf( "main: checking for configuration file\n") ;
#endif

/* search locally */

/* check program parameters */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking program parameters\n") ;
#endif

	if ((pip->tmpdname == NULL) || (pip->tmpdname[0] == '\0'))
	    pip->tmpdname = TMPDNAME ;

/* do we have an activity log file? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 0 lfname=%s\n",lfname) ;
#endif

	if ((lfname == NULL) || (lfname[0] == '\0'))
	    lfname = LOGFNAME ;

	if ((lfname[0] == '/') || (u_access(lfname,W_OK) >= 0))
	    rs = logfile_open(&pip->lh,lfname,0,0666,u.logid) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: 1 lfname=%s rs=%d\n",lfname,rs) ;
#endif

	if ((rs < 0) && (lfname[0] != '/')) {

	    mkpath2(tmpfname, pip->pr,lfname) ;

	    rs = logfile_open(&pip->lh,tmpfname,0,0666,u.logid) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: 2 lfname=%s rs=%d\n",tmpfname,rs) ;
#endif

	} /* end if (we tried to open another log file) */

/* set some defaults */

	if (idxname == NULL) idxname = getenv(VARIDXNAME) ;

	if ((idxname == NULL) || (idxname[0] == '\0'))
	    idxname = INDEXNAME ;

	if ((argval != NULL) && (hashn == 0)) {
	    rs = cfdecmfi(argval,-1,&hashn) ;
	}
	if (hashn <= NHASH) hashn = NHASH ;
	if (rs < 0) goto done ;

/* open or create all of the stuff needed */

	{
	    HASHINFO	hi ;
	    int		c = 0 ;

	    if ((rs = hashinfo_begin(&hi,pip,hashn,idxname)) >= 0) {

	        if ((rs = procargs(pip,&ainfo,&pargs,&hi,ofname,afname)) >= 0) {
	            c = rs ;

	            rs = hashinfo_post(&hi,idxname) ;

	            if (pip->debuglevel > 0) {
	                if (rs >= 0) {
	                    bprintf(pip->efp,"%s: %u files processed\n",
	                        pip->progname,c) ;
	                } else {
	                    bprintf(pip->efp,"%s: failed (%d)\n",
	                        pip->progname,rs) ;
	                }
	            }

	        } /* end if (procargs) */

	        rs1 = hashinfo_end(&hi) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (hashinfo) */
	} /* end block */

/* we are done */
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    ex = mapex(mapexs,rs) ;
	} /* end if */

	logfile_close(&pip->lh) ;

badlogopen:
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

ret1:
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

/* bad argument usage */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
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

	fmt = "%s: USAGE> %s [-idx <name>] [<file(s)>] [-af <argfile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-a]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(pip,aip,app,hip,ofname,afname)
struct proginfo	*pip ;
struct arginfo	*aip ;
BITS		*app ;
HASHINFO	*hip ;
const char	*afname ;
const char	*ofname ;
{
	MEMFILE	*mfp = &hip->mf ;

	bfile	ofile, *ofp = &ofile ;
	bfile	*nfp = &hip->namefile ;

	uint	*hasha = hip->hasha ;

	int	rs = SR_OK ;
	int	hashn = hip->hashn ;
	int	pan = 0 ;
	int	f_append = pip->f.append ;
	int	f_noinput = pip->f.noinput ;

	const char	*cp ;

	char	ofbuf[10] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procargs: hashn=%u\n",hashn) ;
#endif

	strcpy(ofbuf,"wc") ;
	if (f_append) {
	    strcat(ofbuf,"a") ;
	} else
	    strcat(ofbuf,"t") ;

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofname,ofbuf,0666)) >= 0) {

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(app,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (! f) continue ;

	            cp = aip->argv[ai] ;
	            pan += 1 ;
	            rs = progfile(pip,hasha,hashn,mfp,nfp,cp) ;

	            if (rs < 0) break ;
	        } /* end for */
	    } /* end if (ok) */

/* process any files in the argument filename list file */

	    if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (afname[0] == '-') afname = BFILE_STDIN ;

	        if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                cp = strshrink(lbuf) ;

	                if ((cp[0] == '\0') || (cp[0] == '#'))
	                    continue ;

	                pan += 1 ;
	                rs = progfile(pip,hasha,hashn,mfp,nfp,cp) ;

	                if (rs < 0) {
	                    if (*cp == '-')
	                        cp = "*stdinput*" ;
	                    bprintf(pip->efp,
	                        "%s: error processing input file (%d)\n",
	                        pip->progname,rs) ;
	                    bprintf(pip->efp,"%s: errored file=%s\n",
	                        pip->progname,cp) ;
	                } /* end if */

	            } /* end while (reading lines) */

	            bclose(afp) ;
	        } else {
	            if (! pip->f.quiet) {
	                bprintf(pip->efp,
	                    "%s: inaccessible argument list file (%d)\n",
	                    pip->progname,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",
	                    pip->progname,afname) ;
	            }
	        } /* end if */

	    } /* end if (afname) */

/* process standard input */

	    if ((rs >= 0) && (pan == 0) && (! f_noinput)) {

	        pan += 1 ;
	        cp = "-" ;
	        rs = progfile(pip,hasha,hashn,mfp,nfp,cp) ;

	        if (rs < 0) {
	            cp = "*stdinput*" ;
	            bprintf(pip->efp,"%s: error processing input file (%d)\n",
	                pip->progname,rs) ;
	            bprintf(pip->efp,"%s: errored file=%s\n",
	                pip->progname,cp) ;
	        }

	    } /* end if (standard input) */

	    bclose(ofp) ;
	} /* end if (output-file) */

	return (rs >= 0) ? pan : rs ;
}
/* end subroutine (procargs) */


static int hashinfo_begin(hip,pip,hashn,idxname)
HASHINFO	*hip ;
struct proginfo	*pip ;
int		hashn ;
const char	*idxname ;
{
	int	rs = SR_OK ;
	int	size ;

	void	*hasha ;


	memset(hip,0,sizeof(HASHINFO)) ;

	hip->pip = pip ;
	hashn = nextpowtwo(hashn) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/hashinfo_begin: hashn=%u\n",hashn) ;
#endif

	size = (hashn * sizeof(uint)) ;
	if ((rs = uc_malloc(size,&hasha)) >= 0) {
	    const char	*tf = "/tmp/whenXXXXXXXXXX" ;
	    const char	*cp ;
	    char	postfname[MAXPATHLEN+1] ;
	    memset(hasha,0,size) ;
	    hip->hasha = hasha ;
	    hip->hashn = hashn ;
	    if ((rs = mktmpfile(postfname,0660,tf)) >= 0) {
	        const int	of = (O_RDWR | O_CREAT) ;
	        if ((rs = uc_mallocstrw(postfname,rs,&cp)) >= 0) {
	            hip->postfname = cp ;
	            if ((rs = memfile_open(&hip->mf,postfname,of,0660)) >= 0) {
	                char	tmpfname[MAXPATHLEN+1] ;
	                mkfnamesuf1(tmpfname,idxname,"nnames") ;
	                rs = bopen(&hip->namefile,tmpfname,"wct",0666) ;
	                if (rs < 0)
	                    memfile_close(&hip->mf) ;
	            }
	            if (rs < 0) {
	                uc_free(hip->postfname) ;
	                hip->postfname = NULL ;
	            }
	        }
	        if (rs < 0)
	            uc_unlink(postfname) ;
	    }
	    if (rs < 0) {
	        uc_free(hip->hasha) ;
	        hip->hasha = NULL ;
	    }
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/hashinfo_begin: hashn=%u\n",hip->hashn) ;
	    debugprintf("main/hashinfo_begin: ret rs=%d\n",rs) ;
	}
#endif

	return rs ;
}
/* end subroutine (hashinfo_begin) */


static int hashinfo_end(HASHINFO *hip)
{
	int	rs = SR_OK ;
	int	rs1 ;

	rs1 = bclose(&hip->namefile) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = memfile_close(&hip->mf) ;
	if (rs >= 0) rs = rs1 ;

	if (hip->postfname[0] != '\0') {
	    rs1 = uc_unlink(hip->postfname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (hip->postfname != NULL) {
	    rs1 = uc_free(hip->postfname) ;
	    if (rs >= 0) rs = rs1 ;
	    hip->postfname = NULL ;
	}

	if (hip->hasha != NULL) {
	    rs1 = uc_free(hip->hasha) ;
	    if (rs >= 0) rs = rs1 ;
	    hip->hasha = NULL ;
	}

	return rs ;
}
/* end subroutine (hashinfo_end) */


static int hashinfo_post(HASHINFO *hip,const char *idxname)
{
	struct proginfo	*pip = hip->pip ;
	MEMFILE	*mfp = &hip->mf ;
	uint	*hasha = hip->hasha ;
	int	rs = SR_OK ;
	int		hashn = hip->hashn ;
	char	tmpfname[MAXPATHLEN+1] ;
	char	namefname[MAXPATHLEN+1] ;

	mkfnamesuf1(tmpfname,idxname,"nnames") ;
	mkfnamesuf1(namefname,idxname,"names") ;
	if ((rs = postwrite(pip,hasha,hashn,mfp,idxname)) >= 0) {
	    rs = u_rename(tmpfname,namefname) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: u_rename() rs=%d\n",rs) ;
#endif

	}

	return rs ;
}
/* end subroutine (hashinfo_post) */


#ifdef	COMMENT
static int mkfieldterms(uchar *terms)
{
	int	ch ;
	int	i ;
	int	c = 256 ;

	for (i = 0 ; i < 32 ; i += 1)
	    terms[i] = 0xFF ;

	BACLR(terms,'_') ;
	c -= 1 ;

	for (ch = 'a' ; ch <= 'z' ; ch += 1) {
	    BACLR(terms,ch) ;
	    c -= 1 ;
	}

	for (ch = 'A' ; ch <= 'Z' ; ch += 1) {
	    BACLR(terms,ch) ;
	    c -= 1 ;
	}

	for (ch = '0' ; ch <= '9' ; ch += 1) {
	    BACLR(terms,ch) ;
	    c -= 1 ;
	}

	return c ;
}
/* end subroutine (mkfieldterms) */
#endif /* COMMENT */


