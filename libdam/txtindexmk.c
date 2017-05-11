/* txtindexmk */

/* interface to the TXTINDEXMKS loadable object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that provides
	access to the TXTINDEXMK object (which is dynamically loaded).


*******************************************************************************/


#define	TXTINDEXMK_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"txtindexmk.h"
#include	"txtindexmks.h"


/* local defines */

#define	TXTINDEXMK_DEFENTS	(44 * 1000)

#define	TXTINDEXMK_MODBNAME	"txtindexmks"
#define	TXTINDEXMK_OBJNAME	"txtindexmks"

#define	VARPRNAME		"LOCAL"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN		60
#endif

#undef	TIS_TAG
#define	TIS_TAG			TXTINDEXMKS_TAG

#undef	TIS_KEY
#define	TIS_KEY			TXTINDEXMKS_KEY


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;
extern int	getdomainname(char *,int) ;
extern int	getnodedomain(char *,char *) ;
extern int	mkpr(char *,int,const char *,const char *) ;
extern int	pathclean(char *,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	txtindexmk_objloadbegin(TXTINDEXMK *,cchar *,cchar *) ;
static int	txtindexmk_objloadend(TXTINDEXMK *) ;
static int	txtindexmk_loadcalls(TXTINDEXMK *,const char *) ;

static int	isrequired(int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"addeigens",
	"addtags",
	"close",
	"noop",
	"abort",
	NULL
} ;

enum subs {
	sub_open,
	sub_addeigens,
	sub_addtags,
	sub_close,
	sub_noop,
	sub_abort,
	sub_overlast
} ;


/* exported subroutines */


int txtindexmk_open(TXTINDEXMK *op,TXTINDEXMK_PA *pp,cchar *db,int of,mode_t om)
{
	int		rs ;
	const char	*objname = TXTINDEXMK_OBJNAME ;
	char		dbuf[MAXHOSTNAMELEN+1] ;

#if	CF_DEBUGS
	debugprintf("txtindexmk_open: db=%s\n",db) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (db == NULL) return SR_FAULT ;

	if (db[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(TXTINDEXMK)) ;

	if ((rs = getnodedomain(NULL,dbuf)) >= 0) {
	    const int	plen = MAXPATHLEN ;
	    const char	*pn = VARPRNAME ;
	    char	pbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpr(pbuf,plen,pn,dbuf)) >= 0) {
		const char	*pr = pbuf ;
		if ((rs = txtindexmk_objloadbegin(op,pr,objname)) >= 0) {
	    	    if ((rs = (*op->call.open)(op->obj,pp,db,of,om)) >= 0) {
			op->magic = TXTINDEXMK_MAGIC ;
	    	    }
	    	    if (rs < 0)
			txtindexmk_objloadend(op) ;
		} /* end if (txtindexmk_objloadbegin) */
	    } /* end if (mkpr) */
	} /* end if (getnodedomain) */

#if	CF_DEBUGS
	debugprintf("txtindexmk_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (txtindexmk_open) */


/* free up the entire vector string data structure object */
int txtindexmk_close(TXTINDEXMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEXMK_MAGIC) return SR_NOTOPEN ;

	rs1 = (*op->call.close)(op->obj) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = txtindexmk_objloadend(op) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (txtindexmk_close) */


int txtindexmk_addeigens(TXTINDEXMK *op,TXTINDEXMK_KEY keys[],int nkeys)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEXMK_MAGIC) return SR_NOTOPEN ;

	rs = (*op->call.addeigens)(op->obj,keys,nkeys) ;

	return rs ;
}
/* end subroutine (txtindexmk_addeigens) */


int txtindexmk_addtags(TXTINDEXMK *op,TXTINDEXMK_TAG tags[],int ntags)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEXMK_MAGIC) return SR_NOTOPEN ;

	rs = (*op->call.addtags)(op->obj,tags,ntags) ;

	return rs ;
}
/* end subroutine (txtindexmk_addtags) */


int txtindexmk_noop(TXTINDEXMK *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != TXTINDEXMK_MAGIC) return SR_NOTOPEN ;

	if (op->call.noop != NULL) {
	    rs = (*op->call.noop)(op->obj) ;
	}

	return rs ;
}
/* end subroutine (txtindexmk_noop) */


/* private subroutines */


/* find and load the DB-access object */
static int txtindexmk_objloadbegin(TXTINDEXMK *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	const int	vo = VECSTR_OCOMPACT ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("txtindexmk_objloadbegin: pr=%s\n",pr) ;
	debugprintf("txtindexmk_objloadbegin: objname=%s\n",objname) ;
#endif

	if ((rs = vecstr_start(&syms,n,vo)) >= 0) {
	    const int	nlen = SYMNAMELEN ;
	    int		i ;
	    int		f_modload = FALSE ;
	    char	nbuf[SYMNAMELEN + 1] ;

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
			rs = vecstr_add(&syms,nbuf,rs) ;
		    }
		}
		if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
		const char	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            const char	*mn = TXTINDEXMK_MODBNAME ;
	            const int	mo = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
	            if ((rs = modload_open(lp,pr,mn,objname,mo,sv)) >= 0) {
	    		int	mv[2] ;
			f_modload = TRUE ;
	    		if ((rs = modload_getmva(lp,mv,2)) >= 0) {
			    void	*p ;
			    op->objsize = mv[0] ;
		            op->cursize = mv[1] ;
		            if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		                op->obj = p ;
		                rs = txtindexmk_loadcalls(op,objname) ;
		                if (rs < 0) {
			            uc_free(op->obj) ;
			            op->obj = NULL ;
		                }
		            } /* end if (memory-allocation) */
	                } /* end if (modload_getmva) */
	                if (rs < 0) {
			    f_modload = FALSE ;
		            modload_close(lp) ;
			}
	            } /* end if (modload_open) */
		} /* end if (vecstr_getvec) */
	    } /* end if (ok) */

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload) {
	        modload_close(lp) ;
	    }
	} /* end if (vecstr-syms) */

#if	CF_DEBUGS
	debugprintf("txtindexmk_objloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (txtindexmk_objloadbegin) */


static int txtindexmk_objloadend(TXTINDEXMK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->obj != NULL) {
	    rs1 = uc_free(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    op->obj = NULL ;
	}

	rs1 = modload_close(&op->loader) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (txtindexmk_objloadend) */


static int txtindexmk_loadcalls(TXTINDEXMK *op,cchar objname[])
{
	MODLOAD		*lp = &op->loader ;
	const int	nlen = SYMNAMELEN ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		nbuf[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
	         if ((rs = modload_getsym(lp,nbuf,&snp)) == SR_NOTFOUND) {
		     snp = NULL ;
		     if (! isrequired(i)) rs = SR_OK ;
		}
	    }

	    if (rs < 0) break ;

#if	CF_DEBUGS
	    debugprintf("txtindexmk_loadcalls: call=%s %c\n",
		subs[i],
		((snp != NULL) ? 'Y' : 'N')) ;
#endif

	    if (snp != NULL) {

	        c += 1 ;
		switch (i) {

		case sub_open:
		    op->call.open = (int (*)(void *,TXTINDEXMKS_PA *,
			const char *,int,int)) snp ;
		    break ;

		case sub_addeigens:
		    op->call.addeigens = 
			(int (*)(void *,TIS_KEY *,int)) snp ;
		    break ;

		case sub_addtags:
		    op->call.addtags = 
			(int (*)(void *,TIS_TAG *,int)) snp ;
		    break ;

		case sub_noop:
		    op->call.noop = (int (*)(void *)) snp ;
		    break ;

		case sub_abort:
		    op->call.abort = (int (*)(void *)) snp ;
		    break ;

		case sub_close:
		    op->call.close = (int (*)(void *)) snp ;
		    break ;

		} /* end switch */

	    } /* end if (it had the call) */

	} /* end for (subs) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (txtindexmk_loadcalls) */


static int isrequired(int i)
{
	int		f = FALSE ;

	switch (i) {
	case sub_open:
	case sub_addeigens:
	case sub_addtags:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */

	return f ;
}
/* end subroutine (isrequired) */


