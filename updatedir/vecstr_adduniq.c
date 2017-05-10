/* vecstr_adduniq */

/* conditionally add a string in the form of a key-value pair */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine adds a string to the object, but first checks to see if
	it is already present.  Specified strings that already match an
	existing entry are not added.

	Synopsis:

	int vecstr_adduniq(op,sp,sl)
	vecstr		*op ;
	const char	*sp ;
	int		sl ;

	Arguments:

	op		vecstr string to add to
	sp		pointer to key string to add
	sl		length of string to add

	Returns:

	INT_MAX		string was already in container
	>=0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(char **,char **) ;

extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int vecstr_adduniq(vecstr *op,cchar *sp,int sl)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->va == NULL) return SR_NOTOPEN ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if ((rs = vecstr_findn(op,sp,sl)) >= 0) {
	    rs = INT_MAX ;
	} else if (rs == SR_NOTFOUND) {
	    rs = vecstr_add(op,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (vecstr_adduniq) */


