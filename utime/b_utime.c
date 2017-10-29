/* b_utime */

/* SHELL built-in to return load averages */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable at invocation */


/* revision history:

	= 1989-03-01, David A­D­ Morano
        This subroutine was originally written. This whole program, LOGDIR, is
        needed for use on the Sun CAD machines because Sun doesn't support
        LOGDIR or LOGNAME at this time. There was a previous program but it is
        lost and not as good as this one anyway. This one handles NIS+ also.
        (The previous one didn't.)

	= 1998-06-01, David A­D­ Morano
        I enhanced the program a little to print out some other user information
        besides the user's name and login home directory.

	= 1999-03-01, David A­D­ Morano
        I enhanced the program to also print out effective UID and effective
        GID.

	= 2003-10-01, David A­D­ Morano
	This is now a built-in command for the KSH shell.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ utime spec(s)


*******************************************************************************/


#include	<envstandards.h>	/* must be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<sys/loadavg.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<utmpx.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<tmpx.h>
#include	<kinfo.h>
#include	<exitcodes.h>
#include	<mallocstuff.h>
#include	<field.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"b_utime.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,const char *,
			const struct pivars *) ;
extern int	nusers(const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct locinfo_flags {
	uint		fla:1 ;
	uint		nprocs:1 ;
	uint		nusers:1 ;
} ;

struct locinfo {
	PROGINFO	*pip ;
	const char	*utfname ;
	double		fla[3] ;
	uint		nprocs ;
	uint		nusers ;
	struct locinfo_flags	f ;
} ;


/* forward references */

static void	sighand_int(int) ;

static int	usage(PROGINFO *) ;

static int	locinfo_start(PROGINFO *,struct locinfo *,const char *) ;
static int	locinfo_finish(PROGINFO *) ;

static int	procspec(PROGINFO *,void *, const char *) ;

static int	getla(PROGINFO *) ;
static int	getnusers(PROGINFO *) ;
static int	getnprocs(PROGINFO *) ;
static int	nprocs(PROGINFO *) ;


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
	"HELP",
	"af",
	"of",
	"utf",
	"db",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_af,
	argopt_of,
	argopt_utf,
	argopt_db,
	argopt_overlast
} ;

/* define the configuration keywords */
static const char *reqopts[] = {
	"la1min",
	"la5min",
	"la15min",
	"nusers",
	"nprocs",
	NULL
} ;

enum reqopts {
	reqopt_la1min,
	reqopt_la5min,
	reqopt_la15min,
	reqopt_nusers,
	reqopt_nprocs,
	qopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
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


int b_utime(argc,argv,contextp)
int		argc ;
const char	*argv[] ;
void		*contextp ;
{
	PROGINFO	pi, *pip = &pi ;
	struct locinfo	li, *lip = &li ;

	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;

	sigset_t	oldsigmask, newsigmask ;

	SHIO	errfile ;
	SHIO	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1 ;
	int	n, size, len ;
	int	i, j ;
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
	const char	*sn = NULL ;
	const char	*argfname = NULL ;
	const char	*outfname = NULL ;
	const char	*utfname = NULL ;
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
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto ret0 ;
	}

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) == NULL) cp = STDERRFNAME ;
	rs1 = shio_open(&errfile,cp,"wca",0666) ;
	if (rs1 >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    shio_control(&errfile,SHIO_CLINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

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

	        if (isdigit(argp[1])) {

		    rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	            aop = argp + 1 ;
	            aol = argl - 1 ;
	            akp = aop ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + aol - avp ;
	                f_optequal = TRUE ;

	            } else {

	                akl = aol ;
	                avl = 0 ;

	            }

/* keyword match or only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program-root */
	                    case argopt_root:
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

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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
	                            argfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            argfname = argp ;

	                    }

	                    break ;

/* output file name */
	                case argopt_of:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            outfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            outfname = argp ;

	                    }

	                    break ;

/* UTMP file */
	                case argopt_utf:
	                case argopt_db:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            utfname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            utfname = argp ;

	                    }

	                    break ;

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    shio_printf(pip->efp,
	                        "%s: option (%s) not supported\n",
	                        pip->progname,akp) ;

	                } /* end switch */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

/* debug */
	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->debuglevel) ;

	                            }
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

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                            }
	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        shio_printf(pip->efp,
	                            "%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

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

	if (rs < 0)
	    goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_utime: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;

	if (pip->debuglevel > 0) {
	    shio_printf(pip->efp,"%s: pr=%s\n", pip->progname,pip->pr) ;
	    shio_printf(pip->efp,"%s: sn=%s\n", pip->progname,pip->searchname) ;
	}

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help) {
#if	CF_SFIO
	printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif
	}

	if (f_help || f_usage || f_version)
	    goto retearly ;


	ex = EX_OK ;

/* OK, we finally do our thing */

	locinfo_start(pip,lip,utfname) ;

	if ((outfname == NULL) || (outfname[0] == '\0'))
	    outfname = STDOUTFNAME ;

	rs = shio_open(ofp,outfname,"wct",0666) ;

	if (rs < 0)
	    goto ret3 ;

/* go through the loops */

	pan = 0 ;

	for (ai = 1 ; ai <= ai_max ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procspec(pip,ofp,cp) ;

	    if (rs < 0)
	        break ;

	    if (if_int || if_exit)
		break ;

	} /* end for (handling positional arguments) */

	if ((rs >= 0) && (argfname != NULL) && (argfname[0] != '\0')) {
	    SHIO	afile, *afp = &afile ;

	    if (strcmp(argfname,"-") == 0)
	        argfname = STDINFNAME ;

	    rs = shio_open(afp,argfname,"r",0666) ;

	    if (rs >= 0) {

	        FIELD	fsb ;

	        int	ml ;
		int	fl ;

	        char	linebuf[LINEBUFLEN + 1] ;
	        char	name[MAXNAMELEN + 1] ;
		const char	*fp ;

	        while ((rs = shio_readline(afp,linebuf,LINEBUFLEN)) > 0) {
	            len = rs ;

	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
	            if ((rs = field_start(&fsb,linebuf,len)) >= 0) {

	                while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	                    if (fl == 0) continue ;

	                    ml = MIN(fl,MAXNAMELEN) ;
	                    strwcpy(name,fp,ml) ;

	                    pan += 1 ;
	                    rs = procspec(pip,ofp,name) ;

	                    if (fsb.term == '#') break ;
	    		    if (if_int) break ;
	                    if (rs < 0) break ;
	                } /* end while */

	                field_finish(&fsb) ;
	            } /* end if (field) */

		    if (if_int) break ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        shio_close(afp) ;
	    } else {
	        if (! pip->f.quiet) {
	            shio_printf(pip->efp,
	                "%s: could not open argument list file\n",
	                pip->progname) ;
	            shio_printf(pip->efp,"%s: \trs=%d afile=%s\n",
	                pip->progname,rs,argfname) ;
	        }
	    } /* end if */

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    rs = SR_INVALID ;
	    shio_printf(pip->efp,"%s: no specifications given\n",
	        pip->progname) ;

	}

	if (rs >= 0)
	    shio_putc(ofp,'\n') ;

	shio_close(ofp) ;

/* finish */
done:
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
	        ex = EX_DATAERR ;
	        break ;
	    } /* end switch */
	} /* end if */

	locinfo_finish(pip) ;

/* early return thing */
retearly:
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

ret2:
	shio_close(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

/* restore and get out */

	j = 0 ;
	for (i = 0 ; sigignores[i] != 0 ; i += 1)
	    u_sigaction(sigignores[i],(sao + j++),NULL) ;

	for (i = 0 ; sigints[i] != 0 ; i += 1)
	    u_sigaction(sigints[i],(sao + j++),NULL) ;

	u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;
	usage(pip) ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{


	if_exit = TRUE ;
	if_int = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	i ;
	int	wlen = 0 ;


	rs = shio_printf(pip->efp,
	    "%s: USAGE> %s [<reqopts(s)> ...] [-af <afile>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,
	    "%s:  [-utf <utmp>] [-Q] [-D] [-v[=n]] [-HELP] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,"%s:   possible request options are: \n",
	    pip->progname) ;

	wlen += rs ;
	for (i = 0 ; reqopts[i] != NULL ; i += 1) {

	    if ((i % USAGECOLS) == 0) {
	        rs = shio_printf(pip->efp,"%s: \t",pip->progname) ;
	        wlen += rs ;
	    }

	    rs = shio_printf(pip->efp,"%-16s",reqopts[i]) ;
	    wlen += rs ;
	    if ((i % USAGECOLS) == 3) {
	        rs = shio_printf(pip->efp,"\n") ;
	        wlen += rs ;
	    }

	} /* end for */

	if ((i % USAGECOLS) != 0) {
	    rs = shio_printf(pip->efp,"\n") ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process a specification name */
static int procspec(pip,ofp,name)
PROGINFO	*pip ;
void		*ofp ;
const char	name[] ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	ri ;
	int	wlen = 0 ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_utime/locinfo: name=%s\n",name) ;
#endif

	if ((ri = matostr(reqopts,2,name,-1)) >= 0) {
	    switch (ri) {

	    case reqopt_la1min:
	    case reqopt_la5min:
	    case reqopt_la15min:
	        {
	            double	v ;

	            rs = getla(pip) ;
	            v = -1.0 ;
	            switch (ri) {

	            case reqopt_la1min:
	                v = lip->fla[LOADAVG_1MIN] ;

	                break ;

	            case reqopt_la5min:
	                v = lip->fla[LOADAVG_5MIN] ;

	                break ;

	            case reqopt_la15min:
	                v = lip->fla[LOADAVG_15MIN] ;

	                break ;

	            } /* end switch */

	            if ((rs >= 0) && (v > -0.5))
	                rs = shio_printf(ofp," %7.3f",v) ;

	        } /* end block */

	        break ;

	    case reqopt_nusers:
	        rs = getnusers(pip) ;

	        if (rs >= 0)
	            rs = shio_printf(ofp," %3u",rs) ;

	        break ;

	    case reqopt_nprocs:
	        if ((rs = getnprocs(pip)) >= 0)
	            rs = shio_printf(ofp," %3u",rs) ;
	        break ;

	    default:
	        rs = SR_INVALID ;

	    } /* end switch */
	    wlen += rs ;
	} else {
	    rs = shio_printf(ofp," *") ;
	    wlen += rs ;
	} /* end if */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procspec) */


static int locinfo_start(pip,lip,utfname)
PROGINFO	*pip ;
struct locinfo	*lip ;
const char	*utfname ;
{


	pip->lip = lip ;
	memset(lip,0,sizeof(struct locinfo)) ;
	lip->pip = pip ;

	lip->utfname = (char *) utfname ;
	return 0 ;
}


static int locinfo_finish(pip)
PROGINFO	*pip ;
{


	pip->lip = NULL ;
	return 0 ;
}


static int getla(pip)
PROGINFO	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;


	if (! lip->f.fla) {

	    lip->f.fla = TRUE ;
	    rs = uc_getloadavg(lip->fla,3) ;

	}

	return rs ;
}
/* end subroutine (getla) */


static int getnusers(pip)
PROGINFO	*pip ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;


	if (! lip->f.nusers) {

	    lip->f.nusers = TRUE ;
	    rs = nusers(lip->utfname) ;

	    lip->nusers = rs ;
	}

	return (rs >= 0) ? lip->nusers : rs ;
}
/* end subroutine (getnusers) */


static int getnprocs(PROGINFO *pip)
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;

	if (! lip->f.nprocs) {
	    lip->f.nprocs = TRUE ;
	    rs = uc_nprocs(0) ;
	    lip->nprocs = rs ;
	}

	return (rs >= 0) ? lip->nprocs : rs ;
}
/* end subroutine (getnprocs) */


