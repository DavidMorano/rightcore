/* sreqdb */

/* perform various functions on a job */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_CHECKDIR	0		/* using |sreqdb_checkdir()| */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is responsible for providing means to store a job and the
        retrieve it later by its PID.


*******************************************************************************/


#define	SREQDB_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sreqdb.h"
#include	"sreq.h"


/* local defines */

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	chmods(const char *,mode_t) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	sreqdb_delit(SREQDB *,int,SREQ *) ;
static int	sreqdb_entfins(SREQDB *) ;
static int	sreqdb_thrdone(SREQDB *,SREQ **) ;

#if	CF_CHECKDIR
static int	sreqdb_checkdir(SREQDB *) ;
#endif /* CF_CHECKDIR */


/* global variables */


/* exported subroutines */


int sreqdb_start(SREQDB *jlp,cchar *tmpdname,int n)
{
	int		rs ;
	const char	*cp ;

	if (jlp == NULL) return SR_FAULT ;

	if (n < 10) n = 4 ;

	if (tmpdname == NULL) tmpdname = getenv(VARTMPDNAME) ;
	if (tmpdname == NULL) tmpdname = TMPDNAME ;

	if ((rs = uc_mallocstrw(tmpdname,-1,&cp)) >= 0) {
	    int	vo = 0 ;
	    vo |= VECHAND_OREUSE ;
	    vo |= VECHAND_OCONSERVE ;
	    vo |= VECHAND_OSTATIONARY ;
	    jlp->tmpdname = cp ;
	    if ((rs = vechand_start(&jlp->db,n,vo)) >= 0) {
		rs = intiq_start(&jlp->exits) ;
		if (rs < 0)
		    vechand_finish(&jlp->db) ;
	    }
	    if (rs < 0) {
	        uc_free(jlp->tmpdname) ;
	        jlp->tmpdname = NULL ;
	    }
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (sreqdb_start) */


/* free a job structure */
int sreqdb_finish(SREQDB *jlp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("sreqdb_finish: ent\n") ;
#endif /* CF_DEBUGS */

	if (jlp == NULL) return SR_FAULT ;

	rs1 = intiq_finish(&jlp->exits) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sreqdb_finish: mid1\n") ;
#endif /* CF_DEBUGS */

	rs1 = sreqdb_entfins(jlp) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sreqdb_finish: mid2\n") ;
#endif /* CF_DEBUGS */

	rs1 = vechand_finish(&jlp->db) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sreqdb_finish: mid3\n") ;
#endif /* CF_DEBUGS */

	if (jlp->tmpdname != NULL) {
	    rs1 = uc_free(jlp->tmpdname) ;
	    if (rs >= 0) rs = rs1 ;
	    jlp->tmpdname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("sreqdb_finish: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (sreqdb_finish) */


/* add a new job */
int sreqdb_newjob(SREQDB *jlp,int jsn,cchar *jobid,int ifd,int ofd)
{
	int		rs ;
	int		ji = 0 ;
	const char	*try = "sreqdbXXXXXXXXX" ;
	char		template[MAXPATHLEN + 1] ;

	if (jlp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("sreqdb_newjob: ent jobid=%s ifd=%d ofd=%d\n",
		jobid,ifd,ofd) ;
#endif /* CF_DEBUGS */

	if ((rs = mkpath2(template,jlp->tmpdname,try)) >= 0) {
	    SREQ	*jep ;
	    const int	jsize = sizeof(SREQ) ;
	    if ((rs = uc_malloc(jsize,&jep)) >= 0) {
	        if ((rs = sreq_start(jep,template,jobid,ifd,ofd)) >= 0) {
	            if ((rs = vechand_add(&jlp->db,jep)) >= 0) {
		        ji = rs ;
			jep->ji = rs ;
			jep->jsn = jsn ;
		    }
		    if (rs < 0)
		        sreq_finish(jep) ;
		} /* end if (sreq_start) */
		if (rs < 0) {
		    uc_free(jep) ;
		}
	    } /* end if (m-a) */
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("sreqdb_newjob: ret rs=%d ji=%u\n",rs,ji) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? ji : rs ;
}
/* end subroutine (sreqdb_newjob) */


int sreqdb_typeset(SREQDB *op,int ji,int jt,int st)
{
	SREQ		*jep ;
	vechand		*jlp = &op->db ;
	int		rs ;
	if ((rs = vechand_get(jlp,ji,&jep)) >= 0) {
	    if (jep != NULL) {
	        rs = sreq_typeset(jep,jt,st) ;
	    }
	}
	return rs ;
}
/* end subroutine (sreqdb_typeset) */


/* search the job table for a PID match */
int sreqdb_findpid(SREQDB *jlp,pid_t pid,SREQ **jepp)
{
	int		rs ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(&jlp->db,i,jepp)) >= 0 ; i += 1) {
	    if ((*jepp) != NULL) {
	        if ((*jepp)->pid == pid) break ;
	    }
	} /* end for */

	if (rs < 0) (*jepp) = NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sreqdb_findpid) */


#ifdef	COMMENT

/* search for a job in the job table via its (error) filename */
int sreqdb_search(SREQDB *jlp,cchar *fname,SREQ **jepp)
{
	int		rs ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(&jlp->db,i,jepp)) >= 0 ; i += 1) {
	    if ((*jepp) != NULL) {
	        if (strcmp((*jepp)->efname,fname) == 0) break ;
	    }
	} /* end for */

	if (rs < 0) *jepp = NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sreqdb_search) */

#endif /* COMMENT */


/* enumerate all of the jobs */
int sreqdb_get(SREQDB *jlp,int i,SREQ **jepp)
{
	int		rs ;

#if	CF_DEBUGS
	debugprintf("sreqdb_get: ent i=%u\n",i) ;
#endif /* CF_DEBUGS */

	if (jlp == NULL) return SR_FAULT ;

	rs = vechand_get(&jlp->db,i,jepp) ;

#if	CF_DEBUGS
	debugprintf("sreqdb_get: ret rs=%d\n",rs) ;
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (sreqdb_get) */


/* get a job by its structure pointer */
int sreqdb_getp(SREQDB *jlp,SREQ *jep)
{
	SREQ		*jep2 ;
	int		rs ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(&jlp->db,i,&jep2)) >= 0 ; i += 1) {
	    if (jep2 != NULL) {
	        if (jep2 == jep) break ;
	    }
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sreqdb_getp) */


/* delete a job by index */
int sreqdb_del(SREQDB *jlp,int i)
{
	SREQ		*jep ;
	int		rs ;
	int		rs1 ;

	if (jlp == NULL) return SR_FAULT ;

	if ((rs = vechand_get(&jlp->db,i,&jep)) >= 0) {
	    if (jep != NULL) {
	        rs1 = sreqdb_delit(jlp,i,jep) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end if (found job) */

#if	CF_DEBUGS
	{
	    const int	c = vechand_count(&jlp->db) ;
	    debugprintf("sreqdb_del: ret rs=%d c=%u\n",rs,c) ;
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (sreqdb_del) */


/* delete a job by pointer */
int sreqdb_delobj(SREQDB *jlp,SREQ *jep)
{
	SREQ		*ep ;
	int		rs ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("sreqdb_delobj: ent {%p}\n",jep) ;
#endif

	if (jlp == NULL) return SR_FAULT ;
	if (jep == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(&jlp->db,i,&ep)) >= 0 ; i += 1) {
	    if (ep == jep) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("sreqdb_delobj: mid2 rs=%d i=%u\n",rs,i) ;
#endif

	if (rs >= 0) {
	    rs = sreqdb_delit(jlp,i,jep) ;
	}

#if	CF_DEBUGS
	debugprintf("sreqdb_delobj: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sreqdb_delobj) */


int sreqdb_count(SREQDB *jlp)
{

	if (jlp == NULL) return SR_FAULT ;

	return vechand_count(&jlp->db) ;
}
/* end subroutine (sreqdb_count) */


int sreqdb_builtrelease(SREQDB *op)
{
	SREQ		*jep ;
	vechand		*jlp = &op->db ;
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	if (jlp == NULL) return SR_FAULT ;
	for (i = 0 ; (rs1 = vechand_get(jlp,i,&jep)) >= 0 ; i += 1) {
	    if (jep != NULL) {
	        rs = sreq_builtrelease(jep) ;
		c += rs ;
	    }
	    if (rs < 0) break ;
	} /* end for */
	if ((rs >= 0) && (rs1 != rsn)) rs = rs1 ;
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sreqdb_builtrelease) */


/* do we have the given FD in the DB, if so return index */
int sreqdb_findfd(SREQDB *op,int fd)
{
	SREQ		*jep ;
	vechand		*jlp = &op->db ;
	int		rs ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("sreqdb_findfd: ent fd=%d\n",fd) ;
#endif

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(jlp,i,&jep)) >= 0 ; i += 1) {
	    if (jep != NULL) {
	        if ((rs = sreq_havefd(jep,fd)) > 0) break ;
	    }
	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("sreqdb_findfd: ret rs=%d i=%u\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sreqdb_findfd) */


/* child thread calls this to signal that it is exiting */
int sreqdb_exiting(SREQDB *op,int ji)
{
	SREQ		*jep ;
	int		rs ;
	if ((rs = vechand_get(&op->db,ji,&jep)) >= 0) {
	    const int	st = sreqstate_done ;
	    if ((rs = sreq_setstate(jep,st)) >= 0) {
		if ((rs = sreq_exiting(jep)) >= 0) {
		    if (jep->f.thread) {
	                if ((rs = intiq_ins(&op->exits,ji)) >= 0) {
	        	    op->f_threxiting = TRUE ;
			}
		    }
		}
	    }
	}
	return rs ;
}
/* end subroutine (sreqdb_exiting) */


int sreqdb_thrsdone(SREQDB *op,SREQ **rpp)
{
	int		rs = SR_NOTFOUND ;
	if (op->f_threxiting) {
	    const int	rsn = SR_NOTFOUND ;
	    if ((rs = sreqdb_thrdone(op,rpp)) == rsn) {
	        op->f_threxiting = FALSE ;
	        rs = sreqdb_thrdone(op,rpp) ;
	    }
	}
	return rs ;
}
/* end subroutine (sreqdb_thrsdone) */


/* private subroutines */


static int sreqdb_thrdone(SREQDB *op,SREQ **rpp)
{
	INTIQ		*iqp = &op->exits ;
	int		jid ;
	int		rs ;
	if ((rs = intiq_rem(iqp,&jid)) >= 0) {
	    SREQ	*jep ;
	    vechand	*dbp = &op->db ;
	    if ((rs = vechand_get(dbp,jid,&jep)) >= 0) {
		*rpp = jep ;
		rs = jid ;
	    }
	}
	return rs ;
}
/* end subroutine (sreqdb_thrdone) */


/* delete stuff associated with this job */
static int sreqdb_delit(SREQDB *jlp,int ji,SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("sreqdb_delit: ent ji=%u {%p}\n",ji,jep) ;
#endif

	rs1 = sreq_finish(jep) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sreqdb_delit: sreq_finish() rs=%d\n",rs) ;
#endif

	rs1 = vechand_del(&jlp->db,ji) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uc_free(jep) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("sreqdb_delit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sreqdb_delit) */


/* check if the spool directory is present */
#if	CF_CHECKDIR
static int sreqdb_checkdir(SREQDB *jlp)
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(jlp->tmpdname,&sb)) >= 0) {
	    rs = 0 ;
	} else if (isNotPresent(rs)) {
	    const mode_t	dm = (0777 | S_ISVTX) ;
	    if ((rs = mkdirs(jlp->tmpdname,dm)) >= 0) {
		rs = chmods(jlp->tmpdname,dm) ;
	    }
	} /* end if (needed to create) */

	return rs ;
}
/* end subroutine (sreqdb_checkdir) */
#endif /* CF_CHECKDIR */


static int sreqdb_entfins(SREQDB *jlp)
{
	SREQ		*jep ;
	VECHAND		*dbp = &jlp->db ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	for (i = 0 ; vechand_get(dbp,i,&jep) >= 0 ; i += 1) {
	    if (jep != NULL) {
	        rs1 = sreq_finish(jep) ;
	        if (rs >= 0) rs = rs1 ;
		rs1 = uc_free(jep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */
	return rs ;
}
/* end subroutine (sreqdb_entfins) */


