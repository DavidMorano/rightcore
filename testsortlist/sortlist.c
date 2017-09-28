/* sortlist */

/* sorted list operations */


#define	CF_DEBUGS	1		/* compile-time debugging */


/* revision history:

	 = 1999-12-01, David A.D. Morano, 
	Module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These routines are used when the caller just wants to store an item in a
        sorted list. The item us usually just a pointer to the user's real data.
        Items have keys and values, like with a hash table structure except that
        we use a balanced binary tree to store everything.


*******************************************************************************/


#define	SORTLIST_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"sortlist.h"


/* local defines */


/* external subroutines */


/* exported subroutines */


int sortlist_start(slp,cmpfunc)
SORTLIST	*slp ;
int		(*cmpfunc)(void *,void *) ;
{


	if (slp == NULL)
		return SR_FAULT ;

	slp->balance = 0 ;
	slp->root = NULL ;
	slp->magic = SORTLIST_MAGIC ;

	return OK ;
}
/* end subroutine (sortlist_start) */


int sortlist_finish(slp)
sortlist	*slp ;
{
	int	i ;


	if (slp == NULL) 
		return BAD ;

	if (slp->va != NULL) {

	    for (i = 0 ; i < slp->i ; i += 1) {

	        if ((slp->va)[i] != NULL)
	            free((slp->va)[i]) ;

	    } /* end while */

/* free the sortlist array itself */

	    free(slp->va) ;

	    slp->va = NULL ;
	}

	slp->i = 0 ;
	slp->e = 0 ;
	return OK ;
}
/* end subroutine (sortlist_finish) */


/* add an entry to this sorted list */
int sortlist_add(slp,p,cmpfunc)
sortlist	*slp ;
void		*p ;
int		(*cmpfunc)() ;
{
	int	rs ;
	int	i, nn ;
	int	j, bot, top, ii ;

	char	*sp ;

	void	**ep ;


#if	CF_DEBUGS
	debugprintf("sortlistadd: ent\n") ;
#endif

	if (slp == NULL) return -1 ;

#if	CF_DEBUGS
	debugprintf("sortlistadd: ent, i=%d\n",slp->i) ;
#endif

/* do we have to grow the sortlist array ? */

	if ((slp->i + 1) > slp->e) {

	    if (slp->e == 0) {

	        nn = SORTLIST_DEFENTRIES ;
	        ep = (void **)
	            malloc(sizeof(char **) * (nn + 1)) ;

	    } else {

	        nn = slp->e * 2 ;
	        ep = (void **)
	            realloc(slp->va,sizeof(char **) * (nn + 1)) ;

	    }

	    if (ep == NULL) return -1 ;

	    slp->va = ep ;
	    slp->e = nn ;

	} /* end if */

/* do the regular thing */

	slp->c += 1 ;			/* increment list count */

/* link into the list structure */

	if (cmpfunc == NULL) cmpfunc = strcmp ;

/* find the place in the existing list where this new item should be added */

	bot = 0 ;
	top = MAX((slp->i - 1),0) ;
	ii = (bot + top) / 2 ;

#if	CF_DEBUGS
	debugprintf("sortlistadd: bot=%d top=%d ii=%d\n",bot,top,ii) ;
#endif

	while ((top - bot) > 0) {

	    if ((rs = (*cmpfunc)(p,slp->va[ii])) < 0) {
	        top = ii - 1 ;

	    } else if (rs > 0) {
	        bot = ii + 1 ;

	    } else
	        break ;

	    ii = (bot + top) / 2 ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("sortlistadd: found bot=%d top=%d ii=%d\n",bot,top,ii) ;
#endif

	if (ii < slp->i) {

	    if ((*cmpfunc)(p,slp->va[ii]) > 0)
	        ii += 1 ;

/* move all entries from "ii" through "slp->i - 1" down one */

	    for (j = (slp->i - 1) ; j >= ii ; j -= 1) {
	        (slp->va)[j + 1] = (slp->va)[j] ;
	    }

	} /* end if */

/* load the new entry */

	(slp->va)[ii] = p ;
	slp->i += 1 ;
	(slp->va)[slp->i] = NULL ;

	return ii ;
}
/* end subroutine (sortlist_add) */


/* get an entry (enumerated) from the sorted list */
int sortlist_get(slp,i,pp)
sortlist	*slp ;
int		i ;
void		**pp ;
{


#if	CF_DEBUGS
	debugprintf("sortlistget: ent\n") ;
#endif

	if (slp == NULL) return BAD ;

#if	CF_DEBUGS
	debugprintf("sortlistget: i=%d\n",i) ;
#endif

	*pp = NULL ;
	if ((i < 0) || (i >= slp->i)) return BAD ;

#if	CF_DEBUGS
	debugprintf("sortlistget: 2\n") ;
#endif

	if (slp->va == NULL) return BAD ;

#if	CF_DEBUGS
	debugprintf("sortlistget: 3\n") ;
#endif

	*pp = (slp->va)[i] ;

#if	CF_DEBUGS
	debugprintf("sortlistget: 4\n") ;
#endif

	return OK ;
}
/* end subroutine (sortlist_get) */


/* delete an entry */
int sortlist_del(slp,i)
sortlist	*slp ;
int		i ;
{
	int	j ;


	if (slp == NULL) return BAD ;

	if ((i < 0) || (i >= slp->i)) return BAD ;

	if (slp->va == NULL) return OK ;

	slp->i -= 1 ;
	for (j = i ; j < slp->i ; j += 1) {
	    (slp->va)[j] = (slp->va)[j + 1] ;
	}

	(slp->va)[slp->i] = NULL ;

	slp->c -= 1 ;		/* decrement list count */
	return OK ;
}
/* end subroutine (sortlist_del) */


/* return the count of the number of items in this list */
int sortlist_count(slp)
sortlist	*slp ;
{


	if (slp == NULL) return BAD ;

	return slp->c ;
}
/* end subroutine (sortlist_count) */


/* search for an entry in the sorted list */
int sortlist_search(slp,ep,cmpfunc,pp)
sortlist	*slp ;
void		*ep ;
int		(*cmpfunc)() ;
void		**pp ;
{
	int	i, rs ;

	void	*ep2 ;
	void	**rp ;


	if (slp == NULL) return BAD ;

	if (ep == NULL) return BAD ;

	if (pp == NULL) return BAD ;

	if (slp->va == NULL) return BAD ;

	rp = (void **) bsearch(&ep,slp->va,slp->i,
	    sizeof(void *),cmpfunc) ;

	rs = BAD ;
	if (rp != NULL) {
	    i = (rp - slp->va) ;
	    rs = OK ;
	}

	*pp = NULL ;
	if (rs >= 0) *pp = slp->va[i] ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sortlist_search) */


