/* id-a */

/* print out the various IDs assciated with a user */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 95/12/01, David A­D­ Morano

	This program was originally written.


*/


/**************************************************************************

	This program will print out to standard output all of the
	various IDs associated with a user.



***************************************************************************/


#include	<sys/types.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<exitcodes.h>

#include	"localmisc.h"



/* local defines */

#define	VERSION		"0"

#define	NPARG		1
#define	MAXARGINDEX	100
#define	MAXARGGROUPS	(MAXARGINDEX/8 + 1)

#define	LINELEN		80

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + 12)
#endif



/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;
extern int	matstr(const char **,const char *,int) ;


/* local structures */

struct groupprint {
	bfile	*ofp ;
	int	debuglevel ;
	char	buf[LINELEN + 1] ;
	int	linelen ;
	int	buflen ;
} ;


/* forward references */


/* local variables */

static const char *argopts[] = {
	"version",
	NULL
} ;

enum argopts {
	argopt_version,
	argopt_overlast
} ;






int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct groupprint	gp ;

	bfile		errfile, *efp = &errfile ;
	bfile		outfile ;
	bfile		idfile, *fpa[3] ;

	int	argr, argl, aol, akl, avl, npa, maxai, kwi ;
	int	rs, pan, i, len ;
	int	ex = EX_INFO ;
	int	pl ;
	int	readlen ;
	int	linelen ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_dash = FALSE ;
	int	f_debug = FALSE ;
	int	f_extra = FALSE ;
	int	f_usage = FALSE ;
	int	f_version = FALSE ;
	int	f_bol, f_eol ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	*progname ;
	char	*ofname = BFILE_STDOUT ;
	char	linebuf[LINELEN + 1], *lbp ;
	char	buf[LINELEN + 1] ;
	char	*cp, *cp2 ;



	progname = argv[0] ;
	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0) return BAD ;

	for (i = 0 ; i < MAXARGGROUPS ; i += 1) argpresent[i] = 0 ;

/* global initializations */

	gp.ofp = &outfile ;
	gp.debuglevel = 0 ;
	gp.linelen = 0 ;
	gp.buflen = 0 ;


/* parse the arguments */

	rs = SR_OK ;

	npa = 0 ;			/* number of positional so far */
	maxai = 0 ;
	i = 0 ;
	argr = argc - 1 ;
	while (rs >= 0) && (argr > 0)) {

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

	                aol = avp - aop ;
	                akl = avp - aop ;
	                avp += 1 ;
	                avl = aop + argl - 1 - avp ;
	                f_optequal = TRUE ;

	            } else {

	                avl = 0 ;
	                akl = aol ;

	            }

/* do we have a keyword match or should we assume only key letters? */

	            if ((kwi = matostr(argopts,2,akp,akl)) >= 0) {

	                switch (kwi) {

	                case argopt_version:
	                    f_version = TRUE ;
	                    if (f_optequal) 
				goto badargextra ;

	                    break ;

	                } /* end switch (key words) */

	            } else {

	                while (akl--) {

	                    switch ((int) *akp) {

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

	                    case 'D':
	                        f_debug = TRUE ;
	                        gp.debuglevel = 1 ;
	                        if (f_optequal) {

	                            f_optequal = FALSE ;
				    if (avl) {

	                            rs = cfdeci(avp,avl, &gp.debuglevel) ;

					if (rs < 0)
	                                goto badargvalue ;

				    }
	                        }

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    default:
				rs = SR_INVALID ;
	                        printf("%s: unknown option - %c\n",progname,
	                            *aop) ;

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

	        if (i < MAXARGGROUPS) {

	            BASET(argpresent,i) ;
	            maxai = i ;
	            npa += 1 ;

	        } else {

	            if (! f_extra) {

	                f_extra = TRUE ;
	                bprintf(efp,"%s: extra arguments ignored\n",
	                    progname) ;

	            }
	        }

	    } /* end if (key letter/word or positional) */

	} /* end while (all command line argument processing) */



#if	CF_DEBUGS
	debugprintf("main: debuglevel=%u\n",gp.debuglevel) ;
#endif

	if (gp.debuglevel > 0) bprintf(efp,
	    "%s: debuglevel=%u\n",progname,gp.debuglevel) ;

#if	CF_DEBUGS
	debugprintf("main: 1\n") ;
#endif

	if (f_version)
	    bprintf(efp,"%s: version %s\n",
	        progname,VERSION) ;

	if (f_usage) goto usage ;

#if	CF_DEBUGS
	debugprintf("main: 2\n") ;
#endif

	if (f_version) goto retearly ;

#if	CF_DEBUGS
	debugprintf("main: 3\n") ;
#endif


/* open the output file */

	if ((rs = bopen(gp.ofp,ofname,"wct",0666)) < 0) goto badout ;

/* run the ID program */

	fpa[0] = NULL ;
	fpa[1] = &idfile ;
	fpa[2] = NULL ;
	if ((rs = bopencmd(fpa,"/usr/bin/id -a")) < 0) goto badcmd ;

/* read the first line and print out up to "groups" */

	f_bol = TRUE ;
	readlen = LINELEN ;
	i = 0 ;
	lbp = linebuf ;
	linelen = 0 ;
	while ((len = breadline(fpa[1],lbp,readlen)) > 0) {

	    f_eol = FALSE ;
	    if (lbp[len - 1] == '\n') {

	        f_eol = TRUE ;
	        len -= 1 ;

	    }

		linelen += len ;

#if	CF_DEBUG
	    if (gp.debuglevel > 0)
	        debugprintf("main: line=\"%W\" len=%d\n",lbp,len,len) ;
#endif

	    lbp[len] = '\0' ;
	    for (cp = linebuf ; (cp2 = strchr(cp,',')) != NULL ; cp = cp2) {

	        *cp2++ = '\0' ;
	        pl = strlen(cp) ;

#if	CF_DEBUG
	        if (gp.debuglevel > 0)
	            debugprintf("main: got one, cp=\"%s\" pl=%d\n",cp,pl) ;
#endif

	        printout(&gp,cp,pl) ;

#if	CF_DEBUG
	        if (gp.debuglevel > 0)
	            debugprintf("main: back from one, cp=\"%s\" pl=%d\n",cp,pl) ;
#endif

		linelen -= pl ;

	    } /* end for */

		pl = strlen(cp) ;

		if (f_eol && (pl > 0)) {

#if	CF_DEBUG
	if (gp.debuglevel > 1)
		debugprintf("main: remainder w/ EOL pl=%d\n",pl) ;
#endif

			linelen -= pl ;
	        	printout(&gp,cp,pl) ;

			pl = 0 ;
		}

/* if there is any stuff left over, we copy it to the beginning */

	    if (pl > 0) {

#if	CF_DEBUG
	if (gp.debuglevel > 1)
		debugprintf("main: remainder still pl=%d\n",pl) ;
#endif

		strwcpy(buf,cp,pl) ;

		strcpy(linebuf,buf) ;

		readlen = LINELEN - pl ;
	        lbp = linebuf + pl ;
		linelen = pl ;

		} else {

/* reset buffer bounds */

		linelen = 0 ;
		lbp = linebuf ;
		readlen = LINELEN ;

		}

	    f_bol = f_eol ;

	} /* end while (reading line segments) */

/* process any remaining line pieces */

#if	CF_DEBUG
	if (gp.debuglevel > 0)
	    debugprintf("main: flushing\n") ;
#endif

	if (linelen > 0) {

#if	CF_DEBUG
	if (gp.debuglevel > 1)
		debugprintf("main: final remainder linelen=%d\n",linelen) ;
#endif

	        	printout(&gp,linebuf,linelen) ;

	}

/* flush the output stating buffer */

	if (gp.buflen > 0)
	    bprintf(gp.ofp,"%W\n",gp.buf,gp.buflen) ;

/* done */

ret3:
	bclose(fpa[1]) ;

ret2:
	bclose(gp.ofp) ;

retearly:
ret1:
	bclose(efp) ;

ret0:
	return OK ;

badargextra:
	bprintf(efp,"%s: no value associated with this option key\n",
	    progname) ;

	goto badret ;

badargvalue:
	bprintf(efp,"%s: bad argument value was specified\n",
	    progname) ;

	goto badret ;

badcmd:
	bprintf(efp,
	    "%s: underlying ID command would not execute (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badout:
	bprintf(efp,"%s: can't open outfile (%d)\n",progname,rs) ;

	goto badret ;

notenough:
	bprintf(efp,"%s: not enough arguments given\n",progname) ;

	goto badret ;

badparam:
	bprintf(efp,"%s: bad parameter specified\n",progname) ;

	goto badret ;

usage:
	bprintf(efp,"usage: %s [-VD]\n",
	    progname) ;

badret:
	bclose(efp) ;

	return BAD ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



int printout(gpp,cp,pl)
struct groupprint	*gpp ;
char			*cp ;
int			pl ;
{
	int	rs ;


#if	CF_DEBUG
	    if (gpp->debuglevel > 0)
			debugprintf("printout: entered\n") ;
#endif

	if ((gpp->linelen + pl) > 76) {

#if	CF_DEBUG
	    if (gpp->debuglevel > 0)
	        debugprintf("printout: flushing existing, len=%d\n",
			gpp->buflen) ;

	    if (gpp->debuglevel > 0)
	        debugprintf("printout: flushing existing, buf=\"%W\" len=%d\n",
	            gpp->buf,gpp->buflen,gpp->buflen) ;
#endif

	    bprintf(gpp->ofp,"%t,\n",
	        gpp->buf,gpp->buflen) ;

	    gpp->linelen = 32 ;
	    gpp->buflen = bufprintf(gpp->buf,BUFLEN,
		"\t\t\t\t") ;

	}

#if	CF_DEBUG
	    if (gpp->debuglevel > 0)
			debugprintf("printout: about to check for comma\n") ;
#endif

	if (gpp->linelen > 32) {

	    gpp->linelen += 1 ;
	    gpp->buflen += bufprintf((gpp->buf + gpp->buflen),BUFLEN,
		",") ;

	}

#if	CF_DEBUG
	    if (gpp->debuglevel > 0) debugprintf(
		"printout: about to store, pl=%d, buflen=%d\n",
			pl,gpp->buflen) ;
#endif

	rs = bufprintf((gpp->buf + gpp->buflen),BUFLEN,
		"%t",cp,pl) ;

	gpp->buflen += rs ;

#if	CF_DEBUG
	    if (gpp->debuglevel > 0)
		debugprintf("printout: rs=%d, buflen=%d\n",
			rs,gpp->buflen) ;
#endif

	gpp->linelen += pl ;
	return gpp->linelen ;
}
/* end subroutine (printout) */



