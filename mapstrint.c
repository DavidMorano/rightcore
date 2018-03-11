/* mapstrint */

/* Map (database) for String-Integer pairs */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_STRNLEN	0		/* use 'strnlen(3dam)' */


/* revision history:

	= 1998-03-12, David A­D­ Morano
	This module was adapted from some previous code.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object maps a string to an integer value using a hash table.
        The underlying hash table is implemented with a HDB object (currently).


*******************************************************************************/


#define	MAPSTRINT_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<localmisc.h>

#include	"mapstrint.h"


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		mapstrint_fetch(MAPSTRINT *,cchar *,int,MAPSTRINT_CUR *,int *) ;

#ifdef	COMMENT
static int	cmpentry(const char *,const char *,int) ;
#endif


/* local variables */


/* exported subroutines */


int mapstrint_start(MAPSTRINT *dbp,int nitems)
{
	int		rs ;

	rs = hdb_start(dbp,nitems,0,NULL,NULL) ;

	return rs ;
}
/* end subroutine (mapstrint_start) */


int mapstrint_count(MAPSTRINT *dbp)
{
	int		rs ;

	rs = hdb_count(dbp) ;

	return rs ;
}
/* end subroutine (mapstrint_count) */


/* delete this whole DB */
int mapstrint_finish(MAPSTRINT *dbp)
{
	MAPSTRINT_CUR	keycursor ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		n = 0 ;

/* delete all items individually (since they are non-opaque) */

	if ((rs1 = hdb_curbegin(dbp,&keycursor)) >= 0) {
	    HDB_DATUM	key, val ;
	    const char	*ep ;
	    while (hdb_enum(dbp,&keycursor,&key,&val) >= 0) {
	        ep = (const char *) val.buf ;
	        if (ep != NULL) {
	            rs1 = uc_free(ep) ;
		    if (rs >= 0) rs = rs1 ;
	        }
	        n += 1 ;
	    } /* end while */
	    hdb_curend(dbp,&keycursor) ;
	} /* end if */
	if (rs >= 0) rs = rs1 ;

/* delete the framework itself */

	rs1 = hdb_finish(dbp) ;
	if (rs >= 0) rs = rs1 ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mapstrint_finish) */


/* add a string-int pair to the database */
int mapstrint_add(MAPSTRINT *dbp,cchar *kstr,int klen,int ival)
{
	int		rs ;
	int		size ;
	int		*ip ;

#if	CF_DEBUGS
	debugprintf("mapstrint_add: ent klen=%d\n",klen) ;
#endif

	if ((klen > 0) && (kstr == NULL))
	    return SR_INVALID ;

#if	CF_STRNLEN
	klen = strnlen(kstr,klen) ;
#else
	if (klen < 0)
	    klen = strlen(kstr) ;
#endif

	size = sizeof(int) + klen + 1 ;
	if ((rs = uc_malloc(size,&ip)) >= 0) {
	    HDB_DATUM	key, val ;
	    char	*bp ;
	    *ip = ival ;
	    bp = ((char *) ip) + sizeof(int) ;
	    strwcpy(bp,kstr,klen) ;
	    key.buf = bp ;
	    key.len = klen ;
	    val.buf = ip ;
	    val.len = sizeof(int) ;
	    rs = hdb_store(dbp,key,val) ;
	    if (rs < 0) {
		uc_free(ip) ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("mapstrint_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mapstrint_add) */


int mapstrint_already(MAPSTRINT *op,cchar *kstr,int klen)
{
	int		rs ;
	int		v ;
	if ((rs = mapstrint_fetch(op,kstr,klen,NULL,&v)) >= 0) {
	    rs = (v & INT_MAX) ;
	}
	return rs ;
}
/* end if (mapstrint_already) */


/* enumerate all of the entries */
int mapstrint_enum(MAPSTRINT *dbp,MAPSTRINT_CUR *curp,cchar **kpp,int *vp)
{
	HDB_DATUM	key, val ;
	int		rs ;
	int		klen = 0 ;

	if (kpp != NULL)
	    *kpp = NULL ;

	if ((rs = hdb_enum(dbp,curp,&key,&val)) >= 0) {
	    klen = key.len ;
	    if (kpp != NULL)
	        *kpp = (char *) key.buf ;
	    if (vp != NULL) {
	        int	*ip = (int *) val.buf ;
	        *vp = *ip ;
	    }
	} /* end if (hdb_enum) */

	return (rs >= 0) ? klen : rs ;
}
/* end subroutine (mapstrint_enum) */


/* fetch the next entry value matching the given key */
int mapstrint_fetch(dbp,kstr,klen,curp,vp)
MAPSTRINT	*dbp ;
const char	kstr[] ;
int		klen ;
MAPSTRINT_CUR	*curp ;
int		*vp ;
{
	HDB_DATUM	key, val ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("mapstrint_fetch: ent key=%t\n",kstr,klen) ;
#endif

#if	CF_STRNLEN
	klen = strnlen(kstr,klen) ;
#else
	if (klen < 0)
	    klen = strlen(kstr) ;
#endif

	key.buf = kstr ;
	key.len = klen ;

#if	CF_DEBUGS
	debugprintf("mapstrint_fetch: hdb_fetch key=%t\n",kstr,klen) ;
#endif

	if ((rs = hdb_fetch(dbp,key,curp,&val)) >= 0) {
	    if (vp != NULL) {
	        int	*ip = (int *) val.buf ;
	        *vp = *ip ;
	    }
	} /* end if (hdb_fetch) */

#if	CF_DEBUGS
	debugprintf("mapstrint_fetch: ret rs=%d klen=%d\n",rs,klen) ;
#endif

	return (rs >= 0) ? klen : rs ;
}
/* end subroutine (mapstrint_fetch) */


/* fetch the next entry value matching the given key */
int mapstrint_fetchrec(dbp,kstr,klen,curp,kpp,vp)
MAPSTRINT	*dbp ;
const char	kstr[] ;
int		klen ;
MAPSTRINT_CUR	*curp ;
const char	**kpp ;
int		*vp ;
{
	HDB_DATUM	key, val ;
	HDB_DATUM	rkey ;
	int		rs ;

#if	CF_STRNLEN
	klen = strnlen(kstr,klen) ;
#else
	if (klen < 0)
	    klen = strlen(kstr) ;
#endif

	key.buf = kstr ;
	key.len = klen ;

	if (kpp != NULL)
	    *kpp = NULL ;

	if ((rs = hdb_fetchrec(dbp,key,curp,&rkey,&val)) >= 0) {
	    if (kpp != NULL)
	        *kpp = (char *) rkey.buf ;
	    if (vp != NULL) {
	        int	*ip = (int *) val.buf ;
	        *vp = *ip ;
	    }
	} /* end if (hdb_fetchrec) */

	return (rs >= 0) ? klen : rs ;
}
/* end subroutine (mapstrint_fetchrec) */


/* get the current record under the cursor */
int mapstrint_getrec(MAPSTRINT *dbp,MAPSTRINT_CUR *curp,cchar **kpp,int *vp)
{
	HDB_DATUM	key, val ;
	int		rs ;
	int		klen = 0 ;

	if (kpp != NULL)
	    *kpp = NULL ;

	if ((rs = hdb_getrec(dbp,curp,&key,&val)) >= 0) {
	    klen = key.len ;
	    if (kpp != NULL)
	        *kpp = (char *) key.buf ;
	    if (vp != NULL) {
	        int	*ip = (int *) val.buf ;
	        *vp = *ip ;
	    }
	} /* end if (hdb_getrec) */

	return (rs >= 0) ? klen : rs ;
}
/* end subroutine (mapstrint_getrec) */


/* advance the cursor to the next entry regardless of key */
int mapstrint_next(MAPSTRINT *dbp,MAPSTRINT_CUR *curp)
{
	int		rs ;

	rs = hdb_next(dbp,curp) ;

	return rs ;
}
/* end subroutine (mapstrint_next) */


/* advance the cursor to the next entry with the given key */
int mapstrint_nextkey(MAPSTRINT *dbp,cchar *kstr,int klen,MAPSTRINT_CUR *curp)
{
	HDB_DATUM	key ;
	int		rs ;

	if (kstr == NULL) return SR_FAULT ;

#if	CF_STRNLEN
	klen = strnlen(kstr,klen) ;
#else
	if (klen < 0) klen = strlen(kstr) ;
#endif

	key.buf = kstr ;
	key.len = klen ;

	rs = hdb_nextrec(dbp,key,curp) ;

	return rs ;
}
/* end subroutine (mapstrint_nextkey) */


/* delete all of the entries that match a key */
int mapstrint_delkey(MAPSTRINT *dbp,cchar *kstr,int klen)
{
	HDB_DATUM	skey, key, val ;
	MAPSTRINT_CUR	keycursor ;
	int		rs ;
	int		rs1 ;
	char		*ep ;

#if	CF_STRNLEN
	klen = strnlen(kstr,klen) ;
#else
	if (klen < 0) klen = strlen(kstr) ;
#endif

	skey.buf = kstr ;
	skey.len = klen ;

/* delete all of the data associated with this key */

	if ((rs = hdb_curbegin(dbp,&keycursor)) >= 0) {

	    while (hdb_fetchrec(dbp,skey,&keycursor,&key,&val) >= 0) {

	        if (hdb_delcur(dbp,&keycursor,1) >= 0) {

	            ep = (char *) val.buf ;
	            if (ep != NULL)
	                uc_free(ep) ;

	        } /* end if */

	        while (hdb_getrec(dbp,&keycursor,&key,&val) >= 0) {

	            if ((skey.len != key.len) ||
	                (strncmp(skey.buf,key.buf,skey.len) != 0))
	                break ;

	            if (hdb_delcur(dbp,&keycursor,0) >= 0) {

	                ep = (char *) val.buf ;
	                if (ep != NULL)
	                    uc_free(ep) ;

	            } /* end if */

	            rs1 = hdb_nextrec(dbp,skey,&keycursor) ;
	            if (rs1 == SR_NOTFOUND) break ;

	        } /* end while */

	    } /* end while */

	    hdb_curend(dbp,&keycursor) ;
	} /* end if (cursor) */

	return rs ;
}
/* end subroutine (mapstrint_delkey) */


/* delete the item under the cursor */
int mapstrint_delcur(MAPSTRINT *dbp,MAPSTRINT_CUR *curp,int f_adv)
{
	HDB_DATUM	key, val ;
	int		rs ;

	if ((rs = hdb_getrec(dbp,curp,&key,&val)) >= 0) {
	    char	*ep = (char *) val.buf ;
	    rs = hdb_delcur(dbp,curp,f_adv)  ;
	    if ((rs >= 0) && (ep != NULL)) {
	        uc_free(ep) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (mapstrint_delcur) */


int mapstrint_curbegin(MAPSTRINT *dbp,MAPSTRINT_CUR *curp)
{

	return hdb_curbegin(dbp,curp) ;
}
/* end subroutine (mapstrint_curbegin) */


int mapstrint_curend(MAPSTRINT *dbp,MAPSTRINT_CUR *curp)
{

	return hdb_curend(dbp,curp) ;
}
/* end subroutine (mapstrint_curend) */


/* set the value for a string (addressed by its cursor) */
int mapstrint_setval(MAPSTRINT *dbp,MAPSTRINT_CUR *curp,int ival)
{
	HDB_DATUM	key, val ;
	int		rs ;
	int		kl = 0 ;

	if (curp == NULL) return SR_INVALID ;

	if ((rs = hdb_getrec(dbp,curp,&key,&val)) >= 0) {
	    int	*ip = (int *) val.buf ;
	    *ip = ival ;
	    kl = key.len ;
	}

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (mapstrint_setval) */


/* private subroutines */


