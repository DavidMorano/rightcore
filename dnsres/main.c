/* main */

/* generic front-end */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1989-03-01, David A­D­ Morano
	This was written for some program (deleted the particulars).

	= 1998-06-01, David A­D­ Morano
	This was enhanced from the original version.

	= 1999-03-01, David A­D­ Morano
	More enhancements.

	= 2006-04-05, David A­D­ Morano
	Enhanced to add dev-inode uniqueness checking.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ consoletime


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<unistd.h>
#include	<stropts.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<sigman.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<ids.h>
#include	<kinfo.h>		/* not thread safe! */
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 3),2048)
#endif

#ifndef	POLLMULT
#define	POLLMULT	1000
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#undef	O_FLAGS
#define	O_FLAGS		(O_WRONLY | O_NOCTTY)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpcasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	acceptpass(int, struct strrecvfd *,int) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	ndig(double *,int) ;
extern int	ndigmax(double *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strdcpy1(char *,int,const char *) ;
extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;

static int	procopts(PROGINFO *,KEYOPT *) ;
static int	procdaemon(PROGINFO *,const char *) ;
static int	procserve(PROGINFO *,const char *) ;
static int	proconce(PROGINFO *,const char *) ;
static int	procout(PROGINFO *,const char *) ;
static int	procprint(PROGINFO *,int,gid_t) ;

static int	procinfo_begin(PROGINFO *) ;
static int	procinfo_nusers(PROGINFO *) ;
static int	procinfo_nprocs(PROGINFO *) ;
static int	procinfo_check(PROGINFO *) ;
static int	procinfo_end(PROGINFO *) ;

static int	msglogdev(IDS *,const char *) ;

static void	sighand_int(int) ;


/* local variables */

static volatile int	if_int ;
static volatile int	if_exit ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
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
	SIGQUIT,
	0
} ;

static const char *argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"HELP",
	"option",
	"pm",
	"sn",
	"ef",
	"af",
	"if",
	"of",
	"to",
	"mnt",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_option,
	argopt_pm,
	argopt_sn,
	argopt_ef,
	argopt_af,
	argopt_if,
	argopt_of,
	argopt_to,
	argopt_mnt,
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

static const char	*progmodes[] = {
	"consoletime",
	"loginblurb",
	NULL
} ;

enum progmodes {
	progmode_consoletime,
	progmode_loginblurb,
	progmode_overlast
} ;

static const char	*progopts[] = {
	"str",
	"date",
	"time",
	"users",
	"procs",
	"mem",
	"load",
	"la",
	"name",
	"nodetitle",
	"node",
	"term",
	"mesg",
	"to",
	NULL
} ;

enum progopts {
	progopt_str,
	progopt_date,
	progopt_time,
	progopt_users,
	progopt_procs,
	progopt_mem,
	progopt_load,
	progopt_la,
	progopt_name,
	progopt_nodetitle,
	progopt_node,
	progopt_term,
	progopt_mesg,
	progopt_to,
	progopt_overlast
} ;

static const char	*ansiterms[] = {
	"ansi",
	"sun",
	"screen",
	"vt100",
	"vt101",
	"vt102",
	"vt220",
	"vt230",
	"vt240",
	"vt320",
	"vt330",
	"vt340",
	"vt420",
	"vt430",
	"vt440",
	"vt520",
	"vt530",
	"vt540",
	NULL
} ;

static const char	*msglogdevs[] = {
	MSGLOGDEV,
	CONSOLEDEV,
	NULL
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;
	struct ustat	sb ;
	SIGMAN		sm ;
	IDS		id ;
	KEYOPT		akopts ;
	bfile		errfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs ;
	int	rs1 ;
	int	size ;
	int	n, i, j ;
	int	v ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*pmspec = NULL ;
	const char	*efname = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*termtype = NULL ;
	const char	*mntfname = NULL ;
	const char	*cp ;


	if_int = 0 ;
	if_exit = 0 ;

	rs = sigman_start(&sm, sigblocks,sigignores,sigints,sighand_int) ;
	if (rs < 0) goto ret0 ;

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

	for (i = 0 ; i < 3 ; i += 1) {
	    if (u_fstat(i,&sb) < 0) {
		int oflags = (i == 0) ? O_RDONLY : O_WRONLY ;
		u_open(NULLFNAME,oflags,0666) ;
	    }
	}

/* initialize */

	pip->verboselevel = 1 ;
	pip->to_open = -1 ;

	pip->f.term = TRUE ;
	pip->f.mesg = FALSE ;

	pip->f.o_string = FALSE ;
	pip->f.o_time = TRUE ;
	pip->f.o_nodetitle = FALSE ;
	pip->f.o_node = TRUE ;
	pip->f.o_users = TRUE ;
	pip->f.o_procs = TRUE ;
	pip->f.o_mem = TRUE ;
	pip->f.o_load = TRUE ;

/* start parsing the arguments */

	rs = keyopt_start(&akopts) ;
	pip->open.akopts = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1)
	    argpresent[ai] = 0 ;

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

	                case argopt_help:
	                    f_help = TRUE ;
	                    break ;

/* the user specified some progopts */
	                case argopt_option:
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

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            pmspec = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pmspec = argp ;
	                    }
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

/* error file */
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

/* input file */
	                case argopt_if:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            ifname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            ifname = argp ;
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

/* mount file */
	                case argopt_mnt:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            mntfname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            mntfname = argp ;
	                    }
	                    break ;

/* timeout (open) */
	                case argopt_to:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl) {
	                            rs = cfdeci(avp,avl,&v) ;
				    pip->to_open = v ;
				}
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
	                            rs = cfdeci(argp,argl,&v) ;
				    pip->to_open = v ;
				}
	                    }
	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    bprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

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

/* terminal-type */
	                    case 'T':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            termtype = argp ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'd':
				pip->f.daemon = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
					pip->intrun = v ;
				    }
				}
	                        break ;

/* observe MESG flag (group-writeable) on output device */
	                    case 'm':
	                        pip->final.mesg = TRUE ;
	                        pip->f.mesg = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.mesg = (rs > 0) ;
	                            }
	                        }
	                        break ;

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

	                    case 'q':
	                        pip->verboselevel = 0 ;
	                        break ;

	                    case 's':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl) {
				    pip->final.o_string = TRUE ;
				    pip->f.o_string = TRUE ;
	                            pip->string = argp ;
				}
	                        break ;

/* specify if output is a terminal or not */
	                    case 't':
				pip->final.term = TRUE ;
				pip->have.term = TRUE ;
	                        pip->f.term = TRUE ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbool(avp,avl) ;
	                                pip->f.term = (rs > 0) ;
	                            }
	                        }
	                        break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optbalue(avp,avl) ;
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

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
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

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->progname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0) {
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    } else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif /* CF_DEBUGS */

	if (pip->progmode < 0)
	    pip->progmode = progmode_consoletime ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: pm=%s\n",
	        pip->progname,progmodes[pip->progmode]) ;

	if (sn == NULL)
	    sn = progmodes[pip->progmode] ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

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
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* program customization */

	switch (pip->progmode) {
	case progmode_loginblurb:
	    pip->f.o_time = FALSE ;
	    if (ofname == NULL) ofname = "-" ;
	    break ;
	case progmode_consoletime:
	    if ((! pip->final.o_time) && pip->f.daemon)
		pip->f.o_time = FALSE ;
	    break ;
	} /* end switch */

/* progopts */

	rs = procopts(pip,&akopts) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badproc ;
	}

/* final defaults */

	if (termtype == NULL)
	    termtype = getenv(VARTERM) ;

	if (termtype != NULL) {
	    n = matpcasestr(ansiterms,2,termtype,-1) ;
	    pip->f.ansiterm = (n >= 0) ;
	} /* end if */

	if (pip->f.daemon && (! pip->final.term)) {
	    pip->f.term = FALSE ;
	}

	if (pip->string == NULL) {
	    pip->f.o_string = TRUE ;
	    pip->string = getenv(VARSTRING) ;
	}

	pip->daytime = time(NULL) ;

	if ((ofname == NULL) && (! pip->f.daemon)) {
	    const char	*ccp ;
	    const char	*backup = NULL ;
	    if ((rs = ids_load(&id)) >= 0) {
	        for (i = 0 ; (msglogdevs[i] != NULL) ; i += 1) {
		    ccp = msglogdevs[i] ;
	            rs1 = msglogdev(&id,ccp) ;
	            if (rs1 >= 0) {
		        if (backup == NULL) backup = ccp ;
		        if (rs1 > 0) {
		            ofname = ccp ;
		            break ;
		        }
		    }
	        } /* end for */
		ids_release(&id) ;
	    } /* end if (IDS) */
	    if (ofname == NULL)
		ofname = backup ;
	} /* end if */

	if ((ofname != NULL) &&
	    ((ofname[0] == '\0') || (ofname[0] == '-'))) {
	    ofname = OUTPUTDEV ;
	    if (! pip->final.mesg)
	        pip->f.mesg = FALSE ;
	}

	if (mntfname == NULL)
	    mntfname = getenv(VARMNTFNAME) ;

	if (pip->to_open < 0)
	    pip->to_open = TO_OPEN ;

	if (pip->f.daemon) {

	    if ((mntfname == NULL) || (mntfname[0] == '\0')) {
		rs = SR_NOENT ;
		ex = EX_NOINPUT ;
	        bprintf(pip->efp,"%s: inaccessible mount-point\n",
	            pip->progname) ;
	    }

	    if (rs >= 0)
	        rs = procdaemon(pip,mntfname) ;

	} else {

	    if ((ofname == NULL) || (ofname[0] == '\0')) {
		rs = SR_NOENT ;
		ex = EX_CANTCREAT ;
	        bprintf(pip->efp,"%s: inaccessible msg-log device\n",
	            pip->progname) ;
	    }

	    if (rs >= 0)
	        rs = proconce(pip,ofname) ;

	} /* end if */

badproc:
badconsole:
done:
	if ((rs < 0) && (ex == EX_OK)) {
	    switch (rs) {

	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;

	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        bprintf(pip->efp,"%s: inaccessible console device (%d)\n",
	            pip->progname,rs) ;
	        break ;

	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;

	    default:
	        ex = mapex(mapexs,rs) ;
	        break ;

	    } /* end switch */
	} /* end if */

/* early return thing */
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	if (pip->efp != NULL) {
	    bclose(pip->efp) ;
	    pip->efp = NULL ;
	}

	if (pip->open.akopts) {
	    keyopt_finish(&akopts) ;
	    pip->open.akopts = FALSE ;
	}

	proginfo_finish(pip) ;

/* restore and get out */
badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	sigman_finish(&sm) ;

ret0:
	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{

	switch (sn) {

	case SIGINT:
	    if_int = TRUE ;
	    break ;

	default:
	    if_exit = TRUE ;
	    break ;

	} /* end switch */

}
/* end subroutine (sighand_int) */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [-of <consdev>] [-t[=<b>]] [-m[=<b>]] \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:  [-o <option(s)>] [-s <string>] [-to <timeout>]\n",
	    pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:  [-Q] [-D] [-v[=<n>]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: where:\n",
	    pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:  <option> is 'node', 'users', 'procs', 'mem', 'load'\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int procopts(pip,kop)
PROGINFO	*pip ;
KEYOPT		*kop ;
{
	KEYOPT_CUR	cur ;

	int	rs = SR_OK ;
	int	v ;
	int	ki ;
	int	kl, vl ;
	int	c = 0 ;

	const char	*kp, *vp ;
	const char	*cp ;


/* grab options from the environment */

	if ((cp = getenv(VAROPTS)) != NULL)
	    rs = keyopt_loads(kop,cp,-1) ;

	if (rs < 0)
	    goto ret0 ;

/* process program options */

	if ((rs = keyopt_curbegin(kop,&cur)) >= 0) {

	while ((kl = keyopt_enumkeys(kop,&cur,&kp)) >= 0) {

	    ki = matostr(progopts,2,kp,kl) ;

	    if (ki >= 0) {
	        c += 1 ;
	        vl = keyopt_fetch(kop,kp,NULL,&vp) ;
	    }

	    switch (ki) {

	    case progopt_str:
		if (! pip->final.o_string) {
		    pip->f.o_string = TRUE ;
		    if (vl > 0) {
		        strdcpy1w(pip->strbuf,STRBUFLEN,vp,vl) ;
		        pip->string = pip->strbuf ;
		    }
		}
		break ;

	    case progopt_date:
	    case progopt_time:
	        pip->f.o_time = TRUE ;
	        if (vl > 0) {
		    rs = optbool(vp,vl) ;
	            pip->f.o_time = (rs > 0) ;
		}
		break ;

	    case progopt_nodetitle:
	        pip->f.o_nodetitle = TRUE ;
	        if (vl > 0) {
		    rs = optbool(vp,vl) ;
	            pip->f.o_nodetitle = (rs > 0) ;
		}
		break ;

	    case progopt_name:
	    case progopt_node:
	        pip->f.o_node = TRUE ;
	        if (vl > 0) {
		    rs = optbool(vp,vl) ;
	            pip->f.o_node = (rs > 0) ;
		}
	        break ;

	    case progopt_users:
	        pip->f.o_users = TRUE ;
	        if (vl > 0) {
		    rs = optbool(vp,vl) ;
	            pip->f.o_users = (rs > 0) ;
		}
	        break ;

	    case progopt_procs:
	        pip->f.o_procs = TRUE ;
	        if (vl > 0) {
		    rs = optbool(vp,vl) ;
	            pip->f.o_procs = (rs > 0) ;
		}
	        break ;

	    case progopt_mem:
	        pip->f.o_mem = TRUE ;
	        if (vl > 0) {
		    rs = optbool(vp,vl) ;
	            pip->f.o_mem = (rs > 0) ;
		}
	        break ;

	    case progopt_load:
	    case progopt_la:
	        pip->f.o_load = TRUE ;
	        if (vl > 0) {
		    rs = optbool(vp,vl) ;
	            pip->f.o_load = (rs > 0) ;
		}
	        break ;

	    case progopt_term:
		if (! pip->final.term) {
	            pip->final.term = TRUE ;
	            pip->have.term = TRUE ;
	            pip->f.term = TRUE ;
	            if (vl > 0) {
			rs = optbool(vp,vl) ;
	                pip->f.term = (rs > 0) ;
		    }
		}
	        break ;

	    case progopt_mesg:
		if (! pip->final.mesg) {
	            pip->final.mesg = TRUE ;
	            pip->f.mesg = TRUE ;
	            if (vl > 0) {
			rs = optbool(vp,vl) ;
	                pip->f.mesg = (rs > 0) ;
		    }
		}
	        break ;

	    case progopt_to:
	        if (vl > 0) {
		    rs = cfdeci(vp,vl,&v) ;
	            pip->to_open = v ;
		}
	        break ;

	    } /* end switch */

	        if (rs < 0) break ;

	    } /* end while (enumerating) */

	    keyopt_curend(kop,&cur) ;
	} /* end if */

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */


static int procdaemon(pip,mntfname)
PROGINFO	*pip ;
const char	mntfname[] ;
{
	int	rs = SR_OK ;
	int	i ;


	if (mntfname == NULL)
	    return SR_FAULT ;

	if (mntfname[0] == '\0')
	    return SR_INVALID ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: mnt=%s\n",pip->progname,mntfname) ;

	if (pip->intpoll <= 0)
	    pip->intpoll = TO_POLL ;

	if (pip->efp != NULL)
	    bflush(pip->efp) ;

	if (pip->debuglevel == 0)
	    rs = uc_fork() ;

	if (rs == 0) {

	    for (i = 0 ; i < 3 ; i += 1) u_close(i) ;

	    uc_sigignore(SIGHUP) ;

	    u_setsid() ;

	    rs = procserve(pip,mntfname) ;

	} /* end if (child) */

ret0:
	return rs ;
}
/* end subroutine (procdaemon) */


static int procserve(pip,mntfname)
PROGINFO	*pip ;
const char	mntfname[] ;
{
	struct pollfd	fds[2] ;

	time_t	ti_run = pip->daytime ;
	time_t	ti_check = pip->daytime ;
	time_t	ti_wait = pip->daytime ;

	int	rs = SR_OK ;
	int	to = pip->intpoll ;
	int	to_check = TO_CHECK ;
	int	pto ;
	int	nfds ;
	int	n, i ;
	int	cfd, sfd, pfd ;
	int	pipes[2] ;
	int	nhandle = 0 ;


	if (mntfname[0] == '\0')
	    return SR_INVALID ;

	if (to < 1) to = 3 ;

	if (to > to_check) to = to_check ;

	rs = procinfo_begin(pip) ;
	if (rs < 0)
	    goto ret0 ;

	rs = u_pipe(pipes) ;
	sfd = pipes[0] ;		/* server-side */
	cfd = pipes[1] ;		/* client-side */
	if (rs < 0)
	    goto ret1 ;

	rs = u_ioctl(cfd,I_PUSH,"connld") ;
	if (rs < 0)
	    goto ret2 ;

/* attach the client end to the file created above */

	rs = uc_fattach(cfd,mntfname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("main/procserve: mntfname=%s\n",mntfname) ;
	    debugprintf("main/procserve: uc_fattach() rs=%d\n",rs) ;
	}
#endif

	if (rs < 0) {
	    if ((! pip->f.quiet) && (pip->efp != NULL))
	        bprintf(pip->efp,"%s: could not perform mount (%d)\n",
	            pip->progname,rs) ;
	    goto ret3 ;
	}

	u_close(cfd) ;
	cfd = -1 ;

	uc_closeonexec(sfd,TRUE) ;

/* ready */

	nfds = 0 ;
	fds[nfds].fd = sfd ;
	fds[nfds].events = (POLLIN | POLLPRI) ;
	nfds += 1 ;
	fds[nfds].fd = -1 ;
	fds[nfds].events = 0 ;

	pto = (to * POLLMULT) ;
	while ((rs >= 0) && (if_exit || if_int)) {

	    rs = u_poll(fds,nfds,pto) ;
	    n = rs ;
	    pip->daytime = time(NULL) ;

	    if ((rs >= 0) && (n > 0)) {

		for (i = 0 ; (rs >= 0) && (i < nfds) ; i += 1) {
		    int	fd = fds[i].fd ;
	            int	re = fds[i].revents ;

		    if (fd == sfd) {

	                if ((re & POLLIN) || (re & POLLPRI)) {
	                    struct strrecvfd	passer ;
		            gid_t	gid ;

	                    rs = acceptpass(sfd,&passer,-1) ;
	                    pfd = rs ;

	                    if (rs >= 0) {
			        nhandle += 1 ;
	                        gid = passer.gid ;
	                        rs = procprint(pip,pfd,gid) ;
	                        u_close(pfd) ;
	                    } /* end if */

	                } else if (re & POLLHUP) {
	                    rs = SR_HANGUP ;
	                } else if (re & POLLERR) {
	                    rs = SR_POLLERR ;
	                } else if (re & POLLNVAL) {
	                    rs = SR_NOTOPEN ;
	                } /* end if (poll returned) */

		    } /* end if */

	        } /* end for */

	    } else if (rs == SR_INTR)
	        rs = SR_OK ;

#ifdef	COMMENT
	    if ((rs >= 0) && (if_exit || if_int))
		rs = SR_INTR ;
#endif

	    if (rs >= 0) {
	        if ((pip->daytime - ti_check) >= to_check) {
		     ti_check = pip->daytime ;
		     rs = procinfo_check(pip) ;
		}
	    }

#ifdef	COMMENT
	    if ((rs >= 0) && ((pip->daytime - ti_wait) >= (to*4))) {
		ti_wait = pip->daytime ;
	        rs1 = u_waitpid(-1,NULL,WNOHANG) ;
	    }
#endif /* COMMENT */

	    if ((rs >= 0) && (pip->intrun > 0) &&
	        ((pip->daytime - ti_run) >= pip->intrun)) {

	        if (pip->efp != NULL)
	            bprintf(pip->efp,"%s: exiting on run-int timeout\n",
	                pip->progname) ;

	        break ;
	    }

	    if ((pip->efp != NULL) && if_int)		/* fun only! */
		bprintf(pip->efp,"%s: interrupt\n",	/* fun only! */
	                pip->progname) ;

	} /* end while */

ret4:
	uc_fdetach(mntfname) ;

ret3:
ret2:
	if (sfd >= 0) {
	    u_close(sfd) ;
	    sfd = -1 ;
	}

	if (cfd >= 0) {
	    u_close(cfd) ;
	    cfd = -1 ;
	}

ret1:
	procinfo_end(pip) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("main/procserve: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (procserve) */


static int proconce(pip,ofname)
PROGINFO	*pip ;
const char	ofname[] ;
{
	int	rs = SR_OK ;


	if (ofname == NULL)
	    return SR_FAULT ;

	if (ofname[0] == '\0')
	    return SR_INVALID ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: msglog=%s\n",pip->progname,ofname) ;

#ifdef	OPTIONAL
	{
	    struct ustat	sb ;
	    rs = u_stat(ofname,&sb) ;
	    if ((rs >= 0) && S_ISDIR(sb.st_mode))
		rs = SR_ISDIR ;
	}
#endif /* OPTIONAL */

/* do the deed */

	if (rs >= 0)
	    rs = procout(pip,ofname) ;

	return rs ;
}
/* end subroutine (proconce) */


/* open the console */
static int procout(pip,ofname)
PROGINFO	*pip ;
const char	ofname[] ;
{
	gid_t	gid = getgid() ;

	int	rs ;
	int	fd ;
	int	oflags = O_FLAGS ;
	int	operms = 0666 ;
	int	wlen = 0 ;


	if (ofname == NULL)
	    return SR_FAULT ;

	if (ofname[0] == '\0')
	    return SR_INVALID ;

	if ((rs = procinfo_begin(pip)) >= 0) {

	    rs = uc_opene(ofname,oflags,operms,pip->to_open) ;
	    fd = rs ;
	    if (rs >= 0) {

	        rs = procprint(pip,fd,gid) ;

	        u_close(fd) ;
	    } /* end if (uc_opene) */

	    procinfo_end(pip) ;
	} /* end if (procinfo) */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procout) */


/* print the time to the console */
static int procprint(pip,fd,gid)
PROGINFO	*pip ;
int		fd ;
gid_t		gid ;
{
	struct ustat	sb ;

	SBUF	b ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	c = 0 ;
	int	wlen = 0 ;
	int	f_terminal = FALSE ;
	int	f_ok = TRUE ;
	int	f ;

	char	linebuf[LINEBUFLEN + 1] ;


	f = ((! pip->have.term) || pip->f.term) ;
	if (f && (! pip->f.daemon)) {
	    if ((rs = u_fstat(fd,&sb)) >= 0) {
	        f_terminal = (S_ISCHR(sb.st_mode)) ? TRUE : FALSE ;
	        if (f_terminal && pip->f.mesg) {
	            f_ok = (sb.st_mode & S_IWGRP) ? 1 : 0 ;
	        }
	    } /* end if (stat) */
	} /* end if (daemon-mode) */

	if (rs < 0)
	    goto ret1 ;
	if (! f_ok)
	    goto ret1 ;

	rs = sbuf_start(&b,linebuf,LINEBUFLEN) ;
	if (rs < 0)
	    goto ret1 ;

	if (f_terminal)
	    sbuf_char(&b,'\r') ;

/* time */

	if (pip->f.o_string && (pip->string != NULL)) {

	    c += 1 ;
	    sbuf_strw(&b,pip->string,-1) ;

	} /* end if (option-string) */

	if (pip->f.o_time) {
	    char	timebuf[TIMEBUFLEN + 1] ;

	    timestr_logz(pip->daytime,timebuf) ;

	    if (c++ > 0) sbuf_char(&b,' ') ;
	    sbuf_strw(&b,timebuf,23) ;

	} /* end if (option-time) */

/* node-name */

	if ((rs >= 0) && pip->f.o_node) {

	    rs = proginfo_nodename(pip) ;

	    if (rs >= 0) {
	        if (c++ > 0) sbuf_char(&b,' ') ;
	        if (pip->f.o_nodetitle) 
	            sbuf_strw(&b,"node=",-1) ;
	        sbuf_strw(&b,pip->nodename,-1) ;
	    }

	} /* end if (option-nodename) */

/* users (number of logged-in users) */

	if ((rs >= 0) && pip->f.o_users) {

	    rs = procinfo_nusers(pip) ;

	    if (rs >= 0) {
	        if (c++ > 0) sbuf_char(&b,' ') ;
	        sbuf_strw(&b,"users=",-1) ;
	        sbuf_decui(&b,pip->nusers) ;
	    }

	} /* end if (option-users) */

/* number of processes */

	if ((rs >= 0) && pip->f.o_procs) {

	    if ((rs = procinfo_nprocs(pip)) >= 0) {
	        if (c++ > 0) sbuf_char(&b,' ') ;
	        sbuf_strw(&b,"procs=",-1) ;
	        sbuf_decui(&b,pip->nprocs) ;
	    }

	} /* end if (option-processes) */

/* memory usage */

#if	defined(_SC_PHYS_PAGES) && defined(_SC_AVPHYS_PAGES)

	if ((rs >= 0) && pip->f.o_mem) {

	    ulong	n100, mu ;

	    long	mt, ma ;

	    uint	percent ;


	    rs1 = uc_sysconf(_SC_PHYS_PAGES,&mt) ;

	    if (rs1 >= 0)
	        rs1 = uc_sysconf(_SC_AVPHYS_PAGES,&ma) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("main/procprint: mt=%lu ma=%lu\n",mt,ma) ;
#endif

	    if ((rs1 >= 0) && (mt > 0)) {

	        mu = (mt - ma) ;
	        n100 = (mu * 100) ;
	        percent = (n100 / mt) ;

	        if (c++ > 0) sbuf_char(&b,' ') ;
	        sbuf_strw(&b,"mem=",-1) ;
	        sbuf_decui(&b,percent) ;
	        sbuf_char(&b,'%') ;

	    } /* end if */

	} /* end if (memory usage) */

#endif /* defined(_SC_PHYS_PAGES) && defined(_SC_AVPHYS_PAGES) */

/* load averages */

	if ((rs >= 0) && pip->f.o_load) {

	    double	dla[3] ;

	    const char	*fmt ;


	    if ((rs1 = uc_getloadavg(dla,3)) >= 0) {

	        fmt = "la=(%4.1f %4.1f %4.1f)" ;
	        if (ndig(dla,3) > 2) {
	            fmt = "la=(%5.1f %5.1f %5.1f)" ;
	            ndigmax(dla,3,3) ;
	        }

	        if (c++ > 0) sbuf_char(&b,' ') ;
	        sbuf_printf(&b,fmt,dla[0],dla[1],dla[2]) ;

	    } /* end if (load-averages) */

	} /* end if (option-loadaverages) */

/* done */

	if (f_terminal && pip->f.ansiterm)
	    sbuf_strw(&b,"\033[K",-1) ;

	if (f_terminal)
	    sbuf_char(&b,'\r') ;

	sbuf_char(&b,'\n') ;

	rs1 = sbuf_finish(&b) ;
	wlen = rs1 ;
	if (rs >= 0) rs = rs1 ;
	if (rs >= 0)
	    rs = u_write(fd,linebuf,wlen) ;

ret1:
ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procprint) */


static int procinfo_begin(pip)
PROGINFO	*pip ;
{


	return SR_OK ;
}
/* end subroutine (procinfo_begin) */


static int procinfo_end(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (pip->open.si) {
	    pip->open.si = FALSE ;
	    rs1 = kinfo_close(&pip->si) ;	/* not thread safe! */
	    if (rs >= 0) rs = rs1 ;
	}

	if (pip->open.ui) {
	    pip->open.ui = FALSE ;
	    rs1 = tmpx_close(&pip->ui) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (procinfo_end) */


static int procinfo_nprocs(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;


	if (! pip->open.si) {
	    rs = kinfo_open(&pip->si,pip->daytime) ; /* not thread safe! */
	    pip->open.si = (rs >= 0) ;
	}

	if (rs >= 0) {
	    if ((pip->daytime - pip->ti_sysmisc) >= TO_SYSMISC) {
	        KINFO_DATA	kd ;
	 	pip->ti_sysmisc = pip->daytime ;
		rs = kinfo_sysmisc(&pip->si,pip->daytime,&kd) ;
	    	pip->nprocs = kd.nproc ;
	    }
	}

	return (rs >= 0) ? pip->nprocs : rs ;
}
/* end subroutine (procinfo_nprocs) */


static int procinfo_nusers(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;


	if (! pip->open.ui) {
	    rs = tmpx_open(&pip->ui,NULL,O_RDONLY) ;
	    pip->open.ui = (rs >= 0) ;
	}

	if (rs >= 0) {
	    if ((pip->daytime - pip->ti_tmpx) >= TO_TMPX) {
	 	pip->ti_tmpx = pip->daytime ;
		rs = tmpx_nusers(&pip->ui) ;
	    	pip->nusers = rs ;
	    }
	}

	return (rs >= 0) ? pip->nusers : rs ;
}
/* end subroutine (procinfo_nusers) */


static int procinfo_check(pip)
PROGINFO	*pip ;
{
	int	rs = SR_OK ;


	if ((rs >= 0) && pip->open.ui)
	    rs = tmpx_check(&pip->ui,pip->daytime) ;

	if ((rs >= 0) && pip->open.si)
	    rs = kinfo_check(&pip->si,pip->daytime) ;

	return rs ;
}
/* end subroutine (procinfo_check) */


static int msglogdev(IDS *idp,const char *fname)
{
	struct ustat	sb ;

	int	rs ;
	int	f = FALSE ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

	rs = u_stat(fname,&sb) ;
	if (rs >= 0) {
	    rs = sperm(idp,&sb,W_OK) ;
	    if (rs >= 0) {
	        f = f || S_ISCHR(sb.st_mode) ;
	        f = f || S_ISFIFO(sb.st_mode) ;
	        f = f || S_ISSOCK(sb.st_mode) ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (msglogdev) */



