/* kvsfile */

/* perform access table file related functions */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGHDB	0		/* eval HDB performance */
#define	CF_DEBUGFILE	1		/* debug file operations */
#define	CF_DEBUGLIST	1		/* debug list? */
#define	CF_SAFE		1		/* run safer */
#define	CF_PWDINIT	0		/* initialize PWD on open */
#define	CF_CHDIR	1		/* allow for changed directories */
#define	CF_DEVINO	1		/* check device-inode */


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This object was originally written.

	- 2004-05-25, David A­D­ Morano
        This subroutine was adopted for use as a general key-value file reader.

*/

/* Copyright © 1998,2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object processes an access table for use by daemon multiplexing
        server programs that want to control access to their sub-servers.

	Implementation note:

        We let stale keys stay around. They are not lost, just not freed when no
        longer needed. There is no memory leak as they are all freed when the
        object is deconstructed. Stale keys do sort of serve as a ready key
        cache for those cases when they may be need later on with future
        entries!


*******************************************************************************/


#define	KVSFILE_MASTER	0


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

#include	"kvsfile.h"


/* local defines */

#define	KVSFILE_MINCHECKTIME	5	/* file check interval (seconds) */
#define	KVSFILE_CHECKTIME	60	/* file check interval (seconds) */
#define	KVSFILE_CHANGETIME	3	/* wait change interval (seconds) */
#define	KVSFILE_DEFNETGROUP	"DEFAULT"

#define	KVSFILE_FILE		struct kvsfile_file
#define	KVSFILE_KEY		struct kvsfile_key
#define	KVSFILE_ENT		struct kvsfile_e

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif

#ifdef	KVSFILE_KEYLEN
#define	KEYBUFLEN		KVSFILE_KEYLEN
#else
#define	KEYBUFLEN		MAXHOSTNAMELEN
#endif

#if	CF_DEBUGS && CF_DEBUGHDB
#ifndef	HDBHASHTABCOUNTS
#define	HDBHASHTABCOUNTS	"hdbhashtabcounts"
#endif
#endif


/* external subroutines */

extern unsigned int	hashelf(const char *,int) ;

extern int	sncpy1(char *,int,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	getpwd(char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct kvsfile_file {
	const char	*fname ;
	time_t		mtime ;
	uino_t		ino ;
	dev_t		dev ;
	int		size ;
} ;

struct kvsfile_key {
	const char	*kname ;
	int		count ;
} ;

struct kvsfile_e {
	KVSFILE_KEY	*kep ;
	const char	*vname ;
	int		vlen ;
	int		fi ;		/* file index */
	int		ki ;		/* key index */
} ;


/* forward references */

static unsigned int	hashkeyval(KVSFILE_ENT *,int) ;

int		kvsfile_fileadd(KVSFILE *,const char *) ;

static int	kvsfile_filefins(KVSFILE *) ;
static int	kvsfile_keyfins(KVSFILE *) ;
static int	kvsfile_fh(KVSFILE *,dev_t,uino_t) ;
static int	kvsfile_fileparse(KVSFILE *,int) ;
static int	kvsfile_fileparser(KVSFILE *,int,bfile *) ;
static int	kvsfile_getkeyp(KVSFILE *,const char *,KVSFILE_KEY **) ;
static int	kvsfile_filedump(KVSFILE *,int) ;
static int	kvsfile_addentry(KVSFILE *,KVSFILE_ENT *) ;
static int	kvsfile_already(KVSFILE *,KVSFILE_ENT *) ;
static int	kvsfile_checkfiles(KVSFILE *,time_t) ;

#ifdef	COMMENT
static int	kvsfile_filedel(KVSFILE *,int) ;
#endif

static int	file_start(struct kvsfile_file *,const char *) ;
static int	file_finish(struct kvsfile_file *) ;

static int	key_start(KVSFILE_KEY *,const char *) ;
static int	key_increment(KVSFILE_KEY *) ;
static int	key_decrement(KVSFILE_KEY *) ;
static int	key_finish(KVSFILE_KEY *) ;

static int	entry_start(KVSFILE_ENT *,int,int,KVSFILE_KEY *,cchar *,int) ;
static int	entry_finish(KVSFILE_ENT *) ;

static int	cmpfname() ;
static int	cmpkey() ;
static int	cmpkeyval(KVSFILE_ENT *,KVSFILE_ENT *,int) ;


/* local variables */

/* all white space, pound ('#'), colon (':'), and comma (',') */
static const uchar	fterms[] = {
	0x00, 0x1F, 0x00, 0x00,
	0x09, 0x10, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int kvsfile_open(KVSFILE *op,int ndef,cchar *atfname)
{
	int		rs ;
	int		size ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("kvsfile_open: fname=%s\n",atfname) ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (ndef < KVSFILE_DEFENTS) ndef = KVSFILE_DEFENTS ;

	memset(op,0,sizeof(KVSFILE)) ;

/* this vector structure must remain fixed so that indices do not change */

	size = sizeof(KVSFILE_FILE) ;
	opts = (VECOBJ_OSTATIONARY | VECOBJ_OREUSE) ;
	if ((rs = vecobj_start(&op->files,size,ndef,opts)) >= 0) {
	    const int	n = (ndef / 10) ;
	    size = sizeof(KVSFILE_KEY) ;
	    opts = (VECOBJ_OSTATIONARY | VECOBJ_OREUSE) ;
	    if ((rs = vecobj_start(&op->keys,size,n,opts)) >= 0) {
	        uint	(*hk)(KVSFILE_ENT *,int) = hashkeyval ;
	        if ((rs = hdb_start(&op->keyvals,ndef,0,hk,cmpkeyval)) >= 0) {
	            if ((rs = hdb_start(&op->entries,ndef,0,NULL,NULL)) >= 0) {
	                op->magic = KVSFILE_MAGIC ;
	                op->ti_check = time(NULL) ;
	                if ((atfname != NULL) && (atfname[0] != '\0')) {
	                    rs = kvsfile_fileadd(op,atfname) ;
	                    if (rs < 0) {
	                        op->magic = 0 ;
	                    }
	                } /* end if (adding first file) */
	                if (rs < 0)
	                    hdb_finish(&op->entries) ;
	            } /* end if (entries) */
	            if (rs < 0)
	                hdb_finish(&op->keyvals) ;
	        } /* end if (keyvals) */
	        if (rs < 0)
	            vecobj_finish(&op->keys) ;
	    } /* end if (keys) */
	    if (rs < 0)
	        vecobj_finish(&op->files) ;
	} /* end if (files) */

#if	CF_DEBUGS
	debugprintf("kvsfile_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kvsfile_open) */


int kvsfile_close(KVSFILE *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

	rs1 = kvsfile_filedump(op,-1) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = kvsfile_keyfins(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = kvsfile_filefins(op) ;
	if (rs >= 0) rs = rs1 ;

/* free up the rest of the main object data */

	rs1 = hdb_finish(&op->entries) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = hdb_finish(&op->keyvals) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->keys) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (kvsfile_close) */


/* add a file to the list of files */
int kvsfile_fileadd(KVSFILE *op,cchar atfname[])
{
	int		rs = SR_OK ;
	int		fi = 0 ;
	const char	*np ;
	char		abuf[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;
	if (atfname == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("kvsfile_fileadd: fname=%s\n",atfname) ;
#endif

	if (atfname[0] == '\0') return SR_INVALID ;

	np = atfname ;
	if (atfname[0] != '/') {
	    const int	plen = MAXPATHLEN ;
	    char	pbuf[MAXPATHLEN+1] ;

	    if ((rs = getpwd(pbuf,plen)) >= 0) {
	        rs = mkpath2(abuf,pbuf,atfname) ;
	        np = abuf ;
	    }

	} /* end if (added PWD) */

#if	CF_DEBUGS
	debugprintf("kvsfile_fileadd: np=%s\n",np) ;
#endif

	if (rs >= 0) {
	    KVSFILE_FILE	fe ;
	    if ((rs = file_start(&fe,np)) >= 0) {
	        VECOBJ		*flp = &op->files ;
	        const int	nrs = SR_NOTFOUND ;
	        if ((rs = vecobj_search(flp,&fe,cmpfname,NULL)) == nrs) {
	            if ((rs = vecobj_add(&op->files,&fe)) >= 0) {
	                fi = rs ;
	                rs = kvsfile_fileparse(op,fi) ;
	                if (rs < 0)
	                    vecobj_del(&op->files,fi) ;
	            } /* end if (vecobj_add) */
	            if (rs < 0)
	                file_finish(&fe) ;
	        } else {
	            fi = rs ;
	            file_finish(&fe) ;
	        }
	    } /* end if (file-start) */
	} /* end if (ok) */

#if	CF_DEBUGS && CF_DEBUGLIST
	if (rs >= 0) {
	    KVSFILE_ENT	*ep ;
	    KVSFILE_KEY	*kep ;
	    HDB_CUR	cur ;
	    HDB_DATUM	key, val ;
	    int		rs1 ;
	    int		i ;
	    hdb_curbegin(&op->entries,&cur) ;
	    while ((rs1 = hdb_enum(&op->entries,&cur,&key,&val)) >= 0) {
	        ep = (KVSFILE_ENT *) val.buf ;
	        kep = ep->kep ;
	        debugprintf("kvsfile_fileadd: ENUM k=%s v=%s\n",
	            kep->kname,ep->vname) ;
	    } /* end while */
	    hdb_curend(&op->entries,&cur) ;
/* fetch by keyname */
	    for (i = 0 ; (rs1 = vecobj_get(&op->keys,i,&kep)) >= 0 ; i += 1) {
	        if (kep == NULL) continue ;
	        debugprintf("kvsfile_fileadd: GET kep(%p)\n", kep) ;
	        debugprintf("kvsfile_fileadd: GET kc=%d\n", kep->count) ;
	        debugprintf("kvsfile_fileadd: GET k=%s\n", kep->kname) ;
	        hdb_curbegin(&op->entries,&cur) ;
	        while (rs1 >= 0) {
	            key.buf = kep->kname ;
	            key.len = strlen(kep->kname) ;
	            rs1 = hdb_fetch(&op->entries,key,&cur,&val) ;
	            if (rs1 < 0) break ;
	            ep = (KVSFILE_ENT *) val.buf ;
	            debugprintf("kvsfile_fileadd: FETCH v=%s\n",
	                ep->vname) ;
	        } /* end while */
	        hdb_curend(&op->entries,&cur) ;
	    } /* end for */
#if	CF_DEBUGHDB
	    hdb_debugdump(&op->entries) ;
#endif
	    rs1 = hdb_audit(&op->entries) ;
	    debugprintf("kvsfile_fileadd: hdb_audit() rs=%d\n",rs1) ;
	} /* end if */
#endif /* CF_DEBUGHDB */

#if	CF_DEBUGS
	debugprintf("kvsfile_fileadd: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? fi : rs ;
}
/* end subroutine (kvsfile_fileadd) */


/* cursor manipulations */
int kvsfile_curbegin(KVSFILE *op,KVSFILE_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	rs = hdb_curbegin(&op->entries,&curp->ec) ;

	return rs ;
}
/* end subroutine (kvsfile_curbegin) */


int kvsfile_curend(KVSFILE *op,KVSFILE_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	rs = hdb_curend(&op->entries,&curp->ec) ;

	return rs ;
}
/* end subroutine (kvsfile_curend) */


/* enumerate a key */
int kvsfile_enumkey(KVSFILE *op,KVSFILE_CUR *curp,char *keybuf,int keylen)
{
	KVSFILE_KEY	*kep ;
	int		rs = SR_OK ;
	int		oi ;
	int		kl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (keybuf == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

	oi = (curp->i >= 0) ? (curp->i + 1) : 0 ;

/* CONSTCOND */

	while (TRUE) {
	    rs = vecobj_get(&op->keys,oi,&kep) ;
	    if (rs < 0) break ;
	    if (kep != NULL) break ;
	    oi += 1 ;
	} /* end while */

	if (rs >= 0) {
	    rs = sncpy1(keybuf,keylen,kep->kname) ;
	    kl = rs ;
	    curp->i = oi ;
	}

#if	CF_DEBUGS
	debugprintf("kvsfile_enumkey: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (kvsfile_enumkey) */


/* enumerate key-value pairs */
int kvsfile_enum(op,curp,keybuf,keylen,valbuf,vallen)
KVSFILE		*op ;
KVSFILE_CUR	*curp ;
char		keybuf[] ;
int		keylen ;
char		valbuf[] ;
int		vallen ;
{
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs ;
	int		vl ;
	int		kl = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (keybuf == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

	cur = curp->ec ;
	if ((rs = hdb_enum(&op->entries,&cur,&key,&val)) >= 0) {
	    KVSFILE_ENT	*ep ;
	    cchar	*kp, *vp ;

	    kp = (const char *) key.buf ;
	    kl = key.len ;
	    rs = snwcpy(keybuf,keylen,kp,kl) ;

	    if ((rs >= 0) && (valbuf != NULL)) {

	        ep = (KVSFILE_ENT *) val.buf ;

	        vp = ep->vname ;
	        vl = ep->vlen ;
	        rs = snwcpy(valbuf,vallen,vp,vl) ;

	    } /* end if (wanted the value also) */

	    if (rs >= 0) {
	        curp->ec = cur ;
	    }

	} /* end if (had an entry) */

#if	CF_DEBUGS
	debugprintf("kvsfile_enum: ret rs=%d kl=%u\n",rs,kl) ;
#endif

	return (rs >= 0) ? kl : rs ;
}
/* end subroutine (kvsfile_enum) */


int kvsfile_fetch(op,keybuf,curp,valbuf,vallen)
KVSFILE		*op ;
const char	keybuf[] ;
KVSFILE_CUR	*curp ;
char		valbuf[] ;
int		vallen ;
{
	KVSFILE_ENT	*ep ;
	HDB_DATUM	key, val ;
	HDB_CUR		cur ;
	int		rs ;
	int		kl ;
	int		vl = 0 ;
	const char	*kp, *vp ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;
	if (keybuf == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

	kp = (cchar *) keybuf ;
	if (keybuf[0] == '\0')
	    kp = "default" ;

	kl = strlen(kp) ;

#if	CF_DEBUGS
	debugprintf("kvsfile_fetch: key=%s\n",kp) ;
#endif

#if	CF_DEBUGS && CF_DEBUGHDB
	{
	    int	rs1 ;
	    rs1 = hdb_audit(&op->entries) ;
	    debugprintf("kvsfile_fetch: hdb_audit() rs=%d\n",rs1) ;
	    hdb_debugdump(&op->entries) ;
	}
#endif

	key.buf = kp ;
	key.len = kl ;
	cur = curp->ec ;
	if ((rs = hdb_fetch(&op->entries,key,&cur,&val)) >= 0) {

	    ep = (KVSFILE_ENT *) val.buf ;

	    vp = ep->vname ;
	    vl = ep->vlen ;

	    if (valbuf != NULL) {
	        rs = snwcpy(valbuf,vallen,vp,vl) ;
	    }

	    if (rs >= 0) {
	        curp->ec = cur ;
	    }

	} /* end if (had an entry) */

#if	CF_DEBUGS
	debugprintf("kvsfile_fetch: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (kvsfile_fetch) */


/* check if the access tables files have changed */
int kvsfile_check(KVSFILE *op,time_t daytime)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != KVSFILE_MAGIC) return SR_NOTOPEN ;

	if (daytime == 0) daytime = time(NULL) ;

/* should we even check? */

	if ((daytime - op->ti_check) > KVSFILE_CHECKTIME) {
	    f = TRUE ;
	    rs = kvsfile_checkfiles(op,daytime) ;
	}

#if	CF_DEBUGS
	debugprintf("kvsfile_check: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (kvsfile_check) */


/* private subroutines */


static int kvsfile_keyfins(KVSFILE *op)
{
	KVSFILE_KEY	*kep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecobj_get(&op->keys,i,&kep) >= 0 ; i += 1) {
	    if (kep != NULL) {
	        rs1 = key_finish(kep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	return rs ;
}
/* end subroutine (kvsfile_keyfins) */


/* free up all of the files in this KVSFILE list */
static int kvsfile_filefins(KVSFILE *op)
{
	KVSFILE_FILE	*fep ;
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
/* end subroutine (kvsfile_filefins) */


/* check if files have changed */
static int kvsfile_checkfiles(KVSFILE *op,time_t daytime)
{
	struct ustat	sb ;
	KVSFILE_FILE	*fep ;
	int		rs = SR_OK ;
	int		i ;
	int		c_changed = 0 ;

/* check the files */

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep != NULL) {

	    if ((u_stat(fep->fname,&sb) >= 0) &&
	        (sb.st_mtime > fep->mtime)) {

#if	CF_DEBUGS
	        debugprintf("kvsfile_checkfiles: file=%d changed\n",i) ;
	        debugprintf("kvsfile_checkfiles: freeing file entries\n") ;
#endif

	        kvsfile_filedump(op,i) ;

#if	CF_DEBUGS
	        debugprintf("kvsfile_checkfiles: parsing the file again\n") ;
#endif

	        rs = kvsfile_fileparse(op,i) ;

	        if (rs >= 0)
	            c_changed += 1 ;

#if	CF_DEBUGS
	        debugprintf("kvsfile_checkfiles: kvsfile_fileparse rs=%d\n",
	            rs) ;
#endif

	    } /* end if */

	    } /* end if */
	    if (rs < 0) break ;
	} /* end for */

	op->ti_check = daytime ;

#if	CF_DEBUGS
	debugprintf("kvsfile_checkfiles: ret rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (kvsfile_checkfiles) */


static int kvsfile_fh(KVSFILE *op,dev_t dev,uino_t ino)
{
	KVSFILE_FILE	*fep ;
	int		rs ;
	int		i ;

	for (i = 0 ; (rs = vecobj_get(&op->files,i,&fep)) >= 0 ; i += 1) {
	    if (fep != NULL) {
	        if ((fep->dev == dev) && (fep->ino == ino)) break ;
	    }
	} /* end for */

#if	CF_DEBUGS
	debugprintf("kvsfile_fh: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kvsfile_fh) */


/* parse a file */
static int kvsfile_fileparse(KVSFILE *op,int fi)
{
	KVSFILE_FILE	*fep ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("kvsfile_fileparse: ent fi=%u\n",fi) ;
#endif

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        bfile	kvsfile, *lfp = &kvsfile ;
	        if ((rs = bopen(lfp,fep->fname,"r",0664)) >= 0) {
	            BFILE_STAT	sb ;
	            if ((rs = bstat(lfp,&sb)) >= 0) {
	                if (! S_ISDIR(sb.st_mode)) {
	                    if (sb.st_mtime > fep->mtime) {
	                        const int	nrs = SR_NOTFOUND ;
	                        const dev_t	dev = sb.st_dev ;
	                        const uino_t	ino = sb.st_ino ;
	                        if ((rs = kvsfile_fh(op,dev,ino)) == nrs) {
	                            fep->dev = dev ;
	                            fep->ino = ino ;
	                            fep->mtime = sb.st_mtime ;
	                            fep->size = sb.st_size ;
	                            rs = kvsfile_fileparser(op,fi,lfp) ;
	                            c = rs ;
	                            if (rs < 0)
	                                kvsfile_filedump(op,fi) ;
	                        } /* end if (adding file) */
	                    } /* end if (not previously added) */
	                } else
	                    rs = SR_ISDIR ;
	            } /* end if (bstat) */
	            bclose(lfp) ;
	        } /* end if (file) */
	    } else
	        rs = SR_NOTFOUND ;
	} /* end if (vec-get) */

#if	CF_DEBUGS
	debugprintf("kvsfile_fileparse: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (kvsfile_fileparse) */


static int kvsfile_fileparser(KVSFILE *op,int fi,bfile *lfp)
{
	KVSFILE_KEY	*kep = NULL ;
	KVSFILE_ENT	ve ;
	FIELD		fsb ;
	const int	llen = LINEBUFLEN ;
	int		rs = SR_OK ;
	int		c_field ;
	int		ki, len ;
	int		fl, cl ;
	int		c_added = 0 ;
	int		c = 0 ;
	int		f_eol ;
	const char	*fp ;
	const char	*cp ;
	char		lbuf[LINEBUFLEN + 1] ;
	char		keybuf[KEYBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("kvsfile_fileparser: ent fi=%u\n",fi) ;
#endif

	keybuf[0] = '\0' ;

/* loop, reading all records and figuring things out */

	while ((rs = breadline(lfp,lbuf,llen)) > 0) {
	    len = rs ;

	    f_eol = (lbuf[len - 1] == '\n') ;
	    if (f_eol) len -= 1 ;
	    lbuf[len] = '\0' ;

#if	CF_DEBUGS && CF_DEBUGFILE
	    debugprintf("kvsfile_fileparse: line>%t<\n",
	        lbuf,MIN(len,40)) ;
#endif

	    cp = lbuf ;
	    cl = len ;
	    while (CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        cl -= 1 ;
	    }

	    if ((*cp == '\0') || (*cp == '#'))
	        continue ;

	    c_field = 0 ;
	    if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	        while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {

	            if ((c_field++ == 0) && (fsb.term == ':')) {

	                c = 0 ;
	                strwcpy(keybuf,fp,MIN(fl,KEYBUFLEN)) ;

#if	CF_DEBUGS && CF_DEBUGFILE
	                debugprintf("kvsfile_fileparse: key=%s\n",keybuf) ;
#endif

	            } else if ((fl > 0) && (keybuf[0] != '\0')) {

#if	CF_DEBUGS && CF_DEBUGFILE
	                debugprintf("kvsfile_fileparse: value w/ key\n") ;
#endif

/* enter key into string table (if first time) */

#if	CF_DEBUGS && CF_DEBUGFILE
	                debugprintf("kvsfile_fileparse: c=%u\n",c) ;
#endif

	                if (c++ == 0) {
	                    rs = kvsfile_getkeyp(op,keybuf,&kep) ;
	                    ki = rs ;
	                } /* end if (entering key) */

#if	CF_DEBUGS && CF_DEBUGFILE
	                debugprintf("kvsfile_fileparse: mid kep(%p)\n", 
	                    kep) ;
	                if (kep != NULL)
	                    debugprintf("kvsfile_fileparse: mid key=%s\n", 
	                        kep->kname) ;
#endif

	                if ((rs >= 0) && (kep != NULL)) {
	                    int	f = TRUE ;

#if	CF_DEBUGS && CF_DEBUGFILE
	                    debugprintf("kvsfile_fileparse: key=%s val=%t\n",
	                        kep->kname,fp,fl) ;
#endif

	                    if ((rs = entry_start(&ve,fi,ki,kep,fp,fl)) >= 0) {
	                        const int	nrs = SR_NOTFOUND ;

	                        if ((rs = kvsfile_already(op,&ve)) == nrs) {

#if	CF_DEBUGS && CF_DEBUGFILE
	                            debugprintf("kvsfile_fileparse: "
	                                "new value "
	                                "for this key\n") ;
#endif

	                            if ((rs = kvsfile_addentry(op,&ve)) >= 0) {
	                                f = FALSE ;
	                                c_added += 1 ;
	                            }

	                        } /* end if (new entry) */

	                        if (f) entry_finish(&ve) ;
	                    } /* end if (entry initialized) */

	                } /* end if */

	            } /* end if (handling record) */

	            if (fsb.term == '#') break ;
	            if (rs < 0) break ;
	        } /* end while (fields) */

	        field_finish(&fsb) ;
	    } /* end if (field) */

	    if (rs < 0) break ;
	} /* end while (reading lines) */

#if	CF_DEBUGS
	debugprintf("kvsfile_fileparser: ret rs=%d c=%u\n",rs,c_added) ;
#endif

	return (rs >= 0) ? c_added : rs ;
}
/* end subroutine (kvsfile_fileparser) */


/* get a pointer to the current key (make it as necessary) */
static int kvsfile_getkeyp(KVSFILE *op,cchar *keybuf,KVSFILE_KEY **kpp)
{
	KVSFILE_KEY	ke, *kep = NULL ;
	int		rs ;
	int		ki = 0 ;

	if ((rs = key_start(&ke,keybuf)) >= 0) {
	    const int	nrs = SR_NOTFOUND ;
	    int		f = TRUE ;
	    if ((rs = vecobj_search(&op->keys,&ke,cmpkey,&kep)) == nrs) {
	        kep = NULL ;
	        if ((rs = vecobj_add(&op->keys,&ke)) >= 0) {
	            ki = rs ;
	            f = FALSE ;
	            rs = vecobj_get(&op->keys,ki,&kep) ;
	        } /* end if */
	    } else {
	        ki = rs ;
	    }
	    if (f) {
	        key_finish(&ke) ;
	    }
	} /* end if (key-start) */

	if (kpp != NULL) {
	    *kpp = (rs >= 0) ? kep : NULL ;
	}

	return (rs >= 0) ? ki : rs ;
}
/* end subroutine (kvsfile_getlkeyp) */


/* do we have this entry already */
static int kvsfile_already(KVSFILE *op,KVSFILE_ENT *nep)
{
	HDB_DATUM	key, val ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	{
	    KVSFILE_KEY	*kep = nep->kep ;
	    debugprintf("kvsfile_already: ent key=%s val=%s\n",
	        kep->kname, nep->vname) ;
	}
#endif

	key.buf = nep ;
	key.len = sizeof(KVSFILE_ENT) ;

#ifdef	COMMENT
	{
	    KVSFILE_ENT	*ep ;
	    HDB_CUR	cur ;
	    hdb_curbegin(&op->entries,&cur) ;
	    while (hdb_fetch(&op->entries,key,&cur,&value) >= 0) {
	        ep = (KVSFILE_ENT *) val.buf ;
#if	CF_DEBUGS
	        {
	            KVSFILE_KEY	*kep = ep->kep ;
	            debugprintf("kvsfile_already: key=%s val=%s\n",
	                kep->kname, ep->vname) ;
	        }
#endif /* CF_DEBUGS */
	        f = (strcmp(nep->vname,ep->vname) == 0) ;
	        if (f) break ;
	    } /* end while */
	    hdb_curend(&op->entries,&cur) ;
	    rs = (f) ? SR_OK : SR_NOTFOUND ;
	}
#else /* COMMENT */

	rs = hdb_fetch(&op->keyvals,key,NULL,&val) ;

#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("kvsfile_already: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kvsfile_already) */


/* add an entry to entry list */
static int kvsfile_addentry(KVSFILE *op,KVSFILE_ENT *nep)
{
	KVSFILE_KEY	*kep ;
	KVSFILE_ENT	*ep ;
	const int	size = sizeof(KVSFILE_ENT) ;
	int		rs ;

#if	CF_DEBUGS
	kep = nep->kep ;
	debugprintf("kvsfile_addentry: key=%s val=%s\n",
	    kep->kname,nep->vname) ;
#endif

	if ((rs = uc_malloc(size,&ep)) >= 0) {
	    HDB_DATUM	key, val ;
	    *ep = *nep ;
	    kep = ep->kep ;
	    key.buf = ep ;
	    key.len = sizeof(KVSFILE_ENT) ;
	    val.buf = ep ;
	    val.len = sizeof(KVSFILE_ENT) ;
	    if ((rs = hdb_store(&op->keyvals,key,val)) >= 0) {
	        key.buf = kep->kname ;
	        key.len = strlen(kep->kname) ;
	        val.buf = ep ;
	        val.len = sizeof(KVSFILE_ENT) ;
	        rs = hdb_store(&op->entries,key,val) ;
	        if (rs < 0) {
	            HDB_CUR	cur ;
	            hdb_curbegin(&op->keyvals,&cur) ;
	            {
	                key.buf = ep ;
	                key.len = sizeof(KVSFILE_ENT) ;
	                if (hdb_fetch(&op->keyvals,key,&cur,&val) >= 0) {
	                    hdb_delcur(&op->keyvals,&cur,0) ;
			}
	            }
	            hdb_curend(&op->keyvals,&cur) ;
	        } /* end if (bad) */
	    } /* end if (keyvals-store) */
	    if (rs < 0)
	        uc_free(ep) ;
	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("kvsfile_addentry: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kvsfile_addentry) */


/* free up all of the entries in this KVSFILE list associated w/ a file */
static int kvsfile_filedump(KVSFILE *op,int fi)
{
	KVSFILE_ENT	*ep ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;
	int		rs ;
	int		rs1 ;

#if	CF_DEBUGS
	debugprintf("kvsfile_filedump: ent fi=%d\n",fi) ;
#endif

/* delete all keyvals w/ this file */

	if ((rs = hdb_curbegin(&op->keyvals,&cur)) >= 0) {

	    while (hdb_enum(&op->keyvals,&cur,&key,&val) >= 0) {
	        ep = (KVSFILE_ENT *) val.buf ;

	        if ((ep->fi == fi) || (fi < 0)) {

#if	CF_DEBUGS
	            debugprintf("kvsfile_filedump: got one\n") ;
#endif

	            hdb_delcur(&op->keyvals,&cur,0) ;

	        } /* end if (found matching entry) */

	    } /* end while (looping through entries) */

	    rs1 = hdb_curend(&op->keyvals,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

/* delete all entries w/ this file */

	if (rs >= 0) {
	    if ((rs = hdb_curbegin(&op->entries,&cur)) >= 0) {

	        while (hdb_enum(&op->entries,&cur,&key,&val) >= 0) {
	            ep = (KVSFILE_ENT *) val.buf ;

	            if ((ep->fi == fi) || (fi < 0)) {

#if	CF_DEBUGS
	                debugprintf("kvsfile_filedump: got one\n") ;
#endif

	                rs1 = hdb_delcur(&op->entries,&cur,0) ;
	                if (rs >= 0) rs = rs1 ;

	                rs1 = entry_finish(ep) ;
	                if (rs >= 0) rs = rs1 ;

	                rs1 = uc_free(ep) ;
	                if (rs >= 0) rs = rs1 ;

	            } /* end if (found matching entry) */

	        } /* end while (looping through entries) */

	        rs1 = hdb_curend(&op->entries,&cur) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (cursor) */
	} /* end if (ok) */

/* done */

#if	CF_DEBUGS
	debugprintf("kvsfile_filedump: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (kvsfile_filedump) */


#ifdef	COMMENT
/* delete a file from the list of files (that is all) */
static int kvsfile_filedel(KVSFILE *op,int fi)
{
	KVSFILE_FILE	*fep ;
	int		rs ;

	if ((rs = vecobj_get(&op->files,fi,&fep)) >= 0) {
	    if (fep != NULL) {
	        file_finish(fep) ;
	    }
	    rs = vecobj_del(&op->files,fi) ;
	}

	return rs ;
}
/* end subroutine (kvsfile_filedel) */
#endif /* COMMENT */


static int file_start(KVSFILE_FILE *fep,cchar fname[])
{
	int		rs ;
	const char	*cp ;

	if (fname == NULL) return SR_FAULT ;

	memset(fep,0,sizeof(KVSFILE_FILE)) ;

	rs = uc_mallocstrw(fname,-1,&cp) ;
	if (rs >= 0) fep->fname = cp ;

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(KVSFILE_FILE *fep)
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


static int key_start(KVSFILE_KEY *kep,cchar kname[])
{
	int		rs = SR_OK ;

	memset(kep,0,sizeof(KVSFILE_KEY)) ;

	if (kname != NULL) {
	    const int	klen = KEYBUFLEN ;
	    const char	*cp ;
	    rs = uc_mallocstrw(kname,klen,&cp) ;
	    if (rs >= 0) kep->kname = cp ;
	}

	return rs ;
}
/* end subroutine (key_start) */


static int key_finish(KVSFILE_KEY *kep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (kep->kname != NULL) {
	    rs1 = uc_free(kep->kname) ;
	    if (rs >= 0) rs = rs1 ;
	    kep->kname = NULL ;
	}

	kep->count = 0 ;
	return rs ;
}
/* end subroutine (key_finish) */


static int key_increment(KVSFILE_KEY *kep)
{

	if (kep == NULL) return SR_FAULT ;

	kep->count += 1 ;
	return SR_OK ;
}
/* end subroutine (key_increment) */


static int key_decrement(KVSFILE_KEY *kep)
{

	if (kep == NULL) return SR_FAULT ;

	if (kep->count > 0)
	    kep->count -= 1 ;

	return kep->count ;
}
/* end subroutine (key_decrement) */


static int entry_start(ep,fi,ki,kep,vp,vl)
KVSFILE_ENT	*ep ;
int		fi, ki ;
KVSFILE_KEY	*kep ;
const char	*vp ;
int		vl ;
{
	int		rs ;
	const char	*cp ;

	memset(ep,0,sizeof(KVSFILE_ENT)) ;
	ep->fi = fi ;
	ep->ki = ki ;
	ep->kep = kep ;
	ep->vlen = vl ;
	if ((rs = uc_mallocstrw(vp,vl,&cp)) >= 0) {
	    ep->vname = cp ;
	    key_increment(kep) ;
	}

	return rs ;
}
/* end subroutine (entry_start) */


/* free up an entry */
static int entry_finish(ep)
KVSFILE_ENT	*ep ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		nk ;
	int		rc ;

	if (ep->vname == NULL)
	    return SR_OK ;

	nk = key_decrement(ep->kep) ;

	if (ep->vname != NULL) {
	    rs1 = uc_free(ep->vname) ;
	    if (rs >= 0) rs = rs1 ;
	    ep->vname = NULL ;
	}

	rc = (nk == 0) ? ep->ki : INT_MAX ;
	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (entry_finish) */


static int cmpfname(KVSFILE_FILE **e1pp,KVSFILE_FILE **e2pp)
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


static int cmpkey(KVSFILE_KEY **e1pp,KVSFILE_KEY **e2pp)
{
	int		rc = 0 ;
	if ((*e1pp != NULL) || (*e2pp != NULL)) {
	    if (*e1pp != NULL) {
	        if (*e2pp != NULL) {
	            rc = strcmp((*e1pp)->kname,(*e2pp)->kname) ;
	        } else
	            rc = -1 ;
	    } else
	        rc = 1 ;
	}
	return rc ;
}
/* end subroutine (cmpkey) */


/* ARGSUSED */
static unsigned int hashkeyval(ep,len)
KVSFILE_ENT	*ep ;
int		len ;
{
	KVSFILE_KEY	*kep = ep->kep ;
	unsigned int	hv = 0 ;

	hv += hashelf(kep->kname,-1) ;

	hv += hashelf(ep->vname,-1) ;

	return hv ;
}
/* end subroutine (hashkeyval) */


/* ARGSUSED */
static int cmpkeyval(KVSFILE_ENT *e1p,KVSFILE_ENT *e2p,int len)
{
	int		rc ;

	if ((rc = strcmp(e1p->kep->kname,e2p->kep->kname)) == 0) {
	    rc = strcmp(e1p->vname,e2p->vname) ;
	}

	return rc ;
}
/* end subroutine (cmpkeyval) */


