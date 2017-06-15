/* dbi */

/* object to access both NODEDB and CLUSTERDB objects */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1988-06-01, David A­D­ Morano
	This subroutine was originally written (and largely forgotten).

	= 1993-02-01, David A­D­ Morano
	This subroutine was adopted for use in the RUNADVICE program.

	= 1996-07-01, David A­D­ Morano
	This subroutine was adopted for use in the 'rexecd' daemon program.

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
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<nodedb.h>
#include	<clusterdb.h>
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

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dbi_open(DBI *dbip,IDS *idp,cchar *pr)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (dbip == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(dbip,0,sizeof(DBI)) ;

/* store IDs (IS IT OPAQUE ?????????????????????) */
/* this is a POOR excuse for a "copy constructor" !!!!!!!!!!!!!!! */

	memcpy(&dbip->id,idp,sizeof(IDS)) ;

/* node DB */

	rs = mkpath2(tmpfname,pr,NODEFNAME) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS
	debugprintf("dbi_open: nodefname=%s\n",tmpfname) ;
#endif

	rs1 = perm(tmpfname,-1,-1,NULL,R_OK) ;

#if	CF_DEBUGS
	debugprintf("dbi_open: node perm() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0) {
	    rs1 = nodedb_open(&dbip->node,tmpfname) ;
	    dbip->f_node = (rs1 >= 0) ;
	}

/* cluster DB */

	rs = mkpath2(tmpfname,pr,CLUSTERFNAME) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUGS
	debugprintf("dbi_open: cluster1fname=%s\n",tmpfname) ;
#endif

	rs1 = u_stat(tmpfname,&sb) ;

	if ((rs1 >= 0) && S_ISDIR(sb.st_mode))
	    rs1 = SR_ISDIR ;

	if (rs1 >= 0)
	    rs1 = sperm(&dbip->id,&sb,R_OK) ;

	if (rs1 >= 0) {
	    rs1 = clusterdb_open(&dbip->cluster,tmpfname) ;
	    dbip->f_cluster = (rs1 >= 0) ;
	}

/* done */

	rs = dbip->f_node + dbip->f_cluster ;

ret0:

#if	CF_DEBUGS
	debugprintf("dbi_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dbi_open) */


int dbi_close(DBI *dbip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (dbip == NULL) return SR_FAULT ;

	if (dbip->f_cluster) {
	    dbip->f_cluster = FALSE ;
	    rs1 = clusterdb_close(&dbip->cluster) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (dbip->f_node) {
	    dbip->f_node = FALSE ;
	    rs1 = nodedb_close(&dbip->node) ;
	    if (rs >= 0) rs = rs1 ;
	}

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

	if (dbip->f_node) {
	    NODEDB		*nop = &dbip->node ;
	    NODEDB_CUR		cur ;
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

	        nodedb_curend(nop,&cur) ;
	    } /* end if (cursor) */

	} /* end if (DB lookup) */

#if	CF_DEBUGS && 0
	debugprintf("dbi_getclusters: middle\n") ;
#endif

/* try the CLUSTER table if we have one */

	if ((rs >= 0) && dbip->f_cluster) {
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

	        clusterdb_curend(cop,&cur) ;
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
	    if (cp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("dbi_getnodes: attached "
	        "cluster=%s\n",
	        cp) ;
#endif

	    if ((rs = clusterdb_curbegin(cop,&cc)) >= 0) {
		const int	nrs = SR_NOTFOUND ;

	        while (rs >= 0) {

	            cl = clusterdb_fetch(cop,cp,&cc,cbuf,clen) ;
	            if (cl == SR_NOTFOUND) break ;
	            rs = cl ;

#if	CF_DEBUGS
	            debugprintf("dbi_getnodes: cluster "
	                "cl=%d node=%s\n",
	                cl,cbuf) ;
#endif

		    if (rs >= 0) {
	                if ((rs = vecstr_findn(nlp,cbuf,cl)) == nrs) {
	                    c += 1 ;
	                    rs = vecstr_add(nlp,cbuf,cl) ;
	                }
		    } /* end if (ok) */

	        } /* end while (looping through cluster entries) */

	        clusterdb_curend(cop,&cc) ;
	    } /* end if (cursor) */

	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dbi_getnodes) */


