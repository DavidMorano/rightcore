/* procfile */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar
	types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine provides the actual check and fix on the files
	specified.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<paramopt.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	SUFBUFLEN
#define	SUFBUFLEN		MAXNAMELEN
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	mailmsgmathdr(const char *,int,int *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	fileneed(struct proginfo *,bfile *) ;
static int	filefix(struct proginfo *,bfile *) ;


/* local variables */

enum progmodes {
	progmode_msgclean,
	progmode_overlast
} ;

static const char	*leaders[] = {
	"TO",
	"CC",
	"BCC",
	"DATE",
	NULL
} ;







int procfile(pip,pop,fname)
struct proginfo	*pip ;
PARAMOPT	*pop ;
const char	fname[] ;
{
	bfile	infile ;

	time_t	daytime = time(NULL) ;

	int	rs, rs1 ;
	int	wlen ;
	int	bnl ;
	int	f_skipfile ;

	char	timebuf[TIMEBUFLEN + 1] ;
	char	*bnp ;
	char	*tp ;


	if (fname == NULL)
	    return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("procfile: fname=%s\n",fname) ;
	    debugprintf("procfile: progmode=%u\n",pip->progmode) ;
	    debugprintf("procfile: suffix=%u\n",pip->f.suffix) ;
	}
#endif /* CF_DEBUG */

	rs = 0 ;
	pip->c_total += 1 ;

/* check if this file has the correct file-suffix */

	f_skipfile = FALSE ;
	if (pip->f.suffix) {

	    bnl = sfbasename(fname,-1,&bnp) ;

	    tp = strnrchr(bnp,bnl,'.') ;

	    if (tp != NULL) {

		rs1 = vecstr_find(&pip->suffixes,(tp + 1)) ;

		if (rs1 < 0)
			f_skipfile = TRUE ;

	    } else
	        f_skipfile = TRUE ;

	} /* end if (suffix check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: MIDDLE f_skipfile=%u\n",f_skipfile) ;
#endif

	if (! f_skipfile) {

	    struct ustat	sb ;


	    rs1 = u_stat(fname,&sb) ;

	    if ((rs1 < 0) || (! S_ISREG(sb.st_mode)))
	        f_skipfile = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("procfile: ageint=%u \n",
	            pip->ageint) ;
	        debugprintf("procfile: daytime=%s\n",
	            timestr_logz(daytime,timebuf)) ;
	        debugprintf("procfile: mtime=%s\n",
	            timestr_logz(sb.st_mtime,timebuf)) ;
	    }
#endif /* CF_DEBUG */

	    if (! f_skipfile) {

	        if ((daytime - sb.st_mtime) < pip->ageint)
	            f_skipfile = TRUE ;

	    }

	} /* end if (age check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: FINAL f_skipfile=%u\n",f_skipfile) ;
#endif

	wlen = 0 ;
	if (! f_skipfile) {

/* process this file */

	    if ((fname[0] != '\0') && (fname[0] != '-'))
	        rs = bopen(&infile,fname,"rw",0666) ;

	    else
	        rs = bopen(&infile,BFILE_STDIN,"drw",0666) ;

	    if (rs >= 0) {

	        rs1 = bcontrol(&infile,BC_LOCKWRITE,0) ;

	        if (rs1 < 0) {
	            rs = 0 ;
	            f_skipfile = TRUE ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procfile: fileneed()\n") ;
#endif

	        if (! f_skipfile) {

		    pip->c_scanned += 1 ;
	            rs = fileneed(pip,&infile) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procfile: fileneed() rs=%d\n",rs) ;
#endif

		}

	        if ((rs > 0) && (! f_skipfile)) {

		    rs = 1 ;
	            if (! pip->f.nochange) {

	                bseek(&infile,0L,SEEK_SET) ;

		        pip->c_fixed += 1 ;
	                rs = filefix(pip,&infile) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procfile: filefix() rs=%d\n",rs) ;
#endif

	            } /* end if (allowed to do the change-fix) */

	            wlen += rs ;

	        } /* end if (needed fixing) */

	        bclose(&infile) ;

	    } /* end if (opened file) */

	} /* end if (processing file) */

/* should we put out an informational message? */

	if (pip->debuglevel > 0) {

	    int	rc = (rs >= 0) ? wlen : rs ;


	    bprintf(pip->efp,"%s: file=%s (%d)\n",
	        pip->progname, fname,rc) ;

	}

	if ((pip->verboselevel >= 3) && (wlen > 0))
	    bprintf(pip->ofp,"%s\n",fname) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfile) */


/* local subroutines */


static int fileneed(pip,ifp)
struct proginfo	*pip ;
bfile		*ifp ;
{
	int	rs ;
	int	len, hi ;
	int	kl ;
	int	f_bol, f_eol ;
	int	f_need ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*kp ;


	f_need = FALSE ;
	f_bol = TRUE ;
	while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    f_eol = (linebuf[len - 1] == '\n') ;

	    f_need = FALSE ;
	    if (f_bol) {

	        if ((len == 1) && f_eol)
	            break ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process/fileneed: line=>%t<\n",
	                linebuf,
	                ((linebuf[len - 1] == '\n') ? (len - 1) : len)) ;
#endif

		kp = linebuf ;
		kl = mailmsgmathdr(linebuf,len,NULL) ;

	        if ((kl > 0) && 
	            ((hi = matcasestr(leaders,kp,kl)) >= 0)) {

	            if (strncmp(leaders[hi],kp,kl) != 0) {
	                f_need = TRUE ;
	                break ;
	            }
	        }

	    } /* end if (BOL) */

	    f_bol = f_eol ;

	} /* end while (reading lines) */

	return (rs >= 0) ? f_need : rs ;
}
/* end subroutine (fileneed) */


static int filefix(pip,ifp)
struct proginfo	*pip ;
bfile		*ifp ;
{
	bfile	tmpfile ;

	offset_t	uoff ;
	offset_t	off ;
	offset_t	off_seek = -1 ;

	int	rs ;
	int	len, hi ;
	int	kl ;
	int	rlen, wlen ;
	int	f_bol, f_eol ;
	int	f_eoh ;
	int	f ;

	char	template[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	char	*kp ;


/* open the scratch file */

	rs = mkpath2(template,pip->tmpdname,"msgcleanXXXXXX") ;

	if (rs >= 0)
	    rs = mktmpfile(tmpfname,0666,template) ;

	if (rs < 0)
	    goto ret0 ;

	rs = bopen(&tmpfile,tmpfname,"rw",0664) ;

	if (rs < 0)
	    goto ret1 ;

	u_unlink(tmpfname) ;

	tmpfname[0] = '\0' ;

/* loop through the specified file */

	rlen = 0 ;
	off = 0 ;
	f_eoh = FALSE ;
	f_bol = TRUE ;
	while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    f_eol = (linebuf[len - 1] == '\n') ;

	    f = FALSE ;
	    if (f_bol) {

	        if ((len == 1) && f_eol) {
	            f_eoh = TRUE ;
	            break ;
	        }

		kp = linebuf ;
		kl = mailmsgmathdr(linebuf,len,NULL) ;

	        if ((kl > 0) && 
	            ((hi = matcasestr(leaders,kp,kl)) >= 0)) {

	            if (strncmp(leaders[hi],kp,kl) != 0)
	                f = TRUE ;

	        }

	    } /* end if (BOL) */

	    if (f) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("process/filefix: headkey=%t\n",kp,kl) ;
#endif

	        if (off_seek < 0)
	            off_seek = off ;

	        rs = bprintf(&tmpfile,"x-%t:",kp,kl) ;

	        if (rs >= 0) {

		    char	*tp ;


		    tp = strnchr(linebuf,len,':') ;

	            rs = bwrite(&tmpfile,
				(tp + 1),
				(len - ((tp + 1) - linebuf))) ;

		}

	    } else if (off_seek >= 0)
	        rs = bwrite(&tmpfile,linebuf,len) ;

	    if (rs < 0)
	        break ;

	    off += len ;
	    f_bol = f_eol ;

	} /* end while (reading lines) */

	rlen = off ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("process/filefix: off_seek=%llu rs=%d\n",
		off_seek,rs) ;
#endif

/* do we need to copy the rest of the file to the TMPFILE? */

	if ((rs >= 0) && (off_seek >= 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("process/filefix: copying_remainder\n") ;
#endif

	    if (f_eoh)
	        rs = bputc(&tmpfile,'\n') ;

	    if (rs >= 0) {

	        rs = bcopyblock(ifp,&tmpfile,-1) ;

		rlen += rs ;
	    }
#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("process/filefix: copying_remainder rs=%d wlen=%u\n",
		rs,wlen) ;
#endif

	} /* end if (copying remainder) */

/* copy the temporary file back to the original */

	wlen = 0 ;
	if ((rs >= 0) && (off_seek >= 0)) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("process/filefix: copying_back\n") ;
#endif

	    bseek(ifp,off_seek,SEEK_SET) ;

	    bseek(&tmpfile,0L,SEEK_SET) ;

	    wlen = off_seek ;
	    rs = bcopyblock(&tmpfile,ifp,-1) ;
	    wlen += rs ;

	    if ((rs >= 0) && (rlen > wlen)) {
		uoff = wlen ;
		rs = btruncate(ifp,uoff) ;
	    }

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("process/filefix: copying_back rs=%d wlen=%u\n",
		rs,wlen) ;
#endif

	} /* end if (copying rest of file and back again) */

	bclose(&tmpfile) ;

ret1:
	if (tmpfname[0] != '\0')
	    u_unlink(tmpfname) ;

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filefix) */



