/* inet_ntoa_r */

/* convert network-format INET address to base-256 d.d.d.d representation */


/* revision history:

	= 1998-06-26, David A­D­ Morano

	I wrote this to get a reentrant subroutine for this function.
	Maybe someday POSIX will make this subroutine instead but
	until, you can use this!  It's dirty, but POSIX should have
	already done it.  What planet do those POSIX guys live on?


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a reentrant version of the subroutine 'inet_ntoa(3xnet)'.

	Synopsis:

	char *inet_ntoa_r(struct in_addr in,char rbuf,int rlen)

	Arguments:

	in		INET4 address to convert
	rbuf		buffer to hold result
	rlen		length of buffer to hold result

	Returns:

	-		pointer to NUL-terminated result buffer


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>

#include	<vsystem.h>
#include	<sbuf.h>


/* local defines */

#ifndef	INET4ADDRLEN
#define	INET4ADDRLEN	sizeof(struct in_addr)
#endif

#define	UC(b)		(((unsigned int) (b)) & 0xff)


/* external subroutines */


/* local variables */


/* exported subroutines */


char *inet_ntoa_r(in,rbuf,rlen)
struct in_addr	in ;
char		rbuf[] ;
int		rlen ;
{
	SBUF		b ;
	int		rs ;
	int		rs1 ;
	const uchar	*ap = (const uchar *) &in ;

	if (rbuf == NULL) return NULL ;

/* remember that the INET address is *already* in network byte order! */

	if ((rs = sbuf_start(&b,rbuf,rlen)) >= 0) {
	    int	i ;

	    for (i = 0 ; i < INET4ADDRLEN ; i += 1) {
		if (i > 0) sbuf_char(&b,'.') ;
		sbuf_deci(&b,UC(ap[i])) ;
	    }

	    rs1 = sbuf_finish(&b) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return (rs >= 0) ? rbuf : NULL ;
}
/* end subroutine (inet_ntoa_r) */


