/* procfile */

/* process an input file to the output */


#define	CF_DEBUG	1


/* revistion history :

	= 87/09/10, David A­D­ Morano

	This subroutine was originally written.


*/


/*******************************************************************

	This subroutine processes a file by writing its contents out
	to an output file with the correct pagination.



*********************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define	LINELEN		200
#define	BUFLEN		(LINELEN + LINELEN)



/* external subroutines */

extern int	expandline() ;


/* forward references */


/* local structures */


/* local data */





int procfile(gp,infname,fn,f_eject)
struct global	*gp ;
char		infname[] ;
int		fn ;
int		f_eject ;
{
	bfile		infile, *ifp = &infile ;

	struct ustat	sb ;

	int	i ;
	int	page = 0 ;
	int	line, len, rs ;
	int	len2 ;
	int	blanklines = DEFBLANKLINES ;
	int	pointlen ;
	int	f_pagebreak = FALSE ;
	int	f_stdinput = FALSE ;
	int	f_escape ;
	int	f_startbreak = FALSE ;

	char	linebuf[LINELEN + 1], *lbp ;
	char	buf[BUFLEN + 1] ;
	char	headline[LINELEN + 1] ;
	char	timebuf[100] ;
	char	*fontname = "CW" ;
	char	*cp ;


/* check the arguments */

	if ((infname == NULL) || (infname[0] == '-')) {

	    f_stdinput = TRUE ;
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	} else
	    rs = bopen(ifp,infname,"r",0666) ;

	if (rs < 0)
	    goto badinfile ;


	if (gp->f.headers) {

	    gp->maxlines = gp->maxlines - 2 ;
	    if ((rs = bcontrol(ifp,BC_STAT,&sb)) < 0) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1)
	            debugprintf("procfile: bcontrol STAT rs %d\n",rs) ;
#endif

	        sb.st_mtime = time(NULL) ;

	    }

	    if (gp->headerstring == NULL)
	        gp->headerstring = "%s  Page %3d" ;

	} /* end if (page headers requested) */


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("procfile: processing file %d\n",fn) ;
#endif

	if (fn > 1) f_startbreak = TRUE ;


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
	    if (gp->debuglevel > 1)
	        debugprintf("procfile: LINE>%W",lbp,len) ;
#endif


/* handle the case of starting a new file after some previous files */

	    if (f_startbreak) {

	        f_startbreak = FALSE ;
	        if (f_eject)
	            bprintf(gp->ofp,".bp\n") ;

	        bprintf(gp->ofp,".bp\n") ;

	    }


/* handle a user requested page break */

	    while (*lbp == '\014') {

	        if ((gp->debuglevel > 0) || gp->f.verbose)
	            bprintf(gp->efp,
	                "%s: requested page break at fn=%d page=%d line=%d\n",
	                gp->progname,fn,page,line) ;

	        lbp += 1 ;
	        len -= 1 ;
	        page += 1 ;
	        f_pagebreak = FALSE ;

	        if (len == 0) goto done ;

	        line = 0 ;
	        bprintf(gp->ofp,".bp\n") ;

	    } /* end while */

/* if a page break was previously scheduled, handle it */

	    if (f_pagebreak) {

	        f_pagebreak = FALSE ;
	        if ((gp->debuglevel > 0) || gp->f.verbose)
	            bprintf(gp->efp,
	                "%s: scheduled page break at fn=%d page=%d line=%d\n",
	                gp->progname,fn,page,line) ;

	        line = 0 ;
	        bprintf(gp->ofp,".bp\n") ;

	    } /* end if */


/* handle the blank space at the top of all pages */

	    if ((line == 0) && (gp->blankstring[0] != '\0'))
	        bprintf(gp->ofp,"%s",gp->blankstring) ;


/* are we at a page header ? */

	    if ((line == 0) && gp->f.headers) {

	        len2 = sprintf(headline,gp->headerstring,
	            timestr_edate(sb.st_mtime,timebuf),page + 1) ;

	        strncpy(headline + len2,gp->blanks,40) ;

	        if (f_stdinput)
	            strcpy(headline + 40,"** standard input **") ;

	        else
	            strcpy(headline + 40,infname) ;

	        bprintf(gp->ofp,"%s\n\n",headline) ;

	    } /* end if (page header processing) */


/* process this new line */

/* expand it */

	    len = expandline(lbp,len,buf,BUFLEN,&f_escape) ;

/* check for user specified leading offset */

	    if (gp->coffset) bwrite(gp->ofp,gp->blanks,gp->coffset) ;

/* write the expanded line data out */

	    bwrite(gp->ofp,buf,len) ;

/* do we need a trailing EOL ? */

	    if (buf[len - 1] != '\n') bputc(gp->ofp,'\n') ;

/* finally, check for page break */

	    line += 1 ;
	    if (line >= gp->maxlines) {

	        if ((gp->debuglevel > 0) || gp->f.verbose)
	            bprintf(gp->efp,
	                "%s: forced page break at fn=%d page=%d line=%d\n",
	                gp->progname,fn,page,line) ;

	        line = 0 ;
	        page += 1 ;
	        f_pagebreak = TRUE ;
	    }

	} /* end while (main file line reading loop) */


	if (line > 0) page += 1 ;


done:
	bclose(ifp) ;

	return page ;

badinfile:
	return BAD ;
}
/* end subroutine (procfile) */



