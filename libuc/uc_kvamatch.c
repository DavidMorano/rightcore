/* uc_kvamatch */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine finds a keyname (if it exists) in a KVA-type of object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<secdb.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* exported subroutines */


int uc_kvamatch(kva,keyname,rpp)
kva_t		*kva ;
const char	keyname[] ;
const char	**rpp ;
{
	int		rs ;
	const char	*cp ;

	if (kva == NULL) return SR_FAULT ;
	if (keyname == NULL) return SR_FAULT ;

	if (keyname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("uc_kvamatch: keyname=%s\n",keyname) ;
#endif

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)

	if ((cp = kva_match(kva,(char *) keyname)) != NULL) {
	    rs = strlen(cp) ;
	} else
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("uc_kvamatch: kva_match() cp=%s\n",cp) ;
#endif

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? cp : NULL ;

#else /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

	rs = SR_NOSYS ;

#endif /* defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0) */

#if	CF_DEBUGS
	debugprintf("uc_kvamatch: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (uc_kvamatch) */


