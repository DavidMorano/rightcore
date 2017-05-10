/* mroff */

/* format a file into double mini-style pages for a printer */


#define	CF_DEBUG	1


/* revision history:

	= 87/09/10, David A­D­ Morano
	This subroutine was originally written.

	= 98/02/27, David A­D­ Morano
	This subroutine has been enhanced from the original MROFF
	program to work with the new one which can provide
	a non-standard print-out page header.


*/


/*******************************************************************

	This subroutine will read the input file and format it into
	TROFF constant width font style input.


*********************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<time.h>

#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"mroff.h"



/* local defines */

#define	BRAINDEAD	0	/* printers which add blank line on backside */

#define	NTOPLINES	3
#define	MAXBS		20



/* external subroutines */


/* external variables */


/* forward references */

static int	scanc() ;


/* local data */

static char	lbuf[MAXLINES][LINELEN + MAXBS + 3] ;

static int	llen[MAXLINES] ;





int mroff(pip,rop,pagenump,filename,filenum,ofp)
struct proginfo	*pip ;
struct mroff	*rop ;
int		*pagenump ;
char		filename[] ;
int		filenum ;
bfile		*ofp ;
{
	bfile		infile, *ifp = &infile ;

	struct ustat	sb ;

	int		i, j, si ;
	int		page, len, l, lines, rs ;
	int		len2 ;
	int		f_escape ;
	int		f_stdinput = FALSE ;

	char		tbuf[LINELEN + 1] ;
	char		*tbp ;
	char	blankline[LINELEN + 1] ;
	char	headline[LINELEN + 1] ;
	char	nulllines[MAXNULLLINES + 1] ;
	char	timebuf[100] ;
	char	*headerstring, *footerstring ;
	char	*cp ;


/* open the input file */

	if (strcmp(filename,"-") == 0) {

	    f_stdinput = TRUE ;
	    filename = BFILE_STDIN ;
	}

	rs = bopen(ifp,filename,"r",0666) ;

	if (rs < 0) {

	    if ((! pip->f.quiet) && (pip->debuglevel == 0))
	        bprintf(pip->efp,
	            "%s: could not open input file (rs %d)\n",
	            pip->progname,
	            rs) ;

	    return rs ;
	}

	if (f_stdinput)
	    filename = "** standard input **" ;



/* what are we doing ? */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mroff: we do %swant headers\n",
	        ((rop->f.headers) ? "" : "not ")) ;
#endif


/* perform initialization processing */

	for (i = 0 ; i < LINELEN ; i += 1) blankline[i] = ' ' ;


	cp = nulllines ;
	for (i = 0 ; (i < rop->blanklines) && (i < MAXNULLLINES) ; i += 1)
	    *cp++ = '\n' ;

	*cp = '\0' ;


	if (rop->f.headers) {

	    if ((rs = bcontrol(ifp,BC_STAT,&sb)) < 0) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("mroff: bcontrol STAT rs %d\n",rs) ;
#endif

	        sb.st_mtime = time(NULL) ;

	    }

	    headerstring = rop->headerstring ;
	    if (rop->headerstring == NULL)
	        headerstring = "%s Page      %3d" ;

	} /* end if (page headers requested) */


/* start into it ! */

	page = 0 ;

/* top of page processing */
next_page:

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("mroff: top of next page processing\n") ;
#endif

	l = 0 ;
	len = breadline(ifp,tbuf,LINELEN) ;

	if (len > 0) {

/* output page */

	    if (page >= 1) {

	        if (page > 1) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("mroff: outputting a page break \b") ;
#endif

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".bp\n") ;

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".\\\"_ page %d\n",page) ;

	            bprintf(ofp,".\\\"_\n") ;

	        }

#if	BRAINDEAD
	        bprintf(ofp,"\n") ;
#endif

/* output the null lines for the device if necessary */

	        if (nulllines[0] != '\0')
	            bprintf(ofp,"%s",nulllines) ;


/* are we at a page header ? */

	        if (rop->f.headers) {

	            len2 = sprintf(headline,headerstring,
	                timestr_edate(sb.st_mtime,timebuf),page) ;

	            strncpy(headline + len2,blankline,40) ;

	            strcpy(headline + 40,filename) ;

	            bprintf(ofp,"%s\n\n",headline) ;

	        } /* end if (page header processing) */


/* output the page contents */

	        for (i = 0 ; i < lines ; i += 1)
	            bwrite(ofp,lbuf[i],llen[i]) ;


/* and a reference page if any */

	        if (rop->f.reference) {

/* output reference page */

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("mroff: outputting reference page\n") ;
#endif

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".bp\n") ;

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".\\\"_ reference page %d\n",page) ;

	            bprintf(ofp,".\\\"_\n") ;


/* output the null lines for the device if necessary */

	            if (nulllines[0] != '\0')
	                bprintf(ofp,"%s",nulllines) ;


/* are we at a page header ? */

	            if (rop->f.headers) {

	                len2 = sprintf(headline,"%s Reference %3d",
	                    timestr_edate(sb.st_mtime,timebuf),page) ;

	                strncpy(headline + len2,blankline,40) ;

	                strcpy(headline + 40,filename) ;

	                bprintf(ofp,"%s\n\n",headline) ;

	            } /* end if (page header processing) */


/* output the contents of the reference page */

	            for (i = 0 ; i < lines ; i += 1)
	                bwrite(ofp,lbuf[i],llen[i]) ;


	        } /* end if (outputting a reference page) */

	    } /* end if (page number greater than 1) */

/* process the temporary line into the line buffer */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("mroff: about to expand a line 1\n") ;
#endif

	    llen[l] = expandline(tbuf,len,
	        lbuf + l,LINELEN + MAXBS,&f_escape) ;

	    l = 1 ;

	} else {

/* we have reached EOF */

	    if (page >= 1) {

/* output any page that we have */

	        if (page > 1) {

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".bp\n") ;

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".\\\"_ page %d\n",page) ;

	            bprintf(ofp,".\\\"_\n") ;

		}

/* output the null lines for the device if necessary */

	        if (nulllines[0] != '\0')
	            bprintf(ofp,"%s",nulllines) ;


/* are we at a page header ? */

	        if (rop->f.headers) {

	            len2 = sprintf(headline,headerstring,
	                timestr_edate(sb.st_mtime,timebuf),page) ;

	            strncpy(headline + len2,blankline,40) ;

	            strcpy(headline + 40,filename) ;

	            bprintf(ofp,"%s\n\n",headline) ;

	        } /* end if (page header processing) */


/* output the contents of this page */

	        for (i = 0 ; i < lines ; i += 1)
	            bwrite(ofp,lbuf[i],llen[i]) ;


	        if (rop->f.concatenate && rop->f.reference) {

	            bprintf(ofp,".bp\n") ;

	            if (page > 1) {

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".bp\n") ;

	            bprintf(ofp,".\\\"_\n") ;

	            bprintf(ofp,".\\\"_ page %d\n",page) ;

	            bprintf(ofp,".\\\"_\n") ;

			}

	        }
	    }

	    goto done ;

	} /* end if */

/* read the rest of this page into local store */

	while ((len = breadline(ifp,tbuf,LINELEN)) > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("mroff: about to expand a line 2\n") ;
#endif

	    llen[l] = expandline(tbuf,len,
	        lbuf + l,LINELEN + MAXBS,&f_escape) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("mroff: past expand line 2\n") ;
#endif

	    if (scanc(lbuf[l],'\014',llen[l]) >= 0)
	        break ;

	    l += 1 ;
	    if ((l >= rop->maxlines) || (l >= MAXLINES)) break ;

	} /* end while (main line reading loop) */


	page += 1 ;
	lines = l ;
	goto next_page ;


/* let's get out of here */
done:
	bclose(ifp) ;

	return OK ;
}
/* end subroutine (mroff) */


static int scanc(buf,c,n)
int	n ;
char	*buf, c ;
{
	int	j ;

	for (j = 0 ; j < n ; j += 1) {

	    if (buf[j] == c) return j ;

	}

	return -1 ;
}
/* end subroutine (scanc) */



