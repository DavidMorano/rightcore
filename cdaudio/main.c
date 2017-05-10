/* main */

/* get information from a CDROM disk containing audio information */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-03-01, David A­D­ Morano


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This program will read the input file and format it into
	'troff' constant width font style source input language.

	Synopsis:

	$0 [input_file [outfile]] [-DV] [-o offset] [-l lines] 
		[-f font] [-p point_size] [-v vertical_space]


*********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	MAXARGINDEX	100

#define	NARGPRESENT	(MAXARGINDEX/8 + 1)
#define	DEFMAXLINES	66
#define	MAXLINES	180
#define	LINELEN		200
#define	BUFLEN		(LINELEN + LINELEN)
#define	DEFPOINT	10


/* externals */

extern int	cfdec() ;
extern int	isdigitlatin(int) ;


/* forward references */

extern int	expandline() ;


/* local structures */


/* global data */

struct global		g ;


/* local data */

/* define command option words */

static char *argopts[] = {
	"ROOT",
	"DEBUG",
	"VERSION",
	"VERBOSE",
	NULL,
} ;

#define	ARGOPT_ROOT		0
#define	ARGOPT_DEBUG		1
#define	ARGOPT_VERSION		2
#define	ARGOPT_VERBOSE		3


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		errfile, *efp = &errfile ;

	struct ustat	sb ;

	int	argr, argl, aol, avl ;
	int	maxai, pan, npa, kwi, i ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_extra = FALSE ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	page, line, len, l, rs ;
	int	len2 ;
	int	maxlines = DEFMAXLINES ;
	int	blanklines = DEFBLANKLINES ;
	int	coffset = 0 ;
	int	xoffset = 0 ;
	int	yoffset = 0 ;
	int	ps = DEFPOINT, vs = 0 ;
	int	pointlen ;
	int	f_pagebreak = FALSE ;
	int	f_headers = FALSE ;
	int	f_stdinput = FALSE ;
	int	f_escape ;

	const char	*argp, *aop, *avp ;
	char	argpresent[NARGPRESENT] ;
	char	linebuf[LINELEN + 1], *lbp ;
	char	buf[BUFLEN + 1] ;
	char	headline[LINELEN + 1] ;
	char	blanks[LINELEN + 1] ;
	char	blankstring[MAXBLANKLINES + 1] ;
	char	timebuf[100] ;
	const char	*fontname = "CW" ;
	const char	*ifname = NULL ;
	const char	*ofname = NULL ;
	const char	*pointstring  = NULL ;
	const char	*headerstring = NULL ;
	const char	*cp ;


	g.progname = argv[0] ;
	bopen(efp,BFILE_STDERR,"wca",0666) ;

	g.efp = efp ;
	g.ofp = ofp ;
	g.ifp = ifp ;
	g.debuglevel = 0 ;
	g.programroot = NULL ;

	g.f.verbose = FALSE ;

/* process program arguments */

	for (i = 0 ; i < NARGPRESENT ; i += 1) argpresent[i] = 0 ;

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
		const int	ach = MKCHAR(argp[1]) ;

	        if (argl > 1) {

	             if (isdigitlatin(ach)) {

	                if (cfdec(argp + 1,argl - 1,&maxlines))
	                    goto badargvalue ;

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

/* do we have a keyword match or should we assume only key letters? */

	                if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                    switch (kwi) {

/* program root */
	                    case ARGOPT_ROOT:
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
	                            if (avl) g.programroot = avp ;

	                        } else {

	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) g.programroot = argp ;

	                        }

	                        break ;

/* debug level */
	                    case ARGOPT_DEBUG:
	                        g.debuglevel = 1 ;
	                        if (f_optequal) {

#if	CF_DEBUGS
	                            debugprintf(
	                                "main: debug flag, avp=\"%W\"\n",
	                                avp,avl) ;
#endif

	                            f_optequal = FALSE ;
	                            if ((avl > 0) &&
	                                (cfdec(avp,avl,
	                                &g.debuglevel) < 0))
	                                goto badargvalue ;

	                        }

	                        break ;

	                    case ARGOPT_VERSION:
	                        f_version = TRUE ;
	                        break ;

	                    case ARGOPT_VERBOSE:
	                        g.f.verbose = TRUE ;
	                        break ;

	                    } /* end switch (key words) */

	                } else {

	                    while (akl--) {

	                        switch ((int) *akp) {

	                        case 'D':
	                            g.debuglevel = 1 ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
	                                if (cfdec(avp,avl, &g.debuglevel) != OK)
	                                    goto badargvalue ;

	                            }

	                            break ;

	                        case 'V':
	                            f_version = TRUE ;
	                            break ;

/* blank lines at the top of a page */
	                        case 'b':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&blanklines) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'i':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl) ifname = argp ;

	                            break ;

	                        case 'f':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            fontname = argp ;

	                            break ;

/* output page headers */
	                        case 'h':
					f_headers = TRUE ;
	                            if (f_optequal) {

	                                f_optequal = FALSE ;
					if (avl)
						headerstring = avp ;

	                            }

	                            break ;

	                        case 'l':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&maxlines) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'o':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&coffset) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'p':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

#if	CF_DEBUGS
	                            debugprintf("main: about to get pointsize\n") ;
#endif

	                            if (argl) {

#if	CF_DEBUGS
	                                debugprintf("main: got pointsize\n") ;
#endif

	                                pointstring = argp ;
	                                pointlen = argl ;
	                            }

	                            break ;

	                        case 'v':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && (cfdec(argp,argl,&vs) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'x':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&xoffset) < 0))
	                                goto badargvalue ;

	                            break ;

	                        case 'y':
	                            if (argr <= 0) goto badargnum ;

	                            argp = argv[++i] ;
	                            argr -= 1 ;
	                            argl = strlen(argp) ;

	                            if (argl && 
	                                (cfdec(argp,argl,&yoffset) < 0))
	                                goto badargvalue ;

	                            break ;

	                        default:
	                            bprintf(efp,"%s : unknown option - %c\n",
	                                g.progname,*aop) ;

	                        case '?':
	                            f_usage = TRUE ;

	                        } /* end switch */

	                        akp += 1 ;

	                    } /* end while */

	                } /* end if (individual option key letters) */

	            } /* end if (digits as argument or not) */

	        } else {

/* we have a plus or minux sign character alone on the command line */

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
	                    g.progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */


	if (g.debuglevel > 0) bprintf(efp,
	    "%s: finished parsing arguments\n",
	    g.progname) ;


/* get our program root (if we have one) */

	if (g.programroot == NULL) {

	    if ((g.programroot = getenv(ROOTVAR)) == NULL)
	        g.programroot = PROGRAMROOT ;

	}


/* continue w/ the trivia argument processing stuff */

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	}

	if (f_usage) goto usage ;

	if (f_version) goto exit ;


/* load the positional arguments */

#if	CF_DEBUG
	if (g.debuglevel > 0) debugprintf(
	    "main: checking for positional arguments\n") ;
#endif

	pan = 0 ;
	for (i = 0 ; i <= maxai ; i += 1) {

	    if (BATST(argpresent,i)) {

#if	CF_DEBUG
	        if (g.debuglevel > 0) debugprintf(
	            "main: got a positional argument i=%d pan=%d arg=%s\n",
	            i,pan,argv[i]) ;
#endif

	        switch (pan) {

	        case 0:
	            ifname = (char *) argv[i] ;
	            break ;

	        case 1:
	            ofname = (char *) argv[i] ;
	            break ;

	        default:
	            bprintf(g.efp,
	                "%s: extra positional arguments ignored\n",
	                g.progname) ;

	        } /* end switch */

	        pan += 1 ;

	    } /* end if (got a positional argument) */

	} /* end for (loading positional arguments) */


/* check the arguments */

	if ((ifname == NULL) || (ifname[0] == '-')) {

		f_stdinput = TRUE ;
	    ifname = (char *) BFILE_STDIN ;

	}

	if ((ofname == NULL) || (ofname[0] == '-'))
	    ofname = (char *) BFILE_STDOUT ;

/* how many lines per page */

	if (maxlines < 1) maxlines = DEFMAXLINES ;

	if (maxlines > MAXLINES) maxlines = MAXLINES ;

/* establish an offset if any */

	if (coffset < 0) {
	    coffset = 0 ;

	} else if (coffset > LINELEN)
	    coffset = LINELEN ;

/* establish working point size and vertical spacing */

	if (pointstring != NULL) {

	    i = substring(pointstring,pointlen,".") ;

	    if (i < 0)
	        i = substring(pointstring,pointlen,"/") ;

	    if (i >= 0) {

	        if ((i > 0) && (cfdec(pointstring,i,&ps) < 0))
	            goto badargvalue ;

	        if ((pointlen - i) > 1)
	            if (cfdec(pointstring + i + 1,pointlen - i - 1,&vs) < 0)
	                goto badargvalue ;

	    } else {

	        if (cfdec(pointstring,pointlen,&ps) < 0)
	            goto badargvalue ;

	    }

	} /* end if (handling the 'pointstring') */

	if (ps < 2) ps = 6 ;

	if (vs == 0)
	    vs = ps + 2 ;

	else if (vs < ps)
	    vs = ps + 1 ;

	if (g.debuglevel > 0) bprintf(efp,
	    "%s: ps %ld - vs %ld\n",g.progname,ps,vs) ;


/* open files */

	if ((rs = bopen(ifp,ifname,"r",0666)) < 0) 
		goto badinfile ;

	if ((rs = bopen(ofp,ofname,"wct",0666)) < 0) 
		goto badoutopen ;


/* perform initialization processing */

	for (i = 0 ; i < LINELEN ; i += 1) blanks[i] = ' ' ;


	cp = blankstring ;
	for (i = 0 ; (i < blanklines) && (i < MAXBLANKLINES) ; i += 1)
	    *cp++ = '\n' ;

	*cp = '\0' ;


	if (f_headers) {

		maxlines = maxlines - 2 ;
		if ((rs = bcontrol(ifp,BC_STAT,&sb)) < 0) {

#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf("main: bcontrol STAT rs %d\n",rs) ;
#endif

			sb.st_mtime = time(NULL) ;

		}

		if (headerstring == NULL)
			headerstring = "%s  Page %3d" ;

	} /* end if (page headers requested) */


/* output the header stuff */

	bprintf(ofp,".nf\n") ;

	bprintf(ofp,".fp 1 %s\n.ft %s\n",fontname,fontname) ;


/* change to running point size */

	bprintf(ofp,".ps %ld\n",ps) ;

	bprintf(ofp,".vs %d\n",vs) ;


/* output the pages */

	page = 0 ;
	line = 0 ;
	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	    lbp = linebuf ;

	    while ((page == 0) && (line == 0) && 
	        (len > 0) && (*lbp == '\014')) {

	        lbp += 1 ;
	        len -= 1 ;

	    } /* end while (removing top-of-file page breaks) */

	    if (len <= 0) continue ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("main: LINE>%W",lbp,len) ;
#endif

	    while (*lbp == '\014') {

	        if ((g.debuglevel > 0) || g.f.verbose)
	            bprintf(efp,
	                "%s: requested page break at page=%d line=%d\n",
	                g.progname,page,line) ;

	        lbp += 1 ;
	        len -= 1 ;
	        page += 1 ;
	        f_pagebreak = FALSE ;

	        if (len == 0) goto done ;

	        line = 0 ;
	        bprintf(ofp,".bp\n") ;

	    } /* end while */

/* if a page break was previously scheduled, handle it */

	    if (f_pagebreak) {

	        f_pagebreak = FALSE ;
	        if ((g.debuglevel > 0) || g.f.verbose)
	            bprintf(efp,
	                "%s: scheduled page break at page=%d line=%d\n",
	                g.progname,page,line) ;

	        page += 1 ;
	        line = 0 ;
	        bprintf(ofp,".bp\n") ;

	    } /* end if */


/* handle the blank space at the top of all pages */

	    if ((line == 0) && (blankstring[0] != '\0'))
	        bprintf(ofp,"%s",blankstring) ;


/* are we at a page header ? */

	if ((line == 0) && f_headers) {

		len2 = sprintf(headline,headerstring,
			timestr_edate(sb.st_mtime,timebuf),page) ;

		strncpy(headline + len2,blanks,40) ;

		if (f_stdinput)
			strcpy(headline + 40,"** standard input **") ;

		else
			strcpy(headline + 40,ifname) ;

	        bprintf(ofp,"%s\n\n",headline) ;

	} /* end if (page header processing) */


/* process this new line */

/* expand it */

	    len = expandline(lbp,len,buf,BUFLEN,&f_escape) ;

/* check for user specified leading offset */

	    if (coffset) bwrite(ofp,blanks,coffset) ;

/* write the expanded line data out */

	    bwrite(ofp,buf,len) ;

/* do we need a trailing EOL ? */

	    if (buf[len - 1] != '\n') bputc(ofp,'\n') ;

/* finally, check for page break */

	    line += 1 ;
	    if (line >= maxlines) {

	        if ((g.debuglevel > 0) || g.f.verbose)
	            bprintf(efp,
	                "%s: forced page break at page=%d line=%d\n",
	                g.progname,page,line) ;

	        line = 0 ;
	        page += 1 ;
	        f_pagebreak = TRUE ;
	    }

	} /* end while (main file line reading loop) */


/* let's get out of here !! */
done:
	if (line > 0) page += 1 ;

	if ((g.debuglevel > 0) || g.f.verbose)
	    bprintf(efp,"%s: pages processed - %d\n",
	        g.progname,page) ;


/* close off and get out ! */
exit:
	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return OK ;

/* what are we about ? */
usage:
	bprintf(efp,
	    "%s: USAGE> %s [-l lines] [-lines] [infile [outfile]]\n",
	    g.progname,g.progname) ;

	bprintf(efp,"\t\t[-f font] [-o offset] [-DV]\n") ;

	bprintf(efp,"\t\t[-p point_size] [-v vertical_space]\n") ;

	bprintf(efp,
	    "\t-l lines    number of lines per page\n") ;

	bprintf(efp,
	    "\t-lines      number of lines per page\n") ;

	bprintf(efp,
	    "\tinfile      input file\n") ;

	bprintf(efp,
	    "\toutfile     output file\n") ;

	bprintf(efp,
	    "\t-f font     TROFF type font specification\n") ;

	bprintf(efp,
	    "\t-o offset   left margin shift offset\n") ;

	bprintf(efp,
	    "\t-D          debugging flag\n") ;

	bprintf(efp,
	    "\t-V          program version\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",g.progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    g.progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: could not open input file \"%s\" (rs %d)\n",
	    g.progname,ifname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open output file \"%s\" (rs %d)\n",
	    g.progname,ofname,rs) ;

	goto badret ;

badret:
	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



