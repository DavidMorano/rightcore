/* mailmsgatt */

/* mail-message attachment processing */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This module was originally written.

	= 2001-01-03, David A­D­ Morano
        I changed the 'mailmsgattent_type()' subroutine slightly. I changed it
        so that when no content type if found, it will assume a binary content
        type rather than returning a SR_NOTFOUND error.

*/

/* Copyright © 1998,2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object processes and manipulates email mailmsgattment and address.


*******************************************************************************/


#define	MAILMSGATT_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecitem.h>
#include	<localmisc.h>

#include	"mailmsgatt.h"


/* local defines */

#define	MAILMSGATT_DEFENTS	10


/* external subroutines */


/* forward references */


/* local variables */


/* exported subroutines */


int mailmsgatt_start(MAILMSGATT *rhp)
{
	const int	n = MAILMSGATT_DEFENTS ;
	const int	opts = VECITEM_PNOHOLES ;
	int		rs ;

	rs = vecitem_start(rhp,n,opts) ;

	return rs ;
}
/* end subroutine (mailmsgatt_start) */


/* free up the mailmsgattments list object */
int mailmsgatt_finish(MAILMSGATT *rhp)
{
	MAILMSGATTENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecitem_get(rhp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = mailmsgattent_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecitem_finish(rhp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailmsgatt_finish) */


/* add an attachment (w/ default content-type and content-encoding) */
int mailmsgatt_add(MAILMSGATT *rhp,cchar *ct,cchar *ce,cchar *nbuf,int nlen)
{
	MAILMSGATTENT	ve ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: ent\n") ;
#endif

	if (rhp == NULL) return SR_FAULT ;
	if (rhp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: continuing\n") ;
#endif

	if (nlen < 0) nlen = strlen(nbuf) ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: nlen=%d\n",nlen) ;
#endif

	if ((rs = mailmsgattent_start(&ve,ct,ce,nbuf,nlen)) >= 0) {
	    const int	esize = sizeof(MAILMSGATTENT) ;
	    rs = vecitem_add(rhp,&ve,esize) ;
	    if (rs < 0)
	        mailmsgattent_finish(&ve) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("mailmsgatt_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsgatt_add) */


/* delete an mailmsgattment */
int mailmsgatt_del(MAILMSGATT *alp,int i)
{
	MAILMSGATTENT	*ep ;
	int		rs ;
	int		rs1 ;

	if (alp == NULL) return SR_FAULT ;

	if ((rs = vecitem_get(alp,i,&ep)) >= 0) {
	    if (ep != NULL) {
	        rs1 = mailmsgattent_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	    rs1 = vecitem_del(alp,i) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if */

	return rs ;
}
/* end subroutine (mailmsgatt_del) */


/* return the number of hosts seen so far */
int mailmsgatt_count(MAILMSGATT *rhp)
{
	int		rs ;

	if (rhp == NULL) return SR_FAULT ;

	rs = vecitem_count(rhp) ;

	return rs ;
}
/* end subroutine (mailmsgatt_count) */


/* enumerate */
int mailmsgatt_enum(MAILMSGATT *rhp,int i,MAILMSGATTENT **epp)
{
	int		rs ;

	rs = vecitem_get(rhp,i,epp) ;

	return rs ;
}
/* end subroutine (mailmsgatt_enum) */


/* find content types for all of the mailmsgattments using a MIME-types DB */
int mailmsgatt_typeatts(MAILMSGATT *rhp,MIMETYPES *mtp)
{
	MAILMSGATTENT	*ep ;
	int		rs = SR_OK ;
	int		i ;

	if (rhp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailmsgatt_typeatts: ent\n") ;
#endif

	for (i = 0 ; mailmsgatt_enum(rhp,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs = mailmsgattent_type(ep,mtp) ;
	        if (rs < 0) break ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("mailmsgatt_typeatts: exiting rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailmsgatt_typeatts) */


/* private subroutines */


