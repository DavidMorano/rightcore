/* main (asstrip) */

/* strip the comments from an assembler source file */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1986, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This program will read the input file and strip comments
	from it.  The output is nominally written to STDOUT.

	Synopsis:

	$ asstrip [input_file] [-DV] [-o outfile] [-i input]


*********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define		MAXARGINDEX	100
#define		NARGGROUPS	(MAXARGINDEX/8 + 1)
#define		DEFMAXLINES	66
#define		MAXLINES	180
#define		LINELEN		200
#define		BUFLEN		(MAXPATHLEN + (2 * LINELEN))
#define		DEFPOINT	10


/* external subroutines */

extern int	matstr() ;
extern int	cfdec() ;
extern int	procfile() ;

extern char	*strbasename() ;


/* forward references */

void	helpfile(const char *,bfile *) ;


/* local structures */


/* global data */

struct global		g ;


/* local data */

/* define command option words */

static char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	NULL,
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3
#define	ARGOPT_HELP		4


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;

	struct global	*gp = &g ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	len, lines = 0 ;
	int	rs ;
	int	f_help = FALSE ;
	int	maxlines ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGGROUPS] ;
	char	buf[BUFLEN + 1] ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

	g.efp = efp ;
	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.programroot = NULL ;
	g.helpfile = NULL ;

	g.f.verbose = FALSE ;
	g.f.noblanks = FALSE ;

	f_help = FALSE ;

/* process program arguments */

	rs = SR_OK ;
	for (i = 0 ; i < NARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {
		const int	ach = MKCHAR(argp[1]) ;

	        if (argl > 1) {

	            if (isdigitlatin(ach)) {

	                if (cfdec(argp + 1,argl - 1,&maxlines))
	                    goto badargvalue ;

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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matstr(argopts,akp,akl)) >= 0) {

	                    switch (kwi) {

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.programroot = argp ;

	                        }

	                        break ;

/* debug level */
	                    case ARGOPT_DEBUG:
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if ((avl > 0) &&
	                                (cfdec(avp,avl,
	                                &g.debuglevel) < 0))
	                                goto badargvalue ;

	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        g.f.verbose = TRUE ;
	                        break ;

/* help file */
	                    case ARGOPT_HELP:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.helpfile = avp ;

	                        }

	                        f_help  = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

	                        case 'D':
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (cfdec(avp,avl, &g.debuglevel) != OK)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

	                        case 'b':
	                            g.f.noblanks = TRUE ;
	                            break ;

	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ofname = argp ;

	                            break ;

/* print a brief usage summary (and then exit !) */
	                        case '?':
	                            f_usage = TRUE ;
				    break ;

	                        default:
	                            bprintf(efp,
	                        "%s: invalid key=%t\n",
	                        pip->progname,akp,akl) ;

	                        } /* end switch */

	                        akp += 1 ;
				if (rs < 0)
					break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                maxai = i ;
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif


/* get our program root (if we have one) */

	if (g.programroot == NULL) {

	    if (g.programroot == NULL)
	        g.programroot = getenv(VARPROGRAMROOT1) ;

	    if (g.programroot == NULL)
	        g.programroot = getenv(VARPROGRAMROOT2) ;

	    if (g.programroot == NULL)
	        g.programroot = PROGRAMROOT ;

	} /* end if */


/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto exit ;


	if (f_help) {

	    if (g.helpfile == NULL) {

	        len = bufprintf(buf,BUFLEN,"%s/%s",
	            g.programroot,HELPFILE) ;

	        g.helpfile = (char *) malloc_sbuf(buf,len) ;

	    }

	    helpfile(g.helpfile,g.efp) ;

	    goto exit ;

	}


	if (g.debuglevel > 0)
	    bprintf(efp,"%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;


/* open output file */

	if ((ofname == NULL) || (ofname[0] == '-'))
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	else
	    rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0)
	    goto badoutopen ;


/* perform initialization processing */


/* processing the input file arguments */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if (BATST(argpresent,i)) {

#if	CF_DEBUG
	            if (g.debuglevel > 0) debugprintf(
	                "main: got a positional argument i=%d pan=%d arg=%s\n",
	                i,pan,argv[i]) ;
#endif


	            rs = procfile(&g,argv[i],pan + 1) ;

	            if (rs < 0) {

	                if (g.f.verbose)
	                    bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                        g.progname,argv[i]) ;

	            } else
	                lines += rs ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("main: file lines %d - total lines %d\n",
	                    rs,lines) ;
#endif

	            pan += 1 ;

	        } /* end if (got a positional argument) */

	    } /* end for (loading positional arguments) */

	} else {

	    rs = procfile(&g,"-",pan + 1) ;

	    if (rs < 0) {

	        if (g.f.verbose)
	            bprintf(g.efp,"%s: error processing file \"%s\"\n",
	                g.progname,argv[i]) ;

	    } else
	        lines += rs ;

	    pan += 1 ;

	} /* end if */



/* let's get out of here !! */
done:
	if ((g.debuglevel > 0) || g.f.verbose) {

	    bprintf(efp,"%s: files processed - %d\n",
	        g.progname,pan) ;

	    bprintf(efp,"%s: lines processed - %d\n",
	        g.progname,lines) ;

	}

	bclose(ofp) ;


/* close off and get out ! */
exit:
	bclose(efp) ;

	return OK ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-i input] [-o outfile] [infile [...]]\n",
	    g.progname,g.progname) ;

	bprintf(efp,"\t\t[-DV]\n") ;

	bprintf(efp,
	    "\t-i input    input file\n") ;

	bprintf(efp,
	    "\t-o output   output file\n") ;

	bprintf(efp,
	    "\tinfile      input file\n") ;

	bprintf(efp,
	    "\t-V          program version\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: could not open input file \"%s\" (rs %d)\n",
	    g.progname,ifname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output file \"%s\" (rs %d)\n",
	    g.progname,ofname,rs) ;

	goto badret ;

badret:
	bclose(ofp) ;

	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



void helpfile(f,ofp)
const char	f[] ;
bfile		*ofp ;
{
	bfile	file, *ifp = &file ;

	char	buf[BUFLEN + 1] ;


	if ((f == NULL) || (f[0] == '\0')) return ;

	if (bopen(ifp,f,"r",0666) >= 0) {

	    bcopyblock(ifp,ofp,-1) ;

	    bclose(ifp) ;

	}

}
/* end subroutine (helpfile) */



