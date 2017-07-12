/* main (CEX) */

/* program to execute a program on a node in the local cluster */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_DEBUGMALL	1		/* debug memory-allocations */
#define	CF_DEFAULTSHELL	1		/* look for a default shell */
#define	CF_LOCINFOSET	0		/* |locinfo_setentry()| */


/* revision history:

	= 2005-11-21, David A­D­ Morano
	This program was started by copying from the RSLOW program.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ cex [-n] [-i <input>] <program arguments> < <input>


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stropts.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<char.h>
#include	<bfile.h>
#include	<vecstr.h>
#include	<userinfo.h>
#include	<ids.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"dialopts.h"
#include	"config.h"
#include	"defs.h"
#include	"dbi.h"
#include	"proglog.h"


/* local defines */

#define	NPARG		2
#define	NFDS		5
#define	USERBUFLEN	(NODENAMELEN + (2 * 1024))

#define	TO_DIAL		40

#ifndef	NODESFNAME
#define	NODESFNAME	"etc/cluster/nodes"
#endif

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	snsds(char *,int,cchar *,cchar *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	cfdeci(const char *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	authfile(const char *,char *,char *) ;
extern int	inetping(char *,int) ;
extern int	isdigitlatin(int) ;
extern int	isNotPresent(int) ;
extern int	isFailOpen(int) ;

extern int	proguserlist_begin(PROGINFO *) ;
extern int	proguserlist_end(PROGINFO *) ;

extern int	printhelp(bfile *,cchar *,cchar *,cchar *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	dialcprog(cchar *,cchar *,cchar *,
			cchar **,cchar **,int,int,int *) ;
extern int	transfer(PROGINFO *,cchar *,int,int,int,int,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */

extern char	cex_makedate[] ;


/* local structures */

struct errmsg {
	int		rs ;
	int		ex ;
	const char	*msgstr ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		x:1 ;
	uint		d:1 ;
	uint		noinput:1 ;
	uint		wait:1 ;
	uint		waittimed:1 ;
	uint		empty:1 ;
} ;

struct locinfo {
	LOCINFO_FL	have, f, changed, final ;
	LOCINFO_FL	init, open ;
	PROGINFO	*pip ;
	struct ustat	isb ;
	vecstr		stores ;
	const char	**argv ;
	const char	*argname ;
	const char	*ifname ;
	const char	*ofname ;
	const char	*efname ;
	const char	*rxport ;
	int		ifd ;
	int		ofd ;
	int		efd ;
	int		timeout ;
	int		argc ;
	int		ai_pass ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;

static int	getprogmode(PROGINFO *,cchar *) ;
static int	makedate_get(cchar *,cchar **) ;

static int	procprintnodes(PROGINFO *,cchar *) ;
static int	procprintnoding(PROGINFO *,bfile *) ;
static int	procdial(PROGINFO *,cchar *,cchar *) ;

static int	procuserinfo_begin(PROGINFO *,USERINFO *) ;
static int	procuserinfo_end(PROGINFO *) ;

static int	locinfo_start(LOCINFO *,PROGINFO *) ;
static int	locinfo_finish(LOCINFO *) ;

#if	CF_LOCINFOSET
static int	locinfo_setentry(LOCINFO *,cchar **,cchar *,int) ;
#endif /* CF_LOCINFOSET */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"TMPDIR",
	"VERSION",
	"VERBOSE",
	"LOGFILE",
	"HELP",
	"sn",
	"ef",
	"of",
	"if",
	"an",
	"empty",
	"mode",
	"pm",
	NULL
} ;

enum argopts {
	argopt_tmpdir,
	argopt_version,
	argopt_verbose,
	argopt_root,
	argopt_logfile,
	argopt_help,
	argopt_sn,
	argopt_ef,
	argopt_of,
	argopt_if,
	argopt_an,
	argopt_empty,
	argopt_mode,
	argopt_pm,
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

static const char *progmodenames[] = {
	"dial",
	"nodes",
	NULL
} ;

enum progmodenames {
	progmodename_dial,
	progmodename_nodes,
	progmodename_overlast
} ;

static struct errmsg	errmsgs[] = {
	{ SR_PROTO, EX_PROTOCOL, "protocol error" },
	{ SR_NONET, EX_NOHOST, "node not in cluster" },
	{ SR_NOENT, EX_NOPROG, "program not found" },
	{ SR_NOTDIR, EX_UNAVAILABLE, "directory not found" },
	{ SR_HOSTUNREACH, EX_NOHOST, "node is unreachable" },
	{ SR_TIMEDOUT, EX_NOHOST, "connection attempt timed out" },
	{ SR_HOSTDOWN, EX_NOHOST, "node is down" },
	{ SR_CONNREFUSED, EX_TEMPFAIL, "connection was refused" },
	{ SR_NOMEM, EX_TEMPFAIL, "memory was low" },
	{ SR_NOTAVAIL, EX_TEMPFAIL, "node resource not available" },
	{ SR_OK, EX_OK, "" },
} ;


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	LOCINFO		li, *lip = &li ;
	bfile		errfile ;

#if	(CF_DEBUGS || CF_DEBUG) && CF_DEBUGMALL
	uint		mo_start = 0 ;
#endif

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos, ai_pass ;
	int		npa = 0 ;
	int		rs, rs1 ;
	int		i, cl ;
	int		v ;
	int		progmode = 0 ;
	int		timeout = -1 ;
	int		ex = EX_INFO ;
	int		f_optplus, f_optminus, f_optequal ;
	int		f_exitargs = FALSE ;
	int		f_version = FALSE ;
	int		f_makedate = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_noinput = FALSE ;
	int		f_x = FALSE ;
	int		f_d = FALSE ;
	int		f_wait = FALSE ;
	int		f_waittimed = FALSE ;
	int		f_empty = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*progmodename = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*argname = NULL ;
	const char	*rcmdname = NULL ;
	const char	*rhostname = NULL ;
	const char	*rxport = NULL ;
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
	pip->intkeep = DEFINTKEEP ;
	pip->daytime = time(NULL) ;

	pip->lip = lip ;
	rs = locinfo_start(lip,pip) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badlocstart ;
	}

/* parse arguments */

	npa = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	ai_pass = 0 ;
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
	                    f_makedate = f_version ;
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        rs= SR_INVALID ;
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

/* LOGFILE */
	                case argopt_logfile:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->lfname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                pip->lfname = argp ;
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

/* input file-name */
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

/* argument name */
	                case argopt_an:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            argname = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                argname = argp ;
	                        } else
	                            rs = SR_INVALID ;
	                    }
	                    break ;

	                case argopt_empty:
	                    f_empty = TRUE ;
	                    break ;

/* program mode */
	                case argopt_mode:
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            progmodename = avp ;
	                    } else {
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                progmodename = argp ;
	                        } else
	                            rs = SR_INVALID ;
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

	                    case 'V':
	                        f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

/* alternate user's NETRC file */
	                    case 'N':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->netfname = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    pip->netfname = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

/* cluster name to use */
	                    case 'c':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->clustername = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    pip->clustername = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

/* change directories on remote side */
	                    case 'd':
	                        f_d = TRUE ;
	                        break ;

/* input file name */
	                    case 'i':
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

/* specify target host */
	                    case 'h':
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                rhostname = avp ;
	                        } else {
	                            if (argr > 0) {
	                                argp = argv[++ai] ;
	                                argr -= 1 ;
	                                argl = strlen(argp) ;
	                                if (argl)
	                                    rhostname = argp ;
	                            } else
	                                rs = SR_INVALID ;
	                        }
	                        break ;

/* no input mode (command only) */
	                    case 'n':
	                        pip->f.ni = TRUE ;
	                        f_noinput = TRUE ;
	                        break ;

	                    case 'k':
	                        pip->f.keepalive = TRUE ;
	                        break ;

	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* keepalive interval */
	                    case 's':
	                        pip->f.sanity = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdecti(avp,avl,&v) ;
	                                pip->intkeep = v ;
	                            }
	                        }
	                        break ;

/* timeout? */
	                    case 't':
	                        if (argr > 0) {
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdecti(argp,argl,&v) ;
	                                timeout = v ;
	                            }
	                        } else
	                            rs = SR_INVALID ;
	                        break ;

/* export variables in RXPORT */
	                    case 'x':
	                        f_x = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                rxport = avp ;
	                        }
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

	                    case 'w':
	                        f_wait = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            f_waittimed = TRUE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                timeout = rs ;
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

	        switch (npa) {
	        case 0:
	            rhostname = argp ;
	            break ;
	        case 1:
	            rcmdname = argp ;
	            ai_pass = ai + 1 ;
	            f_exitargs = TRUE ;
	            break ;
	        } /* end switch */
	        npa += 1 ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;
	    if (f_exitargs) break ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUGS
	debugprintf("main/while-out rs=%d\n",rs) ;
#endif

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
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
	    debugprintf("main: progname=%s\n",pip->progname) ;
	    debugprintf("main: ai_pass=%u\n",ai_pass) ;
	}
#endif /* CF_DEBUG */

	if (f_version) {
	    cchar	*pn = pip->progname ;
	    bprintf(pip->efp,"%s: version %s\n",pn,VERSION) ;
	    if (f_makedate) {
	        cl = makedate_get(cex_makedate,&cp) ;
	        bprintf(pip->efp,"%s: makedate %t\n",pn,cp,cl) ;
	    }
	} /* end if (version) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main: f_exitargs=%d ai_pass=%d\n",f_exitargs,ai_pass) ;
	    debugprintf("main: rhostname=%s\n",rhostname) ;
	    debugprintf("main: rcmdname=%s\n",rcmdname) ;
	    for ((cl = 0, i = ai_pass) ; argv[i] != NULL ; (cl += 1, i += 1)) {
	        debugprintf("main: rarg[%u]=>%s<\n",cl,argv[i]) ;
	    }
	}
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: sanity timeout=%u\n",
	        pip->progname,pip->intkeep) ;
	}

/* initialze some stuff */

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
	    bprintf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* get our program mode if any */

	progmode = getprogmode(pip,progmodename) ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: progmode=%s(%d)\n",
	        pip->progname,progmodenames[progmode],progmode) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help */

	if (f_help) {
	    ex = EX_INFO ;
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
	}

	if (f_help || f_version || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* check arguments */

	if ((timeout < 0) && (argval != NULL)) {
	    rs = cfdecti(argval,-1,&v) ;
	    timeout = v ;
	}

	if (timeout < 0)
	    timeout = TO_CONNECT ;

/* get a TMPDNAME */

	if (pip->tmpdname == NULL) pip->tmpdname = getenv(VARTMPDNAME) ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

	if (pip->clustername == NULL)
	    pip->clustername = getenv(VARCLUSTER) ;

/* prepatory initialization */

	if (! f_x) {
	    if (rxport == NULL) rxport = getenv(VARRXPORT) ;
	    if (rxport == NULL) rxport = RXPORT ;
	}

	lip->argv = (cchar **) argv ;
	lip->argc = argc ;
	lip->ai_pass = ai_pass ;

	lip->f.x = f_x ;
	lip->f.d = f_d ;
	lip->f.noinput = f_noinput ;
	lip->f.wait = f_wait ;
	lip->f.waittimed = f_waittimed ;
	lip->f.empty = f_empty ;

	lip->ifname = ifname ;
	lip->ofname = ofname ;
	lip->efname = efname ;
	lip->argname = argname ;
	lip->rxport = rxport ;
	lip->timeout = timeout ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: mid1 rs=%d\n",rs) ;
#endif

/* perform program-mode specific functions */

	if (rs >= 0) {
	    USERINFO	u ;
	    if ((rs = userinfo_start(&u,NULL)) >= 0) {
	        if ((rs = procuserinfo_begin(pip,&u)) >= 0) {
	            if ((rs = proglog_begin(pip,&u)) >= 0) {
	                if ((rs = proguserlist_begin(pip)) >= 0) {

	                    if (progmode == progmodename_dial) {
	                        cchar	*pn = pip->progname ;
	                        cchar	*rh = rhostname ;
	                        if ((rh == NULL) || (rh[0] == '\0')) {
	                            cchar	*fmt ;
	                            fmt = "%s: no host specification given\n" ;
	                            rs = SR_INVALID ;
	                            ex = EX_DATAERR ;
	                            bprintf(pip->efp,fmt,pn) ;
	                        }
	                        if (rs >= 0) {
	                            rs = procdial(pip,rhostname,rcmdname) ;
	                        }
	                    } else if (progmode == progmodename_nodes) {
	                        rs = procprintnodes(pip,NULL) ;
	                    } /* end if (according to program mode) */

	                    rs1 = proguserlist_end(pip) ;
	                    if (rs >= 0) rs = rs1 ;
	                } /* end if (proguserlist) */
	                rs1 = proglog_end(pip) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (proglogfile) */
	            rs1 = procuserinfo_end(pip) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (procuserinfo) */
	        rs1 = userinfo_finish(&u) ;
	        if (rs >= 0) rs = rs1 ;
	    } else {
	        cchar	*pn = pip->progname ;
	        cchar	*fmt = "%s: userinfo failure (%d)\n" ;
	        ex = EX_NOUSER ;
	        bprintf(pip->efp,fmt,pn,rs) ;
	    } /* end if (userinfo) */
	} else if (ex == EX_OK) {
	    cchar	*pn = pip->progname ;
	    cchar	*fmt = "%s: invalid argument or configuration (%d)\n" ;
	    ex = EX_USAGE ;
	    bprintf(pip->efp,fmt,pn,rs) ;
	    usage(pip) ;
	} /* end if (ok) */

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
	} /* end if */

retearly:
	if (pip->debuglevel > 0) {
	    bprintf(pip->efp, "%s: exiting ex=%u (%d)\n",
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
#endif

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
	int		rs ;
	int		wlen = 0 ;
	const char	*pn = pip->progname ;
	const char	*fmt ;

	fmt = "%s: USAGE> %s [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  <node> [-n] <program> [<progarg(s)> ...]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int getprogmode(PROGINFO *pip,cchar *progmodename)
{
	int		progmode = 0 ;
	if (progmodename == NULL) {
	    char	*cp ;
	    if ((cp = strchr(pip->progname,'-')) != NULL) {
	        progmodename = (cp + 1) ;
	    }
	} /* end if (program mode) */
	if (progmodename != NULL) {
	    int	i ;
	    for (i = 0 ; progmodenames[i] != NULL ; i += 1) {
	        if (strcmp(progmodenames[i],progmodename) == 0)
	            break ;
	    } /* end for */
	    if (progmodenames[i] != NULL) {
	        progmode = i ;
	    }
	} /* end if (mode name was specified) */
	return progmode ;
}
/* end subroutine (getprogmode) */


static int procuserinfo_begin(PROGINFO *pip,USERINFO *uip)
{
	int		rs = SR_OK ;

	pip->nodename = uip->nodename ;
	pip->domainname = uip->domainname ;
	pip->username = uip->username ;
	pip->gecosname = uip->gecosname ;
	pip->realname = uip->realname ;
	pip->name = uip->name ;
	pip->fullname = uip->fullname ;
	pip->mailname = uip->mailname ;
	pip->org = uip->organization ;
	pip->logid = uip->logid ;
	pip->pid = uip->pid ;
	pip->uid = uip->uid ;
	pip->euid = uip->euid ;
	pip->gid = uip->gid ;
	pip->egid = uip->egid ;

/* set our effective UID to the real user's UID (for safety) */

#ifdef	COMMENT
	if (pip->uid != pip->euid) u_seteuid(pip->uid) ;
	if (pip->gid != pip->egid) u_setegid(pip->gid) ;
#endif /* COMMENT */

	if (rs >= 0) {
	    const int	hlen = MAXHOSTNAMELEN ;
	    char	hbuf[MAXHOSTNAMELEN+1] ;
	    cchar	*nn = pip->nodename ;
	    cchar	*dn = pip->domainname ;
	    if ((rs = snsds(hbuf,hlen,nn,dn)) >= 0) {
	        cchar	**vpp = &pip->hostname ;
	        rs = proginfo_setentry(pip,vpp,hbuf,rs) ;
	    }
	}

	return rs ;
}
/* end subroutine (procuserinfo_begin) */


static int procuserinfo_end(PROGINFO *pip)
{
	int		rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (procuserinfo_end) */


/* get the date out of the ID string */
static int makedate_get(cchar *md,cchar **rpp)
{
	int		rs = SR_OK ;
	int		ch ;
	const char	*cp ;

	if (rpp != NULL) *rpp = NULL ;

	if ((cp = strchr(md,CH_RPAREN)) != NULL) {
	    const char	*sp ;

	    while (CHAR_ISWHITE(*cp)) cp += 1 ;

	    ch = MKCHAR(*cp) ;
	    if (! isdigitlatin(ch)) {
	        while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;
	        while (CHAR_ISWHITE(*cp)) cp += 1 ;
	    } /* end if (skip over the name) */

	    sp = cp ;
	    if (rpp != NULL) *rpp = cp ;

	    while (*cp && (! CHAR_ISWHITE(*cp))) cp += 1 ;

	    rs = (cp - sp) ;
	} else {
	    rs = SR_NOENT ;
	}

	return rs ;
}
/* end subroutine (makedate_get) */


static int procprintnodes(PROGINFO *pip,cchar *ofn)
{
	bfile		outfile, *ofp = &outfile ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((ofn == NULL) || (ofn[0] == '\0'))
	    ofn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ofn,"dwct",0666)) >= 0) {
	    rs = procprintnoding(pip,ofp) ;
	    wlen += rs ;
	    rs1 = bclose(ofp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprintnodes) */


static int procprintnoding(PROGINFO *pip,bfile *ofp)
{
	DBI		info ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = dbi_open(&info,pip->pr)) >= 0) {
	    VECSTR	clusters, nodes ;
	    if ((rs = vecstr_start(&clusters,10,0)) >= 0) {
	        if ((rs = vecstr_start(&nodes,30,0)) >= 0) {
	            cchar	*cn = pip->clustername ;
	            cchar	*nn = pip->nodename ;

	            if ((cn != NULL) && (cn[0] != '\0')) {
	                rs = vecstr_add(&clusters,cn,-1) ;
	            } else {
	                rs = dbi_getclusters(&info,&clusters,nn) ;
	            }

	            if (rs >= 0) {
	                rs = dbi_getnodes(&info,&clusters,&nodes) ;
	            }

	            if (rs >= 0) {
	                int	i ;
	                cchar	*cp ;
	                for (i = 0 ; vecstr_get(&nodes,i,&cp) >= 0 ; i += 1) {
	                    if (cp != NULL) {
	                        rs = bprintline(ofp,cp,-1) ;
	                        wlen += rs ;
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
	            } /* end if */

	            rs1 = vecstr_finish(&nodes) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (nodes) */
	        rs1 = vecstr_finish(&clusters) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (clusters) */
	    rs1 = dbi_close(&info) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (dbi) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procprintnoding: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procprintnoding) */


static int procdial(PROGINFO *pip,cchar *rhostname,cchar *rcmdname)
{
	LOCINFO		*lip = pip->lip ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		ifd = FD_STDIN ;
	int		ofd = FD_STDOUT ;
	int		efd = FD_STDERR ;
	int		rfd ;
	int		rfd2 ;
	const char	*cp ;
	char		timebuf[TIMEBUFLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main/procdial: ent\n") ;
#endif

	if (strcmp(rhostname,"-") == 0)
	    rhostname = pip->nodename ;

#if	CF_DEFAULTSHELL
	if ((rcmdname == NULL) || (rcmdname[0] == '\0')) {
	    if ((rcmdname = getenv(VARSHELL)) == NULL) {
	        rcmdname = DEFPROGSHELL ;
	    }
	} /* end if */
#endif /* COMMENT */

	if ((rcmdname == NULL) || (rcmdname[0] == '\0')) {
	    rs = SR_INVALID ;
	    goto badprog ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: target h=%s \n",rhostname) ;
#endif

	if (pip->open.logprog)
	    proglog_printf(pip,"node=%s", rhostname) ;

/* open files (if necessary) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main: about to open input file\n") ;
#endif

	if (! lip->f.noinput) {
	    struct ustat	isb ;
	    const char		*ifname = lip->ifname ;

	    if ((ifname != NULL) && (ifname[0] != '\0')) {
	        u_close(FD_STDIN) ;
	        rs = u_open(ifname,O_RDONLY,0666) ;
	        ifd = rs ;
	    }

	    if (rs >= 0) rs = u_fstat(ifd,&isb) ;

	} /* end if (we have input) */
	if (rs < 0) goto badinput ;

/* create the remote command */

	if (rs >= 0) {
	    const int	to = lip->timeout ;
	    int		an, size ;
	    int		argc = lip->argc ;
	    int		ai_pass = lip->ai_pass ;
	    int		opts = 0 ;
	    const char	**argv = lip->argv ;
	    const char	**av ;
	    const char	**ev ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("main: dialcprog() rcmdname=%s\n",
	            rcmdname) ;
	        debugprintf("main: argc=%d\n",argc) ;
	        debugprintf("main: creating remote ai_pass=%d\n",ai_pass) ;
	        debugprintf("main: to=%d\n",to) ;
	    }
#endif

	    opts |= DIALOPT_PWD ;
	    if (lip->f.wait) opts |= DIALOPT_WAIT ;
	    if (lip->f.waittimed) opts |= DIALOPT_WTIMED ;
	    if (lip->f.empty) opts |= DIALOPT_EMPTY ;

	    an = 1 ;
	    if (ai_pass > 0) {
	        an += (argc - ai_pass) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: an=%u\n",an) ;
#endif

	    size = (an + 2) * sizeof(char *) ;
	    if ((rs = uc_malloc(size,&av)) >= 0) {
	        const char	*pr = pip->pr ;
	        const char	*rh = rhostname ;
	        const char	*rc = rcmdname ;

	        ev = pip->envv ;

	        if (lip->argname == NULL) {
	            av[0] = VARSHELL ;
	            rs1 = sfbasename(rcmdname,-1,&cp) ;
	            if (rs1 > 0) av[0] = cp ;
	        } else {
	            av[0] = lip->argname ;
		}

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: argz=%s\n",av[0]) ;
#endif

	        for (i = 1 ; i < an ; i += 1) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4)) {
	                int	ai = (ai_pass + i - 1) ;
	                debugprintf("main: i=%u ai=%u\n",i,ai) ;
	                debugprintf("main: i=%u argv[%u]=%s\n",
	                    i,ai,argv[ai]) ;
	            }
#endif

	            av[i] = argv[ai_pass + i - 1] ;
	        }

	        av[i] = NULL ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: dialcprog() \n") ;
#endif

	        rs = dialcprog(pr,rh,rc,av,ev,to,opts,&rfd2) ;
	        rfd = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: dialcprog() rs=%d\n",rs) ;
#endif

	        uc_free(av) ;
	    } /* end if (m-a-f) */

	} /* end block */
	if (rs < 0) goto baddial ;

	pip->daytime = time(NULL) ;

	if (pip->open.logprog) {
	    timestr_logz(pip->daytime,timebuf) ;
	    proglog_printf(pip,"%s connected",timebuf) ;
	}

/* go into transfer mode */

	if (rs >= 0) {

	    rs = transfer(pip,rhostname,rfd,rfd2,ifd,ofd,efd,MSGBUFLEN) ;

	    pip->daytime = time(NULL) ;

	    if (pip->open.logprog) {
	        proglog_printf(pip,"%s connection closed (%d)\n",
	            timestr_logz(pip->daytime,timebuf),rs) ;
	    }

	    if (pip->debuglevel > 0) {
	        bprintf(pip->efp,"%s: connection closed (%d)\n",
	            pip->progname,rs) ;
	    }

	} /* end if (transfer phase) */

/* we're out of here */

	u_close(rfd) ;

	u_close(rfd2) ;

#ifdef	COMMENT
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,
	        "%s: remote command completed (%d)\n",
	        pip->progname,rs) ;
#endif

/* finished */
badinput:
badprog:
ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main/procdial: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
baddial:
	for (i = 0 ; errmsgs[i].rs != 0 ; i += 1) {
	    if (rs == errmsgs[i].rs) break ;
	} /* end for */

#ifdef	COMMENT
	ex = errmsgs[i].ex ;
#endif
	cp = errmsgs[i].msgstr ;
	if (errmsgs[i].rs == 0) {
#ifdef	COMMENT
	    ex = EX_SOFTWARE ;
#endif
	    cp = "unknown error" ;
	}

	proglog_printf(pip,"%s (%d)",cp,rs) ;

	if (errmsgs[i].rs == SR_NOENT) {
	    logfile_printf(&pip->lh,"rcmdname=%s",rcmdname) ;
	}

	if (! pip->f.quiet) {

	    bprintf(pip->efp,"%s: %s (%d)\n",
	        pip->progname,cp,rs) ;

	    if (errmsgs[i].rs == SR_NOENT)
	        bprintf(pip->efp,"%s: rcmdname=%s\n",
	            pip->progname,rcmdname) ;

	} /* end if */

	goto ret0 ;
}
/* end subroutine (procdial) */


static int locinfo_start(LOCINFO *lip,PROGINFO *pip)
{
	int		rs ;

	if (lip == NULL) return SR_FAULT ;

	memset(lip,0,sizeof(LOCINFO)) ;
	lip->pip = pip ;

	if ((rs = vecstr_start(&lip->stores,0,0)) >= 0) {
	    lip->open.stores = TRUE ;
	}

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


#if	CF_LOCINFOSET
int locinfo_setentry(LOCINFO *lip,cchar **epp,cchar *vp,int vl)
{
	VECSTR		*slp ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (lip == NULL) return SR_FAULT ;
	if (epp == NULL) return SR_INVALID ;

	slp = &lip->stores ;
	if (! lip->open.stores) {
	    if ((rs = vecstr_start(slp,0,0)) >= 0) {
	        lip->open.stores = TRUE ;
	    }
	}

	if (rs >= 0) {
	    int	oi = -1 ;
	    if (*epp != NULL) {
		oi = vecstr_findaddr(slp,*epp) ;
	    }
	    if (vp != NULL) {
	        len = strnlen(vp,vl) ;
	        rs = vecstr_store(slp,vp,len,epp) ;
	    } else  {
	        *epp = NULL ;
	    }
	    if ((rs >= 0) && (oi >= 0)) {
	        vecstr_del(slp,oi) ;
	    }
	} /* end if (ok) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (locinfo_setentry) */
#endif /* CF_LOCINFOSET */


