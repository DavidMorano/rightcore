/* envlist */

/* environment variable management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 1998-12-01, David A­D­ Morano

	Module was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object just provides a hash list of environment-like (with a key
        and an associated value) variables.


*******************************************************************************/


#define	ENVLIST_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<hdb.h>
#include	<localmisc.h>

#include	"envlist.h"


/* local defines */

#define	ENVLIST_DEFENT	10

#define	ENVLIST_DBINIT(op,n,at,hash,cmp)	\
					hdb_start((op),(n),(at),(hash),(cmp))
#define	ENVLIST_DBSTORE(op,k,v)		hdb_store((op),(k),(v))
#define	ENVLIST_DBFETCH(op,k,c,vp)	hdb_fetch((op),(k),(c),(vp))
#define	ENVLIST_DBCOUNT(op)		hdb_count((op))
#define	ENVLIST_DBFREE(op)		hdb_finish((op))

#define	ENVLIST_DBDATA			HDB_DATUM


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


int envlist_start(ENVLIST *op,int n)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("envlist_start: ent op=%p\n",op) ;
	debugprintf("envlist_start: n=%d\n",n) ;
#endif

	rs = ENVLIST_DBINIT(op,n,0,NULL,NULL) ;

	return rs ;
}
/* end subroutine (envlist_start) */


/* free up the entire list object structure */
int envlist_finish(ENVLIST *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	rs = ENVLIST_DBFREE(op) ;

	return rs ;
}
/* end subroutine (envlist_finish) */


int envlist_addkeyval(ENVLIST *op,cchar *kp,cchar *vp,int vl)
{
	int		rs = SR_OK ;
	int		kl ;
	int		size = 1 ;
	char		*ep ;

	if (op == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	kl = strlen(kp) ;

	size += (kl+1) ;
	if (vp != NULL) {
	    if (vl < 0) vl = strlen(vp) ;
	    size += vl ;
	}

	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    int		el ;
	    char	*bp = ep ;
	    bp = strwcpy(bp,kp,kl) ;
	    *bp++ = '=' ;
	    bp = strwcpy(bp,vp,vl) ;
	    el = (bp-ep) ;
	    rs = envlist_add(op,ep,el) ;
	    uc_free(ep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (envlist_addkeyval) */


int envlist_add(ENVLIST *op,cchar *sp,int sl)
{
	ENVLIST_DBDATA	key, value ;
	int		rs ;
	int		kl ;
	const char	*tp ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	kl = sl ;
	if ((tp = strnchr(sp,sl,'=')) != NULL) kl = (tp-sp) ;

#if	CF_DEBUGS
	debugprintf("envlist_add: k=>%t<\n",sp,kl) ;
#endif

	key.buf = sp ;
	key.len = kl ;
	value.buf = sp ;
	value.len = sl ;

	rs = ENVLIST_DBSTORE(op,key,value) ;

#if	CF_DEBUGS
	debugprintf("envlist_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (envlist_add) */


/* return the count of the number of items in this list */
int envlist_count(ENVLIST *op)
{
	int		rs ;

	rs = ENVLIST_DBCOUNT(op) ;

	return rs ;
}
/* end subroutine (envlist_count) */


/* search for an entry in the list */
int envlist_present(ENVLIST *op,cchar *sp,int sl,cchar **rpp)
{
	ENVLIST_DBDATA	key, value ;
	int		rs ;
	int		kl ;
	int		vl = 0 ;
	const char	*tp ;

	if (op == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (sl < 0) sl = strlen(sp) ;

	kl = sl ;
	if ((tp = strnchr(sp,sl,'=')) != NULL) kl = (tp-sp) ;

#if	CF_DEBUGS
	debugprintf("envlist_present: k=>%t<\n",sp,kl) ;
#endif

	key.buf = sp ;
	key.len = kl ;

	rs = ENVLIST_DBFETCH(op,key,NULL,&value) ;
	if (rs >= 0) vl = value.len ;

	if (rpp != NULL)
	    *rpp = (rs >= 0) ? (const char *) value.buf : NULL ;

#if	CF_DEBUGS
	if (rs >= 0) {
	    sp = (const char *) value.buf ;
	    sl = value.len ;
	    debugprintf("envlist_present: v=>%t<\n",
		sp,strlinelen(sp,sl,50)) ;
	}
	debugprintf("envlist_present: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (envlist_present) */


