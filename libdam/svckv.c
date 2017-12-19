/* svckv */

/* key-value type functions */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2017-10-13, David A­D­ Morano
	This was split out of the HOMEPAGE program (where it was originally
	local).

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We perform some light key-value type management.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>

#include	"svckv.h"


/* local defines */


/* typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char	cchar ;
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
extern int	msleep(int) ;
extern int	tolc(int) ;
extern int	isdigitlatin(int) ;
extern int	isFailOpen(int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;
extern char	*strnpbrk(cchar *,int,cchar *) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */

static char	*isexecs[] = {
	"p",
	"a",
	NULL
} ;

static cchar	*svcopts[] = {
	"termout",
	NULL
} ;

enum svcopts {
	svcopt_termout,
	svcopt_overlast
} ;


/* exported subroutines */


int svckv_val(cchar *(*kv)[2],int n,cchar *np,cchar **vpp)
{
	int		i ;
	int		vl = 0 ;
	if (vpp != NULL) *vpp = NULL ;
	for (i = 0 ; i < n ; i += 1) {
	    if (strcmp(kv[i][0],np) == 0) {
	        if (vpp != NULL) *vpp = kv[i][1] ;
	        vl = strlen(kv[i][1]) ;
	        break ;
	    }
	} /* end for */
	return vl ;
}
/* end subroutine (svckv_val) */


int svckv_dequote(cchar *(*kv)[2],int n,cchar *np,cchar **vpp)
{
	int		cl = 0 ;
	int		vl ;
	cchar		*vp ;
	cchar		*cp = NULL ;
	if ((vl = svckv_val(kv,n,np,&vp)) > 0) {
	    cl = sfdequote(vp,vl,&cp) ;
	}
	if (vpp != NULL) *vpp = cp ;
	return cl ;
}
/* end subroutine (svckv_dequote) */


int svckv_isfile(cchar *(*kv)[2],int n,cchar **vpp)
{
	int		vl ;
	cchar		*sp = "file" ;
	vl = svckv_val(kv,n,sp,vpp) ;
	return vl ;
}
/* end subroutine (svckv_isfile) */


int svckv_ispass(cchar *(*kv)[2],int n,cchar **vpp)
{
	int		vl ;
	cchar		*sp = "pass" ;
	vl = svckv_val(kv,n,sp,vpp) ;
	return vl ;
}
/* end subroutine (svckv_ispass) */


#ifdef	COMMENT
int svckv_islib(cchar *(*kv)[2],int n,cchar **vpp)
{
	int		vl ;
	cchar		*sp = "so" ;
	vl = svckv_val(kv,n,sp,vpp) ;
	return vl ;
}
/* end subroutine (svckv_islib) */
#endif /* COMMENT */


int svckv_isprog(cchar *(*kv)[2],int n,cchar **vpp)
{
	int		i ;
	int		vl = 0 ;
	for (i = 0 ; isexecs[i] != NULL ; i += 1) {
	    vl = svckv_val(kv,n,isexecs[i],vpp) ;
	    if (vl > 0) break ;
	}
	return vl ;
}
/* end subroutine (svckv_isprog) */


/* return (as the integer return value) a bit-set of options from the SVCENT */
int svckv_svcopts(cchar *(*kv)[2],int n)
{
	int		vl ;
	int		ow = 0 ;
	cchar		*k = "opts" ;
	cchar		*vp ;
	if ((vl = svckv_val(kv,n,k,&vp)) > 0) {
	    int		i ;
	    int		cl ;
	    cchar	*cp ;
	    cchar	*tp ;
	    while ((tp = strnpbrk(vp,vl," ,")) != NULL) {
	        if ((cl = sfshrink(vp,(tp-vp),&cp)) > 0) {
	            if ((i = matstr(svcopts,cp,cl)) >= 0) {
	                ow |= (1<<i) ;
	            }
	        }
	        vl -= ((tp+1)-vp) ;
	        vp = (tp+1) ;
	    } /* end while */
	    if (vl > 0) {
	        if ((cl = sfshrink(vp,vl,&cp)) > 0) {
	            if ((i = matstr(svcopts,cp,cl)) >= 0) {
	                ow |= (1<<i) ;
	            }
	        }
	    }
	} /* end if (svckv) */
	return ow ;
}
/* end subroutine (svckv_svcopts) */


