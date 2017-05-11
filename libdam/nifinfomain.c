/* nifinfo */

/* manage host network interface information */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1994-02-03, David A­D­ Morano

	This module was originally written.


*/

/* Copyright © 1994 David Morano.  All rights reserved. */

/*******************************************************************************

	These subroutines form the public calling interface part of the NIFINFO
	object.


*******************************************************************************/


#define	NIFINFO_MASTER		0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<net/if.h>
#include	<arpa/inet.h>
#include	<netdb.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"nifinfo.h"
#include	"ifaddrlist.h"


/* local defines */

#ifndef	ERRBUFLEN
#define	ERRBUFLEN	MAX(1024,IFADDRLIST_ERRBUFSIZE)
#endif

#define	TO_AGAIN	10

#define	IFADDRLIST	struct ifaddrlist


/* external subroutines */


/* forward references */


/* exported subroutines */


int nifinfo_start(NIFINFO *op)
{
	IFADDRLIST	*if1p, *if2p ;
	int		rs ;
	int		n1, n2 ;
	int		n = 0 ;
	int		to_again = 0 ;
	int		size ;
	char		errbuf[ERRBUFLEN + 1] ;
	void		*p ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(NIFINFO)) ;

	if1p = NULL ;
	n1 = ifaddrlist(&if1p,AF_INET4,errbuf) ;

	if2p = NULL ;
	n2 = ifaddrlist(&if2p,AF_INET6,errbuf) ;

	if (n1 > 0)
	    n += n1 ;

	if (n2 > 0)
	    n += n2 ;

	size = n * sizeof(NIFINFO_ENT) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    int		i, j ;
	    op->a = p ;

	    memset(op->a,0,size) ;

	    j = 0 ;
	    size = 4 ;
	    for (i = 0 ; i < n1 ; i += 1) {

	        op->a[j].af = AF_INET4 ;
	        op->a[j].alen = size ;
	        op->a[j].index = if1p[i].index ;
	        op->a[j].flags = if1p[i].flags ;
	        strcpy(op->a[j].inter,if1p[i].device) ;

	        memcpy(&op->a[j].addr,&if1p[i].addr,size) ;

	        j += 1 ;

	    } /* end for */

	    size = 16 ;
	    for (i = 0 ; i < n2 ; i += 1) {

	        op->a[j].af = AF_INET6 ;
	        op->a[j].alen = size ;
	        op->a[j].index = if2p[i].index ;
	        op->a[j].flags = if2p[i].flags ;
	        strcpy(op->a[j].inter,if2p[i].device) ;

	        memcpy(&op->a[j].addr,&if2p[i].addr,size) ;

	        j += 1 ;

	    } /* end for */

	    op->n = n ;
	    op->magic = NIFINFO_MAGIC ;

	} /* end if */

	if ((n2 >= 0) && (if2p != NULL))
	    uc_free(if2p) ;

	if ((n1 >= 0) && (if1p != NULL))
	    uc_free(if1p) ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (nifinfo_start) */


#ifdef	COMMENT

int nifinfo_curbegin(op,curp)
NIFINFO		*op ;
NIFINFO_CUR	*curp ;
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != NIFINFO_MAGIC) return SR_NOTOPEN ;

	curp->aip = NULL ;
	return SR_OK ;
}
/* end subroutine (nifinfo_curbegin) */


int nifinfo_curend(op,curp)
NIFINFO		*op ;
NIFINFO_CUR	*curp ;
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != NIFINFO_MAGIC) return SR_NOTOPEN ;

	curp->aip = NULL ;
	return SR_OK ;
}
/* end subroutine (nifinfo_curend) */

#endif /* COMMENT */


int nifinfo_get(NIFINFO *op,int i,NIFINFO_ENT **rpp)
{
	int		rs = SR_NOTFOUND ;

	if (op == NULL) return SR_FAULT ;
	if (rpp == NULL) return SR_FAULT ;

	if (op->magic != NIFINFO_MAGIC) return SR_NOTOPEN ;

	if ((i >= 0) && (i < op->n)) {
	    *rpp = (op->a + i) ;
	    rs = SR_OK ;
	}

	return rs ;
}
/* end subroutine (nifinfo_get) */


int nifinfo_match(NIFINFO *op,int af,void *a,int alen)
{
	int		rs ;
	int		rc ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (a == NULL) return SR_FAULT ;

	if (op->magic != NIFINFO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("nifinfo_match: af=%u\n",af) ;
#endif

	for (i = 0 ; i < op->n ; i += 1) {

#if	CF_DEBUGS
	debugprintf("nifinfo_match: index=%u\n",op->a[i].index) ;
#endif

		if (((af == 0) || (op->a[i].af == af)) &&
			((alen < 0) || (op->a[i].alen == alen))) {

			rc = memcmp(&op->a[i].addr,a,op->a[i].alen) ;
			if (rc == 0) break ;

		} /* end if */

	} /* end for */

	rs = (i < op->n) ? i : SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("nifinfo_match: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (nifinfo_match) */


int nifinfo_finish(NIFINFO *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != NIFINFO_MAGIC) return SR_NOTOPEN ;

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (nifinfo_finish) */


