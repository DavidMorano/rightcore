/* mkenvfrom */

/* create an environment FROM address */


#define	CF_DEBUGS	0		/* not-switchable debug print-outs */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine is used to create an environment FROM address for a
	mail message.

	Synopsis:

	int mkenvfrom(pip,rbuf,rlen,fa)
	PROGINFO	*pip ;
	char		rbuf[] ;
	char		rlen ;
	const char	*fa ;

	Arguments:

	pip		pointer to program information
	rbuf		caller supplied buffer to place result in
	rlen		length of caller supplied buffer
	fa 		candidate From Address

	Returns:

	>=0		length of returned ID
	<0		error in process of creating ID


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkbestaddr(char *,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mkenvfrom(PROGINFO *pip,char *dbuf,int dlen,cchar *fap)
{
	int		rs ;

	if (dbuf == NULL) return SR_FAULT ;
	if (fap == NULL) return SR_FAULT ;

	dbuf[0] = '\0' ;
	if (fap != NULL) {
	    rs = mkbestaddr(dbuf,dlen,fap,-1) ;
	} else {
	    rs = sncpy1(dbuf,dlen,pip->username) ;
	}

	return rs ;
}
/* end subroutine (mkenvfrom) */


