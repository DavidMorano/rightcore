/* vecsorthand */

/* vector of sorted handles */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2010-12-01,  David A.D. Morano
	Module was originally written.

*/

/* Copyright © 2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines are used when the caller just wants to store an item in
	a sorted list.  The item us usually just a pointer to the user's real
	data.


*******************************************************************************/


#define	VECSORTHAND_MASTER		1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecsorthand.h"


/* local defines */

#define	VECSORTHAND_DEFENTS	10


/* external subroutines */


/* forward references */

static int	vecsorthand_extend(vecsorthand *op)

static int	defcmpfunc(const void *,const void *) ;


/* exported subroutines */


int vecsorthand_start(vecsorthand *svp,int n,int (*cmpfunc)())
{
	int		rs ;
	int		size = 0 ;
	void		*p ;

	if (svp == NULL) return SR_FAULT ;

	if (n <= 1) n = VECSORTHAND_DEFENTS ;

	memset(svp,0,sizeof(vecsorthand)) ;
	svp->i = 0 ;
	svp->c = 0 ;

	size += (sizeof(void **) * (n+1)) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    svp->va = p ;
	    svp->e = n ;
	    {
		svp->va[0] = NULL ;
		svp->cmpfunc = (cmpfunc != NULL) ? cmpfunc : defcmpfunc ;
	    }
	}
	return rs ;
}
/* end subroutine (vecsorthand_start) */


int vecsorthand_finish(vecsorthand *svp)
{

	if (svp == NULL) return SR_FAULT ;

	if (svp->va != NULL) {
	    int	i ;
	    for (i = 0 ; i < svp->i ; i += 1) {
	        if ((svp->va)[i] != NULL)
	            free((svp->va)[i]) ;
	    } /* end while */
	    free(svp->va) ;
	    svp->va = NULL ;
	}

	svp->i = 0 ;
	svp->e = 0 ;
	return SR_OK ;
}
/* end subroutine (vecsorthand_finish) */


/* add an entry to this sorted list */
int vecsorthand_add(vecsorthand *svp,void *buf)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("vecsorthandadd: ent\n") ;
#endif

	if (svp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("vecsorthandadd: ent, i=%d\n",svp->i) ;
#endif

	if ((rs = vecsorthand_extend(op)) >= 0) {
	int		bot, top, i, j ;

/* do the regular thing */

	svp->c += 1 ;			/* increment list count */

/* find the place in the existing list where this new item should be added */

	bot = 0 ;
	top = MAX((svp->i - 1),0) ;
	i = (bot + top) / 2 ;

#if	CF_DEBUGS
	debugprintf("vecsorthandadd: bot=%d top=%d i=%d\n",bot,top,i) ;
#endif

	while ((top - bot) > 0) {

	    if ((rs = (*svp->cmpfunc)(&buf,(svp->va+i))) < 0) {
	        top = i - 1 ;
	    } else if (rs > 0) {
	        bot = i + 1 ;
	    } else
	        break ;

	    i = (bot + top) / 2 ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("vecsorthandadd: found bot=%d top=%d i=%d\n",bot,top,i) ;
#endif

	if (i < svp->i) {

	    if ((*svp->cmpfunc)(&buf,&svp->va[i]) > 0)
	        i += 1 ;

/* move all entries from "i" through "svp->i - 1" down one */

	    for (j = (svp->i - 1) ; j >= i ; j -= 1) {
	        (svp->va)[j + 1] = (svp->va)[j] ;
	    }

	} /* end if */

/* load the new entry */

	(svp->va)[i] = buf ;
	svp->i += 1 ;
	(svp->va)[svp->i] = NULL ;

	} /* end if (vecsorthand_extend) */

	return i ;
}
/* end subroutine (vecsorthand_add) */


/* get an entry (enumerated) from the vector list */
int vecsorthand_get(vecsorthand *svp,int i,void **rpp)
{

	if (svp == NULL) return SR_FAULT ;

	if (rpp != NULL) *rpp = NULL ;

	if ((i < 0) || (i >= svp->i)) return SR_NOTFOUND ;

	if (svp->va == NULL) return SR_NOTFOUND ;

	if (rpp != NULL) {
	    *rpp = (svp->va)[i] ;
	}

	return SR_OK ;
}
/* end subroutine (vecsorthand_get) */


/* delete an entry */
int vecsorthand_del(vecsorthand *svp,int i)
{
	int		j ;

	if (svp == NULL) return SR_FAULT ;

	if ((i < 0) || (i >= svp->i)) return SR_NOTFOUND ;

	if (svp->va == NULL) return SR_NOTFOUND ;

	svp->i -= 1 ;
	for (j = i ; j < svp->i ; j += 1) {
	    (svp->va)[j] = (svp->va)[j + 1] ;
	}

	(svp->va)[svp->i] = NULL ;

	svp->c -= 1 ;		/* decrement list count */
	return SR_OK ;
}
/* end subroutine (vecsorthand_del) */


/* return the count of the number of items in this list */
int vecsorthand_count(vecsorthand *svp)
{

	if (svp == NULL) return SR_FAULT ;

	return svp->c ;
}
/* end subroutine (vecsorthand_count) */


/* search for an entry in the sorted vector list */
int vecsorthand_search(svp,ep,rpp)
vecsorthand		*svp ;
void		*ep ;
void		**rpp ;
{
	const int	esize = sizeof(void *) ;
	int		rs ;
	int		i ;
	void		*ep2 ;
	void		**rpp2 ;

	if (svp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;

	if (svp->va == NULL) return SR_NOTFOUND ;

	rpp2 = (void **) bsearch(&ep,svp->va,svp->i,esize,svp->cmpfunc) ;

	rs = SR_NOTFOUND ;
	if (rpp2 != NULL) {
	    i = (rpp2 - svp->va) ;
	    rs = SR_OK ;
	}

	if (rpp != NULL) {
		*rpp = ((rs >= 0) ? svp->va[i] : NULL) ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecsorthand_search) */


/* private subroutines */


static int vecsorthand_extend(vecsorthand *op)
{
	int		rs = SR_OK ;
	if ((svp->i + 1) > svp->e) {
	    const int	ndef = VECSORTHAND_DEFENTS ;
	    int		size ;
	    int		ne ;
	    cchar	*ep = NULL ;
	    if (svp->e == 0) {
	 	ne = ndef ;
		size = (ndef*sizeof(char **)) ;
		rs = uc_malloc(size,&np) ;
	    } else {
	        const int	ne = (svp->e * 2) ;
	        size = (ne*sizeof(cchar **)) ;
		rs = uc_realloc(svp->va,size,&np) ;
	    }
	    if (rs >= 0) {
	        svp->va = ep ;
	        svp->e = ne ;
	    }
	} /* end if */
	return rs ;
}
/* end subroutine (vecsorthand_extend) */


static int defcmpfunc(const void *app,const void *bpp)
{
	cchar	**e1pp = (cchar **) app ;
	cchar	**e2pp = (cchar **) bpp ;
	return strcmp(*e1pp,*e2pp) ;
}
/* end subroutine (defcmpfunc) */


