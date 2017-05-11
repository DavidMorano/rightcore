/* uc_initgroups */

/* interface component for UNIX® library-3c */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-04-13, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<grp.h>
#include	<errno.h>

#include	<vsystem.h>


/* exported subroutines */


int uc_initgroups(cchar *name,gid_t gid)
{
	int		rs = SR_OK ;
	if (name == NULL) return SR_FAULT ;
	if (name[0] == '\0') return SR_INVALID ;
	if (initgroups(name,gid) < 0) rs = (- errno) ;
	return rs ;
}
/* end subroutine (uc_initgroups) */


