/* recipient */

/* email recipient processing */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object processes and manipulates email recipient addresses.

	Implementation note:

        Since email addresses can share the same host part, and further since we
        want to be able to group email addresses by their host parts, we store
        the host part of all email addresses in a separate structure then the
        local part. All email addresses will be hashed with the index being the
        host part of the address. This allows super quick retrival of those
        email addresses belonging to a particular host (maybe this wasn't an
        imperative but that is what we did). The host parts are stored in a
        simple VECSTR container object and all email addresess are stored in a
        HDB container object.


*******************************************************************************/


#define	RECIPIENT_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"recipient.h"


/* local defines */

#define	RECIPIENT_DEFENTS	10


/* external subroutines */

extern char	*strwcpylc(char *,const char *,int) ;


/* local structures */


/* forward references */


/* exported subroutines */


int recipient_start(RECIPIENT *op,int n)
{
	int		rs ;
	int		opts ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(RECIPIENT)) ;

	if (n <= 1)
	    n = RECIPIENT_DEFENTS ;

#if	CF_DEBUGS
	debugprintf("recipient_start: ent n=%d\n",n) ;
#endif

	if ((rs = hdb_start(&op->hash,n,0,NULL,NULL)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("recipient_start: hash inited rs=%d\n",rs) ;
#endif

	    opts = VECSTR_OCONSERVE ;
	    rs = vecstr_start(&op->names,n,opts) ;
	    if (rs < 0)
	        hdb_finish(&op->hash) ;

	} /* end if */

#if	CF_DEBUGS
	debugprintf("recipient_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (recipient_start) */


/* add a recipient to the database */
int recipient_add(op,host,local,type)
RECIPIENT	*op ;
const char	host[] ;
const char	local[] ;
int		type ;
{
	HDB_DATUM	key, val ;
	RECIPIENT_VAL	*vp = NULL ;
	RECIPIENT_VAL	ve ;
	int		rs ;
	int		hlen ;
	int		size ;
	int		vidx = -1 ;
	const char	*cp ;
	char		hostbuf[MAXHOSTNAMELEN + 1] ;
	char		*p ;

	if (op == NULL) return SR_FAULT ;
	if (local == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("recipient_add: cont\n") ;
#endif

	if ((host == NULL) || (type < 0))
	    host = RECIPIENT_NOHOST ;

	hlen = strwcpylc(hostbuf,host,MAXHOSTNAMELEN) - hostbuf ;
	host = hostbuf ;

/* is this host in the hash table? */

	key.buf = host ;
	key.len = hlen ;
	if ((rs = hdb_fetch(&op->hash,key,NULL,&val)) >= 0) {
	    vp = (RECIPIENT_VAL *) val.buf ;
	    key.buf = vp->hostpart ;
	    key.len = -1 ;
	} else if (rs == SR_NOENT) {
	    if ((rs = vecstr_add(&op->names,host,hlen)) >= 0) {
	        vidx = rs ;
	        vecstr_get(&op->names,rs,&cp) ;
	        key.buf = cp ;
	        key.len = -1 ;
	    }
	} /* end if */

#if	CF_DEBUGS
	debugprintf("recipient_add: allocating\n") ;
#endif

	if (rs >= 0) {
	    if ((rs = uc_mallocstrw(local,-1,&cp)) >= 0) {
	        size = sizeof(RECIPIENT_VAL) ;
	        memset(&ve,0,size) ;
	        ve.type = type ;
	        ve.a = NULL ;
	        ve.hostpart = (cchar *) key.buf ;
	        ve.localpart = cp ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            vp = (RECIPIENT_VAL *) p ;
	            val.buf = (char *) vp ;
	            val.len = size ;
	            rs = hdb_store(&op->hash,key,val) ;
	            if (rs < 0) {
	                uc_free(vp) ;
	            }
	        } /* end if (m-a) */
	        if (rs < 0) {
	            uc_free(ve.localpart) ;
	        }
	    } /* end if (m-a) */
	    if ((rs < 0) && (vidx >= 0)) {
	        vecstr_del(&op->names,vidx) ;
	    }
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("recipient_add: ret key=%t rs=%d\n",
	    key.buf,key.len,rs) ;
#endif

	return rs ;
}
/* end subroutine (recipient_add) */


/* fetch on a address tripple */
int recipient_already(op,host,local,type)
RECIPIENT	*op ;
const char	host[] ;
const char	local[] ;
int		type ;
{
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	RECIPIENT_VAL	*vp ;
	int		rs ;
	int		hlen ;
	int		f ;
	char		hostbuf[MAXHOSTNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (local == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("recipient_already: cont\n") ;
#endif

	if ((host == NULL) || (type < 0))
	    host = RECIPIENT_NOHOST ;

	hlen = strwcpylc(hostbuf,host,MAXHOSTNAMELEN) - hostbuf ;
	host = hostbuf ;

/* is this host in the hash table? */

	key.buf = host ;
	key.len = hlen ;

	if ((rs = hdb_curbegin(&op->hash,&cur)) >= 0) {

	    while ((rs = hdb_fetch(&op->hash,key,&cur,&val)) >= 0) {

	        vp = (RECIPIENT_VAL *) val.buf ;
	        f = TRUE ;
	        f = f && (type == vp->type) ;
	        f = f && (strcmp(local,vp->localpart) == 0) ;
	        if (f) break ;

	    } /* end while */

	    hdb_curend(&op->hash,&cur) ;
	} /* end if (curosr) */

	return rs ;
}
/* end subroutine (recipient_already) */


/* free up the entire vector string data structure object */
int recipient_finish(op)
RECIPIENT	*op ;
{
	HDB_CUR		keycursor ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

/* pop the hash table first */

	if ((rs = hdb_curbegin(&op->hash,&keycursor)) >= 0) {
	    RECIPIENT_VAL	*vp ;

	    while (hdb_enum(&op->hash,&keycursor,&key,&val) >= 0) {

	        if (val.buf != NULL) {

	            vp = (RECIPIENT_VAL *) val.buf ;

	            if (vp->a != NULL) {
	                uc_free(vp->a) ;
	                vp->a = NULL ;
	            }

	            if (vp->localpart != NULL) {
	                uc_free(vp->localpart) ;
	                vp->localpart = NULL ;
	            }

	            uc_free(vp) ;
	        } /* end if */

	    } /* end while */

	    hdb_curend(&op->hash,&keycursor) ;
	} /* end if (cursor) */

	rs1 = hdb_finish(&op->hash) ;
	if (rs >= 0) rs = rs1 ;

/* pop the vector of strings */

	rs1 = vecstr_finish(&op->names) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (recipient_finish) */


/* return the number of hosts seen so far */
int recipient_counthosts(op)
RECIPIENT	*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = vecstr_count(&op->names) ;

	return rs ;
}
/* end subroutine (recipient_counthosts) */


/* return the count of the number of items in this list */
int recipient_count(op)
RECIPIENT	*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = hdb_count(&op->hash) ;

	return rs ;
}
/* end subroutine (recipient_count) */


#ifdef	COMMENT

/* sort the strings in the vector list */
int recipient_sort(vhp)
RECIPIENT	*vhp ;
{

	if (vhp == NULL) return SR_FAULT ;

	if (vhp->va == NULL) return SR_OK ;

	if (vhp->c > 1)
	    qsort(vhp->va,(size_t) vhp->i,sizeof(char *),ourcmp) ;

	return vhp->i ;
}
/* end subroutine (recipient_sort) */

#endif /* COMMENT */


int recipient_enumhost(op,hcp,hnpp)
RECIPIENT	*op ;
RECIPIENT_HCUR	*hcp ;
const char	**hnpp ;
{
	int		rs ;
	int		i ;
	const char	*dump ;

	if (op == NULL) return SR_FAULT ;
	if (hcp == NULL) return SR_FAULT ;

	if (hnpp == NULL)
	    hnpp = &dump ;

	i = (*hcp >= 0) ? (*hcp + 1) : 0 ;

	while ((rs = vecstr_get(&op->names,i,hnpp)) >= 0) {
	    if (*hnpp != NULL) break ;
	    i += 1 ;
	} /* end while */

	*hcp = (rs >= 0) ? i : -1 ;
	return rs ;
}
/* end subroutine (recipient_enumhost) */


/* fetch the next entry value that matches the given host name */
int recipient_fetchvalue(op,host,vcp,vepp)
RECIPIENT	*op ;
const char	host[] ;
RECIPIENT_VCUR	*vcp ;
RECIPIENT_VAL	**vepp ;
{
	HDB_DATUM	key, val ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	key.buf = host ;
	key.len = -1 ;
	if ((rs = hdb_fetch(&op->hash,key,vcp,&val)) >= 0) {
	    if (vepp != NULL) {
	        *vepp = (RECIPIENT_VAL *) val.buf ;
	    }
	    rs = val.len ;
	}

	return rs ;
}
/* end subroutine (recipient_fetchvalue) */


/* initialize a host cursor */
int recipient_hcurbegin(op,hcp)
RECIPIENT	*op ;
RECIPIENT_HCUR	*hcp ;
{

	if (op == NULL) return SR_FAULT ;
	if (hcp == NULL) return SR_FAULT ;

	*hcp = -1 ;
	return SR_OK ;
}
/* end subroutine (recipient_hcurbegin) */


/* free up a host cursor */
int recipient_hcurend(op,hcp)
RECIPIENT	*op ;
RECIPIENT_HCUR	*hcp ;
{

	if (op == NULL) return SR_FAULT ;
	if (hcp == NULL) return SR_FAULT ;

	*hcp = -1 ;
	return SR_OK ;
}
/* end subroutine (recipient_hcurend) */


/* initialize a value cursor */
int recipient_vcurbegin(op,vcp)
RECIPIENT	*op ;
RECIPIENT_VCUR	*vcp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (vcp == NULL) return SR_FAULT ;

	rs = hdb_curbegin(&op->hash,vcp) ;

	return rs ;
}
/* end subroutine (recipient_vcurbegin) */


/* free up a value cursor */
int recipient_vcurend(op,vcp)
RECIPIENT	*op ;
RECIPIENT_VCUR	*vcp ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (vcp == NULL) return SR_FAULT ;

	rs = hdb_curend(&op->hash,vcp) ;

	return rs ;
}
/* ens subroutine (recipient_vcurend) */


