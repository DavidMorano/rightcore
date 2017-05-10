/* main */

/* SHELL built-in similar to 'who(1)' */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 2004-01-13, David A­D­ Morano

	This program is now the WN KSH built-in command.


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a built-in command to the KSH shell. It should also be able to
        be made into a stand-alone program without much (if almost any)
        difficulty, but I have not done that yet.

        This built-in is pretty straight forward. We used the supplied UNIX
        system subroutines for accessing UTMPX because it is more portable and
        speed is not paramount, like it would be if we were continuously
        scanning the UTMPX DB for changed events. In short, the system UTMPX
        subroutines are fast enough for a one-shot scan through the UTMPX DB
        like we are doing here.

	One interesting thing to note is that we do maintain a cache of
	the username-to-realname translations.  Even though we are only
	scanning the UTMPX DB once, there may be several repeat logins
	by the same username.  Using the cache, we bypass asking the
	system for every username for those usernames that we've seen
	before.  I think that the cost of maintaining the cache is not
	as bad as asking the system for every username.  Your mileage
	may vary! :-)

	Synopsis:


*******************************************************************************/


#if	defined(SFIO) || defined(KSHBUILTIN)
#undef	CF_SFIO
#define	CF_SFIO	1
#else
#ifndef	CF_SFIO
#define	CF_SFIO	0
#endif
#endif

#if	CF_SFIO
#include	<shell.h>
#endif

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<vecobj.h>
#include	<hdbstr.h>
#include	<field.h>
#include	<getxusername.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"shio.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	USERNAMELEN
#define	USERNAMELEN	32
#endif

#ifndef	GNAMELEN
#define	GNAMELEN	80		/* GECOS name length */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100		/* real name length */
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	DEFNUSERS
#define	DEFNUSERS	20		/* default number of users expected */
#endif

#define	COLS_USERNAME	8
#define	COLS_REALNAME	39

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	MAXOUT(f)	if ((f) > 99.9) (f) = 99.9

#define	LOCINFO		struct locinfo
#define	LOCINFO_FL	struct locinfo_flags


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	mkgecosname(char *,int,const char *) ;
extern int	termwritable(const char *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct ustats {
	uint		total ;
	uint		cachehits ;
} ;

struct locinfo_flags {
	uint		stores:1 ;
	uint		header:1 ;
	uint		fmtlong:1 ;
	uint		fmtshort:1 ;
	uint		uniq:1 ;
	uint		users:1 ;
	uint		restricted:1 ;
} ;

struct locinfo {
	vecstr		stores ;
	struct locinfo_flags	have, f, changed, final ;
	struct locinfo_flags	open ;
	struct ustats		s ;
	NAMECACHE		nc ;
	int			to_cache ;
	char			username[USERNAMELEN + 1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;
static int	loadname(PROGINFO *,vecstr *,const char *,int) ;
static int	loadnames(PROGINFO *,VECSTR *,const char *,int) ;
static int	procopts(PROGINFO *,KEYOPT *) ;
static int	process(PROGINFO *,SHIO *,VECSTR *) ;
static int	proccheck(PROGINFO *,vecstr *) ;
static int	procgetdb(PROGINFO *,vecstr *,VECOBJ *) ;
static int	procout(PROGINFO *,SHIO *,VECOBJ *) ;

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGQUIT,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	SIGPIPE,
	SIGPOLL,
	0
} ;

static const int	sigints[] = {
	SIGINT,
	SIGTERM,
	0
} ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"H",
	"HELP",
	"LOGFILE",
	"af",
	"of",
	"h",
	"nh",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_hh,
	argopt_help,
	argopt_logfile,
	argopt_af,
	argopt_of,
	argopt_h,
	argopt_nh,
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

static const char *akonames[] = {
	"header",
	"long",
	"short",
	"uniq",
	"users",
	NULL
} ;

enum akonames {
	akoname_header,
	akoname_long,
	akoname_short,
	akoname_uniq,
	akoname_users,
	akoname_overlast
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


/* exported subroutines */


int main(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
const void	*contextp ;
{
	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;
	PROGINFO	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;
	SHIO		errfile ;
	SHIO		outfile, *ofp = &outfile ;
	KEYOPT		akopts ;
	VECSTR		names ;
	sigset_t	oldsigmask, newsigmask ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan = 0 ;
	int	rs, rs1 ;
	int	n, i, j ;
	int	v, size ;
	int	cl ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	if_exit = 0 ;
	if_int = 0 ;

	n = nelem(sigints) + nelem(sigignores) ;
	size = n * sizeof(struct sigaction) ;
	memset(sao,0,size) ;

/* block some signals and catch the others */

	uc_sigsetempty(&newsigmask) ;

	for (i = 0 ; sigblocks[i] != 0 ; i += 1)
	    uc_sigsetadd(&newsigmask,sigblocks[i]) ;

	u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;

	uc_sigsetempty(&newsigmask) ;

/* ignore these signals */

	j = 0 ;
	for (i = 0 ; sigignores[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;

	    san.sa_handler = SIG_IGN ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigignores[i],&san,(sao + j)) ;

	    j += 1 ;

	} /* end for */

/* interrupt on these signals */

	for (i = 0 ; sigints[i] != 0 ; i += 1) {

	    memset(&san,0,sizeof(struct sigaction)) ;

	    san.sa_handler = sighand_int ;
	    san.sa_mask = newsigmask ;
	    san.sa_flags = 0 ;
	    u_sigaction(sigints[i],&san,(sao + j)) ;

	    j += 1 ;

	} /* end for */

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,environ,argv[0],VERSION) ;

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) == NULL)
	    cp = STDERRFNAME ;

	rs1 = shio_open(&errfile,cp,"wca",0666) ;
	if (rs1 >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	}

/* initialize */

	memset(lip,0,sizeof(struct locinfo)) ;

	pip->lip = &li ;

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

	            rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

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
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                    }
	                    break ;

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

	                case argopt_hh:
	                case argopt_h:
	                    lip->have.header = TRUE ;
	                    lip->f.header = TRUE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdeci(avp,avl,&v) ;
	                            lip->f.header = (v > 0) ;
	                        }
	                    }

	                    break ;

	                case argopt_nh:
	                    lip->have.header = TRUE ;
	                    lip->f.header = FALSE ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdeci(avp,avl,&v) ;
	                            lip->f.header = (v > 0) ;
	                        }
	                    }

	                    break ;

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* argument file */
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

/* output name */
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

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    shio_printf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                } /* end switch */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                        }

	                        break ;

/* quiet mode */
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

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* print header */
	                    case 'h':
	                        lip->have.header = TRUE ;
	                        lip->f.header = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                lip->f.header = (v > 0) ;
	                            }
	                        }

	                        break ;

/* long mode */
	                    case 'l':
	                        lip->have.fmtlong = TRUE ;
	                        lip->f.fmtlong = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                lip->f.fmtlong = (v > 0) ;
	                            }
	                        }

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

/* short mode */
	                    case 's':
	                        lip->f.fmtshort = TRUE ;
	                        lip->have.fmtshort = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                lip->f.fmtshort = (v > 0) ;
	                            }
	                        }

	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        shio_printf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits as argument or not) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

	} /* end while (all command line argument processing) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (rs < 0) {
	    ex = EX_USAGE ;
	    shio_printf(pip->efp,
	        "%s: invalid argument specified (%d)\n",
	        pip->progname,rs) ;
	    usage(pip) ;
	    goto retearly ;
	}

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

/* help file */

	if (f_usage)
	    usage(pip) ;

	if (f_help) {
#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_version || f_usage || f_help)
	    goto retearly ;


	ex = EX_OK ;

/* load up the environment options */

	rs = procopts(pip,&akopts) ;

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* argument defaults */

	if (lip->f.fmtshort && lip->f.uniq)
	    lip->f.users = TRUE ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: f_quick=%u\n",lip->f.fmtshort) ;
#endif

/* other initilization */

	if ((cp = getenv(VARUSERNAME)) != NULL)
	    strwcpy(lip->username,cp,USERNAMELEN) ;

/* OK, do the thing */

	rs = vecstr_start(&names,10,0) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

/* gather invocation login names */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    if (cp[0] != '\0')
	        rs = loadname(pip,&names,cp,-1) ;

	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {
	    SHIO	argfile, *afp = &argfile ;

	    if (strcmp(afname,"-") == 0)
	        afname = STDINFNAME ;

	    if ((rs = shio_open(afp,afname,"r",0666)) >= 0) {
	        int	len ;
	        char	lbuf[LINEBUFLEN + 1] ;

	        while ((rs = shio_readline(afp,lbuf,LINEBUFLEN)) > 0) {
	            len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;
	            lbuf[len] = '\0' ;

		    cp = lbuf ;
		    cl = len ;
	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

		    rs = loadnames(pip,&names,cp,cl) ;
		    pan += rs ;
	            if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        shio_close(afp) ;

	    } else {
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;
	            shio_printf(pip->efp,"%s: \trs=%d argfile=%s\n",
	                pip->progname,rs,afname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if (rs < 0)
	    goto done ;

/* initialize a username-realname cache */

	rs = namecache_open(&lip->nc,VARUSERNAME,lip->to_cache) ;

	if (rs < 0)
	    goto badcacheopen ;

/* add a cache entry if we can */

	{
	    char	*up = getenv(VARUSERNAME) ;

	    if ((up != NULL) && ((cp = getenv(VARNAME)) != NULL))
		rs = namecache_add(&lip->nc,up,cp,-1) ;

	} /* end block */

	if (rs < 0)
	    goto badcacheadd ;

/* open output file */

	if ((ofname == NULL) || (ofname[0] == '\0'))
	    ofname = STDOUTFNAME ;

	if ((rs = shio_open(ofp,ofname,"wct",0666)) >= 0) {

/* do the processing */

	rs = process(pip,ofp,&names) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: process() rs=%d\n",rs) ;
#endif

	shio_close(ofp) ;
	} else {
	    ex = EX_CANTCREAT ;
	    shio_printf(pip->efp,"%s: output unavailable (%d)\n",
	        pip->progname,rs) ;
	}

	if (pip->debuglevel > 0) {
	    NAMECACHE_STATS	s ;

	    shio_printf(pip->efp,"%s: total records=%u\n",
	        pip->progname,lip->s.total) ;

	    rs1 = namecache_stats(&lip->nc,&s) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("main: namecache_stats() rs=%d\n",rs1) ;
	    if (rs1 >= 0) {
	    debugprintf("main: cache total=%u\n", s.total) ;
	    debugprintf("main: cache phits=%u\n", s.phits) ;
	    debugprintf("main: cache nhits=%u\n", s.nhits) ;
	    }
	}
#endif /* CF_DEBUG */

	    if (rs1 >= 0) {
	    shio_printf(pip->efp,"%s: cache phits=%u\n",
	        pip->progname,s.phits) ;
	    shio_printf(pip->efp,"%s: cache nhits=%u\n",
	        pip->progname,s.nhits) ;
	    }

	} /* end if */

/* we're done */
badoutopen:
badcacheadd:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: namecache_close()\n") ;
#endif

	namecache_close(&lip->nc) ;

badcacheopen:
done:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: vecstr_finish() names\n") ;
#endif

	vecstr_finish(&names) ;

ret4:
	if ((rs < 0) && (! pip->f.quiet))
	    shio_printf(pip->efp,
	        "%s: could not perform function (%d)\n",
	        pip->progname,rs) ;

	if ((rs < 0) && (ex == EX_OK)) {

	    ex = mapex(mapexs,rs) ;

	} else if (if_int)
	    ex = EX_INTR ;

/* early return thing */
retearly:
ret3:
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret2:
	if (pip->open.akopts) {
	    pip->open.akopts = FALSE ;
	    keyopt_finish(&akopts) ;
	}

ret1:
	shio_close(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

/* restore and get out */

	j = 0 ;
	for (i = 0 ; sigints[i] != 0 ; i += 1)
	    u_sigaction(sigints[i],(sao + j++),NULL) ;

	for (i = 0 ; sigignores[i] != 0 ; i += 1)
	    u_sigaction(sigignores[i],(sao + j++),NULL) ;

	u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;

	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{


	if_int = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;

	const char	*pn = pip->progname ;
	const char	*fmt ;


	fmt = "%s: USAGE> %s [-h[=n]] [-s|-l] [<logname(s)> ...]\n" ;
	rs = shio_printf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "%s:  [-Q] [-D] [-v[=n]] [-HELP] [-V]\n" ;
	rs = shio_printf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int loadnames(pip,nlp,sp,sl)
PROGINFO	*pip ;
VECSTR		*nlp ;
const char	*sp ;
int		sl ;
{
	FIELD	fsb ;

	int	rs ;
	int	fl ;
	int	c = 0 ;

	const char	*fp ;


	if (nlp == NULL)
	    return SR_FAULT ;

	if ((rs = field_start(&fsb,sp,sl)) >= 0) {

	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl == 0) continue ;

	        c += 1 ;
	        rs = loadname(pip,nlp,fp,fl) ;

	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (loadnames) */


static int loadname(pip,nlp,cp,cl)
PROGINFO	*pip ;
vecstr		*nlp ;
const char	cp[] ;
int		cl ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;


	if (cp[0] == '-') {
	    cp = lip->username ;
	    cl = -1 ;
	    if (lip->username[0] == '\0') {
		rs = getusername(lip->username,USERNAMELEN,-1) ;
		cl = rs ;
	    }
	}

	if (rs >= 0)
	    rs = vecstr_adduniq(nlp,cp,cl) ;

	return rs ;
}
/* end subroutine (loadname) */


/* process the program ako-options */
static int procopts(pip,kop)
PROGINFO	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	c = 0 ;

	const char	*cp ;


	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs >= 0) {
	    KEYOPT_CUR	kcur ;
	    if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	        int		oi ;
	        int		kl, vl ;
	        const char	*kp, *vp ;

		while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

	    	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

	            if ((oi = matostr(akonames,2,kp,kl)) >= 0) {
	                uint	uv ;

	        switch (oi) {

	        case akoname_header:
	            if (! lip->final.header) {
	                lip->have.header = TRUE ;
	                lip->final.header = TRUE ;
	                lip->f.header = TRUE ;
	                if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                    lip->f.header = (uv > 0) ? 1 : 0 ;
	            }

	            break ;

	        case akoname_long:
	            if (! lip->final.fmtlong) {
	                lip->have.fmtlong = TRUE ;
	                lip->final.fmtlong = TRUE ;
	                lip->f.fmtlong = TRUE ;
	                if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                    lip->f.fmtlong = (uv > 0) ? 1 : 0 ;
	            }
	            break ;

	        case akoname_short:
	            if (! lip->final.fmtshort) {

	                lip->have.fmtshort = TRUE ;
	                lip->final.fmtshort = TRUE ;
	                lip->f.fmtshort = TRUE ;
	                if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                    lip->f.fmtshort = (uv > 0) ? 1 : 0 ;

	            }

	            break ;

	        case akoname_uniq:
	            if (! lip->final.uniq) {

	                lip->have.uniq = TRUE ;
	                lip->final.uniq = TRUE ;
	                lip->f.uniq = TRUE ;
	                if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                    lip->f.uniq = (uv > 0) ? 1 : 0 ;

	            }

	            break ;

	        case akoname_users:
	            if (! lip->final.users) {

	                lip->have.users = TRUE ;
	                lip->final.users = TRUE ;
	                lip->f.users = TRUE ;
	                if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                    lip->f.users = (uv > 0) ? 1 : 0 ;

	            }
	            break ;

	        } /* end switch */

	        	c += 1 ;
	            } /* end if (valid option) */

	            if (rs < 0) break ;
	        } /* end while (looping through key options) */

	        keyopt_curend(kop,&kcur) ;
	    } /* end if (cursor) */
	} /* end if (ok) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int process(pip,ofp,nlp)
PROGINFO	*pip ;
SHIO		*ofp ;
VECSTR		*nlp ;
{
	struct locinfo	*lip = pip->lip ;

	vecobj	entries ;

	int	rs ;
	int	size ;
	int	f_entries ;


	size = sizeof(struct utmpx) ;
	rs = vecobj_start(&entries,size,20,0) ;
	f_entries = (rs >= 0) ;

	if (rs >= 0)
	    rs = proccheck(pip,nlp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: do the DB lookups rs=%d\n",rs) ;
#endif

/* do the DB look-ups (access system UTMPX database) */

	if (rs >= 0)
	    rs = procgetdb(pip,nlp,&entries) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: post-process and print rs=%d\n",rs) ;
#endif

	if (rs >= 0)
	    rs = procout(pip,ofp,&entries) ;

/* finish up (we're getting out) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: free entries rs=%d\n",rs) ;
#endif

	if (f_entries)
	    vecobj_finish(&entries) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


/* check any supplied names for ourself */
static int proccheck(pip,nlp)
PROGINFO	*pip ;
vecstr		*nlp ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	i ;
	int	cl ;

	const char	*cp ;


	for (i = 0 ; vecstr_get(nlp,i,&cp) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;

	    lip->f.restricted = TRUE ;
	    if (cp[0] == '-') {
	        cp = lip->username ;
	        cl = -1 ;
	        if (lip->username[0] == '\0') {
	            rs = getusername(lip->username,USERNAMELEN,-1) ;
		    cl = rs ;
		}

		if (rs >= 0) {
	            vecstr_del(nlp,i) ;
	            rs = vecstr_adduniq(nlp,cp,cl) ;
		}

	    }

	    if (rs < 0) break ;
	} /* end for */

	return rs ;
}
/* end subroutine (proccheck) */


/* extract our names from the system UTMPX database */
static int procgetdb(pip,nlp,elp)
PROGINFO	*pip ;
vecstr		*nlp ;
VECOBJ		*elp ;
{
	struct locinfo	*lip = pip->lip ;

	struct utmpx	*up ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	cl ;
	int	c = 0 ;
	int	f ;

	char	*cp ;


	setutxent() ;

	while ((! if_int) && ((up = getutxent()) != NULL)) {

#if	CF_DOTUSER
	    f = (up->ut_type == UTMPX_TUSERPROC) ;
#else
	    f = ((up->ut_type == UTMPX_TUSERPROC) &&
	        (up->ut_user[0] != '.')) ;
#endif /* CF_FOTUSER */

	    if (f && lip->f.restricted) {

	        cp = up->ut_user ;
	        cl = strnlen(up->ut_user,MIN(UTMPX_LUSER,LOGNAMELEN)) ;

	        rs1 = vecstr_findn(nlp,cp,cl) ;

	        f = (rs1 >= 0) ;

	    } /* end if */

	    if (f) {

		c += 1 ;
	        lip->s.total += 1 ;
	        rs = vecobj_add(elp,up) ;

	        if (rs < 0)
	            break ;

	    } /* end if (entered) */

	} /* end while */

	endutxent() ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procgetdb) */


/* process the entries and print them out */
static int procout(pip,ofp,elp)
PROGINFO	*pip ;
SHIO		*ofp ;
VECOBJ		*elp ;
{
	struct locinfo	*lip = pip->lip ;

	struct utmpx	*up ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	ml, cl, rl ;
	int	tmch ;
	int	i ;

	const char	*cp, *rp ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


	if (lip->f.header && (! lip->f.fmtshort)) {
	    const char	*fmt ;

	    if (lip->f.fmtlong) {
	        fmt = "USER       LINE         TIME           "
	            "  ID    SID HOST\n" ;
	    } else
	        fmt = "USER       LINE         TIME           NAME\n" ;

	    rs = shio_printf(ofp,fmt) ;

	} /* end if (header was requested) */

/* do the post-processing and print out */

	for (i = 0 ; (rs >= 0) && (! if_int) && 
	    (vecobj_get(elp,i,&up) >= 0) ; i += 1) {

	    char	ut_userbuf[UTMPX_LUSER + 1] ;
	    char	ut_lbuf[UTMPX_LLINE + 1] ;


	    if (up == NULL) continue ;

	    if (lip->f.fmtshort) {

	        cp = up->ut_user ;
	        cl = strnlen(cp,MIN(COLS_USERNAME,UTMPX_LUSER)) ;

	        rs = shio_printf(ofp,"%t\n",cp,cl) ;

	    } else {

	        ml = UTMPX_LUSER ;
	        strwcpy(ut_userbuf,up->ut_user,ml) ;

	        ml = UTMPX_LLINE ;
	        strwcpy(ut_lbuf,up->ut_line,ml) ;

/* is the terminal writable? */

	        mkpath2(tmpfname,DEVDNAME,ut_lbuf) ;

	        rs1 = termwritable(tmpfname) ;

	        switch (rs1) {

	        case 1:
	            tmch = '+' ;
	            break ;

	        case 2:
	            tmch = '1' + 128 ;
	            break ;

	        default:
	            tmch = ' ' ;

	        } /* end switch */

/* print whichever output format */

	        if (lip->f.fmtlong) {

	            char	ut_idbuf[UTMPX_LID + 1] ;
	            char	ut_hostbuf[UTMPX_LHOST + 1] ;


	            strwcpy(ut_idbuf,up->ut_id,MIN(4,UTMPX_LID)) ;

	            strwcpy(ut_hostbuf,up->ut_host,UTMPX_LHOST) ;

	            rs = shio_printf(ofp,
	                "%-8s %c %-12s %s %4s %6u %s\n",
	                ut_userbuf,tmch,
	                ut_lbuf,
	                timestr_log(up->ut_tv.tv_sec,timebuf),
	                ut_idbuf,
	                up->ut_pid,
	                ut_hostbuf
	                ) ;

	        } else {

	            rs1 = namecache_lookup(&lip->nc,ut_userbuf,&rp) ;

	            rl = rs1 ;
	            if (rl > 0) {

			if (rl > COLS_REALNAME)
			    rl = COLS_REALNAME ;

	                fmt = "%-8s %c %-12s %s (%s)\n" ;

	            } else
	                fmt = "%-8s %c %-12s %s\n" ;

	            rs = shio_printf(ofp,fmt,
	                ut_userbuf,tmch,
	                ut_lbuf,
	                timestr_log(up->ut_tv.tv_sec,timebuf),
	                rp) ;

	        } /* end if */

	    } /* end if (short) */

	} /* end for (entries) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (procout) */


