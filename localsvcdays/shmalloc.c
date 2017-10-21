/* shmalloc */

/* Shared-Memory-Allocation management */


#define	CF_DEBUGS 	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object manages shared-memory allocation in a shared-memory segment.

	Synopsis:
	int shmalloc_init(op,strp,size)
	SHMALLOC	*op ;
	char		*strp ;
	int		size ;

	Arguments:
	- op		object pointer
	- strp		pointer to string table (free-memory area)
	- size		size of free-memory area

	Returns:
	>=0		OK
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<intceil.h>
#include	<intfloor.h>
#include	<localmisc.h>

#include	"shmalloc.h"


/* local defines */

#define	SHMVER		1		/* implementation version */


/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	cfdecui(const char *,int,uint *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int shmalloc_init(SHMALLOC *op,char *strp,int size)
{
	ulong		strv = (ulong) strp ;
	const int	asize = SHMALLOC_ALIGNSIZE ;
	int		rs = SR_OK ;
	char		*rp = (char *) op ;
#if	CF_DEBUGS
	debugprintf("shmalloc_init: ent v=%u size=%u\n",SHMVER,size) ;
#endif
	if (op == NULL) return SR_FAULT ;
	if (strp == NULL) return SR_FAULT ;
	op->used = 0 ;
	if ((strv&(asize-1)) != 0) {
	    const int	diff = (strv- (strv&(asize-1))) ;
	    strp += diff ;
	    size -= diff ;
	}
	size = ifloor(size,asize) ;
	if (size > 0) {
	    op->str = (strp-rp) ;
	    op->b.size = size ;
	    op->b.next = 0 ;
	    {
	        SHMALLOC_B	*fsp = (SHMALLOC_B *) strp ;
	        fsp->size = size ;
	        fsp->next = -1 ;
	    }
	} else {
	    rs = SR_INVALID ;
	}
#if	CF_DEBUGS
	debugprintf("shmalloc_init: ret op->b.size=%d\n",op->b.size) ;
	debugprintf("shmalloc_init: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (shmalloc_init) */


int shmalloc_fini(SHMALLOC *op)
{
	if (op == NULL) return SR_FAULT ;
	return SR_OK ;
}
/* end subroutine (shmalloc_finish) */


/* returns <0 (SR_NOMEM) on NOMEM */
int shmalloc_alloc(SHMALLOC *op,int rsize)
{
	SHMALLOC_B	*psp = &op->b ;
	SHMALLOC_B	*fsp, *nsp ;
	const int	asize = SHMALLOC_ALIGNSIZE ; /* also overhead amount */
	int		bsize = iceil(rsize,SHMALLOC_ALIGNSIZE) ;
	int		next ;
	int		roff = SR_NOMEM ;
	char		*rp = (char *) op ;
	char		*strp ;
#if	CF_DEBUGS
	debugprintf("shmalloc_alloc: ent v=%u rsize=%u\n",SHMVER,rsize) ;
#endif
	if (rsize <= 0) return SR_INVALID ;
	strp = (char *) (rp+op->str) ;
	bsize += asize ; /* add the amount for our overhead */
	next = psp->next ;
#if	CF_DEBUGS
	debugprintf("shmalloc_alloc: bsize=%d next=%d\n",bsize,next) ;
	debugprintf("shmalloc_alloc: head=%d:%d\n",op->b.size,op->b.next) ;
#endif
	while ((next >= 0) && (next < op->b.size)) {
	    fsp = (SHMALLOC_B *) (strp+next) ;
#if	CF_DEBUGS
	    debugprintf("shmalloc_alloc: b=%d:%d\n",fsp->size,fsp->next) ;
#endif
	    if (fsp->size >= bsize) {
	        roff = (next+asize) ;
	        if ((fsp->size - bsize) >= SHMALLOC_BLOCKSIZE) {
#if	CF_DEBUGS
	            debugprintf("shmalloc_alloc: t1\n") ;
#endif
	            nsp = (SHMALLOC_B *) (strp+next+bsize) ;
	            nsp->size = (fsp->size-bsize) ;
	            nsp->next = fsp->next ;
	            psp->next = (next+bsize) ; /* update previous block->next */
	            fsp->size = bsize ;		/* update ourself */
	            fsp->next = -1 ;
	        } else {
#if	CF_DEBUGS
	            debugprintf("shmalloc_alloc: t2\n") ;
#endif
	            psp->next = fsp->next ;
	            fsp->next = -1 ;
	        } /* end if */
	        op->used += fsp->size ;
	        break ;
	    } else {
	        psp = fsp ;
	        next = fsp->next ;
	    } /* end if */
	} /* end while (searching for free block) */
#if	CF_DEBUGS
	debugprintf("shmalloc_alloc: used=%d\n",op->used) ;
	debugprintf("shmalloc_alloc: ret roff=%d\n",roff) ;
#endif
	return roff ;
}
/* end subroutine (shmalloc_alloc) */


int shmalloc_free(SHMALLOC *op,int uoff)
{
	SHMALLOC_B	*psp = &op->b ;
	SHMALLOC_B	*fsp, *csp, *nsp ;
	const int	asize = SHMALLOC_ALIGNSIZE ;
	int		rs = SR_OK ;
	int		fsize ;
	int		foff ;
	int		coff ;
	char		*rp = (char *) op ;
	char		*strp ;
	strp = (char *) (rp+op->str) ;
	if (uoff < asize) return SR_INVALID ;
	foff = (uoff-asize) ;
#if	CF_DEBUGS
	debugprintf("shmalloc_free: ent uo=%d fo=%u\n",uoff,foff) ;
	debugprintf("shmalloc_free: u=%u\n",op->used) ;
#endif
	fsp = (SHMALLOC_B *) (strp+foff) ;
#if	CF_DEBUGS
	debugprintf("shmalloc_free: f=%d:%d:%d\n",foff,fsp->size,fsp->next) ;
#endif
	coff = psp->next ;
	fsize = fsp->size ;
#if	CF_DEBUGS
	debugprintf("shmalloc_free: fsize=%u coff=%d\n",fsize,coff) ;
#endif
	if (fsize <= op->used) {
	    while ((coff >= 0) && (coff < op->b.size)) {
	        csp = (SHMALLOC_B *) (strp+coff) ;
#if	CF_DEBUGS
	        debugprintf("shmalloc_free: c=%d:%d:%d\n",
	            coff,csp->size,csp->next) ;
#endif
	        if (foff < coff) {
#if	CF_DEBUGS
	            debugprintf("shmalloc_free: t1\n") ;
#endif
	            if ((foff+fsp->size) == coff) { /* merge forward */
#if	CF_DEBUGS
	                debugprintf("shmalloc_free: t1a\n") ;
#endif
	                psp->next = foff ;
	                fsp->size += csp->size ;
	                fsp->next = csp->next ;
	            } else { /* free alone */
#if	CF_DEBUGS
	                debugprintf("shmalloc_free: t1b\n") ;
#endif
	                psp->next = foff ;
	                fsp->next = coff ;
	            }
	            op->used -= fsize ;
	            foff = -1 ;
	        } else if (foff == (coff+csp->size)) { /* merge backward */
#if	CF_DEBUGS
	            debugprintf("shmalloc_free: t2\n") ;
#endif
	            csp->size += fsp->size ;
	            if ((coff+csp->size) == csp->next) { /* nerge forward */
	                nsp = (SHMALLOC_B *) (strp+csp->next) ;
	                csp->size += nsp->size ;
	                csp->next = nsp->next ;
	            }
	            op->used -= fsize ;
	            foff = -1 ;
	        } /* end if */
	        if (foff >= 0) {
#if	CF_DEBUGS
	            debugprintf("shmalloc_free: continue\n") ;
#endif
	            psp = csp ;
	            coff = csp->next ;
	        }
	        if (foff < 0) break ;
	    } /* end while */
	    if (foff >= 0) {
#if	CF_DEBUGS
	        debugprintf("shmalloc_free: special foff=%u\n",foff) ;
#endif
	        psp->next = foff ;
	        op->used -= fsize ;
	    }
	} else {
	    rs = SR_NOMSG ;
	}
#if	CF_DEBUGS
	debugprintf("shmalloc_free: ret rs=%d u=%u\n",rs,op->used) ;
#endif
	return (rs >= 0) ? op->used : rs ;
}
/* end subroutine (shmalloc_free) */


int shmalloc_already(SHMALLOC *op,int uoff)
{
	SHMALLOC_B	*psp = &op->b ;
	SHMALLOC_B	*fsp, *csp ;
	const int	asize = SHMALLOC_ALIGNSIZE ;
	int		rs = SR_OK ;
	int		foff ;
	int		fsize ;
	int		coff ;
	int		f = FALSE ;
	char		*rp = (char *) op ;
	char		*strp ;
	strp = (char *) (rp+op->str) ;
	if (uoff < asize) return SR_INVALID ;
	foff = (uoff-asize) ;
#if	CF_DEBUGS
	debugprintf("shmalloc_already: ent uo=%d fo=%u\n",uoff,foff) ;
	debugprintf("shmalloc_already: u=%u\n",op->used) ;
#endif
	fsp = (SHMALLOC_B *) (strp+foff) ;
#if	CF_DEBUGS
	debugprintf("shmalloc_already: f=%d:%d:%d\n",
	    foff,fsp->size,fsp->next) ;
#endif
	fsize = fsp->size ;
	coff = psp->next ;
	if (coff < fsize) {
	    while ((coff >= 0) && (coff < op->b.size)) {
	        csp = (SHMALLOC_B *) (strp+coff) ;
#if	CF_DEBUGS
	        debugprintf("shmalloc_already: c=%d:%d:%d\n",
	            coff,csp->size,csp->next) ;
#endif
	        f = ((foff >= coff) && (foff < (coff+csp->size))) ;
	        if (f) break ;

	        coff = csp->next ;
	    } /* end while */
	} /* end if */
#if	CF_DEBUGS
	debugprintf("shmalloc_already: ret rs=%d f=%u\n",rs,f) ;
#endif
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (shmalloc_already) */


int shmalloc_used(SHMALLOC *op)
{
	return op->used ;
}
/* end subroutine (shmalloc_used) */


int shmalloc_avail(SHMALLOC *op)
{
	SHMALLOC_B	*psp = &op->b ;
	SHMALLOC_B	*csp ;
	int		rs = SR_OK ;
	int		coff ;
	int		csize ;
	int		avail = 0 ;
	char		*rp = (char *) op ;
	char		*strp ;
	strp = (char *) (rp+op->str) ;
	coff = psp->next ;
	while (coff >= 0) {
	    csp = (SHMALLOC_B *) (strp+coff) ;
	    csize = csp->size ;
#if	CF_DEBUGS
	    debugprintf("shmalloc_avail: c=%d:%d:%d\n",coff,csize,csp->next) ;
#endif
	    avail += csize ;
	    coff = csp->next ;
	    psp = csp ;
	} /* end while */
	return (rs >= 0) ? avail : rs ;
}
/* end subroutine (shmalloc_avail) */


int shmalloc_audit(SHMALLOC *op)
{
	int		rs ;
	if ((rs = shmalloc_avail(op)) >= 0) {
#if	CF_DEBUGS
	    debugprintf("shmalloc_audit: u=%d a=%d\n",op->used,rs) ;
#endif
	    if ((op->used+rs) != op->b.size) rs = SR_BADFMT ;
	}
#if	CF_DEBUGS
	debugprintf("shmalloc_audit: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (shmalloc_audit) */


