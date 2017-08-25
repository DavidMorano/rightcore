/* b_testdir */

/* this is a SHELL built-in test fragment */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_NPRINTF	1
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


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

	= 19999-03-01, David A­D­ Morano
        I enhanced the program to also print out effective UID and effective
        GID.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ testdir


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
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<fsdir.h>
#include	<vecstr.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"kshlib.h"
#include	"testdir_config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(2048,LINE_MAX)
#else
#define	LINEBUFLEN	2048
#endif
#endif /* LINEBUFLEN */

#ifdef	ALIASNAMELEN
#define	ABUFLEN		ALIASNAMELEN
#else
#define	ABUFLEN		64
#endif

#ifdef	MAILADDRLEN
#define	VBUFLEN		MAILADDRLEN
#else
#define	VBUFLEN		2048
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	OUTLINELEN
#define	OUTLINELEN	80
#endif

#define	OUTPADLEN	20
#define	DEBUGFNAME	"/tmp/testdir.deb"


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	vstrcmp(const void *,const void *) ;
extern int	vstrcmpr() ;
extern int	vstrkeycmp(const void *,const void *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */

extern char	**environ ;


/* local structures */


/* forward references */

static void	sighand_int(int) ;

static int	usage(struct proginfo *) ;


/* local variables */

static volatile int	if_int ;

static const int	sigblocks[] = {
	SIGUSR1,
	SIGUSR2,
	SIGHUP,
	SIGTERM,
	SIGQUIT,
	SIGCHLD,
	0
} ;

static const int	sigignores[] = {
	0
} ;

static const int	sigints[] = {
	0
} ;

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"ROOT",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_root,
	argopt_af,
	argopt_of,
	argopt_overlast
} ;

static const char	*suffixes[] = {
	"",
	"*",
	"/",
	"@",
	"=",
	"|",
	"^",
	"?",
	NULL
} ;

enum suffixes {
	suffix_reg,
	suffix_exec,
	suffix_dir,
	suffix_link,
	suffix_sock,
	suffix_fifo,
	suffix_dev,
	suffix_unk,
	suffix_overlast

} ;

static const char	blanks[] = "                    " ;








int b_testdir(argc,argv,contextp)
int	argc ;
char	*argv[] ;
void	*contextp ;
{
	struct proginfo	pi, *pip = &pi ;

	struct sigaction	san ;
	struct sigaction	sao[nelem(sigints) + nelem(sigignores)] ;

	sigset_t	oldsigmask, newsigmask ;

	SHIO	errfile ;
	SHIO	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan, npa ;
	int	rs, rs1, n, size, len, c ;
	int	i, j, k ;
	int	sl, cl, ci ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_timeorder = FALSE ;
	int	f_reverse = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	*pr = NULL ;
	char	*argfname = NULL ;
	char	*outfname = NULL ;
	char	*sp, *cp, *cp2 ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;


	if_int = 0 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

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

	proginfo_start(pip,environ,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRFILE)) == NULL)
		cp = STDERRFNAME ;

	rs1 = shio_open(&errfile,cp,"wca",0666) ;

	if (rs1 >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	}

/* initialize */

	pip->verboselevel = 1 ;
	pip->f.quiet = FALSE ;

/* start parsing the arguments */

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	rs = SR_OK ;
	npa = 0 ;			/* number of positional so far */
	ai_max = 0 ;
	ai = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	            if (isdigit(argp[1])) {

	                if ((argl - 1) > 0)
	                    rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	            } else {

#if	CF_DEBUGS
	                debugprintf("b_testdir: got an option\n") ;
#endif

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                akp = aop ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                    debugprintf("b_testdir: got an option key w/ a value\n") ;
#endif

	                    akl = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + aol - avp ;
	                    f_optequal = TRUE ;

#if	CF_DEBUGS
	                    debugprintf("b_testdir: aol=%d avp=\"%s\" avl=%d\n",
	                        aol,avp,avl) ;

	                    debugprintf("b_testdir: akl=%d akp=\"%s\"\n",
	                        akl,akp) ;
#endif

	                } else {

	                    akl = aol ;
	                    avl = 0 ;

	                }

/* keyword match or only key letters ? */

#if	CF_DEBUGS
	                debugprintf("b_testdir: akp=%t\n",akp,akl) ;
#endif

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

#if	CF_DEBUGS
	                    debugprintf("b_testdir: option keyword=%t kwi=%d\n",
	                        akp,akl,kwi) ;
#endif

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

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

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

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

	                    case argopt_af:

#if	CF_DEBUGS
	                        debugprintf("b_testdir: case=af\n") ;
#endif

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

#if	CF_DEBUGS
	                        debugprintf("b_testdir: argfname=%s\n",argfname) ;
#endif

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
	                        shio_printf(pip->efp,
	                            "%s: option (%s) not supported\n",
	                            pip->progname,akp) ;

	                    } /* end switch */

	                } else {

#if	CF_DEBUGS
	                    debugprintf("b_testdir: got an option key letter\n") ;
#endif

	                    while (akl--) {

#if	CF_DEBUGS
	                        debugprintf("b_testdir: option key letter=%c\n",
	                            *akp) ;
#endif

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

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* all */
	                        case 'a':
	                            pip->f.all = TRUE ;
	                            break ;

/* reverse sort */
	                        case 'r':
	                            f_reverse = TRUE ;
	                            break ;

/* time order */
	                        case 't':
	                            f_timeorder = TRUE ;
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

	        if (ai < MAXARGINDEX) {

	            BASET(argpresent,ai) ;
	            ai_max = ai ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                rs = SR_INVALID ;
	                f_extra = TRUE ;
	                shio_printf(pip->efp,
	                    "%s: extra arguments specified\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0)
		goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(1))
	    debugprintf("b_testdir: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    shio_printf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

/* get the program root */

	if (pr == NULL) {

	    pr = getenv(VARPROGRAMROOT1) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT2) ;

	    if (pr == NULL)
	        pr = getenv(VARPROGRAMROOT3) ;

/* try to see if a path was given at invocation */

	    if ((pr == NULL) && (pip->progdname != NULL))
	        proginfo_rootprogdname(pip) ;

/* do the special thing */

#if	CF_GETEXECNAME && defined(SOLARIS) && (SOLARIS >= 8)
	    if ((pr == NULL) && (pip->pr == NULL)) {

	        const char	*pp ;


	        pp = getexecname() ;

	        if (pp != NULL)
	            proginfo_execname(pip,pp) ;

	    }
#endif /* SOLARIS */

	} /* end if (getting a program root) */

	if (pip->pr == NULL) {

	    if (pr == NULL)
	        pr = PROGRAMROOT ;

	    proginfo_setprogroot(pip,pr,-1) ;

	}


/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,SEARCHNAME) ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_testdir: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    shio_printf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    shio_printf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}


/* help file */

	if (f_help)
	    goto help ;


/* some initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;



	ex = EX_OK ;

	if ((outfname == NULL) || (outfname[0] == '\0'))
		outfname = STDOUTFNAME ;

	    rs = shio_open(ofp,outfname,"wct",0666) ;

	if (rs < 0) {

	    ex = EX_CANTCREAT ;
	    shio_printf(pip->efp,
	        "%s: could not open the output file (%d)\n",
	        pip->progname,rs) ;

	    goto done ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("b_testdir: done w/ opening output \n") ;
#endif

	{
	    FSDIR	dir ;

	    FSDIR_ENT	ds ;

	    int		si, clen, blen, fnl ;

	    char	*fnp ;


	    rs = fsdir_open(&dir,".") ;

	    if (rs >= 0) {

	        while ((rs = fsdir_read(&dir,&ds)) > 0) {

	            fnl = rs ;
	            fnp = ds.name ;

	            if (fnp[0] == '.') {

	                if (fnp[1] == '.') {

	                    if (fnp[2] == '\0')
	                        continue ;

	                } else if (fnp[1] == '\0')
	                    continue ;

	                if (! pip->f.all)
	                    continue ;

	            } /* end if (entries to be ignored) */

#if	CF_NPRINTF
	nprintf(DEBUGFNAME,"b_testdir: fml=%u dn=>%t<\n",
		fnl,fnp,strnlen(fnp,fnl)) ;
	nprintf(DEBUGFNAME,"b_testdir: ino=%u\n",ds.ino) ;
	nprintf(DEBUGFNAME,"b_testdir: off=%u\n",ds.off) ;
	nprintf(DEBUGFNAME,"b_testdir: len=%u\n",ds.reclen) ;
#endif

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
			debugprintf("b_testdir: fnl=%u fnp=>%t<\n",
				fnl,fnp,strnlen(fnp,fnl)) ;
#endif

	            rs = shio_printf(ofp,"file=%s\n",fnp) ;

	            if (rs < 0)
	                break ;

	        } /* end while */

	        fsdir_close(&dir) ;

	    } /* end if (opened directory) */

	} /* end block */

	shio_close(ofp) ;


	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;


/* we are done */
done:
ret2:
	if (pip->debuglevel > 0)
	    shio_printf(pip->efp,"%s: program finishing\n",
	        pip->progname) ;


/* early return thing */
retearly:
ret1:
	shio_close(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

/* restore and get out */

	j = 0 ;
	for (i = 0 ; sigints[i] != 0 ; i += 1)
		u_sigaction(sigints[i],(sao + j++),NULL) ;

	for (i = 0 ; sigignores[i] != 0 ; i += 1)
		u_sigaction(sigignores[i],(sao + j++),NULL) ;

	u_sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the information type thing */
usage:
	usage(pip) ;

	goto retearly ;

/* print out some help */
help:

#if	CF_SFIO
	rs1 = printhelp(sfstdout,pip->pr,pip->searchname,HELPFNAME) ;
#else
	rs1 = printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;
#endif

	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	shio_printf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (b_testdir) */



/* LOCAL SUBROUTINES */



static void sighand_int(sn)
int	sn ;
{


	if_int = TRUE ;
}
/* end subroutine (sighand_int) */


static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = shio_printf(pip->efp,
	    "%s: USAGE> %s [files(s)] [-af argfile] [-of outfile]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = shio_printf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? slen : rs ;
}
/* end subroutine (usage) */



