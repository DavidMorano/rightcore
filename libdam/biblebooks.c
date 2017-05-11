/* biblebooks */

/* BIBLEBOOKS object implementation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the BIBLEBOOKS datbase.


*******************************************************************************/


#define	BIBLEBOOKS_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecpstr.h>
#include	<localmisc.h>

#include	"biblebooks.h"


/* local defines */

#define	BIBLEBOOKS_DBNAME	"english"
#define	BIBLEBOOKS_DBDNAME	"share/biblebooks"
#define	BIBLEBOOKS_DBTITLE	"Bible"
#define	BIBLEBOOKS_DEFENTS	67


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	vecpstr_loadfile(vecpstr *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* exported variables */

BIBLEBOOKS_OBJ	biblebooks = {
	"biblebooks",
	sizeof(BIBLEBOOKS),
	sizeof(int)
} ;


/* local variables */


/* exported subroutines */


int biblebooks_open(BIBLEBOOKS *op,cchar pr[],cchar dbname[])
{
	const int	n = BIBLEBOOKS_DEFENTS ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if ((dbname == NULL) || (dbname[0] == '\0')) {
	    dbname = BIBLEBOOKS_DBNAME ;
	}

	if ((rs = vecpstr_start(&op->db,(n*14),n,0)) >= 0) {
	    cchar	*dname = BIBLEBOOKS_DBDNAME ;
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath3(tbuf,pr,dname,dbname)) >= 0) {
		if ((rs = vecpstr_loadfile(&op->db,FALSE,tbuf)) >= 0) {
		    op->magic = BIBLEBOOKS_MAGIC ;
		    rs = vecpstr_count(&op->db) ;
		}
	    }
	    if (rs < 0) {
	        vecpstr_finish(&op->db) ;
		op->magic = 0 ;
	    }
	} /* end if (vecpstr_start) */

	return rs ;
}
/* end subroutine (biblebooks_open) */


/* free up the entire vector string data structure object */
int biblebooks_close(BIBLEBOOKS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOKS_MAGIC) return SR_NOTOPEN ;

	rs1 = vecpstr_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (biblebooks_close) */


int biblebooks_count(BIBLEBOOKS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOKS_MAGIC) return SR_NOTOPEN ;

	rs = vecpstr_count(&op->db) ;

	return rs ;
}
/* end subroutine (biblebooks_count) */


int biblebooks_max(BIBLEBOOKS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOKS_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecpstr_count(&op->db)) > 0) {
	    rs -= 1 ;
	}

	return rs ;
}
/* end subroutine (biblebooks_max) */


int biblebooks_audit(BIBLEBOOKS *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOKS_MAGIC) return SR_NOTOPEN ;

	rs = vecpstr_audit(&op->db) ;

	return rs ;
}
/* end subroutine (biblebooks_audit) */


int biblebooks_lookup(BIBLEBOOKS *op,char *rbuf,int rlen,int bi)
{
	int		rs ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOKS_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("biblebooks_get: bi=%d\n",bi) ;
#endif

	if ((rs = vecpstr_get(&op->db,bi,&cp)) >= 0) {
	    rs = sncpy1(rbuf,rlen,cp) ;
	}

#if	CF_DEBUGS
	debugprintf("biblebooks_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblebooks_lookup) */


int biblebooks_get(BIBLEBOOKS *op,int bi,char *rbuf,int rlen)
{
	return biblebooks_lookup(op,rbuf,rlen,bi) ;
}
/* end subroutine (biblebooks_get) */


int biblebooks_size(BIBLEBOOKS *op)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != BIBLEBOOKS_MAGIC) return SR_NOTOPEN ;
	return vecpstr_strsize(&op->db) ;
}
/* end subroutine (biblebooks_size) */


