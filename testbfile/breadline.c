/* breadline */

/* "Basic I/O" package similiar to "stdio" */
/* last modifed %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_MEMCCPY	0		/* we are faster than 'memccpy()'! */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


	= 1999-01-10, David A­D­ Morano

	I added the little extra code to allow for memory mapped I/O.
	It is all a waste because it is way slower than without it!
	This should teach me to leave old programs alone!!!


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads a single line from the buffer (or where
	ever) and returns that line to the caller.  Any remaining data
	is left for subsequent reads (of any kind).


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
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	BRELOAD		struct bfile_reload


/* external subroutines */

extern int	bfile_pagein(bfile *,offset_t,int) ;


/* external variables */


/* local structures */

struct bfile_reload {
	uint	f_already:1 ;
} ;


/* forward references */

static int	breadlinemap(bfile *,char *,int) ;
static int	breadlinereg(bfile *,char *,int,int) ;
static int	breload(bfile *,BRELOAD *,int,int) ;


/* local variables */


/* exported subroutines */


int breadlinetimed(fp,ubuf,ulen,to)
bfile	*fp ;
char	ubuf[] ;
int	ulen ;
int	to ;
{
	int	rs = SR_OK ;


	if (fp == NULL)
	    return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("breadlinetimed: fd=%u to=%d\n",fp->fd,to) ;
#endif

	if (fp->f.nullfile) goto ret0 ;

/* continue */

	if (fp->oflags & O_WRONLY)
	    return SR_WRONLY ;

	if (fp->f.write) {
	    fp->f.write = FALSE ;
	    if (fp->len > 0)
	        rs = bfile_flush(fp) ;
	} /* end if (switching from writing to reading) */

/* decide how we want to transfer the data */

	if (rs >= 0) {
	    if (fp->f.mapped) {
	        rs = breadlinemap(fp,ubuf,ulen) ;
	    } else {
	        rs = breadlinereg(fp,ubuf,ulen,to) ;
	    } /* end if (read map or not) */
	} /* end if */

ret0:
	return rs ;
}
/* end subroutine (breadlinetimed) */


int breadline(fp,ubuf,ulen)
bfile	*fp ;
char	ubuf[] ;
int	ulen ;
{


	return breadlinetimed(fp,ubuf,ulen,-1) ;
}
/* end subroutine (breadline) */


/* local subroutines */


static int breadlinemap(fp,ubuf,ulen)
bfile		*fp ;
char		ubuf[] ;
int		ulen ;
{
	BFILE_STAT	sb ;

	offset_t	baseoff, runoff ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	mlen ;
	int	i ;
	int	pagemask = (fp->pagesize - 1) ;
	int	tlen = 0 ;
	int	f_already = FALSE ;

	char	*dbp = ubuf ;


#if	CF_DEBUGS
	debugprintf("breadlinetimed: mapped ulen=%d\n",ulen) ;
#endif

	runoff = fp->offset ;
	while ((rs >= 0) && (tlen < ulen)) {

#if	CF_DEBUGS
	    debugprintf("breadlinetimed: tlen=%d\n",tlen) ;
#endif

/* is there more data in the file and are we at a map page boundary? */

	    mlen = fp->fsize - runoff ;

#if	CF_DEBUGS
	    debugprintf("breadlinetimed: more mlen=%d\n",mlen) ;
#endif

	    if ((mlen > 0) &&
	        ((fp->bp == NULL) || (fp->len == fp->pagesize))) {

#if	CF_DEBUGS
	        debugprintf("breadlinetimed: check on page mapping\n") ;
#endif

	        i = (runoff / fp->pagesize) & (BFILE_NMAPS - 1) ;
	        baseoff = runoff & (~ pagemask) ;
	        if ((! fp->maps[i].f.valid) || (fp->maps[i].buf == NULL)
	            || (fp->maps[i].offset != baseoff)) {

#if	CF_DEBUGS
	            debugprintf("breadlinetimed: mapping offset=%llu\n",
	                runoff) ;
#endif

	            bfile_pagein(fp,runoff,i) ;

	        }

	        fp->len = runoff & pagemask ;
	        fp->bp = fp->maps[i].buf + fp->len ;

	    } /* end if (initializing memory mapping) */

/* prepare to move data */

#if	CF_DEBUGS
	    debugprintf("breadlinetimed: preparing to move data\n") ;
#endif

	    if ((fp->pagesize - fp->len) < mlen)
	        mlen = (fp->pagesize - fp->len) ;

	    if ((ulen - tlen) < mlen)
	        mlen = (ulen - tlen) ;

	    if (mlen > 0) {
	        char	*bp ;
	        char	*lastp ;

#if	CF_DEBUGS
	        debugprintf("breadlinetimed: moving data\n") ;
#endif

#if	CF_MEMCCPY
	        if ((lastp = memccpy(dbp,fp->bp,'\n',mlen)) == NULL)
	            lastp = dbp + mlen ;

	        i = lastp - dbp ;
	        dbp += i ;
	        fp->bp += i ;
#else
	        bp = fp->bp ;
	        lastp = fp->bp + mlen ;
	        while (bp < lastp) {

	            if ((*dbp++ = *bp++) == '\n')
	                break ;

	        }

	        i = bp - fp->bp ;
	        fp->bp += i ;
#endif /* CF_MEMCCPY */

	        fp->len += i ;
	        runoff += i ;
	        tlen += i ;
	        if ((i > 0) && (dbp[-1] == '\n'))
	            break ;

	    } /* end if (move it) */

/* if we were file size limited */

	    if (runoff >= fp->fsize) {

#if	CF_DEBUGS
	        debugprintf("breadlinetimed: offset at end\n") ;
#endif

	        if (f_already)
	            break ;

#if	CF_DEBUGS
	        debugprintf("breadlinetimed: re-statting\n") ;
#endif

	        rs = bfilefstat(fp->fd,&sb) ;

#if	CF_DEBUGS
	        debugprintf("breadlinetimed: u_fstat() rs=%d\n",
	            rs) ;
#endif

	        if (rs < 0)
	            break ;

	        fp->fsize = sb.st_size ;
	        f_already = TRUE ;

	    } /* end if (file size limited) */

#if	CF_DEBUGS
	    debugprintf("breadlinetimed: bottom loop\n") ;
#endif

	} /* end while (reading) */

	if (rs >= 0)
	    fp->offset += tlen ;

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (breadlinemap) */


static int breadlinereg(fp,ubuf,ulen,to)
bfile		*fp ;
char		ubuf[] ;
int		ulen ;
int		to ;
{
	BRELOAD		s ;

	const int	opts = FM_TIMED ;

	int	rs = SR_OK ;
	int	mlen ;
	int	i ;
	int	pagemask = (fp->pagesize - 1) ;
	int	tlen = 0 ;
	int	f_already = FALSE ;

	char	*dbp = ubuf ;


#if	CF_DEBUGS
	debugprintf("breadlinetimed: normal unmapped ulen=%u to=%d\n",
	    ulen,to) ;
	debugprintf("breadlinetimed: f_linein=%u\n",fp->f.linein) ;
#endif

	memset(&s,0,sizeof(struct bfile_reload)) ;

	while (ulen > 0) {

	    if (fp->len <= 0) {
	        if (f_already && (! fp->f.linein)) break ;
	        rs = breload(fp,&s,to,opts) ;
#if	CF_DEBUGS
	        debugprintf("breadlinetimed: breload() rs=%d\n",rs) ;
#endif
	        if (rs <= 0) break ;
	        if (fp->len < fp->bsize) f_already = TRUE ;
	    } /* end if (refilling up buffer) */

	    mlen = (fp->len < ulen) ? fp->len : ulen ;

#if	CF_DEBUGS
	    debugprintf("breadlinetimed: mlen=%d\n",mlen) ;
#endif

	    if (mlen > 0) {
	        register char	*bp ;
	        register char	*lastp ;

#if	CF_MEMCCPY
	        if ((lastp = memccpy(dbp,fp->bp,'\n',mlen)) == NULL)
	            lastp = dbp + mlen ;

	        i = lastp - dbp ;
	        dbp += i ;
	        fp->bp += i ;
#else
	        bp = fp->bp ;
	        lastp = fp->bp + mlen ;
	        while (bp < lastp) {
	            if ((*dbp++ = *bp++) == '\n')
	                break ;
	        } /* end while */
	        i = bp - fp->bp ;
	        fp->bp += i ;
#endif /* CF_MEMCCPY */

#if	CF_DEBUGS
	        debugprintf("breadlinetimed: i=%d\n",i) ;
#endif

	        fp->len -= i ;
	        tlen += i ;
	        if ((i > 0) && (dbp[-1] == '\n'))
	            break ;

	        ulen -= mlen ;

	    } /* end if (move it) */

#if	CF_DEBUGS
	    debugprintf("breadlinetimed: bottom while\n") ;
#endif

	} /* end while (trying to satisfy request) */

	if (rs >= 0)
	    fp->offset += tlen ;

#if	CF_DEBUGS
	debugprintf("breadlinetimed: ret rs=%d tlen=%u\n",rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (breadlinereg) */


static int breload(fp,sp,to,opts)
bfile		*fp ;
BRELOAD		*sp ;
int		to ;
int		opts ;
{
	int	rs = SR_OK ;
	int	maxeof = (fp->f.network && (to < 0)) ? BFILE_MAXNEOF : 1 ;
	int	neof = 0 ;
	int	len = 0 ;

#if	CF_DEBUGS
	debugprintf("breadline/breload: to=%d opts=\\x%04x\n",to,opts) ;
#endif
	while ((rs >= 0) && (len == 0) && (neof < maxeof)) {

	    if (to >= 0) {
	        rs = uc_reade(fp->fd,fp->bdata,fp->bsize,to,opts) ;
	        len = rs ;
#if	CF_DEBUGS
	        debugprintf("breadline/breload: uc_reade() rs=%d\n",rs) ;
#endif
	    } else {
	        rs = u_read(fp->fd,fp->bdata,fp->bsize) ;
	        len = rs ;
#if	CF_DEBUGS
	        debugprintf("breadline/breload: u_read() rs=%d\n",rs) ;
#endif
	    }

	    if (rs >= 0) {
	        if (len == 0) {
	            neof += 1 ;
	        } else
	            neof = 0 ;
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



