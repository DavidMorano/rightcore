/* main */

/* this is a generic "main" module modified for the 'textclean' program */


#define	CF_DEBUG	0
#define	CF_DEBUGS	0
#define	CF_GETEXECNAME	1		/* use 'getexecname(3c)' */


/* revision history:

	= 1992-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1992 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a fairly generic front-end subroutine for small programs.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	"paramopt.h"
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr2(const char **,const char *,int) ;
extern int	process(struct proginfo *,struct options,bfile *,
			const char *) ;

extern char	*strbasename(char *) ;
extern char	*strshrink(char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	usage(struct proginfo *) ;


/* local variables */

static const char	*argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"af",
	"of",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_af,
	argopt_of,
	argopt_overlast
} ;

static const char	*procopts[] = {
	"lower",
	"inplace",
	"doublespace",
	"half",
	"leading",
	"oneblank",
	"mscrap",
	NULL
} ;

enum procopts {
	procopt_lower,
	procopt_inplace,
	procopt_double,
	procopt_half,
	procopt_leading,
	procopt_oneblank,
	procopt_mscrap,
	procopt_overlast
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct options	opts ;

	struct proginfo	pi, *pip = &pi ;

	PARAMOPT	param ;

	PARAMOPT_CUR	cur ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl ;
	int	ai, ai_max, ai_pos, kwi ;
	int	argvalue = -1 ;
	int	rs = SR_OK ;
	int	i, pan ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f_makedate = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	rs = proginfo_setbanner(pip,cp) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = paramopt_start(&param) ;
	pip->open.aparams = (rs >= 0) ;

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
	                rs = cfdecti((argp + 1),(argl - 1),&argvalue) ;

	        } else if (argp[1] == '-') {

		    ai_pos = ai ;
	            break ;

	        } else {

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

/* do we have a keyword match or should we assume only key letters ? */

	            if ((kwi = matstr2(argopts,aop,aol)) >= 0) {

	                switch (kwi) {

/* version */
	                case argopt_version:
				f_makedate = f_version ;
	                    f_version = TRUE ;
	                    if (f_optequal) 
				rs = SR_INVALID ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl > 0) {

	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

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

	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) 
					pip->tmpdname = argp ;

	                    }

	                    break ;

	                    case argopt_help:
	                        f_help = TRUE ;
	                        break ;

/* take input file arguments from STDIN */
	                    case argopt_af:
	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                        afname = argp ;

	                        break ;

/* get an output file name other than using STDOUT ! */
	                    case argopt_of:
	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                        ofname = argp ;

	                        break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *aop) {

	                    case 'V':
				f_makedate = f_version ;
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl) {

	                            	rs = cfdeci(avp,avl,&pip->debuglevel) ;

				    }

	                        }

	                        break ;

/* take input file arguments from STDIN */
	                    case 'f':
	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                        afname = argp ;

	                        break ;

	                    case 'h':
	                        opts.half = TRUE ;
	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* quiet */
	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* options */
			    case 'o':
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = paramopt_loads(&param,PO_OPTION,
						avp,avl) ;

	                        } else {

	                        if (argr <= 0) {
					rs = SR_INVALID ;
					break ;
				}

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = paramopt_loads(&param,PO_OPTION,
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

	                            }
	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_INVALID ;
	                        bprintf(pip->efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;
			    if (rs < 0)
				break ;

	                } /* end while */

		    }

	        } /* end if (individual option key letters) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0)
		goto badarg ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,
	        "%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;

	    bcontrol(pip->efp,BC_LINEBUF,0) ;

	    bflush(pip->efp) ;

	}

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version) 
		goto done ;

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

/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* set the options for processing */

	{
		PARAMOPT_CUR	cur ;


	paramopt_curbegin(&param,&cur) ;

	while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

	    if (cp == NULL) continue ;

	    if ((i = matstr2(procopts,cp,-1)) < 0)
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

	    case procopt_oneblank:
	        opts.oneblank = TRUE ;
	        break ;

	    case procopt_mscrap:
	        opts.mscrap = TRUE ;
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

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: positional arguments\n") ;
#endif

	    for (ai = 0 ; ai < argc ; ai += 1) {

	            f = (ai <= ai_max) && BATST(argpresent,ai) ;
	            f = f || (ai > ai_pos) ;
	            if (! f) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

	        rs = process(pip,opts,ofp,argv[ai]) ;

		if (rs < 0)
	            break ;

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	if (pan == 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("main: processing STDIN\n") ;
#endif

	    rs = process(pip,opts,ofp,NULL) ;

	    pan += 1 ;

	} /* end if (program invocation arguments) */


/* process any files in the argument filename list file */

	if (afname != NULL) {

		bfile	argfile, *afp = &argfile ;


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

	            if (buf[len - 1] == '\n') 
			len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            rs = process(pip,opts,ofp,cp) ;

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

	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"\targfile=%s\n",afname) ;

	        }

	    }

	} /* end if (processing file argument file list */

	if (! opts.inplace)
	    bclose(ofp) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting, file processed %d\n",pan) ;
#endif

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

/* good return from program */
ret2:
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: %d input file%s processed\n",
	        pip->progname,pan,((pan == 0) ? "" : "s")) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: program exiting, pan=%d\n",pan) ;
#endif

/* we are out of here */
done:
retearly:
ret1:
	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

	if (pip->efp != NULL)
	    bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	usage(pip) ;

	goto retearly ;

help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

/* bad stuff comes here */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

/* other bad stuff */
badoutopen:
	bprintf(pip->efp,"%s: could not open output (%d)\n",
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



/* LOCAL SUBROUTINES */



static int usage(pip)
struct proginfo	*pip ;
{
	int	rs ;


	bprintf(pip->efp,
	    "%s: USAGE> %s [file(s) ...] [-o options] [-h] [-Vv]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,"%s:\n",
		pip->progname) ;

	bprintf(pip->efp,
	    "%s: \t-h				half space where possible\n",
		pip->progname) ;

	bprintf(pip->efp,
	    "%s: \t-s option[,options,...]	processing options\n",
		pip->progname) ;

	bprintf(pip->efp,"%s:\n",
		pip->progname) ;

	bprintf(pip->efp,
	    "%s: \tprocessing options include :\n",
		pip->progname) ;

	bprintf(pip->efp,
	    "%s: \thalf				half space where possible\n",
		pip->progname) ;

	bprintf(pip->efp,
	    "%s: \tdouble			double space \n",
		pip->progname) ;

	bprintf(pip->efp,
	    "%s: \tinplace			modify file \"in place\"\n",
		pip->progname) ;

	bprintf(pip->efp,
	    "%s: \tlower			convert to lower case\n",
		pip->progname) ;

	rs = bprintf(pip->efp,
	    "%s: \tleading			string leading white space\n",
		pip->progname) ;

	return rs ;
}
/* end subroutine (usage) */



