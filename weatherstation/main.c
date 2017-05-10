/* main */

/* this is a generic "main" module modified for the 'textclean' program */


#define	CF_DEBUG	0
#define	CF_DEBUGS	0


/* revision history:

	= 1992-03-01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a fairly generit front-end subroutine for small
	programs.


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

extern int	matstr(char *const *,const char *,int) ;

extern char	*strbasename(char *), *strshrink(char *) ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_overlast
} ;

static const char	*procopts[] = {
	"lower",
	"inplace",
	"doublespace",
	"half",
	"leading",
	NULL
} ;

enum procopts {
	procopt_lower,
	procopt_inplace,
	procopt_double,
	procopt_half,
	procopt_leading,
	procopt_overlast
} ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;
	struct options	opts ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	argfile, *afp = &argfile ;

	PARAMOPT	aparams ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs ;
	int	i ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	const char	*argval = NULL ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;

	pip->efp = efp ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

/* early things to initialize */

	pip->ofp = ofp ;

	pip->verboselevel = 1 ;

/* processing options */

	opts.lower = FALSE ;
	opts.inplace = FALSE ;
	opts.doublespace = FALSE ;
	opts.half = FALSE ;
	opts.leading = FALSE ;

/* process program arguments */

	paramopt_start(&aparams) ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

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

#if	CF_DEBUGS
	            debugprintf("main: got an option\n") ;
#endif

	            aop = argp + 1 ;
	            akp = aop ;
	            aol = argl - 1 ;
	            f_optequal = FALSE ;
	            if ((avp = strchr(aop,'=')) != NULL) {

#if	CF_DEBUGS
	                debugprintf("main: got an option key w/ a value\n") ;
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
	            debugprintf("main: about to check for a key word match\n") ;
#endif

	            if ((kwi = matstr(argopts,aop,aol)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("main: got an option keyword, kwi=%d\n",
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
					pip->tmpdname = avp ;

	                    } else {

	                        if (argr <= 0) 
					goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) 
					pip->tmpdname = argp ;

	                    }

	                    break ;

/* default action and user specified help */
	                default:
	                    f_usage = TRUE ;
			ex = EX_USAGE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

#if	CF_DEBUGS
	                debugprintf("main: got an option key letter\n") ;
#endif

	                while (akl--) {

#if	CF_DEBUGS
	                    debugprintf("main: option key letters\n") ;
#endif

	                    switch ((int) *aop) {

	                    case 'V':

#if	CF_DEBUGS
	                        debugprintf("main: version key-letter\n") ;
#endif
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            debugprintf("main: debugequal avl=%d avp=%s\n",
	                                avl,avp) ;
#endif

	                            f_optequal = FALSE ;
	                            if (cfdec(avp,avl, &pip->debuglevel) < 0)
	                                goto badargval ;

#if	CF_DEBUGS
	                            debugprintf("main: debuglevel=%u\n",
	                                pip->debuglevel) ;
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

	                    case 'h':
	                        opts.half = TRUE ;
	                        break ;

/* get an output file name other than using STDOUT !*/
	                    case 'o':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                        ofname = argp ;

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
	                                rs = paramopt_loads(&aparams,PO_OPTION,
						avp,avl) ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                paramopt_loads(&param,PO_OPTION,
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
			f_usage = TRUE ;
			ex = EX_USAGE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;

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
		goto done ;


/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDIR ;


/* set the options for processing */

	{
		PARAMOPT_CUR	cur ;


	paramopt_curbegin(&aparams,&cur) ;

	while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

	    if (cp == NULL) continue ;

	    if ((i = matstr(procopts,cp,-1)) < 0)
	        continue ;

	    switch (i) {

	    case procopt_lower:
	        opts.lower = TRUE ;
	        break ;

	    case procopt_inplace:
	        opts.inplace = TRUE ;
	        break ;

	    case procopt_double:
	        opts.doublespace = TRUE ;
	        break ;

	    case procopt_half:
	        opts.half = TRUE ;
	        break ;

	    case procopt_leading:
	        opts.leading = TRUE ;
	        break ;

	    } /* end switch */

	} /* end while */

	paramopt_curend(&param,&cur) ;

	} /* end if (option processing) */


/* open the output file (if we are not processing in place that it) */

	if (! opts.inplace) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: opening output file=%s\n",ofname) ;
#endif

	    if ((ofname == NULL) || (ofname[0] == '\0'))
	        rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	    else
	        rs = bopen(ofp,ofname,"wct",0666) ;

	    if (rs < 0)
	        goto badoutopen ;

	} /* end if (opening the output file) */


/* OK, we do it */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: doing it man \n") ;
#endif

	pan = 0 ;

	if (npa > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: positional arguments\n") ;
#endif

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) && (argv[i][0] != '\0')) 
			continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

	        rs = process(pip,argv[i],opts) ;

		if (rs < 0)
	            break ;

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	} else {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing STDIN\n") ;
#endif

	    rs = process(pip,NULL,opts) ;

	    pan += 1 ;

	} /* end if (program invocation arguments) */


/* process any files in the argument filename list file */

	if (afname != NULL) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: we have an argument file list\n") ;
#endif

	    if ((strcmp(afname,"-") == 0) || (afname[0] == '\0'))
	        rs = bopen(afp,BFILE_STDIN,"dr",0666) ;

	    else
	        rs = bopen(afp,afname,"r",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	buf[BUFLEN + 1] ;


#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing the argument file list\n") ;
#endif

	        while ((len = breadline(afp,buf,BUFLEN)) > 0) {

	            if (buf[len - 1] == '\n') len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            rs = process(pip,cp,opts) ;

		    if (rs < 0)
	                break ;

	            pan += 1 ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: done processing the argument file list\n") ;
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




	if (! opts.inplace)
	    bclose(ofp) ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting, file processed %d\n",pan) ;
#endif


/* good return from program */
goodret:
	if (pip->debuglevel > 0)
	    bprintf(efp,"%s: %d input file%s processed\n",
	        pip->progname,pan,((pan == 0) ? "" : "s")) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: program exiting, pan=%d\n",pan) ;
#endif

	ex = EX_OK ;

/* we are out of here */
done:
earlyret:
ret1:
	paramopt_finish(&aparams) ;

	bclose(pip->efp) ;

ret0:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [file(s) ...] [-s options] [-h] [-Vv]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,
	    "\t-h				half space where possible\n") ;

	bprintf(pip->efp,
	    "\t-s option[,options,...]		processing options\n") ;

	bprintf(pip->efp,"\n") ;

	bprintf(pip->efp,
	    "\tprocessing options include :\n") ;

	bprintf(pip->efp,
	    "\thalf				half space where possible\n") ;

	bprintf(pip->efp,
	    "\tdouble				double space \n") ;

	bprintf(pip->efp,
	    "\tinplace				modify file \"in place\"\n") ;

	bprintf(pip->efp,
	    "\tlower				convert to lower case\n") ;

	bprintf(pip->efp,
	    "\tleading				string leading white space\n") ;

	goto earlyret ;

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
	goto earlyret ;

badoutopen:
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

	goto badret ;

badnodirs:
	bprintf(pip->efp,"%s: no files or directories were specified\n",
	    pip->progname) ;

	goto badret ;

/* come here for a bad return from the program */
badret:
	ex = EX_DATAERR ;

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	goto done ;

}
/* end subroutine (main) */



