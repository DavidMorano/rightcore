/* progfile */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar
	types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine provides the actual check and fix on the files
	specified.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
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
#define	SUFBUFLEN	MAXNAMELEN
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	mailmsgmathdr(const char *,int,int *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	fileneed(PROGINFO *,bfile *) ;
static int	filefix(PROGINFO *,bfile *) ;
static int	procfixer(PROGINFO *,cchar *,bfile *) ;


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
	"SENDER",
	"IN-REPLY-TO",
	"FORWARDED-TO",
	"REFERENCES",
	"SUBJECT",
	"CONTENT-TRANSFER-ENCODING",
	NULL
} ;


/* exported subroutines */


int progfile(pip,pop,fname)
PROGINFO	*pip ;
PARAMOPT	*pop ;
const char	fname[] ;
{
	const time_t	daytime = pip->daytime ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		bnl ;
	int		wlen = 0 ;
	int		f_skipfile ;
	const char	*bnp ;
	const char	*tp ;

	if (pop == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	    debugprintf("progfile: fname=%s\n",fname) ;
	    debugprintf("progfile: progmode=%u\n",pip->progmode) ;
	    debugprintf("progfile: suffix=%u\n",pip->f.suffix) ;
	}
#endif /* CF_DEBUG */

	rs = 0 ;
	pip->c_total += 1 ;

/* check if this file has the correct file-suffix */

	f_skipfile = FALSE ;
	if (pip->f.suffix) {

	    bnl = sfbasename(fname,-1,&bnp) ;

	    if ((tp = strnrchr(bnp,bnl,'.')) != NULL) {
	        rs1 = vecstr_find(&pip->suffixes,(tp + 1)) ;
	        if (rs1 < 0)
	            f_skipfile = TRUE ;
	    } else
	        f_skipfile = TRUE ;

	} /* end if (suffix check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfile: MIDDLE f_skipfile=%u\n",f_skipfile) ;
#endif

	if (! f_skipfile) {
	    struct ustat	sb ;

	    rs1 = u_stat(fname,&sb) ;

	    if ((rs1 < 0) || (! S_ISREG(sb.st_mode)))
	        f_skipfile = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3)) {
	        char	timebuf[TIMEBUFLEN + 1] ;
	        debugprintf("progfile: ageint=%u \n",
	            pip->intage) ;
	        debugprintf("progfile: daytime=%s\n",
	            timestr_logz(daytime,timebuf)) ;
	        debugprintf("progfile: mtime=%s\n",
	            timestr_logz(sb.st_mtime,timebuf)) ;
	    }
#endif /* CF_DEBUG */

	    if ((! f_skipfile) && (pip->intage > 0)) {

	        if ((daytime - sb.st_mtime) < pip->intage)
	            f_skipfile = TRUE ;

	    } /* end if */

	} /* end if (age check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfile: FINAL f_skipfile=%u\n",f_skipfile) ;
#endif

	if (! f_skipfile) {
	    bfile	infile, *ifp = &infile ;

/* process this file */

	    if ((fname[0] == '\0') || (fname[0] == '-'))
	        fname = BFILE_STDIN ;

	    if ((rs = bopen(ifp,fname,"rw",0666)) >= 0) {

	        rs1 = bcontrol(ifp,BC_LOCKWRITE,0) ;
	        if (rs1 < 0) {
	            rs = 0 ;
	            f_skipfile = TRUE ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progfile: fileneed()\n") ;
#endif

	        if (! f_skipfile) {

	            pip->c_scanned += 1 ;
	            rs = fileneed(pip,ifp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progfile: fileneed() rs=%d\n",rs) ;
#endif

	        }

	        if ((rs > 0) && (! f_skipfile)) {

	            rs = 1 ;
	            if (! pip->f.nochange) {

	                bseek(ifp,0L,SEEK_SET) ;

	                pip->c_fixed += 1 ;
	                rs = filefix(pip,ifp) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(3))
	                    debugprintf("progfile: filefix() rs=%d\n",rs) ;
#endif

	            } /* end if (allowed to do the change-fix) */

	            wlen += rs ;

	        } /* end if (needed fixing) */

	        rs1 = bclose(ifp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (opened file) */

	} /* end if (processing file) */

/* should we put out an informational message? */

	if (pip->debuglevel > 0) {
	    int	rc = (rs >= 0) ? wlen : rs ;

	    bprintf(pip->efp,"%s: file=%s (%d)\n",
	        pip->progname,fname,rc) ;

	} /* end if */

	if ((pip->verboselevel >= 3) && (wlen > 0))
	    bprintf(pip->ofp,"%s (%d)\n",fname,wlen) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfile: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


static int fileneed(pip,ifp)
PROGINFO	*pip ;
bfile		*ifp ;
{
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		len, hi ;
	int		kl ;
	int		f_bol, f_eol ;
	int		f_need = FALSE ;
	const char	*kp ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	f_bol = TRUE ;
	while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	    len = rs ;

	    f_eol = (lbuf[len - 1] == '\n') ;

	    if (f_bol) {

	        if ((len == 1) && f_eol)
	            break ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progfile/fileneed: line=>%t<\n",
	                lbuf,strlinelen(lbuf,len,40)) ;
#endif

	        kp = lbuf ;
	        kl = mailmsgmathdr(lbuf,len,NULL) ;

	        if ((kl > 0) && 
	            ((hi = matcasestr(leaders,kp,kl)) >= 0)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3)) {
	                debugprintf("progfile/fileneed: leader=>%s<\n",
	                    leaders[hi]) ;
	                debugprintf("progfile/fileneed: k=>%t<\n",kp,kl) ;
	            }
#endif

	            f_need = (strncmp(leaders[hi],kp,kl) != 0) ;
	            if (f_need) break ;

	        } /* end if */

	    } /* end if (BOL) */

	    f_bol = f_eol ;
	} /* end while (reading lines) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("progfile/fileneed: ret rs=%d f_need=%u\n",rs,f_need) ;
#endif

	return (rs >= 0) ? f_need : rs ;
}
/* end subroutine (fileneed) */


static int filefix(PROGINFO *pip,bfile *ifp)
{
	int		rs ;
	int		wlen = 0 ;
	char		template[MAXPATHLEN + 1] ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = mkpath2(template,pip->tmpdname,"msgcleanXXXXXX")) >= 0) {
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = mktmpfile(tbuf,0660,template)) >= 0) {
		rs = procfixer(pip,tbuf,ifp) ;
		wlen = rs ;
	        if (rs < 0) {
		    u_unlink(tbuf) ;
		}
	    } /* end if (mktmpfile) */
	} /* end if (mkpath) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filefix) */


static int procfixer(PROGINFO *pip,cchar *fname,bfile *ifp)
{
	bfile		tfile, *tfp = &tfile ;
	int		rs ;
	int		rs1 ;
	int		hi ;
	int		kl ;
	int		rlen ;
	int		wlen = 0 ;
	int		f_eoh ;
	int		f ;
	const char	*kp ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = bopen(tfp,fname,"rw",0664)) >= 0) {
	    offset_t	boff ;
	    offset_t	off ;
	    offset_t	off_seek = -1 ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		f_bol, f_eol ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    u_unlink(fname) ;

	    rlen = 0 ;
	    off = 0 ;
	    f_eoh = FALSE ;
	    f_bol = TRUE ;
	    while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;

	        f = FALSE ;
	        if (f_bol) {

	            if ((len == 1) && f_eol) {
	                f_eoh = TRUE ;
	                break ;
	            }

	            kp = lbuf ;
	            if ((kl = mailmsgmathdr(lbuf,len,NULL)) > 0) {
	                if ((hi = matcasestr(leaders,kp,kl)) >= 0) {
	                    if (strncmp(leaders[hi],kp,kl) != 0) f = TRUE ;
			}
	            } /* end if */

	        } /* end if (BOL) */

	        if (f) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(3))
	                debugprintf("progfile/filefix: "
			"headkey=%t\n",kp,kl) ;
#endif

	            if (off_seek < 0)
	                off_seek = off ;

	            if ((rs = bprintf(tfp,"x-%t:",kp,kl)) >= 0) {
	                const char	*tp ;
	                if ((tp = strnchr(lbuf,len,':')) != NULL) {
	                    rs = bwrite(tfp,(tp + 1),(len-((tp+1)-lbuf))) ;
			}
	            } /* end if (bprintf) */

	        } else if (off_seek >= 0)
	            rs = bwrite(tfp,lbuf,len) ;

	        off += len ;
	        f_bol = f_eol ;

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    rlen = off ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(3))
	        debugprintf("progfile/filefix: rs=%d off_seek=%llu\n",
	            rs,off_seek) ;
#endif

/* do we need to copy the rest of the file to the TMPFILE? */

	    if ((rs >= 0) && (off_seek >= 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progfile/filefix: "
			"copying_remainder\n") ;
#endif

	        if (f_eoh)
	            rs = bputc(tfp,'\n') ;

	        if (rs >= 0) {
	            rs = bcopyblock(ifp,tfp,-1) ;
	            rlen += rs ;
	        }

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progfile/filefix: "
			"remainder rs=%d wlen=%u\n",
	                rs,wlen) ;
#endif

	    } /* end if (copying remainder) */

/* copy the temporary file back to the original */

	    wlen = 0 ;
	    if ((rs >= 0) && (off_seek >= 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progfile/filefix: copying_back\n") ;
#endif

	        bseek(ifp,off_seek,SEEK_SET) ;

	        bseek(tfp,0L,SEEK_SET) ;

	        wlen = off_seek ;
	        if ((rs = bcopyblock(tfp,ifp,-1)) >= 0) {
	            wlen += rs ;
	            if (rlen > wlen) {
	                boff = wlen ;
	                rs = btruncate(ifp,boff) ;
		    }
	        } /* end if (bcopyblock) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(3))
	            debugprintf("progfile/filefix: "
			"copy-back rs=%d wlen=%u\n",
	                rs,wlen) ;
#endif

	    } /* end if (copying rest of file and back again) */

	    rs1 = bclose(tfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procfixer) */


