/* main */

/* program to decrypt users' ONC private key and give to KEYSERV */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_DEBUGMALL	1		/* memory-allocation debugging */


/* revision history:

	= 1999-08-17, David A­D­ Morano
	This subroutine was partly taken from the LOGDIR-LOGNAME program (fist
	written for the SunOS 4.xx environment in 1989).

	= 2016-05-02, David A­D­ Morano
	I updated this program to use the default username as would be found by
	calling 'getusername(3dam)'.  This may not already be what is wanted,
	but it might be more of what is wanted more of the time.  We shall see
	how it works out.

*/

/* Copyright © 1998,2016 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ keyauth [<username>]


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<bits.h>
#include	<bfile.h>
#include	<getxusername.h>
#include	<netfile.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif


/* external subroutines */

extern int	sncpy1w(char *,int,cchar *,int) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	nextfield(cchar *,int,cchar **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	getuserhome(char *,int,cchar *) ;
extern int	getnodedomain(char *,char *) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

extern int	printhelp(void *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	progout_begin(PROGINFO *,cchar *) ;
extern int	progout_printf(PROGINFO *,cchar *,...) ;
extern int	progout_end(PROGINFO *) ;

extern int	getournetname(char *,int,cchar *) ;
extern int	authfile(char *,char *,cchar *) ;
extern int	onckeyalready(const char *) ;
extern int	onckeygetset(cchar *,const char *) ;
extern int	havenis() ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(cchar *) ;
extern int	debugprintf(cchar *,...) ;
extern int	debugclose() ;
extern int	debugprinthexblock(cchar *,int,const void *,int) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*timestr_log(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;
static int	procafname(PROGINFO *,char *,cchar *) ;
static int	procuserhome(PROGINFO *,cchar *) ;
static int	procinfonames(PROGINFO *) ;
static int	process(PROGINFO *,cchar *) ;
static int	prockeylogin(PROGINFO *,cchar *,cchar *) ;
static int	prockeylogin_auth(PROGINFO *,cchar *,cchar *) ;
static int	prockeylogin_netrc(PROGINFO *,cchar *,cchar *) ;
static int	procnetrc(PROGINFO *,cchar *,cchar *,cchar *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	"of",
	"nf",
	"netrc",
	"auf",
	"auth",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_of,
	argopt_nf,
	argopt_netrc,
	argopt_auf,
	argopt_auth,
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
	{ SR_NOPKG, EX_UNAVAILABLE },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;

static const char *netrcfiles[] = {
	".netrc",
	"etc/netrc",
	NULL
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	BITS		pargs ;
	bfile		errfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		ai_continue = 1 ;
	int		rs, rs1 ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_version = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*un = NULL ;
	const char	*cp ;
	char		unbuf[USERNAMELEN+1] = { 0 } ;

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

/* initialize */

	pip->verboselevel = 1 ;

	if (rs >= 0) rs = bits_start(&pargs,1) ;
	if (rs < 0) goto badpargs ;

/* start parsing the arguments */

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

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs = SR_INVALID ;
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

/* argument-list file */
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

/* output file-name */
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

/* NETRC file */
	                case argopt_nf:
	                case argopt_netrc:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->nfname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* authorization file */
	                case argopt_auf:
	                case argopt_auth:
	                    if (argr > 0) {
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pip->aufname = argp ;
	                    } else
	                        rs = SR_INVALID ;
	                    break ;

/* handle all keyword defaults */
	                default:
	                    f_usage = TRUE ;
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'n':
	                        pip->f.no = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'q':
	                        pip->verboselevel = 0 ;
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
	                        f_usage = TRUE ;
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

/* continue */

	if ((rs >= 0) && (pip->n == 0) && (argval != NULL)) {
	    rs = optvalue(argval,-1) ;
	    pip->n = rs ;
	}

	if (rs >= 0) {
	    for (ai = ai_continue ; ai < argc ; ai += 1) {
	        f = (ai <= ai_max) && (bits_test(&pargs,ai) > 0) ;
	        f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	        if (f) {
	            un = argv[ai] ;
	            ai_continue = (ai+1) ;
	            break ;
	        }
	    } /* end for */
	} /* end if (ok) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: mid1 rs=%d un=%s\n",rs,un) ;
#endif

	if (pip->aufname == NULL) pip->aufname = getourenv(envv,VARAUFNAME) ;
	if (pip->aufname == NULL) pip->aufname = getourenv(envv,VARAUTH) ;

	if (pip->nfname == NULL) pip->nfname = getourenv(envv,VARNETRC) ;

	if (afname == NULL) afname = getourenv(envv,VARAFNAME) ;

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) {
	    un = unbuf ;
	    rs = procafname(pip,unbuf,afname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: mid2 rs=%d un=%s\n",rs,un) ;
#endif

	if ((rs >= 0) && ((un == NULL) || (un[0] == '\0'))) {
	    rs = getusername(unbuf,USERNAMELEN,-1) ;
	    un = unbuf ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: mid2 rs=%d un=%s\n",rs,un) ;
#endif

	if (pip->debuglevel > 0) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: user=%s\n",pn,un) ;
	}

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: username=%s\n",un) ;
#endif

	if ((rs = procuserhome(pip,un)) >= 0) {
	    if ((rs = procinfonames(pip)) >= 0) {
	        if ((rs = progout_begin(pip,ofname)) >= 0) {
	            rs = process(pip,un) ;
	            if (pip->debuglevel > 0) {
	                cchar	*pn = pip->progname ;
	                cchar	*fmt = "%s: result (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,rs) ;
	            }
	            rs1 = progout_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (progout) */
	    } /* end if (procinfonames) */
	} /* end if (procuserhome) */

/* done */
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {
	    case SR_INVALID:
	        ex = EX_USAGE ;
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
	}

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u rs=%d\n",ex,rs) ;
#endif

	if (pip->efp != NULL) {
	    pip->open.errfile = FALSE ;
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	bits_finish(&pargs) ;

badpargs:
	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the ARGuments */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid arguments specified (%d)\n",
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

	fmt = "%s: USAGE> %s [<username>|-] [-n] [-q]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procafname(PROGINFO *pip,char *unbuf,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		ul = 0 ;

	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    bfile	afile, *afp = &afile ;
	    int		cl ;
	    cchar	*cp ;

	    if (strcmp(afn,"-") == 0) afn = BFILE_STDIN ;

	    if ((rs = bopen(afp,afn,"r",0666)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
	            int	len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

	            if ((len > 0) && (lbuf[0] != '#')) {
	                cchar	*tp ;

	                if ((tp = strnchr(lbuf,len,'#')) != NULL) {
	                    len = (tp-lbuf) ;
	                }

	                if ((cl = nextfield(lbuf,len,&cp)) > 0) {
	                    const int	unlen = USERNAMELEN ;
	                    rs = sncpy1w(unbuf,unlen,cp,cl) ;
	                    ul = rs ;
	                }

	            } /* end block */

	            if (ul > 0) break ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = bclose(afp) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: inaccessible argument-list (%d)\n" ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	        bprintf(pip->efp,"%s: afile=%s\n",pn,afn) ;
	    } /* end if */

	} /* end if (processing file argument file list) */

	return (rs >= 0) ? ul : rs ;
}
/* end subroutine (procafname) */


static int procuserhome(PROGINFO *pip,cchar *un)
{
	const int	hlen = MAXPATHLEN ;
	int		rs ;
	char		hbuf[MAXPATHLEN+1] ;
	if ((rs = getuserhome(hbuf,hlen,un)) >= 0) {
	    cchar	**vpp = &pip->homedname ;
	    rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	}
	return rs ;
}
/* end subroutine (procuserhome) */


static int procinfonames(PROGINFO *pip)
{
	int		rs ;
	char		nbuf[NODENAMELEN + 1] ;
	char		dbuf[MAXHOSTNAMELEN + 1] ;

	dbuf[0] = '\0' ;
	if ((rs = getnodedomain(nbuf,dbuf)) >= 0) {
	    cchar	**vpp = &pip->nodename ;
	    if ((rs = proginfo_setentry(pip,vpp,nbuf,-1)) >= 0) {
	        vpp = &pip->domainname ;
	        if ((rs = proginfo_setentry(pip,vpp,dbuf,-1)) >= 0) {
	            char	hbuf[MAXHOSTNAMELEN+1] ;
	            if (dbuf[0] != '\0') {
	                const int	hlen = MAXHOSTNAMELEN ;
	                if ((rs = snsds(hbuf,hlen,nbuf,dbuf)) >= 0) {
	                    vpp = &pip->hostname ;
	                    rs = proginfo_setentry(pip,vpp,hbuf,-1) ;
	                }
	            }
	        }
	    }
	}

	return rs ;
}
/* end subroutine (procinfonames) */


static int process(PROGINFO *pip,cchar *un)
{
	int		rs ;
	int		dl = pip->debuglevel ;
	int		vl = pip->verboselevel ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: ent un=%s\n",un) ;
#endif

	if ((rs = havenis()) > 0) {
	    const int	nlen = MAXNETNAMELEN ;
	    char	nbuf[MAXNETNAMELEN+1] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: havenis() rs=%d\n",rs) ;
#endif
	    if ((rs = getournetname(nbuf,nlen,un)) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: getournetname() rs=%d\n",rs) ;
#endif
	        if (vl >= 3) {
	            progout_printf(pip,"netname=%s\n",nbuf) ;
	        }
	        if (dl > 0) {
	            fmt = "%s: netname=%s\n" ;
	            bprintf(pip->efp,fmt,pn,nbuf) ;
	        }
	        if ((rs = onckeyalready(nbuf)) > 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: onckeyalready() rs=%d\n",rs) ;
#endif
	            rs = SR_OK ;
	            if ((vl >= 2) || pip->f.no) {
	                fmt = "already key-logged in!\n" ;
	                progout_printf(pip,fmt) ;
	            }
	            if (dl > 0) {
	                fmt = "%s: already key-logged in\n" ;
	                bprintf(pip->efp,fmt,pn) ;
	            }
	        } else if (rs == 0) {
	            if (dl > 0) {
	                fmt = "%s: not already key-logged in\n" ;
	                bprintf(pip->efp,fmt,pn) ;
	            }
	            if (pip->f.no) {
	                fmt = "not key-logged in!\n" ;
	                progout_printf(pip,fmt) ;
	            } else {
	                if ((rs = prockeylogin(pip,nbuf,un)) >= 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: prockeylogin() rs=%d\n",rs) ;
#endif
	                    if (vl >= 2) {
	                        fmt = "key-logged in!\n" ;
	                        progout_printf(pip,fmt) ;
	                    }
	                } else {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: prockeylogin() rs=%d\n",rs) ;
#endif
	                    if (dl > 0) {
	                        fmt = "%s: key-login failed (%d)\n" ;
	                        bprintf(pip->efp,fmt,pn,rs) ;
	                    }
	                }
	            }
	        } else if (rs == SR_NOPKG) {
	            fmt = "%s: key-server not found\n" ;
	            bprintf(pip->efp,fmt,pn) ;
	        } /* end if (onckeyalready) */
	    } else if (rs == 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: getournetname() rs=%d\n",rs) ;
#endif
	        if (vl >= 2) {
	            fmt = "no net-name available\n" ;
	            progout_printf(pip,fmt) ;
	        }
	        if (dl > 0) {
	            fmt = "%s: no net-name available\n" ;
	            bprintf(pip->efp,fmt,pn) ;
	        }
	    } else if (rs == SR_UNAVAIL) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: getournetname() rs=%d\n",rs) ;
#endif
	        if (vl >= 2) {
	            fmt = "NIS server is unavailable\n" ;
	            progout_printf(pip,fmt) ;
	        }
	        if (dl > 0) {
	            fmt = "%s: NIS server is unavailable\n" ;
	            bprintf(pip->efp,fmt,pn) ;
	        }
	    } /* end if (getournetname) */
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: getournetname-out rs=%d\n",rs) ;
#endif
	} else if (rs == 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/process: havenis() rs=%d\n",rs) ;
#endif
	    if (vl >= 2) {
	        fmt = "NIS is not initialized\n" ;
	        progout_printf(pip,fmt) ;
	    }
	    if (dl > 0) {
	        fmt = "%s: NIS is not initialized\n" ;
	        bprintf(pip->efp,fmt,pn) ;
	    }
	} /* end if (havenis) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


static int prockeylogin(PROGINFO *pip,cchar *netname,cchar *un)
{
	int		rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/prockeylogin: ent un=%s\n",un) ;
#endif

	if ((rs = prockeylogin_auth(pip,netname,un)) == 0) {
	    rs = prockeylogin_netrc(pip,netname,un) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/prockeylogin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end if (prockeylogin) */


static int prockeylogin_auth(PROGINFO *pip,cchar *netname,cchar *un)
{
	int		rs ;
	int		f = FALSE ;
	cchar		*aufn = pip->aufname ;
	char		abuf[MAXPATHLEN+1] ;
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/prockeylogin_auth: ent un=%s\n",un) ;
#endif
	if (pip->aufname == NULL) {
	    if ((rs = mkpath2(abuf,pip->homedname,AUTHFNAME)) >= 0) {
	        aufn = abuf ;
	    }
	}

	if (rs >= 0) {
	    char	ubuf[USERNAMELEN+1] ;
	    char	pbuf[PASSWORDLEN+1] ;
	    if ((rs = authfile(ubuf,pbuf,aufn)) >= 0) {
	        if ((strcmp(ubuf,un) == 0) && (pbuf[0] != '\0')) {
	            if ((rs = onckeygetset(netname,pbuf)) >= 0) {
	                f = TRUE ;
	            }
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    } /* end if */
	} /* end if (mkpath) */
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/prockeylogin_auth: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (prockeylogin_auth) */


static int prockeylogin_netrc(PROGINFO *pip,cchar *netname,cchar *un)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/prockeylogin_netrc: ent un=%s\n",un) ;
#endif

	if (pip->nfname != NULL) {
	    rs = procnetrc(pip,un,netname,pip->nfname) ;
	    f = rs ;
	} else {
	    int		i ;
	    cchar	*hdname = pip->homedname ;
	    char	nbuf[MAXPATHLEN+1] ;
	    for (i = 0 ; netrcfiles[i] != NULL ; i += 1) {
	        cchar	*netrc = netrcfiles[i] ;
	        if ((rs = mkpath2(nbuf,hdname,netrc)) >= 0) {
	            rs = procnetrc(pip,un,netname,nbuf) ;
	            f = rs ;
	        } /* end if (mkpath) */
	        if (f) break ;
	        if (rs < 0) break ;
	    } /* end for (NETRC files) */
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/prockeylogin_netrc: ret rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subrouine (prockeylogin_netrc) */


/* process a NETRC file */
static int procnetrc(PROGINFO *pip,cchar *un,cchar *netname,cchar *fname)
{
	NETFILE		nfile ;
	NETFILE_ENT	*nep ;
	int		rs ;
	int		rs1 ;
	int		f_keyed = FALSE ;

	if ((rs = netfile_open(&nfile,fname)) >= 0) {
	    int		j ;
	    cchar	*node = pip->nodename ;
	    cchar	*host = pip->hostname ;
	    cchar	*fmt ;

	    for (j = 0 ; netfile_get(&nfile,j,&nep) >= 0 ; j += 1) {
	        if (nep != NULL) {
	            cchar	*mn = nep->machine ;
	            cchar	*pn = nep->password ;

	            if ((mn != NULL) && (nep->login != NULL)) {
	                if ((strcmp(mn,node) == 0) || (strcmp(mn,host) == 0)) {
	                    if (strcmp(nep->login,un) == 0) {
	                        if ((pn != NULL) && (pn[0] != '\0')) {
	                            if ((rs = onckeygetset(netname,pn)) >= 0) {
	                                f_keyed = TRUE ;
	                            }
	                        }
	                    }
	                }
	            }

	            if (pip->debuglevel > 0) {
	                fmt = "%s: ONCgetset netname=%s passwd=%s (%d)\n" ;
	                bprintf(pip->efp,fmt,pn,netname,nep->password,rs) ;
	            }

	        } /* end if (non-null) */
	        if (f_keyed) break ;
	        if (rs < 0) break ;
	    } /* end for */

	    rs1 = netfile_close(&nfile) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	}

	return (rs >= 0) ? f_keyed : rs ;
}
/* end subroutine (procnetrc) */


