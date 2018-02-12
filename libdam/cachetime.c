/* cachetime */

/* cache-time manager */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2004-09-10, David A­D­ Morano
	I created this from hacking something that was similar that was
	originally created for a PCS program.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages directory lists by:

	+ ensuring unique entries by name
	+ ensuring unique entries by dev-inode pair

	This object is multithread-safe.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"cachetime.h"


/* local defines */

#define	CACHETIME_ENT		struct cachetime_e
#define	CACHETIME_NENTRY	400


/* external subroutines */

extern uint	hashelf(const void *,int) ;

extern int	sncpy1(char *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */


/* forward references */

static int	cachetime_lookuper(CACHETIME *,cchar *,int,time_t *) ;

static int	entry_start(CACHETIME_ENT *,cchar *,int) ;
static int	entry_finish(CACHETIME_ENT *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;


/* local variables */


/* exported subroutines */


int cachetime_start(CACHETIME *op)
{
	const int	n = CACHETIME_NENTRY ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(CACHETIME)) ;

	if ((rs = hdb_start(&op->db,n,1,NULL,NULL)) >= 0) {
	    if ((rs = ptm_create(&op->m,NULL)) >= 0) {
		op->magic = CACHETIME_MAGIC ;
	    }
	    if (rs < 0)
		hdb_finish(&op->db) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("cachetime_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cachetime_start) */


int cachetime_finish(CACHETIME *op)
{
	CACHETIME_ENT	*ep ;
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != CACHETIME_MAGIC) return SR_NOTOPEN ;

	rs1 = ptm_destroy(&op->m) ;
	if (rs >= 0) rs = rs1 ;

/* loop freeing up the entries */

	if ((rs1 = hdb_curbegin(&op->db,&cur)) >= 0) {
	    while (hdb_enum(&op->db,&cur,&key,&val) >= 0) {
	        ep = (CACHETIME_ENT *) val.buf ;
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
		rs1 = uc_free(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end while */
	    rs1 = hdb_curend(&op->db,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */
	if (rs >= 0) rs = rs1 ;

/* free up the whole DB container */

	rs1 = hdb_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("cachetime_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (cachetime_finish) */


int cachetime_lookup(CACHETIME *op,cchar *sp,int sl,time_t *timep)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("cachetime_lookup: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (op->magic != CACHETIME_MAGIC) return SR_NOTOPEN ;

	if (sl < 0) sl = strlen(sp) ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    rs = cachetime_lookuper(op,sp,sl,timep) ;
	    ptm_unlock(&op->m) ;
	} /* end if (mutex) */

#if	CF_DEBUGS
	debugprintf("cachetime_lookup: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cachetime_lookup) */


int cachetime_curbegin(CACHETIME *op,CACHETIME_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CACHETIME_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(CACHETIME_CUR)) ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    rs = hdb_curbegin(&op->db,&curp->cur) ;
	    if (rs < 0)
		ptm_unlock(&op->m) ;
	} /* end if (mutex-locked) */

	return rs ;
}
/* end subroutine (cachetime_curbegin) */


int cachetime_curend(CACHETIME *op,CACHETIME_CUR *curp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CACHETIME_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(CACHETIME_CUR)) ;

	rs1 = hdb_curend(&op->db,&curp->cur) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = ptm_unlock(&op->m) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (cachetime_curend) */


int cachetime_enum(CACHETIME *op,CACHETIME_CUR *curp,
	char *pbuf,int plen,time_t *timep)
{
	CACHETIME_ENT	*ep ;
	HDB_DATUM	key, val ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != CACHETIME_MAGIC) return SR_NOTOPEN ;

	if ((rs = hdb_enum(&op->db,&curp->cur,&key,&val)) >= 0) {
	    ep = (CACHETIME_ENT *) val.buf ;
	    if ((rs = sncpy1(pbuf,plen,ep->name)) >= 0) {
	        if (timep != NULL) {
	            *timep = ep->mtime ;
		}
	    }
	}

	return rs ;
}
/* end subroutine (cachetime_enum) */


int cachetime_stats(CACHETIME *op,CACHETIME_STATS *statp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (statp == NULL) return SR_FAULT ;

	if (op->magic != CACHETIME_MAGIC) return SR_NOTOPEN ;

	if ((rs = ptm_lock(&op->m)) >= 0) {
	    statp->req = op->c_req ;
	    statp->hit = op->c_hit ;
	    statp->miss = op->c_miss ;
	    ptm_unlock(&op->m) ;
	} /* end if (mutex) */

#if	CF_DEBUGS
	debugprintf("cachetime_stats: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (cachetime_stats) */


/* private subroutines */


static int cachetime_lookuper(CACHETIME *op,cchar *sp,int sl,time_t *timep)
{
	CACHETIME_ENT	*ep ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		f_hit = FALSE ;

	key.buf = sp ;
	key.len = sl ;

/* now see if it is already in the list by NAME */

	op->c_req += 1 ;
	if ((rs = hdb_fetch(&op->db,key,NULL,&val)) >= 0) {
	    op->c_hit += 1 ;
	    ep = (CACHETIME_ENT *) val.buf ;
	    if (timep != NULL) *timep = ep->mtime ;
	    f_hit = TRUE ;
	} else if (rs == SR_NOTFOUND) {
	    const int	size = sizeof(CACHETIME_ENT) ;
	    if ((rs = uc_malloc(size,&ep)) >= 0) {
	        if ((rs = entry_start(ep,sp,sl)) >= 0) {
	    	    key.buf = ep->name ;
	    	    key.len = strlen(ep->name) ;
	            val.buf = ep ;
	    	    val.len = size ;
	    	    if ((rs = hdb_store(&op->db,key,val)) >= 0) {
	    		op->c_miss += 1 ;
	    		if (timep != NULL) *timep = ep->mtime ;
		    }
		    if (rs < 0) {
			entry_finish(ep) ;
		    }
	        } /* end if (entry) */
		if (rs < 0) {
		    uc_free(ep) ;
		}
	    } /* end if (memory-allocation) */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("cachetime_lookup: ret rs=%d f_hit=%u\n",rs,f_hit) ;
#endif

	return (rs >= 0) ? f_hit : rs ;;
}
/* end subroutine (cachetime_lookup) */


static int entry_start(CACHETIME_ENT *ep,cchar np[],int nl)
{
	int		rs ;
	const char	*cp ;

	memset(ep,0,sizeof(CACHETIME_ENT)) ;

	if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(cp,&sb)) >= 0) {
		ep->name = cp ;
	        ep->mtime = sb.st_mtime ;
	    }
	    if (rs < 0) {
	        uc_free(cp) ;
	    }
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(CACHETIME_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep->name != NULL) {
	    rs1 = uc_free(ep->name) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->name = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


