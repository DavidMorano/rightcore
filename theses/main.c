/* main */

/* separate the individual theses */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* runt-time debugging */
#define	CF_NOFILL	0		/* turn on no-fill mode */


/* revision history:

	= 1997-09-10, David A.D. Morano

	This subroutine was originally written but it was probably started from
	any one of the numerous subroutine which perform a similar
	"file-processing" fron end.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will read the input file and format it into 'troff'
	constant width font style source input language.

	Synopsis:

	$ theses [<input_file>]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	MAXARGINDEX	100
#define	NARGGROUPS	(MAXARGINDEX/8 + 1)
#define	DEFMAXLINES	66
#define	MAXLINES	180
#define	DEFPOINT	10

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + (2 * LINEBUFLEN))
#endif


/* external subroutines */

extern int	optvalue(cchar *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	substring(const char *,int,const char *) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;

extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;

extern int	progfile(PROGINFO *,const char *,int,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	usage(PROGINFO *) ;


/* local variables */

static const char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	"HELP",
	"sn",
	"af",
	"ef",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_sn,
	argopt_af,
	argopt_ef,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRNAME
} ;

static const struct mapex	mapexs[] = {
	{ SR_NOENT, EX_NOUSER },
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ SR_INTR, EX_INTR },
	{ SR_EXIT, EX_TERM },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(int argc,cchar *argv[],cchar *envv[])
{
	PROGINFO	pi, *pip = &pi ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;

	int	argr, argl, aol, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	pan ;
	int	rs ;
	int	i ;
	int	v ;
	int	pages = 0 ;
	int	maxlines = DEFMAXLINES ;
	int	blanklines = DEFBLANKLINES ;
	int	coffset = 0 ;
	int	xoffset = 0 ;
	int	yoffset = 0 ;
	int	ps = DEFPOINT ;
	int	vs = 0 ;
	int	pointlen ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *avp ;
	const char	*argval = NULL ;
	const char	*pr = NULL ;
	const char	*sn = NULL ;
	const char	*afname = NULL ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;
	const char	*fontname = "CW" ;
	const char	*pointstring  = NULL ;
	const char	*cp ;
	char	argpresent[NARGGROUPS] ;
	char	blanks[LINEBUFLEN + 1] ;
	char	blankstring[MAXBLANKLINES + 1] ;


#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	rs = proginfo_start(pip,envv,argv[0],VERSION) ;
	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto badprogstart ;
	}

	if ((cp = getourenv(envv,VARBANNER)) == NULL) cp = BANNER ;
	proginfo_setbanner(pip,cp) ;

	pip->efp = NULL ;
	if (bopen(&errfile,BFILE_STDERR,"wca",0666) >= 0) {
	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

	pip->ofp = ofp ;

	pip->helpfname = NULL ;
	pip->headerstring = NULL ;
	pip->verboselevel = 1 ;

	pip->f.headers = FALSE ;

	f_help = FALSE ;

/* process program arguments */

	for (ai = 0 ; ai < NARGGROUPS ; ai += 1) 
		argpresent[ai] = 0 ;

	ai = 0 ;
	ai_max = 0 ;
	ai_pos = 0 ;
	argr = argc ;
	for (ai = 0 ; (ai < argc) && (argv[ai] != NULL) ; ai += 1) {
	    if (rs < 0) break ;
	    argr -= 1 ;
	    if (ai == 0) continue ;

	    argp = argv[ai] ;
	    argl = strlen(argp) ;

	    f_optminus = (*argp == '-') ;
	    f_optplus = (*argp == '+') ;
	    if ((argl > 1) && (f_optminus || f_optplus)) {

	        if (isdigit(argp[1])) {

	            argval = (argp + 1) ;

	        } else if (argp[1] == '-') {

	            ai_pos = ai ;
	            break ;

		} else {

	                aop = argp + 1 ;
	                aol = argl - 1 ;
	                f_optequal = FALSE ;
	                if ((avp = strchr(aop,'=')) != NULL) {

	                    aol = avp - aop ;
	                    avp += 1 ;
	                    avl = aop + argl - 1 - avp ;
	                    f_optequal = TRUE ;

	                } else
	                    avl = 0 ;

/* do we have a keyword match or only key letters? */

	                if ((kwi = matostr(argopts,2,aop,aol)) >= 0) {

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

/* debug level */
	                    case argopt_debug:
	                        pip->debuglevel = 1 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                    rs = optvalue(avp,avl) ;
	                                    pip->debuglevel = rs ;
	                            }
	                        }
	                        break ;

	                    case argopt_version:
	                        f_version = TRUE ;
	                        break ;

	                    case argopt_verbose:
	                        pip->verboselevel = 2 ;
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl) {
	                                rs = optvalue(avp,avl) ;
	                                pip->verboselevel = rs ;
	                            }
	                        }
	                        break ;

/* help file */
	                    case argopt_help:
	                        if (f_optequal) {
	                            f_optequal = FALSE ;
	                            if (avl)
	                                pip->helpfname = avp ;
	                        }
	                        f_help  = TRUE ;
	                        break ;

/* program search-name */
	                case argopt_sn:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            sn = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            sn = argp ;
	                    }
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

/* error file name */
	                case argopt_ef:
	                    if (f_optequal) {
	                        f_optequal = FALSE ;
	                        if (avl)
	                            efname = avp ;
	                    } else {
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }
	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;
	                        if (argl)
	                            efname = argp ;
	                    }
	                    break ;

			    default:
				rs = SR_INVALID ;
				break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (aol--) {
			        const int	kc = MKCHAR(*akp) ;

	                        switch (kc) {

	                        case 'D':
	                            pip->debuglevel = 1 ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
	                                if (avl) {
	                                    rs = optvalue(avp,avl) ;
	                                    pip->debuglevel = rs ;
	                                }
	                            }
	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* blank lines at the top of a page */
	                        case 'b':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
					blanklines = v ;
	                            }
	                            break ;

	                        case 'w':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                ofname = argp ;
	                            break ;

	                        case 'f':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl)
	                                fontname = argp ;
	                            break ;

/* output page headers */
	                        case 'h':
	                            pip->f.headers = TRUE ;
	                            if (f_optequal) {
	                                f_optequal = FALSE ;
	                                if (avl)
	                                    pip->headerstring = avp ;
	                            }
	                            break ;

	                        case 'l':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
					maxlines = v ;
	                            }
	                            break ;

	                        case 'o':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
					coffset = v ;
	                            }
	                            break ;

	                        case 'p':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                pointstring = argp ;
	                                pointlen = argl ;
	                            }
	                            break ;

	                        case 'v':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = optvalue(argp,argl) ;
					vs = rs ;
	                            }
	                            break ;

	                        case 'x':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
					xoffset = v ;
	                            }
	                            break ;

	                        case 'y':
	                            if (argr <= 0) {
	                                rs = SR_INVALID ;
					break ;
				    }
	                            argp = argv[++ai] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;
	                            if (argl) {
	                                rs = cfdeci(argp,argl,&v) ;
					yoffset = v ;
	                            }
	                            break ;

	                        case '?':
	                            f_usage = TRUE ;
	                            break ;

	                        default:
	                            rs = SR_INVALID ;
	                            break ;

	                        } /* end switch */

	                        aop += 1 ;
				if (rs < 0)
					break ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

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
	if (pip->debuglevel > 1)
	    debugprintf("main: finished parsing arguments\n") ;
#endif

/* continue w/ the trivia argument processing stuff */

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;
	}

/* get the program root */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs >= 0)
	    rs = proginfo_setsearchname(pip,VARSEARCHNAME,sn) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
	}

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: pr=%s\n",pip->progname,pip->pr) ;
	    bprintf(pip->efp,"%s: sn=%s\n",pip->progname,pip->searchname) ;
	} /* end if */

	if (f_usage)
	    usage(pip) ;

/* help file */

	if (f_help)
	    printhelp(NULL,pip->pr,pip->searchname,pip->helpfname) ;

	if (f_version || f_help || f_usage)
	    goto retearly ;


	ex = EX_OK ;

/* how many lines per page */

	if (maxlines < 1) maxlines = DEFMAXLINES ;

	if (maxlines > MAXLINES) maxlines = MAXLINES ;

	pip->maxlines = maxlines ;

/* establish an offset if any */

	if (coffset < 0) {
	    coffset = 0 ;
	} else if (coffset > LINEBUFLEN)
	    coffset = LINEBUFLEN ;

	pip->coffset = coffset ;

/* establish working point size and vertical spacing */

	if (pointstring != NULL) {

	    i = substring(pointstring,pointlen,".") ;

	    if (i < 0)
	        i = substring(pointstring,pointlen,"/") ;

	    if (i >= 0) {

	        if (i > 0)
			rs = cfdeci(pointstring,i,&ps) ;

	        if (((pointlen - i) > 1) && (rs >= 0))
	            rs = cfdeci((pointstring + i + 1),(pointlen - i - 1),&vs) ;

	    } else {
	        rs = cfdeci(pointstring,pointlen,&ps) ;
	    }

	} /* end if (handling the 'pointstring') */

	if (ps < 2)
	    ps = 6 ;

	if (vs == 0) {
	    vs = ps + 2 ;
	} else if (vs < ps)
	    vs = ps + 1 ;

	if (rs < 0)
		goto badarg ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp, "%s: ps %d - vs %d\n",
	        pip->progname,ps,vs) ;
	}

/* perform initialization processing */

	for (i = 0 ; i < LINEBUFLEN ; i += 1)
	    blanks[i] = ' ' ;

	pip->blanks = blanks ;

	{
	    char	*bp = blankstring ;
	    for (i = 0 ; (i < blanklines) && (i < MAXBLANKLINES) ; i += 1)
	        *bp++ = '\n' ;
	    *bp = '\0' ;
	}

	pip->blankstring = blankstring ;

/* open output file */

	if ((ofname != NULL) && (ofname[0] != '-')) {
	    rs = bopen(ofp,ofname,"wct",0666) ;
	} else
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	    ofp = NULL ;
	    goto badoutfile ;
	}

/* output the header stuff */

#if	CF_NOFILL
	bprintf(ofp,".nf\n") ;
#endif

	bprintf(ofp,".fp 5 %s\n",fontname) ;

	bprintf(ofp,".ft %s\n",fontname) ;

/* specify "no header" */

	bprintf(ofp,".nr N 4\n") ;

/* change to running point size */

	bprintf(ofp,".S %d\n",ps) ;

#ifdef	COMMENT
	bprintf(ofp,".ps %d\n",ps) ;

	bprintf(ofp,".vs %d\n",vs) ;
#endif

/* processing the input file arguments */

#if	CF_DEBUG
	if (pip->debuglevel > 0) 
		debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || ((ai > ai_pos) && (argv[ai] != NULL)) ;
	    if (! f) continue ;

	            rs = progfile(pip,argv[ai],pan,(pages & 1)) ;

	            if (rs < 0) {

	                if (pip->debuglevel > 0)
	                    bprintf(pip->efp,
	                        "%s: error processing file \"%s\" (%d)\n",
	                        pip->progname,argv[ai],rs) ;

	            } else
	                pages += rs ;

	            pan += 1 ;
	    } /* end for (loading positional arguments) */

	if ((rs >= 0) && (pan == 0)) {

	    rs = progfile(pip,"-",pan,(pages & 1)) ;

	    if (rs < 0) {

	                if (pip->debuglevel > 0)
	            bprintf(pip->efp,
	                        "%s: error processing file \"%s\" (%d)\n",
	                        pip->progname,"STDIN",rs) ;

	    } else
	        pages += rs ;

	    pan += 1 ;
	} /* end if */

	if (ofp != NULL) {
	    bclose(ofp) ;
	}

done:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: files processed=%u\n",
	        pip->progname,pan) ;
	    bprintf(pip->efp,"%s: pages processed=%u\n",
	        pip->progname,pages) ;
	}

retearly:
ret1:
	bclose(pip->efp) ;

	proginfo_finish(pip) ;

badprogstart:

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
		pip->progname,rs) ;
	usage(pip) ;
	goto ret1 ;

badoutfile:
	ex = EX_CANTCREAT ;
	bprintf(pip->efp,"%s: could not open output file (%d)\n",
	    pip->progname,pip->progname,rs) ;

	goto ret1 ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

	fmt = "%s: USAGE> %s [-l <lines>] [-<lines>] [<ifile(s)>]\n" ;
	rs = bprintf(pip->efp,fmt,pn,pn) ;
	wlen += rs ;

	fmt = "  [-w <ofile>]\n" ;
	rs = bprintf(pip->efp,fmt,pn) ;
	wlen += rs ;

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */


