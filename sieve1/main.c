/* main */

/* program to return a user's home login directory */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1989-03-01, David A­D­ Morano

	This subroutine was originally written.  This whole program,
	LOGDIR, is needed for use on the Sun CAD machines because Sun
	doesn't support LOGDIR or LOGNAME at this time.  There was a
	previous program but it is lost and not as good as this one
	anyway.  This one handles NIS+ also.  (The previous one
	didn't.) 


	= 1998-06-01, David A­D­ Morano

	I enhanced the program a little to print out some other user
	information besides the user's name and login home directory.


	= 1999-03-01, David A­D­ Morano

	I enhanced the program to also print out effective UID and
	effective GID.


*/


/**************************************************************************

	Synopsis:

	$ sieve maxnum


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	proginfo_setpiv(struct proginfo *,const char *,
			const struct pivars *) ;
extern int	printhelp(bfile *,const char *,const char *,const char *) ;
extern int	sieve(uchar *,uint) ;
extern int	isprime(uint) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"HELP",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_help,
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
	{ 0, 0 }
} ;





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct passwd	*pep ;

	struct group	*gep = NULL ;

	struct proginfo	pi, *pip = &pi ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		nisfile, *nfp = &nisfile ;

	PWENTRY		entry ;

	uid_t	uid_cur ;

	uint	maxnum ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	maxai, pan, npa, i, j, k ;
	int	f_optminus, f_optplus, f_optequal ;
	int	rs, rs1 ;
	int	len, size ;
	int	sl, ci ;
	int	uid ;
	int	ex = EX_INFO ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_self = FALSE ;
	int	f_check = FALSE ;
	int	f_entok = FALSE ;
	int	f_name = FALSE ;
	int	f ;

	uchar	*ba = NULL ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	char	usernamebuf[LOGNAME_MAX + 1] ;
	char	entrybuf[PWENTRY_BUFLEN + 1] ;
	char	ipasswdfname[MAXPATHLEN + 1] ;
	const char	*ofname = NULL ;
	const char	*pwfname = NULL ;
	const char	*pr = NULL ;
	const char	*searchname = NULL ;
	const char	*un = NULL ;
	const char	*cp, *cp2 ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getenv(VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,SHIO_CLINEBUF,0) ;
	}

/* initialize */

	pip->verboselevel = 1 ;

/* start parsing the arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) argpresent[ai] = 0 ;

	au = 0 ;
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

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                            }
	                        }

	                        break ;

			case argopt_help:
				f_help = TRUE ;
				break ;

/* output file name */
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
	                        f_usage = TRUE ;
	                        bprintf(pip->efp,
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

/* version */
	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

				case 'c':
					f_check = TRUE ;
					break ;

				case 'n':
					f_name = TRUE ;
					break ;

/* alternate passwd file */
	                        case 'p':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                pwfname = argp ;

	                            break ;

/* quiet mode */
	                        case 'q':
	                            pip->f.quiet = TRUE ;
	                            break ;

/* verbose mode */
	                        case 'v':
	                            pip->verboselevel = 2 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (avl) {

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
	                            f_usage = TRUE ;
	                            bprintf(pip->efp,
					"%s: unknown option - %c\n",
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

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: debuglevel=%u\n", pip->debuglevel) ;
#endif /* CF_DEBUG */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    usage(pip) ;

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	rs = proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: pr=%s\n",
			pip->progname,pip->pr) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,SEARCHNAME,HELPFNAME) ;

	if (f_version || f_help || f_usage)
		goto retearly ;


	ex = EX_OK ;

/* some initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDIR ;

/* get the first positional argument as the username to key on */

	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i))
	        break ;

	} /* end for */

	if (i > maxai)
		goto badargnum ;

	rs = cfdecui(argv[i],-1,&maxnum) ;

	if (rs < 0)
		goto badarg ;

/* ok, we go */

	size = (maxnum >> 3) + 2 ;
	rs = uc_malloc(size,&ba) ;

	if (rs < 0) {
		ex = EX_OSERR ;
		goto badmem ;
	}


/* do the calculation */

	if (ofname != NULL)
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open the output file (rs %d)\n",
	    pip->progname,rs) ;
	    goto badoutopen ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: done w/ opening output \n") ;
#endif

	rs = sieve(ba,maxnum) ;

	bprintf(ofp,"maxnum=%u found primes=%u\n",maxnum,rs) ;

/* test the sieve */

	if (f_check) {

	for (i = 0 ; i < maxnum ; i += 1) {

		int	f_sieve, f_isprime ;


		f_sieve = BATST(ba,i) ;
		f_isprime = isprime(i) ;

		if (! LEQUIV(f_sieve,f_isprime))
			bprintf(ofp,"error at %u f_sieve=%d f_isprime=%d\n",
				i,f_sieve,f_isprime) ;

	} /* end for */

	} /* end if (check) */

/* print out some primes */

	if (pip->verboselevel > 0) {
		int	c ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: printing primes\n") ;
#endif

		c = 0 ;
		for (i = maxnum ; i > 1 ; i -= 1) {

			if ((i & 1) && BATST(ba,i))
		bprintf(ofp,"prime at %u\n",i) ;

			c += 1 ;
			if (c > 100)
				break ;

		} /* end for */

	}

	bclose(ofp) ;

/* we are done */
badoutopen:
	if (ba != NULL)
	uc_free(ba) ;

badmem:
done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* early return thing */
retearly:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",ex,rs) ;
	        pip->progname,ex,rs) ;

ret2:
	bclose(pip->efp) ;

ret1:
	proginfo_finish(pip) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* the information type thing */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s maxnum \n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,"%s: \t[-?V] [-Dv]\n",
	    pip->progname) ;

	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalilde argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



