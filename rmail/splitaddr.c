/* splitaddr */

/* splitaddr mail address management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1889 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These object manages a mail splitaddr.

	Strategy with memory allocation:

	We just have a static storage area (in the object) that is large enough
	to hold any valid mail address.  We copy the address we are splitting
	into this storage and then split it in place from there.  There is
	already too much (and superfluous) memory allocation going on in the
	world today! :-)

	Matching:

	We do "prefix" matching.

	list		candidate	match
	----------------------------------------------------

	b.a		a		N
	b.a		b.a		Y
	b.a		c.b.a		Y
	c.b.a		b.a		N
	c.b.a		d.b.a		N
	c.b.a		c.b.a		Y
	d@c.b.a		c.b.a		N
	d@c.b.a		y@c.b.a		N
	d@c.b.a		d@c.b.a		Y
	e@c.b.a		e@d.c.b.a	Y


*******************************************************************************/


#define	SPLITADDR_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<netdb.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<localmisc.h>

#include	"address.h"
#include	"splitaddr.h"


/* local defines */

#define	SPLITADDR_DEFENTS	4

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX(MAILADDRLEN,2048)
#endif


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnrchr(cchar *,int,int) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;


/* forward references */


/* local variables */


/* exported subroutines */


int splitaddr_start(SPLITADDR *op,cchar ap[])
{
	const int	nents = SPLITADDR_DEFENTS ;
	int		rs ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (ap == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("splitaddr_start: ent a=%s\n",ap) ;
#endif

	memset(op,0,sizeof(SPLITADDR)) ;

	if ((rs = vechand_start(&op->coms,nents,0)) >= 0) {
	    int		al = strlen(ap) ;
	    cchar	*tp ;
	    char	*bp ;
	    while (al && (ap[al-1] == '.')) {
		al -= 1 ;
	    }
	    if ((rs = uc_malloc((al+1),&bp)) >= 0) {
	        int	bl = al ;
	        int	f = FALSE ;

	        op->mailaddr = (cchar *) bp ;
		strwcpy(bp,ap,al) ;

	        while ((tp = strnrpbrk(bp,bl,".@")) != NULL) {

	            f = (*tp == '@') ;
	            n += 1 ;
	            rs = vechand_add(&op->coms,(tp+1)) ;
	            bl = (tp - bp) ;
	            bp[bl] = '\0' ;

	            if (f) break ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && (bp[0] != '\0')) {

	            if (! f) {
	                n += 1 ;
	                rs = vechand_add(&op->coms,bp) ;
	            } else
	                op->local = bp ;

	        } /* end if */

	        if (rs >= 0) op->nd = n ;

	    } /* end if */
	    if (rs < 0) {
	        vechand_finish(&op->coms) ;
	    }
	} /* end if */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (splitaddr_start) */


int splitaddr_finish(SPLITADDR *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("splitaddr_finish: ent a=%s\n",op->mailaddr) ;
#endif

	if (op->mailaddr != NULL) {
	    rs1 = uc_free(op->mailaddr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mailaddr = NULL ;
	}

	rs1 = vechand_finish(&op->coms) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (splitaddr_finish) */


/* return the count of the number of items in this list */
int splitaddr_count(SPLITADDR *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = vechand_count(&op->coms) ;

	return rs ;
}
/* end subroutine (splitaddr_count) */


int splitaddr_prematch(SPLITADDR *op,SPLITADDR *tp)
{
	int		rs = SR_OK ;
	int		rs1, rs2 ;
	int		i = 0 ;
	int		f_so = FALSE ;
	int		f = FALSE ;
	cchar		*cp1, *cp2 ;

	if (op == NULL) return SR_FAULT ;

	if (tp == NULL) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("splitaddr_prematch: testaddr=%s\n",tp->mailaddr) ;
#endif

/* CONSTCOND */
	while (TRUE) {

	    rs1 = vechand_get(&op->coms,i,&cp1) ;

#if	CF_DEBUGS
	    debugprintf("splitaddr_prematch: rs1=%d\n",rs1) ;
#endif

	    if (rs1 < 0)
	        break ;

	    f_so = FALSE ;
	    rs2 = vechand_get(&tp->coms,i,&cp2) ;

#if	CF_DEBUGS
	    debugprintf("splitaddr_prematch: rs2=%d\n",rs2) ;
#endif

	    if (rs2 < 0)
	        break ;

#if	CF_DEBUGS
	    debugprintf("splitaddr_prematch: cp1=%s\n",cp1) ;
	    debugprintf("splitaddr_prematch: cp2=%s\n",cp2) ;
#endif

	    f_so = TRUE ;
	    f = (strcasecmp(cp1,cp2) == 0) ;

#if	CF_DEBUGS
	    debugprintf("splitaddr_prematch: strcmp() f=%d\n",f) ;
#endif

	    if (! f)
	        break ;

	    i += 1 ;
	} /* end while */

#if	CF_DEBUGS
	debugprintf("splitaddr_prematch: for-out rs1=%d rs2=%d\n",rs1,rs2) ;
	debugprintf("splitaddr_prematch: f=%u f_so=%u\n",f,f_so) ;
#endif

/* handle non-matching related errors first */

	if ((rs1 < 0) && (rs1 != SR_NOTFOUND))
	    rs = rs1 ;

	if ((rs >= 0) && (rs2 < 0) && (rs2 != SR_NOTFOUND))
	    rs = rs2 ;

/* candidate entry must be as long or longer than the list entry */

	if ((rs >= 0) && f)
	    f = f_so ;

/* candidate entry must have local-name match if list entry has local */

	if ((rs >= 0) && f && (op->local != NULL)) {

#if	CF_DEBUGS
	    debugprintf("splitaddr_prematch: list has local\n") ;
#endif

	    f = FALSE ;
	    if (tp->local != NULL)
	        f = (strcmp(op->local,tp->local) == 0) ;

#if	CF_DEBUGS
	    debugprintf("splitaddr_prematch: local match f=%u\n",f) ;
#endif

	} /* end if */

#if	CF_DEBUGS
	debugprintf("splitaddr_prematch: ret rs=%d f=%d\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (splitaddr_prematch) */


int splitaddr_audit(SPLITADDR *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = vechand_audit(&op->coms) ;

	return rs ;
}
/* end subroutine (splitaddr_audit) */


