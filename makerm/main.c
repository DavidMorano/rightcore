/* main */

/* generic short program front-end */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_ALWAYS	1		/* always update the target? */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The program was written from scratch to do what
	the previous program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Fairly generic front-end subroutine.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

extern int	process(struct proginfo *,const char *,char *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char *argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	NULL,
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_overlast
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	struct proginfo	pi, *pip = &pi ;
	struct ustat	sb ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl ;
	int	npa, maxai, kwi ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs = SR_OK ;
	int	i, iw ;
	int	c_processed = 0 ;
	int	c_updated = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*dirfname = NULL ;
	char	*touchfname = NULL ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->ofp = ofp ;

	pip->namelen = MAXNAMELEN ;
	pip->suffixlen = -1 ;

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = SR_OK ;

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
	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal)
	                        goto badargextra ;

	                    break ;

/* verbose */
	                case argopt_verbose:
	                    pip->f.verbose = TRUE ;
	                    if (f_optequal)
	                        goto badargextra ;

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
			    rs = SR_INVALID ;
	                    f_usage = TRUE ;
	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                        }

	                        break ;

/* quiet */
	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* directory name */
	                    case 'd':
	                        if (argr <= 0)
	                            goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            dirfname = argp ;

	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* print something!! */
	                    case 'p':
	                        pip->f.print = TRUE ;
	                        break ;

			    case 'q':
				pip->verboselevel = 0 ;
				break ;

/* require a suffix for file names */
	                    case 's':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) 
					pip->suffix = avp ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) 
					pip->suffix = argp ;

	                        }

	                        break ;

/* touch file */
	                    case 't':
	                        if (argr <= 0)
	                            goto badargnum ;

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            touchfname = argp ;

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->f.verbose = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl) {
	                                rs = cfdeci(avp,avl,&iw) ;
					pip->verboselevel = iw ;
				    }

	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_INVALID ;
				f_usage = TRUE ;
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

			ex = EX_USAGE ;
	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    pip->progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

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
	    bprintf(efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto done ;


/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

	pip->suffixlen = -1 ;
	if (pip->suffix != NULL)
	    pip->suffixlen = strlen(pip->suffix) ;

/* OK, we do it */

	ex = EX_DATAERR ;
	if (npa > 0) {

	    if (dirfname == NULL) {

/* scan for a directory */

	        for (i = 0 ; i <= maxai ; i += 1) {

	            if ((! BATST(argpresent,i)) &&
	                (argv[i][0] != '\0')) continue ;

	            if ((u_stat(argv[i],&sb) >= 0) && S_ISDIR(sb.st_mode))
	                break ;

	        } /* end for */

	        if (i > maxai)
	            goto badnodir ;

	        dirfname = argv[i] ;
	        BACLR(argpresent,i) ;

	        npa -= 1 ;
	        if (npa <= 0)
	            goto badnofiles ;

	    } /* end if (getting a directory) */

/* pop the files proper */

	    if (pip->f.print || pip->f.verbose) {

	        if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0644)) < 0)
	            goto badoutopen ;

	    }

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

		cp = argv[i] ;
	        rs = process(pip,dirfname,cp) ;

		if (rs < 0)
	            break ;

	        c_processed += 1 ;
	        if (rs > 0)
	            c_updated += 1 ;

	    } /* end for (looping through requested circuits) */

	    if (pip->f.print || pip->f.verbose)
	        bclose(ofp) ;


	} else
	    goto badnofiles ;


	ex = EX_DATAERR ;
	if (rs >= 0) {

	    ex = EX_OK ;
	    if ((touchfname != NULL) && (touchfname[0] != '\0')) {

	        bfile	touchfile ;

	        time_t	daytime = time(NULL) ;

	        int	f_write = FALSE ;

	        char	timebuf[TIMEBUFLEN + 1] ;


	        rs = bopen(&touchfile,touchfname,"w",0666) ;

	        if (rs < 0) {

	            f_write = TRUE ;
	            rs = bopen(&touchfile,touchfname,"wct",0666) ;

	        }

	        if (rs >= 0) {

#if	CF_ALWAYS
	                bprintf(&touchfile,"%s\n",
	                    timestr_logz(daytime,timebuf)) ;
#else
	            if (f_write || (c_updated > 0))
	                bprintf(&touchfile,"%s\n",
	                    timestr_logz(daytime,timebuf)) ;
#endif /* CF_ALWAYS */

	            bclose(&touchfile) ;

	        } else
			ex = EX_CANTCREAT ;

	    } /* end if (touch file) */

	} /* end if */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting rs=%d\n") ;
#endif

/* good return from program */
retgood:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    bprintf(pip->efp,"%s: program exiting\n",
	        pip->progname) ;
#endif

/* we are out of here */
done:
retearly:
	bclose(pip->efp) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s file(s) [...] {directory|-d directory} [-npv]\n",
	    pip->progname,pip->progname) ;

	goto retearly ;

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
	goto retearly ;


badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

badnofiles:
	ex = EX_USAGE ;
	if (! pip->f.quiet)
		bprintf(pip->efp,"%s: no files were specified\n",
	    	pip->progname) ;

	goto retearly ;

badnodir:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: no directory was specified\n",
	    pip->progname) ;

	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



