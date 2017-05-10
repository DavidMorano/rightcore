/* vecobj_recip */

/* extend the VECOBJ object w/ some recipient handling */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This code module was completely rewritten to replace the previous
	mail-delivery program for PCS, written around 1990 or so and which also
	served for a time to filter environment for the SENDMAIL daemon.

	= 2004-02-17, David A­D­ Morano
	This was modified to add the MSGID object.  That is a database that
	stores message IDs.  We used it to eliminate duplicate mail deliveries
	which as of late are coming from several popular sources!

*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This set of subroutines extends the VECOBJ object (yes we are using
	that to hold "recipients") for some "recipient" handling.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"recip.h"


/* local defines */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	headkeymat(const char *,const char *,int) ;

extern char	*strdcpy1w(char *,int,const char *,int) ;
extern char	*strdcpy3(char *,int,const char *,const char *,const char *) ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */


/* exported subroutines */


/* add a recipient to the recipient list */
int vecobj_recipadd(vecobj *op,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("vecobj_recipadd: name=%t\n",np,nl) ;
#endif

	if (nl < 0) nl = strlen(np) ;

	if (nl > 0) {
	    RECIP	*rp ;
	    int		i ;
	    int		f ;

	for (i = 0 ; (rs = vecobj_get(op,i,&rp)) >= 0 ; i += 1) {
	    if (rp != NULL) {
	        f = recip_match(rp,np,nl) ;
	        if (f) break ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("vecobj_recipadd: end search rs=%d\n",rs) ;
#endif

	if (rs == SR_NOTFOUND) {
	    RECIP	re ;

	    if ((rs = recip_start(&re,np,nl)) >= 0) {
		c = 1 ;
	        rs = vecobj_add(op,&re) ;
	        if (rs < 0)
	            recip_finish(&re) ;
	    } /* end if (new recip) */

	} /* end if (new recipient) */

	} /* end if (positive) */

#if	CF_DEBUGS
	debugprintf("vecobj_recipadd: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecobj_recipadd) */


/* free up recipients */
int vecobj_recipfins(vecobj *op)
{
	RECIP		*rp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; vecobj_get(op,i,&rp) >= 0 ; i += 1) {
	    if (rp != NULL) {
	        rs1 = recip_finish(rp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (vecobj_recipfins) */


