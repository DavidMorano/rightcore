/* mapstrs */

/* environment variable management */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-12-01, David A­D­ Morano
	Module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object just provides a hash list of environment-like (with a key
        and an associated value) variables.

	Note (important): Although this container object allows for a delete
	of an entry, the storage that was used for that entry is not itself
	deleted.  This is sort of a storage trade-off on how it is expected
	this object will be used (not a lot of deletions).


*******************************************************************************/


#define	MAPSTRS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<strpack.h>
#include	<localmisc.h>

#include	"mapstrs.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int mapstrs_start(MAPSTRS *op,int n)
{
	const int	at = TRUE ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mapstrs_start: ent op=%p\n",op) ;
	debugprintf("mapstrs_start: n=%d\n",n) ;
#endif

	memset(op,0,sizeof(MAPSTRS)) ;

	if ((rs = hdb_start(&op->list,n,at,NULL,NULL)) >= 0) {
	    STRPACK	*spp = &op->store ;
	    const int	csize = MAPSTRS_CHUNKSIZE ;
	    rs = strpack_start(spp,csize) ;
	    if (rs < 0)
		hdb_finish(&op->list) ;
	}

	return rs ;
}
/* end subroutine (mapstrs_start) */


/* free up the entire list object structure */
int mapstrs_finish(MAPSTRS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	{
	    STRPACK	*spp = &op->store ;
	    rs1 = strpack_finish(spp) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = hdb_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mapstrs_finish) */


int mapstrs_add(MAPSTRS *op,cchar *kp,int kl,cchar *vp,int vl)
{
	HDB		*hlp = &op->list ;
	HDB_DATUM	key, val ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;

	key.buf = kp ;
	key.len = kl ;

	if ((rs = hdb_fetch(hlp,key,NULL,&val)) >= 0) {
	    rs = INT_MAX ;
	} else if (rs == SR_NOTFOUND) {
	    STRPACK	*spp = &op->store ;
	    cchar	*rkp ;
	    if ((rs = strpack_store(spp,kp,kl,&rkp)) >= 0) {
		cchar	*rvp ;
	        if (vp == NULL) {
		    vp = kp ;
		    vl = 0 ;
	        }
	        if ((rs = strpack_store(spp,vp,vl,&rvp)) >= 0) {
		    key.buf = rkp ;
		    val.buf = rvp ;
		    val.len = vl ;
		    rs = hdb_store(hlp,key,val) ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (mapstrs_add) */


int mapstrs_del(MAPSTRS *op,cchar *kp,int kl)
{
	HDB		*hlp = &op->list ;
	HDB_DATUM	key ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;

	key.buf = kp ;
	key.len = kl ;
	rs = hdb_delkey(hlp,key) ;

	return rs ;
}
/* end subroutine (mapstrs_del) */


/* return the count of the number of items in this list */
int mapstrs_count(MAPSTRS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = hdb_count(&op->list) ;

	return rs ;
}
/* end subroutine (mapstrs_count) */


/* search for an entry in the list */
int mapstrs_present(MAPSTRS *op,cchar *kp,int kl,cchar **rpp)
{
	HDB_DATUM	key, val ;
	int		rs ;
	int		vl = 0 ;
	const char	*tp ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	if (kl < 0) kl = strlen(kp) ;

	if ((tp = strnchr(kp,kl,'=')) != NULL) kl = (tp-kp) ;

#if	CF_DEBUGS
	debugprintf("mapstrs_present: k=>%t<\n",kp,kl) ;
#endif

	key.buf = kp ;
	key.len = kl ;

	if ((rs = hdb_fetch(&op->list,key,NULL,&val)) >= 0) {
	    vl = val.len ;
	    if (rpp != NULL) {
		*rpp = (cchar *) val.buf ;
	    }
	} else {
	    if (rpp != NULL) *rpp = NULL ;
	}

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mapstrs_present) */


