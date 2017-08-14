/* netorder */

/* network byte order manipulations */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This code was written from scratch to get some portable network ordering
        for Ethernet (ETHCON) development work.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

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


int netorder_rchar(buf,cwp)
char	*buf ;
char	*cwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*cwp = ubuf[0] ;
	return 1 ;
}
/* end subroutine (netorder_rchar) */


int netorder_rshort(buf,swp)
char	*buf ;
short	*swp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*swp = 0 ;
	*swp |= (ubuf[0] << 8) ;
	*swp |= (ubuf[1] << 0) ;

	return 2 ;
}
/* end subroutine (netorder_rshort) */


int netorder_rint(buf,iwp)
char	*buf ;
int	*iwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*iwp = 0 ;
	*iwp |= (ubuf[0] << 24) ;
	*iwp |= (ubuf[1] << 16) ;
	*iwp |= (ubuf[2] << 8) ;
	*iwp |= (ubuf[3] << 0) ;

	return 4 ;
}
/* end subroutine (netorder_rint) */


#if	(LONG_BIT == 64)

int netorder_rlong(buf,lwp)
char	*buf ;
long	*lwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*lwp = 0 ;
	*lwp |= (((long) ubuf[0]) << 56) ;
	*lwp |= (((long) ubuf[1]) << 48) ;
	*lwp |= (((long) ubuf[2]) << 40) ;
	*lwp |= (((long) ubuf[3]) << 32) ;
	*lwp |= (((long) ubuf[4]) << 24) ;
	*lwp |= (((long) ubuf[5]) << 16) ;
	*lwp |= (((long) ubuf[6]) << 8) ;
	*lwp |= (((long) ubuf[7]) << 0) ;

	return 8 ;
}
/* end subroutine (netorder_rlong) */

#else /* (LONG_BIT == 64) */

int netorder_rlong(buf,lwp)
char	*buf ;
long	*lwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*lwp = 0 ;
	*lwp |= (((long) ubuf[0]) << 24) ;
	*lwp |= (((long) ubuf[1]) << 16) ;
	*lwp |= (((long) ubuf[2]) << 8) ;
	*lwp |= (((long) ubuf[3]) << 0) ;

	return 8 ;
}
/* end subroutine (netorder_rlong) */

#endif /* (LONG_BIT == 64) */


int netorder_rll(buf,lwp)
char	*buf ;
LONG	*lwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*lwp = 0 ;
	*lwp |= (((LONG) ubuf[0]) << 56) ;
	*lwp |= (((LONG) ubuf[1]) << 48) ;
	*lwp |= (((LONG) ubuf[2]) << 40) ;
	*lwp |= (((LONG) ubuf[3]) << 32) ;
	*lwp |= (((LONG) ubuf[4]) << 24) ;
	*lwp |= (((LONG) ubuf[5]) << 16) ;
	*lwp |= (((LONG) ubuf[6]) << 8) ;
	*lwp |= (((LONG) ubuf[7]) << 0) ;

	return 8 ;
}
/* end subroutine (netorder_rll) */


int netorder_ruchar(buf,cwp)
char	*buf ;
uchar	*cwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*cwp = ubuf[0] ;
	return 1 ;
}
/* end subroutine (netorder_ruchar) */


int netorder_rushort(buf,swp)
char	*buf ;
ushort	*swp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*swp = 0 ;
	*swp |= (ubuf[0] << 8) ;
	*swp |= (ubuf[1] << 0) ;

	return 2 ;
}
/* end subroutine (netorder_rushort) */


int netorder_ruint(buf,iwp)
char	*buf ;
uint	*iwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*iwp = 0 ;
	*iwp |= (ubuf[0] << 24) ;
	*iwp |= (ubuf[1] << 16) ;
	*iwp |= (ubuf[2] << 8) ;
	*iwp |= (ubuf[3] << 0) ;

	return 4 ;
}
/* end subroutine (netorder_ruint) */


#if	(LONG_BIT == 64)

int netorder_rulong(buf,lwp)
char	*buf ;
ulong	*lwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*lwp = 0 ;
	*lwp |= (((ulong) ubuf[0]) << 56) ;
	*lwp |= (((ulong) ubuf[1]) << 48) ;
	*lwp |= (((ulong) ubuf[2]) << 40) ;
	*lwp |= (((ulong) ubuf[3]) << 32) ;
	*lwp |= (((ulong) ubuf[4]) << 24) ;
	*lwp |= (((ulong) ubuf[5]) << 16) ;
	*lwp |= (((ulong) ubuf[6]) << 8) ;
	*lwp |= (((ulong) ubuf[7]) << 0) ;

	return 8 ;
}
/* end subroutine (netorder_rulong) */

#else /* (LONG_BIT == 64) */

int netorder_rulong(buf,lwp)
char	*buf ;
ulong	*lwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*lwp = 0 ;
	*lwp |= (((ulong) ubuf[0]) << 24) ;
	*lwp |= (((ulong) ubuf[1]) << 16) ;
	*lwp |= (((ulong) ubuf[2]) << 8) ;
	*lwp |= (((ulong) ubuf[3]) << 0) ;

	return 4 ;
}
/* end subroutine (netorder_rulong) */

#endif /* (LONG_BIT == 64) */


int netorder_rull(buf,lwp)
char	*buf ;
ULONG	*lwp ;
{
	uchar	*ubuf = (uchar *) buf ;


	*lwp = 0 ;
	*lwp |= (((ULONG) ubuf[0]) << 56) ;
	*lwp |= (((ULONG) ubuf[1]) << 48) ;
	*lwp |= (((ULONG) ubuf[2]) << 40) ;
	*lwp |= (((ULONG) ubuf[3]) << 32) ;
	*lwp |= (((ULONG) ubuf[4]) << 24) ;
	*lwp |= (((ULONG) ubuf[5]) << 16) ;
	*lwp |= (((ULONG) ubuf[6]) << 8) ;
	*lwp |= (((ULONG) ubuf[7]) << 0) ;

	return 8 ;
}
/* end subroutine (netorder_rull) */


int netorder_wchar(buf,cw)
char	*buf ;
int	cw ;
{
	uchar	*ubuf = (uchar *) buf ;


	ubuf[0] = (uchar) cw ;
	return 1 ;
}
/* end subroutine (netorder_wchar) */


int netorder_wshort(buf,sw)
char	*buf ;
int	sw ;
{
	uchar	*ubuf = (uchar *) buf ;


	ubuf[0] = (uchar) ((sw >> 8) & 0xff) ;
	ubuf[1] = (uchar) ((sw >> 0) & 0xff) ;

	return 2 ;
}
/* end subroutine (netorder_wshort) */


int netorder_wint(buf,iw)
char	*buf ;
int	iw ;
{
	uchar	*ubuf = (uchar *) buf ;


	ubuf[0] = (uchar) ((iw >> 24) & 0xff) ;
	ubuf[1] = (uchar) ((iw >> 16) & 0xff) ;
	ubuf[2] = (uchar) ((iw >> 8) & 0xff) ;
	ubuf[3] = (uchar) ((iw >> 0) & 0xff) ;

	return 4 ;
}
/* end subroutine (netorder_wint) */


#if	(LONG_BIT == 64)

int netorder_wlong(buf,lw)
char	*buf ;
long	lw ;
{
	uchar	*ubuf = (uchar *) buf ;


	ubuf[0] = ((lw >> 56) & 0xff) ;
	ubuf[1] = ((lw >> 48) & 0xff) ;
	ubuf[2] = ((lw >> 40) & 0xff) ;
	ubuf[3] = ((lw >> 32) & 0xff) ;

	ubuf[4] = ((lw >> 24) & 0xff) ;
	ubuf[5] = ((lw >> 16) & 0xff) ;
	ubuf[6] = ((lw >> 8) & 0xff) ;
	ubuf[7] = ((lw >> 0) & 0xff) ;

	return 8 ;
}
/* end subroutine (netorder_wlong) */

#else

int netorder_wlong(buf,lw)
char	*buf ;
long	lw ;
{
	uchar	*ubuf = (uchar *) buf ;


	ubuf[0] = ((lw >> 24) & 0xff) ;
	ubuf[1] = ((lw >> 16) & 0xff) ;
	ubuf[2] = ((lw >> 8) & 0xff) ;
	ubuf[3] = ((lw >> 0) & 0xff) ;

	return 4 ;
}
/* end subroutine (netorder_wlong) */

#endif /* (LONG_BIT == 64) */


int netorder_wll(buf,lw)
char	*buf ;
LONG	lw ;
{
	uchar	*ubuf = (uchar *) buf ;


	ubuf[0] = ((lw >> 56) & 0xff) ;
	ubuf[1] = ((lw >> 48) & 0xff) ;
	ubuf[2] = ((lw >> 40) & 0xff) ;
	ubuf[3] = ((lw >> 32) & 0xff) ;

	ubuf[4] = ((lw >> 24) & 0xff) ;
	ubuf[5] = ((lw >> 16) & 0xff) ;
	ubuf[6] = ((lw >> 8) & 0xff) ;
	ubuf[7] = ((lw >> 0) & 0xff) ;

	return 8 ;
}
/* end subroutine (netorder_wll) */


/* older API */


#ifdef	COMMENT

int netorder_readl(buf,ulwp)
char	*buf ;
ULONG	*ulwp ;
{
	LONG	*lwp = (LONG *) ulwp ;

	return netorder_rlong(buf,lwp) ;
}


int netorder_writel(buf,ulw)
char	*buf ;
ULONG	ulw ;
{


	return netorder_wlong(buf,ulw) ;
}


int netorder_readi(buf,uiwp)
char	*buf ;
uint	*uiwp ;
{
	int	*iwp = (int *) uiwp ;


	return netorder_rint(buf,iwp) ;
}


int netorder_writei(buf,uiw)
char	*buf ;
uint	uiw ;
{


	return netorder_wint(buf,uiw) ;
}


int netorder_reads(buf,uswp)
char	*buf ;
ushort	*uswp ;
{
	short	*swp = (short *) uswp ;


	return netorder_rshort(buf,swp) ;
}


int netorder_writes(buf,usw)
char	*buf ;
uint	usw ;
{


	return netorder_wshort(buf,usw) ;
}

#endif /* COMMENT */



