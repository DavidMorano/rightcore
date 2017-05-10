/* progencode */

/* encode a file (encoded in BASE64) */


#define	CF_DEBUG	0		/* compile-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was written from scratch.


*/

/* Copyright © 1996 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine reads the given input file and encodes the data there
	in BASE64 and then outputs it to the output file.  Optional text mode
	processing is available.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	BASE64LINELEN	76
#define	BASE64BUFLEN	((BASE64LINELEN / 4) * 3)

#undef	BUFLEN
#define	BUFLEN		(100 * BASE64BUFLEN)


/* external subroutines */

extern int	base64_e(const char *,int,char *) ;


/* external variables */


/* global variables */


/* local structures */

struct outbuf {
	char		*buf ;
	int		i ;
} ;


/* forward references */

static int	outbase64(PROGINFO *,bfile *,char *,int) ;
static int	putout(PROGINFO *,bfile *,struct outbuf *,int) ;


/* local variables */


/* exported subroutines */


int progencode(pip,ofp,name)
PROGINFO	*pip ;
const char	name[] ;
bfile		*ofp ;
{
	int		rs ;
	int		len ;
	int		size ;
	int		olen = 0 ;
	char		*buf ;

	if (name == NULL) return SR_FAULT ;
	if (name[0] == '\0') return SR_INVALID ;

	size = BUFLEN + 4 ;
	rs = uc_valloc(size,&buf) ;
	if (rs < 0)
	    goto badalloc ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("progencode: entered name=\"%s\"\n",name) ;
#endif

	olen = 0 ;
	if (pip->f.text) {
	    struct outbuf	ob ;
	    bfile	infile, *ifp = &infile ;
	    int		ch ;

	    ob.buf = buf ;
	    ob.i = 0 ;
	    if (name[0] == '-') name = BFILE_STDIN ;
	    if ((rs = bopen(ifp,name,"r",0666)) >= 0) {

	        while ((ch = bgetc(ifp)) >= 0) {

	            if (ch == '\n') {

			if (rs >= 0) {
	                    rs = putout(pip,ofp,&ob,'\r') ;
	            	    olen += rs ;
			}

			if (rs >= 0) {
	                    rs = putout(pip,ofp,&ob,'\n') ;
	            	    olen += rs ;
			}

	            } else {
	                rs = putout(pip,ofp,&ob,ch) ;
	            	olen += rs ;
		    }

	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (ob.i > 0)) {
	            rs = outbase64(pip,ofp,buf,ob.i) ;
		    olen += rs ;
	        } /* end if */

	        bclose(ifp) ;
	    } /* end if (opnened input file) */

	} else {
	    int		ifd ;

	    if (name[0] != '-') {
	        rs = uc_open(name,O_RDONLY,0666) ;
	        ifd = rs ;
	    } else
	        ifd = FD_STDIN ;

	    if (rs >= 0) {

	        while ((rs = u_read(ifd,buf,BUFLEN)) > 0) {
		    len = rs ;
	            rs = outbase64(pip,ofp,buf,len) ;
	            olen += rs ;
	            if (rs < 0) break ;
	        } /* end while */

	        u_close(ifd) ;
	    } /* end if (opened input file) */

	} /* end if (straight or text-mode) */

badopen:
	uc_free(buf) ;

badalloc:
	return (rs >= 0) ? olen : rs ;
}
/* end subroutine (progencode) */


/* local subroutines */


/* put out a character to the text staging buffer */
static int putout(pip,ofp,bp,ch)
PROGINFO	*pip ;
bfile		*ofp ;
struct outbuf	*bp ;
int		ch ;
{
	int		rs = SR_OK ;

	bp->buf[bp->i++] = ch ;
	if (bp->i == BUFLEN) {

	    rs = outbase64(pip,ofp,bp->buf,BUFLEN) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("putout: outbase64() rs=%d\n",rs) ;
#endif

	    bp->i = 0 ;
	}

	return rs ;
}
/* end subroutine (putout) */


/* write out in BASE64! */
static int outbase64(pip,ofp,buf,buflen)
PROGINFO	*pip ;
bfile		*ofp ;
char		buf[] ;
int		buflen ;
{
	int		rs = SR_OK ;
	int		rlen, i ;
	int		mlen, len ;
	int		wlen = 0 ;
	char		linebuf[BASE64LINELEN + 4] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("outbase64: entered buflen=%d\n",buflen) ;
#endif

	rlen = buflen ;
	i = 0 ;
	while ((rs >= 0) && (rlen > 0)) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("outbase64: rlen=%d\n",rlen) ;
#endif

	    mlen = MIN(BASE64BUFLEN,rlen) ;
	    len = base64_e(buf + i,mlen,linebuf) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("outbase64: mlen=%d base64_e-len=%d\n",
	            mlen,len) ;
	        debugprintf("outbase64: >%W<\n",linebuf,len) ;
	    }
#endif

	    if (pip->f.test) {

	        rs = 0 ;
	        if (len > 0) {
	            int	m ;
	            int	c = 1 ;
	            int	j = 0 ;
	            int	ol = 0 ;

	            while ((rs >= 0) && (j < len)) {

	                m = MIN(c,(len - j)) ;
	                rs = bprintf(ofp,"%t\n",(linebuf + j),m) ;
	                ol += rs ;

	                j += m ;
	                c += 1 ;

	            } /* end while */

	            if (rs >= 0)
	                rs = ol ;

	        } /* end if */

	        wlen += rs ;

	    } else {
	        rs = bwrite(ofp,linebuf,len) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	    rlen -= mlen ;
	    i += mlen ;

	    if (rs < 0) break ;
	} /* end while */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("outbase64: returning rs=%d wlen=%d\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (outbase64) */


