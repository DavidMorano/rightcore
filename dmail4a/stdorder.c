/* stdorder */

/* network byte order manipulations */


/* revision history:

	= 2001-03-24, David A­D­ Morano
        This code was written from scratch to get some portable network ordering
        for Ethernet (ETHCON) development work.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines are used to manipulate data that may be in the portable
	"network ordering."  This ordering is used on network-type transfers.

	Subroutines are provided to read and write host native:

	+ shorts
	+ ints
	+ LONGs (64-bit)

	out of or into a character buffer.  The buffer bytes are what are
	usually read or written to the network.


*******************************************************************************/


#define	STDORDER_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<localmisc.h>


/* local defines */


/* exported subroutines */


int stdorder_rchar(char *buf,char *cwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*cwp = ubuf[0] ;
	return 1 ;
}
/* end subroutine (stdorder_rchar) */


int stdorder_rshort(char *buf,ushort *swp)
{
	uchar	*ubuf = (uchar *) buf ;

	*swp = 0 ;
	*swp |= (ubuf[0] << 0) ;
	*swp |= (ubuf[1] << 8) ;

	return 2 ;
}
/* end subroutine (stdorder_rshort) */


int stdorder_rint(char *buf,int *iwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*iwp = 0 ;
	*iwp |= (ubuf[0] << 0) ;
	*iwp |= (ubuf[1] << 8) ;
	*iwp |= (ubuf[2] << 16) ;
	*iwp |= (ubuf[3] << 24) ;

	return 4 ;
}
/* end subroutine (stdorder_rint) */


#if	(LONG_BIT == 64)

int stdorder_rlong(char *buf,long *lwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((long) ubuf[0]) << 0) ;
	*lwp |= (((long) ubuf[1]) << 8) ;
	*lwp |= (((long) ubuf[2]) << 16) ;
	*lwp |= (((long) ubuf[3]) << 24) ;
	*lwp |= (((long) ubuf[4]) << 32) ;
	*lwp |= (((long) ubuf[5]) << 40) ;
	*lwp |= (((long) ubuf[6]) << 48) ;
	*lwp |= (((long) ubuf[7]) << 56) ;

	return 8 ;
}
/* end subroutine (stdorder_rlong) */

#else /* (LONG_BIT == 64) */

int stdorder_rlong(char *buf,long *lwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((long) ubuf[0]) << 0) ;
	*lwp |= (((long) ubuf[1]) << 8) ;
	*lwp |= (((long) ubuf[2]) << 16) ;
	*lwp |= (((long) ubuf[3]) << 24) ;

	return 8 ;
}
/* end subroutine (stdorder_rlong) */

#endif /* (LONG_BIT == 64) */


int stdorder_rll(char *buf,LONG *lwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((LONG) ubuf[0]) << 0) ;
	*lwp |= (((LONG) ubuf[1]) << 8) ;
	*lwp |= (((LONG) ubuf[2]) << 16) ;
	*lwp |= (((LONG) ubuf[3]) << 24) ;
	*lwp |= (((LONG) ubuf[4]) << 32) ;
	*lwp |= (((LONG) ubuf[5]) << 40) ;
	*lwp |= (((LONG) ubuf[6]) << 48) ;
	*lwp |= (((LONG) ubuf[7]) << 57) ;

	return 8 ;
}
/* end subroutine (stdorder_rll) */


int stdorder_ruchar(char *buf,uchar *cwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*cwp = ubuf[0] ;
	return 1 ;
}
/* end subroutine (stdorder_ruchar) */


int stdorder_rushort(char *buf,ushort *swp)
{
	uchar	*ubuf = (uchar *) buf ;

	*swp = 0 ;
	*swp |= (ubuf[0] << 0) ;
	*swp |= (ubuf[1] << 8) ;

	return 2 ;
}
/* end subroutine (stdorder_rushort) */


int stdorder_ruint(char *buf,uint *iwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*iwp = 0 ;
	*iwp |= (ubuf[0] << 0) ;
	*iwp |= (ubuf[1] << 8) ;
	*iwp |= (ubuf[2] << 16) ;
	*iwp |= (ubuf[3] << 24) ;

	return 4 ;
}
/* end subroutine (stdorder_ruint) */


#if	(LONG_BIT == 64)

int stdorder_rulong(char *buf,ulong *lwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((ulong) ubuf[0]) << 0) ;
	*lwp |= (((ulong) ubuf[1]) << 8) ;
	*lwp |= (((ulong) ubuf[2]) << 16) ;
	*lwp |= (((ulong) ubuf[3]) << 24) ;
	*lwp |= (((ulong) ubuf[4]) << 32) ;
	*lwp |= (((ulong) ubuf[5]) << 40) ;
	*lwp |= (((ulong) ubuf[6]) << 48) ;
	*lwp |= (((ulong) ubuf[7]) << 56) ;

	return 8 ;
}
/* end subroutine (stdorder_rulong) */

#else /* (LONG_BIT == 64) */

int stdorder_rulong(char *buf,ulong *lwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((ulong) ubuf[0]) << 0) ;
	*lwp |= (((ulong) ubuf[1]) << 8) ;
	*lwp |= (((ulong) ubuf[2]) << 16) ;
	*lwp |= (((ulong) ubuf[3]) << 24) ;

	return 4 ;
}
/* end subroutine (stdorder_rulong) */

#endif /* (LONG_BIT == 64) */


int stdorder_rull(char *buf,ULONG *lwp)
{
	uchar	*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((ULONG) ubuf[0]) << 0) ;
	*lwp |= (((ULONG) ubuf[1]) << 8) ;
	*lwp |= (((ULONG) ubuf[2]) << 16) ;
	*lwp |= (((ULONG) ubuf[3]) << 24) ;
	*lwp |= (((ULONG) ubuf[4]) << 32) ;
	*lwp |= (((ULONG) ubuf[5]) << 40) ;
	*lwp |= (((ULONG) ubuf[6]) << 48) ;
	*lwp |= (((ULONG) ubuf[7]) << 56) ;

	return 8 ;
}
/* end subroutine (stdorder_rull) */


int stdorder_wchar(char *buf,int cw)
{
	uchar	*ubuf = (uchar *) buf ;

	ubuf[0] = (uchar) cw ;
	return 1 ;
}
/* end subroutine (stdorder_wchar) */


int stdorder_wshort(char *buf,int sw)
{
	uchar	*ubuf = (uchar *) buf ;

	ubuf[0] = (uchar) ((sw >> 0) & 0xff) ;
	ubuf[1] = (uchar) ((sw >> 8) & 0xff) ;

	return 2 ;
}
/* end subroutine (stdorder_wshort) */


int stdorder_wint(char *buf,int iw)
{
	uchar	*ubuf = (uchar *) buf ;

	ubuf[0] = (uchar) ((iw >> 0) & 0xff) ;
	ubuf[1] = (uchar) ((iw >> 8) & 0xff) ;
	ubuf[2] = (uchar) ((iw >> 16) & 0xff) ;
	ubuf[3] = (uchar) ((iw >> 24) & 0xff) ;

	return 4 ;
}
/* end subroutine (stdorder_wint) */


#if	(LONG_BIT == 64)

int stdorder_wlong(char *buf,long lw)
{
	uchar	*ubuf = (uchar *) buf ;

	ubuf[0] = ((lw >> 0) & 0xff) ;
	ubuf[1] = ((lw >> 8) & 0xff) ;
	ubuf[2] = ((lw >> 16) & 0xff) ;
	ubuf[3] = ((lw >> 24) & 0xff) ;

	ubuf[4] = ((lw >> 32) & 0xff) ;
	ubuf[5] = ((lw >> 40) & 0xff) ;
	ubuf[6] = ((lw >> 48) & 0xff) ;
	ubuf[7] = ((lw >> 56) & 0xff) ;

	return 8 ;
}
/* end subroutine (stdorder_wlong) */

#else /* (LONG_BIT == 64) */

int stdorder_wlong(char *buf,long lw)
{
	uchar	*ubuf = (uchar *) buf ;

	ubuf[0] = ((lw >> 0) & 0xff) ;
	ubuf[1] = ((lw >> 8) & 0xff) ;
	ubuf[2] = ((lw >> 16) & 0xff) ;
	ubuf[3] = ((lw >> 24) & 0xff) ;

	return 4 ;
}
/* end subroutine (stdorder_wlong) */

#endif /* (LONG_BIT == 64) */


int stdorder_wll(char *buf,LONG lw)
{
	uchar	*ubuf = (uchar *) buf ;

	ubuf[0] = ((lw >> 0) & 0xff) ;
	ubuf[1] = ((lw >> 8) & 0xff) ;
	ubuf[2] = ((lw >> 16) & 0xff) ;
	ubuf[3] = ((lw >> 24) & 0xff) ;

	ubuf[4] = ((lw >> 32) & 0xff) ;
	ubuf[5] = ((lw >> 40) & 0xff) ;
	ubuf[6] = ((lw >> 48) & 0xff) ;
	ubuf[7] = ((lw >> 56) & 0xff) ;

	return 8 ;
}
/* end subroutine (stdorder_wll) */


