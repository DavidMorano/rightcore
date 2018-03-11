/* snxtilook */

/* make string version of the XTI |t_look(3nsl)| codes */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Ths subroutine creates in the result string a symbolic representation
	of the XTI |t_look(3nsl)| return codes.

	Synopsis:

	int snxtilook(dbuf,dlen,flags)
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
	int		v ;		/* value */
	cchar		*n ;		/* name */
	cchar		*msg ;		/* messaeg */
} ;


/* foward references */

static int	findent(int) ;


/* local variables */

static const struct val	vals[] = {
	{ T_LISTEN, "LISTEN", "connection indication received" },
	{ T_CONNECT, "CONNECT", "connect confirmation received" },
	{ T_DATA, "DATA", "normal data received" },
	{ T_EXDATA, "EXDATA", "expedited data received" },
	{ T_DISCONNECT, "DISCONNECT", "disconnect received" },
	{ T_UDERR, "UDERR", "data gram error indication" },
	{ T_ORDREL, "ORDREL", "orderly release indication" },
	{ T_GODATA, "GODATA", "sending normal data is again possible" },
	{ T_GOEXDATA, "GOEXDATA", "sending expedited data is again possible" },
	{ -1, NULL }
} ;


/* exported subroutines */


int snxtilook(char *dbuf,int dlen,int v)
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
/* end subroutine (snxtilook) */


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


