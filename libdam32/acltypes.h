/* acltypes */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	ACLTYPES_INCLUDE
#define	ACLTYPES_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */


enum acltypes {
	acltype_user,
	acltype_group,
	acltype_other,
	acltype_mask,
	acltype_defuser,
	acltype_defgroup,
	acltype_defother,
	acltype_defmask,
	acltype_overlast
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	getacltype(const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* ACLTYPES_INCLUDE */


