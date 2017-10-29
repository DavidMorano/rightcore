/* requests */
/* lang=C99 */


#define	CF_DEBUGS	0		/* compile-time debugging */


#include	<envstandards.h>
#include	<sys/types.h>
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>
#include	"requests.h"

/* local defines */


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* local structures */


/* forward references */


/* exported subroutines */


/* local subroutines */


int requests_start(REQUESTS *op)
{
	const int	esize = sizeof(REQUESTS_ITEM) ;
	const int	vo = (VECOBJ_OREUSE|VECOBJ_OSWAP) ;
	int		rs ;
	rs = vecobj_start(&op->ents,esize,1000,vo) ;
	return rs ;
}
/* end subroutine (requests_start) */


int requests_finish(REQUESTS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = vecobj_finish(&op->ents) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (requests_finish) */


int requests_get(REQUESTS *op,int i,REQUESTS_ITEM *ip)
{
	REQUESTS_ITEM	*ep ;
	int		rs ;
	memset(ip,0,sizeof(REQUESTS_ITEM)) ;
	if ((rs = vecobj_get(&op->ents,i,&ep)) >= 0) {
	    if (ep != NULL) {
	        *ip = *ep ;
#if	CF_DEBUGS
	debugprintf("requests_get: ro=%d\n",ep->ro) ;
#endif
		rs = ep->ro ;
	    } else {
		ip->ro = -1 ;
		ip->rs = 0 ;
#if	CF_DEBUGS
	debugprintf("requests_get: result=NULL\n") ;
#endif
		rs = INT_MAX ;
	    }
	}
#if	CF_DEBUGS
	debugprintf("requests_get: ret rs=%d\n",rs) ;
#endif
	return rs ;
}
/* end subroutine (requests_get) */


int requests_add(REQUESTS *op,REQUESTS_ITEM *ip)
{
	int		rs ;
	const void	*ep = (const void *) ip ;
	rs = vecobj_add(&op->ents,ep) ;
	return rs ;
}
/* end subroutine (requests_store) */


int requests_del(REQUESTS *op,int i)
{
	int		rs ;
	rs = vecobj_del(&op->ents,i) ;
	return rs ;
}
/* end subroutine (requests_del) */


int requests_count(REQUESTS *op)
{
	int		rs ;
	rs = vecobj_count(&op->ents) ;
	return rs ;
}
/* end subroutine (requests_count) */


