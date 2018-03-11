/* aclinfo */

/* ACL information */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2005-02-24, David A­D­ Morano
        This code was adopted from the SHCAT program, which in turn had been
        adopted from programs with a lineage dating back (from the previous
        notes in this space) from 1989! I deleted the long list of notes here to
        clean this up.

*/

/* Copyright © 2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines perform (various) ACL handling functions.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/acl.h>
#include	<limits.h>
#include	<unistd.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"aclinfo.h"


/* local defines */

#ifdef	MAX_ACL_ENTRIES
#define	MAXACLS		MAX_ACL_ENTRIES
#else
#define	MAXACLS		1024
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


/* aclinfo (ACL information) */
int aclinfo_mksol(struct aclinfo *ap)
{
	int		soltype = -1 ;
	switch (ap->type) {
	case acltype_user:
	    soltype = (ap->uid >= 0) ? USER : USER_OBJ ;
	    break ;
	case acltype_group:
	    soltype = (ap->gid >= 0) ? GROUP : GROUP_OBJ ;
	    break ;
	case acltype_other:
	    soltype = OTHER_OBJ ;
	    break ;
	case acltype_mask:
	    soltype = CLASS_OBJ ;
	    break ;
	case acltype_defuser:
	    soltype = (ap->uid >= 0) ? DEF_USER : DEF_USER_OBJ ;
	    break ;
	case acltype_defgroup:
	    soltype = (ap->gid >= 0) ? DEF_GROUP : DEF_GROUP_OBJ ;
	    break ;
	case acltype_defother:
	    soltype = DEF_OTHER_OBJ ;
	    break ;
	case acltype_defmask:
	    soltype = DEF_CLASS_OBJ ;
	    break ;
	} /* end switch */
	ap->soltype = soltype ;
	return soltype ;
}
/* end subroutine (aclinfo_mksol) */


int aclinfo_isdeftype(struct aclinfo *ap)
{
	int		f = FALSE ;
	switch (ap->type) {
	case acltype_defuser:
	case acltype_defgroup:
	case acltype_defother:
	case acltype_defmask:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (aclinfo_isdeftype) */


int aclinfo_isidtype(struct aclinfo *ap)
{
	int		f = FALSE ;
	switch (ap->type) {
	case acltype_user:
	case acltype_defuser:
	    f = (ap->uid >= 0) ;
	    break ;
	case acltype_group:
	case acltype_defgroup:
	    f = (ap->gid >= 0) ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (aclinfo_isidtype) */


