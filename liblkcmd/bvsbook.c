/* bvsbook */

/* manage a BVS "book" entry */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module manages a BVS "book" entry.

	Synopsis:

	int bvsbook_get(BVSBOOK *bep,ushort *a) ;

	Arguments:

	- bep		book entry pointer
	- a		entry array buffer

	Returns:

	>=0		OK
	<0		error code


*******************************************************************************/


#define	BVSMK_MASTER	0


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bvsbook.h"


/* local defines */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,const char *,
			const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy6(char *,int,const char *,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif


/* external variables */


/* exported variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int bvsbook_set(BVSBOOK *bep,ushort *a)
{

	a[0] = bep->al ;
	a[1] = bep->ci ;
	a[2] = bep->nverses ;
	a[3] = bep->nzverses ;
	return SR_OK ;
}
/* end subroutine (bvsbook_set) */


int bvsbook_get(BVSBOOK *bep,ushort *a)
{

	bep->al = (uchar) a[0] ;
	bep->ci = a[1] ;
	bep->nverses = a[2] ;
	bep->nzverses = a[3] ;
	return SR_OK ;
}
/* end subroutine (bvsbook_get) */


