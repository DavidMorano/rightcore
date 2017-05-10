/* main */

/* part of the MKREADABLE program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1996-03-01, David A­D­ Morano, March 1996

	The program was written from scratch to do what
	the previous program by the same name did.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */


#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<wdt.h>
#include	<paramopt.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	wdt() ;
extern int	checkname() ;

extern char	*strbasename(), *strshrink() ;


/* forward references */


/* external variables */


/* global variables */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"option",
	NULL,
} ;

#define	ARGOPT_VERSION		0
#define	ARGOPT_VERBOSE		1
#define	ARGOPT_TMPDIR		2
#define	ARGOPT_OPTION		3







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	PARAMOPT	aparams ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	argfile, *afp = &argfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, i ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f_version = FALSE ;
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*pr = NULL ;
	char	*searchname = NULL ;
	char	*afname = NULL ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    debugsetfd(fd_debug) ;


	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;


	pip->efp = efp ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;


/* early things to initialize */

	pip->ofp = ofp ;
	pip->debuglevel = 0 ;
	pip->tmpdname = NULL ;
	pip->namelen = MAXNAMELEN ;

	pip->bytes = 0 ;
	pip->megabytes = 0 ;

	pip->f.verbose = FALSE ;
	pip->f.nochange = FALSE ;
	pip->f.quiet = FALSE ;

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) 
		argpresent[i] = 0 ;

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

	        if (argl > 1) {

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                aol = akl ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* version */
	                case ARGOPT_VERSION:
	                    f_version = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* verbose */
	                case ARGOPT_VERBOSE:
#if	CF_DEBUGS
	                    debugprintf("main: version key-word\n") ;
#endif
	                    pip->f.verbose = TRUE ;
	                    if (f_optequal) goto badargextra ;

	                    break ;

/* temporary directory */
	                case ARGOPT_TMPDIR:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl) pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) pip->tmpdname = argp ;

	                    }

	                    break ;

/* the user specified some options */
	                case ARGOPT_OPTION:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                                paramopt_loadu(&aparams,avp,avl) ;

	                    } else {

	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                                paramopt_loadu(&aparams,argp,argl) ;

	                    }

	                    break ;


/* default action and user specified help */
	                default:
			    rs = SR_INVALID ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

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
				    if (avl)
	                            rs = cfdeci(avp,avl, &pip->namelen) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
				    if (argl)
	                            rs = cfdeci(argp,argl,&pip->namelen) 

	                        }

	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* require a suffix for file names */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = paramopt_loads(&aparams,PO_SUFFIX,
						avp,avl) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = paramopt_loads(&aparams,PO_SUFFIX,
						argp,argl) ;

	                        }

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->f.verbose = TRUE ;
	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_INVALID ;
	                        bprintf(efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
			    if (rs < 0)
				break ;

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
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel %d\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto retearly ;

/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDIR ;



/* get ready */

	if (paramopt_havekey(&aparams,PO_SUFFIX) >= 0) {

	    pip->f.suffix = TRUE ;

	} /* end if */


/* OK, we do it */

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

	        pan += 1 ;
	        rs = process(&g,&aparams,argv[i]) ;

		if (rs < 0)
	            break ;

	    } /* end for (looping through requested circuits) */

	} /* end if */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] == '\0')) {

	    bfile	afile ;


	    if (strcmp(afname,"-") != 0) 
	        rs = bopen(afp,afname,"r",0666) ;

	    else
	        rs = bopen(afp,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(afp,buf,BUFLEN)) > 0) {

		    len = rs ;
	            if (linebuf[len - 1] == '\n') 
			len -= 1 ;

	            linebuf[len] = '\0' ;
	            cp = strshrink(linebuf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = process(&g,&aparams,cp) ;

		    if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(efp,"\trs=%d argfile=%s\n",rs,afname) ;

	        }

	    }

	} /* end if (processing file argument file list */

	if (pan == 0)
	    goto badnodirs ;

	if (rs >= 0) {

	if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0644)) >= 0) {

	    bprintf(ofp,"%d megabytes and %d bytes\n",
	        pip->megabytes,pip->bytes) ;

/* calculate UNIX blocks */

	    {
	        long	blocks, blockbytes ;


	        blocks = pip->megabytes * 1024 * 2 ;
	        blocks += (pip->bytes / UNIXBLOCK) ;
	        blockbytes = (pip->bytes % UNIXBLOCK) ;

	        bprintf(ofp,"%d UNIX blocks and %d bytes\n",
	            blocks,blockbytes) ;

	    } /* end block */

	    bclose(ofp) ;

	} else
	    goto badoutopen ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting\n") ;
#endif

/* good return from program */
retgood:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: program exiting\n",
	        pip->progname) ;
#endif

/* we are out of here */
retearly:
ret1:
	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* come here for a bad return from the program */
badret:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	bclose(pip->ofp) ;

	bclose(pip->efp) ;

	return BAD ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [directory(s) ...] [-s suffixes] [-Vv]\n",
	    pip->progname,pip->progname) ;

	goto badret ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badret ;

badoutopen:
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badinopen:
	bprintf(pip->efp,"%s: could not open standard input (rs=%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badnodirs:
	bprintf(pip->efp,"%s: no files or directories were specified\n",
	    pip->progname) ;

	goto badret ;

}
/* end subroutine (main) */



