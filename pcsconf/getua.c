/* getua */

/* subroutines to access the user-attribute database */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines were was written so that we could use a single
        interface to access the user-attribute database on all UNIX® platforms.
        The idea is that this code module provides a platform independent
        implementation of UNIX 'passwd' database access subroutines.

	These are subroutines:

	getua_begin() 
	getua_ent() 
	getua_end() 
	getua_name() 
	getua_uid()


*******************************************************************************/


#define	GETUA_MASTER	0


#include	"envstandards.h"

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<userattrent.h>
#include	<localmisc.h>

#include	"getua.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getua_begin()
{
	return uc_setuserattr() ;
}


int getua_end()
{
	return uc_enduserattr() ;
}


int getua_ent(userattr_t *uap,char *uabuf,int ualen)
{
	userattr_t	*tuap ;
	int		rs ;
	if ((rs = uc_getuserattr(&tuap)) > 0) {
	    rs = userattrent_load(uap,uabuf,ualen,tuap) ;
	    uc_freeuserattr(tuap) ;
	}
	return rs ;
}


int getua_name(userattr_t *uap,char *uabuf,int ualen,const char *name)
{
	userattr_t	*tuap ;
	int		rs ;
	if ((rs = uc_getusernam(name,&tuap)) > 0) {
	    rs = userattrent_load(uap,uabuf,ualen,tuap) ;
	    uc_freeuserattr(tuap) ;
	}
	return rs ;
}
/* end subroutine (getua_name) */


int getua_uid(userattr_t *uap,char *uabuf,int ualen,uid_t uid)
{
	userattr_t	*tuap ;
	int		rs ;
	if ((rs = uc_getuseruid(uid,&tuap)) > 0) {
	    rs = userattrent_load(uap,uabuf,ualen,tuap) ;
	    uc_freeuserattr(tuap) ;
	}
	return rs ;
}
/* end subroutine (getua_uid) */


