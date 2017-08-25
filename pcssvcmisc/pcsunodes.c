/* pcsunodes */

/* PCS user-nodes */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-09-10, Dave Morano
	This program was originally written.

*/

/* Copyright © 2000 David Morano.  All rights reserved. */

/*******************************************************************************

	This object manages the list of user-nodes.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecpstr.h>
#include	<localmisc.h>

#include	"pcsunodes.h"


/* local defines */

#define	PCSUNODES_FNAME		"etc/usernodes"


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matcasestr(cchar **,cchar *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vecpstr_loadfile(vecpstr *,int,cchar *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	pcsunodes_mktab(PCSUNODES *,VECPSTR *) ;
static int	pcsunodes_load(PCSUNODES *,vecpstr *,cchar **,char *) ;


/* local variables */


/* exported subroutines */


int pcsunodes_start(PCSUNODES *op,cchar *pr)
{
	int		rs ;
	int		rs1 ;
	cchar		*ufn = PCSUNODES_FNAME ;
	char		ufname[MAXPATHLEN+1] ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (pr[0] == '\0') return SR_INVALID ;
	memset(op,0,sizeof(PCSUNODES)) ;

	if ((rs = mkpath2(ufname,pr,ufn)) >= 0) {
	    vecpstr	un ;
	    const int	f = TRUE ;
	    if ((rs = vecpstr_start(&un,0,0,0)) >= 0) {
	        if ((rs = vecpstr_loadfile(&un,f,ufname)) >= 0) {
		    if ((rs = pcsunodes_mktab(op,&un)) >= 0) {
			op->magic = PCSUNODES_MAGIC ;
		    }
		}
		rs1 = vecpstr_finish(&un) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (vecpstr) */
	    if (rs < 0) {
		pcsunodes_finish(op) ;
	    }
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (pcsunodes_start) */


int pcsunodes_finish(PCSUNODES *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (op->magic != PCSUNODES_MAGIC) return SR_NOTOPEN ;

	if (op->unodes != NULL) {
	    rs1 = uc_free(op->unodes) ;
	    if (rs >= 0) rs = rs1 ;
	    op->unodes = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pcsunodes_finish) */


int pcsunodes_get(PCSUNODES *op,int i,cchar **rpp)
{
	int		rs = SR_OK ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != PCSUNODES_MAGIC) return SR_NOTOPEN ;
	if ((i >= 0) && (i < op->n)) {
	    if (rpp != NULL) *rpp = op->unodes[i] ;
	    rs = strlen(op->unodes[i]) ;
	} else {
	    if (rpp != NULL) *rpp = NULL ;
	    rs = SR_INVALID ;
	}
	return rs ;
}
/* end subroutine (pcsunodes_get) */


int pcsunodes_mat(PCSUNODES *op,cchar *mp,int ml)
{
	int		rs ;
	if (op == NULL) return SR_FAULT ;
	if (op->magic != PCSUNODES_MAGIC) return SR_NOTOPEN ;
	if ((rs = matcasestr(op->unodes,mp,ml)) < 0) {
	    rs = SR_NOTFOUND ;
	}
	return rs ;
}
/* end subroutine (pcsunodes_mat) */


int pcsunodes_curbegin(PCSUNODES *op,PCSUNODES_CUR *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != PCSUNODES_MAGIC) return SR_NOTOPEN ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (pcsunodes_curbegin) */


int pcsunodes_curend(PCSUNODES *op,PCSUNODES_CUR *curp)
{
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (op->magic != PCSUNODES_MAGIC) return SR_NOTOPEN ;
	curp->i = -1 ;
	return SR_OK ;
}
/* end subroutine (pcsunodes_curend) */


int pcsunodes_enum(PCSUNODES *op,PCSUNODES_CUR *curp,char *rbuf,int rlen)
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	int		i ;
	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (op->magic != PCSUNODES_MAGIC) return SR_NOTOPEN ;
	i = (curp->i >= 0) ? (curp->i+1) : 0 ;
	if (i < op->n) {
	    if ((rs = sncpy1(rbuf,rlen,op->unodes[i])) >= 0) {
	        rl = rs ;
		curp->i = i ;
	    }
	} else {
	    rbuf[0] = '\0' ;
	    rs = SR_NOTFOUND ;
	}
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (pcsunodes_enum) */


int pcsunodes_audit(PCSUNODES *op)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != PCSUNODES_MAGIC) return SR_NOTOPEN ;
	return SR_OK ;
}
/* end subroutine (pcsunodes_mat) */


/* local subroutines */


static int pcsunodes_mktab(PCSUNODES *op,VECPSTR *ulp)
{
	int		rs ;
	int		c = 0 ;
	if ((rs = vecpstr_count(ulp)) >= 0) {
	    const int	vsize = ((rs+1)*sizeof(cchar *)) ;
	    if ((rs = vecpstr_strsize(ulp)) >= 0) {
		const int	ssize = rs ;
	        int		tsize = (vsize+rs) ;
		char		*bp ;
	        if ((rs = uc_malloc(tsize,&bp)) >= 0) {
	            cchar	**va = (cchar **) bp ;
		    char	*st = (bp + vsize) ;
	            if ((rs = vecpstr_strmk(ulp,st,ssize)) >= 0) {
			if ((rs = pcsunodes_load(op,ulp,va,st)) >= 0) {
			    op->n = rs ;
			    op->unodes = va ;
			}
	            } /* end if (record-table allocated) */
		    if (rs < 0) {
			uc_free(bp) ;
		    }
	        } /* end if (m-a) */
	    } /* end if (vecpstr_strsize) */
	} /* end if (vecpstr_count) */

#if	CF_DEBUGS
	debugprintf("pcsunodes_mktab: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (pcsunodes_mktab) */


static int pcsunodes_load(PCSUNODES *op,vecpstr *ulp,cchar **va,char *st)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = vecpstr_recsize(ulp)) >= 0) {
	    const int	rsize = rs ;
	    int		*rec ;
	    if ((rs = uc_malloc(rsize,&rec)) >= 0) {
	        if ((rs = vecpstr_recmk(ulp,rec,rsize)) >= 0) {
		    const int	n = rs ;
		    int		i ;
		    for (i = 0 ; (i < n) && (rec[i] >= 0) ; i += 1) {
		        if (rec[i] > 0) {
#if	CF_DEBUGS
			    debugprintf("pcsunodes_load: i=%u c=%u v=%s\n",
				i,c,(st+rec[i])) ;
#endif
			    va[c++] = (st + rec[i]) ;
		        }
		    } /* end for */
	        } /* end if */
	        va[c] = NULL ;
	        rs1 = uc_free(rec) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a-f) */
	} /* end if (vecpstr_recsize) */
#if	CF_DEBUGS
	debugprintf("pcsunodes_load: ret rs=%d c=%u\n",rs,c) ;
#endif
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (pcsunodes_load) */


