/* main */

/* generic (more of less) front-end subroutine */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous program by
	the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a fairly generic front-end subroutine.


******************************************************************************/

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

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

extern int	sfshrink(cchar *,int,cchar **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	progfile(PROGINFO *,PARAMOPT *,const char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */

static char *const argopts[] = {
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"option",
	"set",
	"follow",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_overlast
} ;

static char	*const options[] = {
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


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	PARAMOPT	aparams ;
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;

	int		argr, argl, aol, akl, avl, kwi ;
	int		ai, ai_max, ai_pos ;
	int		argvalue = -1 ;
	int		pan = 0 ;
	int		rs ;
	int		i ;
	int		ex = EX_INFO ;
	int		f_optminus, f_optplus, f_optequal ;
	int		f_extra = FALSE ;
	int		f_usage = FALSE ;
	int		f_help = FALSE ;
	int		f_version = FALSE ;
	int		f ;

	cchar		*argp, *aop, *akp, *avp ;
	cchar		*afname = NULL ;
	cchar		*cp ;
	char		argpresent[MAXARGGROUPS] ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;


	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {

	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;

	}

/* early things to initialize */

	pip->ofp = ofp ;

	pip->namelen = MAXNAMELEN ;

	pip->debuglevel = 0 ;
	pip->verboselevel = 1 ;

	pip->f.nochange = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.follow = FALSE ;

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

	for (ai = 0 ; ai < MAXARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[++ai] ;
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

	                        if (argr <= 0) 
					goto badargnum ;

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) 
				pip->tmpdname = argp ;

	                    }

	                    break ;

/* the user specified some options */
	                case argopt_option:
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = paramopt_loads(&aparams,PO_OPTION,
	                            argp,argl) ;

	                    break ;

/* the user specified some options */
	                case argopt_set:
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        paramopt_loadu(&aparams,argp,argl) ;

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

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

	                        }

	                        break ;

/* take input file arguments from STDIN */
	                    case 'f':
	                        if (argr <= 0) 
					goto badargnum ;

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

/* we allow a zero length string as a valid argument */
	                        afname = argp ;

	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->namelen) ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            rs = cfdeci(argp,argl,&pip->namelen) ;

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
	                                paramopt_loads(&aparams,PO_SUFFIX,
						avp,avl) ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = paramopt_loads(&aparams,PO_SUFFIX,
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
				ex = EX_USAGE ;
	                        f_usage = TRUE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                    akp += 1 ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } else {

	            if (i < MAXARGINDEX) {

	                BASET(argpresent,i) ;
	                ai_max = i ;

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            ai_max = i ;

	        } else {

	            if (! f_extra)
	                f_extra = TRUE ;

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

	if (pip->tmpdname == NULL) pip->tmpdname = getenv("TMPDIR") ;
	if (pip->tmpdname == NULL) pip->tmpdname = TMPDNAME ;

/* get ready */

#ifdef	COMMENT

	if ((rs = paramopt_havekey(&aparams,PO_SUFFIX)) > 0) {
	    pip->f.suffix = TRUE ;
	} /* end if */

	if ((rs = paramopt_havekey(&aparams,PO_OPTION)) > 0) {
	    PARAMOPT_CUR	cur ;

	    paramopt_curbegin(&aparams,&cur) ;

	    while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

		if (cp == NULL) continue ;

	        if ((kwi = matstr(progopts,cp,-1)) >= 0) {

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

	    paramopt_curend(&aparams,&cur) ;

	} /* end if (options) */

#endif /* COMMENT */


/* open some output file butt */

	rs = bopen(pip->ofp,BFILE_STDOUT,"wct",0666) ;

	if (rs < 0)
		goto badoutopen ;


/* OK, we do it */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

		cp = argv[ai] ;
		pan += 1 ;
	        rs = progfile(pip,&aparams,cp) ;

		if (rs < 0) break ;
	    } /* end for (looping through requested circuits) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL)) {
	    bfile	argfile, *afp = &argfile ;
	    int		cl ;

	    if ((strcmp(afname,"-") == 0) || (afname[0] == '\0'))
		afname = BFILE_STDIN ;

	    if ((rs = bopen(afp,afname,"r",0666)) >= 0) {
		const int	llen = LINEBUFLEN ;
	        int		len ;
	        char		lbuf[LINEBUFLEN + 1] ;

	        while ((rs = breadline(afp,lbuf,llen)) > 0) {
		    len = rs ;

	            if (lbuf[len - 1] == '\n') len -= 1 ;

	            lbuf[len] = '\0' ;
	            if ((cl = sfshrink(lbuf,len,&cp)) > 0) {
	                if (cp[0] != '#') {
	            	    pan += 1 ;
	            	    rs = progfile(pip,&aparams,cp) ;
			}
		    }

		    if (rs < 0) break ;
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

	if ((rs >= 0) && (pan == 0) && (afname == NULL)) {

	            rs = progfile(pip,&aparams,"-") ;

	}

	bclose(pip->ofp) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting\n") ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: exiting ex=%u (%d)\n",
	        pip->progname,ex,rs) ;

/* we are out of here */
done:

retearly:
ret1:
	if (pip->open.aparams)
	    paramopt_finish(&aparams) ;

	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [file(s) ...] [-Vv]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,
	    "%s: \t[-f {argfile|-}]\n",
		pip->progname) ;

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

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

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

/* come here for a bad return from the program */
badret:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	ex = EX_USAGE ;
	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


