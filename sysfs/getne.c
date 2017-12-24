/* getne */

/* get protocol entry */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines were was written so that we could use a single
        interface to access the 'netent' database on all UNIX® platforms. This
        code module provides a platform independent implementation of UNIX®
        'netent' database access subroutines.

	These are the preferred interfaces:

	preferred interfaces: getne_name(), getne_num() ;
	preferred interfaces: getgr_name(), getgr_gid() ;


*******************************************************************************/


#define	GETNE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"getne.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getne_begin(int sf)
{
	return uc_setnetent(sf) ;
}
/* end subroutine (getne_begin) */


int getne_end()
{
	return uc_endnetent() ;
}
/* end subroutine (getne_end) */


int getne_ent(struct netent *pp,char *rbuf,int rlen)
{
	int		rs ;
	if ((rs = uc_getnetent(pp,rbuf,rlen)) == SR_NOTFOUND) rs = SR_OK ;
	return rs ;
}
/* end subroutine (getne_ent) */


int getne_name(struct netent *pp,char *rbuf,int rlen,cchar *name)
{
	return uc_getnetbyname(name,pp,rbuf,rlen) ;
}
/* end subroutine (getne_name) */


int getne_addr(struct netent *pp,char *rbuf,int rlen,int type,int num)
{
	return uc_getnetbyaddr(num,type,pp,rbuf,rlen) ;
}
/* end subroutine (getne_addr) */


