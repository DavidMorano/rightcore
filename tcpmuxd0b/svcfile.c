/* svcfile */

/* service table file manager */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFILE	1		/* debugging file reading */
#define	CF_DEVINO	0		/* check device-inode */
#define	CF_ALREADY	0		/* disallow duplicate entries */
#define	CF_MOREKEYS	0		/* |ientry_morekeys()| */
#define	CF_FILEDEL	0		/* |svcfile_filedel()| */


/* revision history:

	- 2004-05-25, David A­D­ Morano
	This subroutine was adopted for use as a general key-value file reader.

*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This object processes an access table for use by daemon multiplexing
	server programs that want to control access to their sub-servers.


******************************************************************************/


#define	SVCFILE_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<localmisc.h>

#include	"svcfile.h"


/* local defines */

#define	SVCFILE_INTCHECK	2	/* file-change check interval */
#define	SVCFILE_INTWAIT		2	/* file-change wait interval */

#define	SVCFILE_FILE		struct xsvcfile_file
#define	SVCFILE_KEYNAME		struct svcfile_keyname
#define	SVCFILE_IENT		struct svcfile_ie
#define	SVCFILE_SVCNAME		struct svcfile_svcname

#define	SVCENTRY		struct svcentry
#define	SVCENTRY_KEY		struct svcentry_key

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif

#ifndef	SVCNAMELEN
#ifdef	SVCFILE_SVCLEN
#define	SVCNAMELEN		SVCFILE_SVCLEN
#else
#define	SVCNAMELEN		MAXNAMELEN
#endif
#endif

#undef	ABUFLEN
#define	ABUFLEN			(3 * MAXHOSTNAMELEN)

#undef	DEFCHUNKSIZE
#define	DEFCHUNKSIZE		512

#define	DEFNFILES		10
#define	DEFNENTRIES		10

#define	SVCFILE_KA		sizeof(char *(*)[2])
#define	SVCFILE_BO(v)		\
	((SVCFILE_KA - ((v) % SVCFILE_KA)) % SVCFILE_KA)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	field_srvarg(FIELD *,const uchar *,char *,int) ;
extern int	getpwd(char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct svcfile_svcname {
	const char	*svcname ;
	int		count ;
} ;

struct xsvcfile_file {
	const char	*fname ;
	time_t		mtime ;
	uino_t		ino ;
	dev_t		dev ;
	int		size ;
} ;

struct svcfile_keyname {
	const char	*kname ;
	int		count ;
} ;

struct svcfile_ie {
	const char	*(*keyvals)[2] ;
	const char	*svc ;
	int		nkeys ;			/* number of keys */
	int		size ;			/* total size */
	int		fi ;			/* file index */
} ;

struct svcentry {
	vecobj		keys ;
	const char	*svc ;
} ;

struct svcentry_key {
	const char	*kname ;
	const char	*args ;
	int		kl, al ;
} ;


/* forward references */

int		svcfile_fileadd(SVCFILE *,const char *) ;
int		svcfile_check(SVCFILE *,time_t) ;
int		svcfile_fetch(SVCFILE *,const char *,SVCFILE_CUR *,
			SVCFILE_ENT *,char *,int) ;

static int	svcfile_filefins(SVCFILE *) ;
static int	svcfile_fileparse(SVCFILE *,int) ;
static int	svcfile_fileparser(SVCFILE *,int,const char *) ;
static int	svcfile_filedump(SVCFILE *,int) ;

#if	CF_FILEDEL
static int	svcfile_filedel(SVCFILE *,int) ;
#endif /* CF_FILEDEL */

static int	svcfile_addentry(SVCFILE *,int,SVCENTRY *) ;
static int	svcfile_checkfiles(SVCFILE *,time_t) ;

static int	svcfile_svcadd(SVCFILE *,const char *) ;
static int	svcfile_svcdel(SVCFILE *,const char *) ;
static int	svcfile_svcfins(SVCFILE *) ;

#if	CF_ALREADY
static int	svcfile_already(SVCFILE *,const char *) ;
#endif

#if	CF_DEVINO
static int	svcfile_filealready(SVCFILE *,dev_t,uino_t) ;
#endif

static int	svcentry_start(SVCENTRY *,cchar *) ;
static int	svcentry_addkey(SVCENTRY *,cchar *,int,cchar *,int) ;
static int	svcentry_nkeys(SVCENTRY *) ;
static int	svcentry_size(SVCENTRY *) ;
static int	svcentry_finish(SVCENTRY *) ;

static int	file_start(SVCFILE_FILE *,const char *) ;
static int	file_finish(SVCFILE_FILE *) ;

static int	svcname_start(SVCFILE_SVCNAME *,const char *) ;
static int	svcname_incr(SVCFILE_SVCNAME *) ;
static int	svcname_decr(SVCFILE_SVCNAME *) ;
static int	svcname_finish(SVCFILE_SVCNAME *) ;

static int	ientry_loadstr(SVCFILE_IENT *,char *,SVCENTRY *) ;
static int	ientry_finish(SVCFILE_IENT *) ;
#if	CF_MOREKEYS
static int	ientry_morekeys(SVCFILE_IENT *,int,int) ;
#endif

static int	entry_load(SVCFILE_ENT *,char *,int,SVCFILE_IENT *) ;

static int	cmpfname() ;
static int	cmpsvcname() ;


/* local variables */

/* all white space, pound ('#'), colon (':'), and comma (',') */
static const unsigned char	fterms[] = {
	0x00, 0x1F, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x24,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

#ifdef	COMMENT
/* key field terminators ('#', ',', ':', '=') */
static const unsigned char 	kterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x24,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
} ;
#endif /* COMMENT */

/* argument field terminators (pound '#' and comma ',') */
static const unsigned char 	saterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int svcfile_open(SVCFILE *op,cchar *fname)
{
	int		rs = SR_OK ;
	int		size ;
	int		opts ;
	int		n ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("svcfile_open: ent fname=%s\n",fname) ;
#endif

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(SVCFILE)) ;

/* this vector structure must remain fixed so that indices do not change */

	size = sizeof(SVCFILE_FILE) ;
	n = DEFNFILES ;
	opts = (VECOBJ_OSTATIONARY | VECOBJ_OREUSE) ;
	if ((rs = vecobj_start(&op->files,size,n,opts)) >= 0) {
	    size = sizeof(SVCFILE_SVCNAME) ;
	    opts = VECOBJ_OCOMPACT ;
	    if ((rs = vecobj_start(&op->svcnames,size,10,opts)) >= 0) {
	        n = DEFNENTRIES ;
	        if ((rs = hdb_start(&op->entries,n,0,NULL,NULL)) >= 0) {
	            op->magic = SVCFILE_MAGIC ;
	            op->checktime = time(NULL) ;
	            if (fname != NULL) {
	                rs = svcfile_fileadd(op,fname) ;
	                c = rs ;
	            }
	            if (rs < 0) {
	                hdb_finish(&op->entries) ;
	                op->magic = 0 ;
	            }
	        } /* end if (hdb-start) */
	        if (rs < 0) {
	            svcfile_svcfins(op) ;
	            vecobj_finish(&op->svcnames) ;
	        }
	    } /* end if svcnames) */
	    if (rs < 0) {
	        vecobj_finish(&op->files) ;
	    }
	} /* end if (vecobj_start) */

#if	CF_DEBUGS
	debugprintf("svcfile_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (svcfile_open) */


/* free up the resources occupied by an SVCFILE list object */
int svcfile_close(SVCFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

/* secondary items */

	rs1 = svcfile_filedump(op,-1) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = svcfile_filefins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = svcfile_svcfins(op) ;
	if (rs >= 0) rs = rs1 ;

/* primary items */

	rs1 = hdb_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->svcnames) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (svcfile_close) */


/* add a file to the list of files */
int svcfile_fileadd(SVCFILE *op,cchar *fname)
{
	int		rs = SR_OK ;
	int		fi = 0 ;
	const char	*np ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("svcfile_fileadd: fname=%s\n",fname) ;
#endif

	np = (const char *) fname ;
	if (fname[0] != '/') {
	    char	pwdbuf[MAXPATHLEN+1] ;
	    np = tmpfname ;
	    if ((rs = getpwd(pwdbuf,MAXPATHLEN)) >= 0) {
	        rs = mkpath2(tmpfname,pwdbuf,fname) ;
	    }
	} /* end if (added PWD) */

#if	CF_DEBUGS
	debugprintf("svcfile_fileadd: rs=%d np=%s\n",rs,np) ;
#endif

	if (rs >= 0) {
	    SVCFILE_FILE	fe ;
	    vecobj		*flp = &op->files ;
	    if ((rs = file_start(&fe,np)) >= 0) {
	        const int	nrs = SR_NOTFOUND ;
	        int		f_fin = FALSE ;
	        if ((rs = vecobj_search(flp,&fe,cmpfname,NULL)) == nrs) {
	            if ((rs = vecobj_add(flp,&fe)) >= 0) {
	                fi = rs ;
	                rs = svcfile_fileparse(op,fi) ;
	                if (rs < 0) {
	                    f_fin = TRUE ;
	                    vecobj_del(flp,fi) ;
	                }
	            } /* end if (vecobj_add) */
	        } else {
	            f_fin = TRUE ;
	        }
	        if ((rs < 0) || f_fin) {
	            file_finish(&fe) ;
	        }
	    } /* end if (file_start) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("svcfile_fileadd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (svcfile_fileadd) */


/* cursor manipulations */
int svcfile_curbegin(SVCFILE *op,SVCFILE_CUR *curp)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("svcfile_curbegin: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("svcfile_curbegin: continuing\n") ;
#endif

	if (op->ncursors == 0) {
	    rs = svcfile_check(op,0L) ;
	}

	if (rs >= 0) {
	    curp->i = -1 ;
	    if ((rs = hdb_curbegin(&op->entries,&curp->ec)) >= 0) {
	        op->ncursors += 1 ;
	        op->magic = SVCFILE_MAGIC ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("svcfile_curbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (svcfile_curbegin) */


int svcfile_curend(SVCFILE *op,SVCFILE_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	if ((rs = hdb_curend(&op->entries,&curp->ec)) >= 0) {
	    if (op->ncursors > 0) op->ncursors -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("svcfile_curend: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (svcfile_curend) */


int svcfile_enumsvc(SVCFILE *op,SVCFILE_CUR *curp,char *ebuf,int elen)
{
	SVCFILE_SVCNAME	*snp ;
	int		rs = SR_OK ;
	int		i ;
	int		kl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

	i = (curp->i >= 0) ? (curp->i + 1) : 0 ;

	while ((rs = vecobj_get(&op->svcnames,i,&snp)) >= 0) {
	    if (snp != NULL) break ;
	    i += 1 ;
	} /* end while */

	if (rs >= 0) {
	    if ((rs = sncpy1(ebuf,elen,snp->svcname)) >= 0) {
	        kl = rs ;
	        curp->i = i ;
	    }
	} /* end if */

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (svcfile_enumsvc) */


/* enumerate the entries */
int svcfile_enum(op,curp,ep,ebuf,elen)
SVCFILE		*op ;
SVCFILE_CUR	*curp ;
SVCFILE_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs = SR_OK ;
	int		svclen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if ((ep == NULL) || (ebuf == NULL)) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

	if (elen <= 0) return SR_OVERFLOW ;

	cur = curp->ec ;
	if ((rs = hdb_enum(&op->entries,&cur,&key,&val)) >= 0) {
	    SVCFILE_IENT	*iep = (struct svcfile_ie *) val.buf ;

#if	CF_DEBUGS
	    debugprintf("svcfile_enum: ie size=%u nkeys=%u svc=%s\n",
	        iep->size,iep->nkeys,iep->svc) ;
#endif

	    if ((ep != NULL) && (ebuf != NULL)) {
	        rs = entry_load(ep,ebuf,elen,iep) ;
	        svclen = rs ;
	    } else {
	        svclen = strlen(iep->svc) ;
	    }

	    if (rs >= 0) {
	        curp->ec = cur ;
	    }

	} /* end if (had an entry) */

#if	CF_DEBUGS
	debugprintf("svcfile_enum: svc=%s \n",ep->svc) ;
	debugprintf("svcfile_enum: ret rs=%d svclen=%u\n",rs,svclen) ;
#endif

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (svcfile_enum) */


int svcfile_fetch(op,svcname,curp,ep,ebuf,elen)
SVCFILE		*op ;
const char	svcname[] ;
SVCFILE_CUR	*curp ;
SVCFILE_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs = SR_OK ;
	int		svclen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (svcname == NULL) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("svcfile_fetch: svc=%s \n",svcname) ;
#endif

/* check for update */

	if (op->ncursors == 0) {
	    rs = svcfile_check(op,0L) ;
	}

	if (rs >= 0) {

/* continue */

	    if (curp == NULL) {
	        rs = hdb_curbegin(&op->entries,&cur) ;
	    } else {
	        cur = curp->ec ;
	    }

#if	CF_DEBUGS
	    debugprintf("svcfile_fetch: fetch\n") ;
#endif

	    if (rs >= 0) {
	        key.buf = (void *) svcname ;
	        key.len = strlen(svcname) ;
	        if ((rs = hdb_fetch(&op->entries,key,&cur,&val)) >= 0) {
	            SVCFILE_IENT	*iep = (struct svcfile_ie *) val.buf ;

	            if ((ep != NULL) && (ebuf != NULL)) {

#if	CF_DEBUGS
	                debugprintf("svcfile_fetch: entry_load() \n") ;
#endif

	                rs = entry_load(ep,ebuf,elen,iep) ;
	                svclen = rs ;

#if	CF_DEBUGS
	                debugprintf("svcfile_fetch: entry_load() rs=%d\n",rs) ;
#endif

	            } else {
	                svclen = strlen(iep->svc) ;
	            }

	            if ((rs >= 0) && (curp != NULL)) {
	                curp->ec = cur ;
	            }

	        } /* end if (had an entry) */
	        if (curp == NULL) {
	            hdb_curend(&op->entries,&cur) ;
		}
	    } /* end if (ok) */

	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("svcfile_fetch: ret rs=%d sl=%u\n",rs,svclen) ;
#endif

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (svcfile_fetch) */


/* check if the access tables files have changed */
int svcfile_check(SVCFILE *op,time_t daytime)
{
	const int	to = SVCFILE_INTCHECK ;
	int		rs = SR_OK ;
	int		c = 0 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != SVCFILE_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0)
	    daytime = time(NULL) ;

#if	CF_DEBUGS
	{
	    char	timebuf[TIMEBUFLEN + 1] ;
	    debugprintf("svcfile_check: ent %s\n",
	        timestr_logz(daytime,timebuf)) ;
	}
#endif

/* should we even check? */

	if ((op->ncursors == 0) && ((daytime - op->checktime) >= to)) {
	    op->checktime = daytime ;
	    rs = svcfile_checkfiles(op,daytime) ;
	    c = rs ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("svcfile_check: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (svcfile_check) */


int svcfile_match(SVCFILE *sfp,cchar *name)
{
	int		rs ;

	rs = svcfile_fetch(sfp,name,NULL,NULL,NULL,0) ;

	return rs ;
}
/* end subroutine (svcfile_match) */


/* private subroutines */


/* free up all of the files in this SVCFILE list */
static int svcfile_filefins(SVCFILE *op)
{
	SVCFILE_FILE	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep != NULL) {
	        rs1 = file_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (svcfile_filefins) */


/* check if the access table files have changed */
/* ARGSUSED */
static int svcfile_checkfiles(SVCFILE *op,time_t daytime)
{
	struct ustat	sb ;
	SVCFILE_FILE	*fep ;
	const int	wt = SVCFILE_INTWAIT ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c_changed = 0 ;

/* check the files */

#if	CF_DEBUGS
	debugprintf("svcfile_checkfiles: loop-begin\n") ;
#endif

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    rs1 = u_stat(fep->fname,&sb) ;

	    if ((rs1 >= 0) && ((sb.st_mtime - fep->mtime) >= wt)) {

#if	CF_DEBUGS
	        debugprintf("svcfile_checkfiles: file=%u changed\n",i) ;
	        debugprintf("svcfile_checkfiles: freeing file entries\n") ;
#endif

	        c_changed += 1 ;
	        svcfile_filedump(op,i) ;

#if	CF_DEBUGS
	        debugprintf("svcfile_checkfiles: parsing the file again\n") ;
#endif

	        rs = svcfile_fileparse(op,i) ;

#if	CF_DEBUGS
	        debugprintf("svcfile_checkfiles: svcfile_fileparse rs=%d\n",
	            rs) ;
#endif

	    } /* end if (changed) */

	    if (rs < 0) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("svcfile_checkfiles: ret rs=%d changed=%u\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (svcfile_checkfiles) */


static int svcfile_fileparse(SVCFILE *op,int fi)
{
	SVCFILE_FILE	*fep ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("svcfile_fileparse: ent fi=%u\n",fi) ;
#endif

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        struct ustat	sb ;
	        const char	*fname = fep->fname ;
	        if ((rs = u_stat(fname,&sb)) >= 0) {
	            if (sb.st_mtime > fep->mtime) {
	                fep->dev = sb.st_dev ;
	                fep->ino = sb.st_ino ;
	                fep->mtime = sb.st_mtime ;
	                fep->size = sb.st_size ;
	                rs = svcfile_fileparser(op,fi,fname) ;
	                c = rs ;
	            } /* end if (need new parsing) */
	        } /* end if (stat) */
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} /* end if (vecstr_get) */

#if	CF_DEBUGS
	debugprintf("svcfile_fileparse: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (svcfile_fileparse) */


static int svcfile_fileparser(SVCFILE *op,int fi,cchar *fname)
{
	bfile		svcfile, *lfp = &svcfile ;
	int		rs ;
	int		rs1 ;
	int		fl, kl, al ;
	int		c = 0 ;
	const char	*fp ;
	const char	*kp ;

#if	CF_DEBUGS
	debugprintf("svcfile_fileparse: ent fi=%u\n",fi) ;
#endif

	if ((rs = bopen(lfp,fname,"r",0664)) >= 0) {
	    SVCENTRY	se ;
	    FIELD	fsb ;
	    const int	llen = LINEBUFLEN ;
	    const int	alen = ABUFLEN ;
	    int		fn = 0 ;
	    int		len ;
	    int		cl ;
	    int		c_field ;
	    int		f_ent = FALSE ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    cchar	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    char	abuf[ABUFLEN + 1] ;
	    char	svcname[SVCNAMELEN + 1] ;
	    svcname[0] = '\0' ;

	    while ((rs = breadline(lfp,lbuf,llen)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;
	        if (f_eol) len -= 1 ;
	        lbuf[len] = '\0' ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	        debugprintf("svcfile_fileparse: line>%t<\n",lbuf,len) ;
#endif

	        cp = lbuf ;
	        cl = len ;
	        while (CHAR_ISWHITE(*cp)) {
	            cp += 1 ;
	            cl -= 1 ;
	        }

	        if ((*cp == '\0') || (*cp == '#')) continue ;
	        if (! f_bol) continue ;

	        c_field = 0 ;
	        if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	            while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {

#if	CF_DEBUGS && CF_DEBUGSFILE
	                debugprintf("svcfile_fileparse: c_field=%u\n",c_field) ;
	                debugprintf("svcfile_fileparse: ft=>%c< fl=%d\n",
	                    fsb.term,fl) ;
	                debugprintf("svcfile_fileparse: f=>%t<\n",
	                    fp,fl) ;
#endif

	                if ((c_field++ == 0) && (fsb.term == ':')) {

	                    fn = 0 ;
	                    strwcpy(svcname,fp,MIN(fl,SVCNAMELEN)) ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	                    debugprintf("svcfile_fileparse: svc=%s\n",svcname) ;
#endif

	                } else if ((fl > 0) && (svcname[0] != '\0')) {

#if	CF_DEBUGS && CF_DEBUGSFILE
	                    debugprintf("svcfile_fileparse: value w/ key\n") ;
#endif

/* create a SVCENTRY if found a first key */

#if	CF_DEBUGS && CF_DEBUGSFILE
	                    debugprintf("svcfile_fileparse: c=%u\n",c) ;
#endif

	                    if (fn++ == 0) {

#if	CF_DEBUGS && CF_DEBUGSFILE
	                        debugprintf("svcfile_fileparse: f_ent=%u\n",
	                            f_ent) ;
#endif

	                        if (f_ent) {

	                            if (rs >= 0) {
	                                c += 1 ;
	                                rs = svcfile_addentry(op,fi,&se) ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	                                debugprintf("svcfile_fileparse: "
	                                    "svcfile_addentry() "
	                                    "rs=%d\n",rs) ;
#endif

	                            }

	                            f_ent = FALSE ;
	                            svcentry_finish(&se) ;
	                        } /* end if */

	                        if (rs >= 0) {
	                            rs = svcentry_start(&se,svcname) ;
	                            f_ent = (rs >= 0) ;
	                        }

	                    } /* end if (created SVCENTRY) */

#if	CF_DEBUGS && CF_DEBUGSFILE
	                    debugprintf("svcfile_fileparse: f=>%t<\n",
	                        fp,strlinelen(fp,fl,60)) ;
#endif

	                    kp = fp ;
	                    kl = fl ;
	                    abuf[0] = '\0' ;
	                    al = 0 ;
	                    if (fsb.term != ',') {

#if	CF_DEBUGS && CF_DEBUGSFILE
	                        debugprintf("svcfile_fileparse: not comma\n") ;
#endif

	                        al = field_srvarg(&fsb,saterms,abuf,alen) ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	                        debugprintf("svcfile_fileparse: a=>%t<\n",
	                            abuf,strlinelen(abuf,al,60)) ;
#endif

	                    }

	                    if ((rs >= 0) && f_ent) {
	                        rs = svcentry_addkey(&se,kp,kl,abuf,al) ;
#if	CF_DEBUGS && CF_DEBUGSFILE
	                        debugprintf("svcfile_fileparse: "
	                            "svcentry_addkey() rs=%d\n", rs) ;
#endif
	                    }

	                } /* end if (handling record) */

	                if (fsb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while (fields) */

	            field_finish(&fsb) ;
	        } /* end if (field) */

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading extended lines) */

#if	CF_DEBUGS
	    debugprintf("svcfile_fileparse: while-out\n") ;
#endif

	    if (f_ent) {
	        if (rs >= 0) {
	            c += 1 ;
	            rs = svcfile_addentry(op,fi,&se) ;
	        }
	        f_ent = FALSE ;
	        rs1 = svcentry_finish(&se) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (extra entry) */

	    rs1 = bclose(lfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

	if (rs < 0) {
	    svcfile_filedump(op,fi) ;
	}

#if	CF_DEBUGS
	debugprintf("svcfile_fileparse: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (svcfile_fileparser) */


#if	CF_DEVINO
static int svcfile_filealready(SVCFILE *op,dev_t dev,uino_t ino)
{
	SVCFILE_FILE	*fep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		f = FALSE ;

	for (i = 0 ; (rs1 = vecobj_get(&op->files,i,&fep)) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;
	    f = ((fep->dev == dev) && (fep->ino == ino)) ;
	    if (f) break ;
	} /* end for */
	if ((rs >= 0) && (rs1 != SR_NOTFOUND)) rs = rs1 ;

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (svcfile_filealready) */
#endif /* CF_DEVINO */


/* add an entry to the access entry list */
static int svcfile_addentry(SVCFILE *op,int fi,SVCENTRY *nep)
{
	SVCFILE_IENT	*iep ;
	int		size ;
	int		rs ;
	int		f_added = FALSE ;

#if	CF_ALREADY
	if ((rs1 = svcfile_already(op,nep->svc)) == SR_NOTFOUND) {
#endif

	    f_added = TRUE ;
	    size = sizeof(SVCFILE_IENT) ;
	    if ((rs = uc_malloc(size,&iep)) >= 0) {
	        const int	n = svcentry_nkeys(nep) ;
	        void		*p ;
	        iep->fi = fi ;
	        size = (n+1) * 2 * sizeof(char *) ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            char	*bp ;
	            iep->nkeys = n ;
	            iep->keyvals = p ;
	            size = svcentry_size(nep) ;
	            if ((rs = uc_malloc(size,&bp)) >= 0) {
	                iep->size = size ;
	                if ((rs = ientry_loadstr(iep,bp,nep)) >= 0) {
	                    HDB_DATUM	key, val ;
	                    const int	sl = rs ;
	                    key.buf = iep->svc ;
	                    key.len = sl ;
	                    val.buf = iep ;
	                    val.len = sizeof(SVCFILE_IENT) ;
	                    if ((rs = hdb_store(&op->entries,key,val)) >= 0) {
	                        rs = svcfile_svcadd(op,iep->svc) ;
	                        if (rs < 0)
	                            hdb_delkey(&op->entries,key) ;
	                    } /* end if (hdb_store) */
	                } /* end if (ientry_loadstr) */
	                if (rs < 0) {
	                    iep->svc = NULL ;
	                    uc_free(bp) ;
	                }
	            } /* end if (memory-allocation) */
	            if (rs < 0) {
	                uc_free(iep->keyvals) ;
	                iep->keyvals = NULL ;
	            }
	        } /* end if (memory-allocation) */
	        if (rs < 0)
	            uc_free(iep) ;
	    } /* end if (memory-allocation) */

#if	CF_ALREADY
	}
#endif

#if	CF_DEBUGS
	debugprintf("svcfile_addentry: svc=%s\n",iep->svc) ;
	debugprintf("svcfile_addentry: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f_added : rs ;
}
/* end subroutine (svcfile_addentry) */


#if	CF_ALREADY
static int svcfile_already(SVCFILE *op,cchar *svcname)
{
	HDB_DATUM	key ;
	int		rs ;

	key.buf = (void *) svcname ;
	key.len = strlen(svcname) ;
	rs = hdb_fetch(&op->entries,key,NULL,NULL) ;

	return rs ;
}
/* end subroutine (svcfile_already) */
#endif /* CF_ALREADY */


/* free up all of the entries in this SVCFILE list associated w/ a file */
static int svcfile_filedump(SVCFILE *op,int fi)
{
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("svcfile_filedump: delete all fi=%d\n",fi) ;
#endif

	if ((rs = hdb_curbegin(&op->entries,&cur)) >= 0) {
	    SVCFILE_IENT	*ep ;
	    while (hdb_enum(&op->entries,&cur,&key,&val) >= 0) {

	        ep = (SVCFILE_IENT *) val.buf ;

	        if ((ep->fi == fi) || (fi < 0)) {

#if	CF_DEBUGS
	            debugprintf("svcfile_filedump: svc=%s\n",ep->svc) ;
#endif

	            c += 1 ;
	            rs1 = hdb_delcur(&op->entries,&cur,0) ;
	            if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	            debugprintf("svcfile_filedump: hdb_delcur() rs=%d\n",
	                rs1) ;
#endif
	            rs1 = svcfile_svcdel(op,ep->svc) ;
	            if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	            debugprintf("svcfile_filedump: _svcdel() rs=%d\n",rs1) ;
#endif
	            rs1 = ientry_finish(ep) ;
	            if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	            debugprintf("svcfile_filedump: _iefin() rs=%d\n",rs1) ;
#endif

	            rs1 = uc_free(ep) ;
	            if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	            debugprintf("svcfile_filedump: uc_free() rs=%d\n",rs1) ;
#endif
	        } /* end if (found matching entry) */

	    } /* end while (looping through entries) */
	    rs1 = hdb_curend(&op->entries,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */
#if	CF_DEBUGS
	debugprintf("svcfile_filedump: ret rs=%d c=%u\n",rs,c) ;
#endif

	return rs ;
}
/* end subroutine (svcfile_filedump) */


#if	CF_FILEDEL
static int svcfile_filedel(SVCFILE *op,int fi)
{
	SVCFILE_FILE	*fep ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        rs1 = file_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_del(&op->files,fi) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if */
	} /* end if (vecobj_get) */

	return rs ;
}
/* end subroutine (svcfile_filedel) */
#endif /* CF_FILEDEL */


static int svcfile_svcadd(SVCFILE *op,cchar *svc)
{
	SVCFILE_SVCNAME	sn, *snp ;
	VECOBJ		*lp = &op->svcnames ;
	int		rs ;
	int		f_added = FALSE ;

	sn.svcname = svc ;
	sn.count = 1 ;
	if ((rs = vecobj_search(lp,&sn,cmpsvcname,&snp)) >= 0) {
	    rs = svcname_incr(snp) ;
	} else if (rs == SR_NOTFOUND) {
	    f_added = TRUE ;
	    if ((rs = svcname_start(&sn,svc)) >= 0) {
	        rs = vecobj_add(lp,&sn) ;
	        if (rs < 0)
	            svcname_finish(&sn) ;
	    } /* end if (memory-allocation) */
	} /* end if */

	return (rs >= 0) ? f_added : rs ;
}
/* end subroutine (svcfile_svcadd) */


static int svcfile_svcdel(SVCFILE *op,cchar *svc)
{
	SVCFILE_SVCNAME	sn, *snp ;
	VECOBJ		*lp = &op->svcnames ;
	int		rs ;
	int		rs1 ;
	int		si = 0 ;

#if	CF_DEBUGS
	debugprintf("svcfile_svcdel: svc=%s\n",svc) ;
#endif

	sn.svcname = svc ;
	sn.count = 0 ;
	if ((rs = vecobj_search(lp,&sn,cmpsvcname,&snp)) >= 0) {
	    si = rs ;
	    rs1 = svcname_decr(snp) ;
	    if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	    debugprintf("svcfile_svcdel: svcname_decr() rs=%d\n",rs1) ;
#endif
	    if (rs1 == 0) {
	        rs1 = svcname_finish(snp) ;
	        if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	        debugprintf("svcfile_svcdel: svcname_finish() rs=%d\n",rs1) ;
#endif
	        rs1 = vecobj_del(lp,si) ;
	        if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	        debugprintf("svcfile_svcdel: vecobj_del() rs=%d\n",rs1) ;
#endif
	    } /* end if (count == 0) */
	} else if (rs == SR_NOTFOUND)
	    rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("svcfile_svcdel: ret rs=%d si=%u\n",rs,si) ;
#endif

	return (rs >= 0) ? si : rs ;
}
/* end subroutine (svcfile_svcdel) */


static int svcfile_svcfins(SVCFILE *op)
{
	SVCFILE_SVCNAME	*snp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;

	for (i = 0 ; vecobj_get(&op->svcnames,i,&snp) >= 0 ; i += 1) {
	    if (snp != NULL) {
	        c += 1 ;
	        rs1 = svcname_finish(snp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("svcfile_svcfins: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (svcfile_svcfins) */


static int file_start(SVCFILE_FILE *fep,cchar *fname)
{
	int		rs ;
	const char	*cp ;

	if (fname == NULL) return SR_FAULT ;

	memset(fep,0,sizeof(SVCFILE_FILE)) ;

	if ((rs = uc_mallocstrw(fname,-1,&cp)) >= 0) {
	    fep->fname = cp ;
	}

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(SVCFILE_FILE *fep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (fep == NULL) return SR_FAULT ;

	if (fep->fname != NULL) {
	    rs1 = uc_free(fep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (file_finish) */


static int svcname_start(SVCFILE_SVCNAME *snp,const char *svc)
{
	int		rs ;
	const char	*cp ;
#if	CF_DEBUGS
	debugprintf("svcname_start: svc=%s\n",svc) ;
#endif
	snp->count = 0 ;
	if ((rs = uc_mallocstrw(svc,-1,&cp)) >= 0) {
	    snp->count = 1 ;
	    snp->svcname = cp ;
	}
	return rs ;
}
/* end subroutine (svcname_start) */


static int svcname_finish(SVCFILE_SVCNAME *snp)
{
	const int	c = snp->count ;
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("svcname_finish: svc=%s c=%u\n",snp->svcname,c) ;
#endif
	snp->count = 0 ;
	if (snp->svcname != NULL) {
	    rs1 = uc_free(snp->svcname) ;
	    if (rs >= 0) rs = rs1 ;
	    snp->svcname = NULL ;
	}
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (svcname_finish) */


static int svcname_incr(SVCFILE_SVCNAME *snp)
{
	const int	c = snp->count ;
	snp->count += 1 ;
	return c ;
}
/* end subroutine (svcname_incr) */


static int svcname_decr(SVCFILE_SVCNAME *snp)
{
	int		rs = SR_OK ;
	int		rs1 ;
#if	CF_DEBUGS
	debugprintf("svcname_decr: svc=%s c=%u\n",snp->svcname,snp->count) ;
#endif
	if (snp->count > 0) snp->count -= 1 ;
	if (snp->count == 0) {
	    if (snp->svcname != NULL) {
	        rs1 = uc_free(snp->svcname) ;
	        if (rs >= 0) rs = rs1 ;
#if	CF_DEBUGS
	        debugprintf("svcname_decr: uc_free() rs=%d\n",rs1) ;
#endif
	        snp->svcname = NULL ;
	    }
	} /* end if (count == 0) */
	return (rs >= 0) ? snp->count : rs ;
}
/* end subroutine (svcname_decr) */


static int svcentry_start(SVCENTRY *sep,cchar *svc)
{
	int		rs ;
	const char	*cp ;

	memset(sep,0,sizeof(SVCENTRY)) ;

	if ((rs = uc_mallocstrw(svc,-1,&cp)) >= 0) {
	    const int	size = sizeof(SVCENTRY_KEY) ;
	    sep->svc = cp ;
	    rs = vecobj_start(&sep->keys,size,5,VECOBJ_PORDERED) ;
	    if (rs < 0) {
	        uc_free(cp) ;
	        sep->svc = NULL ;
	    }
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("svcentry_start: svc=%s\n",sep->svc) ;
#endif

	return rs ;
}
/* end subroutine (svcentry_start) */


/* free up an entry */
static int svcentry_finish(SVCENTRY *sep)
{
	SVCENTRY_KEY	*kep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&sep->keys,i,&kep) >= 0 ; i += 1) {
	    if (kep != NULL) {
	        if (kep->kname != NULL) {
	            rs1 = uc_free(kep->kname) ;
	            if (rs >= 0) rs = rs1 ;
	            kep->kname = NULL ;
	        }
	    }
	} /* end for */

	rs1 = vecobj_finish(&sep->keys) ;
	if (rs >= 0) rs = rs1 ;

	if (sep->svc != NULL) {
	    rs1 = uc_free(sep->svc) ;
	    if (rs >= 0) rs = rs1 ;
	    sep->svc = NULL ;
	}

	return rs ;
}
/* end subroutine (svcentry_finish) */


/* add a key to this entry */
static int svcentry_addkey(SVCENTRY *sep,cchar *kp,int kl,cchar *ap,int al)
{
	int		rs ;
	int		size = 0 ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("svcfile/svcentry_addkey: kl=%d kp=%t\n",kl,kp,kl) ;
	if (al >= 0)
	    debugprintf("svcfile/svcentry_addkey: al=%d ap=%t\n",al,ap,al) ;
#endif

	if (kl < 0) kl = strlen(kp) ;
	if (al < 0) al = strlen(ap) ;
	size += (kl+1) ;
	size += (al+1) ;
	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    SVCENTRY_KEY	key ;
	    key.kl = kl ;
	    key.kname = bp ;
	    key.al = 0 ;
	    key.args = NULL ;
	    bp = (strwcpy(bp,kp,kl) + 1) ;
	    if (al > 0) {
	        key.al = al ;
	        key.args = bp ;
	        bp = (strwcpy(bp,ap,al) + 1) ;
	    }
	    rs = vecobj_add(&sep->keys,&key) ;
	    if (rs < 0)
	        uc_free(key.kname) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (svcentry_addkey) */


static int svcentry_nkeys(SVCENTRY *sep)
{
	return vecobj_count(&sep->keys) ;
}
/* end subroutine (svcentry_nkeys) */


static int svcentry_size(SVCENTRY *sep)
{
	SVCENTRY_KEY	*kep ;
	int		i ;
	int		size = 0 ;
	size += (strlen(sep->svc) + 1) ;
	for (i = 0 ; vecobj_get(&sep->keys,i,&kep) >= 0 ; i += 1) {
	    if (kep != NULL) {
	        size += (kep->kl+1) ;
	        size += (kep->al+1) ;
	    }
	} /* end for */
	return size ;
}
/* end subroutine (svcentry_size) */


static int ientry_loadstr(SVCFILE_IENT *iep,char *bp,SVCENTRY *nep)
{
	SVCENTRY_KEY	*kep ;
	int		i ;
	int		j = 0 ;
	int		sl = strlen(nep->svc) ;
	iep->svc = bp ;
	bp = (strwcpy(bp,nep->svc,sl)+1) ;
	for (i = 0 ; vecobj_get(&nep->keys,i,&kep) >= 0 ; i += 1) {
	    if (kep != NULL) {
	        iep->keyvals[j][0] = NULL ;
	        iep->keyvals[j][1] = NULL ;
	        if (kep->kname != NULL) {
	            iep->keyvals[j][0] = bp ;
	            bp = (strwcpy(bp,kep->kname,-1)+1) ;
	        }
	        if (kep->args != NULL) {
	            iep->keyvals[j][1] = bp ;
	            bp = (strwcpy(bp,kep->args,-1)+1) ;
	        }
	        j += 1 ;
	    }
	} /* end for */
	iep->keyvals[j][0] = NULL ;
	iep->keyvals[j][1] = NULL ;
	return sl ;
}
/* end subroutine (ientry_loadstr) */


static int ientry_finish(SVCFILE_IENT *iep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (iep == NULL) return SR_FAULT ;

	if (iep->keyvals != NULL) {
	    rs1 = uc_free(iep->keyvals) ;
	    if (rs >= 0) rs = rs1 ;
	    iep->keyvals = NULL ;
	}

	if (iep->svc != NULL) {
	    rs1 = uc_free(iep->svc) ;
	    if (rs >= 0) rs = rs1 ;
	    iep->svc = NULL ;
	}

	return rs ;
}
/* end subroutine (ientry_finish) */


#if	CF_MOREKEYS
static int ientry_morekeys(SVCFILE_IENT *iep,int c,int i)
{
	int		f_more = TRUE ;

	if (c >= 0) f_more = (i < c) ;
	f_more = f_more && (iep->keyvals[i][0] != NULL) ;

	return f_more ;
}
/* end subroutine (ientry_morekeys) */
#endif /* CF_MOREKEYS */


/* load up the user-interface entry from the internal structure */
static int entry_load(SVCFILE_ENT *ep,char *ebuf,int elen,SVCFILE_IENT *iep)
{
	int		rs = SR_OK ;
	int		bo, i, kal ;
	int		rlen = 0 ;
	const char	*(*keyvals)[2] ;
	const char	*kp, *vp ;
	char		*bp ;

	if (iep == NULL) return SR_FAULT ;

	if ((ebuf != NULL) && (elen <= 0)) return SR_OVERFLOW ;

	bo = SVCFILE_BO((ulong) ebuf) ;

#if	CF_DEBUGS
	debugprintf("entry_load: bo=%u\n",bo) ;
#endif

	if (iep->size <= (elen - bo)) {

	    keyvals = (const char *(*)[2]) (ebuf + bo) ;
	    kal = (iep->nkeys + 1) * 2 * sizeof(char *) ;
	    bp = (char *) (ebuf + bo + kal) ;
#ifdef	COMMENT
	    bl = (elen - bo - kal) ;
#endif

	    ep->svc = bp ;
	    bp = strwcpy(bp,iep->svc,-1) + 1 ;

	    rlen = (bp - ep->svc - 1) ;

#if	CF_DEBUGS
	    debugprintf("entry_load: svc=%s rlen=%u\n",ep->svc,rlen) ;
	    debugprintf("entry_load: for-begin nkeys=%u\n",iep->nkeys) ;
#endif

	    for (i = 0 ; i < iep->nkeys ; i += 1) {

	        kp = iep->keyvals[i][0] ;
	        vp = iep->keyvals[i][1] ;

#if	CF_DEBUGS
	        debugprintf("entry_load: k[%u]=%s\n",i,kp) ;
	        debugprintf("entry_load: v[%u](%p)=>%s<\n",i,vp,vp) ;
#endif

	        keyvals[i][0] = bp ;
	        bp = strwcpy(bp,kp,-1) + 1 ;

#if	CF_DEBUGS && 0
	        debugprintf("entry_load: NA\n") ;
#endif
#if	CF_DEBUGS && 0
	        debugprintf("entry_load: k=%s\n",kp) ;
	        debugprintf("entry_load: v(%p)=>%s<\n",vp,vp) ;
#endif

	        if (vp != NULL) {
	            keyvals[i][1] = bp ;
	            bp = strwcpy(bp,vp,-1) + 1 ;
	        } else {
	            keyvals[i][1] = NULL ;
		}

	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("entry_load: for-end i=%u rs=%d\n",i,rs) ;
#endif

	    keyvals[i][0] = NULL ;
	    keyvals[i][1] = NULL ;

	    ep->keyvals = keyvals ;
	    ep->fi = iep->fi ;
	    ep->nkeys = iep->nkeys ;
	    ep->size = iep->size ;

	} else {
	    rs = SR_OVERFLOW ;
	}

#if	CF_DEBUGS
	debugprintf("entry_load: ret rs=%d rlen=%u\n",rs,rlen) ;
#endif

	return (rs >= 0) ? rlen : rs ;
}
/* end subroutine (entry_load) */


static int cmpfname(SVCFILE_FILE **e1pp,SVCFILE_FILE **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = strcmp((*e1pp)->fname,(*e2pp)->fname) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpfname) */


static int cmpsvcname(const void *v1p, const void *v2p)
{
	SVCFILE_SVCNAME	**e1pp = (SVCFILE_SVCNAME **) v1p ;
	SVCFILE_SVCNAME	**e2pp = (SVCFILE_SVCNAME **) v2p ;
	int		rc = 0 ;

	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
		    SVCFILE_SVCNAME	*e1p = (SVCFILE_SVCNAME *) *e1pp ;
		    SVCFILE_SVCNAME	*e2p = (SVCFILE_SVCNAME *) *e2pp ;
		    int		n1, n2 ;

		    n1 = (e1p->svcname == NULL) ;
		    n2 = (e2p->svcname == NULL) ;
		    if (n1 || n2) {
	    	        if (! (n1 && n2)) {
	        	    rc = (n1) ? 1 : -1 ;
	    	        }
		    }

#if	CF_DEBUGS
	debugprintf("svcfile/cmpsvcnames: s1=%s\n",e1p->svcname) ;
	debugprintf("svcfile/cmpsvcnames: s2=%s\n",e2p->svcname) ;
#endif

		    if ((rc == 0) && (! n1) && (! n2)) {
	    	        rc = strcmp(e1p->svcname,e2p->svcname) ;
	            }

	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}

	return rc ;
}
/* end subroutine (cmpsvcname) */


