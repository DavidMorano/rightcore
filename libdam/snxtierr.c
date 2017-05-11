/* snxtierr */

/* make string version of the XTI t-error codes */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Ths subroutine creates in the result string a symbolic representation
	of the XTI t-error codes.

	Synopsis:

	int snxtierr(dbuf,dlen,flags)
	char		*dbuf ;
	int		dlen ;
	int		flags ;

	Arguments:

	dbuf		destination string buffer
	dlen		destination string buffer length
	flags		open-flags

	Returns:

	>=0		number of bytes in result
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<xti.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;


/* external variables */


/* local structures */

struct val {
	int		v ;
	cchar		*n ;
	cchar		*msg ;
} ;


/* forward references */

static int	findent(int) ;


/* local variables */

static const struct val	vals[] = {
	{ TBADADDR, "BADADDR", "incorrect address format" },
 	{ TBADOPT, "BADOPT", "incorrect options format" },
	{ TACCES, "ACCESS", "illegal permissions" },
	{ TBADF, "BADF", "illegal file descriptor" },
	{ TNOADDR, "NOADDR", "could not allocate address" },
	{ TOUTSTATE, "OUTSTATE", "routine will place interface out of state" },
	{ TBADSEQ, "BADSEQ", "illegal called-calling sequence number" },
	{ TSYSERR, "SYSERR", "system error" },
	{ TLOOK, "LOOK", "an event requires attention" },
	{ TBADDATA, "BADDATA", "illegal amount of data" },
	{ TBUFOVFLW, "BUFOVFLW", "buffer not large enough" },
	{ TFLOW, "FLOW" "cannot send message - (blocked)" },
	{ TNODATA, "NODATA", "no message currently available" },
	{ TNODIS, "NODIS", "disconnect message not found" },
	{ TNOUDERR, "NOUDERR", "unitdata error message not found" },
	{ TBADFLAG, "BADFLAG", "incorrect flags specified" },
	{ TNOREL, "NOREL", "orderly release message not found" },
	{ TNOTSUPPORT, "NOTSUPPORT", "primitive not supported by provider" },
	{ TSTATECHNG, "STATECHNG", "state is in process of changing" },
	{ TNOSTRUCTYPE, "TNOSTRUCTYPE", 
	"unsupported structure type requested" },
	{ TBADNAME, "BADNAME", "invalid transport provider name" },
	{ TBADQLEN, "BADQLEN", "listener queue length limit is zero" },
	{ TADDRBUSY, "ADDRBUSY", "transport address is in use" },
	{ TINDOUT, "INDOUT", "outstanding connection indications" },
	{ TPROVMISMATCH, "PROVMISMATCH", 
	"listener-acceptor transport provider mismatch" },
	{ TRESQLEN, "TRESQLEN", 
	"connection acceptor has listen queue length limit greater than zero" },
	{ TRESADDR, "RESADDR", 
	"connection acceptor-listener addresses not "
	"same but required by transport" },
	{ TQFULL, "QFULL", "incoming connection queue is full" },
	{ TPROTO, "PROTO", "protocol error on transport primitive" },
	{ -1, NULL }
} ;


/* exported subroutines */


int snxtierr(char *dbuf,int dlen,int v)
{
	int		rs ;
	int		i ;
	cchar		*n ;

	if (dbuf == NULL) return SR_FAULT ;

	i = findent(v) ;
	n = (i >= 0) ? vals[i].n : "UNKNOWN" ;
	rs = sncpy1(dbuf,dlen,n) ;

	return rs ;
}
/* end subroutine (snxtierr) */


/* local subroutines */


static int findent(int v)
{
	int		f = FALSE ;
	int		i ;
	for (i = 0 ; vals[i].v >= 0 ; i += 1) {
	    f = (v == vals[i].v) ;
	    if (f) break ;
	} /* end for */
	return (f) ? i : -1 ;
}
/* end subroutine (findent) */


