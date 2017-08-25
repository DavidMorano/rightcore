/* netorder */

/* network byte order manipulations */


/* revision history:

	= 2001-03-24, David A­D­ Morano
        This code was written from scratch to get some portable network ordering
        for Ethernet (ETHCON) development work.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These routines are used to manipulate data that may be in the portable
        "network ordering." This ordering is used on network-type transfers. We
        will be implementing the IP protocol stack (UDP but maybe not TCP) on
        the ETHCON (TN2060) circuit pack computer.

	Subroutines are provided to read and write host native:

	+ shorts
	+ ints
	+ LONGs (64-bit)

        out of or into a character buffer. The buffer bytes are what are usually
        read or written to the network.


*******************************************************************************/


#define	NETORDER_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<localmisc.h>


/* exported subroutines */


int netorder_rchar(char *buf,char *cwp)
{
	const int	rs = sizeof(char) ;
	uchar		*ubuf = (uchar *) buf ;

	*cwp = ubuf[0] ;
	return rs ;
}
/* end subroutine (netorder_rchar) */


int netorder_rshort(char *buf,short *swp)
{
	const int	rs = sizeof(short) ;
	uchar		*ubuf = (uchar *) buf ;

	*swp = 0 ;
	*swp |= (ubuf[0] << 8) ;
	*swp |= (ubuf[1] << 0) ;

	return rs ;
}
/* end subroutine (netorder_rshort) */


int netorder_rint(char *buf,int *iwp)
{
	const int	rs = sizeof(int) ;
	uchar		*ubuf = (uchar *) buf ;

	*iwp = 0 ;
	*iwp |= (ubuf[0] << 24) ;
	*iwp |= (ubuf[1] << 16) ;
	*iwp |= (ubuf[2] << 8) ;
	*iwp |= (ubuf[3] << 0) ;

	return rs ;
}
/* end subroutine (netorder_rint) */


#if	(LONG_BIT == 64)

int netorder_rlong(char *buf,long *lwp)
{
	const int	rs = sizeof(long) ;
	uchar		*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((long) ubuf[0]) << 56) ;
	*lwp |= (((long) ubuf[1]) << 48) ;
	*lwp |= (((long) ubuf[2]) << 40) ;
	*lwp |= (((long) ubuf[3]) << 32) ;
	*lwp |= (((long) ubuf[4]) << 24) ;
	*lwp |= (((long) ubuf[5]) << 16) ;
	*lwp |= (((long) ubuf[6]) << 8) ;
	*lwp |= (((long) ubuf[7]) << 0) ;

	return rs ;
}
/* end subroutine (netorder_rlong) */

#else /* (LONG_BIT == 64) */

int netorder_rlong(char *buf,long *lwp)
{
	const int	rs = sizeof(long) ;
	uchar		*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((long) ubuf[0]) << 24) ;
	*lwp |= (((long) ubuf[1]) << 16) ;
	*lwp |= (((long) ubuf[2]) << 8) ;
	*lwp |= (((long) ubuf[3]) << 0) ;

	return rs ;
}
/* end subroutine (netorder_rlong) */

#endif /* (LONG_BIT == 64) */


int netorder_rll(char *buf,LONG *lwp)
{
	const int	rs = sizeof(LONG) ;
	uchar		*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((LONG) ubuf[0]) << 56) ;
	*lwp |= (((LONG) ubuf[1]) << 48) ;
	*lwp |= (((LONG) ubuf[2]) << 40) ;
	*lwp |= (((LONG) ubuf[3]) << 32) ;
	*lwp |= (((LONG) ubuf[4]) << 24) ;
	*lwp |= (((LONG) ubuf[5]) << 16) ;
	*lwp |= (((LONG) ubuf[6]) << 8) ;
	*lwp |= (((LONG) ubuf[7]) << 0) ;

	return rs ;
}
/* end subroutine (netorder_rll) */


int netorder_ruchar(char *buf,uchar *cwp)
{
	const int	rs = sizeof(uchar) ;
	uchar		*ubuf = (uchar *) buf ;

	*cwp = ubuf[0] ;
	return rs ;
}
/* end subroutine (netorder_ruchar) */


int netorder_rushort(char *buf,ushort *swp)
{
	const int	rs = sizeof(ushort) ;
	uchar		*ubuf = (uchar *) buf ;

	*swp = 0 ;
	*swp |= (ubuf[0] << 8) ;
	*swp |= (ubuf[1] << 0) ;

	return rs ;
}
/* end subroutine (netorder_rushort) */


int netorder_ruint(char *buf,uint *iwp)
{
	const int	rs = sizeof(uint) ;
	uchar		*ubuf = (uchar *) buf ;

	*iwp = 0 ;
	*iwp |= (ubuf[0] << 24) ;
	*iwp |= (ubuf[1] << 16) ;
	*iwp |= (ubuf[2] << 8) ;
	*iwp |= (ubuf[3] << 0) ;

	return rs ;
}
/* end subroutine (netorder_ruint) */


#if	(LONG_BIT == 64)

int netorder_rulong(char *buf,ulong *lwp)
{
	const int	rs = sizeof(ulong) ;
	uchar		*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((ulong) ubuf[0]) << 56) ;
	*lwp |= (((ulong) ubuf[1]) << 48) ;
	*lwp |= (((ulong) ubuf[2]) << 40) ;
	*lwp |= (((ulong) ubuf[3]) << 32) ;
	*lwp |= (((ulong) ubuf[4]) << 24) ;
	*lwp |= (((ulong) ubuf[5]) << 16) ;
	*lwp |= (((ulong) ubuf[6]) << 8) ;
	*lwp |= (((ulong) ubuf[7]) << 0) ;

	return rs ;
}
/* end subroutine (netorder_rulong) */

#else /* (LONG_BIT == 64) */

int netorder_rulong(char *buf,ulong *lwp)
{
	const int	rs = sizeof(ulong) ;
	uchar		*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((ulong) ubuf[0]) << 24) ;
	*lwp |= (((ulong) ubuf[1]) << 16) ;
	*lwp |= (((ulong) ubuf[2]) << 8) ;
	*lwp |= (((ulong) ubuf[3]) << 0) ;

	return rs ;
}
/* end subroutine (netorder_rulong) */

#endif /* (LONG_BIT == 64) */


int netorder_rull(char *buf,ULONG *lwp)
{
	const int	rs = sizeof(ULONG) ;
	uchar		*ubuf = (uchar *) buf ;

	*lwp = 0 ;
	*lwp |= (((ULONG) ubuf[0]) << 56) ;
	*lwp |= (((ULONG) ubuf[1]) << 48) ;
	*lwp |= (((ULONG) ubuf[2]) << 40) ;
	*lwp |= (((ULONG) ubuf[3]) << 32) ;
	*lwp |= (((ULONG) ubuf[4]) << 24) ;
	*lwp |= (((ULONG) ubuf[5]) << 16) ;
	*lwp |= (((ULONG) ubuf[6]) << 8) ;
	*lwp |= (((ULONG) ubuf[7]) << 0) ;

	return rs ;
}
/* end subroutine (netorder_rull) */


/* write char */
int netorder_wchar(char *buf,int cw)
{
	const int	rs = sizeof(char) ;
	uchar		*ubuf = (uchar *) buf ;

	ubuf[0] = (uchar) cw ;
	return rs ;
}
/* end subroutine (netorder_wchar) */


/* write short */
int netorder_wshort(char *buf,int sw)
{
	const int	rs = sizeof(short) ;
	uchar		*ubuf = (uchar *) buf ;

	ubuf[0] = (uchar) ((sw >> 8) & 0xff) ;
	ubuf[1] = (uchar) ((sw >> 0) & 0xff) ;

	return rs ;
}
/* end subroutine (netorder_wshort) */


/* write int */
int netorder_wint(char *buf,int iw)
{
	const int	rs = sizeof(int) ;
	uchar		*ubuf = (uchar *) buf ;

	ubuf[0] = (uchar) ((iw >> 24) & 0xff) ;
	ubuf[1] = (uchar) ((iw >> 16) & 0xff) ;
	ubuf[2] = (uchar) ((iw >> 8) & 0xff) ;
	ubuf[3] = (uchar) ((iw >> 0) & 0xff) ;

	return rs ;
}
/* end subroutine (netorder_wint) */


#if	(LONG_BIT == 64)

/* write long */
int netorder_wlong(char *buf,long lw)
{
	const int	rs = sizeof(long) ;
	uchar		*ubuf = (uchar *) buf ;

	ubuf[0] = ((lw >> 56) & 0xff) ;
	ubuf[1] = ((lw >> 48) & 0xff) ;
	ubuf[2] = ((lw >> 40) & 0xff) ;
	ubuf[3] = ((lw >> 32) & 0xff) ;

	ubuf[4] = ((lw >> 24) & 0xff) ;
	ubuf[5] = ((lw >> 16) & 0xff) ;
	ubuf[6] = ((lw >> 8) & 0xff) ;
	ubuf[7] = ((lw >> 0) & 0xff) ;

	return rs ;
}
/* end subroutine (netorder_wlong) */

#else

/* write long */
int netorder_wlong(char *buf,long lw)
{
	const int	rs = sizeof(long) ;
	uchar		*ubuf = (uchar *) buf ;

	ubuf[0] = ((lw >> 24) & 0xff) ;
	ubuf[1] = ((lw >> 16) & 0xff) ;
	ubuf[2] = ((lw >> 8) & 0xff) ;
	ubuf[3] = ((lw >> 0) & 0xff) ;

	return rs ;
}
/* end subroutine (netorder_wlong) */

#endif /* (LONG_BIT == 64) */


/* write LONG */
int netorder_wll(char *buf,LONG lw)
{
	const int	rs = sizeof(LONG) ;
	uchar		*ubuf = (uchar *) buf ;

	ubuf[0] = ((lw >> 56) & 0xff) ;
	ubuf[1] = ((lw >> 48) & 0xff) ;
	ubuf[2] = ((lw >> 40) & 0xff) ;
	ubuf[3] = ((lw >> 32) & 0xff) ;

	ubuf[4] = ((lw >> 24) & 0xff) ;
	ubuf[5] = ((lw >> 16) & 0xff) ;
	ubuf[6] = ((lw >> 8) & 0xff) ;
	ubuf[7] = ((lw >> 0) & 0xff) ;

	return rs ;
}
/* end subroutine (netorder_wll) */


/* older API */


#ifdef	COMMENT

int netorder_readl(char *buf,ULONG *ulwp)
{
	LONG	*lwp = (LONG *) ulwp ;

	return netorder_rlong(buf,lwp) ;
}


int netorder_writel(char *buf,ULONG ulw)
{

	return netorder_wlong(buf,ulw) ;
}


int netorder_readi(char *buf,uint *uiwp)
char	*buf ;
{
	int	*iwp = (int *) uiwp ;

	return netorder_rint(buf,iwp) ;
}


int netorder_writei(char *buf,uint uiw)
{

	return netorder_wint(buf,uiw) ;
}


int netorder_reads(char *buf,ushort *uswp)
{
	short	*swp = (short *) uswp ;

	return netorder_rshort(buf,swp) ;
}


int netorder_writes(char *buf,uint usw)
{

	return netorder_wshort(buf,usw) ;
}

#endif /* COMMENT */


