/* csro */

/* email CSRO processing */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object processes and manipulates email CSRO addresses.

	Implementation note:

        Since email addresses can share the same host part, and further since we
        want to be able to group email addresses by their host parts, we store
        the host part of all email addresses in a separate structure than the
        local part. All email addresses will be hashed with the index being the
        host part of the address. This allows super quick retrival of those
        email addresses belonging to a particular host (maybe this wasn't an
        imperative but that is what we did). The host parts are stored in a
        simple VECSTR container object and all email addresess are registered in
        a HDB container object.


*******************************************************************************/


#define	CSRO_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<vecstr.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"csro.h"


/* local defines */


/* external subroutines */


/* forward references */

int		csro_already(CSRO *,const char *,const char *,ULONG) ;

static int	vcmpname() ;
static int	vcmpentry() ;


/* local variables */


/* exported subroutines */


int csro_start(CSRO *op,int n)
{
	int		rs ;
	int		opts ;
	int		size ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(CSRO)) ;

	if (n <= 1)
	    n = CSRO_DEFENTS ;

#if	CF_DEBUGS
	debugprintf("csro_start: ent n=%d\n",n) ;
#endif

	opts = 0 ;
	size = sizeof(CSRO_VALUE) ;
	if ((rs = vecobj_start(&op->entries,size,n,opts)) >= 0) {
	    opts = VECSTR_OCONSERVE ;
	    if ((rs = vecstr_start(&op->names,n,opts)) >= 0) {
	        op->magic = CSRO_MAGIC ;
	    }
	    if (rs < 0)
	        vecobj_finish(&op->entries) ;
	}

#if	CF_DEBUGS
	debugprintf("csro_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (csro_start) */


int csro_finish(CSRO *op)
{
	CSRO_VALUE	*vp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

/* pop entries first */

	for (i = 0 ; vecobj_get(&op->entries,i,&vp) >= 0 ; i += 1) {
	    if (vp != NULL) {
	        if (vp->fname != NULL) {
	            rs1 = uc_free(vp->fname) ;
	            if (rs >= 0) rs = rs1 ;
	            vp->fname = NULL ;
	        }
	    }
	} /* end while */

	rs1 = vecobj_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

/* pop the vector of strings */

	rs1 = vecstr_finish(&op->names) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (csro_finish) */


int csro_add(CSRO *op,cchar *mailname,cchar *fname,ULONG mailoff)
{
	CSRO_VALUE	ve ;
	int		rs, rs1 ;
	int		ni, nlen ;
	const char	*np ;

	if (op == NULL) return SR_FAULT ;
	if (mailname == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("csro_add: continuing\n") ;
#endif

	nlen = strlen(mailname) ;

/* do we already have this name */

	if ((rs = vecstr_findn(&op->names,mailname,nlen)) >= 0) {
	    ni = rs ;

	    memset(&ve,0,sizeof(CSRO_VALUE)) ;
	    ve.mailname = mailname ;
	    ve.fname = fname ;
	    ve.mailoff = mailoff ;

	    rs1 = vecobj_search(&op->entries,&ve,vcmpentry,NULL) ;

	    if (rs1 >= 0)
	        goto ret0 ;

	} else if (rs == SR_NOENT) {

#if	CF_DEBUGS
	    debugprintf("csro_add: host not already added\n") ;
	    debugprintf("csro_add: host=%s\n",host) ;
#endif

	    rs = vecstr_add(&op->names,mailname,nlen) ;
	    ni = rs ;

#if	CF_DEBUGS
	    debugprintf("csro_add: host NAA rs=%d\n",rs) ;
#endif

	} /* end if (getting existing name) */

	if (rs < 0)
	    goto bad0 ;

	rs = vecstr_get(&op->names,ni,&np) ;
	if (rs < 0)
	    goto bad0 ;

#if	CF_DEBUGS
	debugprintf("csro_add: allocating\n") ;
#endif

	memset(&ve,0,sizeof(CSRO_VALUE)) ;

	ve.mailname = np ;
	ve.mailoff = mailoff ;
	if (fname != NULL) {
	    ve.fname = mallocstr(fname) ;
	    if (ve.fname == NULL) goto bad2 ;
	} else
	    ve.fname = NULL ;

	rs = vecobj_add(&op->entries,&ve) ;
	if (rs < 0)
	    goto bad3 ;

#if	CF_DEBUGS
	debugprintf("csro_add: ret rs=%d\n", rs) ;
#endif

ret0:
	return rs ;

/* bad things come here */
bad3:
	if (ve.fname != NULL) {
	    uc_free(ve.fname) ;
	    ve.fname = NULL ;
	}

bad2:
bad0:
	goto ret0 ;
}
/* end subroutine (csro_add) */


/* do we already have an entry like this? */
int csro_already(CSRO *op,cchar *mailname,cchar *fname,ULONG mailoff)
{
	CSRO_VALUE	ve ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (mailname == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("csro_already: ent\n") ;
#endif

/* is this entry already in the entry table? */

	memset(&ve,0,sizeof(CSRO_VALUE)) ;
	ve.mailname = mailname ;
	ve.fname = fname ;
	ve.mailoff = mailoff ;

	rs = vecobj_search(&op->entries,&ve,vcmpentry,NULL) ;

	return rs ;
}
/* end subroutine (csro_already) */


/* return the number of hosts seen so far */
int csro_countnames(CSRO *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_count(&op->names) ;

	return rs ;
}
/* end subroutine (csro_countnames) */


/* return the count of the number of items in this list */
int csro_count(CSRO *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = vecobj_count(&op->entries) ;

	return rs ;
}
/* end subroutine (csro_count) */


/* sort the strings in the vector list */
int csro_sort(CSRO *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	rs = vecobj_sort(&op->entries,vcmpname) ;

	return rs ;
}
/* end subroutine (csro_sort) */


int csro_getname(CSRO *op,CSRO_NCURSOR *hcp,cchar **hnpp)
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
/* end subroutine (csro_getname) */


/* fetch the next entry value that matches the given host name */
int csro_getvalue(CSRO *op,cchar *mailname,CSRO_VCURSOR *vcp,CSRO_VALUE **vepp)
{
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (mailname == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	i = (*vcp < 0) ? 0 : (*vcp + 1) ;

	while ((rs = vecobj_get(&op->entries,i,vepp)) >= 0) {
	    int	f = (*vepp != NULL) ;
	    f = f && (strcmp((*vepp)->mailname,mailname) == 0) ;
	    if (f) break ;
	    i += 1 ;
	} /* end while */

	if (rs >= 0)
	    *vcp = i ;

	return rs ;
}
/* end subroutine (csro_getvalue) */


/* initialize a host cursor */
int csro_ncurbegin(CSRO *op,CSRO_NCURSOR *hcp)
{

	if (op == NULL) return SR_FAULT ;
	if (hcp == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	*hcp = -1 ;
	return SR_OK ;
}
/* end subroutine (csro_ncurbegin) */


/* free up a host cursor */
int csro_ncurend(CSRO *op,CSRO_NCURSOR *hcp)
{

	if (op == NULL) return SR_FAULT ;
	if (hcp == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	*hcp = -1 ;
	return SR_OK ;
}
/* end subroutine (csro_ncurend) */


/* initialize a value cursor */
int csro_vcurbegin(CSRO *op,CSRO_VCURSOR *vcp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (vcp == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	*vcp = -1 ;
	return rs ;
}
/* end subroutine (csro_vcurbegin) */


/* free up a value cursor */
int csro_vcurend(CSRO *op,CSRO_VCURSOR *vcp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (vcp == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	*vcp = -1 ;
	return rs ;
}
/* ens subroutine (csro_vcurend) */


/* private subroutines */


static int vcmpname(CSRO_VALUE **e1p,CSRO_VALUE **e2p)
{
	int		rc = 0 ;
	if ((*e1p != NULL) || (*e2p != NULL)) {
	    if (*e1p != NULL) {
	        if (*e2p != NULL) {
	            if (rc == 0) {
		        cchar	*s1 = (*e1p)->mailname ;
		        cchar	*s2 = (*e2p)->mailname ;
	                if ((rc = strcmp(s1,s2)) == 0) {
	                    rc = ((*e1p)->mailoff - (*e2p)->mailoff) ;
	                }
	            }
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	} 
	return rc ;
}
/* end subroutine (vcmpname) */


static int vcmpentry(CSRO_VALUE **e1p,CSRO_VALUE **e2p)
{
	int		rc = 0 ;
	if ((*e1p != NULL) || (*e2p != NULL)) {
	    if (*e1p != NULL) {
	        if (*e2p != NULL) {
		    cchar	*s1 = (*e1p)->mailname ;
		    cchar	*s2 = (*e2p)->mailname ;
	            if ((rc = strcmp(s1,s2)) == 0) {
	                if ((rc = ((*e1p)->mailoff - (*e2p)->mailoff)) == 0) {
	                    if ((*e1p)->fname == NULL) {
	                        rc = 1 ;
	                    } else if ((*e1p)->fname == NULL) {
	                        rc = -1 ;
	                    } else {
	                        rc = strcmp((*e1p)->fname,(*e2p)->fname) ;
		            }
		        }
	            } /* end if */
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmpentry) */


