/* main */

/* format a file into double mini-style pages for a printer */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1987-09-10, David A­D­ Morano

*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will read the input file and format it into TROFF constant
        width font style input.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>


/* defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	VERSION		"0a"
#define	NPARG		2	/* number of positional arguments */

#define	REFPAGE		1
#define	BRAINDEAD	0	/* printers which add blank line on backside */

#define	NTOPLINES	3
#define	MAXLINES	130
#define	MAXBS		20


/* external subroutines */


/* local variables */

static char	lbuf[MAXLINES][LINEBUFLEN + MAXBS + 3] ;

static int	llen[MAXLINES] ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	input, *ifp = &input ;
	bfile	output, *ofp = &output ;
	bfile	error, *efp = &error ;

	int	argl, aol ;
	int	pan, i, j, si ;
	int	page, len, l, lines, rs ;
	int	tbl ;
	int	f_version = FALSE ;

	char	*progname, *argp, *aop ;
	char	tbuf[LINEBUFLEN + 1] ;
	char	*tbp ;

	char	f_refpage = FALSE ;
	char	f_concatenate = FALSE ;
	char	*fontname = "CW" ;

	char	*infile = (char *) 0 ;
	char	*outfile = (char *) 0 ;


	progname = argv[0] ;
	bopen(efp,BIO_STDERR,"wca",0666) ;


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
	            switch (*aop) {

	            case 'c':
	                f_concatenate = TRUE ;
	                break ;

	            case 'r':
	                f_refpage = TRUE ;
	                break ;

	            case 'V':
			f_version = TRUE ;
	                bprintf(efp,"%s: version %s\n",
				progname,VERSION) ;

	                break ;

	            case 'i':
	                if (argc <= 0) goto badargnum ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                if (argl) infile = argp ;

	                break ;

	            case 'o':
	                if (argc <= 0) goto badargnum ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                if (argl) outfile = argp ;

	                break ;

	            case 'f':
	                if (argc <= 0) goto badargnum ;

	                argp = argv[i++] ;
	                argc -= 1 ;
	                argl = strlen(argp) ;

	                fontname = argp ;

	                break ;

	            default:
	                bprintf(efp,"%s: unknown option - %c\n",
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
	                if (argl > 0) infile = argp ;

	                break ;

	            case 1:
	                if (argl > 0) outfile = argp ;

	                break ;

	            default:
	                break ;

	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
			progname) ;

	        }

	    } /* end if */

	} /* end while */


	if (f_version) goto earlyret ;


/* check arguments */

	if (infile == (char *) 0) infile = (char *) BIO_STDIN ;

	if (outfile == (char *) 0) outfile = (char *) BIO_STDOUT ;


/* open files */

	rs = bopen(ifp,infile,"r",0666) ;

	if (rs < 0) return rs ;

	rs = bopen(ofp,outfile,"wct",0666) ;

	if (rs < 0) return rs ;


/* perform initialization processing */

	bprintf(ofp,".nf\n") ;

	bprintf(ofp,".fp 1 %s\n.ft %s\n",fontname,fontname) ;


/* change to running point size */

	bprintf(ofp,".ps 6\n") ;

	bprintf(ofp,".vs 8\n") ;


	page = 0 ;

/* top of page processing */
next_page:
	l = 0 ;
	len = breadline(ifp,tbuf,LINEBUFLEN) ;

	if (len > 0) {

/* output page */

	    if (page >= 1) {

	        if (page > 1) bprintf(ofp,".bp\n\n") ;

#if	BRAINDEAD
	        bprintf(ofp,"\n") ;
#endif

	        for (i = 0 ; i < lines ; i += 1) {
	            bwrite(ofp,lbuf[i],llen[i]) ;
	        }

	        if (f_refpage) {

/* output reference page */

#if	CF_DEBUG
	            eprintf("main: outputting reference page\n") ;
#endif

	            bprintf(ofp,".bp\n") ;

	            if (page > 1) bprintf(ofp,"\n") ;

	            for (i = 0 ; (i < NTOPLINES) && (i < lines) ; i += 1) {

	                if ((si = substring(lbuf[i],llen[i],"Page")) >= 0) {

	                    for (j = (llen[i] - 1) ; j >= si ; j -= 1) {

	                        lbuf[i][j + strlen("Reference ")] =
	                            lbuf[i][j] ;

	                    }

	                    movc(strlen("Reference "),
	                        "Reference ",lbuf[i] + si) ;

	                    bwrite(ofp,lbuf[i],llen[i] + strlen("Reference ")) ;

	                } else {

	                    bwrite(ofp,lbuf[i],llen[i]) ;

	                }

	            }

	            for (i = NTOPLINES ; i < lines ; i += 1) {

	                bwrite(ofp,lbuf[i],llen[i]) ;

	            }

	        }

	    }

/* process the temporary line into the line buffer */

	    tbp = lbuf[l] ;
	    tbl = 0 ;
	    for (i = 0 ; (i < len) && (tbl <= (LINEBUFLEN + MAXBS)) ; i += 1) {

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

	        if (page > 1) bprintf(ofp,".bp\n\n") ;

	        bprintf(ofp,"\n") ;

	        for (i = 0 ; i < lines ; i += 1) {

	            bwrite(ofp,lbuf[i],llen[i]) ;

	        }

	        if (f_concatenate && f_refpage) {

	            bprintf(ofp,".bp\n") ;

	            if (page > 1) bprintf(ofp,"\n") ;

	            bprintf(ofp," \n.bp\n") ;

	        }

	    } /* end if */

	    goto done ;

	} /* end if */

/* read the rest of this page into local store */

	while ((len = breadline(ifp,tbuf,LINEBUFLEN)) > 0) {

	    tbp = lbuf[l] ;
	    tbl = 0 ;
	    for (i = 0 ; (i < len) && (tbl <= (LINEBUFLEN + MAXBS)) ; i += 1) {

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

	    if (scanc(lbuf[l],'\014',tbl) >= 0) {
	        break ;
	    }

	    l += 1 ;
	    if (l >= MAXLINES) break ;

	}

	page += 1 ;
	lines = l ;
	goto next_page ;


done:
	bclose(ifp) ;

	bclose(ofp) ;

earlyret:
badret:
	bclose(efp) ;

	return OK ;

usage:
	bprintf(efp,
		"%s: USAGE> %s [-c] [-r] [infile] [outfile] ",
			progname,progname) ;

	bprintf(efp,"[-f font] [-o outfile]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;
}
/* end subroutine (main) */


int scanc(buf,c,n)
int	n ;
char	*buf, c ;
{
	int	j ;

	for (j = 0 ; j < n ; j += 1) {

	    if (buf[j] == c) return j ;

	}

	return -1 ;
}
/* end subroutine */



