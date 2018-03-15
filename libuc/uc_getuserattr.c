/* uc_getuserattr */

/* interface component for UNIX® library-3c */
/* deal with user-attributes (appears in coming version of Solaris®) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-06-16, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module supplies the following subroutines for accessing the system
	user-attribute database:

		uc_setuserattr()
		uc_enduserattr()
		uc_getuserattr()
		uc_getusernam()
		uc_freeuserattr()

	These subroutines were grouped together into a single module (not at
	all typical) because they have to be used together, since using them
	apart makes little sense.

	Note that these stupid subroutines are thread-safe but are not
	reentrant!  Yes, that is what you get to when you have extreme brain
	damaged system developers.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<user_attr.h>

#include	<vsystem.h>
#include	<userattrent.h>
#include	<localmisc.h>


/* exported subroutines */


int uc_setuserattr()
{
	int		rs ;

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)
	{
	    rs = SR_OK ;
	    setuserattr() ;
	}
#else /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */
	{
	    rs = SR_NOSYS ;
	}
#endif /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

	return rs ;
}
/* end subroutine (uc_setuserattr) */


int uc_enduserattr()
{
	int		rs ;

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)
	{
	    rs = SR_OK ;
	    enduserattr() ;
	}
#else /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */
	{
	    rs = SR_NOSYS ;
	}
#endif /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

	return rs ;
}
/* end subroutine (uc_enduserattr) */


int uc_getuserattr(userattr_t **rpp)
{
	userattr_t	*uap ;
	int		rs = SR_OK ;

	if (rpp == NULL) return SR_FAULT ;

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)
	{
	    uap = getuserattr() ;
	    if (uap != NULL) {
		rs = userattrent_size(uap) ;
	    }
	}
#else /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */
	{
	    rs = SR_NOSYS ;
	}
#endif /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

	*rpp = (rs >= 0) ? uap : NULL ;
	return rs ;
}
/* end subroutine (uc_getuserattr) */


int uc_getusernam(cchar *username,userattr_t **rpp)
{
	userattr_t	*uap ;
	int		rs ;

	if (rpp == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	if (username[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_getusernam: username=%s\n",username) ;
#endif

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)
	{
	    uap = getusernam(username) ;
	    if (uap != NULL) {
		rs = userattrent_size(uap) ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	}
#else /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */
	{
	    rs = SR_NOSYS ;
	}
#endif /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

#if	CF_DEBUGS
	debugprintf("uc_getusernam() rs=%d\n",rs) ;
#endif

	*rpp = (rs >= 0) ? uap : NULL ;
	return rs ;
}
/* end subroutine (uc_getusernam) */


int uc_getuseruid(uid_t uid,userattr_t **rpp)
{
	userattr_t	*uap ;
	int		rs ;

	if (rpp == NULL) return SR_FAULT ;

	if (uid < 0) return SR_INVALID ;

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)
	{
	    uap = getuseruid(uid) ;
	    if (uap != NULL) {
		rs = userattrent_size(uap) ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	}
#else /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */
	{
	    rs = SR_NOSYS ;
	}
#endif /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

	*rpp = (rs >= 0) ? uap : NULL ;
	return rs ;
}
/* end subroutine (uc_getuseruid) */


int uc_freeuserattr(userattr_t *uap)
{
	int		rs ;

	if (uap == NULL) return SR_FAULT ;

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)
	{
	    rs = SR_OK ;
	    free_userattr(uap) ;
	}
#else /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */
	{
	    rs = SR_NOSYS ;
	}
#endif /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

	return rs ;
}
/* end subroutine (uc_freeuserattr) */


