/* dirseen */

/* directory list manager */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2006-09-10, David A­D­ Morano
	I created this from hacking something that was similar that was
	originally created for a PCS program.

*/

/* Copyright © 2006 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages directory lists by:

	+ ensuring unique entries by name
	+ ensuring unique entries by dev-inode pair


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"dirseen.h"


/* local defines */

#define	DIRSEEN_ENT	struct dirseen_e


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* local structures */

struct dirseen_e {
	const char	*name ;
	uino_t		ino ;
	dev_t		dev ;
	int		namelen ;
	uint		f_stat:1 ;
} ;


/* forward references */

static int vcmpname(DIRSEEN_ENT **,DIRSEEN_ENT **) ;
static int vcmpdevino(DIRSEEN_ENT **,DIRSEEN_ENT **) ;

static int entry_start(DIRSEEN_ENT *,const char *,int,dev_t,uino_t) ;
static int entry_finish(DIRSEEN_ENT *) ;


/* local variables */


/* exported subroutines */


int dirseen_start(DIRSEEN *op)
{
	int		rs ;
	int		vopts ;
	int		size ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(DIRSEEN)) ;

	vopts = 0 ;
	size = sizeof(DIRSEEN_ENT) ;
	if ((rs = vecobj_start(&op->list,size,DIRSEEN_NDEF,vopts)) >= 0) {
	    op->magic = DIRSEEN_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("dirseen_start: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dirseen_start) */


int dirseen_finish(DIRSEEN *op)
{
	DIRSEEN_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	for (i = 0 ; vecobj_get(&op->list,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecobj_finish(&op->list) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (dirseen_finish) */


/* add a directory name to the list */
int dirseen_add(DIRSEEN *op,cchar *dbuf,int dlen,struct ustat *sbp)
{
	DIRSEEN_ENT	e ;
	dev_t		dev = 0 ;
	uino_t		ino = 0 ;
	int		rs ;
	int		pl ;
	const char	*pp ;

	if (op == NULL) return SR_FAULT ;
	if (dbuf == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	if (dlen < 0) dlen = strlen(dbuf) ;

/* any NUL (blank) paths need to be converted to "." */

	pp = dbuf ;
	pl = dlen ;
	if (pl == 0) {
	    pp = "." ;
	    pl = 1 ;
	}

/* enter it */

	if (sbp != NULL) {
	    dev = sbp->st_dev ;
	    ino = sbp->st_ino ;
	}

	if ((rs = entry_start(&e,pp,pl,dev,ino)) >= 0) {
	    op->strsize += rs ;
	    rs = vecobj_add(&op->list,&e) ;
	    if (rs < 0)
		entry_finish(&e) ;
	} /* end if (entry_start) */

#if	CF_DEBUGS
	debugprintf("dirseen_add: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dirseen_add) */


int dirseen_havename(DIRSEEN *op,cchar *np,int nl)
{
	NULSTR		s ;
	int		rs ;
	int		rs1 ;
	cchar		*name = NULL ;

	if (op == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	if ((rs = nulstr_start(&s,np,nl,&name)) >= 0) {
	    {
	        DIRSEEN_ENT	e ;
	        e.name = name ;
	        rs = vecobj_search(&op->list,&e,vcmpname,NULL) ;
	    }
	    rs1 = nulstr_finish(&s) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */

	return rs ;
}
/* end subroutine (dirseen_havename) */


int dirseen_havedevino(DIRSEEN *op,struct ustat *sbp)
{
	DIRSEEN_ENT	e ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	e.dev = sbp->st_dev ;
	e.ino = sbp->st_ino ;
	rs = vecobj_search(&op->list,&e,vcmpdevino,NULL) ;

	return rs ;
}
/* end subroutine (dirseen_havedevino) */


int dirseen_count(DIRSEEN *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	rs = vecobj_count(&op->list) ;

	return rs ;
}
/* end subroutine (dirseen_count) */


int dirseen_curbegin(DIRSEEN *op,DIRSEEN_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(DIRSEEN_CUR)) ;

	return SR_OK ;
}
/* end subroutine (dirseen_curbegin) */


int dirseen_curend(DIRSEEN *op,DIRSEEN_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	memset(curp,0,sizeof(DIRSEEN_CUR)) ;

	return SR_OK ;
}
/* end subroutine (dirseen_curend) */


int dirseen_enum(DIRSEEN *op,DIRSEEN_CUR *curp,char *rbuf,int rlen)
{
	DIRSEEN_ENT	*ep ;
	int		rs ;
	int		i ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != DIRSEEN_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? curp->i : 0 ;

	while ((rs = vecobj_get(&op->list,i,&ep)) >= 0) {
	    if (ep != NULL) break ;
	} /* end while */

	if (rs >= 0) {
	    if ((rs = sncpy1(rbuf,rlen,ep->name)) >= 0) {
	        curp->i = (i + 1) ;
	    }
	}

	return rs ;
}
/* end subroutine (dirseen_enum) */


/* private subroutines */


int entry_start(DIRSEEN_ENT *ep,cchar *np,int nl,dev_t dev,uino_t ino)
{
	int		rs ;
	const char	*cp ;

	memset(ep,0,sizeof(DIRSEEN_ENT)) ;
	ep->dev = dev ;
	ep->ino = ino ;

	if ((rs = uc_mallocstrw(np,nl,&cp)) >= 0) {
	    ep->name = cp ;
	    ep->namelen = (rs-1) ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


int entry_finish(DIRSEEN_ENT *ep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;

	if (ep->name != NULL) {
	    rs1 = uc_free(ep->name) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->name = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


static int vcmpname(DIRSEEN_ENT **e1pp,DIRSEEN_ENT **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = strcmp((*e1pp)->name,(*e2pp)->name) ;
		} else
		    rc = -1 ;
	    } else
		rc = 1 ;
	} /* end if */
	return rc ;
}
/* end subroutine (vcmpname) */


static int vcmpdevino(DIRSEEN_ENT **e1pp,DIRSEEN_ENT **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            if (((rc = (*e1pp)->ino - (*e2pp)->ino)) == 0) {
	                rc = (*e1pp)->dev - (*e2pp)->dev ;
	            }
		} else
		    rc = -1 ;
	    } else
		rc = 1 ;
	} /* end if */
	return rc ;
}
/* end subroutine (vcmpdevino) */


