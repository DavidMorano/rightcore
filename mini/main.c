/* mini */

/* format a file into double mini-style pages for a printer */


/* revision history:

	= 1986-09-10, David A­D­ Morano

	This was originally written.


*/

/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This program will read the input file format it into
	troff constant width font style input.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<fcntl.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		2	/* number of positional arguments */
#define	DPRINT		0

#define	REFPAGE		1
#define	BRAINDEAD	0	/* printers which add blank line on backside */
#define	SCREWBOTTOM	1	/* printers which leave off the last line */

#define	NTOPLINES	3
#define	MAXBS		20

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern char	*strdcpy1(char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	scanc(const char *,int,int) ;


/* local variables */

static char	lbuf[MAXLINES][LINEBUFLEN + MAXBS + 3] ;

static int	llen[MAXLINES] ;


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile		input, *ifp = &input ;
	bfile		output, *ofp = &output ;
	bfile		error, *efp = &error ;
	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	argl, aol ;
	int	pan, i, j, si ;
	int	page, len, l, lines, rs ;
	int	tbl ;

	const char	*progname, *argp, *aop ;
	const char	*pn = NULL ;
	const char	*sn = NULL ;
	const char	*fontname = "CB" ;
	const char	*efname = NULL ;
	const char	*ofname = NULL ;
	const char	*ifname = NULL ;

	char	*tbp ;
	char	tbuf[LINEBUFLEN + 1] ;

	char	f_refpage = FALSE ;
	char	f_concatenate = FALSE ;


	progname = argv[0] ;

	if ((efname == NULL) || (efname[0] == '\0') || (efname[0] == '-'))
	    efname = BFILE_STDERR ;

	if (bopen(efp,efname,"wc",0666) < 0) return BAD ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

		aop = argp ;
		aol = argl ;
		while (--aol) {

	            aop += 1 ;
	            switch ((int) *aop) {

	            case 'c':
	                f_concatenate = TRUE ;
	                break ;

	            case 'r':
	                f_refpage = TRUE ;
	                break ;

	            case 'V':
	                bprintf(efp,"%s version : %s\n",progname,VERSION) ;
	                break ;

	            case 'i':
	                if (argc <= 0) goto badargnum ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                if (argl) ifname = argp ;

	                break ;

	            case 'o':
	                if (argc <= 0) goto badargnum ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                if (argl) ofname = argp ;

	                break ;

	            case 'f':
	                if (argc <= 0) goto badargnum ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                fontname = argp ;

	                break ;

	            default:
	                bprintf(efp,"%s : unknown option - %c\n",
				progname,*aop) ;

	                goto usage ;

	            } /* end switch */

		} /* end while */

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {
	            case 0:
	                if (argl > 0) ifname = argp ;
	                break ;
	            case 1:
	                if (argl > 0) ofname = argp ;
	                break ;
	            default:
	                break ;
	            }

	            pan += 1 ;
	        } else {
	            bprintf(efp,"extra arguments ignored\n") ;
	        }

	    } /* end if */

	} /* end while */

/* open files */

	if ((ofname == NULL) || (ofname[0] == '\0') || (ofname[0] == '-'))
	    ofname = BFILE_STDOUT ;

	if (bopen(ofp,ofname,"wc",0666) < 0) return BAD ;


	if ((ifname == NULL) || (ifname[0] == '\0') || (ifname[0] == '-'))
	    ifname = BFILE_STDIN ;

	rs = bopen(ifp,ifname,"r",0666) ;
	if (rs < 0) return rs ;


/* perform initialization processing */

	bprintf(ofp,".nf\n") ;

	bprintf(ofp,".fp 1 %s\n",fontname) ;
	bprintf(ofp,".ft %s\n",fontname) ;


/* change to running point size */

	bprintf(ofp,".ps 6\n") ;

	bprintf(ofp,".vs 8\n") ;


	page = 0 ;

/* top of page processing */
next_page:
	l = 0 ;
	len = breadline(ifp,tbuf,llen) ;

	if (len > 0) {

/* output page */

	    if (page >= 1) {

	        if (page > 1)
#if	SCREWBOTTOM
	        bprintf(ofp,".bp\n") ;
#else
	        bprintf(ofp,".bp\n\n") ;
#endif


#if	BRAINDEAD || SCREWBOTTOM
	        bprintf(ofp,"\n") ;
#endif

	        for (i = 0 ; i < lines ; i += 1) {

	            bwrite(ofp,lbuf[i],llen[i]) ;

	        }

	        if (f_refpage) {

/* output reference page */

#if	DPRINT
	            bprintf(efp,"outputting reference page\n") ;
#endif

	            bprintf(ofp,".bp\n") ;

	            if (page > 1) bprintf(ofp,"\n") ;

	            for (i = 0 ; (i < NTOPLINES) && (i < lines) ; i += 1) {

	                if ((si = substring(lbuf[i],llen[i],"Page")) >= 0) {
			    const char	*rs = "Reference " ;
			    int		rl ;

			    rl = strlen(rs) ;
	                    for (j = (llen[i] - 1) ; j >= si ; j -= 1) {
	                        lbuf[i][j + rl] = lbuf[i][j] ;
	                    }

			    strncpy((lbuf[i]+si),rs,rl) ;

	                    bwrite(ofp,lbuf[i],llen[i] + rl) ;

	                } else {

	                    bwrite(ofp,lbuf[i],llen[i]) ;

	                }

	            }

	            for (i = NTOPLINES ; i < lines ; i += 1) {

	                bwrite(ofp,lbuf[i],llen[i]) ;

	            }

	        } /* end if (refpage) */

	    } /* end if (pages) */

/* process the temporary line into the line buffer */

	    tbp = lbuf[l] ;
	    tbl = 0 ;
	    for (i = 0 ; (i < len) && (tbl <= (llen + MAXBS)) ; i += 1) {

	        if (tbuf[i] != '\\') {

	            *tbp++ = tbuf[i] ;
	            tbl += 1 ;

	        } else {

	            *tbp++ = '\\' ;
	            *tbp++ = '\\' ;
	            tbl += 2 ;
	        }

	    }

	    llen[l] = tbl ;

	    l = 1 ;

	} else {

/* we have reached EOF */

	    if (page >= 1) {

/* output any page that we have */

	        if (page > 1)
#if	SCREWBOTTOM
			bprintf(ofp,".bp\n") ;
#else
			bprintf(ofp,".bp\n\n") ;
#endif

	        bprintf(ofp,"\n") ;

	        for (i = 0 ; i < lines ; i += 1) {

	            bwrite(ofp,lbuf[i],llen[i]) ;

	        }

	        if (f_concatenate && f_refpage) {

	            bprintf(ofp,".bp\n") ;

	            if (page > 1) bprintf(ofp,"\n") ;

	            bprintf(ofp," \n.bp\n") ;

	        }
	    }

	    goto done ;
	}

/* read the rest of this page into local store */

	while ((len = breadline(ifp,tbuf,llen)) > 0) {

	    tbp = lbuf[l] ;
	    tbl = 0 ;
	    for (i = 0 ; (i < len) && (tbl <= (llen + MAXBS)) ; i += 1) {

	        if (tbuf[i] != '\\') {

	            *tbp++ = tbuf[i] ;
	            tbl += 1 ;

	        } else {

	            *tbp++ = '\\' ;
	            *tbp++ = '\\' ;
	            tbl += 2 ;
	        }

	    }

	    llen[l] = tbl ;

	    if (scanc(lbuf[l],'\014',tbl) >= 0) break ;

	    l += 1 ;
	    if (l >= MAXLINES) break ;

	}

	page += 1 ;
	lines = l ;
	goto next_page ;

done:
	bclose(ifp) ;

	bclose(ofp) ;

badret:
	bclose(efp) ;

	return OK ;

usage:
	bprintf(efp,
		"%s usage: mroff [-c] [-r] [infile] [outfile] ",progname) ;

	bprintf(efp,"[-f font] [-o outfile]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;
}
/* end subroutine (main) */


/* local subroutines */


static int scanc(buf,mch,n)
int		n, mch ;
const char	buf[] ;
{
	int	j ;
	int	f = FALSE ;


	for (j = 0 ; j < n ; j += 1) {
	    f = ((buf[j] & UCHAR_MAX) == mch) ;
	    if (f) break ;
	}

	return (f) ? j : -1 ;
}
/* end subroutine (scanc) */


