/* main */

/* separate the individual theses */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_NOFILL	0		/* turn on no-fill mode */
#define	CF_LEADER	0		/* print leader */


/* revision history:

	= 1997-09-10, David A­D­ Morano

	This subroutine was originally written but it was probably started
	from any one of the numerous subroutine which perform a similar
	"file-processing" fron end.


*/

/* Copyright © 1997 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This program will read the input file and format it into
	'troff' constant width font style source input language.

	Synopsis:

	$ cookieset <input_file> > <out.dwb>


*********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<estrings.h>
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

#define	BUFLEN		(MAXPATHLEN + (2 * LINEBUFLEN))

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isdigitlatin(int) ;

extern int	proginfo_setpiv(PROGINFO *,cchar *,const struct pivars *) ;
extern int	printhelp(void *,const char *,const char *,const char *) ;
extern int	procfile(PROGINFO *,char *,int,int) ;


/* external variables */


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
	"af",
	NULL
} ;

enum argopts {
	argopt_root,
	argopt_debug,
	argopt_version,
	argopt_verbose,
	argopt_help,
	argopt_af,
	argopt_overlast
} ;

static const struct pivars	initvars = {
	VARPROGRAMROOT1,
	VARPROGRAMROOT2,
	VARPROGRAMROOT3,
	PROGRAMROOT,
	VARPRLOCAL
} ;

static const struct mapex	mapexs[] = {
	{ SR_AGAIN, EX_TEMPFAIL },
	{ SR_DEADLK, EX_TEMPFAIL },
	{ SR_NOLCK, EX_TEMPFAIL },
	{ SR_TXTBSY, EX_TEMPFAIL },
	{ SR_ACCESS, EX_NOPERM },
	{ SR_REMOTE, EX_PROTOCOL },
	{ SR_NOSPC, EX_TEMPFAIL },
	{ 0, 0 }
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	PROGINFO	pi, *pip = &pi ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	argr, argl, aol, avl, kwi ;
	int	ai, ai_max, ai_pos ;
	int	argvalue = -1 ;
	int	pan ;
	int	rs ;
	int	i, sl ;
	int	pages = 0 ;
	int	maxlines = DEFMAXLINES ;
	int	blanklines = DEFBLANKLINES ;
	int	coffset = 0 ;
	int	xoffset = 0 ;
	int	yoffset = 0 ;
	int	pointlen ;
	int	ps = DEFPOINT ;
	int	vs = 0 ;
	int	ex = EX_INFO ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_help = FALSE ;
	int	f ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGGROUPS] ;
	char	blanks[LINEBUFLEN + 1] ;
	char	blankstring[MAXBLANKLINES + 1] ;
	const char	*pr = NULL ;
	const char	*afname = NULL ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*pointstring  = NULL ;
	const char	*fontname = "CW" ;
	const char	*cp ;


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

/* other initialization */

	pip->ofp = ofp ;

	pip->verboselevel = 1 ;

/* process program arguments */

	rs = SR_OK ;
	for (ai = 0 ; ai < NARGGROUPS ; ai += 1)
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

	            rs = cfdeci((argp + 1),(argl - 1),&maxlines) ;

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

/* debug level */
	                case argopt_debug:
	                    pip->debuglevel = 1 ;
	                    if (f_optequal) {

	                        f_optequal = FALSE ;
	                        if (avl)
	                            rs = cfdeci(avp,avl, &pip->debuglevel) ;

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

	                            rs = cfdeci(avp,avl,
	                                &pip->verboselevel) ;

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

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

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

	                    case 'Q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* program-root */
	                    case 'R':
	                        if (argr <= 0) {
	                            rs = SR_INVALID ;
	                            break ;
	                        }

	                        argp = argv[++ai] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl)
	                            pr = argp ;

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

	                            rs = cfdeci(argp,argl,&blanklines) ;

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

	                            rs = cfdeci(argp,argl,&maxlines) ;

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

	                            rs = cfdeci(argp,argl,&coffset) ;

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

	                    case 'q':
	                        pip->verboselevel = 0 ;
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

	                            rs = cfdeci(argp,argl,&vs) ;

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

	                            rs = cfdeci(argp,argl,&xoffset) ;

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

	                            rs = cfdeci(argp,argl,&yoffset) ;

	                        }

	                        break ;

/* print a brief usage summary (and then exit!) */
	                    case '?':
	                        f_usage = TRUE ;
	                        break ;

	                    default:
	                        rs = SR_INVALID ;
	                        bprintf(pip->efp,
	                            "%s : unknown option - %c\n",
	                            pip->progname,*akp) ;

	                    } /* end switch */

	                    akp += 1 ;
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

#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",pip->debuglevel) ;
#endif

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: debuglevel=%u\n",
	        pip->progname,pip->debuglevel) ;


/* continue w/ the trivia argument processing stuff */

	if (f_version)
	    bprintf(pip->efp,"%s: version %s\n",
	        pip->progname,VERSION) ;

	if (f_usage)
	    goto usage ;

	if (f_version)
	    goto retearly ;


/* get our program root (if we have one) */

	rs = proginfo_setpiv(pip,pr,&initvars) ;

	if (rs < 0) {
	    ex = EX_OSERR ;
	    goto retearly ;
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

/* how many lines per page */

	if (maxlines < 1)
	    maxlines = DEFMAXLINES ;

	if (maxlines > MAXLINES)
	    maxlines = MAXLINES ;

	pip->maxlines = maxlines ;

/* establish an offset if any */

	if (coffset < 0)
	    coffset = 0 ;

	else if (coffset > LINEBUFLEN)
	    coffset = LINEBUFLEN ;

	pip->coffset = coffset ;

/* establish working point size and vertical spacing */

	if (pointstring != NULL) {

	    i = substring(pointstring,pointlen,".") ;

	    if (i < 0)
	        i = substring(pointstring,pointlen,"/") ;

	    if (i >= 0) {

	        if ((i > 0) && (cfdeci(pointstring,i,&ps) < 0))
	            goto badarg ;

	        if ((pointlen - i) > 1)
	            if (cfdeci(pointstring + i + 1,pointlen - i - 1,&vs) < 0)
	                goto badarg ;

	    } else {

	        if (cfdeci(pointstring,pointlen,&ps) < 0)
	            goto badarg ;

	    }

	} /* end if (handling the 'pointstring') */

	if (ps < 2)
	    ps = 6 ;

	if (vs == 0)
	    vs = ps + 2 ;

	else if (vs < ps)
	    vs = ps + 1 ;

	if (pip->debuglevel > 0)
	    bprintf(pip->efp, "%s: ps %d - vs %d\n",
	        pip->progname,ps,vs) ;


/* open output file */

	if ((ofname == NULL) || (ofname[0] == '-'))
	    rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	else
	    rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0) {
	    ofp = NULL ;
	    ex = EX_CANTCREAT ;
	    bprintf(pip->efp,"%s: could not open output file (%d)\n",
	        pip->progname,pip->progname,rs) ;

	    goto badoutopen ;
	}

/* perform initialization processing */

	for (i = 0 ; i < LINEBUFLEN ; i += 1)
	    blanks[i] = ' ' ;

	pip->blanks = blanks ;

	cp = blankstring ;
	for (i = 0 ; (i < blanklines) && (i < MAXBLANKLINES) ; i += 1)
	    *cp++ = '\n' ;

	*cp = '\0' ;
	pip->blankstring = blankstring ;

#if	CF_LEADER

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

#endif /* CF_LEADER */

	bprintf(ofp,".nr iJ 5\n") ;

	bprintf(ofp,".if \\n(iI>0 .nr iJ \\n(iI\n") ;


/* processing the input file arguments */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("main: checking for positional arguments\n") ;
#endif

	pan = 0 ;

	cp = NULL ;
	for (ai = 1 ; ai < argc ; ai += 1) {

	    f = (ai <= ai_max) && BATST(argpresent,ai) ;
	    f = f || (ai > ai_pos) ;
	    if (! f) continue ;

	    cp = argv[ai] ;
	    pan += 1 ;
	    rs = procfile(pip,argv[ai],(pan + 1),(pages & 1)) ;

	    pages += rs ;
	    if (rs < 0)
	        break ;

	} /* end for (loading positional arguments) */

/* process any files in the argument filename list file */

	if ((rs >= 0) && (afname != NULL) && (afname[0] != '\0')) {

	    bfile	argfile ;


	    cp = NULL ;
	    if (strcmp(afname,"-") == 0)
	        rs = bopen(&argfile,BFILE_STDIN,"dr",0666) ;

	    else
	        rs = bopen(&argfile,afname,"r",0666) ;

	    if (rs >= 0) {

	        int	len ;

	        char	linebuf[LINEBUFLEN + 1] ;


	        while ((rs = breadline(&argfile,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            if (linebuf[len - 1] == '\n')
	                len -= 1 ;

	            linebuf[len] = '\0' ;
	            cp = linebuf ;

	            if ((cp[0] == '\0') || (cp[0] == '#'))
	                continue ;

	            pan += 1 ;
	            rs = procfile(pip,cp,(pan + 1),(pages & 1)) ;

	            pages += rs ;
	            if (rs < 0)
	                break ;

	        } /* end while (reading lines) */

	        bclose(&argfile) ;

	    } else {

	        if (! pip->f.quiet) {

	            bprintf(pip->efp,
	                "%s: could not open the argument list file\n",
	                pip->progname) ;

	            bprintf(pip->efp,"%s: \trs=%d argfile=%s\n",
	                pip->progname,rs,afname) ;

	        }

	    }

	} /* end if (processing file argument file list) */

	if ((rs >= 0) && (pan == 0)) {

	    cp = "-" ;
	    pan += 1 ;
	    rs = procfile(pip,cp,(pan + 1),(pages & 1)) ;

	    pages += rs ;

	} /* end if */

	if (ofp != NULL)
	    bclose(ofp) ;

	if ((rs < 0) && (! pip->f.quiet)) {

	    const char	*fmt ;


	    if (cp != NULL) {

	        if (*cp == '-')
	            cp = "*STDIN*" ;

	        bprintf(pip->efp,
	            "%s: error on file=%s (%d)\n",
	            pip->progname,cp,rs) ;

	    } else {

	        bprintf(pip->efp,
	            "%s: error on argfile=%s (%d)\n",
	            pip->progname,afname,rs) ;

	    }

	} /* end if (error report) */

/* finish */
badoutopen:
done:
ret5:
	if ((rs < 0) && (ex == EX_OK)) {

	    switch (rs) {

	    case SR_INVALID:
	        ex = EX_USAGE ;
	        break ;

	    case SR_NOENT:
	        ex = EX_CANTCREAT ;
	        break ;

	    case SR_AGAIN:
	        ex = EX_TEMPFAIL ;
	        break ;

	    default:
	        ex = mapex(mapexs,rs) ;

	        break ;

	    } /* end switch */

	} /* end if */

/* let's get out of here! */
ret2:
	if (pip->debuglevel > 0) {

	    bprintf(pip->efp,"%s: files processed - %d\n",
	        pip->progname,pan) ;

	    bprintf(pip->efp,"%s: pages processed - %d\n",
	        pip->progname,pages) ;

	}

/* close off and get out! */
exit:
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	proginfo_finish(pip) ;

#if	(CF_DEBUGS || CF_DEBUG)
	debugclose() ;
#endif

	return ex ;

/* what are we about? */
usage:
	usage(pip) ;

	goto retearly ;

help:
	printhelp(NULL,pip->pr,SEARCHNAME,pip->helpfname) ;

	goto retearly ;

/* bad stuff */
badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: invalid argument specified (%d)\n",
	    pip->progname,rs) ;

	usage(pip) ;

	goto retearly ;

}
/* end subroutine (main) */


/* local subroutines */


static int usage(PROGINFO *pip)
{
	int	rs ;
	int	wlen ;


	wlen = 0 ;
	rs = bprintf(pip->efp,
	    "%s: USAGE> %s [<file(s)> ...]\n",
	    pip->progname,pip->progname) ;

	wlen += rs ;
	rs = bprintf(pip->efp,
	    "%s:\t [-l <lines>] [-<lines>] [-w <outfile>] \n",
	    pip->progname) ;

	wlen += rs ;
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (usage) */



