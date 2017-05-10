/* process */

/* add content length and content lines to a mail header */
/* last modified %G% version %I% */


#define	CF_DEBUG	1


/* revision history:

	= 1998/07-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes the input so that it conforms MORE properly to
        a mail message format with a content length header.

        The whole way in which this subroutine worls is very much flawed. RFC822
        headers are NOT handled any where near correctly! This is really garbage
        but who wants to write this stuff from scratch?


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<sys/param.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<userinfo.h>
#include	<mailmsgmatenv.h>
#include	<localmisc.h>

#include	"headerkeys.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	FMAT(cp)	((cp)[0] == 'F')


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfdecl(const char *,int,long *) ;
extern int	mheader() ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_edate(time_t,char *) ;
extern char	*timestr_hdate(time_t,char *) ;


/* external variables */


/* local structures */


/* local variables */







int process(pip,ofp,filename)
struct proginfo	*pip ;
bfile		*ofp ;
char		filename[] ;
{
	struct ustat	stat_i ;

	MSGMATENV	me ;

	bfile	infile, *ifp = &infile ;
	bfile	tmpfile, *tfp = &tmpfile ;

	offset_t	off_clen ;
	offset_t	offset ;

	time_t	daytime ;
	time_t	filedate ;

	long	clen = 0 ;

	int	rs ;
	int	len, line, clines ;
	int	flen ;
	int	fl, ml, cl ;
	int	oflags ;
	int	f_header ;
	int	f_bol, f_eol ;
	int	f_clen ;
	int	f_date = FALSE ;
	int	f_article = FALSE ;
	int	f_newsgroups = FALSE ;
	int	f_mailer = FALSE ;
	int	f_writeback = FALSE ;
	int	f_seekable = FALSE ;
	int	f_leading = TRUE ;
	int	f_first = TRUE ;
	int	f ;

	char	buf[BUFLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: entered\n") ;
#endif


/* initial initialization */

	tmpfname[0] = '\0' ;
	daytime = time(NULL) ;

	if (strcmp(filename,"-") == 0)
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	    else
	    rs = bopen(ifp,filename,"r",0666) ;

	if (rs < 0)
	    goto badinopen ;

/* get the total length of the input file (if we can) */

	flen = -1 ;
	if (bcontrol(ifp,BC_STAT,&stat_i) >= 0)
	    flen = stat_i.st_size ;

	filedate = stat_i.st_mtime ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: input file len=%ld\n",
	        flen) ;
#endif

	if ((flen < 0) || (bseek(ifp,0L,SEEK_CUR) < 0))
	    f_writeback = TRUE ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: writeback=%d seekable=%d\n",
	        f_writeback,f_seekable) ;
#endif

	if (! pip->f.seekable) {

	    if (f_writeback) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process: making temporary file - wb=%d\n",
	                f_writeback) ;
#endif

	        mkpath2(tmpfname, pip->tmpdname, "acXXXXXXXXXXXX") ;

	        rs = bopen(tfp,tmpfname,"rwct",0600) ;

	        if (rs < 0)
	            goto badtmpopen ;

	        while ((len = bread(ifp,buf,BUFLEN)) > 0) {

	            if ((rs = bwrite(tfp,buf,len)) < len)
	                goto badtmpwrite ;

	        } /* end while */

	        bclose(ifp) ;

	        bflush(tfp) ;

/* rewind the input file */

	        bseek(tfp,0L,SEEK_SET) ;

	        ifp = tfp ;
	        rs = bcontrol(ifp,BC_STAT,&stat_i) ;

	        if (rs < 0)
	            goto badstat ;

	        flen = stat_i.st_size ;
	        f_writeback = FALSE ;

	    } /* end if (both input and output were not seekable !) */

	} else
	    bseek(ofp,0L,SEEK_END) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: before hard processing - wb=%d\n",f_writeback) ;
#endif

/* do the hard processing */

	f_header = TRUE ;
	line = 0 ;
	clines = 0 ;
	f_bol = TRUE ;
	f_clen = FALSE ;
	offset = 0 ;
	while ((rs = breadline(ifp,buf,BUFLEN)) > 0) {

	    len = rs ;
	    f_eol = (buf[len - 1] == '\n') ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: got a line - bol=%d eol=%d\n%W",
	            f_bol,f_eol,buf,len) ;
#endif

	    offset += len ;

	    if (f_header) {

	        if (f_first) {

	            f = FMAT(buf) ;
		    if (f) {
			if ((rs = mailmsgmatenv(&me,buf,(len-1)) >= 0)) {
			   f = rs ;
			} else
			   break ;
		    }

	            if (f) {
	                char	datestr[TIMEBUFLEN + 1] ;

	                datestr[0] = '\0' ;
	                if ((me.date.p != NULL) && (me.date.p[0] != '\0')) {

	                    ml = MIN(TIMEBUFLEN,me.date.len) ;
	                    strwcpy(datestr,me.date.p,ml) ;

	                }

	                if (datestr[0] == '\0')
	                    timestr_edate(daytime,datestr) ;

	                cp = me.address.p ;
	                cl = me.address.len ;
	                if ((cp == NULL) || (*cp == '\0')) {
	                    cp = pip->address_from ;
	                    cl = -1 ;
	                }

	                bprintf(ofp,"From %t %s ",
	                    cp,cl,datestr) ;

	                if ((me.remote.p != NULL) && 
	                    (me.remote.p[0] != '\0'))
	                    bprintf(ofp," remote from %t",
	                        me.remote.p,me.remote.len) ;

	                bprintf(ofp,"\n") ;

	                len = 0 ;	/* cancel standard write */

	            } else {

	                if (pip->f.wantenvelope) {

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("process: checking/adding envelope\n") ;
#endif

	                    if (pip->address_from == NULL)
	                        pip->address_from = pip->up->username ;

	                    cp = timestr_edate(filedate,timebuf) ;

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("process: edate=>%s<\n",cp) ;
#endif

	                    bprintf(ofp,"From %s %s\n",
	                        pip->address_from,
	                        timestr_edate(filedate,timebuf)) ;

	                } /* end if (adding envelope header) */

	            } /* end if */

/* add an optional "received" postmark */

	            if (pip->f.postmark) {

	                bprintf(ofp,"received:\n") ;

	                if (pip->r_machine != NULL)
	                    bprintf(ofp,"\tfrom %s\n",pip->r_machine) ;

	                bprintf(ofp,"\tby %s.%s\n",
	                    pip->nodename, pip->domainname) ;

	                if (pip->r_transport != NULL)
	                    bprintf(ofp,"\twith %s\n",pip->r_transport) ;

#ifdef	COMMENT
	                bprintf(ofp,"\tid %s\n",pip->r_transport) ;

	                bprintf(ofp,"\tfor %s\n",pip->r_transport) ;
#endif

	                bprintf(ofp,"\t; %s\n",
	                    timestr_hdate(daytime,timebuf)) ;

	            } /* end if (optional postmark) */

	        } /* end if (first) */

/* check for the end-of-header condition */

	        if (buf[0] == '\n') {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf(
	                    "process: end of the header \n") ;
#endif

	            f_header = FALSE ;

/* should we add a date ? */

	            if (! f_date) {

	                f_date = TRUE ;
	                bprintf(ofp,
	                    "%s: %s\n",HK_DATE,
	                    timestr_hdate(daytime,timebuf)) ;

	            } /* end if (adding "date" header) */

/* check to see if we should add a "x-mailer" header */

	            if ((! f_mailer) && (pip->header_mailer != NULL) && 
	                (pip->header_mailer[0] != '\0')) {

	                bprintf(ofp,
	                    "%s: %s\n",HK_MAILER,pip->header_mailer) ;

	            } /* end if (adding "x-mailer" header) */

/* check to see if we should add a "newsgroups" header */

	            if ((! f_newsgroups) && (pip->header_newsgroups != NULL) && 
	                (pip->header_newsgroups[0] != '\0')) {

	                bprintf(ofp,
	                    "%s: %s\n",HK_NEWSGROUPS,pip->header_newsgroups) ;

	            } /* end if (adding "newsgroups" header) */

/* check to see if we should add a "article-id" header */

	            if ((! f_article) && (pip->header_article != NULL) && 
	                (pip->header_article[0] != '\0')) {

	                bprintf(ofp,
	                    "%s: <%s>\n",HK_ARTICLEID,pip->header_article) ;

	            } /* end if (adding "article-id" header) */

/* check to see if we should add a "content-length" header */

	            if (! f_clen) {

	                if (! f_writeback) {

	                    clen = flen - offset ;
	                    bprintf(ofp,"%s: %ld\n",HK_CLEN,
	                        (flen - offset)) ;

	                } else {

/* do not use "input" offset since we may have written differently than read */

	                    btell(ofp,&off_clen) ;

	                    off_clen += (strlen(HK_CLEN) + 2) ;

#if	CF_DEBUG
	                    if (pip->debuglevel > 1)
	                        debugprintf("process: 1 
				"offset=%lld off_clen=%lld\n",
	                            offset,off_clen) ;
#endif

	                    bprintf(ofp,"%s:           \n",HK_CLEN) ;

#if	CF_DEBUG
	                    if (pip->debuglevel > 1) {
	                        btell(ofp,&offset) ;
	                        debugprintf("process: 2 current offset=%ld\n",
	                            offset) ;
	                    }
#endif /* CF_DEBUG */

	                    offset = 0 ;

	                } /* end if (writeback) */

	            } /* end if (adding content-length) */

	        } else if (fl = mheader(HK_CLEN,buf)) {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: already have clen\n") ;
#endif

	            f_clen = TRUE ;
	            if (pip->debuglevel > 0) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("process: calling cfdecl\n") ;
#endif

	                if (cfdecl(buf + fl,-1,&clen) < 0)
	                    clen = -1 ;

	                bprintf(pip->efp,"%s: clen=%ld\n",clen) ;

	            } /* end if (verbose) */

	        } else if (mheader(HK_DATE,buf)) {

	            f_date = TRUE ;

	        } else if (mheader(HK_ARTICLEID,buf)) {

	            f_article = TRUE ;

	        } else if (mheader(HK_NEWSGROUPS,buf)) {

	            f_newsgroups = TRUE ;

	        } else if (mheader(HK_MAILER,buf)) {

	            f_mailer = TRUE ;

	        } else if (f_leading && (! pip->f.wantenvelope) &&
	            ((strncmp(buf,"From ",5) == 0) ||
	            (strncmp(buf,">From ",6) == 0))) {

	            cp = buf + 5 ;
	            while (CHAR_ISWHITE(*cp))
	                cp += 1 ;

/* do not write out this line if it is a UNIX envelope header */

	            if (*cp != ':')
	                len = 0 ;

	                else
	                f_leading = FALSE ;

	        } else {

	            f_leading = FALSE ;

	        } /* end if (header string match cascade) */

/* end of being within the header area processing */

	    } else {

/* outside of header, start counting content lines on EOLs */

	        if (f_eol)
	            clines += 1 ;

	    } /* end if */

	    if (len > 0)
	        bwrite(ofp,buf,len) ;

/* count total lines including the header section */

	    if (f_eol)
	        line += 1 ;

	    f_first = FALSE ;
	    f_bol = f_eol ;

	} /* end while (reading lines) */

/* OK, do we have any writeback stuff to do ? */

	if ((rs >= 0) && (! f_clen) && f_writeback) {

	    bcontrol(ofp,BC_GETFL,&oflags) ;

	    if (oflags & O_APPEND) {

	        oflags &= (~ O_APPEND) ;
	        bcontrol(ofp,BC_SETFL,oflags) ;

	    } /* end if (file was in APPEND mode) */

	    bseek(ofp,off_clen,SEEK_SET) ;

	    bprintf(ofp,"%ld",offset) ;

	    clen = (long) offset ;

	} /* end if */

badstat:
badtmpopen:
badtmpwrite:
	bclose(ifp) ;

badinopen:
	return rs ;
}
/* end subroutine (process) */



