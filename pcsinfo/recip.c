/* recip */

/* recipient object for DMAIL¦DMAILBOX */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_MAXNAMELEN	1		/* use MAXNAMELEN */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is an recipil address handling module object. It can parse out and
        store hierarchically organized RECIPs.


*******************************************************************************/


#define	RECIP_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecitem.h>
#include	<localmisc.h>

#include	"recip.h"


/* local defines */


/* external subroutines */

extern int	strwcmp(const char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int recip_start(RECIP *op,cchar *np,int nl)
{
	int		rs ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("recip_start: ent name=%t\n",np,nl) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	memset(op,0,sizeof(RECIP)) ;
	op->uid = -1 ;

#if	CF_MAXNAMELEN
	if (nl > MAXNAMELEN) nl = MAXNAMELEN ;
#endif

	if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	    op->recipient = cp ;
	    if ((rs = vecitem_start(&op->mds,10,0)) >= 0) {
	        op->magic = RECIP_MAGIC ;
	    }
	    if (rs < 0)
	        uc_free(cp) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("recip_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (recip_start) */


int recip_finish(RECIP *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;

	if (op->recipient != NULL) {
	    rs1 = uc_free(op->recipient) ;
	    if (rs >= 0) rs = rs1 ;
	    op->recipient = NULL ;
	    if (op->name != NULL) {
	        rs1 = uc_free(op->name) ;
	        if (rs >= 0) rs = rs1 ;
	        op->name = NULL ;
	    }
	    if (op->maildname != NULL) {
	        rs1 = uc_free(op->maildname) ;
	        if (rs >= 0) rs = rs1 ;
	        op->maildname = NULL ;
	    }
	    rs1 = vecitem_finish(&op->mds) ;
	    if (rs >= 0) rs = rs1 ;
	} else
	    rs = SR_BUGCHECK ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (recip_finish) */


/* get the recipient name */
int recip_get(RECIP *op,const char **rpp)
{
	int		rs = SR_OK ;
	const char	*rp = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("recip_get: n=%s\n",op->recipient) ;
#endif

	if (op->recipient != NULL) {
	    rp = op->recipient ;
	    rs = strlen(rp) ;
	} else
	    rs = SR_NOTOPEN ;

	if (rpp != NULL)
	    *rpp = rp ;

#if	CF_DEBUGS
	debugprintf("recip_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (recip_get) */


/* set that this is an actual user */
int recip_setuser(RECIP *op,uid_t uid)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	op->f.user = TRUE ;
	op->uid = uid ;
	return SR_OK ;
}
/* end subroutine (recip_setuser) */


/* set the user's real-name (if she has one) */
int recip_setname(RECIP *op,cchar *np,int nl)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	if (np != NULL) {
	    cchar	*cp ;
	    if (nl < 0) nl = strlen(np) ;
	    if (op->name != NULL) {
	        uc_free(op->name) ;
	        op->name = NULL ;
	    }
	    if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	        op->name = cp ;
	    } /* end if (m-a) */
	}
	return rs ;
}
/* end subroutine (recip_setuser) */


/* set the user's mail-spool directory (if she has one) */
int recip_setmailspool(RECIP *op,cchar *np,int nl)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	if (np != NULL) {
	    cchar	*cp ;
	    if (nl < 0) nl = strlen(np) ;
	    if (op->maildname != NULL) {
	        uc_free(op->maildname) ;
	        op->maildname = NULL ;
	    }
	    if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	        rs = nl ;
	        op->maildname = cp ;
	    } /* end if (m-a) */
	}
	return rs ;
}
/* end subroutine (recip_setmailspool) */


/* set mail-box offset */
int recip_mbo(RECIP *op,int mbo)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	op->mbo = mbo ;
	return SR_OK ;
}
/* end subroutine (recip_mbo) */


/* set message-file offset-length */
int recip_mo(RECIP *op,int moff,int mlen)
{
	RECIP_ENT	mo ;
	const int	esize = sizeof(RECIP_ENT) ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;

#ifdef	OPTIONAL
	memset(&mo,0,sizeof(RECIP_ENT)) ;
#endif

	mo.moff = moff ;
	mo.mlen = mlen ;
	op->n += 1 ;
	rs = vecitem_add(&op->mds,&mo,esize) ;

	return rs ;
}
/* end subroutine (recip_mo) */


/* set delivery status */
int recip_ds(RECIP *op,int ds)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	op->ds = ds ;
	return SR_OK ;
}
/* end subroutine (recip_ds) */


int recip_match(RECIP *op,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;

	if (np[0] != '\0') {
#if	CF_MAXNAMELEN
	    if (nl > MAXNAMELEN) nl = MAXNAMELEN ;
#endif
	    if (op->recipient != NULL) {
	        f = (strwcmp(op->recipient,np,nl) == 0) ;
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (recip_match) */


int recip_getmbo(RECIP *op)
{
	int		rs = SR_OK ;
	int		mbo = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;

	if (op->ds >= 0) {
	    mbo = (op->mbo & INT_MAX) ;
	} else {
	    rs = op->ds ;
	}

	return (rs >= 0) ? mbo : rs ;
}
/* end subroutine (recip_getmbo) */


int recip_getmo(RECIP *op,int i,int *offp)
{
	int		rs = SR_OK ;
	int		mo = 0 ;
	int		ml = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;

	if (op->ds >= 0) {
	    RECIP_ENT	*ep ;
	    if ((rs = vecitem_get(&op->mds,i,&ep)) >= 0) {
	        mo = ep->moff ;
	        ml = ep->mlen ;
	    }
	} else {
	    rs = op->ds ;
	}

	if (offp != NULL) {
	    *offp = (rs >= 0) ? mo : 0 ;
	}

	return (rs >= 0) ? ml : rs ;
}
/* end subroutine (recip_getmo) */


/* get the user name (if available) */
int recip_getname(RECIP *op,cchar **rpp)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	if (rpp != NULL ) {
	    *rpp = op->name ;
	}
	if (op->name != NULL) {
	    rs = strlen(op->name) ;
	}
	return rs ;
}
/* end subroutine (recip_getname) */


/* get the user mail-spool directory (if available) */
int recip_getmailspool(RECIP *op,cchar **rpp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	if (rpp != NULL) {
	    *rpp = op->maildname ;
	}
	if (op->maildname != NULL) {
	    rs = strlen(op->maildname) ;
	}
	return rs ;
}
/* end subroutine (recip_getmailspool) */


/* get the user state */
int recip_isuser(RECIP *op)
{
	int	f ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	f = op->f.user ;
	return f ;
}
/* end subroutine (recip_isuser) */


/* get the user state */
int recip_getuser(RECIP *op,uid_t *up)
{
	int	f ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != RECIP_MAGIC) return SR_NOTOPEN ;
	if (up != NULL) *up = op->uid ;
	f = op->f.user ;
	return f ;
}
/* end subroutine (recip_getuser) */


