/* strstore */

/* string storeage-table object */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_PREALLOC	1		/* pre-allocate a chunk */
#define	CF_SAFE		0		/* extra safety */


/* revision history:

	= 1998-03-24, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module creates and manages a string storage object.  This
	object is sort of a write-only storage area for storing strings.
	Strings cannot be deleted from the object once they are added
	(something not commonly needed anyway).  Since most string storage
	requirements do NOT need the ability to remove strings once they are
	entered, this object provides a nice optimized manager for handling
	that sort of situation.

	This object is very similar to a STRSTORE object (in that strings
	cannot be deleted) but is somewhat more optimized.

	Arguments:

	op		pointer to the strstore object
	<others>

	Returns:

	>=0		the total length of the filled up strstore so far!
	<0		error


*******************************************************************************/


#define	STRSTORE_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"strstore.h"


/* local defines */

#define	MODP2(v,n)	((v) & ((n) - 1))
#define	STRENTRY	struct strentry


/* external subroutines */

extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;
extern uint	hashagain(uint,int,int) ;

extern int	iceil(int,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */

struct strentry {
	uint	khash ;
	uint	hi ;
	uint	si ;
} ;


/* forward references */

int		strstore_store(STRSTORE *,const char *,int,const char **) ;
int		strstore_already(STRSTORE *,const char *,int) ;

static int	strstore_chunknew(STRSTORE *,int) ;
static int	strstore_chunkfins(STRSTORE *) ;
static int	strstore_manage(STRSTORE *,const char *,int,int) ;

static int	chunk_start(STRSTORE_CHUNK *,int) ;
static int	chunk_adv(STRSTORE_CHUNK *) ;
static int	chunk_finish(STRSTORE_CHUNK *) ;

static int	indexlen(int) ;
static int	indexsize(int) ;
static int	hashindex(uint,int) ;


/* local variables */


/* exported subroutines */


int strstore_start(STRSTORE *op,int n,int chunksize)
{
	const int	vo = VECHAND_PORDERED ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (n < STRSTORE_STARTLEN)
	    n = STRSTORE_STARTLEN ;

	if (chunksize < STRSTORE_CHUNKSIZE)
	    chunksize = STRSTORE_CHUNKSIZE ;

	memset(op,0,sizeof(STRSTORE)) ;
	op->chunksize = chunksize ;

	if ((rs = vechand_start(&op->chunks,MAX((n/6),6),vo)) >= 0) {
	    if ((rs = vechand_start(&op->list,n,vo)) >= 0) {
	        if ((rs = lookaside_start(&op->imgr,sizeof(int),n)) >= 0) {
		    const int	hn = ((n*3)/2) ;
	            if ((rs = hdb_start(&op->smgr,hn,TRUE,NULL,NULL)) >= 0) {
	                op->magic = STRSTORE_MAGIC ;
#if	CF_PREALLOC
	                rs = strstore_chunknew(op,0) ;
	                if (rs < 0)  {
	                    hdb_finish(&op->smgr) ;
	                    op->magic = 0 ;
	                }
#endif /* CF_PREALLOC */
	            }
	            if (rs < 0)
	                lookaside_finish(&op->imgr) ;
	        } /* end if (lookaside_start) */
	        if (rs < 0)
		    vechand_finish(&op->list) ;
	    } /* end if (vechand) */
	    if (rs < 0)
		vechand_finish(&op->chunks) ;
	} /* end if (vechand) */

	return rs ;
}
/* end subroutine (strstore_start) */


/* free up this strstore object */
int strstore_finish(STRSTORE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;

	rs1 = hdb_finish(&op->smgr) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = lookaside_finish(&op->imgr) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

/* pop them */

	rs1 = strstore_chunkfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->chunks) ;
	if (rs >= 0) rs = rs1 ;

	op->ccp = NULL ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (strstore_finish) */


int strstore_add(STRSTORE *op,cchar sp[],int sl)
{

	return strstore_store(op,sp,sl,NULL) ;
}
/* end subroutine (strstore_add) */


int strstore_adduniq(STRSTORE *op,cchar sp[],int sl)
{
	const int	nrs = SR_NOTFOUND ;
	int		rs ;

	if ((rs = strstore_already(op,sp,sl)) == nrs) {
	    rs = strstore_store(op,sp,sl,NULL) ;
	}

	return rs ;
}
/* end subroutine (strstore_adduniq) */


/* add a character string to this STRSTORE object */
int strstore_store(STRSTORE *op,cchar sp[],int sl,cchar **rpp)
{
	STRSTORE_CHUNK	*ccp ;
	int		rs = SR_OK ;
	int		amount ;
	int		si = 0 ;
	char		*ep = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;

	if (sl < 0)
	    sl = strlen(sp) ;

	amount = (sl + 1) ;
	ccp = op->ccp ;
	if ((ccp == NULL) || (amount > (ccp->csize - ccp->i))) {
	    rs = strstore_chunknew(op,amount) ;
	    ccp = op->ccp ;
	}

	if (rs >= 0) {
	    int	i ;
	    si = op->totalsize ;
	    ep = (ccp->cdata + ccp->i) ;
	    strwcpy(ep,sp,sl) ;

	    rs = vechand_add(&op->list,ep) ;
	    i = rs ;
	    if (rs >= 0) {
	        rs = strstore_manage(op,ep,sl,si) ;
	        if (rs < 0) vechand_del(&op->list,i) ;
	    }

	} /* end if */

	if (rs >= 0) {
	    ccp->i += amount ;
	    ccp->c += 1 ;		/* count in chunk */
	    op->c += 1 ;		/* count in object */
	    op->totalsize += amount ;
	}

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? ep : NULL ;

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (strstore_store) */


int strstore_curbegin(strstore *op,strstore_cur *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (strstore_curbegin) */


int strstore_curend(strstore *op,strstore_cur *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
	if (curp == NULL) return SR_FAULT ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (strstore_curend) */


/* get a string from the table by its index */
int strstore_enum(STRSTORE *op,STRSTORE_CUR *curp,cchar **rpp)
{
	int		rs = SR_OK ;
	int		val ;
	int		cl = 0 ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;

	val = (curp->i >= 0) ? (curp->i+1) : 0 ;

	if (val < op->c) {
	    if ((rs = vechand_get(&op->list,val,&cp)) >= 0) {
	        cl = strlen(cp) ;
	        if (rpp != NULL) *rpp = cp ;
	        curp->i = val ;
	    }
	} else
	    rs = SR_NOTFOUND ;

	return (rs >= 0) ? cl : rs ;
}
/* end subroutine (strstore_enum) */


/* is a given string already represented? */
int strstore_already(STRSTORE *op,cchar *sp,int sl)
{
	HDB_DATUM	key, val ;
	int		rs ;
	int		*ip ;
	int		si = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;

	if (sl < 0)
	    sl = strlen(sp) ;

/* do we have it already? */

	key.buf = sp ;
	key.len = sl ;
	if ((rs = hdb_fetch(&op->smgr,key,NULL,&val)) >= 0) {
	    ip = (int *) val.buf ;
	    si = *ip ;
	} /* end if */

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (strstore_already) */


/* get the string count in the table */
int strstore_count(STRSTORE *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (strstore_count) */


int strstore_size(STRSTORE *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;

	return op->totalsize ;
}
/* end subroutine (strstore_size) */


int strstore_strsize(STRSTORE *op)
{
	int		size ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;

	size = iceil(op->totalsize,sizeof(int)) ;

	return size ;
}
/* end subroutine (strstore_strsize) */


int strstore_strmk(STRSTORE *op,char tabp[],int tabl)
{
	STRSTORE_CHUNK	*ccp ;
	int		size ;
	int		i ;
	int		c = 0 ;
	char		*bp = tabp ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
#endif

	if (tabp == NULL) return SR_FAULT ;

	size = iceil(op->totalsize,sizeof(int)) ;

	if (tabl < size)
	    return SR_OVERFLOW ;

	for (i = 0 ; vechand_get(&op->chunks,i,&ccp) >= 0 ; i += 1) {
	    if (ccp != NULL) {
	        if (ccp->cdata != NULL) {
	            c += 1 ;
	            memcpy(bp,ccp->cdata,ccp->i) ;
	            bp += ccp->i ;
	        }
	    }
	} /* end for */

	while (bp < (tabp + tabl)) {
	    *bp++ = '\0' ;
	}

	return c ;
}
/* end subroutine (strstore_strmk) */


/* calculate the index table length (entries) */
int strstore_recsize(STRSTORE *op)
{
	int		size ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
#endif

	{
	    int	n = (op->c + 1) ;
	    size = (n + 1) * sizeof(int) ;
	}

	return size ;
}
/* end subroutine (strstore_recsize) */


int strstore_recmk(STRSTORE *op,int rdata[],int rsize)
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		n, size ;
	int		*ip ;
	int		c = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
#endif

	if (rdata == NULL) return SR_FAULT ;

	n = (op->c + 1) ;
	size = (n + 1) * sizeof(int) ;
	if (rsize < size) return SR_OVERFLOW ;

	rdata[c++] = 0 ;		/* ZERO-entry is NUL-string */

/* load all other entries */

	if ((rs = hdb_curbegin(&op->smgr,&cur)) >= 0) {

	    while (hdb_enum(&op->smgr,&cur,&key,&val) >= 0) {

	        ip = (int *) val.buf ;
	        rdata[c++] = *ip ;

	    } /* end while (looping through strings) */

	    hdb_curend(&op->smgr,&cur) ;
	} /* end if */

/* done (place end-marker) */

	rdata[c] = -1 ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (strstore_recmk) */


/* calculate the index table length (entries) */
int strstore_indlen(STRSTORE *op)
{
	int		n ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
#endif

	n = indexlen(op->c+1) ;

	return n ;
}
/* end subroutine (strstore_indlen) */


/* calculate the index table size */
int strstore_indsize(STRSTORE *op)
{
	int		isize ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
#endif

	{
	    int il = indexlen(op->c+1) ;
	    isize = indexsize(il) ;
	}

	return isize ;
}
/* end subroutine (strstore_indsize) */


/* make an index table of the string table */
int strstore_indmk(STRSTORE *op,int (*it)[3],int itsize,int nskip)
{
	VECOBJ		ses ;
	const int	esize = sizeof(STRENTRY) ;
	int		rs ;
	int		rs1 ;
	int		il ;
	int		isize ;
	int		opts ;
	int		sc = 0 ;

#if	CF_SAFE1
	if (op == NULL) return SR_FAULT ;
#endif

#if	CF_SAFE2
	if (op->magic != STRSTORE_MAGIC) return SR_NOTOPEN ;
#endif

	if (nskip < 0)
	    nskip = 0 ;

	il = indexlen(op->c+1) ;

	isize = indexsize(il) ;

	if (itsize < isize)
	    return SR_OVERFLOW ;

/* start in */

	memset(it,0,isize) ;

	opts = VECOBJ_OCOMPACT ;
	if ((rs = vecobj_start(&ses,esize,op->c,opts)) >= 0) {
	    struct strentry	se, *sep ;
	    HDB_CUR	cur ;
	    HDB_DATUM	key, val ;
	    uint	khash, chash, nhash ;
	    int		lhi, nhi, hi, si ;
	    int		c ;

	    if ((rs = hdb_curbegin(&op->smgr,&cur)) >= 0) {
		int		sl ;
		int		*ip ;
		const char	*sp ;

	        while ((sl = hdb_enum(&op->smgr,&cur,&key,&val)) >= 0) {

	            sp = (const char *) key.buf ;
	            sl = key.len ;

	            ip = (int *) val.buf ;
	            si = *ip ;
	            khash = hashelf(sp,sl) ;

	            hi = hashindex(khash,il) ;

	            if (it[hi][0] == 0) {
	                it[hi][0] = si ;
	                it[hi][1] = (khash & INT_MAX) ;
	                it[hi][2] = 0 ;
	                sc += 1 ;
	            } else {
	                se.khash = khash ;
	                se.si = si ;
	                se.hi = hi ;
	                rs = vecobj_add(&ses,&se) ;
	            } /* end if */

	            if (rs < 0) break ;
	        } /* end while (looping through strings) */

	        hdb_curend(&op->smgr,&cur) ;
	    } /* end if (cursor) */

	    if (rs >= 0) {
		int	i ;

	        for (i = 0 ; vecobj_get(&ses,i,&sep) >= 0 ; i += 1) {

	            khash = sep->khash ;
	            si = sep->si ;
	            hi = sep->hi ;

	            chash = (khash & INT_MAX) ;
	            nhash = khash ;

	            c = 0 ;
	            while (it[hi][0] > 0) {

	                if ((it[hi][1] & INT_MAX) == chash)
	                    break ;

	                it[hi][1] |= (~ INT_MAX) ;
	                nhash = hashagain(nhash,c,nskip) ;

	                hi = hashindex(nhash,il) ;

	                c += 1 ;

	            } /* end while */

	            sc += c ;
	            if (it[hi][0] > 0) {

	                lhi = hi ;
	                while ((nhi = it[lhi][2]) > 0) {
	                    lhi = nhi ;
			}

	                hi = hashindex((lhi + 1),il) ;

	                while (it[hi][0] > 0) {
	                    hi = hashindex((hi + 1),il) ;
			}
	                it[lhi][2] = hi ;

	            } /* end while */

	            it[hi][0] = si ;
	            it[hi][1] = chash ;
	            it[hi][2] = 0 ;

	        } /* end for */

	        it[il][0] = -1 ;
	        it[il][1] = 0 ;
	        it[il][2] = 0 ;

	        if (sc < 0)
	            sc = 0 ;

	    } /* end if */

	    rs1 = vecobj_finish(&ses) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vechand) */

	return (rs >= 0) ? sc : rs ;
}
/* end subroutine (strstore_indmk) */


/* private subroutines */


static int strstore_chunknew(STRSTORE *op,int amount)
{
	STRSTORE_CHUNK	*cep ;
	const int	csize = sizeof(STRSTORE_CHUNK) ;
	int		rs ;

	if (op->chunksize > amount)
	    amount = op->chunksize ;

	if ((rs = uc_malloc(csize,&cep)) >= 0) {
	    if ((rs = chunk_start(cep,(amount + 1))) >= 0) {
	        if ((rs = vechand_add(&op->chunks,cep)) >= 0) {
	    	    op->ccp = cep ;
	            if (op->totalsize == 0) {
	                chunk_adv(cep) ;
	                op->totalsize = 1 ;
		    }
		}
		if (rs < 0)
		    chunk_finish(cep) ;
	    }
	    if (rs < 0)
	        uc_free(cep) ;
	} /* end if (memory-allocations) */

	return rs ;
}
/* end subroutine (strstore_chunknew) */


static int strstore_chunkfins(STRSTORE *op)
{
	STRSTORE_CHUNK	*ccp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vechand_get(&op->chunks,i,&ccp) >= 0 ; i += 1) {
	    if (ccp != NULL) {
	        rs1 = chunk_finish(ccp) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(ccp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (strstore_chunkfins) */


static int strstore_manage(STRSTORE *op,cchar *kp,int kl,int si)
{
	int		rs = SR_OK ;
	int		*ip ;

	if ((rs = lookaside_get(&op->imgr,&ip)) >= 0) {
	    HDB_DATUM	key, val ;

	    *ip = si ;
	    key.buf = kp ;
	    key.len = kl ;
	    val.buf = ip ;
	    val.len = sizeof(int *) ;
	    rs = hdb_store(&op->smgr,key,val) ;
	    if (rs < 0)
	        lookaside_release(&op->imgr,ip) ;

	} /* end if */

	return rs ;
}
/* end subroutine (strstore_manage) */


static int chunk_start(STRSTORE_CHUNK *cnp,int csize)
{
	int		rs = SR_OK ;

	memset(cnp,0,sizeof(STRSTORE_CHUNK)) ;

	if (csize > 0) {
	    cnp->csize = csize ;
	    rs = uc_malloc(csize,&cnp->cdata) ;
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (chunk_start) */


static int chunk_adv(STRSTORE_CHUNK *cnp)
{

	cnp->cdata[0] = '\0' ;
	cnp->i += 1 ;
	return SR_OK ;
}
/* end subroutine (chunk_adv) */


static int chunk_finish(STRSTORE_CHUNK *cnp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (cnp->cdata != NULL) {
	    rs1 = uc_free(cnp->cdata) ;
	    if (rs >= 0) rs = rs1 ;
	    cnp->cdata = NULL ;
	}

	cnp->csize = 0 ;
	cnp->i = 0 ;
	cnp->c = 0 ;
	return rs ;
}
/* end subroutine (chunk_finish) */


static int indexlen(int n)
{
	return nextpowtwo(n) ;
}
/* end subroutine (indexlen) */


static int indexsize(int il)
{
	return ((il + 1) * 3 * sizeof(int)) ;
}
/* end subroutine (indexsize) */


/* calculate the next hash from a given one */
static int hashindex(uint hv,int n)
{
	int		hi = MODP2(hv,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */


