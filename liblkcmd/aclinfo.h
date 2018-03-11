/* aclinfo */


/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

#ifndef	ACLINFO_INCLUDE
#define	ACLINFO_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/acl.h>

#include	<localmisc.h>

#include	"acltypes.h"


struct aclinfo {
	uid_t		uid ;
	gid_t		gid ;
	int		type ;
	int		soltype ;
	int		op ;		/* add or subtract */
	int		perm ;		/* permission bits */
} ;


typedef struct aclinfo	aclinfo_t ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	aclinfo_mksol(struct aclinfo *) ;
extern int	aclinfo_isdeftype(struct aclinfo *) ;
extern int	aclinfo_isidtype(struct aclinfo *) ;

#ifdef	__cplusplus
}
#endif


#endif /* ACLINFO_INCLUDE */


