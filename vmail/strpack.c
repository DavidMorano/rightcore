/* strpack */

/* string pack object */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_PREALLOC	0		/* pre-allocate a chunk */


/* revision history:

	= 2011-01-12, David A­D­ Morano
        This object module was morphed from some previous one. I do not remember
        what the previous one was.

*/

/* Copyright © 2011 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module creates and manages a string storage object.  This
	object is sort of a write-only storage area for storing strings.
	Strings cannot be deleted from the object once they are added
	(something not commonly needed anyway).  Since most string storage
	requirements do NOT need the ability to remove strings once they are
	entered, this object provides a nice optimized manager for handling
	that sort of situation.

	This object is very similar to a STRTAB object (in that strings cannot
	be deleted) but is somewhat more optimized.

	Arguments:

	op		pointer to the strpack object
	<others>

	Returns:

	>=0		the total length of the filled up strpack so far!
	<0		error


*******************************************************************************/


#define	STRPACK_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vechand.h>
#include	<localmisc.h>

#include	"strpack.h"


/* local defines */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

int		strpack_store(STRPACK *,cchar *,int,cchar **) ;

static int	strpack_chunknew(STRPACK *,int) ;
static int	strpack_chunkfins(STRPACK *) ;

static int	chunk_start(STRPACK_CHUNK *,int) ;
static int	chunk_adv(STRPACK_CHUNK *) ;
static int	chunk_finish(STRPACK_CHUNK *) ;


/* local variables */


/* exported subroutines */


int strpack_start(STRPACK *op,int chunksize)
{
	const int	vo = VECHAND_PORDERED ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (chunksize < STRPACK_CHUNKSIZE)
	    chunksize = STRPACK_CHUNKSIZE ;

	memset(op,0,sizeof(STRPACK)) ;
	op->chunksize = chunksize ;

	if ((rs = vechand_start(&op->chunks,0,vo)) >= 0) {

#if	CF_PREALLOC
	    rs = strpack_chunknew(op,0) ;
#endif /* CF_PREALLOC */

	    if (rs >= 0) {
		op->magic = STRPACK_MAGIC ;
	    }
	    if (rs < 0)
		vechand_finish(&op->chunks) ;
	} /* end if (vechand_start) */

	return rs ;
}
/* end subroutine (strpack_start) */


/* free up this strpack object */
int strpack_finish(STRPACK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRPACK_MAGIC) return SR_NOTOPEN ;

/* pop them */

	rs1 = strpack_chunkfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&op->chunks) ;
	if (rs >= 0) rs = rs1 ;

	op->ccp = NULL ;
	op->magic = 0 ;
	return rs ;
}
/* end subroutine (strpack_finish) */


/* add a character string to this object */
int strpack_store(STRPACK *op,cchar *sp,int sl,cchar **rpp)
{
	STRPACK_CHUNK	*ccp ;
	int		rs = SR_OK ;
	int		amount ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != STRPACK_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

	amount = (sl + 1) ;
	ccp = op->ccp ;
	if ((ccp == NULL) || (amount > (ccp->csize - ccp->i))) {
	    rs = strpack_chunknew(op,amount) ;
	    ccp = op->ccp ;
	}

	if (rs >= 0) { /* chunk-add */
	    char	*bp = (ccp->cdata + ccp->i) ;
	    strwcpy(bp,sp,sl) ;
	    ccp->i += amount ;
	    ccp->c += 1 ;		/* count in chunk */
	    op->c += 1 ;		/* count in object */
	    op->totalsize += amount ;
	    if (rpp != NULL) *rpp = (rs >= 0) ? bp : NULL ;
	}

	return (rs >= 0) ? sl : rs ;
}
/* end subroutine (strpack_store) */


/* get the string count */
int strpack_count(STRPACK *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRPACK_MAGIC) return SR_NOTOPEN ;

	return op->c ;
}
/* end subroutine (strpack_count) */


int strpack_size(STRPACK *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != STRPACK_MAGIC) return SR_NOTOPEN ;

	return op->totalsize ;
}
/* end subroutine (strpack_size) */


/* private subroutines */


static int strpack_chunknew(STRPACK *op,int amount)
{
	STRPACK_CHUNK	*cep ;
	int		rs ;
	int		csize = sizeof(STRPACK_CHUNK) ;

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
		} /* end if (vechand) */
	        if (rs < 0)
	            chunk_finish(cep) ;
	    } /* end if (chunk) */
	    if (rs < 0)
	        uc_free(cep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (strpack_chunknew) */


static int strpack_chunkfins(STRPACK *op)
{
	STRPACK_CHUNK	*ccp ;
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
/* end subroutine (strpack_chunkfins) */


static int chunk_start(STRPACK_CHUNK *cnp,int csize)
{
	int		rs = SR_OK ;

	memset(cnp,0,sizeof(STRPACK_CHUNK)) ;

	if (csize > 0) {
	    cnp->csize = csize ;
	    rs = uc_malloc(csize,&cnp->cdata) ;
	} else
	    rs = SR_INVALID ;

	return rs ;
}
/* end subroutine (chunk_start) */


static int chunk_adv(STRPACK_CHUNK *cnp)
{

	cnp->cdata[0] = '\0' ;
	cnp->i += 1 ;
	return SR_OK ;
}
/* end subroutine (chunk_adv) */


static int chunk_finish(STRPACK_CHUNK *cnp)
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


