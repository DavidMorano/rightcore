/* getclusters */

/* get all clusters for the specified nodename */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable */


/* revision history:

	= 1995-07-01, David A­D­ Morano
	This subroutine was originally written.

	= 2004-11-22, David A­D­ Morano
	This subroutine was hi-jacked and pressed into service to get the name
	of the primary cluster that the local node is in.

*/

/* Copyright © 1995,2004 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Get all cluster names given a nodename.

	Synopsis:

	int getclusters(pr,slp,nodename)
	const char	pr[] ;
	vecstr		*slp ;
	const char	nodename[] ;

	Arguments:

	pr		program root
	slp		pointer to VECSTR object for results
	nodename	nodename used to find associated cluster

	Returns:

	>=0		string length of cluster name
	SR_OK		if OK
	SR_NOTFOUND	if could not get something needed for correct operation
	SR_ISDIR	database file was a directory (admin error)
	<0		some other error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"nodedb.h"
#include	"clusterdb.h"


/* local defines */

#define	NODEFNAME	"etc/node"
#define	CLUSTERFNAME	"etc/cluster"


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int getclusters_ndb(const char *,vecstr *,const char *) ;
static int getclusters_cdb(const char *,vecstr *,const char *) ;

#if	CF_DEBUGS
static int	debugprintlist(const char *,vecstr *) ;
#endif /* CF_DEBUGS */


/* local variables */


/* exported subroutines */


int getclusters(pr,slp,nn)
const char	pr[] ;
vecstr		*slp ;
const char	nn[] ;
{
	int		rs = SR_OK ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("getclusters: ent\n") ;
	debugprintf("getclusters: pr=%s\n",pr) ;
	debugprintf("getclusters: nodename=%s\n",nn) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (slp == NULL) return SR_FAULT ;
	if (nn == NULL) return SR_FAULT ;

	if (nn[0] == '\0') return SR_INVALID ;

/* first NODE-DB then the CLUSTER-DB */

	if ((rs = getclusters_ndb(pr,slp,nn)) >= 0) {
	    c += rs ;
	    if ((rs = getclusters_cdb(pr,slp,nn)) >= 0) {
		c += rs ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("getclusters: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getclusters) */


/* local subroutines */


static int getclusters_ndb(const char *pr,vecstr *slp,const char *nn)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		tbuf[MAXPATHLEN+1] ;

	if ((rs = mkpath2(tbuf,pr,NODEFNAME)) >= 0) {
	    NODEDB	st ;
	    NODEDB_ENT	ste ;
	    NODEDB_CUR	cur ;

	    if ((rs = nodedb_open(&st,tbuf)) >= 0) {

	        if ((rs = nodedb_curbegin(&st,&cur)) >= 0) {
		    const int	nrs = SR_NOTFOUND ;
	            const int	elen = NODEDB_ENTLEN ;
	            char	ebuf[NODEDB_ENTLEN + 1] ;

	            while (rs >= 0) {

	                rs1 = nodedb_fetch(&st,nn,&cur,&ste,ebuf,elen) ;
	                if (rs1 == nrs) break ;
			rs = rs1 ;

			if (rs >= 0) {
	            	    if ((ste.clu != NULL) && (ste.clu[0] != '\0')) {
	                	if ((rs = vecstr_find(slp,ste.clu)) == nrs) {
	                    	    c += 1 ;
	                    	    rs = vecstr_add(slp,ste.clu,-1) ;
	                	}
	            	    } /* end if (got one) */
			} /* end if (ok) */

	            } /* end while (looping through node entries) */

	            nodedb_curend(&st,&cur) ;
		} /* end if (nodedb-cursor) */

	        rs1 = nodedb_close(&st) ;
		if (rs >= 0) rs = rs1 ;
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintlist("getclusters_ndb",slp) ;
	debugprintf("getclusters_ndb: ret rs=%d c=%u\n",rs,c) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getclusters_ndb) */


static int getclusters_cdb(const char *pr,vecstr *slp,const char *nn)
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	char		tbuf[MAXPATHLEN+1] ;

	if ((rs = mkpath2(tbuf,pr,CLUSTERFNAME)) >= 0) {
	    CLUSTERDB		cluster ;
	    CLUSTERDB_CUR	cur ;

	    if ((rs = clusterdb_open(&cluster,tbuf)) >= 0) {

	        if ((rs = clusterdb_curbegin(&cluster,&cur)) >= 0) {
		    const int	nrs = SR_NOTFOUND ;
		    const int	clen = NODENAMELEN ;
	            char	cbuf[NODENAMELEN+ 1] ;

	            while (rs >= 0) {

	                rs1 = clusterdb_fetchrev(&cluster,nn,&cur,cbuf,clen) ;
	                if (rs1 == nrs) break ;
			rs = rs1 ;

			if (rs >= 0) {
	            	    if ((rs = vecstr_find(slp,cbuf)) == nrs) {
	                	c += 1 ;
	                	rs = vecstr_add(slp,cbuf,-1) ;
			    }
	            	} /* end if (ok) */

	            } /* end while (fetchrev) */

	            clusterdb_curend(&cluster,&cur) ;
		} /* end if (clusterdb-cursor) */

	        rs1 = clusterdb_close(&cluster) ;
		if (rs >= 0) rs = rs1 ;
	    } else if (isNotPresent(rs)) {
		rs = SR_OK ;
	    }
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintlist("getclusters_cdb",slp) ;
	debugprintf("getclusters_cdb: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getclusters_cdb) */


#if	CF_DEBUGS
static int debugprintlist(const char *id,vecstr *slp)
{
	int		rs ;
	int		i ;
	int		c = 0 ;
	const char	*cp ;
	for (i = 0 ; vecstr_get(slp,i,&cp) >= 0 ; i += 1) {
	    if (cp == NULL) continue ;
	    c += 1 ;
	    debugprintf("%s: c=%s\n",id,cp) ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (debugprintlist) */
#endif /* CF_DEBUGS */


