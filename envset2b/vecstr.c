/* vecstr */

/* vector string operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_VSTRSORT	0		/* heap sort instead of quick-sort? */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These routines are used when the caller wants to store a COPY of the
	passed string data into a vector.  These routines will copy and store
	the copied data in the list.  The advantage is that the caller does not
	have to keep the orginal data around in order for the list data to be
	accessed later.  String data (unlike "element" data) can not contain
	NULL characters-bytes.


*******************************************************************************/


#define	VECSTR_MASTER		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"vecstr.h"


/* local defines */


/* typedefs */

typedef int (*vecstr_vcmpfunc)(cchar **,cchar **) ;
typedef int (*sort_func)(const void *,const void *) ;


/* external subroutines */

extern int	iceil(int,int) ;

extern int	nleadstr(const char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

#if	CF_VSTRSORT
extern void	vstrsort(char **,int,int (*)()) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

int		vecstr_get(VECSTR *,int,cchar **) ;
int		vecstr_store(VECSTR *,cchar *,int,cchar **) ;

static int	vecstr_setopts(VECSTR *,int) ;
static int	vecstr_extvec(VECSTR *) ;
static int	vecstr_addsp(VECSTR *,cchar *) ;
static int	vecstr_insertsp(VECSTR *,int,cchar *) ;

static int	vcmpdef(cchar **,cchar **) ;


/* local variables */


/* exported subroutines */


int vecstr_start(vecstr *op,int n,int options)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (n <= 1)
	    n = VECSTR_DEFENTS ;

	if ((vecstr_setopts(op,options)) >= 0) {
	    int		size ;
	    void	*va ;
	    op->va = NULL ;
	    op->i = 0 ;
	    op->fi = 0 ;
	    op->c = 0 ;
	    op->n = n ;
	    size = (n + 1) * sizeof(const char **) ;
	    if ((rs = uc_malloc(size,&va)) >= 0) {
	        op->va = (const char **) va ;
	        op->va[0] = NULL ;
	        op->stsize = 1 ;
	    }
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (vecstr_start) */


/* free up the entire vector string data structure object */
int vecstr_finish(vecstr *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if ((op->va)[i] != NULL) {
	        rs1 = uc_free((op->va)[i]) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end while */

/* free the pointer vector array itself */

	rs1 = uc_free(op->va) ;
	if (rs >= 0) rs = rs1 ;
	op->va = NULL ;

	op->i = 0 ;
	op->n = 0 ;
	return rs ;
}
/* end subroutine (vecstr_finish) */


int vecstr_audit(vecstr *op)
{
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] != NULL) {
		c += 1 ;
	        cp = (const char *) op->va[i] ;
	        rs |= *cp ;		/* access might SEGFAULT (as wanted) */
	    }
	} /* end for */

	rs = (c == op->c) ? SR_OK : SR_BADFMT ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_audit) */


/* add a string to the vector container */
int vecstr_store(vecstr *op,cchar *sp,int sl,cchar **rpp)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

/* do we have to grow the vector array? */

	if ((op->i + 1) > op->n) {
	    rs = vecstr_extvec(op) ;
	}

	if (rs >= 0) {
	    const int	size = (sl+1) ;
	    char	*bp ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        strwcpy(bp,sp,sl) ;
	        op->stsize += size ;
	        i = vecstr_addsp(op,bp) ;
		if (rpp != NULL) *rpp = bp ;
	    } /* end if (m-a) */
	} /* end if (ok) */

	if ((rpp != NULL) && (rs < 0)) *rpp = NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecstr_store) */


/* add a string that is a key=value */
int vecstr_addkeyval(vecstr *op,cchar *kp,int kl,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (kl < 0) kl = strlen(kp) ;

#if	CF_DEBUGS
	debugprintf("vecstr_addkeyval: key=%t\n",kp,kl) ;
#endif

	if ((vl < 0) || ((vp == NULL) && (vl != 0))) {
	    vl = (vp != NULL) ? strlen(vp) : 0 ;
	}

#if	CF_DEBUGS
	if (vp != NULL)
	    debugprintf("vecstr_addkeyval: val=%t\n",vp,vl) ;
#endif

/* do we have to grow the vector array? */

	if ((op->i + 1) > op->n) {
	    rs = vecstr_extvec(op) ;
	}

	if (rs >= 0) {
	    const int	size = (kl + 1 + vl + 1) ;
	    char	*ap ;
	    if ((rs = uc_malloc(size,&ap)) >= 0) {
		char	*bp = ap ;
	        bp = strwcpy(bp,kp,kl) ;
	        *bp++ = '=' ;
	        if (vp != NULL) bp = strwcpy(bp,vp,vl) ;
	        *bp = '\0' ;
	        op->stsize += size ;
	        i = vecstr_addsp(op,ap) ;
	    } /* end if (memory-allocation) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("vecstr_addkeyval: ret rs=%d i=%d\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecstr_addkeyval) */


int vecstr_add(vecstr *op,cchar *sp,int sl)
{

	return vecstr_store(op,sp,sl,NULL) ;
}
/* end subroutine (vecstr_add) */


/* get a string by its index */
int vecstr_get(vecstr *op,int i,cchar **spp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vecstr_get: ent i=%d\n",i) ;
#endif

	if ((i < 0) || (i >= op->i)) {
	    rs = SR_NOTFOUND ;
	}

	if (spp != NULL) {
	     *spp = (rs >= 0) ? op->va[i] : NULL ;
	}


#if	CF_DEBUGS
	debugprintf("vecstr_get: ret rs=%d result=%s\n",rs,*spp) ;
#endif

	return rs ;
}
/* end subroutine (vecstr_get) */


/* get the last entry */
int vecstr_getlast(vecstr *op,cchar **spp)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (op->c > 0) {
	    i = (op->i-1) ;
	    while ((i >= 0) && ((op->va)[i] == NULL)) {
		i -= 1 ;
	    }
	    if (i < 0) rs = SR_BUGCHECK ;
	} else {
	    rs = SR_NOTFOUND ;
	}

	if (spp != NULL) {
	     *spp = (rs >= 0) ? (op->va)[i] : NULL ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecstr_getlast) */


int vecstr_del(vecstr *op,int i)
{
	int		f_fi = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((i < 0) || (i >= op->i))
	    return SR_NOTFOUND ;

/* delete the entry (free the actual string data) */

	if ((op->va)[i] != NULL) {

	    op->c -= 1 ;		/* decrement list count */
	    if (op->f.stsize) {
	        op->stsize -= (strlen((op->va)[i]) + 1) ;
	    }

	    uc_free((op->va)[i]) ;

	} /* end if (freeing the actual string data) */

/* apply the appropriate deletion based on management policy */

	if (op->f.ostationary) {

	    (op->va)[i] = NULL ;
	    if (i == (op->i - 1))
	        op->i -= 1 ;

	    f_fi = TRUE ;

	} else if (op->f.issorted || op->f.oordered) {

	    if (op->f.ocompact) {
		int	j ;

	        op->i -= 1 ;
	        for (j = i ; j < op->i ; j += 1) {
	            (op->va)[j] = (op->va)[j + 1] ;
		}

	        (op->va)[op->i] = NULL ;

	    } else {

	        (op->va)[i] = NULL ;
	        if (i == (op->i - 1))
	            op->i -= 1 ;

	        f_fi = TRUE ;

	    } /* end if */

	} else {

	    if ((op->f.oswap || op->f.ocompact) && (i < (op->i - 1))) {

	        (op->va)[i] = (op->va)[op->i - 1] ;
	        (op->va)[--op->i] = NULL ;
	        op->f.issorted = FALSE ;

	    } else {

	        (op->va)[i] = NULL ;
	        if (i == (op->i - 1))
	            op->i -= 1 ;

	        f_fi = TRUE ;

	    } /* end if */

	} /* end if */

	if (op->f.oconserve) {

	    while (op->i > i) {

	        if (op->va[op->i - 1] != NULL)
	            break ;

	        op->i -= 1 ;

	    } /* end while */

	} /* end if */

	if (f_fi && (i < op->fi))
	    op->fi = i ;

	return op->c ;
}
/* end subroutine (vecstr_del) */


int vecstr_delall(vecstr *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] != NULL) {
		rs1 = uc_free(op->va[i]) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	op->i = 0 ;
	op->fi = 0 ;
	op->va[0] = NULL ;

	op->c = 0 ;
	op->stsize = 1 ;
	return rs ;
}
/* end subroutine (vecstr_delall) */


/* return the count of the number of items in this list */
int vecstr_count(vecstr *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (vecstr_count) */


/* sort the strings in the vector list */
int vecstr_sort(vecstr *op,vecstr_vcmpfunc vcmpfunc)
{

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("vecstr_sort: ent issorted=%u\n",op->f.issorted) ;
#endif

	if (vcmpfunc == NULL)
	    vcmpfunc = vcmpdef ;

	if ((! op->f.issorted) && (op->c > 1)) {
	    sort_func sfunc = (sort_func) vcmpfunc ;

#if	CF_VSTRSORT
	    vstrsort(op->va,op->i,sfunc) ;
#else
	    qsort(op->va,op->i,sizeof(char *),sfunc) ;
#endif /* CF_VSTRSORT */

	} /* end if (sorting) */

	op->f.issorted = TRUE ;
	return op->c ;
}
/* end subroutine (vecstr_sort) */


/* search for a string in the vector list */
int vecstr_search(vecstr *op,cchar *sp,vecstr_vcmpfunc vcmpfunc,cchar **rpp)
{
	const int	esize = sizeof(char *) ;
	int		rs ;
	int		i ;
	const char	**rpp2 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (vcmpfunc == NULL)
	    vcmpfunc = vcmpdef ;

	if (op->f.osorted && (! op->f.issorted)) {
	    op->f.issorted = TRUE ;
	    if (op->c > 1) {
	        sort_func sfunc = (sort_func) vcmpfunc ;
	        qsort(op->va,op->i,esize,sfunc) ;
	    }
	} /* end if (sorting) */

	if (op->f.issorted) {
	    sort_func sfunc = (sort_func) vcmpfunc ;

	    rpp2 = (cchar **) bsearch(&sp,op->va,op->i,esize,sfunc) ;

	    rs = SR_NOTFOUND ;
	    if (rpp2 != NULL) {
	        i = rpp2 - op->va ;
	        rs = SR_OK ;
	    }

	} else {

	    for (i = 0 ; i < op->i ; i += 1) {
	        rpp2 = op->va + i ;
	        if (*rpp2 != NULL) {
	            if ((*vcmpfunc)(&sp,rpp2) == 0) break ;
		}
	    } /* end for */

	    rs = (i < op->i) ? SR_OK : SR_NOTFOUND ;

	} /* end if (sorted or not) */

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? op->va[i] : NULL ;
	}

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecstr_search) */


/* search for a string in the vector list */
int vecstr_finder(vecstr *op,cchar *sp,vecstr_vcmpfunc vcmpfunc,cchar **rpp)
{
	int		rs = SR_NOTFOUND ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (vcmpfunc == NULL)
	    vcmpfunc = vcmpdef ;

	if (rpp != NULL)
	    *rpp = NULL ;

	for (i = 0 ; i < op->i ; i += 1) {
	    if (op->va[i] != NULL) {
	        if ((*vcmpfunc)(&sp,(op->va + i)) == 0) break ;
	    }
	} /* end for */

	if (i < op->i) {
	    rs = i ;
	    if (rpp != NULL) *rpp = op->va[i] ;
	}

	return rs ;
}
/* end subroutine (vecstr_finder) */


/* find if a string is already in the vector list */

/*
	This method (subroutine) is equivalent to:

		vecstr_finder(lp,string,NULL,NULL)

	but is shorter (and hopefully faster).

*/

int vecstr_find(vecstr *op,cchar *sp)
{
	int		sch ;
	int		rs = SR_NOTFOUND ;
	int		i ;
	const char	*ep ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	sch = sp[0] ; /* ok: since all get promoted similarly */
	for (i = 0 ; i < op->i ; i += 1) {
	    ep = op->va[i] ;
	    if (ep == NULL) continue ;
	    if ((sch == ep[0]) && (strcmp(sp,ep) == 0))
	        break ;
	} /* end for */

	if (i < op->i) rs = i ;

	return rs ;
}
/* end subroutine (vecstr_find) */


/* find a counted string */
int vecstr_findn(vecstr *op,cchar *sp,int sl)
{
	int		sch ; 
	int		rs = SR_NOTFOUND ;
	int		i ;
	int		m ;
	const char	*ep ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

	sch = sp[0] ; /* ok: since all get promoted similarly */
	for (i = 0 ; i < op->i ; i += 1) {
	    ep = op->va[i] ;
	    if (ep != NULL) {
	        if (sch == ep[0]) {
		    m = nleadstr(ep,sp,sl) ;
		    if ((m == sl) && (ep[m] == '\0')) break ;
		}
	    }
	} /* end for */

	if (i < op->i) rs = i ;

	return rs ;
}
/* end subroutine (vecstr_findn) */


int vecstr_findaddr(vecstr *op,cchar *addr)
{
	int		rs = SR_NOTFOUND ;
	int		i ;
	const char	*ep ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

#ifdef	COMMENT
	if (addr == NULL) return SR_FAULT ;
#endif

	for (i = 0 ; i < op->i ; i += 1) {
	    ep = op->va[i] ;
	    if ((ep != NULL) && (addr == ep)) {
		rs = i ;
		break ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (vecstr_findaddr) */


/* get the vector array address */
int vecstr_getvec(vecstr *op,cchar ***rppp)
{

	if (op == NULL) return SR_FAULT ;
	if (rppp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	*rppp = op->va ;
	return op->i ;
}
/* end subroutine (vecstr_getvec) */


int vecstr_strsize(vecstr *op)
{
	int		size = 1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if (! op->f.stsize) {
	    int		i ;
	    for (i = 0 ; op->va[i] != NULL ; i += 1) {
	        if (op->va[i] != NULL) {
	            size += (strlen(op->va[i]) + 1) ;
		}
	    } /* end for */
	    op->stsize = size ;
	    op->f.stsize = TRUE ;
	} /* end if (calculating size) */

	size = iceil(op->stsize,sizeof(int)) ;

	return size ;
}
/* end subroutine (vecstr_strsize) */


int vecstr_strmk(vecstr *op,char *tab,int tabsize)
{
	int		rs = SR_OK ;
	int		size ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (tab == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

/* check supplied tabsize */

	size = iceil(op->stsize,sizeof(int)) ;

	if (tabsize >= size) {
	    int		i ;
	    char	*bp = tab ;
	    c = 1 ;
	    *bp++ = '\0' ;
	    for (i = 0 ; op->va[i] != NULL ; i += 1) {
	        if (op->va[i] != NULL) {
	    	    c += 1 ;
	            bp = strwcpy(bp,op->va[i],-1) + 1 ;
		}
	    } /* end for */
	    while (bp < (tab + tabsize)) {
	        *bp++ = '\0' ;
	    }
	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_strmk) */


/* make the record table and the string table simultaneously */
int vecstr_recmkstr(vecstr *op,int *rec,int recsize,char *tab,int tabsize)
{
	int		rs = SR_OK ;
	int		size ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rec == NULL) return SR_FAULT ;
	if (tab == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

/* check supplied tabsize */

	if (op->stsize == 0) {
	    vecstr_strsize(op) ;
	}

	size = iceil(op->stsize,sizeof(int)) ;

	if (tabsize >= size) {
	    size = (op->c + 2) * sizeof(int) ;
	    if (recsize >= size) {
		int	i ;
		char	*bp = tab ;
	        rec[c++] = 0 ;
	        *bp++ = '\0' ;
	        for (i = 0 ; op->va[i] != NULL ; i += 1) {
	            if (op->va[i] != NULL) {
	                rec[c++] = (bp - tab) ;
	                bp = (strwcpy(bp,op->va[i],-1) + 1) ;
		    }
	        } /* end for */
	        rec[c] = -1 ;
	    } else {
	        rs = SR_OVERFLOW ;
	    }
	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_recmkstr) */


int vecstr_recsize(vecstr *op)
{
	int		size ;

	if (op == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	size = (op->c + 2) * sizeof(int) ;
	return size ;
}
/* end subroutine (vecstr_recsize) */


int vecstr_recmk(vecstr *op,int *rec,int recsize)
{
	int		rs = SR_OK ;
	int		size ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (rec == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

/* check supplied size */

	size = (op->c + 2) * sizeof(int) ;

	if (recsize >= size) {
	    int	si = 0 ;
	    int	i ;
	    rec[c++] = si ;
	    si += 1 ;
	    for (i = 0 ; op->va[i] != NULL ; i += 1) {
	        if (op->va[i] != NULL) {
	            rec[c++] = si ;
	            si += (strlen(op->va[i]) + 1) ;
		}
	    } /* end for */
	    rec[c] = -1 ;
	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (vecstr_recmk) */


int vecstr_insert(vecstr *op,int ii,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->va == NULL) return SR_NOTOPEN ;

	if ((ii < 0) || (ii > op->i)) return SR_INVALID ;

	if (sl < 0)
	    sl = strlen(sp) ;

/* do we have to grow the vector array? */

	if ((op->i + 1) > op->n) {
	    rs = vecstr_extvec(op) ;
	}

	if (rs >= 0) {
	    char	*bp ;
	    int		size = (sl+1) ;
	    if ((rs = uc_malloc(size,&bp)) >= 0) {
	        strwcpy(bp,sp,sl) ;
	        op->stsize += size ;
	        i = vecstr_insertsp(op,ii,bp) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("vecstr_insert: ret rs=%d i=%d\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (vecstr_insert) */


/* private subroutines */


static int vecstr_setopts(vecstr *op,int options)
{

	memset(&op->f,0,sizeof(struct vecstr_flags)) ;
	if (options & VECSTR_OREUSE) op->f.oreuse = 1 ;
	if (options & VECSTR_OSWAP) op->f.oswap = 1 ;
	if (options & VECSTR_OSTATIONARY) op->f.ostationary = 1 ;
	if (options & VECSTR_OCOMPACT) op->f.ocompact = 1 ;
	if (options & VECSTR_OSORTED) op->f.osorted = 1 ;
	if (options & VECSTR_OORDERED) op->f.oordered = 1 ;
	if (options & VECSTR_OCONSERVE) op->f.oconserve = 1 ;
	if (options & VECSTR_OSTSIZE) op->f.stsize = 1 ;

	return SR_OK ;
}
/* end subroutine (vecstr_setopts) */


static int vecstr_extvec(vecstr *op)
{
	int		rs = SR_OK ;

	if ((op->i + 1) > op->n) {
	    int		nn, size ;
	    void	*na ;

	    if (op->va == NULL) {
	        nn = VECSTR_DEFENTS ;
	        size = (nn + 1) * sizeof(char **) ;
	        rs = uc_malloc(size,&na) ;
	    } else {
	        nn = (op->n + 1) * 2 ;
	        size = (nn + 1) * sizeof(char **) ;
	        rs = uc_realloc(op->va,size,&na) ;
	    }

	    if (rs >= 0) {
	        op->va = (const char **) na ;
		op->va[op->i] = NULL ;
	        op->n = nn ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("vecstr_extvec: va=%p\n",op->va) ;
	debugprintf("vecstr_extvec: n=%u\n",op->n) ;
#endif

	return rs ;
}
/* end subroutine (vecstr_extvec) */


static int vecstr_addsp(vecstr *op,cchar *sp)
{
	int		i = 0 ;
	int		f_done = FALSE ;
	int		f ;

/* OK, handle any specific options */

	f = op->f.oreuse || op->f.oconserve ;
	if (f && (op->c < op->i)) {

	    i = op->fi ;
	    while ((i < op->i) && (op->va[i] != NULL)) {
	        i += 1 ;
	    }

	    op->fi = i + 1 ;
	    if (i < op->i) {
	        (op->va)[i] = (char *) sp ;
	        f_done = TRUE ;
	    }

	} /* end if (reuse or conserve) */

	if (! f_done) {
	    i = op->i ;
	    (op->va)[(op->i)++] = sp ;
	    (op->va)[op->i] = NULL ;
	}

	op->c += 1 ;
	op->f.issorted = FALSE ;

	return i ;
}
/* end subroutine (vecstr_addsp) */


static int vecstr_insertsp(vecstr *op,int ii,cchar *sp)
{

	if (ii == op->i) {

	    op->i += 1 ;
	    op->va[op->i] = NULL ;

	} else if (op->va[ii] != NULL) {
	    int		i, j ;

/* find */

	    for (i = (ii + 1) ; i < op->i ; i += 1) {
		if (op->va[i] == NULL) break ;
	    }

/* management */

	    if (i == op->i) {
	        op->i += 1 ;
	        op->va[op->i] = NULL ;
	    }

/* move-up */

	    for (j = i ; j > ii ; j -= 1) {
		op->va[j] = op->va[j-1] ;
	    }

	} /* end if */

	op->va[ii] = sp ;

	op->c += 1 ;
	op->f.issorted = FALSE ;

	return ii ;
}
/* end subroutine (vecstr_insertsp) */


static int vcmpdef(cchar **e1pp,cchar **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = strcmp(*e1pp,*e2pp) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (vcmpdef) */


