/* b_helloworlder */

/* SHELL built-in: print something to STDOUT */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_SHIO		1		/* allow for SHIO? */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	Synopsis:

	$ helloworlder <text>


*****************************************************************************/


#undef	CF_SFIO
#if	defined(SFIO) || (defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	CF_SFIO
#include	<shell.h>
#endif

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<pwd.h>

#if	CF_SFIO
#else
#include	<stdio.h>
#endif

#include	<vsystem.h>
#include	<baops.h>
#include	<getxusername.h>
#include	<getax.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"helloworlder_config.h"
#include	"defs.h"

#if	CF_SHIO
#include	"shio.h"
#endif


/* local defines */

#ifndef	LOGNAMELEN
#ifdef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	USERNAMELEN
#ifdef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	VARUSERNAME
#define	VARUSERNAME	"USERNAME"
#endif

#ifndef	VARLOGNAME
#define	VARLOGNAME	"LOGNAME"
#endif

#ifndef	VARUSER
#define	VARUSER		"USER"
#endif


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdecti(const char *,int,int *) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

#if	(! CF_SFIO)
extern char	**environ ;
#endif


/* local structures */

struct locinfo_flags {
	uint		nouser : 1 ;
} ;

struct locinfo {
	struct locinfo_flags	f ;
} ;


/* forward references */

static void	sighand_int(int) ;

static int	usage(struct proginfo *) ;

static int	procname(struct proginfo *,FILE *,const char *) ;
static int	getname(struct proginfo *,struct passwd *,char *,int,
			const char *) ;


/* local variables */

static volatile int	if_exit ;
static volatile int	if_int ;

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
#if	defined(SIGXFSZ)
	SIGXFSZ,
#endif
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
	"pm",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_pm,
	argopt_af,
	argopt_of,
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
	"helloworld",
	"username",
	"userdir",
	"logdir",
	"userhome",
	NULL
} ;

enum progmodes {
	progmode_helloworld,
	progmode_username,
	progmode_userdir,
	progmode_logdir,
	progmode_userhome,
	progmode_overlast
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


int b_helloworlder(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	struct proginfo	pi, *pip = &pi ;

	struct locinfo	li, *lip = &li ;

	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;

	FILE		*ofp = stdout ;

	sigset_t	oldsigmask, newsigmask ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs = SR_OK ;
	int	pmlen = -1 ;
	int	progmode = progmode_username ;
	int	n, i, j ;
	int	size, v ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	char	usernamebuf[USERNAMELEN + 1] ;
	const char	*pr = NULL ;
	const char	*pm = NULL ;
	const char	*sn = NULL ;
	const char	*argfname = NULL ;
	const char	*outfname = NULL ;
	const char	*tp, *cp ;


	if_int = 0 ;
	if_exit = 0 ;

	n = nelem(sigints) + nelem(sigignores) ;
	size = n * sizeof(struct sigaction) ;
	memset(sao,0,size) ;

/* block some signals and catch the others */

	uc_sigsetempty(&newsigmask) ;

	for (i = 0 ; sigblocks[i] != 0 ; i += 1)
	    uc_sigsetadd(&newsigmask,sigblocks[i]) ;

	u_sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;

/* ignore these signals */

	uc_sigsetempty(&newsigmask) ;

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

	if ((cp = getenv(VARBANNER)) == NULL)
	    cp = BANNER ;

	proginfo_setbanner(pip,cp) ;

	pip->efp = stderr ;
	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    pip->f.errfile = TRUE ;
	    pip->efp = fopen(cp,"w") ;
	}

	pip->lip = lip ;
	memset(lip,0,sizeof(struct locinfo)) ;

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	rs = SR_OK ;
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

		int ch = (argp[1] & 0xff) ;
	        if (isdigit(ch)) {

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
	                            rs = cfdeci(avp,avl,&v) ;
	                            pip->verboselevel = v ;
				}
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
	                            pm = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            pm = argp ;
	                    }
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

/* handle all keyword defaults */
	                default:
	                    rs = SR_INVALID ;
	                    fprintf(pip->efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                } /* end switch */

	            } else {

	                while (akl--) {

			    int	kc = (*akp & 0xff) ;
	                    switch (kc) {

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

/* debug */
	                    case 'D':
	                        pip->debuglevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                pip->debuglevel = v ;
				    }
	                        }
	                        break ;

/* quiet mode */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* version */
	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

			    case 'q':
	                        pip->verboselevel = 0 ;
				break ;

/* verbose mode */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = cfdeci(avp,avl,&v) ;
	                                pip->verboselevel = v ;
				    }
	                        }
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        fprintf(pip->efp,
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

	if (rs < 0)
	    goto badarg ;

	if (pip->debuglevel > 0)
	    fprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    fprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* figure out a program mode */

	if (pm == NULL)
	    pm = pip->progname ;

	progmode = matostr(progmodes,1,pm,-1) ;

	if (progmode < 0)
	    progmode = 0 ;

	pip->progmode = progmode ;
	sn = (char *) progmodes[progmode] ;

	if (pip->debuglevel > 0)
	    fprintf(pip->efp,"%s: pm=%s(%u)\n",
		pip->progname,progmodes[progmode],progmode) ;

/* set program-root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {

	    fprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    fprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	} /* end if */

/* help file */

	if (f_help) {

#if	CF_SFIO
	    printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	    printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif

	} /* end if */

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* some preliminary initialization */

	usernamebuf[0] = '\0' ;
	pip->username = usernamebuf ;

/* OK, we finally do our thing */

	if ((outfname != NULL) && (outfname[0] != '\0')) {
	    pip->f.outfile = TRUE ;
	    ofp = fopen(outfname,"w") ;
	}

	if (ofp == NULL) {
	    rs = SR_NOTOPEN ;
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

/* go through the loops */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procname(pip,ofp,cp) ;
	    if (rs < 0)
	        break ;

	} /* end for (handling positional arguments) */

#if	CF_SHIO
	if ((rs >= 0) && (argfname != NULL) && (argfname[0] != '\0')) {

	    SHIO	argfile, *afp = &argfile ;


	    if (strcmp(argfname,"-") == 0)
	        argfname = STDINFNAME ;

	    rs = shio_open(afp,argfname,"r",0666) ;

	    if (rs >= 0) {

	        FIELD	fsb ;

		int	len ;
	        int	ml ;
	        int	fl ;

	        const char	*fp ;

	        char	linebuf[LINEBUFLEN + 1] ;
	        char	name[MAXNAMELEN + 1] ;


	        while ((rs = shio_readline(afp,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
	            if ((rs = field_start(&fsb,linebuf,len)) >= 0) {

	                while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {

	                    if (fl == 0)
	                        continue ;

	                    ml = MIN(fl,MAXNAMELEN) ;
	                    strwcpy(name,fp,ml) ;

	                    pan += 1 ;
	    		    rs = procname(pip,ofp,name) ;
	                    if (rs < 0)
	                        break ;

	                    if (fsb.term == '#')
	                        break ;

	                    if (if_exit)
	                        break ;

	                } /* end while */

	                field_finish(&fsb) ;

	            } /* end if (field) */

		    if (rs < 0)
			break ;

	            if (if_exit)
	                break ;

	        } /* end while (reading lines) */

	        shio_close(afp) ;

	    } else {

	        if (! pip->f.quiet) {

	            fprintf(pip->efp,
	                "%s: could not open argument list file\n",
	                pip->progname) ;

	            fprintf(pip->efp,"%s: rs=%d argfile=%s\n",
	                pip->progname,rs,argfname) ;

	        }

	    } /* end if */

	} /* end if (processing file argument file list) */
#endif /* CF_SHIO */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procname(pip,ofp,cp) ;

	} /* end if (default) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("b_helloworlder: f_outfile=%u\n",pip->f.outfile) ;
#endif

	if (pip->f.outfile) {
	    fclose(ofp) ;
	} else
	    fflush(ofp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_helloworlder: ex=%u rs=%d nouser=%u\n",
		ex,rs,lip->f.nouser) ;
#endif

done:
badoutopen:
	if ((rs < 0) && (ex == EX_OK)) {

	    switch (rs) {

	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;

	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;

	    case SR_SRCH:
		ex = EX_NOUSER ;
		break ;

	    default:
	        ex = EX_DATAERR ;
	        break ;

	    } /* end switch */

	} /* end if */

retearly:
	if (pip->debuglevel > 0)
	    fprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("b_helloworlder: exiting ex=%u (%d)\n",ex,rs) ;
#endif

/* early return thing */
ret2:
	if (pip->f.errfile) {
	    fclose(pip->efp) ;
	} else
	    fflush(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

/* restore and get out */
ret0:
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
	fprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (b_helloworlder) */


/* local subroutines */


static void sighand_int(sn)
int	sn ;
{


	if_exit = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen = 0 ;


	rs = fprintf(pip->efp,
	    "%s: USAGE> %s [<text>] \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = fprintf(pip->efp,
	    "%s:  [-Q] [-D] [-v[=<n>]] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process a name */
static int procname(pip,ofp,name)
struct proginfo	*pip ;
FILE		*ofp ;
const char	name[] ;
{
	struct locinfo	*lip = pip->lip ;

	int	rs = SR_OK ;
	int	wlen = 0 ;


	if (name == NULL)
	    return SR_FAULT ;

	if (name[0] == '\0')
	    goto ret0 ;

	if (name[0] == '-')
	    name = "hello world!" ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_helloworlder/procname: name=>%s<\n",name) ;
#endif

	if ((rs >= 0) && (pip->verboselevel > 0)) {
	        rs = fprintf(ofp,"%s\n",name) ;
	        wlen += rs ;
	} /* end if (printing) */

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procname) */



