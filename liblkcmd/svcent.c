/* svcent */

/* service file entry (SVCENT) */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-10-13, David A­D­ Morano
	This was split out of the HOMEPAGE program (where it was originally
	local).

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We perform some slight management on SVCENT objects.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"svcent.h"
#include	"svckv.h"


/* local defines */


/* typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef cchar	cchar ;
#endif


/* external subroutines */

extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfdequote(cchar *,int,cchar **) ;
extern int	sfbasename(cchar *,int,cchar **) ;
extern int	sfdirname(cchar *,int,cchar **) ;
extern int	nextfieldterm(cchar *,int,cchar *,cchar **) ;
extern int	nchr(cchar *,int,int) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	cfdeci(cchar *,int,int *) ;
extern int	cfdecti(cchar *,int,int *) ;
extern int	cfdecmfi(cchar *,int,int *) ;
extern int	cfdecmful(cchar *,int,ulong *) ;
extern int	ctdeci(char *,int,int) ;
extern int	msleep(int) ;
extern int	tolc(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int svcent_getval(SVCENT *sep,cchar *k,cchar **rpp)
{
	const int	n = sep->nkeys ;
	cchar		*(*kv)[2] = sep->keyvals ;
	return svckv_val(kv,n,k,rpp) ;
}
/* end subroutine (svcent_getval) */


int svcent_getdeval(SVCENT *sep,cchar *k,cchar **rpp)
{
	const int	n = sep->nkeys ;
	cchar		*(*kv)[2] = sep->keyvals ;
	return svckv_dequote(kv,n,k,rpp) ;
}
/* end subroutine (svcent_getdeval) */


int svcent_islib(SVCENT *sep,cchar **rpp)
{
	const int	n = sep->nkeys ;
	int		vl ;
	cchar		*(*kv)[2] = sep->keyvals ;
	cchar		*k1 = "so" ;
	if ((vl = svcent_getval(sep,k1,rpp)) > 0) {
	    cchar	*dummy ;
	    if (svckv_isprog(kv,n,&dummy) == 0) {
		vl = 0 ;
	    }
	}
	return vl ;
}
/* end subroutine (svcent_islib) */


