/* nodedb */

/* magement for the NODE-DB file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFILE	0		/* for debugging file reading */
#define	CF_SAFECHECK	0		/* extra safety */


/* revision history:

	= 2004-05-25, David A­D­ Morano
	This subroutine was adopted for use as the node-database reader.

	= 2013-04-20, David A­D­ Morano
        I was totally on drugs when I originally wrote this. It was incredibly
        complicated. It allowed for service entries to span multiple lines (like
        a service-entry file). I must have been popping acid at the time. This
        service-entry-spanning-muliple lines business was never used, so I got
        rid of it. It all has to be and look so simple so that no errors are
        really possible. This is often easier said than done, but when some
        complicated behavior is not needed (at all), then we get rid of it in
        favor of much simpler implementations.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object manages access to one or more NODE-DB files.


*******************************************************************************/


#define	NODEDB_MASTER	1


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
#include	<vecobj.h>
#include	<hdb.h>
#include	<char.h>
#include	<localmisc.h>

#include	"nodedb.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif /* LINEBUFLEN */

#define	NODEDB_MINCHECKTIME	5	/* file check interval (seconds) */
#define	NODEDB_CHECKTIME	60	/* file check interval (seconds) */
#define	NODEDB_CHANGETIME	3	/* wait change interval (seconds) */
#define	NODEDB_DEFNETGROUP	"DEFAULT"

#define	NODEDB_FILE		struct nodedb_file
#define	NODEDB_KEYNAME		struct nodedb_keyname
#define	NODEDB_IE		struct nodedb_ie

#define	SVCENTRY		struct svcentry
#define	SVCENTRY_KEY		struct svcentry_key

#define	LINEINFO		struct lineinfo
#define	LINEINFO_FIELD		struct lineinfo_field

#define	NODEDB_KA		sizeof(char *(*)[2])
#define	NODEDB_BO(v)		\
	((NODEDB_KA - ((v) % NODEDB_KA)) % NODEDB_KA)

#undef	ARGSBUFLEN
#define	ARGSBUFLEN		(3 * MAXHOSTNAMELEN)

#undef	DEFCHUNKSIZE
#define	DEFCHUNKSIZE		512

#define	KEYALIGNMENT		sizeof(char *(*)[2])


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	getpwds(struct ustat *,char *,int) ;
extern int	getpwd(char *,int) ;
extern int	field_srvarg(FIELD *,const uchar *,char *,int) ;
extern int	isNotPresent(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct nodedb_file {
	const char	*fname ;
	time_t		mtime ;
	dev_t		dev ;
	uino_t		ino ;
	int		size ;
} ;

struct nodedb_keyname {
	const char	*kname ;
	int		count ;
} ;

struct nodedb_ie {
	const char	*(*keys)[2] ;
	const char	*svc, *clu, *sys ;
	const char	*a ;			/* allocation */
	int		nkeys ;			/* number of keys */
	int		size ;			/* total size */
	int		fi ;			/* file index */
} ;

struct lineinfo_field {
	const char	*fp ;
	int		fl ;
} ;

struct lineinfo {
	LINEINFO_FIELD	f[3] ;
} ;

struct svcentry {
	vecobj		keys ;
	const char	*svc ;
	const char	*clu ;
	const char	*sys ;
	const char	*a ;
} ;

struct svcentry_key {
	const char	*kname ;
	const char	*args ;
	int		kl, al ;
} ;


/* forward references */

int		nodedb_fileadd(NODEDB *,const char *) ;

#if	(NODEDB_MASTER == 1)
int		nodedb_curbegin(NODEDB *,NODEDB_CUR *) ;
int		nodedb_curend(NODEDB *,NODEDB_CUR *) ;
#endif

static int	nodedb_pwd(NODEDB *) ;
static int	nodedb_filefins(NODEDB *) ;
static int	nodedb_entfins(NODEDB *) ;
static int	nodedb_fileparse(NODEDB *,int) ;
static int	nodedb_fileparser(NODEDB *,NODEDB_FILE *,int) ;
static int	nodedb_fileparseline(NODEDB *,int,cchar *,int) ;
static int	nodedb_filedump(NODEDB *,int) ;
static int	nodedb_filedel(NODEDB *,int) ;
static int	nodedb_addentry(NODEDB *,int,SVCENTRY *) ;
static int	nodedb_checkfiles(NODEDB *,time_t) ;

static int	file_start(NODEDB_FILE *,const char *) ;
static int	file_finish(NODEDB_FILE *) ;

static int	ientry_start(NODEDB_IE *,int,SVCENTRY *) ;
static int	ientry_finish(NODEDB_IE *) ;

static int	svcentry_start(SVCENTRY *,LINEINFO *) ;
static int	svcentry_addkey(SVCENTRY *,cchar *,int,cchar *,int) ;
static int	svcentry_finish(SVCENTRY *) ;

static int	entry_load(NODEDB_ENT *,char *,int,NODEDB_IE *) ;

static int	cmpfname() ;

static int	freeit(const char **) ;


/* local variables */

/* all white-space, pound ('#'), colon (':'), equal ('='), and comma (',') */
static const uchar	fterms[] = {
	0x00, 0x0A, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x24,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

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


int nodedb_open(NODEDB *op,cchar *fname)
{
	int		rs ;
	int		defentries = 0 ;
	int		size ;

#if	CF_DEBUGS
	debugprintf("nodedb_open: ent fname=%s\n",fname) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if (defentries < NODEDB_DEFENTS)
	    defentries = NODEDB_DEFENTS ;

	memset(op,0,sizeof(NODEDB)) ;

	size = sizeof(NODEDB_FILE) ;
	if ((rs = vecobj_start(&op->files,size,10,VECOBJ_PREUSE)) >= 0) {
	    if ((rs = hdb_start(&op->entries,defentries,0,NULL,NULL)) >= 0) {
	        op->checktime = time(NULL) ;
	        op->magic = NODEDB_MAGIC ;
	        if ((fname != NULL) && (fname[0] != '\0')) {
	            rs = nodedb_fileadd(op,fname) ;
	            if (rs < 0) {
	                op->magic = 0 ;
	            }
	        } /* end if (had an optional file) */
	        if (rs < 0)
	            hdb_finish(&op->entries) ;
	    } /* end if (entries) */
	    if (rs < 0)
	        vecobj_finish(&op->files) ;
	} /* end if (files) */

#if	CF_DEBUGS
	debugprintf("nodedb_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (nodedb_open) */


/* free up the resources occupied by an NODEDB list object */
int nodedb_close(NODEDB *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

	rs1 = nodedb_entfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = nodedb_filefins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up the rest of the main object data */

	rs1 = hdb_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (nodedb_close) */


/* add a file to the list of files */
int nodedb_fileadd(NODEDB *op,cchar *fname)
{
	NODEDB_FILE	fe ;
	const int	rsn = SR_NOTFOUND ;
	int		rs = SR_OK ;
	const char	*np = fname ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("nodedb_fileadd: ent fname=%s\n",fname) ;
#endif

	if (fname[0] != '/') {
	    if ((rs = nodedb_pwd(op)) >= 0) {
	        np = tmpfname ;
	        rs = mkpath2(tmpfname,op->pwd,fname) ;
	    }
	} /* end if (added PWD) */

#if	CF_DEBUGS
	debugprintf("nodedb_fileadd: np=%s\n",np) ;
#endif

	if (rs >= 0) {
	    if ((rs = file_start(&fe,np)) >= 0) {
	        vecobj	*flp = &op->files ;
	        if ((rs = vecobj_search(flp,&fe,cmpfname,NULL)) == rsn) {
	            if ((rs = vecobj_add(flp,&fe)) >= 0) {
	                int	fi = rs ;
	                rs = nodedb_fileparse(op,fi) ;
	                if (rs < 0) {
	                    nodedb_filedel(op,fi) ;
	                }
	            } /* end if (vecobj_add) */
	        } /* end if (vecobj_search) */
	        if (rs < 0) {
	            file_finish(&fe) ;
	        }
	    } /* end if (file-start) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("nodedb_fileadd: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (nodedb_fileadd) */


/* cursor manipulations */
int nodedb_curbegin(NODEDB *op,NODEDB_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	if ((rs = hdb_curbegin(&op->entries,&curp->ec)) >= 0) {
	    op->cursors += 1 ;
	}

	return rs ;
}
/* end subroutine (nodedb_curbegin) */


int nodedb_curend(NODEDB *op,NODEDB_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	if ((rs = hdb_curend(&op->entries,&curp->ec)) >= 0) {
	    op->cursors -= 1 ;
	}

	return rs ;
}
/* end subroutine (nodedb_curend) */


/* enumerate the entries */
int nodedb_enum(NODEDB *op,NODEDB_CUR *curp,NODEDB_ENT *ep,char *ebuf,int elen)
{
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs ;
	int		svclen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (ep == NULL) return SR_FAULT ;
	if (ebuf == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

	cur = curp->ec ;
	if ((rs = hdb_enum(&op->entries,&cur,&key,&val)) >= 0) {
	    NODEDB_IE	*iep = (NODEDB_IE *) val.buf ;

#if	CF_DEBUGS
	    debugprintf("nodedb_enum: elen=%u\n",elen) ;
	    debugprintf("nodedb_enum: ie size=%u nkeys=%u svc=%s\n",
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
	debugprintf("nodedb_enum: svc=%s\n",ep->svc) ;
	debugprintf("nodedb_enum: ret rs=%d svclen=%u\n",rs,svclen) ;
#endif

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (nodedb_enum) */


int nodedb_fetch(NODEDB *op,cchar *svcbuf,NODEDB_CUR *curp,NODEDB_ENT *ep,
		char *ebuf,int elen)
{
	NODEDB_IE	*iep ;
	NODEDB_CUR	dbcur ;
	HDB_DATUM	key, val ;
	HDB_CUR		hcur ;
	int		rs ;
	int		svclen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (svcbuf == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("nodedb_fetch: ent elen=%u\n",elen) ;
#endif

	if (curp == NULL) {
	    curp = &dbcur ;
	    nodedb_curbegin(op,&dbcur) ;
	}

	key.buf = svcbuf ;
	key.len = strlen(svcbuf) ;

	hcur = curp->ec ;
	if ((rs = hdb_fetch(&op->entries,key,&hcur,&val)) >= 0) {
	    iep = (NODEDB_IE *) val.buf ;

#if	CF_DEBUGS
	    debugprintf("nodedb_fetch: ie size=%u nkeys=%u svc=%s\n",
	        iep->size,iep->nkeys,iep->svc) ;
#endif

	    if ((ep != NULL) && (ebuf != NULL)) {
	        rs = entry_load(ep,ebuf,elen,iep) ;
	        svclen = rs ;
	    } else {
	        svclen = strlen(iep->svc) ;
	    }

	    if (rs >= 0) {
	        curp->ec = hcur ;
	    }

	} /* end if (hdb_fetch) */

	if (curp == &dbcur) {
	    nodedb_curend(op,&dbcur) ;
	}

#if	CF_DEBUGS
	debugprintf("nodedb_fetch: ret rs=%d svclen=%u\n",rs,svclen) ;
#endif

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (nodedb_fetch) */


/* check if the underlying file has changed */
int nodedb_check(NODEDB *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

/* should we even check? */

	if (op->cursors == 0) {
	    time_t	dt = time(NULL) ;
	    if ((dt - op->checktime) > NODEDB_CHECKTIME) {
	        rs = nodedb_checkfiles(op,dt) ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("nodedb_check: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (nodedb_check) */


/* private subroutines */


static int nodedb_pwd(NODEDB *op)
{
	struct ustat	sb ;
	int		rs = SR_OK ;

	if ((op->pwd[0] != '\0') && (op->pwd_len > 0)) {
	    if ((rs = u_stat(".",&sb)) >= 0) {
	        if ((sb.st_ino != op->pwd_ino) ||
	            (sb.st_dev != op->pwd_dev)) {
	            op->pwd[0] = '\0' ;
	            op->pwd_len = 0 ;
	        }
	    }
	}

	if ((rs >= 0) && (op->pwd[0] == '\0')) {

/* load up the PWD */

	    op->pwd_len = 0 ;
	    rs = getpwds(&sb,op->pwd,MAXPATHLEN) ;
	    if (rs >= 0) op->pwd_len = rs ;

#ifdef	COMMENT
	    op->pwd[op->pwd_len] = '\0' ;
#endif

	    if (rs >= 0) {
	        op->pwd_ino = sb.st_ino ;
	        op->pwd_dev = sb.st_dev ;
	    }

	} /* end if (needed new PWD) */

	return (rs >= 0) ? op->pwd_len : rs ;
}
/* end subroutine (nodedb_pwd) */


/* free up all of the files in this NODEDB list */
static int nodedb_filefins(NODEDB *op)
{
	NODEDB_FILE	*fep ;
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
/* end subroutine (nodedb_filefins) */


/* check if the access table files have changed */
static int nodedb_checkfiles(NODEDB *op,time_t daytime)
{
	NODEDB_FILE	*fep ;
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		i ;
	int		c_changed = 0 ;

#if	CF_DEBUGS
	debugprintf("nodedb_checkfiles: ent\n") ;
#endif

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep != NULL) {
	        if ((rs = u_stat(fep->fname,&sb)) >= 0) {
	            if (sb.st_mtime > fep->mtime) {
	                c_changed += 1 ;
	                nodedb_filedump(op,i) ;
	                rs = nodedb_fileparse(op,i) ;
		    }
		} else if (isNotPresent(rs)) {
		    rs = SR_OK ;
	        } /* end if (u_stat) */
	    }
	    if (rs < 0) break ;
	} /* end for (looping through files) */

	op->checktime = daytime ;

#if	CF_DEBUGS
	debugprintf("nodedb_checkfiles: ret rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (nodedb_checkfiles) */


static int nodedb_fileparse(NODEDB *op,int fi)
{
	NODEDB_FILE	*fep ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("nodedb_fileparse: ent fi=%u\n",fi) ;
#endif

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        rs = nodedb_fileparser(op,fep,fi) ;
		c = rs ;
	        if (rs < 0) {
	            nodedb_filedump(op,fi) ;
	        }
	    } else {
#if	CF_DEBUGS
		debugprintf("nodedb_fileparse: NULL\n") ;
#endif
	        rs = SR_NOTFOUND ;
	    }
	} /* end if (vector_get) */

#if	CF_DEBUGS
	debugprintf("nodedb_fileparse: ret rs=%d added=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (nodedb_fileparse) */


static int nodedb_fileparser(NODEDB *op,NODEDB_FILE *fep,int fi)
{
	bfile		loadfile, *lfp = &loadfile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = bopen(lfp,fep->fname,"r",0664)) >= 0) {
	    USTAT	sb ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    if ((rs = bcontrol(lfp,BC_STAT,&sb)) >= 0) {
	        if (S_ISREG(sb.st_mode)) {
	            if (fep->mtime < sb.st_mtime) {
	                const int	llen = LINEBUFLEN ;
	                int		cl ;
	                cchar		*cp ;
	                char		lbuf[LINEBUFLEN + 1] ;
	                fep->dev = sb.st_dev ;
	                fep->ino = sb.st_ino ;
	                fep->mtime = sb.st_mtime ;
	                fep->size = sb.st_size ;
	                while ((rs = breadlines(lfp,lbuf,llen,NULL)) > 0) {
	                    int	len = rs ;

	                    f_eol = (lbuf[len - 1] == '\n') ;
	                    if (f_eol) len -= 1 ;
	                    lbuf[len] = '\0' ;

	                    if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                        if (f_bol && (*cp != '#')) {
	                            rs = nodedb_fileparseline(op,fi,cp,cl) ;
	                            if (rs > 0) c += 1 ;
	                        }
	                    }

	                    f_bol = f_eol ;
	                    if (rs < 0) break ;
	                } /* end while (reading extended lines) */
	            } /* end if (needed) */
	        } else {
	            rs = SR_ISDIR ;
	        }
	    } /* end if (bcontrol) */
	    rs1 = bclose(lfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (nodedb_fileparser) */


static int nodedb_fileparseline(NODEDB *op,int fi,cchar *lp,int ll)
{
	SVCENTRY		se ;
	LINEINFO		li ;
	FIELD		fsb ;
	int		rs ;
	int		c_field = 0 ;
	int		f_ent = FALSE ;

	memset(&li,0,sizeof(LINEINFO)) ;

	if ((rs = field_start(&fsb,lp,ll)) >= 0) {
	    const int	argslen = ARGSBUFLEN ;
	    int		fl ;
	    const char	*fp ;
	    char	argsbuf[ARGSBUFLEN + 1] ;

	    while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {

#if	CF_DEBUGS && CF_DEBUGSFILE
	        debugprintf("nodedb_fileparse: c_field=%u\n",c_field) ;
	        debugprintf("nodedb_fileparse: ft=>%c< fl=%d f=%t\n",
	            fsb.term,fp,fp,fl) ;
#endif

	        if ((c_field == 0) && (fl == 0))
	            break ;

	        if ((c_field < 3) && (fsb.term == ':')) {
	            li.f[c_field].fp = fp ;
	            li.f[c_field].fl = fl ;
	        }

	        if ((c_field >= 3) && (fl > 0)) {
	            const char	*kp = fp ;
	            int		kl = fl ;
	            int		al ;

	            if (! f_ent) {
	                rs = svcentry_start(&se,&li) ;
	                f_ent = (rs >= 0) ;
	                if (rs < 0) break ;
	            }

	            argsbuf[0] = '\0' ;
	            al = 0 ;
	            if (fsb.term == '=') {
	                al = field_srvarg(&fsb,saterms,argsbuf,argslen) ;
	            }

	            if ((rs >= 0) && f_ent) {
	                rs = svcentry_addkey(&se,kp,kl,argsbuf,al) ;
	            }

#if	CF_DEBUGS && CF_DEBUGSFILE
	            debugprintf("nodedb_fileparse: "
	                "svcentry_addkey() rs=%d\n", rs) ;
#endif

	        } /* end if (handling key-value pair) */

	        c_field += 1 ;
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while (fields) */

	    field_finish(&fsb) ;
	} /* end if (field) */

	if (rs >= 0) {
	    if ((! f_ent) && (c_field > 0) && (li.f[0].fl > 0)) {
	        rs = svcentry_start(&se,&li) ;
	        f_ent = (rs >= 0) ;
	    }
	}

	if ((rs >= 0) && f_ent) {
	    rs = nodedb_addentry(op,fi,&se) ;
	    svcentry_finish(&se) ;
	} /* end if (adding previous entry) */

	return (rs >= 0) ? f_ent : rs ;
}
/* end subroutine (nodedb_fileparseline) */


/* add an entry to the access entry list */
static int nodedb_addentry(NODEDB *op,int fi,SVCENTRY *sep)
{
	NODEDB_IE	*iep ;
	const int	size = sizeof(NODEDB_IE) ;
	int		rs ;

#if	CF_SAFECHECK
	if (sep == NULL) return SR_FAULT ;
#endif /* CF_SAFECHECK */

	if ((rs = uc_malloc(size,&iep)) >= 0) {
	    if ((rs = ientry_start(iep,fi,sep)) >= 0) {
	        HDB_DATUM	key, val ;
	        key.buf = iep->svc ;
	        key.len = strlen(iep->svc) ;
	        val.buf = iep ;
	        val.len = sizeof(NODEDB_IE) ;
	        rs = hdb_store(&op->entries,key,val) ;
	        if (rs < 0)
	            ientry_finish(iep) ;
	    }
	    if (rs < 0)
	        uc_free(iep) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (nodedb_addentry) */


/* free up all of the entries in this NODEDB list associated w/ a file */
static int nodedb_filedump(NODEDB *op,int fi)
{
	NODEDB_IE	*iep ;
	HDB		*elp = &op->entries ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("nodedb_filedump: fi=%d\n",fi) ;
#endif

	if ((rs = hdb_curbegin(elp,&cur)) >= 0) {

	    while (hdb_enum(elp,&cur,&key,&val) >= 0) {

	        iep = (NODEDB_IE *) val.buf ;

	        if ((iep->fi == fi) || (fi < 0)) {

	            c += 1 ;

#if	CF_DEBUGS
	            debugprintf("nodedb_filedump: svc=%s\n",iep->svc) ;
#endif

	            rs1 = hdb_delcur(elp,&cur,0) ;
	            if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	            debugprintf("nodedb_filedump: 1 \n") ;
#endif

	            rs1 = ientry_finish(iep) ;
	            if (rs >= 0) rs = rs1 ;

#if	CF_DEBUGS
	            debugprintf("nodedb_filedump: 2 freeing entry\n") ;
#endif

	            rs1 = uc_free(iep) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (found matching entry) */

	    } /* end while (looping through entries) */

	    rs1 = hdb_curend(elp,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

#if	CF_DEBUGS
	debugprintf("nodedb_filedump: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (nodedb_filedump) */


static int nodedb_filedel(NODEDB *op,int fi)
{
	NODEDB_FILE	*fep ;
	int		rs ;
	int		rs1 ;

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        rs1 = file_finish(fep) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = vecobj_del(&op->files,fi) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (nodedb_filedel) */


/* free up all of the entries in this NODEDB list */
static int nodedb_entfins(NODEDB *op)
{
	int		rs ;

	rs = nodedb_filedump(op,-1) ;

	return rs ;
}
/* end subroutine (nodedb_entfins) */


static int file_start(NODEDB_FILE *fep,cchar *fname)
{
	int		rs = SR_OK ;
	const char	*cp ;

#if	CF_SAFECHECK
	if (fname == NULL) return SR_FAULT ;
#endif /* CF_SAFECHECK */

	memset(fep,0,sizeof(NODEDB_FILE)) ;

	rs = uc_mallocstrw(fname,-1,&cp) ;
	if (rs >= 0) fep->fname = cp ;

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(NODEDB_FILE *fep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFECHECK
	if (fep == NULL) return SR_FAULT ;
#endif /* CF_SAFECHECK */

	if (fep->fname != NULL) {
	    rs1 = uc_free(fep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (file_finish) */


static int ientry_start(NODEDB_IE *iep,int fi,SVCENTRY *sep)
{
	int		rs ;
	int		size = 0 ;
	int		c = 0 ;

#if	CF_SAFECHECK
	if (iep == NULL) return SR_FAULT ;
#endif /* CF_SAFECHECK */

	memset(iep,0,sizeof(NODEDB_IE)) ;
	iep->fi = fi ;

/* basic checks on input data */

	if (sep == NULL) return SR_FAULT ;

	if ((sep->svc == NULL) || (sep->svc[0] == '\0')) return SR_INVALID ;

/* ok to continue */

	if ((rs = vecobj_count(&sep->keys)) >= 0) {
	    SVCENTRY_KEY	*kep ;
	    int		i ;
	    void	*p ;

	    iep->nkeys = rs ;
	    c = rs ;

/* find the size to allocate (everything) */

	    size += ((c + 1) * 2 * sizeof(char *)) ;

	    for (i = 0 ; vecobj_get(&sep->keys,i,&kep) >= 0 ; i += 1) {
	        if (kep != NULL) {
	            if (kep->kname != NULL) size += kep->kl ;
	            size += 1 ;
	            if (kep->args != NULL) size += kep->al ;
	            size += 1 ;
	        }
	    } /* end for */

	    for (i = 0 ; i < 3 ; i += 1) {
	        cchar	*cp ;
	        switch (i) {
	        case 0:
	            cp = sep->svc ;
	            break ;
	        case 1:
	            cp = sep->clu ;
	            break ;
	        case 2:
	            cp = sep->sys ;
	            break ;
	        } /* end switch */
	        size += (strlen(cp) + 1) ;
	    } /* end for */

/* allocate */

	    iep->size = size ;
	    if ((rs = uc_malloc(size,&p)) >= 0) {
	        int	j = 0 ;
	        cchar	*(*keys)[2] = p ;
	        char	*bp = p ;

	        iep->a = (const char *) p ;
	        iep->keys = keys ;

	        bp += ((c + 1) * 2 * sizeof(char *)) ;

/* copy over the key-table and the key and value strings */

	        for (i = 0 ; vecobj_get(&sep->keys,i,&kep) >= 0 ; i += 1) {
	            if (kep != NULL) {
	                if (kep->kname != NULL) {
	                    keys[j][0] = bp ;
	                    bp = strwcpy(bp,kep->kname,kep->kl) ;
	                    keys[j][1] = bp++ ;
	                    if (kep->args != NULL) {
	                        keys[j][1] = bp ;
	                        bp = (strwcpy(bp,kep->args,kep->al) + 1) ;
	                    }
	                    j += 1 ;
	                }
	            }
	        } /* end for */

	        keys[j][0] = NULL ;
	        keys[j][1] = NULL ;

/* copy over the other stuff */

	        for (i = 0 ; i < 3 ; i += 1) {
	            cchar	*cp ;
	            switch (i) {
	            case 0:
	                iep->svc = bp ;
	                cp = sep->svc ;
	                break ;
	            case 1:
	                iep->clu = bp ;
	                cp = sep->clu ;
	                break ;
	            case 2:
	                iep->sys = bp ;
	                cp = sep->sys ;
	                break ;
	            } /* end switch */
	            bp = (strwcpy(bp,cp,-1) + 1) ;
	        } /* end for */

	    } else {
	        iep->svc = NULL ;
	    } /* end if (m-a) */

	} /* end if (vecobj_count) */

#if	CF_DEBUGS
	debugprintf("nodedb/ientry_start: svc=%s\n",iep->svc) ;
	debugprintf("nodedb/ientry_start: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (ientry_start) */


static int ientry_finish(NODEDB_IE *iep)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFECHECK
	if (iep == NULL) return SR_FAULT ;
#endif /* CF_SAFECHECK */

	if (iep->a != NULL) {
	    rs1 = uc_free(iep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    iep->a = NULL ;
	}

	iep->svc = NULL ;
	return rs ;
}
/* end subroutine (ientry_finish) */


static int svcentry_start(SVCENTRY *sep,LINEINFO *lip)
{
	int		rs ;
	int		i ;
	int		size = 0 ;
	char		*bp ;

#if	CF_SAFECHECK
	if (lip == NULL) return SR_FAULT ;
#endif /* CF_SAFECHECK */

	memset(sep,0,sizeof(SVCENTRY)) ;

	for (i = 0 ; i < 3 ; i += 1) {
	    int		cl = lip->f[i].fl ;
	    const char	*cp = lip->f[i].fp ;
	    if (cl < 0) cl = strlen(cp) ;
	    size += (cl + 1) ;
	} /* end for */

	if ((rs = uc_malloc(size,&bp)) >= 0) {
	    sep->a = bp ;

	    for (i = 0 ; i < 3 ; i += 1) {
	        switch (i) {
	        case 0:
	            sep->svc = bp ;
	            break ;
	        case 1:
	            sep->clu = bp ;
	            break ;
	        case 2:
	            sep->sys = bp ;
	            break ;
	        } /* end switch */
	        bp = (strwcpy(bp,lip->f[i].fp,lip->f[i].fl) + 1) ;
	    } /* end for */

/* prepare for arguments */

	    size = sizeof(SVCENTRY_KEY) ;
	    rs = vecobj_start(&sep->keys,size,5,VECOBJ_PORDERED) ;
	    if (rs < 0) {
	        uc_free(sep->a) ;
	        sep->a = NULL ;
	    }
	} else {
	    sep->svc = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("svcentry_start: ret rs=%d svc=%s\n",rs,sep->svc) ;
#endif

	return rs ;
}
/* end subroutine (svcentry_start) */


/* add a key to this entry */
static int svcentry_addkey(SVCENTRY *sep,cchar *kp,int kl,cchar *ap,int al)
{
	SVCENTRY_KEY	key ;
	int		rs ;
	int		size = 0 ;
	char		*bp ;

#if	CF_SAFECHECK
	if (sep == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;
	if (kp[0] == '\0') return SR_INVALID ;
#endif /* CF_SAFECHECK */

#if	CF_DEBUGS
	debugprintf("nodedb/svcentry_addkey: kl=%d kp=%t\n",kl,kp,kl) ;
	if (al >= 0)
	    debugprintf("nodedb/svcentry_addkey: al=%d ap=%t\n",al,ap,al) ;
#endif

/* ok */

	memset(&key,0,sizeof(SVCENTRY_KEY)) ;

	if (kl < 0) kl = strlen(kp) ;
	size += (kl + 1) ;
	if (ap != NULL) {
	    if (al < 0) al = strlen(ap) ;
	    size += (al + 1) ;
	} else {
	    size += 1 ;
	}

	if ((rs = uc_malloc(size,&bp)) >= 0) {

/* copy over (load) the key-name */

	    key.kname = bp ;
	    bp = (strwcpy(bp,kp,kl) + 1) ;
	    key.kl = kl ;

/* copy over (load) the arguments (if any) */

	    key.args = bp ;
	    key.al = 0 ;
	    if (ap != NULL) {
	        bp = (strwcpy(bp,ap,al) + 1) ;
	        key.al = al ;
	    } else {
	        *bp++ = '\0' ;
	    }

/* add the key object to the key-list */

	    rs = vecobj_add(&sep->keys,&key) ;
	    if (rs < 0) {
	        if (key.kname != NULL) {
	            uc_free(key.kname) ;
	            key.kname = NULL ;
	            key.args = NULL ;
	        }
	    }

	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (svcentry_addkey) */


/* free up an entry */
static int svcentry_finish(SVCENTRY *sep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sep->svc != NULL) {
	    SVCENTRY_KEY	*kep ;
	    int			i ;
	    for (i = 0 ; vecobj_get(&sep->keys,i,&kep) >= 0 ; i += 1) {
	        if (kep != NULL) {
	            if (kep->kname != NULL) {
	                rs1 = uc_free(kep->kname) ;
	                if (rs >= 0) rs = rs1 ;
	            }
	        }
	    } /* end for */
	    rs1 = vecobj_finish(&sep->keys) ;
	    if (rs >= 0) rs = rs1 ;
	    rs1 = freeit(&sep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    sep->svc = NULL ;
	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (svcentry_finish) */


static int entry_load(NODEDB_ENT *ep,char *ebuf,int elen,NODEDB_IE *iep)
{
	int		rs = SR_OK ;
	int		bo ;
	int		svclen = 0 ;

#if	CF_DEBUGS
	debugprintf("entry_load: ent elen=%u\n",elen) ;
	debugprintf("entry_load: ie size=%u nkeys=%u svc=%s\n",
	    iep->size,iep->nkeys,iep->svc) ;
#endif

	bo = NODEDB_BO((ulong) ebuf) ;
	if (iep->size <= (elen - bo)) {
	    int		i ;
	    int		kal = (iep->nkeys + 1) * 2 * sizeof(char *) ;
	    cchar	*(*keys)[2] = (cchar *(*)[2]) (ebuf + bo) ;
	    char	*bp ;

	    bp = (char *) (ebuf + bo + kal) ;

/* copy in the nodename */

	    ep->svc = bp ;
	    bp = (strwcpy(bp,iep->svc,-1) + 1) ;

	    svclen = (bp - ep->svc - 1) ;

#if	CF_DEBUGS
	    debugprintf("nodedb_fetch: svc=%s svclen=%u\n",ep->svc,svclen) ;
#endif

/* copy in the clustername */

	    ep->clu = bp ;
	    if (iep->clu != NULL) {
	        bp = (strwcpy(bp,iep->clu,-1) + 1) ;
	    } else {
	        *bp++ = '\0' ;
	    }

/* copy in the systemname */

	    ep->sys = bp ;
	    if (iep->sys != NULL) {
	        bp = (strwcpy(bp,iep->sys,-1) + 1) ;
	    } else {
	        *bp++ = '\0' ;
	    }

/* copy in the key=values */

	    for (i = 0 ; i < iep->nkeys ; i += 1) {

	        keys[i][0] = bp ;
	        bp = (strwcpy(bp,iep->keys[i][0],-1) + 1) ;

#if	CF_DEBUGS
	        debugprintf("nodedb_fetch: k=%s\n",keys[i][0]) ;
#endif

	        if (iep->keys[i][1] != NULL) {
	            keys[i][1] = bp ;
	            bp = (strwcpy(bp,iep->keys[i][1],-1) + 1) ;
	        } else {
	            keys[i][1] = NULL ;
	        }

	    } /* end for */

	    keys[i][0] = NULL ;
	    keys[i][1] = NULL ;

	    ep->keys = keys ;
	    ep->fi = iep->fi ;
	    ep->nkeys = iep->nkeys ;
	    ep->size = iep->size ;

	} else {
	    rs = SR_OVERFLOW ;
	}

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (entry_load) */


static int freeit(cchar **pp)
{
	int		rs = SR_OK ;
	if (*pp != NULL) {
	    rs = uc_free(*pp) ;
	    *pp = NULL ;
	}
	return rs ;
}
/* end subroutine (freeit) */


static int cmpfname(NODEDB_FILE **e1pp,NODEDB_FILE **e2pp)
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


