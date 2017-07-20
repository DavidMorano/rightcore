/* main (HOSTADDRINFO) */

/* program to get a canonical host name */


#define	CF_DEBUGS	0		/* compile-time debug switch */
#define	CF_DEBUG	0		/* run-time debug switch */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_LOCSETET	0		/* |locinfo_setentry()| */


/* revision history:

	- 2005-05-23, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ hostaddrinfo [<name(s)> ...] [-v[=n]] [-V]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<keyopt.h>
#include	<bits.h>
#include	<bfile.h>
#include	<nulstr.h>
#include	<inetaddr.h>
#include	<netorder.h>
#include	<hostinfo.h>
#include	<cthex.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	USERBUFLEN
#define	USERBUFLEN	(NODENAMELEN + (2 * 1024))
#endif

#ifndef	DIGBUFLEN
#define	DIGBUFLEN	40		/* can hold int128_t in decimal */
#endif

#ifndef	INET4_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	16
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46	/* Solaris® says this is 46! */
#endif

#define	PREPNAME	struct prepname
#define	PREPNAME_MAGIC	0x17161524

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sfskipwhite(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	optvalue(const char *,int) ;
extern int	getnodename(char *,int) ;
extern int	getaf(const char *,int) ;
extern int	inetpton(void *,int,int,cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	hasINET4AddrStr(cchar *,int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const PIVARS *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */

extern char	hostaddrinfo_makedate[] ;


/* local structures */

struct locinfo_flags {
	uint		stores:1 ;
	uint		ao:1 ;
	uint		skipaddr:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	open ;
	vecstr		stores ;
	PROGINFO	*pip ;
	int		afamily ;
	uint		sum ;
} ;

struct prepname {
	ulong		magic ;
	const char	*hostname ;
	int		hostnamelen ;
	int		f_alloc ;
} ;


/* forward subroutines */

static int	usage(PROGINFO *) ;

static int	procargs(PROGINFO *,ARGINFO *,BITS *,cchar *,cchar *) ;

static int	procname(PROGINFO *,bfile *,const char *) ;
static int	procspecial(PROGINFO *,bfile *,cchar *,int) ;
static int	procinfo(PROGINFO *,bfile *,char *,cchar *,int) ;
static int	procprint4(PROGINFO *,bfile *,cchar *,cuchar *,int) ;
static int	procprint6(PROGINFO *,bfile *,cchar *,cuchar *,int) ;

static int	prepname_start(PREPNAME *,cchar *,int) ;
static int	prepname_finish(PREPNAME *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCSETENT
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif

static int	morder_ruint(const void *,uint *) ;


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"to",
	"cname",
	"rs",
	"ao",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_to,
	argopt_cname,
	argopt_rs,
	argopt_ao,
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


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	ARGINFO		ainfo ;
	BITS		pargs ;
	KEYOPT		akopts ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		rs, rs1 ;
	int		v ;
	int		ex = EX_INFO ;
	int		f_optplus, f_optminus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*afspec = NULL ;
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

/* miscellaneous early stuff */

	pip->verboselevel = 1 ;
	pip->af = AF_UNSPEC ;

	pip->lip = &li ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* parse arguments */

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
	    if ((argl > 1) && (f_optplus || f_optminus)) {
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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
	                    break ;

/* verbose mode */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* program search name */
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

/* argument list file */
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

/* error file */
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

/* lookup-timeout */
	                case argopt_to:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdecti(argp,argl,&v) ;
	                            pip->to_lookup = v ;
	                        }
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* want cannonical-name */
	                case argopt_cname:
	                    pip->f.cname = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.cname = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* want return-status */
	                case argopt_rs:
	                    pip->f.rs = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            pip->f.rs = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* address-only */
	                case argopt_ao:
	                    lip->f.ao = TRUE ;
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = optbool(avp,avl) ;
	                            lip->f.ao = (rs > 0) ;
	                        }
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'a':
	                    case 'f':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                afspec = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                rs = keyopt_loads(&akopts,argp,argl) ;
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

/* verbose */
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

/* initialize some other common stuff */

	if (rs >= 0) {
	    if ((rs = proginfo_setpiv(pip,pr,&initvars)) >= 0) {
	        rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;
	    }
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname, pip->searchname) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_USAGE ;

/* check arguments */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if ((afspec != NULL) && (afspec[0] != '\0')) {
	    rs = getaf(afspec,-1) ;
	    pip->af = rs ;
	}

/* user-information */

	if (rs >= 0) {
	    char	nbuf[NODENAMELEN+1] ;
	    if ((rs = getnodename(nbuf,NODENAMELEN)) >= 0) {
	        const char	**vpp = &pip->nodename ;
	        rs = proginfo_setentry(pip,vpp,nbuf,rs) ;
	    }
	}

	if (afname == NULL) afname = getenv(VARAFNAME) ;

/* get a TMPDIR */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	ex = EX_OK ;

/* go */

	memset(&ainfo,0,sizeof(ARGINFO)) ;
	ainfo.argc = argc ;
	ainfo.ai = ai ;
	ainfo.argv = argv ;
	ainfo.ai_max = ai_max ;
	ainfo.ai_pos = ai_pos ;

	if (rs >= 0) {
	    const char	*ofn = ofname ;
	    const char	*afn = afname ;
	    rs = procargs(pip,&ainfo,&pargs,ofn,afn) ;
	}

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

/* finish up */
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
	locinfo_finish(lip) ;

badlocstart:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	{
	    uint	mo ;
	    uc_mallout(&mo) ;
	    debugprintf("main: final mallout=%u\n",(mo-mo_start)) ;
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
	bprintf(pip->efp,"%s: invalid argument(s) specified (%d)\n",
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

	fmt = "%s: USAGE> %s [<name(s)> ...] [-f <af>]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	if (rs >= 0) rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procargs(PROGINFO *pip,ARGINFO *aip,BITS *bop,cchar *ofn,cchar *afn)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	if ((ofn == NULL) || (ofn[0] == '\0') || (ofn[0] == '-'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"wct",0666)) >= 0) {
	    int		pan = 0 ;
	    int		cl ;
	    const char	*cp ;

	    if (rs >= 0) {
	        int	ai ;
	        int	f ;
	        for (ai = 1 ; ai < aip->argc ; ai += 1) {

	            f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	            f = f || ((ai > aip->ai_pos) && (aip->argv[ai] != NULL)) ;
	            if (f) {
	                cp = aip->argv[ai] ;
	                pan += 1 ;
	                rs = procname(pip,ofp,cp) ;
	                wlen += rs ;
	            }

	            if (rs < 0) break ;
	        } /* end for (processing positional arguments) */
	    } /* end if (ok) */

	    if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	        bfile	afile, *afp = &afile ;

	        if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	        if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = breadline(afp,lbuf,llen)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    lbuf[(cp-lbuf)+cl] = '\0' ;
	                    if (cp[0] != '#') {
	                        pan += 1 ;
	                        rs = procname(pip,ofp,cp) ;
	                        wlen += rs ;
	                    }
	                }

	                if (rs < 0) {
			    fmt = "%s: error (%d) name=%s\n" ;
	                    bprintf(pip->efp,fmt,pn,rs,cp) ;
	                    break ;
	                }
	            } /* end while (reading lines) */

	            rs1 = bclose(afp) ;
		    if (rs >= 0) rs = rs1 ;
	        } else {
	            if (! pip->f.quiet) {
	                fmt = "%s: inaccessible argument list (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	                bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	            }
	        } /* end if */

	    } /* end if (processing file argument file list) */

	    if ((rs >= 0) && (pan == 0)) {

	        cp = "-" ;
	        pan += 1 ;
	        rs = procname(pip,ofp,cp) ;
	        wlen += rs ;

	    } /* end if (processing requests) */

	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} else {
	    fmt = "%s: inaccessible output (%d)\n" ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    bprintf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procargs) */


static int procname(PROGINFO *pip,bfile *ofp,cchar *np)
{
	NULSTR		n ;
	int		rs ;
	int		rs1 ;
	int		nl = -1 ;
	cchar		*name = NULL ;
	char		chostname[MAXHOSTNAMELEN + 1] ;

	if (np == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procname: name=%s\n",np) ;
#endif

	chostname[0] = '\0' ;
	if ((np[0] == '\0') || (np[0] == '-')) {
	    np = pip->nodename ;
	    nl = 1 ;
	}

	if ((rs = nulstr_start(&n,np,nl,&name)) >= 0) {
	    nl = rs ;
	    while (name[nl-1] == '.') nl -= 1 ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: query=%t\n",
	        pip->progname,name,nl) ;
	}

	if ((rs = procspecial(pip,ofp,name,nl)) == 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procname: procinfo=%t\n",name,nl) ;
#endif
	    rs = procinfo(pip,ofp,chostname,name,nl) ;
	}

	if (rs == SR_NOTFOUND) {

	    if (! pip->f.quiet) {
	        bprintf(pip->efp,"%s: not found query=%s (%d)\n",
	            pip->progname,name,rs) ;
	    }

	    rs = bprintf(ofp,"\n") ;

	} /* end if */

	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("procname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procname) */


static int procspecial(PROGINFO *pip,bfile *ofp,cchar *np,int nl)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		f = TRUE ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procspecial: af=%d\n",pip->af) ;
#endif
	f = f && lip->f.ao ;
	f = f && ((pip->af == AF_UNSPEC) || (pip->af == AF_INET4)) ;
	if (f) {
	f = hasINET4AddrStr(np,nl) ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procspecial: hasINET4AddrStr=%u\n",f) ;
#endif
	}
	if (f) {
	    bprintline(ofp,np,nl) ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end if (procspecial) */


static int procinfo(PROGINFO *pip,bfile *ofp,char *cname,cchar *np,int nl)
{
	LOCINFO		*lip = pip->lip ;
	PREPNAME	pn ;
	int		rs ;
	int		rs1 ;

	if (pip->verboselevel >= 2) {
	    bprintf(ofp,"cname=%s\n",cname) ;
	}

	if ((rs = prepname_start(&pn,np,nl)) >= 0) {
	    HOSTINFO		hi ;
	    HOSTINFO_CUR	cur ;
	    const int		f_all = (! lip->f.ao) ;
	    const char		*hn = pn.hostname ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("procinfo: hostname=%s\n",pn.hostname) ;
#endif

	    if (f_all) {
	        bprintf(ofp,"%s:\n",pn.hostname) ;
	    }

	    if ((rs1 = hostinfo_start(&hi,pip->af,hn)) >= 0) {
	        int		nl ;
	        int		al ;
	        const uchar	*ap ;
	        const char	*indent = "    " ;
	        const char	*np ;

/* effective */

		if (f_all) {
	        rs1 = hostinfo_geteffective(&hi,&np) ;
	        if (rs1 < 0) np = "" ;
	        bprintf(ofp,"%sename=%s\n",indent,np) ;
		}

/* canonical */

		if (f_all) {
	        rs1 = hostinfo_getcanonical(&hi,&np) ;
	        if (rs1 < 0) np = "" ;
	        bprintf(ofp,"%scname=%s\n",indent,np) ;
		}

/* names */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procinfo: names\n") ;
#endif

		if (f_all) {
	            if ((rs = hostinfo_curbegin(&hi,&cur)) >= 0) {

	                while ((nl = hostinfo_enumname(&hi,&cur,&np)) >= 0) {
	                    if (nl > 0) {
	                        rs = bprintf(ofp,"%salias=%s\n",indent,np) ;
			    }
	                    if (rs < 0) break ;
	                } /* end while */

	                hostinfo_curend(&hi,&cur) ;
	            } /* end if (cursor) */
		} /* end if (print all) */

/* addresses */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("procinfo: addrs\n") ;
#endif

	        if (rs >= 0) {

	            if ((rs = hostinfo_curbegin(&hi,&cur)) >= 0) {

	                while ((al = hostinfo_enumaddr(&hi,&cur,&ap)) >= 0) {

	                    if (pip->verboselevel >= 2) {
	                        bprintf(ofp,"%saddrlen=%u\n",indent,al) ;
			    }

	                    if (al == INET4ADDRLEN) {
	                        rs = procprint4(pip,ofp,indent,ap,al) ;
	                    } else if (al == INET6ADDRLEN) {
	                        rs = procprint6(pip,ofp,indent,ap,al) ;
	                    } else if (f_all) {
	                        rs = bprintf(ofp,"%saddr=(not known)\n",
				indent) ;
	                    }

	                    if (rs < 0) break ;
	                } /* end while */

	                hostinfo_curend(&hi,&cur) ;
	            } /* end if (cursor) */

	        } /* end if */

	        rs1 = hostinfo_finish(&hi) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (hostinfo) */

	    rs1 = prepname_finish(&pn) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (progname) */

	return rs ;
}
/* end subroutine (procinfo) */


static int procprint4(pip,ofp,indent,ap,al)
PROGINFO	*pip ;
bfile		*ofp ;
const char	*indent ;
const uchar	*ap ;
int		al ;
{
	LOCINFO		*lip = pip->lip ;
	INETADDR	ia ;
	int		rs ;
	int		f_all ;
	char		astr4[INET4_ADDRSTRLEN + 1] ;

	if ((al >= 0) && (al < INET4ADDRLEN)) return SR_DOM ;

	f_all = (! lip->f.ao) ;
	if ((rs = inetaddr_start(&ia,ap)) >= 0) {
	    const int	aslen = INET4_ADDRSTRLEN ;

	    if ((rs = inetaddr_getdotaddr(&ia,astr4,aslen)) >= 0) {
	        uint	v ;
	        morder_ruint(ap,&v) ;
		if (f_all) {
	            rs = bprintf(ofp,"%saddr=%s (\\x%08X)\n",indent,astr4,v) ;
		} else {
	            rs = bprintline(ofp,astr4,-1) ;
		}
	    } else if (f_all) {
	        rs = bprintf(ofp,"%saddr=*\n",indent) ;
	    }

	    inetaddr_finish(&ia) ;
	} else if (f_all) {
	    rs = bprintf(ofp,"%saddr=*\n",indent) ;
	}

	return rs ;
}
/* end subroutine (procprint4) */


static int procprint6(pip,ofp,indent,ap,al)
PROGINFO	*pip ;
bfile		*ofp ;
const char	*indent ;
const uchar	*ap ;
int		al ;
{
	LOCINFO		*lip = pip->lip ;
	const int	diglen = DIGBUFLEN ;
	int		rs = SR_OK ;
	int		i ;
	int		pl ;
	int		f_all ;
	char		astr6[INET6_ADDRSTRLEN + 1] ;
	char		digbuf[DIGBUFLEN + 1] ;

	if ((al >= 0) && (al < INET6ADDRLEN)) return SR_DOM ;

	pl = 0 ;
	f_all = (! lip->f.ao) ;
	for (i = 0 ; i < INET6ADDRLEN ; i += 1) {
	    cthexi(digbuf,diglen,(ap[i] & 0xff)) ;
	    if ((i > 0) && ((i & 1) == 0)) astr6[pl++] = ':' ;
	    astr6[pl++] = digbuf[6] ;
	    astr6[pl++] = digbuf[7] ;
	} /* end for */
	astr6[pl] = '\0' ;

	if (f_all) {
	    rs = bprintf(ofp,"%saddr=%s\n", indent,astr6) ;
	} else {
	    rs = bprintline(ofp,astr6,-1) ;
	}

	return rs ;
}
/* end subroutine (procprint6) */


static int prepname_start(PREPNAME *ep,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (ep == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

	memset(ep,0,sizeof(PREPNAME)) ;

	if (nl < 0) {
	    nl = strlen(np) ;
	} else {
	    f = TRUE ;
	}

	while ((nl > 0) && (np[nl-1] == '.')) {
	    f = TRUE ;
	    nl -= 1 ;
	}

	ep->hostname = np ;
	if (f) {
	    const char	*ccp ;
	    if ((rs = uc_mallocstrw(np,nl,&ccp)) >= 0) {
	        ep->f_alloc = TRUE ;
	        ep->hostname = ccp ;
	        ep->hostnamelen = nl ;
	    }
	}

	if (rs >= 0)
	    ep->magic = PREPNAME_MAGIC ;

#if	CF_DEBUGS
	debugprintf("prepname_start: ret rs=%d f=%u f_a=%u\n",
	    rs,f, ep->f_alloc) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end if (prepname_start) */


static int prepname_finish(PREPNAME *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("prepname_finish: entered\n") ;
#endif

	if (ep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("prepname_finish: magic?\n") ;
#endif

	if (ep->magic != PREPNAME_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("prepname_finish: f_a=%u\n", ep->f_alloc) ;
#endif

	if (ep->f_alloc && (ep->hostname != NULL)) {
	    rs1 = uc_free(ep->hostname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->hostname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("prepname_finish: ret\n") ;
#endif

	ep->f_alloc = FALSE ;
	ep->hostname = NULL ;
	ep->magic = 0 ;
	return rs ;
}
/* end if (prepname_finish) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs = SR_OK ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	return rs ;
}
/* end subroutine (locinfo_start) */


static int locinfo_finish(LOCINFO *lip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (lip == NULL) return SR_FAULT ;

	if (lip->open.stores) {
	    lip->open.stores = FALSE ;
	    rs1 = vecstr_finish(&lip->stores) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (locinfo_finish) */


#if	CF_LOCSETENT
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar vp[],int vl)
{
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_FAULT ;

	if (! lip->open.stores) {
	    rs = vecstr_start(&lip->stores,4,0) ;
	    lip->open.stores = (rs >= 0) ;
	}

	if (rs >= 0) {
	    int	oi = -1 ;

	    if (*epp != NULL) oi = vecstr_findaddr(&lip->stores,*epp) ;

	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(&lip->stores,vp,len,epp) ;
	    } else
	        *epp = NULL ;

	    if ((rs >= 0) && (oi >= 0))
	        vecstr_del(&lip->stores,oi) ;

	} /* end if */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCSETENT */


/* machine-order */
static int morder_ruint(const void *buf,uint *vp)
{
	int		rs ;
	void		*vbuf = (void *) buf ;

	rs = netorder_ruint(vbuf,vp) ;

	return rs ;
}
/* end if (morder_ruint) */


