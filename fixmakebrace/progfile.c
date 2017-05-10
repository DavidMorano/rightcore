/* procfile */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar
	types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine provides the actual check and fix on the files
	specified.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	SUFBUFLEN
#define	SUFBUFLEN		MAXNAMELEN
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	sfthing(const char *,int,const char *,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	progfiler(PROGINFO *,bfile *,const char *) ;
static int	fileneed(struct proginfo *,bfile *) ;
static int	filefix(struct proginfo *,bfile *) ;


/* local variables */

static const char	ssb[] = {
	CH_LBRACE, CH_RBRACE, 0
} ;

static const char	ssp[] = {
	CH_LPAREN, CH_RPAREN, 0
} ;


/* exported subroutines */


int progfile(pip,ofp,fp,fl)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*fp ;
int		fl ;
{
	NULSTR		n ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	const char	*fname ;
	if (fp == NULL) return SR_FAULT ;
	if ((rs = nulstr_start(&n,fp,fl,&fname)) >= 0) {
	    rs = progfiler(pip,ofp,fname) ;
	    wlen += rs ;
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


/* ARGSUSED */
static int progfiler(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	*fname ;
{
	bfile		infile ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	int		f_skipfile = FALSE ;

	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: fname=%s\n",fname) ;
#endif /* CF_DEBUG */

	pip->c_total += 1 ;

/* check if this file has the correct file-suffix */


	if (! f_skipfile) {

/* process this file */

	    if ((fname[0] != '\0') && (fname[0] != '-')) {
	        rs = bopen(&infile,fname,"rw",0666) ;
	    } else
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

	if ((pip->verboselevel >= 3) && (wlen > 0)) {
	    bprintf(pip->ofp,"%s\n",fname) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("procfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progfiler) */


static int fileneed(pip,ifp)
struct proginfo	*pip ;
bfile		*ifp ;
{
	int		rs ;
	int		len ;
	int		f_bol, f_eol ;
	int		f_need ;
	const char	*ss ;
	char		lbuf[LINEBUFLEN + 1] ;

	ss = (pip->f.backward) ? ssp : ssb ;

	f_need = FALSE ;
	f_bol = TRUE ;
	while ((rs = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    f_eol = (lbuf[len - 1] == '\n') ;

	    f_need = FALSE ;
	    if (f_bol) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process/fileneed: line=>%t<\n",
	                lbuf,
	                ((lbuf[len - 1] == '\n') ? (len - 1) : len)) ;
#endif

	        if (sfthing(lbuf,len,ss,NULL) >= 0) {
	            f_need = TRUE ;
	            break ;
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
	bfile		tmpfile ;
	offset_t	boff ;
	offset_t	off ;
	offset_t	off_seek = -1 ;
	int		rs ;
	int		len ;
	int		kl, sl, cl ;
	int		rlen ;
	int		wlen = 0 ;
	int		f_bol, f_eol ;
	const char	*ss ;
	const char	*kp, *sp, *cp ;
	char		template[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		lbuf[LINEBUFLEN + 1] ;

	ss = (pip->f.backward) ? ssp : ssb ;

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
	f_bol = TRUE ;
	f_eol = FALSE ;
	while ((rs = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    f_eol = (lbuf[len - 1] == '\n') ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("procfile/filefix: line=>%t<\n",
	            lbuf,((f_eol) ? (len - 1) : len)) ;
#endif

	    sp = lbuf ;
	    sl = len ;
	    while ((kl = sfthing(sp,sl,ss,&kp)) >= 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procfile/filefix: sfthing() rs=%d kn=%t\n",
			kl,kp,kl) ;
#endif

	        if (off_seek < 0)
	            off_seek = off ;

	        cp = sp ;
	        cl = ((kp - 2) - sp) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("procfile/filefix: leader=>%t<\n",cp,cl) ;
#endif

		if (kl > 0) {
	        if (pip->f.backward) {
	            rs = bprintf(&tmpfile,"%t$%c%t%c",
	                cp,cl,CH_LBRACE,kp,kl,CH_RBRACE) ;
	        } else {
	            rs = bprintf(&tmpfile,"%t$%c%t%c",
	                cp,cl,CH_LPAREN,kp,kl,CH_RPAREN) ;
		}
	        wlen += rs ;
		} /* end if (non-zero-length key) */

	        sl -= ((kp + kl + 1) - sp) ;
	        sp = (kp + kl + 1) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2) && (sl > 0))
	            debugprintf("procfile/filefix: remainder=>%t<\n",
	                sp,((sp[sl - 1] == '\n') ? (sl - 1) : sl)) ;
#endif

	        if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (sl > 0) && (off_seek >= 0)) {
	        rs = bwrite(&tmpfile,sp,sl) ;
	        wlen += rs ;
	    }

	    off += len ;
	    f_bol = f_eol ;

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	rlen = off ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process/filefix: off_seek=%ul rs=%d\n",
	        off_seek,rs) ;
#endif

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
		boff = wlen ;
	        rs = btruncate(ifp,boff) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process/filefix: copying_back rs=%d wlen=%u\n",
	            rs,wlen) ;
#endif

	} /* end if (copying rest of file and back again) */

	bclose(&tmpfile) ;

ret1:
	if (tmpfname[0] != '\0') {
	    u_unlink(tmpfname) ;
	}

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filefix) */


