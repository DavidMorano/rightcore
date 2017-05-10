/* process */

/* last modified %G% version %I% */


#define	CF_DEBUG	1		/* run-time debugging */


/* revision history:

	= 1992-03-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module will take as input a filename and it will perform
	specified cleanup activities on the contents of the file and write
	the results out to either STDOUT or back to the original file.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		4096
#endif

#define	S_SEARCH	0		/* search for EOL */
#define	S_CR		1		/* saw a carriage return character */
#define	S_Z		2		/* saw a control-Z character */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern char	*strbasename(char *) ;


/* external variables */


/* exported subroutines */


int process(gp,filename,opts)
struct proginfo	*gp ;
char		filename[] ;
struct options	opts ;
{
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int	rs ;
	int	i, len ;
	int	rlen ;
	int	state ;
	int	f_bol = TRUE ;
	int	f_eol = FALSE ;
	int	f_newline = FALSE ;
	int	f_regular ;

	char	buf[BUFLEN + 1], *bp ;
	char	template[MAXPATHLEN + 1], tmpfname[MAXPATHLEN + 1] ;
	char	*oflags ;


#if	CF_DEBUG
	if (gp->debuglevel > 1) {
	    debugprintf("process: entered, filename=%s\n",filename) ;
	    if (opts.inplace)
	        debugprintf("process: got the inplace\n") ;
	    if (opts.leading)
	        debugprintf("process: strip leading white space\n") ;
	}
#endif /* CF_DEBUG */


/* open files */

	if ((filename == NULL) || (filename[0] == '\0') ||
	    (filename[0] == '-')) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: opening STDIN\n") ;
#endif

	    oflags = (opts.inplace) ? "drw" : "dr" ;
	    rs = bopen(ifp,BFILE_STDIN,oflags,0666) ;

	} else {

	    oflags = (opts.inplace) ? "rw" : "r" ;
	    rs = bopen(ifp,filename,oflags,0666) ;

	}

	if (rs < 0)
	    return rs ;

	if (opts.inplace) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: inplace tmpfile\n") ;
#endif

	    mkpath2(template, gp->tmpdir, "tcXXXXXXXXXXXX") ;

	    if ((rs = mktmpfile(tmpfname,0666,template)) < 0)
	        return rs ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: inplace tmpfile rs=%d\n",rs) ;
#endif

	    rs = bopen(ofp,tmpfname,"rwct",0666) ;

	    u_unlink(tmpfname) ;

	    if (rs < 0)
	        return rs ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: inplace open rs=%d other=%d\n",rs,17) ;
#endif

	} else
	    ofp = gp->ofp ;


/* go through the loops */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: through the loops\n") ;
#endif

	rlen = 0 ;
	state = S_SEARCH ;
	while ((len = bread(ifp,buf,BUFLEN)) > 0) {

	    for (i = 0 ; i < len ; i += 1) {

#if	CF_DEBUG
	        if (gp->debuglevel > 1) {

	            debugprintf("process: character %02X\n",buf[i]) ;

	            if (buf[i] == '\004')
	                debugprintf("process: got a control-D 1 at %d\n",i) ;

	        }
#endif /* CF_DEBUG */

	        if (buf[i] == '\004') {

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("process: got a control-D 2\n") ;
#endif

	            continue ;
	        }

	        f_regular = FALSE ;
	        switch (buf[i] & 0xFF) {

	        case CH_LF:
	            f_eol = f_bol = TRUE ;
	            if (state == S_Z)
	                bputc(ofp,CH_SUB) ;

	            state = S_SEARCH ;
	            if ((! f_newline) || (! opts.half)) {

	                bputc(ofp,'\n') ;

	                if (opts.doublespace)
	                    bputc(ofp,'\n') ;

	            }

	            f_newline = (! f_newline) ;
	            break ;

	        case CH_CR:
	            f_bol = f_eol = TRUE ;
	            f_newline = FALSE ;
	            if (state == S_Z)
	                bputc(ofp,CH_SUB) ;

	            else if (state == S_CR)
	                bputc(ofp,'\n') ;

	            state = S_CR ;
	            break ;

/* a control-Z character */
	        case CH_SUB:
	            f_eol = FALSE ;
	            f_newline = FALSE ;
	            if (state == S_CR) 
			bputc(ofp,'\n') ;

	            state = S_Z ;
	            break ;

/* we got a space character */
	        case CH_SP:
	        case CH_TAB:

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
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
	            switch (state) {

	            case S_Z:
	                state = S_SEARCH ;
	                bputc(ofp,CH_SUB) ;

	                if (opts.lower)
	                    bputc(ofp,tolower(buf[i])) ;

	                else
	                    bputc(ofp,buf[i]) ;

	                break ;

	            case S_CR:
	                state = S_SEARCH ;
	                bputc(ofp,'\n') ;

	                if (opts.lower)
	                    bputc(ofp,tolower(buf[i])) ;

	                else
	                    bputc(ofp,buf[i]) ;

	                break ;

	            default:
	                if (opts.lower)
	                    bputc(ofp,tolower(buf[i])) ;

	                else
	                    bputc(ofp,buf[i]) ;

	            } /* end switch (on state) */

	        } /* end switch (on characters) */

	    } /* end for (looping through the buffer) */

	    rlen += len ;

	} /* end while (reading data blocks) */


	if ((state == S_CR) || f_regular)
	    bputc(ofp,'\n') ;


	if (opts.inplace) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: inplace copyback, rlen=%d\n",rlen) ;
#endif

	    rs = bseek(ifp,0L,SEEK_SET) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: seek IN rs=%d\n",rs) ;
#endif

	    rs = bseek(ofp,0L,SEEK_SET) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: seek OUT rs=%d\n",rs) ;
#endif

	    rs = bcopyblock(ofp,ifp,rlen) ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: bcopyclock rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {

	        int	wlen = rs ;


	        bcontrol(ifp,BC_TRUNCATE,wlen) ;

#if	CF_DEBUG
	        {
	            struct ustat	a ;
	            if (gp->debuglevel > 1) {
	                bcontrol(ifp,BC_STAT,&a) ;
	                debugprintf("process: BIO size=%ld\n",a.st_size) ;
	                uc_truncate(filename,wlen) ;
	                u_stat(filename,&a) ;
	                debugprintf("process: SYS size=%ld\n",a.st_size) ;
	            }
	        }
#endif /* CF_DEBUG */

	    }

	    bclose(ofp) ;

	} /* end if */

	bclose(ifp) ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: exiting rlen=%d\n",rlen) ;
#endif

	return rlen ;
}
/* end subroutine (process) */



