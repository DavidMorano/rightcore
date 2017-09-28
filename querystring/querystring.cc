/* querystring */
/* lang=C++11 */

/* Query-String manager */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_STRNLEN	0		/* use 'strnlen(3dam)' */


/* revision history:

	= 2017-09-25, David A­D­ Morano
	This module was adapted from some previous code.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We take a raw "query-string" and process it so that its components
	can be accessed.


*******************************************************************************/


#define	QUERYSTRING_MASTER	0


#include	<envstandards.h>

#include	<sys/types.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vector>
#include	<pair>
#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>

#include	"querystring.h"


/* local defines */

#define	KEYVAL	struct keyval


/* typedefs */


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct keyval {
	cchar		*k ;
	cchar		*v ;
} ;


/* forward references */


/* local variables */


/* exported subroutines */


int querystring_start(QUERYSTRING *op,cchar *sp,int sl)
{
	vector<keyval>	kv ;
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		rs1 ;
	memset(opm0,sizeof(QUERYSTRING)) ;
	if ((rs = strpack_start(&op->p,llen)) >= 0) {


	    if (rs < 0)
		strpack_finish(&op->p) ;
	} /* end if (strpack_start) */
	return rs ;
}
/* end subroutine (querystring_start) */


int querystring_finish(QUERYSTRING *dbp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = strpack_finish(&op->p) :
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (querystring_finish) */


int querystring_count(QUERYSTRING *op)
{
	return op->n ;
}
/* end subroutine (querystring_count) */


int querystring_already(QUERYSTRING *op,cchar *kstr,int klen)
{
	int		rs ;
	int		v ;
	if ((rs = querystring_fetch(op,kstr,klen,NULL,&v)) >= 0) {
	    rs = (v & INT_MAX) ;
	}
	return rs ;
}
/* end if (querystring_already) */


/* enumerate all of the entries */
int querystring_enum(QUERYSTRING *dbp,QUERYSTRING_CUR *curp,cchar **kpp,int *vp)
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
/* end subroutine (querystring_enum) */


/* fetch the next entry value matching the given key */
int querystring_fetch(dbp,kstr,klen,curp,vp)
QUERYSTRING	*dbp ;
const char	kstr[] ;
int		klen ;
QUERYSTRING_CUR	*curp ;
int		*vp ;
{
	HDB_DATUM	key, val ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("querystring_fetch: ent key=%t\n",kstr,klen) ;
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
	debugprintf("querystring_fetch: hdb_fetch key=%t\n",kstr,klen) ;
#endif

	if ((rs = hdb_fetch(dbp,key,curp,&val)) >= 0) {
	    if (vp != NULL) {
	        int	*ip = (int *) val.buf ;
	        *vp = *ip ;
	    }
	} /* end if (hdb_fetch) */

#if	CF_DEBUGS
	debugprintf("querystring_fetch: ret rs=%d klen=%d\n",rs,klen) ;
#endif

	return (rs >= 0) ? klen : rs ;
}
/* end subroutine (querystring_fetch) */


int querystring_curbegin(QUERYSTRING *dbp,QUERYSTRING_CUR *curp)
{

	return hdb_curbegin(dbp,curp) ;
}
/* end subroutine (querystring_curbegin) */


int querystring_curend(QUERYSTRING *dbp,QUERYSTRING_CUR *curp)
{

	return hdb_curend(dbp,curp) ;
}
/* end subroutine (querystring_curend) */


/* private subroutines */


