/* cfdecmf */

/* 'C'onvert 'F'rom a 'DEC'imal with a 'M'ultiply 'F'factor */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These are subroutines to convert decimal strings to binary integer
	value but with optional multiplying factors (letters) attached to the
	end of the given string.  The value represented by the string (without
	the multiplying factor) is multiplied by the multiplying factor.

	multiply-factor		value
	-------------------------------

	b			512
	k			1024
	m			(1024*1024)
	g			(1024*1024*1024)
	kb			(1024*512)
	mb			(1024*1024*512)


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#undef	UBLOCKSIZE
#define	UBLOCKSIZE	512

#define	OUR_ISWHITE(c)	(CHAR_ISWHITE(c) || ((c) == CH_NBSP))
#define	TOLC(ch)	CHAR_TOLC(ch)


/* external subroutines */

extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecui(cchar *,int,uint *) ;
extern int	cfdecl(cchar *,int,long *) ;
extern int	cfdecll(cchar *,int,LONG *) ;
extern int	cfdecul(cchar *,int,ulong *) ;
extern int	cfdecull(cchar *,int,ULONG *) ;


/* external variables */


/* local structures */


/* forward references */

static int	getmf(cchar *,int,ulong *) ;


/* local variables */


/* exported subroutines */


int cfdecmfi(cchar *sbuf,int slen,int *rp)
{
	ulong		mf ;
	int		sl ;
	int		rs ;

	sl = getmf(sbuf,slen,&mf) ;

	if ((rs = cfdeci(sbuf,sl,rp)) >= 0) {
	    *rp = (*rp * mf) ;
	}

	return rs ;
}
/* end subroutine (cfdecmfi) */


int cfdecmfui(cchar *sbuf,int slen,uint *rp)
{
	ulong		mf ;
	int		rs ;
	int		sl ;

	sl = getmf(sbuf,slen,&mf) ;

	if ((rs = cfdecui(sbuf,sl,rp)) >= 0) {
	    *rp = (*rp * mf) ;
	}

	return rs ;
}
/* end subroutine (cfdecmfui) */


int cfdecmfl(cchar *sbuf,int slen,long *rp)
{
	ulong		mf ;
	int		rs ;
	int		sl ;

	sl = getmf(sbuf,slen,&mf) ;

	if ((rs = cfdecl(sbuf,sl,rp)) >= 0) {
	    *rp = (*rp * mf) ;
	}

	return rs ;
}
/* end subroutine (cfdecmfl) */


int cfdecmful(cchar *sbuf,int slen,ulong *rp)
{
	ulong		mf ;
	int		rs ;
	int		sl ;

	sl = getmf(sbuf,slen,&mf) ;

	if ((rs = cfdecul(sbuf,sl,rp)) >= 0) {
	    *rp = (*rp * mf) ;
	}

	return rs ;
}
/* end subroutine (cfdecmful) */


/* to LONG integer */
int cfdecmfll(cchar *sbuf,int slen,LONG *rp)
{
	ulong		mf ;
	int		rs ;
	int		sl ;

	sl = getmf(sbuf,slen,&mf) ;

	if ((rs = cfdecll(sbuf,sl,rp)) >= 0) {
	    *rp = (*rp * mf) ;
	}

	return rs ;
}
/* end subroutine (cfdecmfll) */


int cfdecmfull(cchar *sbuf,int slen,ULONG *rp)
{
	ulong		mf ;
	int		rs ;
	int		sl ;

	sl = getmf(sbuf,slen,&mf) ;

	if ((rs = cfdecull(sbuf,sl,rp)) >= 0) {
	    *rp = (*rp * mf) ;
	}

	return rs ;
}
/* end subroutine (cfdecmfull) */


/* local subroutines */


static int getmf(cchar *sbuf,int slen,ulong *rp)
{
	ulong		mf ;
	int		sl = strnlen(sbuf,slen) ;
	uchar		*ubuf = (uchar *) sbuf ;

	while ((sl > 0) && OUR_ISWHITE(ubuf[sl - 1])) {
	    sl -= 1 ;
	}

	mf = 1 ;
	if (sl > 0) {

	    if (TOLC(ubuf[sl-1]) == 'b') {
	        mf = UBLOCKSIZE ;

	    } else if (TOLC(ubuf[sl-1]) == 'k') {
	        mf = 1024 ;

	    } else if (TOLC(ubuf[sl-1]) == 'm') {
	        mf = 1024 * 1024 ;

	    } else if (TOLC(ubuf[sl-1]) == 'g') {
	        mf = 1024 * 1024 * 1024 ;

	    } else if (sl > 1) {

	        if ((TOLC(ubuf[sl-2]) == 'k') && (ubuf[sl-1] == 'b')) {

	            mf = 1024 * UBLOCKSIZE ;
	            sl -= 1 ;

	        } else if ((TOLC(ubuf[sl-2]) == 'm') && (ubuf[sl-1] == 'b')) {

	            mf = 1024 * 1024 * UBLOCKSIZE ;
	            sl -= 1 ;

		} /* end if */

	    } /* end if */

	    if (mf > 1) {

	        sl -= 1 ;
		while ((sl > 0) && OUR_ISWHITE(ubuf[sl-1])) {
	    	    sl -= 1 ;
		}

	    } /* end if (adjusting buffer length) */

	} /* end if */

	*rp = mf ;
	return sl ;
}
/* end subroutine (getmf) */


