/* offindex */

/* offset-index object */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object is used to create a line-index of the TXTINDEX "tag" file.
	The line-index consists of file offsets of the beginning of each line
	in the file.  Each line corresponds with a "tag" record.   The length
	of each "record" is also stored.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"offindex.h"


/* local defines */

#define	OFFINDEX_E	struct offindex_e

#define	NDEF		100


/* external subroutines */


/* external variables */


/* local structures */

struct offindex_e {
	offset_t	lineoff ;
	int		linelen ;
} ;


/* forward references */

static int	vecmp(struct offindex_e **,struct offindex_e **) ;


/* local variables */


/* exported subroutines */


int offindex_start(OFFINDEX *op,int n)
{
	const int	size = sizeof(OFFINDEX_E) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (n < NDEF)
	    n = NDEF ;

	memset(op,0,sizeof(OFFINDEX)) ;

	if ((rs = vecobj_start(&op->list,size,n,0)) >= 0) {
	    op->magic = OFFINDEX_MAGIC ;
	}

	return rs ;
}
/* end subroutine (offindex_start) */


int offindex_finish(OFFINDEX *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != OFFINDEX_MAGIC) return SR_NOTOPEN ;

	rs1 = vecobj_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (offindex_finish) */


int offindex_add(OFFINDEX *op,offset_t off,int len)
{
	OFFINDEX_E	e ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != OFFINDEX_MAGIC) return SR_NOTOPEN ;

	e.lineoff = off ;
	e.linelen = len ;
	rs = vecobj_add(&op->list,&e) ;

	return rs ;
}
/* end subroutine (offindex_add) */


int offindex_lookup(OFFINDEX *op,offset_t off)
{
	OFFINDEX_E	*oep ;
	OFFINDEX_E	key ;
	int		(*vcmp)(void *,void *) ;
	int		rs = SR_NOSYS ;
	int		len = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != OFFINDEX_MAGIC) return SR_NOTOPEN ;

	if (! op->f.setsorted) {
	    op->f.setsorted = TRUE ;
	    vecobj_setsorted(&op->list) ;
	}

	key.lineoff = off ;
	key.linelen = 0 ;
	vcmp = (int (*)(void *,void *)) vecmp ;
	if ((rs = vecobj_search(&op->list,&key,vcmp,&oep)) >= 0) {
	    rs = SR_BADFMT ;
	    if (oep != NULL) {
	        rs = SR_OK ;
	        len = oep->linelen ;
	    }
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (offindex_lookup) */


/* private subroutines */


static int vecmp(OFFINDEX_E **e1pp,OFFINDEX_E **e2pp)
{
	int		rc ;

	if (*e1pp != NULL) {
	    if (*e2pp != NULL) {
	        rc = ((*e1pp)->lineoff - (*e2pp)->lineoff) ;
	    } else
	        rc = -1 ;
	} else
	    rc = 1 ;

	return rc ;
}
/* end subroutine (vecmp) */


