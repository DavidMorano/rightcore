/* dbi */

/* little object to access both NODEDB and CLUSTERDB objects */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 1988-06-01, David A­D­ Morano
	This subroutine was originally written (and largely forgotten).

	- 1993-02-01, David A­D­ Morano
	This subroutine was adopted for use in the RUNADVICE program.

	- 1996-07-01, David A­D­ Morano
	This subroutine was adopted for use in the 'rexecd' daemon program.

	- 2004-05-25, David A­D­ Morano
	This subroutine was adopted for use as a general key-value file reader.

*/

/* Copyright © 1988,1993,1996,2004 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This hack just keeps some interactions with a NODEDB object and
        a CLUSTERDB object together. These two objects are often
        accessed together when attempting to determine a current default
        cluster name.


******************************************************************************/


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
#include	<time.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<nodedb.h>
#include	<clusterdb.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"dbi.h"

#ifdef	DMALLOC
#include	<dmalloc.h>
#endif


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int dbi_open(dbip,idp,pr)
DBI		*dbip ;
IDS		*idp ;
const char	pr[] ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ;

	char	tmpfname[MAXPATHLEN + 1] ;

	if (dbip == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(dbip,0,sizeof(DBI)) ;

/* store IDs (IS IT OPAQUE ?????????????????????) */
/* this is a POOR excuse for a "copy constructor" !!!!!!!!!!!!!!! */

	memcpy(&dbip->id,idp,sizeof(IDS)) ;

/* node DB */

	mkpath2(tmpfname,pr,NODEFNAME) ;

#if	CF_DEBUGS
	debugprintf("dbi_open: nodefname=%s\n",tmpfname) ;
#endif

	rs1 = perm(tmpfname,-1,-1,NULL,R_OK) ;

#if	CF_DEBUGS
	debugprintf("dbi_open: node perm() rs=%d\n",rs1) ;
#endif

	if (rs1 >= 0)
	    rs1 = nodedb_open(&dbip->node,tmpfname) ;

	dbip->f_node = (rs1 >= 0) ;

/* cluster DB */

	mkpath2(tmpfname,pr,CLUSTERFNAME) ;

#if	CF_DEBUGS
	debugprintf("dbi_open: cluster1fname=%s\n",tmpfname) ;
#endif

	rs1 = u_stat(tmpfname,&sb) ;

	if ((rs1 >= 0) && S_ISDIR(sb.st_mode))
	    rs1 = SR_ISDIR ;

	if (rs1 >= 0)
	    rs1 = sperm(&dbip->id,&sb,R_OK) ;

	if (rs1 >= 0)
	    rs1 = clusterdb_open(&dbip->cluster,tmpfname) ;

	dbip->f_cluster = (rs1 >= 0) ;

/* done */

	rs = dbip->f_node + dbip->f_cluster ;

#if	CF_DEBUGS
	debugprintf("dbi_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (dbi_open) */


int dbi_getclusters(dbip,sp,nodename)
DBI		*dbip ;
vecstr		*sp ;
const char	nodename[] ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	c = 0 ;

	if (dbip == NULL) return SR_FAULT ;
	if (nodename == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;

	if (nodename[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS && 0
	debugprintf("dbi_getclusters: entered \n") ;
	debugprintf("dbi_getclusters: nodename=%s\n",nodename) ;
#endif

	if (dbip->f_node) {
	    NODEDB_CUR	cur ;
	    NODEDB_ENT	ste ;
	    char	ebuf[NODEDB_ENTLEN + 1] ;

	    nodedb_curbegin(&dbip->node,&cur) ;

	    while (TRUE) {

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: node whiletop\n") ;
#endif

	        rs1 = nodedb_fetch(&dbip->node,nodename,&cur,
	            &ste,ebuf,NODEDB_ENTLEN) ;

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: nodedb_fetch() rs=%d\n",rs1) ;
#endif

	        if (rs1 < 0)
	            break ;

	        if ((ste.clu != NULL) && (ste.clu[0] != '\0')) {

#if	CF_DEBUGS && 0
	            debugprintf("dbi_getclusters: cluster=%s\n",ste.clu) ;
#endif

	            if (vecstr_find(sp,ste.clu) == SR_NOTFOUND) {

	                c += 1 ;
	                rs = vecstr_add(sp,ste.clu,-1) ;

	                if (rs < 0)
	                    break ;

	            }

	        } /* end if (got one) */

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: node whilebot\n") ;
#endif

	    } /* end while */

#if	CF_DEBUGS && 0
	    debugprintf("dbi_getclusters: node whileout\n") ;
#endif

	    nodedb_curend(&dbip->node,&cur) ;

	} /* end if (DB lookup) */

#if	CF_DEBUGS && 0
	debugprintf("dbi_getclusters: middle\n") ;
#endif

/* try the CLUSTER table if we have one */

	if (dbip->f_cluster) {
	    CLUSTERDB_CUR	cur ;
	    char	cname[NODENAMELEN + 1] ;

#if	CF_DEBUGS && 0
	    debugprintf("dbi_getclusters: cluster lookup\n") ;
#endif

	    clusterdb_curbegin(&dbip->cluster,&cur) ;

	    while (TRUE) {

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: cluster whiletop\n") ;
#endif

	        rs1 = clusterdb_fetchrev(&dbip->cluster,nodename,&cur,
	            cname,NODENAMELEN) ;

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: clusterdb_fetchrev() rs=%d\n",
			rs1) ;
#endif

	        if (rs1 < 0)
	            break ;

	        if (vecstr_find(sp,cname) == SR_NOTFOUND) {
	            c += 1 ;
	            rs = vecstr_add(sp,cname,-1) ;
	            if (rs < 0) break ;
	        }

#if	CF_DEBUGS && 0
	        debugprintf("dbi_getclusters: cluster whilebot\n") ;
#endif

	    } /* end while (reverse fetch) */

#if	CF_DEBUGS && 0
	    debugprintf("dbi_getclusters: cluster whileout\n") ;
#endif

	    clusterdb_curend(&dbip->cluster,&cur) ;
	} /* end if */

#if	CF_DEBUGS && 0
	debugprintf("dbi_getclusters: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dbi_getclusters) */


int dbi_close(dbip)
DBI	*dbip ;
{

	if (dbip == NULL) return SR_FAULT ;

	if (dbip->f_cluster)
	    clusterdb_close(&dbip->cluster) ;

	if (dbip->f_node)
	    nodedb_close(&dbip->node) ;

	return SR_OK ;
}
/* end subroutine (dbi_close) */


int dbi_getnodes(dbip,clp,nlp)
DBI		*dbip ;
vecstr		*clp, *nlp ;
{
	CLUSTERDB_CUR	cc ;

	int	rs = SR_OK, rs1, i ;
	int	cl ;
	int	c = 0 ;

	char	cnode[NODENAMELEN + 1] ;
	char	*cp ;

	if (dbip == NULL) return SR_FAULT ;
	if (clp == NULL) return SR_FAULT ;
	if (nlp == NULL) return SR_FAULT ;

	for (i = 0 ; vecstr_get(clp,i,&cp) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("dbi_getnodes: attached "
	        "cluster=%s\n",
	        cp) ;
#endif

	    clusterdb_curbegin(&dbip->cluster,&cc) ;

	    while (TRUE) {

	        rs1 = clusterdb_fetch(&dbip->cluster,cp,
	            &cc,
	            cnode,NODENAMELEN) ;

		if (rs1 == SR_NOTFOUND)
		    break ;

		rs = rs1 ;
	        if (rs1 < 0)
	            break ;

	        cl = rs1 ;

#if	CF_DEBUGS
	        debugprintf("dbi_getnodes: cluster "
	            "cl=%d node=%s\n",
	            cl,cnode) ;
#endif

	        rs1 = vecstr_findn(nlp,cnode,cl) ;

	        if (rs1 == SR_NOTFOUND) {

		    c += 1 ;
	            rs = vecstr_add(nlp, cnode, cl) ;
	            if (rs < 0)
	                break ;

	        }

	    } /* end while */

	    clusterdb_curend(&dbip->cluster,&cc) ;

	} /* end for */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (dbi_getnodes) */


