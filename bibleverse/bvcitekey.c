/* bvcitekey */

/* manage BV cite key */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We manage a BV cite-key object.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bvcitekey.h"


/* local defines */


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int bvcitekey_set(BVCITEKEY *bvp,uint *ckp)
{
	register uint	ck = 0 ;

	ck |= (bvp->nlines & UCHAR_MAX) ;
	ck = (ck << 8) ;
	ck |= (bvp->b & UCHAR_MAX) ;
	ck = (ck << 8) ;
	ck |= (bvp->c & UCHAR_MAX) ;
	ck = (ck << 8) ;
	ck |= (bvp->v & UCHAR_MAX) ;

	*ckp = ck ;
	return SR_OK ;
}
/* end subroutine (bvcitekey_set) */


int bvcitekey_get(BVCITEKEY *bvp,uint *ckp)
{
	register uint	ck = *ckp ;

	bvp->v = (ck & UCHAR_MAX) ;
	ck = (ck >> 8) ;
	bvp->c = (ck & UCHAR_MAX) ;
	ck = (ck >> 8) ;
	bvp->b = (ck & UCHAR_MAX) ;
	ck = (ck >> 8) ;
	bvp->nlines = (ck & UCHAR_MAX) ;

	return SR_OK ;
}
/* end subroutine (bvcitekey_get) */


