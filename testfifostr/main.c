/* main */

/* program to test the FIFOSTR object */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	1		/* switchable at invocation */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */
#define	CF_FIFOSTR	1		/* use FIFOSTR */
#define	CF_ASCII	1		/* use only ASCII characters */


/* revision history:

	= 1999-03-01, David A­D­ Morano

	This was adopted from previous code.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Synopsis:

	$ testfifostr.x


*****************************************************************************/


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<vecstr.h>
#include	<fifostr.h>
#include	<randomvar.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LOGNAMELEN
#define	LOGNAMELEN	32
#endif

#ifndef	BUFLEN
#define	BUFLEN		100
#endif

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	NLOOPS		100000
#define	NSTRLEN		70


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	printhelp(bfile *,const char *,const char *,const char *) ;

extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_elapsed(time_t,char *) ;


/* external variables */


/* local structures */

struct getrchar {
	RANDOMVAR	*rp ;
	uint		v ;
	int		i ;
} ;

struct listentry {
	char	*name ;
} ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	getrchar_init(struct getrchar *,RANDOMVAR *) ;
static int	getrchar_char(struct getrchar *) ;
static int	getrchar_free(struct getrchar *) ;


/* local variables */

static const char *argopts[] = {
	    "ROOT",
	    "VERSION",
	    "VERBOSE",
	    "HELP",
	    "af",
	    "of",
	    NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_af,
	argopt_of,
	argopt_overlast
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct listentry	le, *lp ;

	RANDOMVAR	rv ;

	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, rs1, len, n, i, j, k ;
	int	sl, cl ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	nodename[NODENAMELEN + 1] ;
	char	domainname[MAXHOSTNAMELEN + 1] ;
	const  char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*sp, *cp, *cp2 ;

#if	CF_DEBUGS || CF_DEBUG
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    pip->f.errfile = TRUE ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* initialize */

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

	        if (isdigit(argp[1])) {

	            if ((argl - 1) > 0)
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

/* do we have a keyword match or should we assume only key letters ? */

	            if ((kwi = matstr(argopts,akp,akl)) >= 0) {

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

	                        argp = argv[++i] ;
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

/* quiet mode */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
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
	                        bprintf(pip->efp,
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
	    debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

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

#if	CF_GETEXECNAME && defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
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
	    debugprintf("main: pr=%s\n",pip->pr) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: pr=%s\n",
	        pip->progname,pip->pr) ;

	    bprintf(pip->efp,"%s: sn=%s\n",
	        pip->progname,pip->searchname) ;

	}

/* help file */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* some initialization */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* open the output file */

	if (ofname != NULL)
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main: done w/ opening output \n") ;
#endif

	{
	    struct getrchar	gc ;

	    FIFOSTR	fs ;

	    VECSTR	vs ;

	    int	c_fifo = 0 ;
	    int	opts ;

	    char	strbuf[BUFLEN + 1] ;


	    rs = randomvar_start(&rv,0,0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: randomvar_start() rs=%d rv(%p)\n",
	            rs,&rv) ;
#endif

#if	CF_FIFOSTR
	    rs = fifostr_start(&fs) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("main: fifostr_start() rs=%d\n",rs) ;
#endif

#else
	opts = VECSTR_OORDERED ;
	    vecstr_start(&vs,NLOOPS,opts) ;
#endif

	    getrchar_init(&gc,&rv) ;

	    for (i = 0 ; i < NLOOPS ; i += 1) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("main: i=%u\n",i) ;
#endif

/* make a random string of random length */

	        randomvar_getint(&rv,&n) ;

	        n = MIN((n % NSTRLEN),BUFLEN) ;
	        for (j = 0 ; j < n ; j += 1)
	            strbuf[j] = getrchar_char(&gc) ;

	        strbuf[j] = '\0' ;

	        c_fifo += 1 ;

#if	CF_FIFOSTR
	        fifostr_add(&fs,strbuf,n) ;
#else
	        vecstr_add(&vs,strbuf,n) ;
#endif

	        if ((c_fifo > 100) && (n & 1)) {
	            c_fifo -= 1 ;

#if	CF_FIFOSTR
	            fifostr_remove(&fs,strbuf,BUFLEN) ;
#else
	            vecstr_del(&vs,0) ;
#endif

	        }

	        if (c_fifo > 1000) {

	            c_fifo -= 50 ;
	            for (k = 0 ; k < 50 ; k += 1) {

#if	CF_FIFOSTR
	                fifostr_remove(&fs,strbuf,BUFLEN) ;
#else
	                vecstr_del(&vs,0) ;
#endif

	            }

	        }

	    } /* end for (looping) */

#if	CF_FIFOSTR
	{
		FIFOSTR_CUR	cur ;


		fifostr_curbegin(&fs,&cur) ;

	    for (j = 0 ; j < 10 ; j += 1) {

	        rs1 = fifostr_enum(&fs,&cur,strbuf,BUFLEN) ;

		len = rs1 ;
		if (rs1 < 0)
			break ;

		rs = bprintf(ofp,"%t\n",strbuf,len) ;

		if (rs < 0)
		    break ;

	    } /* end for */

		fifostr_curend(&fs,&cur) ;

	} /* end block */
#endif /* CF_FIFOSTR */

	    for (k = 0 ; k < c_fifo ; k += 1) {

#if	CF_FIFOSTR
	        fifostr_remove(&fs,strbuf,BUFLEN) ;
#else
	        vecstr_del(&vs,0) ;
#endif /* CF_FIFOSTR */

	    }

	    getrchar_free(&gc) ;

#if	CF_FIFOSTR
	    fifostr_finish(&fs) ;
#else
	    vecstr_finish(&vs) ;
#endif /* CF_FIFOSTR */

	    randomvar_finish(&rv) ;

	} /* end block */

/* we are done */
done:
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

	bclose(ofp) ;

/* early return thing */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* the information type thing */
usage:
	usage(pip) ;

	goto retearly ;

/* print out some help */
help:
	ex = EX_INFO ;
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* the bad things */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto ret1 ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open the output file (%d)\n",
	    pip->progname,rs) ;

	goto badret ;

/* error types of returns */
badret:
	bclose(pip->efp) ;

	goto retearly ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s \n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* get a random character object */
static int getrchar_init(op,rp)
struct getrchar	*op ;
RANDOMVAR	*rp ;
{


	memset(op,0,sizeof(struct getrchar)) ;

	op->rp = rp ;
	return 0 ;
}

static int getrchar_char(op)
struct getrchar	*op ;
{
	uint	c ;


#if	CF_DEBUGS
	debugprintf("main/getrchar: rp(%p)\n",
	    op->rp) ;
#endif

again:
	if (op->i == 0)
	    randomvar_getuint(op->rp,&op->v) ;

	c = op->v & 255 ; 
	op->v = op->v >> 8 ;
	op->i = (op->i + 1) & 3 ;

	if (isspace(c))
		goto again ;

#if	CF_ASCII
	if (! (isalpha(c) || isprint(c)))
	    goto again ;
#else
	if (! (isalpha(c) || isprintlatin(c)))
	    goto again ;
#endif /* CF_ASCII */

	return c ;
}

static int getrchar_free(op)
struct getrchar	*op ;
{


	return 0 ;
}



