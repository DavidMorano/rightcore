/* dbi */

/* object to access both NODEDB and CLUSTERDB objects */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1988-06-01, David A­D­ Morano
	This subroutine was originally written (and largely forgotten).

	= 1993-02-01, David A­D­ Morano
	This subroutine was adopted for use in the RUNADVICE program.

	= 1996-07-01, David A­D­ Morano
	This subroutine was adopted for use in the REXECD daemon program.

	= 2004-05-25, David A­D­ Morano
	This subroutine was adopted for use as a general key-value file reader.

*/

/* Copyright © 1988,1993,1996,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This hack just keeps some interactions with a NODEDB object and a
        CLUSTERDB object together. These two objects are often accessed together
        when attempting to determine a current default cluster name.


*******************************************************************************/


#define	DBI_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<ids.h>
#include	<localmisc.h>

#include	"dbi.h"


/* local defines */

#ifndef	NODEFNAME
#define	NODEFNAME	"etc/node"
#endif

#ifndef	CLUSTERFNAME
#define	CLUSTERFNAME	"etc/cluster"
#endif


/* external subroutines */

extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	isNotPresent(int) ;
extern int	isNotAccess(int) ;
extern int	isFailOpen(int) ;

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int dbi_nodebegin(DBI *,IDS *,cchar *) ;
static int dbi_nodeend(DBI *) ;
static int dbi_clusterbegin(DBI *,IDS *,cchar *) ;
static int dbi_clusterend(DBI *) ;


/* local variables */


/* exported subroutines */


int dbi_open(DBI *op,cchar *pr)
{
	IDS		id ;
	int		rs ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(DBI)) ;
	if ((rs = ids_load(&id)) >= 0) {
	    if ((rs = dbi_nodebegin(op,&id,pr)) >= 0) {
	        rs = dbi_clusterbegin(op,&id,pr) ;
	        if (rs < 0)
	            dbi_nodeend(op) ;
	    } /* end if (node-db) */
	    rs1 = ids_release(&id) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (ids) */

	return rs ;
}
/* end subroutine (dbi_open) */


int dbi_close(DBI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	rs1 = dbi_nodeend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = dbi_clusterend(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (dbi_close) */


int dbi_getclusters(DBI *dbip,vecstr *slp,cchar *nodename)
{
	const int	nrs = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

	if (dbip == NULL) return SR_FAULT ;
	if (slp == NULL) return SR_FAULT ;
	if (nodename == NULL) return SR_FAULT ;

	if (nodename[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS && 0
	debugprintf("dbi_getclusters: entered \n") ;
	debugprintf("dbi_getclusters: nodename=%s\n",nodename) ;
#endif

	if (dbip->open.node) {
	    NODEDB	*nop = &dbip->node ;
	    NODEDB_CUR	cur ;
	    NODEDB_ENT	ste ;

	    if ((rs = nodedb_curbegin(nop,&cur)) >= 0) {
	        const int	elen = NODEDB_ENTLEN ;
	        char		ebuf[NODEDB_ENTLEN + 1] ;

	        while (rs >= 0) {

#if	CF_DEBUGS && 0
	            debugprintf("dbi_getclusters: node whiletop\n") ;
#endif

	            rs1 = nodedb_fetch(nop,nodename,&cur,&ste,ebuf,elen) ;
	            if (rs1 == nrs) break ;
	            rs = rs1 ;

	            if (rs >= 0) {
	                if ((ste.clu != NULL) && (ste.clu[0] != '\0')) {

#if	CF_DEBUGS && 0
	                    debugprintf("dbi_getclusters: cluster=%s\n",
	                        ste.clu) ;
#endif

	                    if ((rs = vecstr_find(slp,ste.clu)) == nrs) {
	                        c += 1 ;
	                        rs = vecstr_add(slp,ste.clu,-1) ;
	                    }

	                } /* end if (got one) */
	            } /* end if (ok) */

#if	CF_DEBUGS && 0
	            debugprintf("dbi_getclusters: node whilebot\n") ;
#endif

	        } /* end while (looping through node-db entries) */

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: node whileout\n") ;
#endif

	        rs1 = nodedb_curend(nop,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */

	} /* end if (DB lookup) */

#if	CF_DEBUGS && 0
	debugprintf("dbi_getclusters: middle\n") ;
#endif

/* try the CLUSTER table if we have one */

	if ((rs >= 0) && dbip->open.cluster) {
	    CLUSTERDB		*cop = &dbip->cluster ;
	    CLUSTERDB_CUR	cur ;

#if	CF_DEBUGS && 0
	    debugprintf("dbi_getclusters: cluster lookup\n") ;
#endif

	    if ((rs = clusterdb_curbegin(cop,&cur)) >= 0) {
	        const int	clen = NODENAMELEN ;
	        char		cbuf[NODENAMELEN + 1] ;

	        while (rs >= 0) {

#if	CF_DEBUGS && 0
	            debugprintf("dbi_getclusters: cluster whiletop\n") ;
#endif

	            rs1 = clusterdb_fetchrev(cop,nodename,&cur,cbuf,clen) ;
	            if (rs1 == nrs) break ;
	            rs = rs1 ;

	            if (rs >= 0) {
	                if ((rs = vecstr_find(slp,cbuf)) == nrs) {
	                    c += 1 ;
	                    rs = vecstr_add(slp,cbuf,-1) ;
	                }
	            } /* end if (ok) */

#if	CF_DEBUGS && 0
	            debugprintf("dbi_getclusters: cluster whilebot\n") ;
#endif

	        } /* end while (reverse fetch) */

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: cluster whileout\n") ;
#endif

	        rs1 = clusterdb_curend(cop,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cluster-cursor) */

	} /* end if */

#if	CF_DEBUGS && 0
	debugprintf("dbi_getclusters: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dbi_getclusters) */


int dbi_getnodes(DBI *dbip,vecstr *clp,vecstr *nlp)
{
	CLUSTERDB	*cop ;
	CLUSTERDB_CUR	cc ;
	const int	clen = NODENAMELEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		cl ;
	int		c = 0 ;
	const char	*cp ;
	char		cbuf[NODENAMELEN + 1] ;

	if (dbip == NULL) return SR_FAULT ;
	if (clp == NULL) return SR_FAULT ;
	if (nlp == NULL) return SR_FAULT ;

	cop = &dbip->cluster ;
	for (i = 0 ; vecstr_get(clp,i,&cp) >= 0 ; i += 1) {
	    if (cp != NULL) {

#if	CF_DEBUGS
	        debugprintf("dbi_getnodes: attached "
	            "cluster=%s\n", cp) ;
#endif

	        if ((rs = clusterdb_curbegin(cop,&cc)) >= 0) {
	            const int	nrs = SR_NOTFOUND ;

	            while (rs >= 0) {

	                cl = clusterdb_fetch(cop,cp,&cc,cbuf,clen) ;
	                if (cl == SR_NOTFOUND) break ;
	                rs = cl ;

#if	CF_DEBUGS
	                debugprintf("dbi_getnodes: cluster "
	                    "cl=%d node=%s\n", cl,cbuf) ;
#endif

	                if (rs >= 0) {
	                    if ((rs = vecstr_findn(nlp,cbuf,cl)) == nrs) {
	                        c += 1 ;
	                        rs = vecstr_add(nlp,cbuf,cl) ;
	                    }
	                } /* end if (ok) */

	            } /* end while (looping through cluster entries) */

	            rs1 = clusterdb_curend(cop,&cc) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (clusterdb-cursor) */

	    }
	    if (rs < 0) break ;
	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dbi_getnodes) */


/* private subroutines */


static int dbi_nodebegin(DBI *op,IDS *idp,cchar *pr)
{
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,pr,NODEFNAME)) >= 0) {
	    USTAT	sb ;
	    if ((rs = uc_stat(tbuf,&sb)) >= 0) {
	        if ((rs = sperm(idp,&sb,R_OK)) >= 0) {
	            NODEDB	*ndp = &op->node ;
	            if ((rs = nodedb_open(ndp,tbuf)) >= 0) {
	                op->open.node = TRUE ;
	            }
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	}
	return rs ;
}
/* end subroutine (dbi_nodebegin) */


static int dbi_nodeend(DBI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->open.node) {
	    NODEDB	*ndp = &op->node ;
	    op->open.node = FALSE ;
	    rs1 = nodedb_close(ndp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (dbi_nodeend) */


static int dbi_clusterbegin(DBI *op,IDS *idp,cchar *pr)
{
	int		rs ;
	char		tbuf[MAXPATHLEN+1] ;
	if ((rs = mkpath2(tbuf,pr,CLUSTERFNAME)) >= 0) {
	    USTAT	sb ;
	    if ((rs = uc_stat(tbuf,&sb)) >= 0) {
	        if ((rs = sperm(idp,&sb,R_OK)) >= 0) {
	            CLUSTERDB	*ndp = &op->cluster ;
	            if ((rs = clusterdb_open(ndp,tbuf)) >= 0) {
	                op->open.cluster = TRUE ;
	            }
	        } else if (isNotAccess(rs)) {
	            rs = SR_OK ;
	        }
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	}
	return rs ;
}
/* end subroutine (dbi_clusterbegin) */


static int dbi_clusterend(DBI *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->open.cluster) {
	    CLUSTERDB	*ndp = &op->cluster ;
	    op->open.cluster = FALSE ;
	    rs1 = clusterdb_close(ndp) ;
	    if (rs >= 0) rs = rs1 ;
	}
	return rs ;
}
/* end subroutine (dbi_clusterend) */


