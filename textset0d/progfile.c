/* progfile */

/* process an input file to the output */


#define	CF_DEBUG	1		/* run-time debug print-outs */


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
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		(2 * LINEBUFLEN)
#endif


/* external subroutines */

extern int	expandline(char *,int,cchar *,int,int *) ;
extern int	bufprintf(char *,int,cchar *,...) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*timestr_edate(time_t,char *) ;


/* forward references */

static int	mkheader(PROGINFO *,char *,int,cchar *,int,time_t,int) ;


/* local structures */


/* local variables */


/* exported subroutines */


int progfile(PROGINFO *pip,cchar *ifn)
{
	bfile		ifile, *ifp = &ifile ;
	int		rs ;
	int		rs1 ;
	int		line, len ;
	int		page = 0 ;
	int		wlen = 0 ;
	int		f_pagebreak = FALSE ;
	int		f_si = FALSE ;
	int		f_startbreak = FALSE ;
	cchar		*pn = pip->progname ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ent ifn=%s\n",ifn) ;
#endif

/* check the arguments */

	if ((ifn == NULL) || (ifn[0] == '-')) {
	    f_si = TRUE ;
	    ifn = BFILE_STDIN ;
	}

	if ((rs = bopen(ifp,ifn,"r",0666)) >= 0) {
	    time_t	ft ;
	    const int	llen = LINEBUFLEN ;
	    int		f_esc ;
	    cchar	*bls = pip->blanklines ;
	    char	lbuf[LINEBUFLEN + 1], *lbp ;

	    if (pip->files > 0) {
	        rs = bprintf(pip->ofp,".bp\n") ;
		wlen += rs ;
	    }

	    if ((rs >= 0) && pip->f.headers) {
	        USTAT	sb ;

	        pip->pagelines = (pip->pagelines - 2) ;
	        if ((rs = bcontrol(ifp,BC_STAT,&sb)) >= 0) {
	            ft = sb.st_mtime ;
	        } else if (rs == SR_NOTSEEK) {
	            ft = time(NULL) ;
	            rs = SR_OK ;
	        }

	        if (pip->headerstring == NULL) {
	            pip->headerstring = "%s  Page %3d  " ;
	        }

	    } /* end if (page headers requested) */

	    if (pip->files > 1)
	        f_startbreak = TRUE ;

/* output a header for this file */

	    fmt = ((f_si) ? "*stdin*" : ifn) ;
	    rs = bprintf(pip->ofp,".\\\"_ file=%s\n",fmt) ;
	    wlen += rs ;

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
	            debugprintf("progfile: LINE>%t",lbp,len) ;
#endif

/* handle the case of starting a new file after some previous files */

	        if ((rs >= 0) && f_startbreak) {
	            f_startbreak = FALSE ;
	            if (pip->f.eject) {
	                rs = bprintf(pip->ofp,".bp\n") ;
	    		wlen += rs ;
	            }
		    if (rs >= 0) {
	                rs = bprintf(pip->ofp,".bp\n") ;
	    		wlen += rs ;
		    }
	        }

/* handle a user requested page break */

	        while ((rs >= 0) && (*lbp == '\014')) {

	            if (pip->debuglevel > 0) {
			const int	nf = pip->files ;
			const int	np = pip->pages ;
	                fmt = "%s: requested page break\n" ;
	                bprintf(pip->efp,fmt,pn) ;
	                fmt = "%s: fn=%u page=%u line=%u\n" ;
	                bprintf(pip->efp,fmt,pn,nf,np,line) ;
	            }

	            lbp += 1 ;
	            len -= 1 ;
	            page += 1 ;
	            f_pagebreak = FALSE ;
	            if (len == 0) break ;

	            line = 0 ;
	            rs = bprintf(pip->ofp,".bp\n") ;
		    wlen += rs ;

	        } /* end while */

/* if a page break was previously scheduled, handle it */

	        if ((rs >= 0) && f_pagebreak) {
	            fmt = "%s: scheduled page break fn=%u page=%u line=%u\n" ;

	            f_pagebreak = FALSE ;
	            if (pip->debuglevel > 0) {
	                bprintf(pip->efp,fmt,pn,pip->files,pip->pages,line) ;
	            }

	            line = 0 ;
	            rs = bprintf(pip->ofp,".bp\n") ;
		    wlen += rs ;

	        } /* end if */

/* handle the blank space at the top of all pages */

	        if ((rs >= 0) && (line == 0) && (bls[0] != '\0')) {
	            rs = bprintln(pip->ofp,bls,-1) ;
		    wlen += rs ;
	        }

/* are we at a page header? */

	        if ((rs >= 0) && (line == 0) && pip->f.headers) {
	    	    const int	hlen = LINEBUFLEN ;
	    	    char	hbuf[LINEBUFLEN + 1] ;
		    if ((rs = mkheader(pip,hbuf,hlen,ifn,f_si,ft,page)) >= 0) {
	            	rs = bprintf(pip->ofp,"%t\n\n",hbuf,rs) ;
		    }
		}

/* process this new line - expand it */

		if (rs >= 0) {
		    const int	elen = LINEBUFLEN ;
		    char	ebuf[LINEBUFLEN+1] ;
		    if ((rs = expandline(ebuf,elen,lbp,len,&f_esc)) >= 0) {
			const int	el = rs ;

/* check for user specified leading offset */

	        	if (pip->coffset)
	            	    bwrite(pip->ofp,pip->blanks,pip->coffset) ;

/* write the expanded line data out */

	        	rs = bprintln(pip->ofp,ebuf,el) ;
			wlen += rs ;

		    } /* end if (expandline) */
	        } /* end if (ok) */

/* finally, check for page break */

	        line += 1 ;
	        if ((rs >= 0) && (line >= pip->pagelines)) {

	            if (pip->debuglevel > 0) {
	                fmt = "%s: forced page break fn=%d page=%d line=%d\n" ;
	                bprintf(pip->efp,fmt,pn,pip->files,pip->pages,line) ;
	            }

	            line = 0 ;
	            page += 1 ;
	            f_pagebreak = TRUE ;
	        }

	    } /* end while (main file line reading loop) */

	    if (line > 0)
	        page += 1 ;

	    pip->pages += page ;
	    pip->files += 1 ;

	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


static int mkheader(PROGINFO *pip,char *hbuf,int hlen,cchar *ifn,int f_si,
		time_t pt,int pn)
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	if (pip->headerstring != NULL) {
	    if ((rs = sbuf_start(&b,hbuf,hlen)) >= 0) {
	        cchar	*fmt = pip->headerstring ;
	        char	tbuf[TIMEBUFLEN+1] ;
	        timestr_edate(pt,tbuf) ;
	        if ((rs = sbuf_printf(&b,fmt,tbuf,(pn+1))) >= 0) {
		    if (f_si) {
		        ifn = "** standard input **" ;
		    }
		    rs = sbuf_strw(&b,ifn,-1) ;
	        } /* end if (sbuf_printf) */
	        rs1 = sbuf_finish(&b) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (sbuf) */
	} /* end if */
	return rs ;
}
/* end subroutine (mkheader) */


