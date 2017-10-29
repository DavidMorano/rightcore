/* main */

/* part of the 'filesize' program */


#define	F_DEBUG		1
#define	F_DEBUGS	0


/* revision history :

	= Dave Morano, March 1996
	The program was written from scratch to do what
	the previous program by the same name did.

*/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<wdt.h>

#include	"misc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define		MAXARGINDEX	600
#define		MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	optmatch() ;
extern int	wdt() ;
extern int	checkname() ;

extern char	*strbasename(), *strshrink() ;


/* local forward references */


/* external variables */


/* global variables */

struct global		g ;


/* local variables */

static char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL,
} ;

#define	ARGOPT_VERSION		0
#define	ARGOPT_VERBOSE		1
#define	ARGOPT_TMPDIR		2




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	argfile, *afp = &argfile ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	i ;
	int	rs, pan ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	err_fd ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*afname = NULL ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    esetfd(err_fd) ;


	g.progname = strbasename(argv[0]) ;

	g.efp = efp ;
	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* early things to initialize */

	g.ofp = ofp ;
	g.debuglevel = 0 ;
	g.tmpdir = NULL ;
	g.suffix = NULL ;
	g.namelen = MAXNAMELEN ;
	g.suffixlen = -1 ;

	g.bytes = 0 ;
	g.megabytes = 0 ;

	g.f.verbose = FALSE ;
	g.f.nochange = FALSE ;
	g.f.quiet = FALSE ;


/* process program arguments */

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[++i] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 0) && (f_optminus || f_optplus)) {

	        if (argl > 1) {

#if	F_DEBUGS
	            eprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	F_DEBUGS
	                eprintf("main: got an option key w/ a value\n") ;
#endif

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword match or should we assume only key letters ? */

#if	F_DEBUGS
	            eprintf("main: about to check for a key word match\n") ;
#endif

	            if ((kwi = optmatch(argopts,aop,aol)) >= 0) {

#if	F_DEBUGS
	                eprintf("main: got an option keyword, kwi=%d\n",
	                    kwi) ;
#endif

	                switch (kwi) {

/* version */
	                case ARGOPT_VERSION:
#if	F_DEBUGS
	                    eprintf("main: version key-word\n") ;
#endif
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* verbose */
	                case ARGOPT_VERBOSE:
#if	F_DEBUGS
	                    eprintf("main: version key-word\n") ;
#endif
	                    g.f.verbose = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* temporary directory */
	                case ARGOPT_TMPDIR:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) g.tmpdir = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) g.tmpdir = argp ;

	                    }

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

#if	F_DEBUGS
	                eprintf("main: got an option key letter\n") ;
#endif

	                while (aol--) {

#if	F_DEBUGS
	                    eprintf("main: option key letters\n") ;
#endif

	                    switch (*aop) {

	                    case 'V':

#if	F_DEBUGS
	                        eprintf("main: version key-letter\n") ;
#endif
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

#if	F_DEBUGS
	                            eprintf("main: debugequal avl=%d avp=%s\n",
	                                avl,avp) ;
#endif

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl, &g.debuglevel) < 0)
	                                goto badargvalue ;

#if	F_DEBUGS
	                            eprintf("main: debuglevel=%d\n",
	                                g.debuglevel) ;
#endif
	                        }

	                        break ;

/* take input file arguments from STDIN */
	                    case 'f':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                        afname = argp ;

	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl, &g.namelen) < 0)
	                                goto badargvalue ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (cfdec(argp,argl,&g.namelen) < 0)
	                                goto badargvalue ;

	                        }

	                        break ;

/* no-change */
	                    case 'n':
	                        g.f.nochange = TRUE ;
	                        break ;

/* quiet */
	                    case 'q':
	                        g.f.quiet = TRUE ;
	                        break ;

/* require a suffix for file names */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.suffix = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.suffix = argp ;

	                        }

	                        break ;

/* verbose output */
	                    case 'v':
	                        g.f.verbose = TRUE ;
	                        break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            g.progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } /* end switch */

	                    aop += 1 ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

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


/* check arguments */

#if	F_DEBUGS
	eprintf("main: finished parsing command line arguments\n") ;
#endif

	if (g.debuglevel > 0) {

	    bprintf(g.efp,
	        "%s: debuglevel %d\n",
	        g.progname,g.debuglevel) ;

	    bcontrol(g.efp,BC_LINEBUF,0) ;

	    bflush(g.efp) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto done ;


/* check a few more things */

	if (g.tmpdir == NULL)
	    g.tmpdir = getenv("TMPDIR") ;

	if (g.tmpdir == NULL)
	    g.tmpdir = TMPDIR ;


	g.suffixlen = -1 ;
	if (g.suffix != NULL)
	    g.suffixlen = strlen(g.suffix) ;


/* get ready */



/* OK, we do it */

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

	        if ((rs = process(&g,argv[i])) < 0)
	            break ;

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	} /* end if */


/* process any files in the argument filename list file */

	if (afname != NULL) {

#if	F_DEBUG
	    if (g.debuglevel > 1)
	        eprintf("main: we have an argument file list\n") ;
#endif

	    if ((strcmp(afname,"-") == 0) || (afname[0] == '\0'))
	        rs = bopen(afp,BIO_STDIN,"dr",0666) ;

	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	buf[BUFLEN + 1] ;


#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("main: processing the argument file list\n") ;
#endif

	        while ((len = bgetline(afp,buf,BUFLEN)) > 0) {

	            if (buf[len - 1] == '\n') len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            if ((rs = process(&g,cp)) < 0)
	                break ;

	            pan += 1 ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

#if	F_DEBUG
	        if (g.debuglevel > 1)
	            eprintf("main: done processing the argument file list\n") ;
#endif

	    } else {

	        if (! g.f.quiet) {

	            bprintf(efp,
	                "%s: could not open the argument list file\n",
	                g.progname) ;

	            bprintf(efp,"\trs=%d argfile=%s\n",rs,afname) ;

	        }

	    }

	} /* end if (processing file argument file list */


	if (pan == 0)
	    goto badnodirs ;


	if ((rs = bopen(ofp,BIO_STDOUT,"dwct",0644)) < 0)
	    goto badoutopen ;

	bprintf(ofp,"%d megabytes and %d bytes\n",
	    g.megabytes,g.bytes) ;

	{
	    long	blocks, blockbytes ;


	    blocks = g.megabytes * 1024 * 2 ;
	    blocks += (g.bytes / 512) ;
	    blockbytes = (g.bytes % 512) ;

	    bprintf(ofp,"%d UNIX blocks and %d bytes\n",
	        blocks,blockbytes) ;

	}

	bclose(ofp) ;


#if	F_DEBUG
	if (g.debuglevel > 1)
	    eprintf("main: exiting\n") ;
#endif


/* good return from program */
goodret:

#if	F_DEBUG
	if (g.debuglevel > 1)
	    bprintf(g.efp,"%s: program exiting\n",
	        g.progname) ;
#endif


/* we are out of here */
done:
	bclose(g.efp) ;

	return OK ;

/* come here for a bad return from the program */
badret:

#if	F_DEBUGS
	eprintf("main: exiting program BAD\n") ;
#endif

	bclose(g.ofp) ;

	bclose(g.efp) ;

	return BAD ;

/* program usage */
usage:
	bprintf(g.efp,
	    "%s: USAGE> %s [directory(s) ...] [-s suffix] [-Vv]\n",
	    g.progname,g.progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badoutopen:
	bprintf(g.efp,"%s: could not open output (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badinopen:
	bprintf(g.efp,"%s: could not open standard input (rs=%d)\n",
	    g.progname,rs) ;

	goto badret ;

badnodirs:
	bprintf(g.efp,"%s: no files or directories were specified\n",
	    g.progname) ;

	goto badret ;

}
/* end subroutine (main) */



