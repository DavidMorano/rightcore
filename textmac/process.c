/* process */

/* clean up the file given as an argument */
/* last modified %G% version %I% */


#define	CF_DEBUG	0


/* revision history:

	= 1992-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This module will take as input a fname and it will perform
	specified cleanup activities on the contents of the file and write
	the results out to either STDOUT or back to the original file.


**************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

#include	<ascii.h>
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
extern int	mktmpfile(char *,mode_t,const char *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* exported subroutines */


int process(pip,opts,ofp,fname)
struct proginfo	*pip ;
struct options	opts ;
FILE		*ofp ;
const char	fname[] ;
{
	FILE	*ifp ;
	FILE	*wfp ;

	int	rs = SR_OK ;
	int	i, len ;
	int	rlen, wlen ;
	int	state ;
	int	newlines = 0 ;
	int	f_bol = TRUE ;
	int	f_eol ;
	int	f_newline = FALSE ;
	int	f_regular ;

	char	buf[BUFLEN + 1] ;
	char	template[MAXPATHLEN + 1] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	*oflags ;


#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("process: entered, fname=%s\n",fname) ;
	    if (opts.inplace)
	        debugprintf("process: got the inplace\n") ;
	    if (opts.leading)
	        debugprintf("process: strip leading white space\n") ;
	}
#endif /* CF_DEBUG */


/* open files */

	if ((fname == NULL) || (fname[0] == '\0') ||
	    (fname[0] == '-')) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: opening STDIN\n") ;
#endif

	    oflags = (opts.inplace) ? "r+" : "r" ;
	    ifp = fdopen(BFILE_STDIN,oflags) ;

	    if (ifp == NULL)
		rs = SR_NOEXIST ;

	} else {

	    oflags = (opts.inplace) ? "r+" : "r" ;
	    ifp = fopen(fname,oflags) ;

	    if (ifp == NULL)
		rs = SR_NOEXIST ;

	}

	if (rs < 0)
	    return rs ;

	if (opts.inplace) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: inplace tmpfile\n") ;
#endif

	    mkpath2(template, pip->tmpdname, "tcXXXXXXXXXXXX") ;

	    rs = mktmpfile(tmpfname,0666,template) ;

	    if (rs < 0)
	        return rs ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: inplace tmpfile rs=%d\n",rs) ;
#endif

	    wfp = fopen(tmpfname,"r+") ;

	    if (wfp == NULL)
		rs = SR_NOEXIST ;

	    u_unlink(tmpfname) ;

	    if (rs < 0)
	        return rs ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: inplace open rs=%d other=%d\n",rs,17) ;
#endif

	} else
	    wfp = ofp ;


/* go through the loops */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: through the loops\n") ;
#endif

	rlen = 0 ;
	wlen = 0 ;
	state = S_SEARCH ;
	while ((rs = bread(ifp,buf,BUFLEN)) > 0) {

	    len = rs ;
	    for (i = 0 ; i < len ; i += 1) {

	        int	ch, ch2 ;


#if	CF_DEBUG
	        if (pip->debuglevel > 1) {

	            debugprintf("process: character %02X\n",buf[i]) ;

	            if (buf[i] == '\004')
	                debugprintf("process: got a control-D 1 at %d\n",i) ;

	        }
#endif /* CF_DEBUG */

	        ch = buf[i] ;
	        if (buf[i] == '\004') {

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: got a control-D 2\n") ;
#endif

	            continue ;
	        }

	        f_regular = FALSE ;
	        switch (buf[i] & 0xFF) {

	            int	f_extra ;


	        case CH_LF:
	            f_eol = f_bol = TRUE ;
	            if (state == S_Z)
	                wlen += bputc(wfp,C_SUB) ;

	            state = S_SEARCH ;
	            if ((! f_newline) || (! opts.half)) {

	                if ((! opts.oneblank) || (newlines <= MAXBLANKLINES)) {

	                    wlen += bputc(wfp,'\n') ;

	                    if (opts.doublespace)
	                        wlen += bputc(wfp,'\n') ;

	                }
	            }

	            f_newline = (! f_newline) ;
	            newlines += 1 ;
	            break ;

	        case CH_CR:
	            f_bol = f_eol = TRUE ;
	            f_newline = FALSE ;
	            if (state == S_Z) {

	                wlen += bputc(wfp,C_SUB) ;

	            } else if (state == S_CR) {

	                if ((! opts.oneblank) || (newlines <= MAXBLANKLINES)) {

	                    wlen += bputc(wfp,'\n') ;

	                    if (opts.doublespace)
	                        wlen += bputc(wfp,'\n') ;

	                }

	                newlines += 1 ;

	            }

	            state = S_CR ;
	            break ;

/* a control-Z character */
	        case CH_SUB:
	            f_eol = FALSE ;
	            f_newline = FALSE ;
	            newlines = 0 ;
	            if (state == S_CR) {

	                if ((! opts.oneblank) || (newlines <= MAXBLANKLINES)) {

	                    wlen += bputc(wfp,'\n') ;

	                    if (opts.doublespace)
	                        wlen += bputc(wfp,'\n') ;

	                }

	            }

	            state = S_Z ;
	            break ;

/* we got a space character */
	        case CH_SP:
	        case CH_TAB:

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: got a space, leading=%d bol=%d\n",
	                    opts.leading,f_bol) ;
#endif

	            if (opts.leading && f_bol)
	                break ;

/* fall through to the following case */

/* handle all other characters */
	        default:
	            f_bol = f_eol = FALSE ;
	            f_newline = FALSE ;
	            f_regular = TRUE ;
	            f_extra = FALSE ;
	            newlines = 0 ;
	            if (opts.mscrap) {

	                switch (ch) {

	                case '\320':
	                case '\321':
	                    ch = '-' ;
	                    ch2 = '-' ;
	                    f_extra = TRUE ;
	                    break ;

	                case '\223':
	                case '\224':
	                case '\322':
	                case '\323':
	                    ch = '\"' ;
	                    break ;

	                case '\222':
	                case '\325':
	                    ch = '\'' ;
	                    break ;

	                } /* end switch */

	            } /* end if (M$ crap !) */

	            if (opts.lower)
	                ch = tolower(ch) ;

	            switch (state) {

	            case S_Z:
	                state = S_SEARCH ;
	                wlen += bputc(wfp,C_SUB) ;

	                wlen += bputc(wfp,ch) ;

	                break ;

	            case S_CR:
	                state = S_SEARCH ;
	                if ((! opts.oneblank) || (newlines <= MAXBLANKLINES)) {

	                    wlen += bputc(wfp,'\n') ;

	                    if (opts.doublespace)
	                        wlen += bputc(wfp,'\n') ;

	                }

	                wlen += bputc(wfp,ch) ;

	                break ;

	            default:
	                wlen += bputc(wfp,ch) ;

	            } /* end switch (on state) */

	            if (f_extra)
	                wlen += bputc(wfp,ch2) ;

	        } /* end switch (on characters) */

	    } /* end for (looping through the buffer) */

	    rlen += len ;

	} /* end while (reading data blocks) */


	if ((state == S_CR) || f_regular) {

	    if ((! opts.oneblank) || (newlines <= MAXBLANKLINES)) {

	        wlen += bputc(wfp,'\n') ;

	    }
	}

	if (opts.inplace) {

	    int	f_ok = (rs >= 0) ;


#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: inplace copyback, rlen=%d\n",rlen) ;
#endif

	    if (f_ok) {

	        rs = bseek(ifp,0L,SEEK_SET) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process: seek IN rs=%d\n",rs) ;
#endif

	        rs = bseek(wfp,0L,SEEK_SET) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process: seek OUT rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("process: rlen=%d wlen=%d\n",rlen,wlen) ;
#endif

/* copy at least 'wlen' amount, EOF will truncate early ! */

	        len = MAX(rlen,wlen) ;
	        rs = bcopyblock(wfp,ifp,len) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
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

	    fclose(wfp) ;

	} /* end if (inplace) */

	fclose(ifp) ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: ret rs=%d rlen=%d\n",rs,rlen) ;
#endif

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (process) */



