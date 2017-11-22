/* jobdb */

/* perform various functions on a job */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module is responsible for providing means to store a job and the
        retrieve it later by its PID.


*******************************************************************************/


#define	JOBDB_MASTER	0


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
#include	<vecitem.h>
#include	<localmisc.h>

#if	CF_DEBUGS
#include	<debug.h>
#endif

#include	"jobdb.h"


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

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	jobdb_delit(JOBDB *,int,JOBDB_ENT *) ;
static int	jobdb_checkdir(JOBDB *) ;

static int	entry_start(JOBDB_ENT *,const char *,const char *,int) ;
static int	entry_finish(JOBDB_ENT *) ;

static int	mkfile(cchar *,cchar **) ;


/* global variables */


/* exported subroutines */


int jobdb_start(JOBDB *jlp,int initsize,cchar *tmpdname)
{
	int		rs ;
	const char	*cp ;

	if (jlp == NULL) return SR_FAULT ;

	if (initsize < 10) initsize = 10 ;

	if (tmpdname == NULL) tmpdname = getenv(VARTMPDNAME) ;
	if (tmpdname == NULL) tmpdname = TMPDNAME ;

	if ((rs = uc_mallocstrw(tmpdname,-1,&cp)) >= 0) {
	    const int	vo = (VECITEM_OREUSE | VECITEM_OCONSERVE) ;
	    jlp->tmpdname = cp ;
	    rs = vecitem_start(&jlp->db,initsize,vo) ;
	    if (rs < 0) {
	        uc_free(jlp->tmpdname) ;
	        jlp->tmpdname = NULL ;
	    }
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (jobdb_start) */


/* free a job structure */
int jobdb_finish(JOBDB *jlp)
{
	VECITEM		*elp = &jlp->db ;
	JOBDB_ENT	*jep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("jobdb_finish: ent\n") ;
#endif

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; vecitem_get(elp,i,&jep) >= 0 ; i += 1) {
	    if (jep != NULL) {
	        rs1 = entry_finish(jep) ;
	        if (rs >= 0) rs = rs1 ;
		c += 1 ;
	    }
	} /* end for */

	rs1 = vecitem_finish(elp) ;
	if (rs >= 0) rs = rs1 ;

	if (jlp->tmpdname != NULL) {
	    rs1 = uc_free(jlp->tmpdname) ;
	    if (rs >= 0) rs = rs1 ;
	    jlp->tmpdname = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("jobdb_finish: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (jobdb_finish) */


/* add a new job */
int jobdb_newjob(JOBDB *jlp,cchar *jobid,int f_so)
{
	int		rs ;

	if (jlp == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("jobdb_newjob: ent f_so=%u\n",f_so) ;
#endif

	if ((rs = jobdb_checkdir(jlp)) >= 0) {
	    cchar	*try = "jobdbXXXXXXXXX" ;
	    char	template[MAXPATHLEN + 1] ;
	    if ((rs = mkpath2(template,jlp->tmpdname,try)) >= 0) {
	        JOBDB_ENT	je ;
	        if ((rs = entry_start(&je,template,jobid,f_so)) >= 0) {
	            const int	esize = sizeof(JOBDB_ENT) ;
	            rs = vecitem_add(&jlp->db,&je,esize) ;
	            if (rs < 0)
		        entry_finish(&je) ;
		} /* end if (entry_start) */
	    } /* end if (mkpath) */
	} /* end if (jobdb_checkdir) */

#if	CF_DEBUGS
	debugprintf("jobdb_newjob: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (jobdb_newjob) */


/* search the job table for a PID match */
int jobdb_findpid(JOBDB *jlp,pid_t pid,JOBDB_ENT **jepp)
{
	int		rs ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vecitem_get(&jlp->db,i,jepp)) >= 0 ; i += 1) {
	    if ((*jepp) != NULL) {
	        if ((*jepp)->pid == pid) break ;
	    }
	} /* end for */

	if (rs < 0) (*jepp) = NULL ;

	return rs ;
}
/* end subroutine (jobdb_findpid) */


/* search for a job in the job table via its filename */
int jobdb_search(JOBDB *jlp,cchar *fname,JOBDB_ENT **jepp)
{
	int		rs ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vecitem_get(&jlp->db,i,jepp)) >= 0 ; i += 1) {
	    if ((*jepp) != NULL) {
	        if (strcmp((*jepp)->efname,fname) == 0) break ;
	    }
	} /* end for */

	if (rs < 0) (*jepp) = NULL ;

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (jobdb_search) */


/* enumerate all of the jobs */
int jobdb_get(JOBDB *jlp,int i,JOBDB_ENT **jepp)
{

	if (jlp == NULL) return SR_FAULT ;

	return vecitem_get(&jlp->db,i,jepp) ;
}
/* end subroutine (jobdb_get) */


/* get a job by its structure pointer */
int jobdb_getp(JOBDB *jlp,JOBDB_ENT *jep)
{
	JOBDB_ENT	*jep2 ;
	int		rs ;
	int		i ;

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vecitem_get(&jlp->db,i,&jep2)) >= 0 ; i += 1) {
	    if (jep2 != NULL) {
	        if (jep2 == jep) break ;
	    }
	} /* end for */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (jobdb_getp) */


/* delete a job by index */
int jobdb_del(JOBDB *jlp,int i)
{
	JOBDB_ENT	*jep ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("jobdb_del: ent i=%i\n",i) ;
#endif

	if (jlp == NULL) return SR_FAULT ;

	if ((rs = vecitem_get(&jlp->db,i,&jep)) >= 0) {
	    if (jep != NULL) {
	        rs1 = jobdb_delit(jlp,i,jep) ;
		if (rs >= 0) rs = rs1 ;
	    }
	} /* end if (found job) */

#if	CF_DEBUGS
	{
	    const int	c = vecitem_count(&jlp->db) ;
	    debugprintf("jobdb_del: ret rs=%d c=%u\n",rs,c) ;
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (jobdb_del) */


/* delete a job by pointer */
int jobdb_delp(JOBDB *jlp,JOBDB_ENT *jep)
{
	JOBDB_ENT	*jep2 ;
	int		rs ;
	int		i ;

#if	CF_DEBUGS
	debugprintf("jobdb_delp: ent\n") ;
#endif

	if (jlp == NULL) return SR_FAULT ;

	for (i = 0 ; (rs = vecitem_get(&jlp->db,i,&jep2)) >= 0 ; i += 1) {
	    if (jep2 != NULL) {
	        if (jep2 == jep) {
	            rs = jobdb_delit(jlp,i,jep) ;
		    break ;
	        }
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("jobdb_delp: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (jobdb_delp) */


int jobdb_count(JOBDB *jlp)
{

	if (jlp == NULL) return SR_FAULT ;

	return vecitem_count(&jlp->db) ;
}
/* end subroutine (jobdb_count) */


/* private subroutines */


/* delete stuff associated with this job */
static int jobdb_delit(JOBDB *jlp,int i,JOBDB_ENT *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("jobdb_delit: ent i=%u\n",i) ;
#endif

	if (jlp == NULL) return SR_FAULT ;

	rs1 = entry_finish(jep) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecitem_del(&jlp->db,i) ;
	if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	debugprintf("jobdb_delit: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (jobdb_delit) */


/* check if the spool directory is present */
static int jobdb_checkdir(JOBDB *jlp)
{
	struct ustat	sb ;
	const int	rsn = SR_NOENT ;
	int		rs ;

	if ((rs = u_stat(jlp->tmpdname,&sb)) == rsn) {
	    const mode_t	dm = (0777 | S_ISVTX) ;
	    if ((rs = mkdirs(jlp->tmpdname,dm)) >= 0) {
		rs = chmod(jlp->tmpdname,dm) ;
	    }
	} /* end if (needed to create) */

	return rs ;
}
/* end subroutine (jobdb_checkdir) */


static int entry_start(JOBDB_ENT *jep,cchar *template,cchar *jobid,int f_so)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("jobdb/entry_start: ent jobid=%s f_so=%u\n",jobid,f_so) ;
#endif

	memset(jep,0,sizeof(JOBDB_ENT)) ;
	jep->pid = -1 ;
	jep->atime = dt ;
	jep->stime = dt ;

	strwcpy(jep->jobid,jobid,JOBDB_JOBIDLEN) ;

	if ((rs = mkfile(template,&cp)) >= 0) {
	    jep->efname = (char *) cp ;
	    if (f_so) {
	        rs = mkfile(template,&cp) ;
	        jep->ofname = (char *) cp ;
	    }
	    if (rs < 0) {
	        if (jep->efname != NULL) {
	            if (jep->efname[0] != '\0') {
		        u_unlink(jep->efname) ;
		        jep->efname[0] = '\0' ;
		    }
	            uc_free(jep->efname) ;
	            jep->efname = NULL ;
	        }
	    } /* end if (error-recovery) */
	} /* end if */

	return rs ;
}
/* end subroutine (entry_start) */


static int entry_finish(JOBDB_ENT *jep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("jobdb/entry_finish: ent jodb=%s\n",jep->jobid) ;
#endif

	if (jep->ofname != NULL) {
	    if (jep->ofname[0] != '\0') {
		u_unlink(jep->ofname) ;
		jep->ofname[0] = '\0' ;
	    }
	    rs1 = uc_free(jep->ofname) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->ofname = NULL ;
	}

	if (jep->efname != NULL) {
	    if (jep->efname[0] != '\0') {
		u_unlink(jep->efname) ;
		jep->efname[0] = '\0' ;
	    }
	    rs1 = uc_free(jep->efname) ;
	    if (rs >= 0) rs = rs1 ;
	    jep->efname = NULL ;
	}

	jep->pid = -1 ;
	jep->jobid[0] = '\0' ;

#if	CF_DEBUGS
	debugprintf("jobdb/entry_finish: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (entry_finish) */


static int mkfile(cchar *template,cchar **rpp)
{
	int		rs ;
	int		tl = 0 ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((rs = mktmpfile(tbuf,0666,template)) >= 0) {
	    tl = rs ;
	    rs = uc_mallocstrw(tbuf,tl,rpp) ;
	    if (rs < 0) {
	        u_unlink(tbuf) ;
		*rpp = NULL ;
	    } /* end if (error-recovery) */
	} /* end if (mktmpfile) */

	return (rs >= 0) ? tl : rs ;
}
/* end subroutines (mkfile) */


