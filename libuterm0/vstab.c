/* vstab */

/* vector string operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 1998-12-01, David A­D­ Morano

	Module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This object manages the process FD table.


**************************************************************************/


#define	VSTAB_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vstab.h"


/* local defines */

#define	VSTAB_DEFENTRIES	10


/* external subroutines */


/* forward referecens */

int		vstab_get(VSTAB *,int,char **) ;

static int	defaultcmp() ;


/* exported subroutines */


int vstab_init(vsp,osize)
VSTAB	*vsp ;
int	osize ;
{
	int	rs ;
	int	n ;
	int	size ;

	void	*np ;


	if (vsp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("vstab_init: entered osize=%d\n",osize) ;
#endif

	    n = VSTAB_DEFENTRIES ;

	vsp->va = NULL ;
	vsp->c = vsp->i = 0 ;
	size = n * sizeof(void **) ;

#if	CF_DEBUGS
	debugprintf("vstab_init: uc_malloc() size=%d\n",
	    size) ;
#endif

	rs = uc_malloc(size, &np) ;

#if	CF_DEBUGS
	debugprintf("vstab_init: uc_malloc() rs=%d\n",rs) ;
#endif

	if (rs < 0)
		goto ret0 ;

	vsp->va = (void **) np ;
	vsp->va[0] = NULL ;
	vsp->n = n ;
	vsp->osize = osize ;

ret0:
	return rs ;
}
/* end subroutine (vstab_init) */


int vstab_audit(vhp)
VSTAB	*vhp ;
{
	int	rs ;
	int	i ;

	char	*cp ;


	if (vhp == NULL)
	    return SR_FAULT ;

	if (vhp->va == NULL)
	    return SR_NOTOPEN ;

	for (i = 0 ; i < vhp->i ; i += 1) {

	    if (vhp->va[i] != NULL) {

	        cp = (char *) vhp->va[i] ;
	        rs = *cp ;		/* access might SEGFAULT */

	    }

	} /* end for */

	rs = (vhp->va[i] == NULL) ? SR_OK : SR_BADFMT ;

	return rs ;
}
/* end subroutine (vstab_audit) */


/* free up the entire vector string data structure object */
int vstab_free(vsp)
VSTAB	*vsp ;
{
	int	i ;


#if	CF_DEBUGS
	debugprintf("vstab_free: entered\n") ;
#endif

	if (vsp == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("vstab_free: continuing, c=%d i=%d\n",vsp->c,vsp->i) ;
#endif

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

#if	CF_DEBUGS
	    debugprintf("vstab_free: about to loop\n") ;
#endif

	    for (i = 0 ; i < vsp->i ; i += 1) {

#if	CF_DEBUGS
	        debugprintf("vstab_free: loop i=%d\n",i) ;
#endif

	        if ((vsp->va)[i] != NULL) {

#if	CF_DEBUGS
	            debugprintf("vstab_free: va[%d]=%08X\n",i,(vsp->va)[i]) ;
#endif

	            uc_free((vsp->va)[i]) ;

	        }

	    } /* end while */

/* free the pointer vector array itself */

#if	CF_DEBUGS
	    debugprintf("vstab_free: free array\n") ;
#endif

	    uc_free(vsp->va) ;

	    vsp->va = NULL ;

#if	CF_DEBUGS
	debugprintf("vstab_free: exiting\n") ;
#endif

	vsp->i = 0 ;
	vsp->n = 0 ;
	return SR_OK ;
}
/* end subroutine (vstab_free) */


int vstab_getfd(vsp,fd,rpp)
VSTAB	*vsp ;
int	fd ;
void	**rpp ;
{
	int	rs ;
	int	i, nn ;
	int	size ;

	void	**npp ;
	void	*np ;


#if	CF_DEBUGS
	debugprintf("vstab_getfd: entered\n") ;
#endif

	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	if (fd < 0)
	    return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("vstab_getfd: SR_OK\n") ;
#endif

/* do we have to grow the vector array ? */

	if (fd > vsp->n) {

	    if (vsp->n == 0) {

	        nn = VSTAB_DEFENTRIES ;
	        size = nn * sizeof(void **) ;
	        rs = uc_malloc(size,&npp) ;

	    } else {

	        nn = vsp->n * 2 ;
	        size = nn * sizeof(void **) ;
	        rs = uc_realloc(vsp->va,size,&npp) ;

	    }

	    if (rs < 0)
	        return rs ;

	    vsp->va = npp ;
	    vsp->n = nn ;

	} /* end if */

/* do the regular thing */

	if (vsp->va[fd] == NULL) {

		rs = uc_malloc(vsp->osize,&np) ;

#if	CF_DEBUGS
	debugprintf("vstab_getfd: uc_malloc() rs=%d return=%p\n",rs,np) ;
#endif

		if (rs >= 0)
		vsp->va[fd] = np ;

	} else
		np = vsp->va[fd] ;

	if ((rs >= 0) && (rpp != NULL))
		*rpp = np ;

#if	CF_DEBUGS
	debugprintf("vstab_getfd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (vstab_getfd) */


int vstab_del(vsp,fd)
VSTAB	*vsp ;
int	fd ;
{
	int	rs ;


	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	if (fd < 0)
		return SR_INVALID ;

	if ((fd < 0) || (fd >= vsp->i))
	    return SR_NOTFOUND ;

	if ((vsp->va)[fd] != NULL) {

	    vsp->c -= 1 ;		/* decrement list count */
	    uc_free((vsp->va)[fd]) ;

	}

	return rs ;
}
/* end subroutine (vstab_del) */


/* return the count of the number of items in this list */
int vstab_count(vsp)
VSTAB	*vsp ;
{


	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	return vsp->c ;
}
/* end subroutine (vstab_count) */


#ifdef	COMMENT

/* search for a string in the vector list */
int vstab_search(vsp,s,cmpfunc,rpp)
VSTAB	*vsp ;
char	s[] ;
int	(*cmpfunc)() ;
char	**rpp ;
{
	int	rs ;
	int	i ;

	char	**rpp2 ;


	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	if (cmpfunc == NULL)
	    cmpfunc = defaultcmp ;

	if ((vsp->policy == VSTAB_PSORTED) &&
	    (! vsp->f_sorted)) {

	        vsp->f_sorted = TRUE ;
	        if (vsp->c > 1)
	            (void) qsort(vsp->va,(size_t) vsp->i,
			sizeof(char *),cmpfunc) ;

	    } /* end if (sorting) */

	    if (vsp->f_sorted) {

	        rpp2 = (char **) bsearch(&s,vsp->va,vsp->i,
	            sizeof(char *),cmpfunc) ;

	        rs = SR_NOTFOUND ;
	        if (rpp2 != NULL) {

	            i = rpp2 - vsp->va ;
	            rs = SR_OK ;

	        }

	    } else {

	        for (i = 0 ; i < vsp->i ; i += 1) {

	            rpp2 = vsp->va + i ;
	            if (*rpp2 == NULL) continue ;

	            if ((*cmpfunc)(&s,rpp2) == 0)
	                break ;

	        } /* end for */

		rs = (i < vsp->i) ? SR_OK : SR_NOTFOUND ;

	    } /* end if (sorted or not) */

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? vsp->va[i] : NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vstab_search) */


/* search for a string in the vector list */
int vstab_finder(vsp,s,cmpfunc,rpp)
VSTAB	*vsp ;
char	s[] ;
int	(*cmpfunc)() ;
char	**rpp ;
{
	int	rs ;
	int	i ;

	char	*rp ;


	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	if (cmpfunc == NULL)
	    cmpfunc = defaultcmp ;

	if (rpp == NULL)
	    rpp = &rp ;

	(*rpp) = NULL ;

	rs = SR_NOTFOUND ;
	for (i = 0 ; i < vsp->i ; i += 1) {

	    if (vsp->va[i] == NULL) continue ;

	    if ((*cmpfunc)(&s,(vsp->va + i)) == 0)
	        break ;

	} /* end for */

	if (i < vsp->i) {
		rs = i ;
		*rpp = vsp->va[i] ;
	}

	return rs ;
}
/* end subroutine (vstab_finder) */


/* find if a string is already in the vector list */

/*
	This method (subroutine) is really just :

		vstab_finder(lp,string,NULL,NULL)

	but is shorter.

*/

int vstab_find(vsp,s)
VSTAB	*vsp ;
char	s[] ;
{
	int	rs ;
	int	i ;


	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	rs = SR_NOTFOUND ;
	for (i = 0 ; i < vsp->i ; i += 1) {

	    if (vsp->va[i] == NULL) continue ;

	    if (strcmp(s,vsp->va[i]) == 0)
	        break ;

	} /* end for */

	if (i < vsp->i)
		rs = i ;

	return rs ;
}
/* end subroutine (vstab_find) */


/* find a counted string */
int vstab_findn(vsp,s,sl)
VSTAB	*vsp ;
char	s[] ;
int	sl ;
{
	int	rs ;
	int	i ;

	char	*rp ;


	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	if (sl < 0)
	    sl = strlen(s) ;

	rs = SR_NOTFOUND ;
	for (i = 0 ; i < vsp->i ; i += 1) {

	    if (vsp->va[i] == NULL) continue ;

	    rp = vsp->va[i] ;
	    if ((strncmp(s,rp,sl) == 0) && (rp[sl] == '\0'))
	        break ;

	} /* end for */

	if (i < vsp->i)
		rs = i ;

	return rs ;
}
/* end subroutine (vstab_findn) */

#endif /* COMMENT */


/* get the vector array address */
int vstab_getvec(vsp,rppp)
VSTAB	*vsp ;
void	***rppp ;
{


	if (vsp == NULL)
	    return SR_FAULT ;

	if (vsp->va == NULL)
		return SR_NOTOPEN ;

	if (rppp == NULL)
	    return SR_FAULT ;

	*rppp = vsp->va ;
	return SR_OK ;
}
/* end subroutine (vstab_getvec) */



/* INTERNAL SUBROUTINES */



#ifdef	COMMENT

static int defaultcmp(e1pp,e2pp)
char	**e1pp, **e2pp ;
{


	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	return strcmp(*e1pp,*e2pp) ;
}
/* end subroutine (defaultcmp) */

#endif /* COMMENT */



