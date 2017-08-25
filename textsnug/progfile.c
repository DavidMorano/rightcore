/* progfile */

/* clean up the file given as an argument */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module will take as input a fname and it will perform specified
        cleanup activities on the contents of the file and write the results out
        to either STDOUT or back to the original file.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		4096
#endif

#define	MAXBLANKLINES	1		/* max blank lines in 'oneblank' mode */

#define	S_SEARCH	0		/* search for EOL */
#define	S_CR		1		/* saw a carriage return character */
#define	S_Z		2		/* saw a control-Z character */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	siskipwhite(const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	mktmpfile(char *,mode_t,const char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	procline(PROGINFO *,bfile *,const char *,int) ;


/* local variables */


/* exported subroutines */


int progfile(pip,ofp,fname)
PROGINFO	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	bfile		infile, *ifp = &infile ;
	bfile		workfile, *wfp = &workfile ;
	const int	llen = LINEBUFLEN ;

	int	rs = SR_OK ;
	int	i, len ;
	int	state ;
	int	newlines = 0 ;
	int	wlen ;
	int	rlen = 0 ;
	int	f_bol = TRUE ;
	int	f_eol ;
	int	f_newline = FALSE ;
	int	f_regular ;

	const char	*oflags ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	template[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("process: entered, fname=%s\n",fname) ;
	    if (pip->f.inplace)
	        debugprintf("process: got the inplace\n") ;
	    if (pip->f.leading)
	        debugprintf("process: strip leading white space\n") ;
	}
#endif /* CF_DEBUG */

/* open files */

	if ((fname == NULL) || (fname[0] == '\0') ||
	    (fname[0] == '-')) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: opening STDIN\n") ;
#endif

	    oflags = (pip->f.inplace) ? "drw" : "dr" ;
	    rs = bopen(ifp,BFILE_STDIN,oflags,0666) ;

	} else {

	    oflags = (pip->f.inplace) ? "rw" : "r" ;
	    rs = bopen(ifp,fname,oflags,0666) ;

	}

	if ((rs >= 0) && pip->f.inplace) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: inplace tmpfile\n") ;
#endif

	    mkpath2(template, pip->tmpdname, "tcXXXXXXXXXXXX") ;

	    rs = mktmpfile(tmpfname,0666,template) ;

	    if (rs >= 0) {
	        rs = bopen(wfp,tmpfname,"rwct",0666) ;
	        u_unlink(tmpfname) ;
	    }

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: inplace open rs=%d other=%d\n",rs,17) ;
#endif

	} else
	    wfp = ofp ;

	if (rs < 0)
	    goto ret0 ;

/* go through the loops */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: through the loops\n") ;
#endif

	rlen = 0 ;
	wlen = 0 ;
	state = S_SEARCH ;
	while ((rs = breadline(ifp,lbuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    rs = procline(pip,wfp,lbuf,len) ;
	    wlen += rs ;

	    if (rs < 0) break ;
	    rlen += len ;
	} /* end while (reading data blocks) */

/* is this work "in-place"? */

	if (pip->f.inplace) {
	    int	f_ok = (rs >= 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: inplace copyback, rlen=%d\n",rlen) ;
#endif

	    if (f_ok) {

	        bseek(ifp,0L,SEEK_SET) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: seek IN rs=%d\n",rs) ;
#endif

	        bseek(wfp,0L,SEEK_SET) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process: seek OUT rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: rlen=%d wlen=%d\n",rlen,wlen) ;
#endif

/* copy at least 'wlen' amount, EOF will truncate early ! */

	        len = MAX(rlen,wlen) ;
	        rs = bcopyblock(wfp,ifp,len) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: bcopyclock rs=%d\n",rs) ;
#endif

	        if (rs >= 0) {
	            int	tlen = rs ;

	            bcontrol(ifp,BC_TRUNCATE,tlen) ;

#if	CF_DEBUG
	            {
	                struct ustat	a ;
	                if (pip->debuglevel > 1) {
	                    bcontrol(ifp,BC_STAT,&a) ;
	                    debugprintf("process: BIO size=%ld\n",a.st_size) ;
	                    uc_truncate(fname,tlen) ;
	                    u_stat(fname,&a) ;
	                    debugprintf("process: SYS size=%ld\n",a.st_size) ;
	                }
	            }
#endif /* CF_DEBUG */

	        } /* end if (good write back) */

	    } /* end if (OK to write) */

	    bclose(wfp) ;
	} /* end if (inplace) */

	bclose(ifp) ;

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: ret rs=%d rlen=%d\n",rs,rlen) ;
#endif

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


static int procline(pip,wfp,lbuf,llen)
PROGINFO	*pip ;
bfile		*wfp ;
const char	lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;
	int		sl, cl ;
	int		si ;
	int		c = 0 ;
	int		wlen = 0 ;
	const char	*tp, *sp ;
	const char	*cp ;


	if ((llen > 0) && (lbuf[llen - 1] == '\n')) {
	    llen -= 1 ;
	}

	sp = lbuf ;
	sl = llen ;
	if (rs >= 0) {
	    if ((si = siskipwhite(sp,sl)) > 0) {
	        if (! pip->f.rmleading) {
	            rs = bwrite(wfp,sp,si) ;
	        }
	            sp += si ;
	            sl -= si ;
	    }
	}

	if ((rs >= 0) && pip->f.rmtrailing) {
	    while ((sl > 0) && CHAR_ISWHITE(sp[sl-1]))
	        sl -= 1 ;
	}

	if (rs >= 0) {
	    if (pip->f.rmmiddle) {

	        c = 0 ;
	        while ((cl = nextfield(sp,sl,&cp)) > 0) {

	            if (c++ > 0) {
	                rs = bputc(wfp,' ') ;
	                wlen += rs ;
	            }

	            if (rs >= 0) {
	                rs = bwrite(wfp,cp,cl) ;
	                wlen += rs ;
	            }

	            sl -= ((cp + cl) - sp) ;
	            sp = (cp + cl) ;

	            if (rs < 0) break ;
	        } /* end while */

	    } else {
	        rs = bwrite(wfp,sp,sl) ;
	        wlen += rs ;
	    }

	} /* end if */

	if (rs >= 0) {
	    rs = bputc(wfp,'\n') ;
	    wlen += rs ;
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (procline) */


