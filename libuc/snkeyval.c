/* snkeyval */

/* string formatting (key-value pair) */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We make something that looks like an environment vaiable string (with
	the key included) from a key and value specification (consisting of a
	string pointer and a string length).

	Synopsis:

	int snkeyval(dbuf,dlen,kp,kl,vp,vl)
	char		dbuf[] ;
	int		dlen ;
	const char	kp[], vp[] ;
	int		kl, vl ;

	Arguments:

	dbuf		destination buffer
	dlen		destination buffer length
	kp		key-pointer
	vp		value-pointer
	kl		key length
	vl		value-length

	Returns:

	>=0		length of created string
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#define	MIDDLECHAR	'='


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;


/* exported subroutines */


int snkeyval(char *dbuf,int dlen,cchar *kp,int kl,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (rs >= 0) {
	    rs = storebuf_strw(dbuf,dlen,i,kp,kl) ;
	    i += rs ;
	}

	if (rs >= 0) {
	    rs = storebuf_char(dbuf,dlen,i,MIDDLECHAR) ;
	    i += rs ;
	}

	if ((rs >= 0) && (vp != NULL)) {
	    rs = storebuf_strw(dbuf,dlen,i,vp,vl) ;
	    i += rs ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (snkeyval) */


