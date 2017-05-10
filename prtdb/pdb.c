/* pdb */

/* printer database */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine manages the default printer database.

	Synopsis:

	int pdb_open(op,pr,ur,uname,fname)
	PDB		*op ;
	const char	pr[] ;
	const char	ur[] ;
	const char	uname[] ;
	const char	fname[] ;

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
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<baops.h>
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

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int pdb_fetcher(PDB *,int,const char *,const char *,char *,int) ;
static int pdb_dbopen(PDB *,int) ;
static int pdb_dbcheck(PDB *,int) ;
static int pdb_dbclose(PDB *,int) ;
static int pdb_findfile(PDB *,char *,int) ;


/* local variables */

static const char	*dbsched[] = {
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


int pdb_open(op,pr,ur,uname,fname)
PDB		*op ;
const char	pr[] ;
const char	ur[] ;
const char	uname[] ;
const char	fname[] ;
{
	int		rs = SR_NOMEM ;

	if (op == NULL) return SR_FAULT ;

	if (fname == NULL) return SR_FAULT ;

	memset(op,0,sizeof(PDB)) ;

	if ((pr != NULL) && (pr[0] != '\0')) {

	    op->pr = mallocstr(pr) ;

	    if (op->pr == NULL)
	        goto bad1 ;

	}

	if ((ur != NULL) && (ur[0] != '\0')) {

	    op->ur = mallocstr(ur) ;

	    if (op->ur == NULL)
	        goto bad2 ;

	}

	if (uname != NULL) {

	    op->uname = mallocstr(uname) ;

	    if (op->uname == NULL)
	        goto bad3 ;

	}

	op->fname = mallocstr(fname) ;

	if (op->fname == NULL)
	    goto bad4 ;

#if	CF_DEBUGS
	    debugprintf("pdb_open: pr=%s\n",op->pr) ;
	    debugprintf("pdb_open: ur=%s\n",op->ur) ;
	    debugprintf("pdb_open: util=%s\n",op->uname) ;
	    debugprintf("pdb_open: fname=%s\n",op->fname) ;
#endif

	rs = SR_OK ;
	op->magic = PDB_MAGIC ;

/* get out */
ret0:

#if	CF_DEBUGS
	    debugprintf("pdb_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad4:
	if (op->uname != NULL)
	    uc_free(op->uname) ;

bad3:
	if (op->ur != NULL)
	    uc_free(op->ur) ;

bad2:
	if (op->pr != NULL)
	    uc_free(op->pr) ;

bad1:
	goto ret0 ;
}
/* end subroutine (pdb_open) */


int pdb_fetch(op,printer,keyname,buf,buflen)
PDB		*op ;
const char	printer[] ;
const char	keyname[] ;
char		buf[] ;
int		buflen ;
{
	int		rs ;
	int		w ;

	if (op == NULL) return SR_FAULT ;
	if (printer == NULL) return SR_FAULT ;
	if (keyname == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (op->magic != PDB_MAGIC) return SR_NOTOPEN ;

	w = pdb_local ;
	rs = pdb_fetcher(op,w,printer,keyname,buf,buflen) ;

	if (rs == SR_NOTFOUND) {

	    w = pdb_system ;
	    rs = pdb_fetcher(op,w,printer,keyname,buf,buflen) ;

	} /* end if (searching system DB) */

	return rs ;
}
/* end subroutine (pdb_fetch) */


int pdb_check(op,daytime)
PDB		*op ;
time_t		daytime ;
{
	int		rs1 ;
	int		w ;
	int		f_changed = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != PDB_MAGIC) return SR_NOTOPEN ;

	op->dt = (daytime != 0) ? daytime : time(NULL) ;

	for (w = 0 ; w < pdb_overlast ; w += 1) {

	    rs1 = pdb_dbcheck(op,w) ;

	    if (! f_changed)
		f_changed = (rs1 > 0) ;

	} /* end for */

	return f_changed ;
}
/* end if (pdb_check) */


int pdb_close(op)
PDB		*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		w ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != PDB_MAGIC)
	    return SR_NOTOPEN ;

/* close out the DBs */

	for (w = 0 ; w < pdb_overlast ; w += 1) {
	   rs1 = pdb_dbclose(op,w) ;
	   if (rs >= 0) rs = rs1 ;
	}

/* free everything else */

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->uname != NULL) {
	    rs1 = uc_free(op->uname) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->ur != NULL) {
	    rs1 = uc_free(op->ur) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (pdb_close) */


/* private subroutines*/


int pdb_fetcher(op,w,printer,keyname,buf,buflen)
PDB		*op ;
int		w ;
const char	printer[] ;
const char	keyname[] ;
char		buf[] ;
int		buflen ;
{
	SVCFILE_CUR	cur ;
	struct pdb_db	*dbp ;
	int		rs = SR_OK ;
	int		i ;

	if (printer == NULL) return SR_FAULT ;
	if (keyname == NULL) return SR_FAULT ;
	if (buf == NULL) return SR_FAULT ;

	if (w >= pdb_overlast) return SR_INVALID ;

	dbp = (op->dbs + w) ;

	if (! dbp->f_open)
	    rs = pdb_dbopen(op,w) ;

	if (rs >= 0) {
	    SVCFILE_ENT	ste ;
	    char	stebuf[STEBUFLEN + 1] ;

	    svcfile_curbegin(&dbp->dbfile,&cur) ;

/* CONSTCOND */

	    while (TRUE) {

	        rs = svcfile_fetch(&dbp->dbfile,printer,&cur,
	            &ste,stebuf,STEBUFLEN) ;

	        if (rs < 0)
	            break ;

	        rs = SR_NOTFOUND ;
	        for (i = 0 ; ste.keyvals[i][0] != NULL ; i += 1) {

	            if (strcmp(keyname,ste.keyvals[i][0]) == 0) {

	                rs = sncpy1(buf,buflen,ste.keyvals[i][1]) ;

	                if ((rs >= 0) || (rs == SR_OVERFLOW))
	                    break ;

	            } /* end if (got a key match) */

	        } /* end for (looping through entry keys) */

	        if ((rs >= 0) || (rs == SR_OVERFLOW))
	            break ;

	    } /* end while (looping through enties) */

	    svcfile_curend(&dbp->dbfile,&cur) ;
	} /* end if (DB is open) */

	return rs ;
}
/* end subroutine (pdb_fetcher) */


static int pdb_dbopen(op,w)
PDB		*op ;
int		w ;
{
	struct pdb_db	*dbp ;
	int		rs = SR_OK ;
	int		intfind ;
	char		dbfname[MAXPATHLEN + 1] ;

	dbp = (op->dbs + w) ;
	if (dbp->f_open)
	    goto ret0 ;

	if (op->dt == 0)
	    op->dt = time(NULL) ;

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

	} else
	    rs = SR_NOENT ;

ret0:
	return rs ;
}
/* end subroutine (pdb_dbopen) */


static int pdb_findfile(op,dbfname,w)
PDB		*op ;
char		dbfname[] ;
int		w ;
{
	struct pdb_db	*dbp ;
	int		rs = SR_OK ;
	const char	*tp ;

	if ((tp = strchr(op->fname,'/')) == NULL) {
	    vecstr	svars ;

	    if ((rs = vecstr_start(&svars,6,0)) >= 0) {

		tp = "/" ;
		switch (w) {

		case pdb_local:
	            if ((op->ur != NULL) && (op->ur[0] != '\0'))
			tp = op->ur ;
		    break ;

		case pdb_system:
	            if ((op->pr != NULL) && (op->pr[0] != '\0'))
			tp = op->pr ;
		    break ;

		} /* end switch */

	        vecstr_envset(&svars,"p",tp,-1) ;

	        vecstr_envset(&svars,"e","etc",-1) ;

	        if ((op->uname != NULL) && (op->uname[0] != '\0'))
	            vecstr_envset(&svars,"n",op->uname,-1) ;

	        rs = permsched(dbsched,&svars,
	            dbfname,MAXPATHLEN, op->fname,R_OK) ;

	        vecstr_finish(&svars) ;
	    } /* end if */

	} else
	    rs = mkpath1(dbfname,op->fname) ;

	if (rs >= 0) {
	    struct ustat	sb ;

	    rs = u_stat(dbfname,&sb) ;

	    if ((rs >= 0) && S_ISDIR(sb.st_mode)) {
		dbfname[0] = '\0' ;
	        rs = SR_ISDIR ;
	    }

	    if (rs >= 0) {
		dbp = op->dbs + w ;
		dbp->ti_mtime = sb.st_mtime ;
	    }

	} /* end if (directory check) */

	return rs ;
}
/* end subroutine (pdb_findfile) */


static int pdb_dbcheck(op,w)
PDB		*op ;
int		w ;
{
	struct pdb_db	*dbp ;
	int		rs = SR_OK ;

	if (w >= pdb_overlast)
	    return SR_INVALID ;

	dbp = op->dbs + w ;

	if (dbp->f_open)
	    rs = svcfile_check(&dbp->dbfile,op->dt) ;

	return rs ;
}
/* end subroutine (pdb_dbcheck) */


static int pdb_dbclose(op,w)
PDB		*op ;
int		w ;
{
	struct pdb_db	*dbp ;
	int		rs = SR_OK ;

	if (w >= pdb_overlast)
	    return SR_INVALID ;

	dbp = op->dbs + w ;

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


