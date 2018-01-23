/* breadline */
/* lang=C++11 */

/* "Basic I/O" package similiar to "stdio" */
/* last modifed %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_MEMCCPY	0		/* we are faster than 'memccpy()'! */


/* revision history:

	= 1998-07-01, David A­D­ Morano
	This subroutine was originally written.

	= 1999-01-10, David A­D­ Morano
        I added the little extra code to allow for memory mapped I/O. It is all
        a waste because it is way slower than without it! This should teach me
        to leave old programs alone!!!

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine reads a single line from the buffer (or where ever) and
        returns that line to the caller. Any remaining data is left for
        subsequent reads (of any kind).


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	RELOAD		struct bfile_reload


/* external subroutines */

#if	CF_DEBUGS
extern "C" int	debugprintf(cchar *,...) ;
#endif

/* external variables */


/* local structures */

struct bfile_reload {
	uint		f_partial:1 ;
	bfile_reload() {
	    f_partial = FALSE ;
	} ;
} ;


/* forward references */

static int	breadlinemap(bfile *,char *,int) ;
static int	breadlinereg(bfile *,char *,int,int) ;
static int	breload(bfile *,RELOAD *,int,int) ;

static int	isoureol(int) ;


/* local variables */


/* exported subroutines */


int breadlinetimed(bfile *fp,char *ubuf,int ulen,int to)
{
	int		rs = SR_OK ;

	if (fp == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("breadlinetimed: ent fd=%u llen=%d to=%d\n",
	    fp->fd,ulen,to) ;
#endif

	if (fp->f.nullfile) goto ret0 ;

/* continue */

	if (fp->oflags & O_WRONLY)
	    return SR_WRONLY ;

	if (fp->f.write) {
	    fp->f.write = FALSE ;
	    if (fp->len > 0) rs = bfile_flush(fp) ;
	} /* end if (switching from writing to reading) */

/* decide how we want to transfer the data */

	if (rs >= 0) {
	    if (fp->f.mapinit) {
	        rs = breadlinemap(fp,ubuf,ulen) ;
	    } else {
	        rs = breadlinereg(fp,ubuf,ulen,to) ;
	    } /* end if (read map or not) */
	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("breadlinetimed: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (breadlinetimed) */


int breadline(bfile *fp,char *ubuf,int ulen)
{

	return breadlinetimed(fp,ubuf,ulen,-1) ;
}
/* end subroutine (breadline) */


int breadln(bfile *fp,char *ubuf,int ulen)
{
	return breadlinetimed(fp,ubuf,ulen,-1) ;
}


int breadlntimed(bfile *fp,char *ubuf,int ulen,int to)
{
	return breadlinetimed(fp,ubuf,ulen,to) ;
}


/* local subroutines */


static int breadlinemap(bfile *fp,char *ubuf,int ulen)
{
	BFILE_STAT	sb ;
	offset_t	baseoff, runoff ;
	int		rs = SR_OK ;
	int		mlen ;
	int		i ;
	int		pagemask = (fp->pagesize - 1) ;
	int		tlen = 0 ;
	int		f_partial = FALSE ;
	char		*dbp = ubuf ;

#if	CF_DEBUGS
	debugprintf("breadlinemap: ent ulen=%d\n",ulen) ;
#endif

	runoff = fp->offset ;
	while ((rs >= 0) && (tlen < ulen)) {

#if	CF_DEBUGS
	    debugprintf("breadlinemap: tlen=%d\n",tlen) ;
#endif

/* is there more data in the file and are we at a map page boundary? */

	    mlen = fp->size - runoff ;

#if	CF_DEBUGS
	    debugprintf("breadlinemap: more mlen=%d\n",mlen) ;
#endif

	    if ((mlen > 0) &&
	        ((fp->bp == NULL) || (fp->len == fp->pagesize))) {

#if	CF_DEBUGS
	        debugprintf("breadlinemap: check on page mapping\n") ;
#endif

	        i = (runoff / fp->pagesize) & (BFILE_NMAPS - 1) ;
	        baseoff = runoff & (~ pagemask) ;
	        if ((! fp->maps[i].f.valid) || (fp->maps[i].buf == NULL)
	            || (fp->maps[i].offset != baseoff)) {

#if	CF_DEBUGS
	            debugprintf("breadlinemap: mapping offset=%llu\n",
	                runoff) ;
#endif

	            bfile_pagein(fp,runoff,i) ;
	        }

	        fp->len = runoff & pagemask ;
	        fp->bp = fp->maps[i].buf + fp->len ;

	    } /* end if (initializing memory mapping) */

/* prepare to move data */

#if	CF_DEBUGS
	    debugprintf("breadlinemap: preparing to move data\n") ;
#endif

	    if ((fp->pagesize - fp->len) < mlen) {
	        mlen = (fp->pagesize - fp->len) ;
	    }

	    if ((ulen - tlen) < mlen) {
	        mlen = (ulen - tlen) ;
	    }

	    if (mlen > 0) {
	        char	*bp ;
	        char	*lastp ;

#if	CF_DEBUGS
	        debugprintf("breadlinemap: moving data\n") ;
#endif

#if	CF_MEMCCPY
	        if ((lastp = memccpy(dbp,fp->bp,'\n',mlen)) == NULL) {
	            lastp = dbp + mlen ;
	        }
	        i = lastp - dbp ;
	        dbp += i ;
	        fp->bp += i ;
#else
	        bp = fp->bp ;
	        lastp = fp->bp + mlen ;
	        while (bp < lastp) {
	            if (isoureol(*dbp++ = *bp++)) break ;
	        }
	        i = bp - fp->bp ;
	        fp->bp += i ;
#endif /* CF_MEMCCPY */

	        fp->len += i ;
	        runoff += i ;
	        tlen += i ;
	        if ((i > 0) && isoureol(dbp[-1])) break ;

	    } /* end if (move it) */

/* if we were file size limited */

	    if ((rs >= 0) && (runoff >= fp->size)) {

#if	CF_DEBUGS
	        debugprintf("breadlinemap: offset at end\n") ;
#endif

	        if (f_partial) break ;

#if	CF_DEBUGS
	        debugprintf("breadlinemap: re-statting\n") ;
#endif

	        rs = bfilefstat(fp->fd,&sb) ;

#if	CF_DEBUGS
	        debugprintf("breadlinemap: u_fstat() rs=%d\n", rs) ;
#endif

	        fp->size = sb.st_size ;
	        f_partial = TRUE ;
	    } /* end if (file size limited) */

#if	CF_DEBUGS
	    debugprintf("breadlinemap: bottom loop\n") ;
#endif

	} /* end while (reading) */

	if (rs >= 0) {
	    fp->offset += tlen ;
	}

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (breadlinemap) */


static int breadlinereg(bfile *fp,char *ubuf,int ulen,int to)
{
	RELOAD		s ;
	const int	opts = FM_TIMED ;
	int		rs = SR_OK ;
	int		mlen ;
	int		i ;
	int		tlen = 0 ;
	int		f_partial = FALSE ;
	char		*dbp = ubuf ;

#if	CF_DEBUGS
	debugprintf("breadlinereg: ent ulen=%u to=%d\n",ulen,to) ;
	debugprintf("breadlinereg: f_inpartline=%u\n",fp->f.inpartline) ;
#endif

	while ((rs >= 0) && (ulen > 0)) {

	    if (fp->len == 0) {
	        if (f_partial && fp->f.inpartline) break ;
	        rs = breload(fp,&s,to,opts) ;
	        if (rs <= 0) break ;
	        if (fp->len < fp->bsize) f_partial = TRUE ;
	    } /* end if (refilling up buffer) */

	    mlen = (fp->len < ulen) ? fp->len : ulen ;

#if	CF_DEBUGS
	    debugprintf("breadlinereg: mlen=%d\n",mlen) ;
#endif

	    if ((rs >= 0) && (mlen > 0)) {
	        char	*bp ;
	        char	*lastp ;

#if	CF_MEMCCPY
	        if ((lastp = memccpy(dbp,fp->bp,'\n',mlen)) == NULL) {
	            lastp = dbp + mlen ;
	        }
	        i = lastp - dbp ;
	        dbp += i ;
	        fp->bp += i ;
#else
	        bp = fp->bp ;
	        lastp = fp->bp + mlen ;
	        while (bp < lastp) {
	            if (isoureol(*dbp++ = *bp++)) break ;
	        } /* end while */
	        i = bp - fp->bp ;
	        fp->bp += i ;
#endif /* CF_MEMCCPY */

#if	CF_DEBUGS
	        {
	            debugprintf("breadlinereg: i=%d\n",i) ;
	            if (i > 0) {
	                debugprintf("breadlinereg: ieol=%u\n",
				isoureol(dbp[-1])) ;
		    }
	        }
#endif /* CF_DEBUGS */

	        fp->len -= i ;
	        tlen += i ;
	        if ((i > 0) && isoureol(dbp[-1])) break ;

	        ulen -= mlen ;

	    } /* end if (move it) */

	} /* end while (trying to satisfy request) */

	if (rs >= 0) {
	    fp->offset += tlen ;
	}

#if	CF_DEBUGS
	debugprintf("breadlinereg: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (breadlinereg) */


/* ARGSUSED */
static int breload(bfile *fp,RELOAD *sp,int to,int opts)
{
	int		rs = SR_OK ;
	int		maxeof ;
	int		neof = 0 ;
	int		len = 0 ;

#if	CF_DEBUGS
	debugprintf("breadline/breload: to=%d opts=\\x%04x\n",to,opts) ;
#endif
	maxeof = (fp->f.network && (to < 0)) ? BFILE_MAXNEOF : 1 ;
	while ((rs >= 0) && (len == 0) && (neof < maxeof)) {

	    if (to >= 0) {
	        rs = uc_reade(fp->fd,fp->bdata,fp->bsize,to,opts) ;
	        len = rs ;
	    } else {
	        rs = u_read(fp->fd,fp->bdata,fp->bsize) ;
	        len = rs ;
	    }

	    if (rs >= 0) {
	        neof = (len == 0) ? (neof+1) : 0 ;
	    }

	} /* end while */

	if (rs >= 0) {
	    fp->len = len ;
	    fp->bp = fp->bdata ;
	}

#if	CF_DEBUGS
	debugprintf("breadline/breload: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (breload) */


static int isoureol(int ch)
{
	return (ch == '\n') ;
}
/* end subroutine (isoureol) */


