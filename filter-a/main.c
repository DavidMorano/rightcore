/* main */

/* fitler some text */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1987-09-01, David A­D­ Morano

	This subroutine was originally written but it was probably started
	from any one of the numerous subroutine which perform a similar
	"file-processing" fron end.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will read the input file and filter into a file
	with one email address per line.

	Synopsis:

	$0 [input_file [...]] [-DV] [-o offset]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	100
#define	NARGGROUPS	(MAXARGINDEX/8 + 1)
#define	DEFMAXLINES	66
#define	MAXLINES	180
#define	LINELEN		200
#define	DEFPOINT	10

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + (2 * LINELEN))
#endif



/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	procfile(struct proginfo *,bfile *,const char *,int) ;
extern int	isdigitlatin(int) ;

extern char	*strbasename(char *) ;


/* forward references */

static int	usage(struct proginfo *) ;
static int	helpfile(const char *,bfile *) ;


/* local structures */


/* global variables */


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	NULL
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4







int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	lines = 0 ;
	int	rs, rs1, i, cl ;
	int	maxlines = DEFMAXLINES ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	const char	*pr = NULL ;
	const char	*pmspec = NULL ;
	const char	*searchname = NULL ;
	const char	*helpfname = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	memset(pip,0,sizeof(struct proginfo)) ;
	pip->verboselevel = 1 ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(&errfile,BFILE_STDERR,"wca",0666) < 0)
	    bcontrol(&errfile,BC_LINEBUF,0) ;


	pip->efp = &errfile ;
	pip->debuglevel = 0 ;


/* process program arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < NARGGROUPS ; ai += 1) 
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

	            if ((argl - 1) > 0)
	                rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

	            ai_pos = ai ;
	            break ;

	        } else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    aol = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    f_optequal = TRUE ;

	                } else
	                    avl = 0 ;

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* program root */
	                    case ARGOPT_ROOT:
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

/* debug level */
	                    case ARGOPT_DEBUG:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                &pip->debuglevel) ;

	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        pip->verboselevel = 2 ;
	                        break ;

/* help file */
	                    case ARGOPT_HELP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					helpfname = NULL ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* output file */
	                        case 'w':
	                            if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				    }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					ofname = argp ;

	                            break ;

	                        default:
				    rs = SR_INVALID ;
	                            bprintf(pip->efp,
					"%s : unknown option - %c\n",
	                                pip->progname,*aop) ;

				break ;

/* fall through to the next case */

/* print a brief usage summary (and then exit !) */
	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
					break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

		} /* end if (digits or whatever) */

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
	    bprintf(pip->efp,"%s: debuglevel %u\n",
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

	if (searchname == NULL)
	    searchname = getenv(VARSEARCHNAME) ;

#ifdef	COMMENT
	if ((searchname == NULL) && (pmspec != NULL)) {

	    searchname = pmspec ;

	}
#endif /* COMMENT */

	if (searchname == NULL) {

	    searchname = pip->progname ;
	    if ((cp = strchr(pip->progname,'.')) != NULL) {

	        searchname = tmpfname ;
	        strwcpy(tmpfname,pip->progname,(cp - pip->progname)) ;

	    }
	}

	proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

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

/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;

	if (f_help) {

	    if (helpfname == NULL) {

		helpfname = tmpfname ;
		mkpath2(tmpfname, pip->pr,HELPFNAME) ;

	    }

	    helpfile(helpfname,pip->efp) ;

	    goto retearly ;

	}


/* open output file */

	if ((ofname != NULL) && (ofname[0] != '-'))
	    rs = bopen(ofp,ofname,"wct",0666) ;

	else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0)
	    goto badoutopen ;

/* processing the input file arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 0) 
		debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

		pan += 1 ;
	            rs = procfile(pip,ofp,argv[ai],(pan + 1)) ;

	            if (rs < 0) {

	                if (pip->verboselevel > 1)
	                    bprintf(pip->efp,
				"%s: error processing file \"%s\"\n",
	                        pip->progname,argv[ai]) ;

	            } else
	                lines += rs ;

	    } /* end for (loading positional arguments) */

	if ((rs >= 0) && (pan == 0)) {

	    pan += 1 ;
	    rs = procfile(pip,ofp,"-",(pan + 1)) ;

	    if (rs < 0) {

	        if (pip->verboselevel > 1)
	            bprintf(pip->efp,
			"%s: error processing file \"%s\"\n",
	                pip->progname,argv[ai]) ;

	    } else
	        lines += rs ;

	} /* end if */

/* close the output file */

	bclose(ofp) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* let's get out of here !! */
done:
	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: files processed %u\n",
	        pip->progname,pan) ;

	}

/* close off and get out ! */
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

/* what are we about? */
usage:
	usage(pip) ;

	goto retearly ;

help:
	printhelp(NULL,pip->pr,SEARCHNAME,HELPFNAME) ;

	goto retearly ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (%d)\n",
	    pip->progname,rs) ;

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
	    "%s: USAGE> %s [-w outfile] [infile [...]]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,"\t\t[-DV]\n") ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "\tinfile      input file\n") ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "\t-w outfile  output file\n") ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "\t-D          debugging flag\n") ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "\t-V          program version\n") ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


static int helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;

	int	rs ;
	int	wlen ;

	char	buf[BUFLEN + 1] ;


	if ((f == NULL) || (f[0] == '\0')) 
		return SR_FAULT ;

	wlen = 0 ;
	if ((rs = bopen(ifp,f,"r",0666)) >= 0) {

	    rs = bcopyblock(ifp,ofp,-1) ;

	    wlen += rs ;
	    bclose(ifp) ;

	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (helpfile) */



