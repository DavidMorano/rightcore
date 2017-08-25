/* progfile */

/* process an input file to the output */


#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revistion history:

	= 1987-09-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1987 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes a file by writing its contents out to an
        output file with the correct pagination.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(2 * LINEBUFLEN)
#endif


/* external subroutines */

extern int	expandline(char *,int,char *,int,int *) ;
extern int	bufprintf(char *,int,const char *,...) ;


/* forward references */


/* local structures */


/* local variables */


/* exported subroutines */


int progfile(PROGINFO *pip,cchar *infname)
{
	struct ustat	sb ;
	bfile		infile, *ifp = &infile ;
	time_t		pagetime ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		line, len ;
	int		len2 ;
	int		page = 0 ;
	int		wlen = 0 ;
	int		f_pagebreak = FALSE ;
	int		f_stdinput = FALSE ;
	int		f_escape ;
	int		f_startbreak = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;
	cchar		*cp ;
	char		lbuf[LINEBUFLEN + 1], *lbp ;
	char		buf[BUFLEN + 1] ;
	char		headline[LINEBUFLEN + 1] ;
	char		timebuf[TIMEBUFLEN + 1] ;

/* check the arguments */

	if ((infname == NULL) || (infname[0] == '-')) {
	    f_stdinput = TRUE ;
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;
	} else
	    rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0)
	    goto badinfile ;

	if (pip->f.headers) {

	    pip->pagelines = (pip->pagelines - 2) ;
	    rs = bcontrol(ifp,BC_STAT,&sb) ;

	    if (rs < 0) {
	        pagetime = time(NULL) ;
	    } else
	        pagetime = sb.st_mtime ;

	    if (pip->headerstring == NULL)
	        pip->headerstring = "%s  Page %3d" ;

	} /* end if (page headers requested) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: processing file %d\n",fn) ;
#endif

	if (pip->fileno > 1)
	    f_startbreak = TRUE ;

/* output a header for this file */

	cp = (char *) ((f_stdinput) ? "*stdin*" : infname) ;
	bprintf(pip->ofp,".\\\"_ file=%s\n",cp) ;

/* output the pages */

	page = 0 ;
	line = 0 ;
	while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	    len = rs ;

	    lbp = lbuf ;
	    while ((page == 0) && (line == 0) && 
	        (len > 0) && (*lbp == '\014')) {

	        lbp += 1 ;
	        len -= 1 ;

	    } /* end while (removing top-of-file page breaks) */

	    if (len <= 0) continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("progfile: LINE>%W",lbp,len) ;
#endif

/* handle the case of starting a new file after some previous files */

	    if (f_startbreak) {

	        f_startbreak = FALSE ;
	        if (pip->f.eject) {
	            bprintf(pip->ofp,".bp\n") ;
		}
	        bprintf(pip->ofp,".bp\n") ;

	    }

/* handle a user requested page break */

	    while (*lbp == '\014') {

	        if (pip->debuglevel > 0) {
		    fmt = "%s: requested page break fn=%u page=%u line=%u\n" ;
	            bprintf(pip->efp,fmt,pn,pip->fileno,pip->page,line) ;
	 	}

	        lbp += 1 ;
	        len -= 1 ;
	        page += 1 ;
	        f_pagebreak = FALSE ;
	        if (len == 0) goto done ;

	        line = 0 ;
	        bprintf(pip->ofp,".bp\n") ;

	    } /* end while */

/* if a page break was previously scheduled, handle it */

	    if (f_pagebreak) {

	        f_pagebreak = FALSE ;
	        if (pip->debuglevel > 0) {
		    fmt = "%s: scheduled page break fn=%u page=%u line=%u\n" ;
	            bprintf(pip->efp,fmt,pip->fileno,pip->page,line) ;
		}

	        line = 0 ;
	        bprintf(pip->ofp,".bp\n") ;

	    } /* end if */

/* handle the blank space at the top of all pages */

	    if ((line == 0) && (pip->blankstring[0] != '\0')) {
	        bprintf(pip->ofp,"%s",pip->blankstring) ;
	    }

/* are we at a page header ? */

	    if ((line == 0) && pip->f.headers) {

	        len2 = bufprintf(headline,LINEBUFLEN,
			pip->headerstring,
	            timestr_edate(pagetime,timebuf),(page + 1)) ;

	        strncpy((headline + len2),pip->blanks,40) ;

	        if (f_stdinput) {
	            strcpy((headline + 40),"** standard input **") ;
	        } else {
	            strcpy((headline + 40),infname) ;
		}

	        bprintf(pip->ofp,"%s\n\n",headline) ;

	    } /* end if (page header processing) */

/* process this new line */

/* expand it */

	    len = expandline(lbp,len,buf,BUFLEN,&f_escape) ;

/* check for user specified leading offset */

	    if (pip->coffset)
	        bwrite(pip->ofp,pip->blanks,pip->coffset) ;

/* write the expanded line data out */

	    bwrite(pip->ofp,buf,len) ;

/* do we need a trailing EOL ? */

	    if (buf[len - 1] != '\n')
	        bputc(pip->ofp,'\n') ;

/* finally, check for page break */

	    line += 1 ;
	    if (line >= pip->pagelines) {

	        if (pip->debuglevel > 0) {
		    fmt = "%s: forced page break fn=%d page=%d line=%d\n" ;
	            bprintf(pip->efp,fmt,pip->fileno,pip->page,line) ;
		}

	        line = 0 ;
	        page += 1 ;
	        f_pagebreak = TRUE ;
	    }

	} /* end while (main file line reading loop) */

	if (line > 0)
	    page += 1 ;

	pip->page += page ;

done:
	bclose(ifp) ;

badinfile:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progfile) */


