/* acltypes */

/* ACL types (types of ACLs) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2005-02-24, David A­D­ Morano
        This code was adopted from the SHCAT program, which in turn had been
        adopted from programs with a lineage dating back (from the previous
        notes in this space) from 1989! I deleted the long list of notes here to
        clean this up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine performs ACL handling.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/acl.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"acltypes.h"


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
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* global variables */

static const char	*acltypes[] = {
	"user",
	"group",
	"other",
	"mask",
	"duser",
	"dgroup",
	"dother",
	"dmask",
	NULL
} ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int getacltype(const char *tp,int tl)
{

	return matostr(acltypes,1,tp,tl) ;
}
/* end subroutine (getacltype) */


