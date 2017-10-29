/* bprintf */

/* this is a home made "printf" routine */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* debug print-outs */
#define	CF_DEBUGFLUSH	0		/* ? */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	Originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This version of printf is compatible with the Version 7 C printf.
	This function is implemented differently in that the function
	that does the actual formatting is 'bufprintf(3dam)'.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<ctype.h>
#include	<stdarg.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#undef	FD_WRITE
#define	FD_WRITE	4

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	bfile_flush(bfile *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	vbufprintf(char *,int,const char *,va_list) ;


/* external variables */


/* local structures */


/* forward references */

static int	bwriteline(bfile *,const char *,int) ;


/* exported subroutines */


int bprintf(bfile *fp,const char *fmt,...)
{
	int		llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		fmtlen ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (fp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (fp->f.nullfile) goto ret0 ;

/* continue */

	{
	va_list	ap ;
	va_begin(ap,fmt) ;
	rs = vbufprintf(lbuf,llen,fmt,ap) ;
	fmtlen = rs ;
	va_end(ap) ;
	}

#if	CF_DEBUGS
	debugprintf("bprintf: vbufprintf() rs=%d\n",rs) ;
#endif

	if ((rs >= 0) && (fmtlen > 0)) {
	    rs = bwriteline(fp,lbuf,fmtlen) ;
	    wlen = rs ;
	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("bprintf: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bprintf) */


/* 'vprintf'-like routine */
int bvprintf(fp,fmt,ap)
bfile		*fp ;
const char	fmt[] ;
va_list		ap ;
{
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		fmtlen ;
	int		wlen = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

	if (fp == NULL) return SR_FAULT ;
	if (fmt == NULL) return SR_FAULT ;

	if (fp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (fp->f.nullfile) goto ret0 ;

	rs = vbufprintf(lbuf,llen,fmt,ap) ;
	fmtlen = rs ;
	if ((rs >= 0) && (fmtlen > 0)) {
	    rs = bwriteline(fp,lbuf,fmtlen) ;
	    wlen = rs ;
	}

ret0:
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bvprintf) */


/* local subroutines */


static int bwriteline(fp,lbuf,llen)
bfile		*fp ;
const char	lbuf[] ;
int		llen ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;

	if (llen > 0) {

#if	CF_DEBUGS
	debugprintf("bprintf/bwriteline: llen=%u line=>%t<\n",
		llen,lbuf,strlinelen(lbuf,llen,40)) ;
#endif

	    rs = bwrite(fp,lbuf,llen) ;
	    wlen = rs ;
	    if ((rs >= 0) && (wlen > 0) && (lbuf[wlen-1] == '\n')) {
		int	f = FALSE ;
		f = f || (fp->bm == bfile_bmnone) ;
	        f = f || (fp->bm == bfile_bmline) ;
		if (f)
	            rs = bfile_flush(fp) ;
	    }
	}

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (bwriteline) */


