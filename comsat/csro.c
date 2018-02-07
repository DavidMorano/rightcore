/* csro (ComSat Receive Offset) */

/* email ComSat Receive Offset (CSRO) processing */


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
        email addresses belonging to a particular host (maybe this wasn not an
        imperative but that is what we did). The host parts are stored in a
        simple VECSTR container object and all email addresess are registered in
        a VECOBJ container object.


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

#define	VALUE		CSRO_VALUE


/* external subroutines */

extern char	*strwcpy(char *,cchar *,int) ;


/* forward references */

int		csro_already(CSRO *,cchar *,cchar *,ULONG) ;

static int	value_start(VALUE *,cchar *,cchar *,ULONG) ;
static int	value_finish(VALUE *) ;

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

	for (i = 0 ; vecobj_get(&op->entries,i,&vp) >= 0 ; i += 1) {
	    if (vp != NULL) {
		rs1 = entry_finish(vp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end while */

	rs1 = vecobj_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&op->names) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (csro_finish) */


int csro_add(CSRO *op,cchar *mailname,cchar *fname,ULONG mailoff)
{
	CSRO_VALUE	ve ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (mailname == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("csro_add: ent\n") ;
#endif

/* do we already have this name */

	if ((rs = value_start(&ve,mailname,fname,mailoff)) >= 0) {
	    VECSTR	*nlp = &op->names ;
	    VECOBJ	*vlp = &op->entries ;
	    const int	nlen = strlen(mailname) ;
	    int		f_release = FALSE ;
	    if ((rs = vecstr_findn(nlp,mailname,nlen)) >= 0) {
	        if ((rs = vecobj_search(vlp,&ve,vcmpentry,NULL)) >= 0) {
		     f_release = TRUE ;
		}
	    } else if (rs == SR_NOENT) {
		cchar	*np ;
	        if ((rs = vecstr_add(nlp,mailname,nlen)) >= 0) {
	            int		ni = rs ;
	            if ((rs = vecstr_get(nlp,ni,&np)) >= 0) {
			rs = vecobj_add(vlp,&ve) ;
		    }
		}
	    } /* end if (getting existing name) */
	    if (f_release) {
		rs1 = value_finish(&ve) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end if (value) */

#if	CF_DEBUGS
	debugprintf("csro_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (csro_add) */


/* do we already have an entry like this? */
int csro_already(CSRO *op,cchar *mailname,cchar *fname,ULONG mailoff)
{
	CSRO_VALUE	ve ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (mailname == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("csro_already: ent\n") ;
#endif

	if ((rs = value_start(&ve,mailname,fname,mailoff)) >= 0) {
	    const int	rsn = SR_NOTFOUND ;
	    if ((rs = vecobj_search(&op->entries,&ve,vcmpentry,NULL)) >= 0) {
	        f = TRUE ;
	    } else if (rs == rsn) {
	        rs = SR_OK ;
	    }
	    rs1 = value_finish(&ve) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (value) */

	return (rs >= 0) ? f : rs ;
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

	if (op == NULL) return SR_FAULT ;
	if (vcp == NULL) return SR_FAULT ;

	if (op->magic != CSRO_MAGIC) return SR_NOTOPEN ;

	*vcp = -1 ;
	return SR_OK ;
}
/* ens subroutine (csro_vcurend) */


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


/* private subroutines */


static int value_start(VALUE *ep,cchar *mailname,cchar *fname,ULONG mailoff)
{
	int		rs ;
	int		size = 1 ;
	char		*bp ;
	memset(ep,0,sizeof(VALUE)) ;
	ep->mailoff = mailoff ;
	size += (strlen(mailname)+1) ;
	if (fname != NULL) {
	   size += (strlen(fname)+1) ;
	}
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    ep->mailname = bp ;
	    bp = (strwcpy(bp,fname,-1)+1) ;
	    ep->fname = bp ;
	    if (fname != NULL) {
	        bp = (strwcpy(bp,fname,-1)+1) ;
	    } else {
		*bp = '\0' ;
	    }
	} /* end if (m-a) */
	return rs ;
}
/* end subroutine (value_start) */


static int value_finish(VALUE *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = uc_free(ep->mailname) ;
	if (rs >= 0) rs = rs1 ;
	ep->mailname = NULL ;
	return rs ;
}
/* end subroutine (value_finish) */


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


