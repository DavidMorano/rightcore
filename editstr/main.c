/* main */

/* part of the EDITSTR program */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1996-02-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This is a fairly generic front-end subroutine for a program.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<keyopt.h>
#include	<vecstr.h>
#include	<paramopt.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	10000
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 20),2048)
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecti(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	process(PROGINFO *,PARAMOPT *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* global variables */

int	if_int ;


/* local structures */

struct locinfo_flags {
	uint	header : 1 ;
	uint	fmtlong : 1 ;
	uint	fmtshort : 1 ;
	uint	uniq : 1 ;
	uint	users : 1 ;
} ;

struct locinfo {
	struct locinfo_flags	have, f ;
	char			username[USERNAMELEN + 1] ;
} ;


/* forward references */

static int	usage(PROGINFO *) ;
static int	procopts(PROGINFO *,KEYOPT *) ;


/* external variables */


/* local variables */

static const char	*argopts[] = {
	"ROOT",
	"VERSION",
	"VERBOSE",
	"TMPDIR",
	"HELP",
	"pm",
	"sn",
	"option",
	"set",
	"follow",
	"af",
	"of",
	"nice",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_version,
	argopt_verbose,
	argopt_tmpdir,
	argopt_help,
	argopt_pm,
	argopt_sn,
	argopt_option,
	argopt_set,
	argopt_follow,
	argopt_af,
	argopt_of,
	argopt_nice,
	argopt_overlast
} ;

static const char	*progmodes[] = {
	"msgclean",
	NULL
} ;

enum progmodes {
	progmode_msgclean,
	progmode_overlast
} ;

static const char *akonames[] = {
	"header",
	"long",
	"short",
	"uniq",
	"users",
	NULL
} ;

enum akonames {
	akoname_header,
	akoname_long,
	akoname_short,
	akoname_uniq,
	akoname_users,
	akoname_overlast
} ;

static const char	*progopts[] = {
	"follow",
	"nofollow",
	"nostop",
	NULL
} ;

enum progopts {
	progopt_follow,
	progopt_nofollow,
	progopt_nostop,
	progopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;







int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;

	PARAMOPT	aparams ;

	KEYOPT	akopts ;

	bfile	errfile ;
	bfile	outfile ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs, cl, opts ;
	int	ageint = -1 ;
	int	nice = -1 ;
	int	v = -1 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	const char	*pr = NULL ;
	const char	*pmspec = NULL ;
	const char	*searchname = NULL ;
	const char	*afname = NULL ;
	const char	*ofname = NULL ;
	const char	*cp ;


	if_int = 0 ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	proginfo_start(pip,envv,argv[0],VERSION) ;

	proginfo_setbanner(pip,BANNER) ;

	if ((cp = getenv(VARERRORFNAME)) != NULL) {
	    rs = bopen(&errfile,cp,"wca",0666) ;
	} else
	    rs = bopen(&errfile,BFILE_STDERR,"dwca",0666) ;
	if (rs >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* early things to initialize */

	pip->ofp = &outfile ;

	pip->namelen = MAXNAMELEN ;

	pip->verboselevel = 1 ;

	pip->bytes = 0 ;
	pip->megabytes = 0 ;
	pip->progmode = -1 ;
	pip->ageint = -1 ;

/* process program arguments */

	if (rs >= 0) {
	    rs = keyopt_start(&akopts) ;
	    pip->open.akopts = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = paramopt_start(&aparams) ;
	    pip->open.aparams = (rs >= 0) ;
	}

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
		const int	ach = MKCHAR(argp[1]) ;

	        if (isdigitlatin(ach)) {

		    rs = cfdeci((argp + 1),(argl - 1),&argvalue) ;

	        } else if (ach == '-') {

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

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

/* program root */
	                case argopt_root:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pr = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pr = argp ;

	                    }

	                    break ;

/* program mode */
	                case argopt_pm:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            pmspec = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pmspec = argp ;

	                    }

	                    break ;

/* search name */
	                case argopt_sn:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            searchname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            searchname = argp ;

	                    }

	                    break ;

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
	                        if (avl)
	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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

/* the user specified some progopts */
	                case argopt_option:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = paramopt_loads(&aparams,PO_OPTION,
	                            argp,argl) ;

	                    break ;

/* the user specified some progopts */
	                case argopt_set:
	                    if (argr <= 0) {
	                        rs = SR_INVALID ;
	                        break ;
	                    }

	                    argp = argv[++ai] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl)
	                        rs = paramopt_loadu(&aparams,argp,argl) ;

	                    break ;

/* argument files */
	                case argopt_af:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            afname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            afname = argp ;

	                    }

	                    break ;

/* output file */
	                case argopt_of:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            ofname = avp ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            ofname = argp ;

	                    }

	                    break ;

/* nice value */
	                case argopt_nice:
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl, &nice) ;

	                    } else {

	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = cfdeci(argp,argl, &nice) ;

	                    }

	                    break ;

/* follow symbolic links */
	                case argopt_follow:
	                    pip->f.follow = TRUE ;
	                    break ;

/* default action and user specified help */
	                default:
	                    rs = SR_INVALID ;

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

/* follow symbolic links */
	                    case 'f':
	                        pip->f.follow = TRUE ;
	                        break ;

/* file name length restriction */
	                    case 'l':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl)
	                            rs = cfdeci(avp,avl, &pip->namelen) ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

				    if (argl)
	                            rs = cfdeci(argp,argl,&pip->namelen) ;

	                        }

	                        break ;

/* no-change */
	                    case 'n':
	                        pip->f.nochange = TRUE ;
	                        break ;

/* options */
	                    case 'o':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            rs = keyopt_loads(&akopts,argp,argl) ;

	                        break ;

/* recurse down directories */
	                    case 'r':
	                        pip->f.recurse = TRUE ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl) {
	                                rs = cfdeci(avp,avl, &v) ;
				        pip->f.recurse = (v > 0) ? 1 : 0 ;
				    }
	                        }

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

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = paramopt_loads(&aparams,PO_SUFFIX,
	                                    argp,argl) ;

	                        }

	                        break ;

/* age interval */
	                    case 't':
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdecti(avp,avl,&ageint) ;

	                        } else {

	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
	                                break ;
	                            }

	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl)
	                                rs = cfdecti(argp,argl,&ageint) ;

	                        }

	                        break ;

/* verbose output */
	                    case 'v':
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl)
	                                rs = cfdeci(avp,avl,
	                                    &pip->verboselevel) ;

	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                            "%s: invalid option=%c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
	                    if (rs < 0)
	                        break ;

	                } /* end while */

	            } /* end if (individual option key letters) */

	        } /* end if (digits or progopts) */

	    } else {

	        if (ai >= MAXARGINDEX)
	            break ;

	        BASET(argpresent,ai) ;
	        ai_max = ai ;

	    } /* end if (key letter/word or positional) */

	    ai_pos = ai ;

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
	    goto retearly ;

/* get the program root */

	proginfo_setpiv(pip,pr,&initvars) ;

/* program search name */

	proginfo_setsearchname(pip,VARSEARCHNAME,searchname) ;

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

/* get our program mode */

	if (pmspec == NULL)
	    pmspec = pip->searchname ;

	pip->progmode = matstr(progmodes,pmspec,-1) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    if (pip->progmode >= 0)
	        debugprintf("main: progmode=%s(%u)\n",
	            progmodes[pip->progmode],pip->progmode) ;
	    else
	        debugprintf("main: progmode=NONE\n") ;
	}
#endif

	if (pip->progmode < 0)
	    pip->progmode = progmode_msgclean ;

/* help file */

	if (f_help)
	    goto help ;


	ex = EX_OK ;

/* load up the environment options */

	rs = procopts(pip,&akopts) ;

	if (rs < 0) {
	    ex = EX_USAGE ;
	    goto retearly ;
	}

/* check a few more things */

	if (pip->tmpdname == NULL)
	    pip->tmpdname = getenv(VARTMPDNAME) ;

	if (pip->tmpdname == NULL)
	    pip->tmpdname = TMPDNAME ;

/* what about the age interval */

	if ((ageint < 0) && (argvalue >= 0))
		ageint = argvalue ;

	if ((pip->ageint < 0) && (ageint > 0))
		pip->ageint = ageint ;

	if (pip->ageint < 1)
		pip->ageint = AGEINT ;

/* initialize suffix list */

	opts = 0 ;
	rs = vecstr_start(&pip->suffixes,10,opts) ;
	if (rs < 0) {
		ex = EX_OSERR ;
		goto ret1 ;
	}

/* get ready */

	if ((rs = paramopt_havekey(&aparams,PO_SUFFIX)) > 0) {
	    PARAMOPT_CUR	cur ;

	    paramopt_curbegin(&aparams,&cur) ;

	    while (paramopt_enumvalues(&aparams,PO_SUFFIX,&cur,&cp) >= 0) {

		if (cp == NULL) continue ;

	        pip->f.suffix = TRUE ;
		rs = vecstr_add(&pip->suffixes,cp,-1) ;
		if (rs < 0)
			break ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("main: suf> %s\n",cp) ;
#endif /* CF_DEBUG */

	    } /* end while */

	    paramopt_curend(&aparams,&cur) ;

	} /* end if (suffix list) */

	if (rs < 0) {
		ex = EX_OSERR ;
		goto ret2 ;
	}

	if ((rs = paramopt_havekey(&aparams,PO_OPTION)) > 0) {
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

	            case progopt_nostop:
	                pip->f.nostop = TRUE ;
	                break ;

	            } /* end switch */

	        } /* end if (progopts) */

	    } /* end while */

	    paramopt_curend(&aparams,&cur) ;

	} /* end if (progopts) */

/* optionally 'nice' us */

	if (nice > 0)
	    u_nice(nice) ;

/* open output */

	if ((ofname != NULL) && (ofname[0] != '\0'))
	    rs = bopen(pip->ofp,ofname,"wct",0644) ;

	else
	    rs = bopen(pip->ofp,BFILE_STDOUT,"dwct",0644) ;

	if (rs < 0) {

	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open output (%d)\n",
	        pip->progname,rs) ;

	    goto badoutopen ;
	}

/* OK, we do it */

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = process(pip,&aparams,cp) ;

	        if (rs < 0) {

			bprintf(pip->efp,
			"%s: processing error (%d) in file=%s\n",
			pip->progname,rs,cp) ;

			break ;
		}

	} /* end for (looping through requested circuits) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	argfile ;


	    if (strcmp(afname,"-") != 0)
	        rs = bopen(&argfile,afname,"r",0666) ;

	    else
	        rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&argfile,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
	            if (cp[0] == '\0')
	                continue ;

	            pan += 1 ;
	            rs = process(pip,&aparams,cp) ;

	            if (rs < 0) {

			bprintf(pip->efp,
			"%s: processing error (%d) in file=%s\n",
			pip->progname,rs,cp) ;

			break ;
		    }

	        } /* end while (reading lines) */

	        bclose(&argfile) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file (%d)\n",
	                pip->progname,rs) ;

	            bprintf(pip->efp,"%s: \targfile=%s\n",
	                pip->progname,afname) ;

	        }

	    }

	} /* end if (processing file argument file list) */

/* print out options summary */

	if ((rs >= 0) && (pip->verboselevel >= 2)) {

	    bprintf(pip->ofp,"files total=%u scanned=%u processed=%u\n",
		pip->c_total, pip->c_scanned, pip->c_fixed) ;

	} /* end if (verbose output) */

	bclose(pip->ofp) ;

	if ((pan == 0) && (afname == NULL)) {

	    ex = EX_USAGE ;
	    bprintf(pip->efp,"%s: no files or directories specified\n",
	        pip->progname) ;

	    goto badnodirs ;
	}

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: files total=%u scanned=%u processed=%u\n",
		pip->progname,pip->c_total, pip->c_scanned, pip->c_fixed) ;

	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

badnodirs:
badoutopen:
ret2:
	vecstr_finish(&pip->suffixes) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: exiting ex=%u (%d)\n",ex,rs) ;
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

	if (pip->open.akopts)
	    keyopt_finish(&akopts) ;

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

/* print out some help */
help:
	printhelp(NULL,pip->pr,pip->searchname,HELPFNAME) ;

	goto retearly ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



static int usage(pip)
PROGINFO	*pip ;
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<file(s)>] [<dir(s)>[-s <suffix(s)>]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s: \t[-f] [-af {<argfile>|-}] [-v[=n]] [-V]\n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


/* process the program ako-options */
static int procopts(pip,kop)
PROGINFO	*pip ;
KEYOPT		*kop ;
{
	struct locinfo	*lip = pip->lip ;
	KEYOPT_CUR	kcur ;

	int	rs = SR_OK ;
	int	oi ;
	int	kl, vl ;
	int	c = 0 ;

	char	*kp, *vp ;
	char	*cp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_wn/procopts: entered\n") ;
#endif

	if ((cp = getenv(VAROPTS)) != NULL) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_wn/procopts: varopts=>%s<\n",cp) ;
#endif

	    rs = keyopt_loads(kop,cp,-1) ;

	    if (rs < 0)
	        goto ret0 ;

	} /* end if (key options) */

/* process program options */

	keyopt_curbegin(kop,&kcur) ;

	while ((kl = keyopt_enumkeys(kop,&kcur,&kp)) >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_wn/procopts: opt=>%t<\n",kp,kl) ;
#endif

/* get the first value for this key */

	    vl = keyopt_fetch(kop,kp,NULL,&vp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_wn/procopts: opt_value=>%t<\n",vp,vl) ;
#endif

/* do we support this option? */

	    if ((oi = matostr(akonames,3,kp,kl)) >= 0) {

	        uint	uv ;


#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_wn/procopts: opt_oi=%u\n",oi) ;
#endif

	        switch (oi) {

	        case akoname_header:
		    if (! lip->have.header) {

	            lip->have.header = TRUE ;
	            lip->f.header = TRUE ;
	            if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                lip->f.header = (uv > 0) ? 1 : 0 ;

		    }

	            break ;

	        case akoname_long:
		    if (! lip->have.fmtlong) {

	            lip->have.fmtlong = TRUE ;
	            lip->f.fmtlong = TRUE ;
	            if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                lip->f.fmtlong = (uv > 0) ? 1 : 0 ;

		    }

	            break ;

	        case akoname_short:
		    if (! lip->have.fmtshort) {

	            lip->have.fmtshort = TRUE ;
	            lip->f.fmtshort = TRUE ;
	            if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                lip->f.fmtshort = (uv > 0) ? 1 : 0 ;

		    }

	            break ;

	        case akoname_uniq:
		    if (! lip->have.uniq) {

	            lip->have.uniq = TRUE ;
	            lip->f.uniq = TRUE ;
	            if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                lip->f.uniq = (uv > 0) ? 1 : 0 ;

		    }

	            break ;

	        case akoname_users:
		    if (! lip->have.users) {

	            lip->have.users = TRUE ;
	            lip->f.users = TRUE ;
	            if ((vl > 0) && (cfdecui(vp,vl,&uv) >= 0))
	                lip->f.users = (uv > 0) ? 1 : 0 ;

		    }

	            break ;

	        } /* end switch */

	        c += 1 ;

	    } /* end if (valid option) */

	} /* end while (looping through key options) */

	keyopt_curend(kop,&kcur) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("b_wn/procopts: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procopts) */



