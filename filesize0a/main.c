/* main */

/* part of the 'filesize' program */


#define	CF_DEBUGS	0
#define	CF_DEBUG		1


/* revision history :

	= 1996-02-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generic front-end subroutine for a program.


*******************************************************************************/


#include	<envstandards.h>

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
#include	<paramopt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	optmatch3(char *const *,char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern char	*strbasename(char *), *strshrink(char *) ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */

static cchar	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"option",
	"set",
	"follow",
	"af",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_af,
	argopt_overlast
} ;

static cchar	*options[] = {
	"follow",
	"nofollow",
	NULL
} ;

enum options {
	opt_follow,
	opt_nofollow,
	opt_overlast
} ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	PARAMOPT	param ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		argfile, *afp = &argfile ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	ex = EX_INFO ;
	int	rs, i, pan ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	fd_debug ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*afname = NULL ;
	char	*cp ;


	if ((cp = getenv(DEBUGFDVAR1)) == NULL)
	    cp = getenv(DEBUGFDVAR2) ;

	if ((cp != NULL) &&
	    (cfdeci(cp,-1,&fd_debug) >= 0))
	    esetfd(fd_debug) ;


	memset(pip,0,sizeof(PROGINFO)) ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BIO_STDERR,"dwca",0666) >= 0) {
	    pip->efp = efp ;
	    bcontrol(efp,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->ofp = ofp ;
	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;
	pip->tmpdir = NULL ;
	pip->namelen = MAXNAMELEN ;

	pip->bytes = 0 ;
	pip->megabytes = 0 ;

	(void) memset(&pip->f,0,sizeof(PROGINFO_flags)) ;

	pip->f.nochange = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.follow = FALSE ;
	pip->f.suffix = FALSE ;


/* process program arguments */

	paramopt_init(&param) ;

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

#if	CF_DEBUGS
	            eprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
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

#if	CF_DEBUGS
	            eprintf("main: about to check for a key word match\n") ;
#endif

	            if ((kwi = optmatch3(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                eprintf("main: got an option keyword, kwi=%d\n",
	                    kwi) ;
#endif

	                switch (kwi) {

/* version */
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        goto badargextra ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->verboselevel = 2 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl > 0) {

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

	                            if (rs < 0)
	                                goto badargval ;

	                        }
	                    }

	                    break ;

/* temporary directory */
	                case argopt_tmpdir:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pip->tmpdir = avp ;

	                    } else {

	                        if (argr <= 0)
	                            goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->tmpdir = argp ;

	                    }

	                    break ;

/* the user specified some options */
	                case argopt_option:
	                    if (argr <= 0)
	                        goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        paramopt_loads(&param,OPTION,
	                            argp,argl) ;

	                    break ;

/* the user specified some options */
	                case argopt_set:
	                    if (argr <= 0)
	                        goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        paramopt_loadu(&param,argp,argl) ;

	                    break ;

/* argument files */
	                case argopt_af:
	                    if (argr <= 0)
	                        goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        afname = argp ;

	                    break ;

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
	                    ex = EX_USAGE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

#if	CF_DEBUGS
	                eprintf("main: got an option key letter\n") ;
#endif

	                while (aol--) {

#if	CF_DEBUGS
	                    eprintf("main: option key letters\n") ;
#endif

	                    switch (*aop) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            eprintf("main: debugequal avl=%d avp=%s\n",
	                                avl,avp) ;
#endif

	                            f_optequal = FALSE ;
	                            if (cfdeci(avp,avl, &pip->debuglevel) < 0)
	                                goto badargval ;

#if	CF_DEBUGS
	                            eprintf("main: debuglevel=%d\n",
	                                pip->debuglevel) ;
#endif
	                        }

	                        break ;

/* take input file arguments from STDIN */
	                    case 'f':
				pip->f.follow = TRUE ;
	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdeci(avp,avl, &pip->namelen) < 0)
	                                goto badargval ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (cfdeci(argp,argl,&pip->namelen) < 0)
	                                goto badargval ;

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
	                                paramopt_loads(&param,SUFFIX,avp,avl) ;

	                        } else {

	                            if (argr <= 0)
	                                goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramopt_loads(&param,SUFFIX,
	                                    argp,argl) ;

	                        }

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                                if (rs < 0)
	                                    goto badargval ;

	                            }
	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

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

	            if (! f_extra)
	                f_extra = TRUE ;

	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


/* check arguments */

#if	CF_DEBUGS
	eprintf("main: finished parsing command line arguments\n") ;
	eprintf("main: npa=%d\n",npa) ;
#endif


	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel %d\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_extra)
	    goto badargignore ;

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;


/* check a few more things */

	if (pip->tmpdir == NULL)
	    pip->tmpdir = getenv("TMPDIR") ;

	if (pip->tmpdir == NULL)
	    pip->tmpdir = TMPDIR ;



/* get ready */

	if (paramopt_findkey(&param,SUFFIX,NULL) >= 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        eprintf("main: suffixes so far\n") ;


	    }
#endif /* CF_DEBUG */

	    pip->f.suffix = TRUE ;

	} /* end if */


	if (paramopt_findkey(&param,OPTION,NULL) >= 0) {

	    PARAMOPT_CURSOR	cur ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) {

	        eprintf("main: parameter options\n") ;

	    }
#endif /* CF_DEBUG */

	    paramopt_curbegin(&param,&cur) ;

	    while (paramopt_enumkeys(&param,&cur,&cp) >= 0) {

	        if ((kwi = optmatch(argopts,cp,-1)) >= 0) {

	            switch (kwi) {

	            case opt_follow:
	                pip->f.follow = TRUE ;
	                break ;

	            case opt_nofollow:
	                pip->f.follow = FALSE ;
	                break ;

	            } /* end switch */

	        } /* end if (options) */

	    } /* end while */

	    paramopt_curend(&param,&cur) ;

	} /* end if (options) */



/* OK, we do it */

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

	        if ((rs = process(pip,argv[i],&param)) < 0)
	            break ;

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	} /* end if */


/* process any files in the argument filename list file */

	if (afname != NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("main: we have an argument file list\n") ;
#endif

	    if ((strcmp(afname,"-") == 0) || (afname[0] == '\0'))
	        rs = bopen(afp,BIO_STDIN,"dr",0666) ;

	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {
	        int	len ;
	        char	buf[BUFLEN + 1] ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: processing the argument file list\n") ;
#endif

	        while ((len = breadline(afp,buf,BUFLEN)) > 0) {

	            if (buf[len - 1] == '\n') len -= 1 ;
	            buf[len] = '\0' ;

	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("main: calling process name=\"%s\"\n",cp) ;
#endif

	            if ((rs = process(pip,cp,&param)) < 0)
	                break ;

	            pan += 1 ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("main: done processing the argument file list\n") ;
#endif

	    } else {
	        if (! pip->f.quiet) {
	            bprintf(efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;
	            bprintf(efp,"\trs=%d argfile=%s\n",rs,afname) ;
	        }
	    }

	} /* end if (processing file argument file list */


	if ((pan == 0) && (afname == NULL))
	    goto badnodirs ;


	if ((rs = bopen(ofp,BIO_STDOUT,"dwct",0644)) >= 0) {

	    bprintf(ofp,"%d megabyte%s and %d bytes\n",
	        pip->megabytes,
	        ((pip->megabytes == 1) ? "" : "s"),
	        pip->bytes) ;

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


	ex = EX_OK ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    eprintf("main: exiting\n") ;
#endif

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: program exiting\n",
	        pip->progname) ;
#endif

/* we are out of here */
done:

retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [directory(s) ...] [-s suffixes] [-Vv]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,
	    "%s: \t[-f {argfile|-}]\n",
	    pip->progname) ;

	ex = EX_INFO ;
	goto retearly ;

badargignore:
	bprintf(efp,"%s: extra arguments provided\n",
	    pip->progname) ;

	goto badarg ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargval:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto ret1 ;

badinopen:
	ex = EX_NOINPUT ;
	bprintf(pip->efp,"%s: could not open standard input (rs=%d)\n",
	    pip->progname,rs) ;

	goto ret1 ;

badnodirs:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: no files or directories were specified\n",
	    pip->progname) ;

	goto ret1 ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

	goto ret1 ;

/* come here for a bad return from the program */
badret:

#if	CF_DEBUGS
	eprintf("main: exiting program BAD\n") ;
#endif

	ex = EX_USAGE ;
	goto retearly ;

}
/* end subroutine (main) */


