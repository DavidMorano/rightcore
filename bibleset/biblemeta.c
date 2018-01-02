/* biblemeta */

/* vector string operations */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written for hardware CAD support.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module implements an interface (a trivial one) that allows access
        to the BIBLEMETA datbase.


*******************************************************************************/


#define	BIBLEMETA_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"biblemeta.h"


/* local defines */

#define	BIBLEMETA_DBNAME	"english"
#define	BIBLEMETA_DBDNAME	"lib/bibleset/metawords"
#define	BIBLEMETA_DBTITLE	"Bible"
#define	BIBLEMETA_DEFENTRIES	67


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */


/* exported variables */

BIBLEMETA_OBJ	biblemeta = {
	"biblemeta",
	sizeof(BIBLEMETA)
} ;


/* local variables */


/* exported subroutines */


int biblemeta_open(op,pr,dbname)
BIBLEMETA	*op ;
const char	pr[] ;
const char	dbname[] ;
{
	const int	n = BIBLEMETA_DEFENTRIES ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if ((dbname == NULL) || (dbname[0] == '\0'))
	    dbname = BIBLEMETA_DBNAME ;

	if ((rs = vecstr_start(&op->db,n,0)) >= 0) {
	    char	tbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath3(tbuf,pr,BIBLEMETA_DBDNAME,dbname)) >= 0) {
	        if ((rs = vecstr_loadfile(&op->db,FALSE,tbuf)) >= 0) {
	            if ((rs = vecstr_count(&op->db)) >= 0) {
	                op->magic = BIBLEMETA_MAGIC ;
	            }
	        } /* end if (vecstr_loadfile) */
	    } /* end if (mkpath) */
	    if (rs < 0)
		vecstr_finish(&op->db) ;
	} /* end if (vecstr_start) */

	return rs ;
}
/* end subroutine (biblemeta_open) */


/* free up the entire vector string data structure object */
int biblemeta_close(op)
BIBLEMETA	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEMETA_MAGIC) return SR_NOTOPEN ;

	rs1 = vecstr_finish(&op->db) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (biblemeta_close) */


int biblemeta_count(op)
BIBLEMETA	*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEMETA_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_count(&op->db) ;

	return rs ;
}
/* end subroutine (biblemeta_count) */


int biblemeta_max(op)
BIBLEMETA	*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEMETA_MAGIC) return SR_NOTOPEN ;

	if ((rs = vecstr_count(&op->db)) > 0) {
	    rs -= 1 ;
	}

	return rs ;
}
/* end subroutine (biblemeta_max) */


int biblemeta_audit(op)
BIBLEMETA	*op ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEMETA_MAGIC) return SR_NOTOPEN ;

	rs = vecstr_audit(&op->db) ;

	return rs ;
}
/* end subroutine (biblemeta_audit) */


/* get a string by its index */
int biblemeta_get(op,i,rbuf,rlen)
BIBLEMETA	*op ;
int		i ;
char		rbuf[] ;
int		rlen ;
{
	int		rs ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != BIBLEMETA_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("biblemeta_get: i=%d\n",i) ;
#endif

	if ((rs = vecstr_get(&op->db,i,&cp)) >= 0) {
	    rs = sncpy1(rbuf,rlen,cp) ;
	}

#if	CF_DEBUGS
	debugprintf("biblemeta_get: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblemeta_get) */


