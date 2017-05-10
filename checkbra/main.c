/* main */

/* generic (more of less) front-end subroutine */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 1998-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a fairly generic front-end subroutine for small programs.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<baops.h>
#include	<field.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;

extern int	procfile(struct proginfo *,PARAMOPT *,const char *) ;

extern char	*strshrink(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static const char	*argopts[] = {
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

static const char	*progopts[] = {
	"follow",
	"nofollow",
	NULL
} ;

enum progopts {
	progopt_follow,
	progopt_nofollow,
	progopt_overlast
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	PARAMOPT	aparams ;

	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan, npa ;
	int	rs, rs1, i ;
	int	len ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*afname = NULL ;
	char	*cp ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	pip->efp = efp ;
	if (bopen(efp,BFILE_STDERR,"dwca",0666) >= 0)
	    bcontrol(efp,BC_LINEBUF,0) ;

/* early things to initialize */

	pip->ofp = ofp ;

	pip->tmpdname = NULL ;
	pip->namelen = MAXNAMELEN ;

	pip->verboselevel = 1 ;

	pip->f.nochange = FALSE ;
	pip->f.quiet = FALSE ;
	pip->f.follow = FALSE ;
	pip->f.suffix = FALSE ;

/* process program arguments */

	rs = paramopt_start(&aparams) ;
	pip->open.aparams = (rs >= 0) ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) 
		argpresent[i] = 0 ;

	npa = 0 ;			/* number of positional so far */
	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
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

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pip->tmpdname = argp ;

	                    }

	                    break ;

/* the user specified some options */
	                case argopt_option:
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
				    break ;
				}

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = paramopt_loads(&aparams,PO_OPTION,
	                            argp,argl) ;

	                    break ;

/* the user specified some options */
	                case argopt_set:
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
				    break ;
				}

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = paramopt_loadu(&aparams,argp,argl) ;

	                    break ;

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
	                    break ;

	                case argopt_af:
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
				    break ;
				}

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        afname = argp ;

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
	                            if (avl) {

	                                rs = cfdeci(avp,avl, 
	                                    &pip->debuglevel) ;

	                            }

	                        }

	                        break ;

/* take input file arguments from STDIN */
	                    case 'f':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
				    break ;
				}

	                        argp = argv[++i] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        afname = argp ;

	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            rs = cfdeci(avp,avl, &pip->namelen) ;

	                        } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
				    break ;
				}

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) {

	                                rs = cfdeci(argp,argl,&pip->namelen) ;

	                            }

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

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
				    break ;
				}

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
	                npa += 1 ;	/* increment position count */

	            }

	        } /* end if */

	    } else {

	        if (i < MAXARGINDEX) {

	            BASET(argpresent,i) ;
	            ai_max = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra)
	                f_extra = TRUE ;

	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */

	if (rs < 0)
		goto badarg ;

/* check arguments */

#if	CF_DEBUGS
	debugprintf("main: finished parsing command line arguments\n") ;
	debugprintf("main: npa=%d\n",npa) ;
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

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv("TMPDIR") ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;


/* get ready */

	if (paramopt_havekey(&aparams,PO_SUFFIX) >= 0) {

	    pip->f.suffix = TRUE ;

	} /* end if */

	if (paramopt_havekey(&aparams,PO_OPTION) >= 0) {

	    PARAMOPT_CUR	cur ;


	    paramopt_curbegin(&aparams,&cur) ;

	    while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

		if (cp == NULL) continue ;

	        if ((kwi = matostr(progopts,1,cp,-1)) >= 0) {

	            switch (kwi) {

	            case progopt_follow:
	                pip->f.follow = TRUE ;
	                break ;

	            case progopt_nofollow:
	                pip->f.follow = FALSE ;
	                break ;

	            } /* end switch */

	        } /* end if (options) */

	    } /* end while */

	    paramopt_curend(&aparams,&cur) ;

	} /* end if (options) */


/* open some output file butt */

	rs = bopen(pip->ofp,BFILE_STDOUT,"wct",0666) ;

	if (rs < 0)
	    goto badoutopen ;


/* OK, we do it */

	pan = 0 ;
	if (npa > 0) {

	    for (i = 0 ; i <= ai_max ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: calling process name=\"%s\"\n",argv[i]) ;
#endif

		cp = argv[ai] ;
	        pan += 1 ;
	        rs = procfile(pip,&aparams,cp) ;

	        if (rs < 0)
	            break ;

	    } /* end for (looping through requested circuits) */

	} /* end if */

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

	        char	buf[BUFLEN + 1] ;


#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: processing the argument file list\n") ;
#endif

	        while ((rs = breadline(afp,buf,BUFLEN)) > 0) {

		    len = rs ;
	            if (buf[len - 1] == '\n')
	                len -= 1 ;

	            buf[len] = '\0' ;
	            cp = strshrink(buf) ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("main: calling process name=\"%s\"\n",cp) ;
#endif

	            pan += 1 ;
	            rs = procfile(pip,&aparams,cp) ;

	            if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        bclose(afp) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("main: done processing argument files\n") ;
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

	if ((pan == 0) && (afname == NULL)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procfile(pip,&aparams,cp) ;

	}

	bclose(pip->ofp) ;


	ex = EX_OK ;
	if (rs < 0)
		ex = EX_DATAERR ;


/* we are out of here */
done:
retearly:
ret2:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting rs=%d ex=%u\n",rs,ex) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: program exiting ex=%u\n",
	        pip->progname,ex) ;

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

/* bad stuff comes here */
badarg:
	ex = EX_USAGE ;
	bprintf(efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

badargignore:
	ex = EX_USAGE ;
	bprintf(efp,"%s: extra arguments provided\n",
	    pip->progname) ;

	goto ret2 ;

badargextra:
	ex = EX_USAGE ;
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto ret2 ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (rs=%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

badinopen:
	ex = EX_NOINPUT ;
	bprintf(pip->efp,"%s: could not open standard input (rs=%d)\n",
	    pip->progname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



