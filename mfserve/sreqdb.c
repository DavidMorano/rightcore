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

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	sreqdb_delit(SREQDB *,int,SREQ *) ;
static int	sreqdb_entfins(SREQDB *) ;

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
	    const int	vo = (VECHAND_OREUSE | VECHAND_OCONSERVE) ;
	    jlp->tmpdname = cp ;
	    rs = vechand_start(&jlp->db,n,vo) ;
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

	if (jlp == NULL) return SR_FAULT ;

	rs1 = sreqdb_entfins(jlp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_finish(&jlp->db) ;
	if (rs >= 0) rs = rs1 ;

	if (jlp->tmpdname != NULL) {
	    rs1 = uc_free(jlp->tmpdname) ;
	    if (rs >= 0) rs = rs1 ;
	    jlp->tmpdname = NULL ;
	}

	return rs ;
}
/* end subroutine (sreqdb_finish) */


/* add a new job */
int sreqdb_newjob(SREQDB *jlp,cchar *jobid,int ifd,int ofd)
{
	int		rs ;
	const char	*try = "sreqdbXXXXXXXXX" ;
	char		template[MAXPATHLEN + 1] ;

	if (jlp == NULL) return SR_FAULT ;

	if ((rs = mkpath2(template,jlp->tmpdname,try)) >= 0) {
	    SREQ	*jep ;
	    const int	jsize = sizeof(SREQ) ;
	    if ((rs = uc_malloc(jsize,&jep)) >= 0) {
	        if ((rs = sreq_start(jep,template,jobid,ifd,ofd)) >= 0) {
	            rs = vechand_add(&jlp->db,jep) ;
		    if (rs < 0)
		        sreq_finish(jep) ;
		} /* end if (sreq_start) */
		if (rs < 0) {
		    uc_free(jep) ;
		}
	    } /* end if (m-a) */
	} /* end if (mkpath) */

	return rs ;
}
/* end subroutine (sreqdb_newjob) */


/* search the job table for a PID match */
int sreqdb_findpid(SREQDB *jlp,pid_t pid,SREQ **jepp)
{
	int		rs = SR_NOTFOUND ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(&jlp->db,i,jepp)) >= 0 ; i += 1) {
	    if ((*jepp) != NULL) {
	        if ((*jepp)->pid == pid) break ;
	    }
	} /* end for */

	if (rs < 0) (*jepp) = NULL ;

	return rs ;
}
/* end subroutine (sreqdb_findpid) */


/* search for a job in the job table via its filename */
int sreqdb_search(SREQDB *jlp,cchar *fname,SREQ **jepp)
{
	int		rs = SR_NOTFOUND ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(&jlp->db,i,jepp)) >= 0 ; i += 1) {
	    if ((*jepp) != NULL) {
	        if (strcmp((*jepp)->efname,fname) == 0) break ;
	    }
	} /* end for */

	if (rs < 0) (*jepp) = NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sreqdb_search) */


/* enumerate all of the jobs */
int sreqdb_get(SREQDB *jlp,int i,SREQ **jepp)
{

	if (jlp == NULL) return SR_FAULT ;

	return vechand_get(&jlp->db,i,jepp) ;
}
/* end subroutine (sreqdb_get) */


/* get a job by its structure pointer */
int sreqdb_getp(SREQDB *jlp,SREQ *jep)
{
	SREQ		*jep2 ;
	int		rs = SR_NOTFOUND ;
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
int sreqdb_delp(SREQDB *jlp,SREQ *jep)
{
	SREQ		*jep2 ;
	int		rs = SR_NOTFOUND ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vechand_get(&jlp->db,i,&jep2)) >= 0 ; i += 1) {
	    if (jep2 != NULL) {
	        if (jep2 == jep) {
	            rs = sreqdb_delit(jlp,i,jep) ;
		    break ;
	        }
	    }
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (sreqdb_delp) */


int sreqdb_count(SREQDB *jlp)
{

	if (jlp == NULL) return SR_FAULT ;

	return vechand_count(&jlp->db) ;
}
/* end subroutine (sreqdb_count) */


/* private subroutines */


/* delete stuff associated with this job */
static int sreqdb_delit(SREQDB *jlp,int i,SREQ *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = sreq_finish(jep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vechand_del(&jlp->db,i) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = uc_free(jep) ;
	if (rs >= 0) rs = rs1 ;

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


