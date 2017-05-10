/* getse */

/* get service entry */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines were was written so that we could use a single
        interface to access the 'servent' database on all UNIX® platforms. This
        code module provides a platform independent implementation of UNIX®
        'servent' database access subroutines.

	These are the preferred interfaces:

	preferred interfaces: getse_name(), getse_port() ;


*******************************************************************************/


#define	GETPE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"getse.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getse_begin(int sf)
{
	return uc_setservent(sf) ;
}
/* end subroutine (getse_begin) */


int getse_end()
{
	return uc_endservent() ;
}
/* end subroutine (getse_end) */


int getse_ent(struct servent *sep,char rbuf[],int rlen)
{
	int		rs ;
	if ((rs = uc_getservent(sep,rbuf,rlen)) == SR_NOTFOUND) rs = SR_OK ;
	return rs ;
}
/* end subroutine (getse_ent) */


int getse_name(struct servent *sep,char rbuf[],int rlen,cchar pn[],cchar *svc)
{
	return uc_getservbyname(svc,pn,sep,rbuf,rlen) ;
}
/* end subroutine (getse_name) */


int getse_port(struct servent *sep,char rbuf[],int rlen,cchar pn[],int num)
{
	return uc_getservbyport(num,pn,sep,rbuf,rlen) ;
}
/* end subroutine (getse_port) */


