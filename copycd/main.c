/* main */

/* part of the COPYCD program */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 96/02/01, David A­D­ Morano

	The program was written from scratch for reading certain
	CDROMs that (although are supposed to be in ISO-9660 format)
	have weirdo directory names and file names that wreak
	havoc on UNIX !


*/



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
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"paramopt.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	MAXARGINDEX	600
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)



/* external subroutines */

extern int	matstr3(char **,char *,int) ;
extern int	cfdeci(char *,int,int *) ;

extern char	*strbasename(char *), *strshrink(char *) ;


/* local forward references */


/* external variables */


/* global variables */


/* local variables */

static char *argopts[] = {
	    "VERSION",
	    "VERBOSE",
	    "TMPDIR",
	    "option",
	    "set",
	    "follow",
	    NULL,
} ;

#define	ARGOPT_VERSION		0
#define	ARGOPT_VERBOSE		1
#define	ARGOPT_TMPDIR		2
#define	ARGOPT_OPTION		3
#define	ARGOPT_SET		4
#define	ARGOPT_FOLLOW		5


static const char	*progopts[] = {
	    "follow",
	    "nofollow",
	    NULL
} ;


#define	OPTION_FOLLOW		0
#define	OPTION_NOFOLLOW		1






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	argfile, *afp = &argfile ;

	struct proginfo	pi, *pip = &pi ;

	struct ustat	sb ;

	PARAMOPT	aparams ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, i ;
	int	fd_debug = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*srcdir = NULL ;
	char	*dstdir = NULL ;
	char	*cp ;


	if (((cp = getenv("ERROR_FD")) != NULL) &&
	    (cfdeci(cp,-1,&err_fd) >= 0))
	    debugsetfd(err_fd) ;


	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;


	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0)
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}


/* early things to initialize */

	pip->ofp = ofp ;
	pip->debuglevel = 0 ;
	pip->tmpdname = NULL ;
	pip->namelen = MAXNAMELEN ;

	pip->bytes = 0 ;
	pip->megabytes = 0 ;

	(void) memset(&pip->f,0,sizeof(struct proginfo_flags)) ;

	pip->f.verbose = FALSE ;
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
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

/* verbose */
	                case ARGOPT_VERBOSE:
	                    pip->f.verbose = TRUE ;
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

/* temporary directory */
	                case ARGOPT_TMPDIR:
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

/* the user specified some options */
	                case ARGOPT_OPTION:
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = paramopt_loads(&aparams,PO_OPTION,
	                            argp,argl) ;

	                    break ;

/* the user specified some options */
	                case ARGOPT_SET:
	                    if (argr <= 0) 
				goto badargnum ;

	                    argp = argv[++i] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = paramopt_loadu(&aparams,argp,argl) ;

	                    break ;

/* follow symbolic links */
	                case ARGOPT_FOLLOW:
				pip->f.follow = TRUE ;
				break ;

/* default action and user specified help */
	                default:
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
	                            if (cfdeci(avp,avl, &pip->debuglevel) < 0)
	                                goto badargvalue ;

	                        }

	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (cfdeci(avp,avl, &pip->namelen) < 0)
	                                goto badargvalue ;

	                        } else {

	                            if (argr <= 0) 
					goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (cfdeci(argp,argl,&pip->namelen) < 0)
	                                goto badargvalue ;

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

	                            if (argr <= 0) 
					goto badargnum ;

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

	                    default:
	                        bprintf(efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    case '?':
	                        f_usage = TRUE ;

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
	debugprintf("main: npa=%d\n",npa) ;
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

	if (paramopt_havekey(&aparams,PO_OPTION) >= 0) {

	    PARAMOPT_CUR	cur ;


	    paramopt_curbegin(&aparams,&cur) ;

	    while (paramopt_enumvalues(&aparams,PO_OPTION,&cur,&cp) >= 0) {

		if (cp == NULL) continue ;

	        if ((kwi = matstr(progopts,cp,-1)) >= 0) {

	            switch (kwi) {

	            case OPTION_FOLLOW:
	                pip->f.follow = TRUE ;
	                break ;

	            case OPTION_NOFOLLOW:
	                pip->f.follow = FALSE ;
	                break ;

	            } /* end switch */

	        } /* end if (options) */

	    } /* end while */

	    paramopt_curend(&aparam,&cur) ;

	} /* end if (options) */


/* OK, we do it */

	pan = 1 ;
	if (npa > 0) {

	    for (i = 0 ; i <= maxai ; i += 1) {

	        if ((! BATST(argpresent,i)) &&
	            (argv[i][0] != '\0')) continue ;

		switch (pan) {

		case 1:
			srcdir = argv[i] ;
			break ;

		case 2:
			dstdir = argv[i] ;
			break ;

		} /* end switch */

	        pan += 1 ;

	    } /* end for (looping through requested circuits) */

	} /* end if */


/* check some arguments */

	if ((pan == 0) || (srcdir == NULL) || (dstdir == NULL)) {

	    goto badnodirs ;
	}

	if ((srcdir[0] == '\0') || (dstdir[0] == '\0'))
	    goto badnodirs ;

/* does the source directory exist? */

	rs = perm(srcdir,-1,-1,NULL,X_OK | R_OK) ;

	if (rs < 0)
		goto baddir ;

	if (((rs = u_stat(srcdir,&sb)) < 0) ||
		(! S_ISDIR(sb.st_mode)))
		goto baddir ;

/* does the destination directory exist? */

	rs = perm(dstdir,-1,-1,NULL,X_OK | R_OK) ;

	if (rs < 0)
		goto baddir ;

	if (((rs = u_stat(dstdir,&sb)) < 0) ||
		(! S_ISDIR(sb.st_mode)))
		goto baddir ;


/* we're starting off ! */




/* we're getting out */

	if ((rs = bopen(ofp,BFILE_STDOUT,"dwct",0644)) >= 0) {

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

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("main: exiting\n") ;
#endif

/* good return from program */
goodret:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

#if	CF_DEBUG
	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: program exiting ex=%u\n",
	        pip->progname,ex) ;
#endif

/* we are out of here */
retearly:
ret1:
	if (pip->open.aparams)
	paramopt_finish(&aparams);

	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

	return ex ;

/* program usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [directory(s) ...] [-s suffixes] [-Vv]\n",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp,
	    "\t[-f {argfile|-}]\n") ;

	ex = EX_INFO ;
	goto retearly ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    pip->progname) ;

	goto badarg ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    pip->progname) ;

	goto badarg ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badnodirs:
	bprintf(pip->efp,"%s: insufficient directories were specified\n",
	    pip->progname) ;

	goto badarg ;

baddir:
	bprintf(pip->efp,"%s: error accessing specified directory (%d)\n",
	    pip->progname,rs) ;

	goto badarg ;

/* come here for a bad return from the program */
badarg:

#if	CF_DEBUGS
	debugprintf("main: exiting program BAD\n") ;
#endif

	ex = EX_USAGE ;
	goto retearly ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output (%d)\n",
	    pip->progname,rs) ;

	goto retearly ;

}
/* end subroutine (main) */



