/* pdb */

/* printer database */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was adapted from others programs that did similar types
        of functions.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine manages the default printer database.

	Synopsis:

	int pdb_open(op,pr,ur,uname,fname)
	PDB		*op ;
	cchar		pr[] ;
	cchar		ur[] ;
	cchar		uname[] ;
	cchar		fname[] ;

	Arguments:

	op		printer database handle
	pr		program root (distribution root)
	ur		user root
	uname		utility name
	fname		filename to open

	Returns:

	>=0		good
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<svcfile.h>
#include	<vecstr.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"pdb.h"


/* local defines */

#define	PDB_TOFIND	5
#define	PDB_TOCHECK	5
#define	PDB_TOOPEN	60
#define	PDB_TOCALC	30

#undef	STEBUFLEN
#define	STEBUFLEN	4096


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;
extern int	mkpath1(char *,cchar *) ;
extern int	vecstr_envadd(vecstr *,cchar *,cchar *,int) ;
extern int	vecstr_envset(vecstr *,cchar *,cchar *,int) ;
extern int	permsched(cchar **,vecstr *,char *,int,cchar *,int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int pdb_fetcher(PDB *,char *,int,cchar *,cchar *,int) ;
static int pdb_dbopen(PDB *,int) ;
static int pdb_dbcheck(PDB *,int) ;
static int pdb_dbclose(PDB *,int) ;
static int pdb_findfile(PDB *,char *,int) ;


/* local variables */

static cchar	*dbsched[] = {
	"%p/%e/%n/%n.%f",
	"%p/%e/%n/%f",
	"%p/%e/%n.%f",
	"%p/%e/%f",
	"%p/%n.%f",
	"%p/%f",
	"%n.%f",
	"%f",
	NULL
} ;


/* exported subroutines */


int pdb_open(PDB *op,cchar *pr,cchar *ur,cchar *uname,cchar *fname)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (ur == NULL) return SR_FAULT ;
	if (uname == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(PDB)) ;

	size += (strlen(pr)+1) ;
	size += (strlen(ur)+1) ;
	size += (strlen(uname)+1) ;
	size += (strlen(fname)+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    op->a = bp ;
	    op->pr = bp ;
	    bp = (strwcpy(bp,pr,-1)+1) ;
	    op->ur = bp ;
	    bp = (strwcpy(bp,ur,-1)+1) ;
	    op->uname = bp ;
	    bp = (strwcpy(bp,uname,-1)+1) ;
	    op->fname = bp ;
	    bp = (strwcpy(bp,fname,-1)+1) ;
	    op->magic = PDB_MAGIC ;
	} /* end if (m-a) */

#if	CF_DEBUGS
	    debugprintf("pdb_open: pr=%s\n",op->pr) ;
	    debugprintf("pdb_open: ur=%s\n",op->ur) ;
	    debugprintf("pdb_open: util=%s\n",op->uname) ;
	    debugprintf("pdb_open: fname=%s\n",op->fname) ;
#endif

#if	CF_DEBUGS
	    debugprintf("pdb_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pdb_open) */


int pdb_close(PDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		w ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PDB_MAGIC) return SR_NOTOPEN ;

/* close out the DBs */

	for (w = 0 ; w < pdb_overlast ; w += 1) {
	   rs1 = pdb_dbclose(op,w) ;
	   if (rs >= 0) rs = rs1 ;
	}

/* free everything else */

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pdb_close) */


int pdb_fetch(PDB *op,char *vbuf,int vlen,cchar *printer,cchar *key)
{
	const int	rsn = SR_NOTFOUND ;
	int		rs ;
	int		w ;

	if (op == NULL) return SR_FAULT ;
	if (printer == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;
	if (vbuf == NULL) return SR_FAULT ;

	if (op->magic != PDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("prtdb_fetch: ent p=%s k=%s\n",printer,key) ;
#endif

	w = pdb_local ;
	if ((rs = pdb_fetcher(op,vbuf,vlen,printer,key,w)) == rsn) {
	    w = pdb_system ;
	    rs = pdb_fetcher(op,vbuf,vlen,printer,key,w) ;
	} /* end if (searching system DB) */

#if	CF_DEBUGS
	debugprintf("prtdb_fetch: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pdb_fetch) */


int pdb_check(PDB *op,time_t dt)
{
	int		rs1 ;
	int		w ;
	int		f_changed = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PDB_MAGIC) return SR_NOTOPEN ;

	op->dt = (dt != 0) ? dt : time(NULL) ;

	for (w = 0 ; w < pdb_overlast ; w += 1) {

	    rs1 = pdb_dbcheck(op,w) ;

	    if (! f_changed)
		f_changed = (rs1 > 0) ;

	} /* end for */

	return f_changed ;
}
/* end if (pdb_check) */


/* private subroutines*/


int pdb_fetcher(PDB *op,char *buf,int vlen,cchar *printer,cchar *key,int w)
{
	SVCFILE_CUR	cur ;
	PDB_DB		*dbp ;
	int		rs = SR_OK ;
	int		i ;

	if (printer == NULL) return SR_FAULT ;
	if (key == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (w >= pdb_overlast) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("prtdb_fetcher: ent p=%s k=%s w=%u\n",printer,key,w) ;
#endif

	dbp = (op->dbs + w) ;

	if (! dbp->f_open) {
	    rs = pdb_dbopen(op,w) ;
	}

	if (rs >= 0) {
	    SVCFILE_ENT	ste ;
	    char	stebuf[STEBUFLEN + 1] ;

	    svcfile_curbegin(&dbp->dbfile,&cur) ;

/* CONSTCOND */

	    while (TRUE) {

	        rs = svcfile_fetch(&dbp->dbfile,printer,&cur,
	            &ste,stebuf,STEBUFLEN) ;

	        if (rs < 0) break ;

	        rs = SR_NOTFOUND ;
	        for (i = 0 ; ste.keyvals[i][0] != NULL ; i += 1) {
	            if (strcmp(key,ste.keyvals[i][0]) == 0) {
	                rs = sncpy1(buf,vlen,ste.keyvals[i][1]) ;
	                if ((rs >= 0) || (rs == SR_OVERFLOW)) break ;
	            } /* end if (got a key match) */
	        } /* end for (looping through entry keys) */

	        if ((rs >= 0) || (rs == SR_OVERFLOW)) break ;

	    } /* end while (looping through enties) */

	    svcfile_curend(&dbp->dbfile,&cur) ;
	} /* end if (DB is open) */

#if	CF_DEBUGS
	debugprintf("prtdb_fetcher: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pdb_fetcher) */


static int pdb_dbopen(PDB *op,int w)
{
	PDB_DB		*dbp = (op->dbs + w) ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("prtdb_dbopen: ent w=%u\n",w) ;
#endif

	if (! dbp->f_open) {
	int		intfind ;
	char		dbfname[MAXPATHLEN + 1] ;

	if (op->dt == 0) op->dt = time(NULL) ;

/* do not try to open (find file and open) if we recently already tried */

	intfind = (op->dt - dbp->ti_find) ;
	if (intfind >= PDB_TOFIND) {

	dbp->ti_find = op->dt ;

/* try to open the DB file */

	dbfname[0] = '\0' ;
	rs = pdb_findfile(op,dbfname,w) ;

	if ((rs >= 0) && (dbfname[0] != '\0')) {

	    rs = svcfile_open(&dbp->dbfile,dbfname) ;

	    dbp->f_open = (rs >= 0) ? 1 : 0 ;
	    if (rs >= 0)
	        dbp->ti_open = op->dt ;

	} /* end if (trying to open) */

	} else {
	    rs = SR_NOENT ;
	}

	} /* end if (needed) */

#if	CF_DEBUGS
	debugprintf("prtdb_dbopen: ret rs=%d\n") ;
#endif

	return rs ;
}
/* end subroutine (pdb_dbopen) */


static int pdb_findfile(PDB *op,char *rbuf,int w)
{
	PDB_DB		*dbp ;
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	cchar		*tp ;

#if	CF_DEBUGS
	debugprintf("prtdb_findfile: ent w=%u\n",w) ;
	debugprintf("prtdb_findfile: fname=%s\n",op->fname) ;
#endif

	rbuf[0] = '\0' ;
	if ((tp = strchr(op->fname,'/')) == NULL) {
	    vecstr	svars ;

	    if ((rs = vecstr_start(&svars,6,0)) >= 0) {

		tp = "/" ;
		switch (w) {
		case pdb_local:
	            if ((op->ur != NULL) && (op->ur[0] != '\0')) {
			tp = op->ur ;
		    }
		    break ;
		case pdb_system:
	            if ((op->pr != NULL) && (op->pr[0] != '\0')) {
			tp = op->pr ;
		    }
		    break ;
		} /* end switch */

	        vecstr_envset(&svars,"p",tp,-1) ;

	        vecstr_envset(&svars,"e","etc",-1) ;

	        if ((op->uname != NULL) && (op->uname[0] != '\0')) {
	            vecstr_envset(&svars,"n",op->uname,-1) ;
		}

	        rs = permsched(dbsched,&svars,rbuf,rlen,op->fname,R_OK) ;

	        vecstr_finish(&svars) ;
	    } /* end if */

	} else {
	    rs = mkpath1(rbuf,op->fname) ;
	}

	if (rs >= 0) {
	    USTAT	sb ;
	    if ((rs = uc_stat(rbuf,&sb)) >= 0) {
	        if (S_ISDIR(sb.st_mode)) {
		    rbuf[0] = '\0' ;
	            rs = SR_ISDIR ;
		} else {
		    dbp = (op->dbs + w) ;
		    dbp->ti_mtime = sb.st_mtime ;
		}
	    } /* end if (uc_stat) */
	} /* end if (directory check) */

#if	CF_DEBUGS
	debugprintf("prtdb_findfile: ret rs=%d\n",rs) ;
	debugprintf("prtdb_findfile: ret rbuf=%s\n",rbuf) ;
#endif

	return rs ;
}
/* end subroutine (pdb_findfile) */


static int pdb_dbcheck(PDB *op,int w)
{
	PDB_DB		*dbp ;
	int		rs = SR_OK ;

	if (w >= pdb_overlast) return SR_INVALID ;

	dbp = (op->dbs + w) ;

	if (dbp->f_open)
	    rs = svcfile_check(&dbp->dbfile,op->dt) ;

	return rs ;
}
/* end subroutine (pdb_dbcheck) */


static int pdb_dbclose(PDB *op,int w)
{
	PDB_DB		*dbp ;
	int		rs = SR_OK ;

	if (w >= pdb_overlast) return SR_INVALID ;

	dbp = (op->dbs + w) ;

	if (dbp->f_open) {
	    dbp->ti_find = 0 ;
	    dbp->ti_mtime = 0 ;
	    dbp->ti_open = 0 ;
	    dbp->f_open = FALSE ;
	    rs = svcfile_close(&dbp->dbfile) ;
	}

	return rs ;
}
/* end subroutine (pdb_dbclose) */


