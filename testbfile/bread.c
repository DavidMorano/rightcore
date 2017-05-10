/* bread */

/* "Basic I/O" package similiar to some other thing whose initials is "stdio" */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_MEMCPY	1		/* use 'memcpy(3c)' */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


	= 1999-01-10, David A­D­ Morano

	Wow, I finally got around to adding memory mapping to this thing!
	Other subroutines of mine have been using memory mapped I/O for
	years but this is one of those routines where it should have been
	applied a long time ago because of its big performance benefits!

	It is all a waste because it is way slower than without it!
	This should teach me to leave old programs alone!


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We do the reading.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#if	BFILE_LARGEFILE
#define	BSTAT		struct stat64
#else
#define	BSTAT		struct ustat
#endif


/* external subroutines */

extern int	bfile_pagein(bfile *,offset_t,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* forward references */

static int bfile_readmapped(bfile *,void *,int,int,int) ;
static int bfile_readreg(bfile *,void *,int,int,int) ;


/* local variables */


/* exported subroutines */


int breade(fp,ubuf,ulen,to,opts)
bfile		*fp ;
void		*ubuf ;
int		ulen ;
int		to ;
int		opts ;
{
	int	rs = SR_OK ;


#if	CF_DEBUGS
	debugprintf("bread: entered ulen=%d\n",ulen) ;
	debugprintf("bread: to=%d opts=%04x\n",to,opts) ;
	debugprintf("bread: FM_TIMED=%u\n",
		((opts & FM_TIMED) ? 1 : 0)) ;
#endif

	if (fp == NULL) return SR_FAULT ;
	if (ubuf == NULL) return SR_FAULT ;
	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;
	if (fp->oflags & O_WRONLY) return SR_WRONLY ;

	if (! fp->f.nullfile) {
	    if (rs >= 0) {
	        if (fp->f.mapped) {
	            rs = bfile_readmapped(fp,ubuf,ulen,to,opts) ;
	        } else
	            rs = bfile_readreg(fp,ubuf,ulen,to,opts) ;
	    } /* end if */
	} /* end if (not nullfile) */

#if	CF_DEBUGS
	debugprintf("bread: ret rs=%d \n",rs) ;
#endif

	return rs ;
}
/* end routine (breade) */


int bread(fp,ubuf,ulen)
bfile		*fp ;
void		*ubuf ;
int		ulen ;
{


	return breade(fp,ubuf,ulen,-1,0) ;
}
/* end routine (bread) */


/* local subroutines */


static int bfile_readmapped(fp,ubuf,ulen,to,opts)
bfile		*fp ;
void		*ubuf ;
int		ulen ;
int		to ;
int		opts ;
{
	BSTAT	sb ;

	int	rs ;
	int	tlen = 0 ;
	int	pagemask = fp->pagesize - 1 ;
	int	i, mlen ;
	int	f_already ;


#if	CF_DEBUGS
	debugprintf("bread: mapped\n") ;
#endif

	f_already = FALSE ;
	while (tlen < ulen) {

/* is there more data in the file and are we at a map page boundary? */

	    mlen = fp->fsize - fp->foff ;
	    if ((mlen > 0) &&
	        ((fp->bp == NULL) || (fp->len == fp->pagesize))) {

	        i = (fp->foff / fp->pagesize) & (BFILE_NMAPS - 1) ;
	        if ((! fp->maps[i].f.valid) || (fp->maps[i].buf == NULL)
	            || (fp->maps[i].offset != (fp->foff & (~ pagemask))))
	            bfile_pagein(fp,fp->foff,i) ;

	        fp->len = fp->foff & pagemask ;
	        fp->bp = fp->maps[i].buf + fp->len ;

	    } /* end if (initializing memory mapping) */

/* prepare to move data */

	    if ((fp->pagesize - fp->len) < mlen)
	        mlen = (fp->pagesize - fp->len) ;

	    if ((ulen - tlen) < mlen)
	        mlen = (ulen - tlen) ;

	    if (mlen > 0) {

	        memcpy((ubuf + tlen),fp->bp,mlen) ;

	        fp->bp += mlen ;
	        fp->len += mlen ;
	        fp->foff += mlen ;
	        tlen += mlen ;

	    } /* end if (move it) */

/* if we were file-size limited */

	    if (fp->foff >= fp->fsize) {
	        if (f_already) break ;

	        rs = u_fstat(fp->fd,&sb) ;
	        if (rs < 0) break ;

	        fp->fsize = sb.st_size ;
	        f_already = TRUE ;

	    } /* end if (file size limited) */

	} /* end while (reading) */

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (bfile_readmapped) */


static int bfile_readreg(fp,ubuf,ulen,to,opts)
bfile		*fp ;
void		*ubuf ;
int		ulen ;
int		to ;
int		opts ;
{
	offset_t	fo = fp->foff ;
	int	rs = SR_OK ;
	int	bl = 0 ;
	int	blen = ulen ;
	char	*bp = (char *) ubuf ;

	while ((rs >= 0) && (bl < ulen)) {
	    rs = bfile_bufread(fp,fo,(bp+bl),(blen-bl),to,opts) ;
	    if (rs == 0) break ;
	    bl += rs ;
	    fo += rs ;
	} /* end while */

	if (rs >= 0) fp->foff = fo ;

	return (rs >= 0) ? bl : rs ;
}
/* end subroutine (bfile_readreg) */


