/* keys */

/* handle the keys while processing a file */


#define	CF_DEBUGS 	0		/* non-switchable debug print-outs */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 1994-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a set of subroutines for managing keys while processing an input
        file (of them).


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<ptm.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;



/* external variables */


/* forward references */

int	keys_ender(PROGINFO *,HDB *,bfile *,PTM *,cchar *,offset_t,int) ;


/* local variables */


/* exported subroutines */


int keys_begin(PROGINFO *pip,HDB *dbp,int hashsize)
{
	const int	f = FALSE ;
	int		rs ;

	if (pip == NULL) return SR_FAULT ;

	rs = hdb_start(dbp,hashsize,f,NULL,NULL) ;

	return rs ;
}
/* end subroutine (keys_begin) */


int keys_add(PROGINFO *pip,HDB *dbp,cchar *sp,int sl)
{
	HDB_DATUM	key, value, dumbvalue ;
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("keys_add: klen=%d\n",sl) ;
#endif

	if (sl < 0)
	    sl = strlen(sp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("keys_add: key=%t klen=%d\n",sp,sl,sl) ;
#endif

	key.len = sl ;
	key.buf = (void *) sp ;			/* prepare for check */

	value.len = 0 ;
	value.buf = NULL ;

/* if it is already present, we're done, return */

	if ((rs = hdb_fetch(dbp,key,NULL,&dumbvalue)) == nrs) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	        f = TRUE ;
	        key.buf = (void *) cp ;
	        rs = hdb_store(dbp,key,value) ;
	    }
	} /* end if */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("keys_add: add rs=%d f=%u\n",rs,f) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (keys_add) */


int keys_end(pip,dbp,ofp,omp,fname,recoff,reclen)
PROGINFO	*pip ;
HDB		*dbp ;
bfile		*ofp ;
PTM		*omp ;
const char	fname[] ;
offset_t	recoff ;
int		reclen ;
{
	int		rs ;
	int		rs1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("keys_end: ent\n") ;
#endif

	if ((rs = ptm_lock(omp)) >= 0) {

	    rs = keys_ender(pip,dbp,ofp,omp,fname,recoff,reclen) ;

	    rs1 = ptm_unlock(omp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ptm) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("keys_end: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (keys_end) */


/* local subroutines */


/* ARGSUSED */
int keys_ender(pip,dbp,ofp,omp,fname,recoff,reclen)
PROGINFO	*pip ;
HDB		*dbp ;
bfile		*ofp ;
PTM		*omp ;
const char	fname[] ;
offset_t	recoff ;
int		reclen ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		n = 0 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("keys_ender: ent\n") ;
#endif

	if (recoff <= INT_MAX) {
	    HDB_CUR	keycursor ;
	    HDB_DATUM	key, value ;
	    uint	tagoff = recoff ;
	    uint	taglen = reclen ;

	    if ((rs = hdb_curbegin(dbp,&keycursor)) >= 0) {

	        if (hdb_enum(dbp,&keycursor,&key,&value) >= 0) {

	            if (! pip->f.removelabel) {
	                bprintf(ofp,"%s:%u,%u\t",fname,tagoff,taglen) ;
		    }

	            rs = bwrite(ofp, (void *) key.buf,key.len) ;

	            n = 1 ;
	            while ((rs >= 0) && 
	                ((pip->maxkeys < 0) || (n < pip->maxkeys)) && 
	                (hdb_enum(dbp,&keycursor,&key,&value) >= 0)) {

	                if ((rs = bputc(ofp,' ')) >= 0) {
	                    rs = bwrite(ofp, (void *) key.buf,key.len) ;
			}

	                n += 1 ;
	            } /* end while */

	            if (rs >= 0)
	                rs = bputc(ofp,'\n') ;

	        } /* end if */

	        hdb_curend(dbp,&keycursor) ;
	    } /* end if (cursor) */

/* delete this whole DB */

	    if ((rs1 = hdb_curbegin(dbp,&keycursor)) >= 0) {
	        while (hdb_enum(dbp,&keycursor,&key,&value) >= 0) {
	            if (key.buf != NULL) {
	                rs1 = uc_free((void *) key.buf) ;
			if (rs >= 0) rs = rs1 ;
	            }
	        } /* end while */
	        rs1 = hdb_curend(dbp,&keycursor) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	    if (rs >= 0) rs = rs1 ;

	    rs1 = hdb_finish(dbp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (within range) */

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("keys_ender: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (keys_ender) */


