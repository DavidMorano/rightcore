/* nodedb */

/* magement for the NODE-DB file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFILE	0		/* for debugging file reading */


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
#include	<storeitem.h>
#include	<char.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"nodedb.h"


/* local defines */

#define	NODEDB_MINCHECKTIME	5	/* file check interval (seconds) */
#define	NODEDB_CHECKTIME	60	/* file check interval (seconds) */
#define	NODEDB_CHANGETIME	3	/* wait change interval (seconds) */
#define	NODEDB_DEFNETGROUP	"DEFAULT"

#define	NODEDB_FILE		struct nodedb_file
#define	NODEDB_KEYNAME		struct nodedb_keyname
#define	NODEDB_IE		struct nodedb_ie

#define	NODEDB_KA		sizeof(char *(*)[2])
#define	NODEDB_BO(v)		\
	((NODEDB_KA - ((v) % NODEDB_KA)) % NODEDB_KA)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN		MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN		2048
#endif
#endif

#undef	ARGSBUFLEN
#define	ARGSBUFLEN		(3 * MAXHOSTNAMELEN)

#undef	DEFCHUNKSIZE
#define	DEFCHUNKSIZE		512

#define	KEYALIGNMENT		sizeof(char *(*)[2])


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	getpwds(struct ustat *,char *,int) ;
extern int	getpwd(char *,int) ;
extern int	field_srvarg(FIELD *,const uchar *,char *,int) ;

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
	struct lineinfo_field	f[3] ;
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
static int	nodedb_fileparseline(NODEDB *,int,const char *,int) ;
static int	nodedb_filedump(NODEDB *,int) ;
static int	nodedb_filedel(NODEDB *,int) ;
static int	nodedb_addentry(NODEDB *,int,struct svcentry *) ;
static int	nodedb_checkfiles(NODEDB *,time_t) ;

static int	file_start(struct nodedb_file *,const char *) ;
static int	file_finish(struct nodedb_file *) ;

static int	ientry_start(struct nodedb_ie *,int,struct svcentry *) ;
static int	ientry_finish(struct nodedb_ie *) ;

static int	svcentry_start(struct svcentry *,struct lineinfo *) ;
static int	svcentry_addkey(struct svcentry *,const char *,int,
			const char *,int) ;
static int	svcentry_finish(struct svcentry *) ;

static int	entry_load(NODEDB_ENT *,char *,int,struct nodedb_ie *) ;

static int	cmpfname() ;

#ifdef	COMMENT
static int	cmpkey() ;
#endif

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


int nodedb_open(op,fname)
NODEDB		*op ;
const char	fname[] ;
{
	int	rs = SR_OK ;
	int	defentries = 0 ;
	int	size ;


#if	CF_DEBUGS
	debugprintf("nodedb_open: fname=%s\n",fname) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if (defentries < NODEDB_DEFENTS)
	    defentries = NODEDB_DEFENTS ;

	memset(op,0,sizeof(NODEDB)) ;

	size = sizeof(struct nodedb_file) ;
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
int nodedb_close(op)
NODEDB		*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC)
	    return SR_NOTOPEN ;

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
int nodedb_fileadd(op,fname)
NODEDB		*op ;
const char	fname[] ;
{
	NODEDB_FILE	fe ;
	int		rs = SR_OK ;
	int		fi ;
	int		f_file = FALSE ;
	const char	*np ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("nodedb_fileadd: fname=%s\n",fname) ;
#endif

	if (fname == NULL)
	    return SR_FAULT ;

	np = fname ;
	if (fname[0] != '/') {

#if	CF_DEBUGS
	    debugprintf("nodedb_fileadd: relative pathname\n") ;
#endif

	    rs = nodedb_pwd(op) ;
	    if (rs >= 0) {
	        np = tmpfname ;
	        rs = mkpath2(tmpfname,op->pwd,fname) ;
	    }

#if	CF_DEBUGS
	    debugprintf("nodedb_fileadd: tmpfname=%s\n",tmpfname) ;
#endif

	} /* end if (added PWD) */

#if	CF_DEBUGS
	debugprintf("nodedb_fileadd: np=%s\n",np) ;
#endif

	if (rs >= 0) {
	    rs = file_start(&fe,np) ;
	    f_file = (rs >= 0) ;
	}

	if (rs < 0)
	    goto bad1 ;

	rs = vecobj_search(&op->files,&fe,cmpfname,NULL) ;

	if ((rs >= 0) || (rs != SR_NOTFOUND))
	    goto bad2 ;

/* initialize the storage for this file */

	rs = vecobj_add(&op->files,&fe) ;
	fi = rs ;
	if (rs < 0) goto bad3 ;
	f_file = FALSE ;

	rs = nodedb_fileparse(op,fi) ;
	if (rs < 0)
	    goto bad4 ;

ret0:

#if	CF_DEBUGS
	debugprintf("nodedb_fileadd: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* handle bad things */
bad4:
	nodedb_filedel(op,fi) ;

bad3:
bad2:
	if (f_file) {
	    f_file = FALSE ;
	    file_finish(&fe) ;
	}

bad1:
	goto ret0 ;
}
/* end subroutine (nodedb_fileadd) */


/* cursor manipulations */
int nodedb_curbegin(op,curp)
NODEDB		*op ;
NODEDB_CUR	*curp ;
{
	int	rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	if ((rs = hdb_curbegin(&op->entries,&curp->ec)) >= 0)
	    op->cursors += 1 ;

	return rs ;
}
/* end subroutine (nodedb_curbegin) */


int nodedb_curend(op,curp)
NODEDB		*op ;
NODEDB_CUR	*curp ;
{
	int	rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

	curp->i = -1 ;
	if ((rs = hdb_curend(&op->entries,&curp->ec)) >= 0)
	    op->cursors -= 1 ;

	return rs ;
}
/* end subroutine (nodedb_curend) */


/* enumerate the entries */
int nodedb_enum(op,curp,ep,ebuf,elen)
NODEDB		*op ;
NODEDB_CUR	*curp ;
NODEDB_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	struct nodedb_ie	*iep ;

	HDB_DATUM	key, val ;
	HDB_CUR		cur ;

	int	rs ;
	int	svclen = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	if ((ep == NULL) || (ebuf == NULL))
	    return SR_FAULT ;

	cur = curp->ec ;
	if ((rs = hdb_enum(&op->entries,&cur,&key,&val)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("nodedb_enum: elen=%u\n",elen) ;
#endif

	    iep = (struct nodedb_ie *) val.buf ;

#if	CF_DEBUGS
	    debugprintf("nodedb_enum: ie size=%u nkeys=%u svc=%s\n",
	        iep->size,iep->nkeys,iep->svc) ;
#endif

	    if ((ep != NULL) && (ebuf != NULL)) {
	        rs = entry_load(ep,ebuf,elen,iep) ;
	        svclen = rs ;
	    } else
	        svclen = strlen(iep->svc) ;

	    if (rs >= 0)
	        curp->ec = cur ;

	} /* end if (had an entry) */

#if	CF_DEBUGS
	debugprintf("nodedb_enum: svc=%s \n",ep->svc) ;
	debugprintf("nodedb_enum: ret rs=%d svclen=%u\n",rs,svclen) ;
#endif

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (nodedb_enum) */


int nodedb_fetch(op,svcbuf,curp,ep,ebuf,elen)
NODEDB		*op ;
char		svcbuf[] ;
NODEDB_CUR	*curp ;
NODEDB_ENT	*ep ;
char		ebuf[] ;
int		elen ;
{
	struct nodedb_ie	*iep ;

	NODEDB_CUR	dbcur ;

	HDB_DATUM	key, val ;
	HDB_CUR		hcur ;

	int	rs ;
	int	svclen = 0 ;


	if (op == NULL) return SR_FAULT ;
	if (svcbuf == NULL) return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC) return SR_NOTOPEN ;

	if (curp == NULL) {
	    curp = &dbcur ;
	    nodedb_curbegin(op,&dbcur) ;
	}

	key.buf = svcbuf ;
	key.len = strlen(svcbuf) ;

	hcur = curp->ec ;
	if ((rs = hdb_fetch(&op->entries,key,&hcur,&val)) >= 0) {
#if	CF_DEBUGS
	    debugprintf("nodedb_fetch: elen=%u\n",elen) ;
#endif

	    iep = (struct nodedb_ie *) val.buf ;

#if	CF_DEBUGS
	    debugprintf("nodedb_fetch: ie size=%u nkeys=%u svc=%s\n",
	        iep->size,iep->nkeys,iep->svc) ;
#endif

	    if ((ep != NULL) && (ebuf != NULL)) {
	        rs = entry_load(ep,ebuf,elen,iep) ;
	        svclen = rs ;
	    } else
	        svclen = strlen(iep->svc) ;

	    if (rs >= 0)
	        curp->ec = hcur ;

	} /* end if (had an entry) */

	if (curp == &dbcur)
	    nodedb_curend(op,&dbcur) ;

#if	CF_DEBUGS
	debugprintf("nodedb_fetch: ret rs=%d svclen=%u\n",rs,svclen) ;
#endif

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (nodedb_fetch) */


/* check if the underlying file has changed */
int nodedb_check(op)
NODEDB		*op ;
{
	time_t	daytime ;

	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != NODEDB_MAGIC)
	    return SR_NOTOPEN ;

/* should we even check? */

	if (op->cursors == 0) {

	    daytime = time(NULL) ;

	    if ((daytime - op->checktime) > NODEDB_CHECKTIME)
	        rs = nodedb_checkfiles(op,daytime) ;

	}

#if	CF_DEBUGS
	debugprintf("nodedb_check: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (nodedb_check) */


/* private subroutines */


static int nodedb_pwd(op)
NODEDB		*op ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;


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
static int nodedb_filefins(op)
NODEDB		*op ;
{
	NODEDB_FILE	*fep ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    rs1 = file_finish(fep) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	return rs ;
}
/* end subroutine (nodedb_filefins) */


/* check if the access table files have changed */
static int nodedb_checkfiles(op,daytime)
NODEDB		*op ;
time_t		daytime ;
{
	struct nodedb_file	*fep ;

	struct ustat	sb ;

	int	rs = SR_OK ;
	int	i ;
	int	c_changed = 0 ;


#if	CF_DEBUGS
	debugprintf("nodedb_checkfiles: enter\n") ;
#endif

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {
	    if (fep == NULL) continue ;

	    if ((u_stat(fep->fname,&sb) >= 0) &&
	        (sb.st_mtime > fep->mtime)) {

	        c_changed += 1 ;

#if	CF_DEBUGS
	        debugprintf("nodedb_checkfiles: file=%d changed\n",i) ;
	        debugprintf("nodedb_checkfiles: freeing file entries\n") ;
#endif

	        nodedb_filedump(op,i) ;

#if	CF_DEBUGS
	        debugprintf("nodedb_checkfiles: parsing the file again\n") ;
#endif

	        rs = nodedb_fileparse(op,i) ;
	        if (rs < 0) break ;

#if	CF_DEBUGS
	        debugprintf("nodedb_checkfiles: nodedb_fileparse rs=%d\n",rs) ;
#endif

	    } /* end if */

	} /* end for (looping through files) */

	op->checktime = daytime ;

#if	CF_DEBUGS
	debugprintf("nodedb_checkfiles: ret rs=%d changed=%d\n",
	    rs,c_changed) ;
#endif

	return (rs >= 0) ? c_changed : rs ;
}
/* end subroutine (nodedb_checkfiles) */


static int nodedb_fileparse(op,fi)
NODEDB		*op ;
int		fi ;
{
	struct nodedb_file	*fep ;

	struct ustat	sb ;

	bfile	loadfile, *lfp = &loadfile ;

	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	len ;
	int	cl ;
	int	c_added = 0 ;
	int	f_bol, f_eol ;

	const char	*cp ;

	char	lbuf[LINEBUFLEN + 1] ;


#if	CF_DEBUGS
	debugprintf("nodedb_fileparse: ent fi=%u\n",fi) ;
#endif

	rs = vecobj_get(&op->files,fi,&fep) ;
	if (rs < 0)
	    goto ret0 ;

	if (fep == NULL) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

#if	CF_DEBUGS
	debugprintf("nodedb_fileparse: 2\n") ;
#endif

	rs = bopen(lfp,fep->fname,"r",0664) ;
	if (rs < 0)
	    goto ret0 ;

	rs = bcontrol(lfp,BC_STAT,&sb) ;
	if (rs < 0)
	    goto done ;

	if (S_ISDIR(sb.st_mode)) {
	    rs = SR_ISDIR ;
	    goto done ;
	}

/* have we already parsed this one? */

#if	CF_DEBUGS
	debugprintf("nodedb_fileparse: 4\n") ;
#endif

	if (fep->mtime >= sb.st_mtime)
	    goto done ;

#if	CF_DEBUGS
	debugprintf("nodedb_fileparse: 5\n") ;
#endif

	fep->dev = sb.st_dev ;
	fep->ino = sb.st_ino ;
	fep->mtime = sb.st_mtime ;
	fep->size = sb.st_size ;

/* loop, reading all records and figuring things out */

	f_bol = TRUE ;
	while ((rs = breadlines(lfp,lbuf,llen,NULL)) > 0) {
	    len = rs ;

	    f_eol = (lbuf[len - 1] == '\n') ;
	    if (f_eol)
	        len -= 1 ;

	    lbuf[len] = '\0' ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	    debugprintf("nodedb_fileparse: line>%t<\n",lbuf,len) ;
#endif

	    cp = lbuf ;
	    cl = len ;
	    while (CHAR_ISWHITE(*cp)) {
	        cp += 1 ;
	        cl -= 1 ;
	    }

	    if ((*cp == '\0') || (*cp == '#'))
	        continue ;

	    if (f_bol) {
	        rs = nodedb_fileparseline(op,fi,cp,cl) ;
	        if (rs > 0) c_added += 1 ;
	    }

	    if (rs < 0)
	        break ;

	    f_bol = f_eol ;

	} /* end while (reading extended lines) */

done:
	bclose(lfp) ;
	if (rs < 0) goto bad1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("nodedb_fileparse: ret rs=%d added=%d\n",
	    rs,c_added) ;
#endif

	return (rs >= 0) ? c_added : rs ;

/* bad stuff */
bad1:
	nodedb_filedump(op,fi) ;
	goto ret0 ;
}
/* end subroutine (nodedb_fileparse) */


static int nodedb_fileparseline(op,fi,lp,ll)
NODEDB		*op ;
int		fi ;
const char	*lp ;
int		ll ;
{
	struct svcentry		se ;

	struct lineinfo		li ;

	FIELD	fsb ;

	const int	argslen = ARGSBUFLEN ;

	int	rs = SR_OK ;
	int	c_field = 0 ;
	int	fl ;
	int	f_ent = FALSE ;

	const char	*fp ;

	char	argsbuf[ARGSBUFLEN + 1] ;


	memset(&li,0,sizeof(struct lineinfo)) ;

	rs = field_start(&fsb,lp,ll) ;
	if (rs < 0) goto ret0 ;

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
	        if (fsb.term == '=')
	            al = field_srvarg(&fsb,saterms,argsbuf,argslen) ;

	        if ((rs >= 0) && f_ent)
	            rs = svcentry_addkey(&se,kp,kl,argsbuf,al) ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	        debugprintf("nodedb_fileparse: "
	            "svcentry_addkey() rs=%d\n", rs) ;
#endif

	    } /* end if (handling key-value pair) */

	    c_field += 1 ;
	    if (fsb.term == '#')
	        break ;

	    if (rs < 0)
	        break ;

	} /* end while (fields) */

	field_finish(&fsb) ;

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

ret0:
	return (rs >= 0) ? f_ent : rs ;
}
/* end subroutine (nodedb_fileparseline) */


#ifdef	COMMENT

/* do we have this entry already */
static int nodedb_already(op,nep)
NODEDB		*op ;
NODEDB_ENT 	*nep ;
{
	struct nodedb_key	*kep ;

	NODEDB_ENT	*ep ;

	HDB_CUR		cur ;
	HDB_DATUM	key, val ;

	int	rs ;
	int	f = FALSE ;


#if	CF_DEBUGS
	debugprintf("nodedb_already: new val=%s\n",nep->vname) ;
#endif

	kep = nep->kep ;

	key.buf = kep->kname ;
	key.len = strlen(kep->kname) ;

#if	CF_DEBUGS
	debugprintf("nodedb_already: new key=%s\n",kep->kname) ;
#endif

	if ((rs = hdb_curbegin(&op->entries,&cur)) >= 0) {

	    while (hdb_fetch(&op->entries,key,&cur,&val) >= 0) {

	        ep = (struct nodedb_ent *) val.buf ;

#if	CF_DEBUGS
	        debugprintf("nodedb_already: val=%s\n",ep->vname) ;
#endif

	        f = (strcmp(nep->vname,ep->vname) == 0) ;

	        if (f) break ;
	    } /* end while */

	    hdb_curend(&op->entries,&cur) ;
	} /* end if (cursor) */

	if ((rs >= 0) && (! f)) rs = SR_NOTFOUND ;

	return rs ;
}
/* end subroutine (nodedb_already) */

#endif /* COMMENT */


/* add an entry to the access entry list */
static int nodedb_addentry(op,fi,sep)
NODEDB		*op ;
int		fi ;
struct svcentry	*sep ;
{
	struct nodedb_ie	*iep ;

	int	rs ;
	int	size ;


	if (sep == NULL) return SR_FAULT ;

	size = sizeof(struct nodedb_ie) ;
	if ((rs = uc_malloc(size,&iep)) >= 0) {
	    if ((rs = ientry_start(iep,fi,sep)) >= 0) {
	        HDB_DATUM	key, val ;
	        key.buf = iep->svc ;
	        key.len = strlen(iep->svc) ;
	        val.buf = iep ;
	        val.len = sizeof(struct nodedb_ie) ;
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
static int nodedb_filedump(op,fi)
NODEDB		*op ;
int		fi ;
{
	struct nodedb_ie	*iep ;

	HDB		*elp = &op->entries ;
	HDB_CUR		cur ;
	HDB_DATUM	key, val ;

	int	rs ;
	int	rs1 ;
	int	c = 0 ;


#if	CF_DEBUGS
	debugprintf("nodedb_filedump: fi=%d\n",fi) ;
#endif

	if ((rs = hdb_curbegin(elp,&cur)) >= 0) {

	    while (hdb_enum(elp,&cur,&key,&val) >= 0) {

	        iep = (struct nodedb_ie *) val.buf ;

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


static int nodedb_filedel(op,fi)
NODEDB		*op ;
int		fi ;
{
	NODEDB_FILE	*fep ;

	int	rs ;
	int	rs1 ;


	rs = vecobj_get(&op->files,fi,&fep) ;

	if ((rs >= 0) && (fep != NULL)) {

	    rs1 = file_finish(fep) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = vecobj_del(&op->files,fi) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end if */

	return rs ;
}
/* end subroutine (nodedb_filedel) */


/* free up all of the entries in this NODEDB list */
static int nodedb_entfins(op)
NODEDB		*op ;
{
	int	rs ;


	rs = nodedb_filedump(op,-1) ;

	return rs ;
}
/* end subroutine (nodedb_entfins) */


static int file_start(fep,fname)
NODEDB_FILE	*fep ;
const char	fname[] ;
{
	int	rs = SR_OK ;

	const char	*cp ;


	if (fname == NULL)
	    return SR_FAULT ;

	memset(fep,0,sizeof(NODEDB_FILE)) ;

	rs = uc_mallocstrw(fname,-1,&cp) ;
	if (rs >= 0) fep->fname = cp ;

	return rs ;
}
/* end subroutine (file_start) */


static int file_finish(fep)
NODEDB_FILE	*fep ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (fep == NULL)
	    return SR_FAULT ;

	if (fep->fname != NULL) {
	    rs1 = uc_free(fep->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    fep->fname = NULL ;
	}

	return rs ;
}
/* end subroutine (file_finish) */


static int ientry_start(iep,fi,sep)
struct nodedb_ie	*iep ;
int			fi ;
struct svcentry		*sep ;
{
	struct svcentry_key	*kep ;

	const char	*(*keys)[2] ;

	int	rs ;
	int	i, j ;
	int	size ;
	int	c = 0 ;

	void	*p ;
	char	*bp ;


	if (iep == NULL) return SR_FAULT ;

	memset(iep,0,sizeof(struct nodedb_ie)) ;
	iep->fi = fi ;

/* basic checks on input data */

	if (sep == NULL) return SR_FAULT ;

	if ((sep->svc == NULL) || (sep->svc[0] == '\0')) return SR_INVALID ;

/* ok to continue */

	size = 0 ;
	c = vecobj_count(&sep->keys) ;
	if (c < 0) goto ret0 ;

	iep->nkeys = c ;

/* find the size to allocate (everything) */

	size += ((c + 1) * 2 * sizeof(char *)) ;

	for (i = 0 ; vecobj_get(&sep->keys,i,&kep) >= 0 ; i += 1) {
	    if (kep == NULL) continue ;
	    if (kep->kname != NULL) size += kep->kl ; 
	    size += 1 ;
	    if (kep->args != NULL) size += kep->al ; 
	    size += 1 ;
	}

	for (i = 0 ; i < 3 ; i += 1) {
	    const char	*cp ;
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
	    }
	    size += (strlen(cp) + 1) ;
	} /* end for */

/* allocate */

	iep->size = size ;
	rs = uc_malloc(size,&p) ;
	if (rs < 0) goto bad0 ;

/* copy over everything */

	iep->a = (const char *) p ;
	keys = (const char *(*)[2]) p ;

	iep->keys = keys ;

	bp = (char *) p ;
	bp += ((c + 1) * 2 * sizeof(char *)) ;

/* copy over the key-table and the key and value strings */

	j = 0 ;
	for (i = 0 ; vecobj_get(&sep->keys,i,&kep) >= 0 ; i += 1) {
	    if (kep == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("nodedb/ientry_start: kl=%d k=%s\n",
	        kep->kl,kep->kname) ;
	    debugprintf("nodedb/ientry_start: al=%d a=%s\n",
	        kep->al,kep->args) ;
#endif

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

	} /* end for */

	keys[j][0] = NULL ;
	keys[j][1] = NULL ;

/* copy over the other stuff */

	for (i = 0 ; i < 3 ; i += 1) {
	    const char	*cp ;
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
	    }
	    bp = (strwcpy(bp,cp,-1) + 1) ;
	} /* end for */

ret0:

#if	CF_DEBUGS
	debugprintf("nodedb/ientry_start: svc=%s\n",iep->svc) ;
	debugprintf("nodedb/ientry_start: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c : rs ;

/* bad things */
bad0:
	iep->svc = NULL ;
	goto ret0 ;
}
/* end subroutine (ientry_start) */


static int ientry_finish(iep)
NODEDB_IE	*iep ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (iep == NULL) return SR_FAULT ;

	if (iep->a != NULL) {
	    rs1 = uc_free(iep->a) ;
	    if (rs >= 0) rs = rs1 ;
	    iep->a = NULL ;
	}

	iep->svc = NULL ;
	return rs ;
}
/* end subroutine (ientry_finish) */


#ifdef	COMMENT

static int key_init(kep,kname)
struct nodedb_key	*kep ;
const char		kname[] ;
{
	int	kl ;


	memset(kep,0,sizeof(struct nodedb_key)) ;

	kl = NODEDB_SVCLEN ;
	if (kname != NULL)
	    kep->kname = mallocstrw(kname,kl) ;

	return (kep->kname != NULL) ? SR_OK : SR_NOMEM ;
}


static int key_free(kep)
struct nodedb_key	*kep ;
{


	if (kep->kname != NULL) {
	    uc_free(kep->kname) ;
	    kep->kname = NULL ;
	}

	return 0 ;
}

#endif /* COMMENT */


static int svcentry_start(sep,lip)
struct svcentry	*sep ;
struct lineinfo	*lip ;
{
	int	rs ;
	int	i ;
	int	size ;

	char	*bp ;


	if (lip == NULL) return SR_FAULT ;

	memset(sep,0,sizeof(struct svcentry)) ;

	size = 0 ;
	for (i = 0 ; i < 3 ; i += 1) {
	    const char	*cp = lip->f[i].fp ;
	    int		cl = lip->f[i].fl ;
	    if (cl < 0) cl = strlen(cp) ;
	    size += (cl + 1) ;
	} /* end for */

	rs = uc_malloc(size,&bp) ;
	if (rs < 0) goto bad0 ;

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
	    }
	    bp = strwcpy(bp,lip->f[i].fp,lip->f[i].fl) + 1 ;
	} /* end for */

/* prepare for arguments */

	size = sizeof(struct svcentry_key) ;
	rs = vecobj_start(&sep->keys,size,5,VECOBJ_PORDERED) ;
	if (rs < 0) goto bad1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("svcentry_start: svc=%s\n",sep->svc) ;
#endif

	return rs ;

/* bad stuff */
bad1:
	uc_free(sep->a) ;
	sep->a = NULL ;

bad0:
	sep->svc = NULL ;
	goto ret0 ;
}
/* end subroutine (svcentry_start) */


/* add a key to this entry */
static int svcentry_addkey(sep,kp,kl,ap,al)
struct svcentry	*sep ;
const char	*kp ;
int		kl ;
const char	*ap ;
int		al ;
{
	struct svcentry_key	key ;

	int	rs ;
	int	size ;

	char	*bp ;


	if (sep == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("nodedb/svcentry_addkey: kl=%d kp=%t\n",kl,kp,kl) ;
	if (al >= 0)
	    debugprintf("nodedb/svcentry_addkey: al=%d ap=%t\n",al,ap,al) ;
#endif

/* sanity checks on input arguments */

	if (kp == NULL) return SR_FAULT ;

	if (kp[0] == '\0') return SR_INVALID ;

/* ok */

	memset(&key,0,sizeof(struct svcentry_key)) ;

	size = 0 ;
	if (kl < 0) kl = strlen(kp) ;
	size += (kl + 1) ;
	if (ap != NULL) {
	    if (al < 0) al = strlen(ap) ;
	    size += (al + 1) ;
	} else
	    size += 1 ;

	rs = uc_malloc(size,&bp) ;
	if (rs < 0) goto bad0 ;

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
	} else
	    *bp++ = '\0' ;

/* add the key object to the key-list */

	rs = vecobj_add(&sep->keys,&key) ;
	if (rs < 0) goto bad1 ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	if (key.kname != NULL) {
	    uc_free(key.kname) ;
	    key.kname = NULL ;
	    key.args = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (svcentry_addkey) */


/* free up an entry */
static int svcentry_finish(sep)
struct svcentry	*sep ;
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (sep->svc != NULL) {
	    struct svcentry_key	*kep ;
	    int	i ;
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


static int entry_load(ep,ebuf,elen,iep)
NODEDB_ENT	*ep ;
char		ebuf[] ;
int		elen ;
NODEDB_IE	*iep ;
{
	int	rs = SR_OK ;
	int	svclen ;
	int	bo, i, kal ;

	const char	*(*keys)[2] ;

#if	CF_DEBUGS
	debugprintf("entry_load: elen=%u\n",elen) ;
#endif

#if	CF_DEBUGS
	debugprintf("entry_load: ie size=%u nkeys=%u svc=%s\n",
	    iep->size,iep->nkeys,iep->svc) ;
#endif

	bo = NODEDB_BO((ulong) ebuf) ;
	if (iep->size <= (elen - bo)) {
	    char	*bp ;

	    keys = (const char *(*)[2]) (ebuf + bo) ;
	    kal = (iep->nkeys + 1) * 2 * sizeof(char *) ;
	    bp = (char *) (ebuf + bo + kal) ;

/* copy in the nodename */

	    ep->svc = bp ;
	    bp = strwcpy(bp,iep->svc,-1) + 1 ;

	    svclen = (bp - ep->svc - 1) ;

#if	CF_DEBUGS
	    debugprintf("nodedb_fetch: svc=%s svclen=%u\n",ep->svc,svclen) ;
#endif

/* copy in the clustername */

	    ep->clu = bp ;
	    if (iep->clu != NULL) {
	        bp = strwcpy(bp,iep->clu,-1) + 1 ;
	    } else
	        *bp++ = '\0' ;

/* copy in the systemname */

	    ep->sys = bp ;
	    if (iep->sys != NULL) {
	        bp = strwcpy(bp,iep->sys,-1) + 1 ;
	    } else
	        *bp++ = '\0' ;

/* copy in the key=values */

	    for (i = 0 ; i < iep->nkeys ; i += 1) {

	        keys[i][0] = bp ;
	        bp = strwcpy(bp,iep->keys[i][0],-1) + 1 ;

#if	CF_DEBUGS
	        debugprintf("nodedb_fetch: k=%s\n",keys[i][0]) ;
#endif

	        if (iep->keys[i][1] != NULL) {
	            keys[i][1] = bp ;
	            bp = strwcpy(bp,iep->keys[i][1],-1) + 1 ;
	        } else
	            keys[i][1] = NULL ;

	    } /* end for */

	    keys[i][0] = NULL ;
	    keys[i][1] = NULL ;

	    ep->keys = keys ;
	    ep->fi = iep->fi ;
	    ep->nkeys = iep->nkeys ;
	    ep->size = iep->size ;

	} else
	    rs = SR_OVERFLOW ;

	return (rs >= 0) ? svclen : rs ;
}
/* end subroutine (entry_load) */


static int freeit(pp)
const char	**pp ;
{
	int		rs = SR_OK ;
	if (*pp != NULL) {
	    rs = uc_free(*pp) ;
	    *pp = NULL ;
	}
	return rs ;
}
/* end subroutine (freeit) */


static int cmpfname(e1pp,e2pp)
struct nodedb_file	**e1pp, **e2pp ;
{
	int		rc ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

	rc = strcmp((*e1pp)->fname,(*e2pp)->fname) ;

	return rc ;
}
/* end subroutine (cmpfname) */


#ifdef	COMMENT
static int cmpkey(e1pp,e2pp)
struct nodedb_key	**e1pp, **e2pp ;
{
	int		rc ;

	if ((*e1pp == NULL) && (*e2pp == NULL))
	    return 0 ;

	if (*e1pp == NULL)
	    return 1 ;

	if (*e2pp == NULL)
	    return -1 ;

#if	CF_DEBUGS
	debugprintf("nodedb/cmpkey: k1(%p)=%s k2(%p)=%s\n",
	    (*e1pp)->kname,
	    (*e1pp)->kname,
	    (*e2pp)->kname,
	    (*e2pp)->kname) ;
#endif

	rc = strcmp((*e1pp)->kname,(*e2pp)->kname) ;

	return rc ;
}
/* end subroutine (cmpkey) */
#endif /* COMMENT */


