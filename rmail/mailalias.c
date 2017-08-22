/* mailalias */
/* lang=C++11 */

/* manage a MAILALIAS object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFILE	1		/* file parsing */
#define	CF_DEBUGSSHIFT	0
#define	CF_SAFE		1		/* safe mode */
#define	CF_FASTKEYMATCH	1		/* faster key matching */
#define	CF_DEFPROFILE	0		/* always use default MA-profile */


/* revision history:

	= 2003-06-11, David A­D­ Morano
	I snarfed this file from some hardware research use since it seemed be
	a pretty good match for the present algorithm needs.  We'll see how it
	works out!

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module is used to manage a MAILALIAS object.

	We always check if your alias key is the string "Postmater" (in a
	case-insensitive way) and if it is, we convert it to LOWER case before
	doing the 'mailalias_fetch()'.  Also, unlike other fetching (with other
	DBs), there is no concept of "prefix" match fetching!

        We use TWO string tables in this DB (unlike some other similar DBs). We
        use one string table for keys and another for values. I sort of did this
        for fun (so far). This is actually potentially less space-efficient than
        using just one string table since strings that are both a key and a
        value are not combined in one table but rather appear separately in each
        of the two tables. However, the up side is that:

	a. making a record table of only keys is done by just taking
	advantage of the STRTAB method for that

	b. we easily can provide an interface to transverse or
	otherwise just query the keys if we want to later

        c. we can do more space-efficient building of the DB file since we only
        need to keep the smaller key string table around for the main indexing!


*******************************************************************************/


#define	MAILALIAS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>		/* Memory Management */
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<strings.h>		/* |strncasecmp(3c)| */

#include	<vsystem.h>
#include	<endian.h>
#include	<endianstr.h>
#include	<vecobj.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<kvsfile.h>
#include	<ids.h>
#include	<strtab.h>
#include	<localmisc.h>

#include	"mailalias.h"


/* local defines */

#define	MAILALIAS_DNAME		"var/mailalias"
#define	MAILALIAS_FE		"mac"		/* Mail Alias Cache */
#define	MAILALIAS_DIRMODE	0775

#define	MAILALIAS_IDLEN		(MAILALIAS_FILEMAGICSIZE + 4)
#define	MAILALIAS_HEADLEN	(header_overlast * sizeof(int))
#define	MAILALIAS_TOPLEN	(MAILALIAS_IDLEN + MAILALIAS_HEADLEN)

#define	MAILALIAS_IDOFF		0
#define	MAILALIAS_HEADOFF	MAILALIAS_IDLEN
#define	MAILALIAS_BUFOFF	(MAILALIAS_HEADOFF + MAILALIAS_HEADLEN)

#define	DBMAKE			class dbmake

#define	RECORD			class record

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_APROFILE	(1 * 60)
#define	TO_FILECOME	15		/* timeout for file to "come in" */
#define	TO_LOCK		(5 * 60)
#define	TO_OPEN		(60 * 60)
#define	TO_ACCESS	(1 * 60)
#define	TO_CHECK	5		/* minimum check interval */
#define	TO_FILECHECK	5		/* DB file check */
#define	TO_FILEOLD	10		/* backing-store check */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((3 * 1024),2048)
#endif

#ifndef	ALIASNAMELEN
#define	ALIASNAMELEN	64
#endif

#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NREC_GUESS	100		/* guess of number of records */
#define	NSHIFT		6


/* external subroutines */

#if	defined(BSD) && (! defined(EXTERN_STRNCASECMP))
extern "C" int	strncasecmp(const char *,const char *,int) ;
#endif

extern "C" uint	nextpowtwo(uint) ;
extern "C" uint	uceil(uint,int) ;
extern "C" uint	ulceil(ulong,int) ;
extern "C" uint	hashelf(const char *,int) ;

extern "C" int	snopenflags(char *,int,int) ;
extern "C" int	sncpy2(char *,int,const char *,const char *) ;
extern "C" int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern "C" int	sncpy4(char *,int,cchar *,cchar *,cchar *,cchar *) ;
extern "C" int	sncpy5(char *,int,cchar *,cchar *,cchar *,cchar *,cchar *) ;
extern "C" int	mkpath1(char *,const char *) ;
extern "C" int	mkpath2(char *,const char *,const char *) ;
extern "C" int	mkpath3(char *,const char *,const char *,const char *) ;
extern "C" int	mkpath1w(char *,const char *,int) ;
extern "C" int	mkfnamesuf1(char *,const char *,const char *) ;
extern "C" int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern "C" int	matstr(const char **,const char *,int) ;
extern "C" int	mkmagic(char *,int,cchar *) ;
extern "C" int	sfshrink(cchar *,int,cchar **) ;
extern "C" int	sfdirname(cchar *,int,cchar **) ;
extern "C" int	randlc(int) ;
extern "C" int	sperm(IDS *,USTAT *,int) ;
extern "C" int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern "C" int	mkdirs(cchar *,mode_t) ;
extern "C" int	opentmpfile(cchar *,int,mode_t,char *) ;
extern "C" int	msleep(int) ;
extern "C" int	isfsremote(int) ;
extern "C" int	isNotPresent(int) ;
extern "C" int	isNotAccess(int) ;
extern "C" int	isFailOpen(int) ;
extern "C" int	isValidMagic(cchar *,int,cchar *) ;

#if	CF_DEBUGS || CF_DEBUG
extern "C" int	debugprintf(const char *,...) ;
extern "C" int	strlinelen(const char *,int,int) ;
#endif

extern "C" char	*strwcpy(char *,const char *,int) ;
extern "C" char	*strwcpylc(char *,const char *,int) ;
extern "C" char	*strwcpyuc(char *,const char *,int) ;
extern "C" char	*timestr_log(time_t,char *) ;
extern "C" char	*timestr_logz(time_t,char *) ;


/* local structures */

class dbmake {
public:
	VECOBJ		*rlp ;
	STRTAB		*klp ;
	STRTAB		*vlp ;
	uint		nrecs ;
	uint		nkeys ;
	int		fd ;
	int		ikey ;
	int		c ;
	int		ktlen ;
	int		ktsize ;
	int		reclen ;
	int		recsize ;
	int		rilen ;
	int		risize ;
	int		sksize ;
	int		svsize ;
	int		f_havekey ;
	dbmake(VECOBJ *arlp,STRTAB *aklp,STRTAB *avlp,int afd) : ikey(0), c(0) {
	    rlp = arlp ;
	    klp = aklp ;
	    vlp = avlp ;
	    fd = afd ;
	    f_havekey = FALSE ;
	} ;
} ;

class record {
public:
	record() : key(0), val(0) { 
	} ;
	uint		key ;
	uint		val ;
} ;


/* forward references */

static int	mailalias_hdrload(MAILALIAS *) ;
static int	mailalias_hdrloader(MAILALIAS *) ;
static int	mailalias_enterbegin(MAILALIAS *,time_t) ;
static int	mailalias_enterend(MAILALIAS *,time_t) ;

static int	mailalias_fileopen(MAILALIAS *,time_t) ;
static int	mailalias_fileclose(MAILALIAS *) ;
static int	mailalias_filechanged(MAILALIAS *,struct ustat *) ;
static int	mailalias_mapbegin(MAILALIAS *,time_t dt) ;
static int	mailalias_mapend(MAILALIAS *) ;

#if	(! CF_FASTKEYMATCH)
static int	mailalias_keymatch(MAILALIAS *,int,int,cchar *) ;
#endif

static int	mailalias_dbopen(MAILALIAS *,time_t) ;
static int	mailalias_dbopenfile(MAILALIAS *,time_t) ;
static int	mailalias_dbopenmake(MAILALIAS *,time_t) ;
static int	mailalias_dbopenwait(MAILALIAS *) ;
static int	mailalias_isremote(MAILALIAS *) ;
static int	mailalias_dbclose(MAILALIAS *) ;
static int	mailalias_dbmake(MAILALIAS *,time_t) ;
static int	mailalias_dbmaker(MAILALIAS *,time_t,cchar *) ;
static int	mailalias_dbmaking(MAILALIAS *,int,time_t,int) ;
static int	mailalias_procfile(MAILALIAS *,DBMAKE *,cchar *) ;
static int	mailalias_procfileline(MAILALIAS *,DBMAKE *,cchar *,int) ;
static int	mailalias_wrfile(MAILALIAS *,DBMAKE *, time_t) ;
static int	mailalias_wrfiler(MAILALIAS *,DBMAKE *, time_t) ;
static int	mailalias_wrfilekeytab(MAILALIAS *,DBMAKE *) ;
static int	mailalias_wrfilerec(MAILALIAS *,DBMAKE *) ;
static int	mailalias_wrfilekeys(MAILALIAS *,DBMAKE *) ;
static int	mailalias_wrfilevals(MAILALIAS *,DBMAKE *) ;
static int	mailalias_mkind(MAILALIAS *,VECOBJ *,cchar *,int (*)[2],int) ;
static int	mailalias_fileold(MAILALIAS *,time_t) ;
static int	mailalias_aprofile(MAILALIAS *,time_t) ;

static int	mailalias_checkchanged(MAILALIAS *,time_t) ;
static int	mailalias_checkold(MAILALIAS *,time_t) ;

static int	mailalias_mapcheck(MAILALIAS *,time_t) ;

static int	hashindex(uint,int) ;


/* local variables */

enum headers {
	header_wtime,
	header_wcount,
	header_key,
	header_keylen,
	header_rec,
	header_reclen,
	header_ind,
	header_indlen,
	header_skey,
	header_skeysize,
	header_sval,
	header_svalsize,
	header_overlast
} ;

static const char	*defprofile[] = {
	"/etc/mail/aliases",
	"etc/mail/aliases",
	"var/mail/nis.aliases",
	NULL
} ;

static const char	*aptabsched[] = {
	"etc/mail/mail.aptab",
	"etc/mail/aptab",
	"etc/mail.aptab",
	"etc/aptab",
	NULL
} ;

/* all white space plus colon (':') */
static const uchar	kterms[] = {
	0x00, 0x1F, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

/* all white space plus comma (',') */
static const uchar	vterms[] = {
	0x00, 0x1F, 0x00, 0x00,
	0x01, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int mailalias_open(MAILALIAS *op,cchar *pr,cchar *pname,int of,mode_t om,int ot)
{
	const time_t	dt = time(NULL) ;
	int		rs ;
	int		f_create = FALSE ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;
	if (pname == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("mailalias_open: ent\n") ;
#endif

	memset(op,0,sizeof(MAILALIAS)) ;
	op->fd = -1 ;
	op->oflags = of ;
	op->operm = om ;
	op->otype = ot ;

	op->f.ocreate = ((of & O_CREAT) == O_CREAT) ;
	op->f.owrite |= ((of & O_WRONLY) == O_WRONLY) ;
	op->f.owrite |= ((of & O_RDWR) == O_RDWR) ;

	op->aprofile = defprofile ;
	op->pagesize = getpagesize() ;

	if ((rs = ids_load(&op->id)) >= 0) {
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	        op->pr = cp ;
	        if ((rs = uc_mallocstrw(pname,-1,&cp)) >= 0) {
	            cchar	*fe = MAILALIAS_FE ;
	            char	endian[2] ;
	            char	fname[MAXPATHLEN + 1] ;
	            op->apname = cp ;
	            endian[0] = (ENDIAN) ? '1' : '0' ;
	            endian[1] = '\0' ;
	            if ((rs = mkfnamesuf2(fname,pname,fe,endian)) >= 0) {
	                const char	*db = MAILALIAS_DNAME ;
	                char		dbfname[MAXPATHLEN + 1] ;
	                if ((rs = mkpath3(dbfname,pr,db,fname)) >= 0) {
	                    if ((rs = uc_mallocstrw(dbfname,-1,&cp)) >= 0) {
	                        VECSTR		*alp = &op->apfiles ;
	                        const int	vo = VECSTR_PNOHOLES ;
	                        op->dbfname = cp ;
	                        if ((rs = vecstr_start(alp,5,vo)) >= 0) {
	                            if ((rs = mailalias_dbopen(op,dt)) >= 0) {
	                                f_create = (rs > 0) ;
	                                op->magic = MAILALIAS_MAGIC ;
	                            }
	                            if (rs < 0)
	                                vecstr_finish(&op->apfiles) ;
	                        } /* end if (vecstr_start) */
	                        if (rs < 0) {
	                            uc_free(op->dbfname) ;
	                            op->dbfname = NULL ;
	                        }
	                    } /* end if (m-a) */
	                } /* end if (mkpath) */
	            } /* end if (mkfnamesuf) */
	            if (rs < 0) {
	                uc_free(op->apname) ;
	                op->apname = NULL ;
	            }
	        } /* end if (ma-a) */
	        if (rs < 0) {
	            uc_free(op->pr) ;
	            op->pr = NULL ;
	        }
	    } /* end if (m-a) */
	    if (rs < 0)
	        ids_release(&op->id) ;
	} /* end if (ids_load) */

#if	CF_DEBUGS
	debugprintf("mailalias_open: ret rs=%d f_create=%u\n",
	    rs,f_create) ;
#endif

	return (rs >= 0) ? f_create : rs ;
}
/* end subroutine (mailalias_open) */


/* free up this mailalias object */
int mailalias_close(MAILALIAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	rs1 = mailalias_dbclose(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = vecstr_finish(&op->apfiles) ;
	if (rs >= 0) rs = rs1 ;

	if (op->dbfname != NULL) {
	    rs1 = uc_free(op->dbfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->dbfname = NULL ;
	}

	if (op->apname != NULL) {
	    rs1 = uc_free(op->apname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->apname = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	rs1 = ids_release(&op->id) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (mailalias_close) */


/* get the string count in the table */
int mailalias_audit(MAILALIAS *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	return rs ;
}
/* end subroutine (mailalias_audit) */


/* get the string count in the table */
int mailalias_count(MAILALIAS *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	return (op->rtlen - 1) ;
}
/* end subroutine (mailalias_count) */


/* calculate the index table length (number of entries) at this point */
int mailalias_indcount(MAILALIAS *op)
{

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	return op->rilen ;
}
/* end subroutine (mailalias_indcount) */


/* initialize a cursor */
int mailalias_curbegin(MAILALIAS *op,MAILALIAS_CUR *curp)
{

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	op->cursors += 1 ;
	op->f.cursorlockbroken = FALSE ;
	op->f.cursoracc = FALSE ;

	curp->i = -1 ;

	curp->magic = MAILALIAS_MAGIC ;
	return SR_OK ;
}
/* end subroutine (mailalias_curbegin) */


/* free up a cursor */
int mailalias_curend(MAILALIAS *op,MAILALIAS_CUR *curp)
{
	const time_t	dt = time(NULL) ;
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	if (op->f.cursoracc)
	    op->ti_access = dt ;

	if (op->cursors > 0) op->cursors -= 1 ;
	curp->i = -1 ;

	curp->magic = 0 ;
	return rs ;
}
/* end subroutine (mailalias_curend) */


/* enumerate */
int mailalias_enum(MAILALIAS *op,MAILALIAS_CUR *curp,char *kbuf,int klen,
		char *vbuf,int vlen)
{
	time_t		dt = 0 ;
	int		rs ;
	int		rs1 ;
	int		ri = 0 ;
	int		vl = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (curp == NULL) return SR_FAULT ;

	if (curp->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	if (op->cursors == 0) return SR_INVALID ;

	ri = (curp->i < 1) ? 1 : (curp->i + 1) ;

#if	CF_DEBUGS
	debugprintf("mailalias_enum: ri=%d\n",ri) ;
#endif

/* capture a hold on the file */

	if ((rs = mailalias_enterbegin(op,dt)) >= 0) {
	    if (ri < op->rtlen) {
	        int	ai, vi ;
	        int	cl ;

	        ai = op->rectab[ri][0] ;
	        vi = op->rectab[ri][1] ;

#if	CF_DEBUGS
	        debugprintf("mailalias_enum: ai=%u vi=%u\n",ai,vi) ;
#endif

	        if ((ai < op->sklen) && (vi < op->svlen)) {
	            char	*bp ;

	            if (kbuf != NULL) {

	                cl = ALIASNAMELEN ;
	                if (klen >= 0)
	                    cl = MIN(klen,ALIASNAMELEN) ;

	                bp = strwcpy(kbuf,(op->skey + ai),cl) ;
	                vl = (bp - kbuf) ;

	            } else {
	                vl = strlen(op->skey + ai) ;
	            }

	            if (vbuf != NULL) {
	                cl = MAILADDRLEN ;
	                if (vlen >= 0) {
	                    cl = MIN(vlen,MAILADDRLEN) ;
	                }
	                strwcpy(vbuf,(op->sval + vi),cl) ;
	            } /* end if (value buffer present) */

/* update the cursor */

	            curp->i = ri ;

	        } else {
	            rs = SR_BADFMT ;
	        }

	    } else {
	        rs = SR_NOTFOUND ;
	    }
	    rs1 = mailalias_enterend(op,dt) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if ( mailalias-enter) */

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mailalias_enum) */


/* fetch an entry by key lookup */
/* ARGSUSED */
int mailalias_fetch(MAILALIAS *op,int opts,cchar *aname,MAILALIAS_CUR *curp,
		char *vbuf,int vlen)
{
	MAILALIAS_CUR	cur ;
	time_t		dt = 0 ;
	uint		khash ;
	int		rs = SR_OK ;
	int		vl = 0 ;
	int		rs1 ;
	int		f_cur = FALSE ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;
#endif

	if (aname == NULL) return SR_INVALID ;

	if (curp == NULL) {
	    curp = &cur ;
	    curp->i = -1 ;
	} else {
	    f_cur = TRUE ;
	    if (op->cursors == 0) {
	        rs = SR_INVALID ;
	    }
	} /* end if */

	if (rs >= 0) {
	    if ((rs = mailalias_enterbegin(op,dt)) >= 0) {
	        const int	ns = NSHIFT ;
	        int		vi, hi, ri, ki ;
	        int		cl, hl, n ;
	        int		f ;
	        const char	*hp ;
	        const char	*cp ;

/* continue with regular fetch activities */

	        op->f.cursoracc = TRUE ;	/* doesn't hurt if no cursor! */
	        n = op->rilen ;

/* OK, we go from here */

	        if (curp->i < 0) {
	            char	keybuf[ALIASNAMELEN + 1] ;

#if	CF_DEBUGS
	            debugprintf("mailalias_fetch: new cursor\n") ;
#endif

	            f = (strcasecmp(aname,"Postmaster") == 0) ;

	            if (f) {
	                hp = keybuf ;
	                cp = strwcpylc(keybuf,aname,ALIASNAMELEN) ;
	                hl = cp - keybuf ;
	            } else {
	                hp = aname ;
	                hl = strlen(aname) ;
	            } /* end if */

	            khash = hashelf(hp,hl) ;

	            hi = hashindex(khash,op->rilen) ;

#if	CF_DEBUGS
	            debugprintf("mailalias_fetch: khash=%08x hi=%d\n",
	                khash,hi) ;
#endif

/* start searching! */

	            if (op->ropts & MAILALIAS_OSEC) {
			int	c = 0 ;
	                int	f ;

#if	CF_DEBUGS
	                debugprintf("mailalias_fetch: "
	                    "secondary initial ri=%d\n",(op->indtab)[hi][0]) ;
#endif

	                c = 0 ;
	                while ((ri = (op->indtab)[hi][0]) != 0) {

#if	CF_DEBUGS
	                    debugprintf("mailalias_fetch: trymatch ri=%d\n",
	                        ri) ;
#endif

#if	CF_FASTKEYMATCH
	                    ki = op->rectab[ri][0] ;
	                    f = (strcmp(aname,(op->skey + ki)) == 0) ;
#else
	                    f = mailalias_keymatch(op,opts,ri,aname) ;
#endif

	                    if (f)
	                        break ;

	                    op->collisions += 1 ;
	                    if (op->ropts & MAILALIAS_ORANDLC) {
	                        khash = randlc(khash + c) ;
	                    } else {
	                        khash = 
	                            ((khash << (32 - ns)) | (khash >> ns)) + c ;
	                    }

	                    hi = hashindex(khash,n) ;

#if	CF_DEBUGS
	                    debugprintf("mailalias_fetch: "
	                        "new khash=%08x hi=%d\n", khash,hi) ;
#endif

	                    c += 1 ;

	                } /* end while */

#if	CF_DEBUGS
	                debugprintf("mailalias_fetch: index-key-match ri=%d\n",
	                    ri) ;
#endif

	                if (ri == 0) {
	                    rs = SR_NOTFOUND ;
	                }

	            } /* end if (secondary hasing) */

	        } else {

/* get the next record index (if there is one) */

#if	CF_DEBUGS
	            debugprintf("mailalias_fetch: old cursor\n") ;
#endif

	            hi = curp->i ;
	            if (hi != 0) {
	                ri = (op->indtab)[hi][0] ;
	                if (ri != 0) {
	                    hi = (op->indtab)[hi][1] ;
	                    if (hi != 0) {
	                        ri = (op->indtab)[hi][0] ;
	                    } else {
	                        rs = SR_NOTFOUND ;
	                    }
	                } else {
	                    rs = SR_NOTFOUND ;
	                }
	            } else {
	                rs = SR_NOTFOUND ;
	            }

	        } /* end if (preparation) */

#if	CF_DEBUGS
	        debugprintf("mailalias_fetch: match search rs=%d hi=%d\n",
	            rs,hi) ;
#endif

	        if (rs >= 0) {

	            while ((ri = (op->indtab)[hi][0]) != 0) {

#if	CF_FASTKEYMATCH
	                ki = op->rectab[ri][0] ;
	                f = (strcmp(aname,(op->skey + ki)) == 0) ;
#else
	                f = mailalias_keymatch(op,opts,ri,aname) ;
#endif

	                if (f) break ;

	                hi = (op->indtab)[hi][1] ;
	                if (hi == 0) break ;

	                op->collisions += 1 ;

	            } /* end while */

	            if ((ri == 0) || (hi == 0)) {
	                rs = SR_NOTFOUND ;
	            }

	        } /* end if (following the existing chain) */

#if	CF_DEBUGS
	        debugprintf("mailalias_fetch: searched rs=%d ri=%u hi=%u\n",
	            rs,ri,hi) ;
#endif

/* if successful, retrieve value */

	        if (rs >= 0) {

	            vi = op->rectab[ri][1] ;
	            if (vbuf != NULL) {
	                cl = MIN(vlen,ALIASNAMELEN) ;
	                cp = strwcpy(vbuf,(op->sval + vi),cl) ;
#if	CF_DEBUGS
	                debugprintf("mailalias_fetch: value=%s\n",vbuf) ;
#endif
	                vl = (cp - vbuf) ;
	            } else {
	                vl = strlen(op->sval + vi) ;
	            }

/* update cursor */

	            if (f_cur) {
#if	CF_DEBUGS
	                debugprintf("mailalias_fetch: cursor update hi=%u\n",
	                    hi) ;
#endif
	                curp->i = hi ;
	            }

	        } /* end if (got one) */

	        rs1 = mailalias_enterend(op,dt) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (mailalias-enter) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("mailalias_fetch: ret rs=%d vl=%d\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mailalias_fetch) */


#ifdef	COMMENT
/* get information */
int mailalias_info(MAILALIAS *op,MAILALIAS_INFO *rp)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;
	if (rp == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	if (rp != NULL) {
	    rp->collisions = op->collisions ;
	}

	return rs ;
}
/* end subroutine (mailalias_info) */
#endif /* COMMENT */


/* do some checking */
int mailalias_check(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	if ((! op->f.held) && (op->mapdata != NULL)) {
	    if (dt == 0) dt = time(NULL) ;
	    if ((dt - op->ti_check) >= TO_CHECK) {
	        op->ti_check = dt ;
	        if ((rs = mailalias_checkchanged(op,dt)) > 0) {
	            f = TRUE ;
	            rs = mailalias_fileclose(op) ;
	        } else if (rs == 0) {
	            if ((rs = mailalias_checkold(op,dt)) > 0) {
	                f = TRUE ;
	                rs = mailalias_fileclose(op) ;
	            }
	        }
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_check) */


/* private subroutines */


static int mailalias_checkchanged(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if ((dt - op->ti_filecheck) > TO_FILECHECK) {
	    USTAT	sb ;
	    op->ti_filecheck = dt ;
	    if ((rs = u_stat(op->dbfname,&sb)) >= 0) {
	        rs = mailalias_filechanged(op,&sb) ;
	        f = rs ;
	    } else if (isNotPresent(rs)) {
	        rs = SR_OK ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_checkchanged) */


/* is the file old? */
static int mailalias_checkold(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (op->f.ocreate && op->f.owrite) {
	    if ((dt - op->ti_fileold) > TO_FILEOLD) {
	        op->ti_fileold = dt ;
	        rs = mailalias_fileold(op,dt) ;
	        f = rs ;
	    }
	} /* end if (within check interval) */
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_checkold) */


/* read the file header and check it out */
static int mailalias_hdrload(MAILALIAS *op)
{
	const int	msize = MAILALIAS_FILEMAGICSIZE ;
	int		rs = SR_OK ;
	cchar		*cp = (cchar *) op->mapdata ;
	if (isValidMagic(cp,msize,MAILALIAS_FILEMAGIC)) {
	    cp += msize ;
	    if (cp[0] == MAILALIAS_FILEVERSION) {
	        if (cp[1] == ENDIAN) {
	            op->ropts = MKCHAR(cp[2]) ;
	            rs = mailalias_hdrloader(op) ;
	        } else {
	            rs = SR_NOTSUP ;
	        }
	    } else {
	        rs = SR_NOTSUP ;
	    }
	} else {
	    rs = SR_BADFMT ;
	}
	return rs ;
}
/* end subroutine (mailalias_hdrload) */


static int mailalias_hdrloader(MAILALIAS *op)
{
	uint		*table = (uint *) (op->mapdata + MAILALIAS_IDLEN) ;
	int		rs = SR_OK ;

#if	CF_DEBUGS
	{
	    int	i ;
	    for (i = 0 ; i < header_overlast ; i += 1) {
	        debugprintf("mailalias_hdrload: header[%3u]=%u\n",
	            i,table[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

#ifdef	COMMENT
	op->i.wtime = table[header_wtime] ;
	op->i.wcount = table[header_wcount] ;
#endif

	op->keytab = (int *) (op->mapdata + table[header_key]) ;
	op->rectab = (int (*)[2]) (op->mapdata + table[header_rec]) ;
	op->indtab = (int (*)[2]) (op->mapdata + table[header_ind]) ;
	op->skey = (cchar *) (op->mapdata + table[header_skey]) ;
	op->sval = (cchar *) (op->mapdata + table[header_sval]) ;

	op->ktlen = table[header_keylen] ;
	op->rtlen = table[header_reclen] ;
	op->rilen = table[header_indlen] ;
	op->sklen = table[header_skeysize] ;
	op->svlen = table[header_svalsize] ;

	return rs ;
}
/* end subroutine (mailalias_hdrloader) */


static int mailalias_fileopen(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("mailalias_fileopen: fname=%s\n",op->dbfname) ;
#endif

	if (op->fd < 0) {
	    int	oflags = op->oflags ;
	    oflags &= (~ O_CREAT) ;
	    oflags &= (~ O_RDWR) ;
	    oflags &= (~ O_WRONLY) ;
	    oflags |= O_RDONLY ;
	    if ((rs = u_open(op->dbfname,oflags,op->operm)) >= 0) {
	        op->fd = rs ;
	        op->ti_open = dt ;
	        rs = uc_closeonexec(op->fd,TRUE) ;
		if (rs < 0) {
		    u_close(op->fd) ;
		    op->fd = -1 ;
		}
	    } /* end if */
	} /* end if (needed open) */

	return (rs >= 0) ? op->fd : rs ;
}
/* end subroutine (mailalias_fileopen) */


static int mailalias_fileclose(MAILALIAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}
	return rs ;
}
/* end subroutine (mailalias_fileclose) */


static int mailalias_mapbegin(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	if (op->mapdata == NULL) {
	    size_t	ms = ulceil(op->fi.size,op->pagesize) ;
	    const int	fd = op->fd ;
	    int		mp = PROT_READ ;
	    int		mf = MAP_SHARED ;
	    void	*md ;
	    if (ms == 0) ms = op->pagesize ;
	    if (dt == 0) dt = time(NULL) ;
	    if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	        op->mapdata = (cchar *) md ;
	        op->mapsize = ms ;
	        op->ti_map = dt ;
	        f = TRUE ;
	    }
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_mapbegin) */


static int mailalias_mapend(MAILALIAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;
	if (op->mapdata != NULL) {
	    rs1 = u_munmap(op->mapdata,op->mapsize) ;
	    if (rs >= 0) rs = rs1 ;
	    op->mapdata = NULL ;
	    op->mapsize = 0 ;
	}
	return rs ;
}
/* end subroutine (mailalias_mapend) */


#if	(! CF_FASTKEYMATCH)

static int mailalias_keymatch(MAILALIAS *op,int opts,int ri,cchar *aname)
{
	int		ki ;
	int		f ;

	ki = op->rectab[ri][0] ;

#if	CF_DEBUGS
	debugprintf("mailalias_keymatch: ri=%u ki=%u aname=%s key=%s\n",
	    ri,ki,aname,(op->skey + ki)) ;
#endif


	f = (strcmp((op->skey + ki),aname) == 0) ;

	return f ;
}
/* end subroutine (mailalias_keymatch) */

#endif /* (! CF_FASTKEYMATCH) */


static int mailalias_dbopen(MAILALIAS *op,time_t dt)
{
	int		rs ;
	int		f_create = FALSE ;

	if ((rs = mailalias_dbopenfile(op,dt)) >= 0) {
	    f_create = rs ;
#if	CF_DEBUGS
	    debugprintf("mailalias_dbopen: open-out rs=%d\n",rs) ;
	    debugprintf("mailalias_dbopen: f_ocreate=%u\n",op->f.ocreate) ;
#endif
	    if ((rs = mailalias_dbopenmake(op,dt)) >= 0) {
	        if ((rs = mailalias_isremote(op)) >= 0) {
	            if ((rs = mailalias_dbopenwait(op)) >= 0) {
	                if ((rs = mailalias_mapbegin(op,dt)) >= 0) {
	                    if ((rs = mailalias_hdrload(op)) >= 0) {
	                        op->ti_access = dt ;
	                    }
	                    if (rs < 0)
	                        mailalias_mapend(op) ;
	                } /* end if (mailalias_mapbegin) */
	            } /* end if (mailalias_dbopenwait) */
	        } /* end if (mailalias_isremote) */
	    } /* end if (mailalias_dbopenmake) */
	    if (rs < 0) {
	        mailalias_fileclose(op) ;
	    }
	} /* end if (mailalias_dbopenfile) */

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: ret rs=%d f_create=%u\n",
	    rs,f_create) ;
#endif

	return (rs >= 0) ? f_create : rs ;
}
/* end subroutine (mailalias_dbopen) */


static int mailalias_dbopenfile(MAILALIAS *op,time_t dt)
{
	int		rs ;
	int		of = op->oflags ;
	int		f_create = FALSE ;
	of &= (~ O_CREAT) ;
	of &= (~ O_RDWR) ;
	of &= (~ O_WRONLY) ;
	of |= O_RDONLY ;
#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("mailalias_dbopenfile: ent of=%s\n",obuf) ;
	}
#endif /* CF_DEBUGS */

	if ((rs = u_open(op->dbfname,of,op->operm)) >= 0) {
	    USTAT	sb ;
	    op->fd = rs ;

	    if ((rs = u_fstat(op->fd,&sb)) >= 0) {

	        op->fi.mtime = sb.st_mtime ;
	        op->fi.size = sb.st_size ;
	        op->fi.ino = sb.st_ino ;
	        op->fi.dev = sb.st_dev ;

	        op->ti_open = dt ;

	        if (op->f.ocreate && op->f.owrite) {
	            f_create = (sb.st_size == 0) ;
	            if (! f_create) {
	                if ((rs = mailalias_fileold(op,dt)) > 0) {
	                    f_create = TRUE ;
	                }
	            } /* end if */
	            if (f_create) {
	                mailalias_fileclose(op) ;
	            }
	        } /* end if (we can possibly create the DB) */

	    } /* end if (stat) */

	    if (rs < 0) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	    f_create = TRUE ;
	    op->f.needcreate = TRUE ;
	} /* end if (successful file open) */

	if ((rs >= 0) && f_create) {
	    op->f.needcreate = TRUE ;
	}

	return (rs >= 0) ? f_create : rs ;
}
/* end subroutine (mailalias_dbopenfile) */


static int mailalias_dbopenmake(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	if (op->f.ocreate && op->f.owrite && op->f.needcreate) {
	    if ((rs = mailalias_dbmake(op,dt)) >= 0) {
	        const int	of = O_RDONLY ;
	        if ((rs = u_open(op->dbfname,of,op->operm)) >= 0) {
	            USTAT	sb ;
	            op->fd = rs ;
	            if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	                op->fi.mtime = sb.st_mtime ;
	                op->fi.size = sb.st_size ;
	                op->fi.ino = sb.st_ino ;
	                op->fi.dev = sb.st_dev ;
	                op->ti_open = dt ;
	            }
	            if (rs < 0) {
	                u_close(op->fd) ;
	                op->fd = -1 ;
	            }
	        } /* end if (open) */
	    } /* end if */
	} /* end if (created or re-created the file) */
	return rs ;
}
/* end subroutine (mailalias_dbopenmake) */


static int mailalias_dbopenwait(MAILALIAS *op)
{
	USTAT		sb ;
	const int	to = TO_FILECOME ;
	int		rs = SR_OK ;
	int		msize ; /* minimum size */
	int		i ;

	msize = (MAILALIAS_TOPLEN) ;

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: rs=%d filesize=%d headersize=%d\n",
	    rs,sb.st_size,msize) ;
#endif

/* wait for the file to come in if it is not yet available */

	if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	    op->fi.size = sb.st_size ;
	    for (i = 0 ; (i < to) && (sb.st_size < msize) ; i += 1) {
	        if (i > 0) sleep(10) ;
	        rs = u_fstat(op->fd,&sb) ;
	        op->fi.size = sb.st_size ;
	        if (rs < 0) break ;
	    } /* end while */
	    if (rs >= 0) {
	        if (i < to) {
	            op->fi.size = sb.st_size ;
	            op->fi.mtime = sb.st_mtime ;
	        } else {
	            rs = SR_TIMEDOUT ;
	        }
	    }
	} /* end if (u_fstat) */
	return rs ;
}
/* end subroutine (mailalias_dbopenwait) */


static int mailalias_isremote(MAILALIAS *op)
{
	int		rs ;
	if ((rs = isfsremote(op->fd)) > 0) {
	    op->f.remote = TRUE ;
	}
	return rs ;
}
/* end subroutine (mailalias_isremote) */


static int mailalias_dbclose(MAILALIAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = mailalias_mapend(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mailalias_fileclose(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailalias_dbclose) */


/* make the index file */
static int mailalias_dbmake(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*cp ;

	if (dt == 0) dt = time(NULL) ;

/* get the directory of the DB file and see if it is writable to us */

	if ((cl = sfdirname(op->dbfname,-1,&cp)) > 0) {
	    char	dbuf[MAXPATHLEN + 1] ;
	    if ((rs = mkpath1w(dbuf,cp,cl)) >= 0) {
	        USTAT	sb ;
	        if ((rs = u_stat(dbuf,&sb)) >= 0) {
	            if (! S_ISDIR(sb.st_mode)) rs = SR_NOTDIR ;
	            if (rs >= 0) {
	                rs = sperm(&op->id,&sb,W_OK) ;
	            }
	        } else if (rs == SR_NOENT) {
	            const mode_t	dm = MAILALIAS_DIRMODE ;
	            rs = mkdirs(dbuf,dm) ;
	        }
	        if (rs >= 0) {
	            rs = mailalias_dbmaker(op,dt,dbuf) ;
	        }
	    }
	} else {
	    rs = SR_ISDIR ;
	} /* end if */
	return rs ;
}
/* end subroutine (mailalias_dbmake) */


static int mailalias_dbmaker(MAILALIAS *op,time_t dt,cchar *dname)
{
	const int	size = MAILALIAS_TOPLEN ;
	const int	clen = MAXNAMELEN ;
	int		n = NREC_GUESS ;
	int		rs ;
	int		rs1 ;
	cchar		*suf = MAILALIAS_FE ;
	cchar		*end = ENDIANSTR ;
	char		cbuf[MAXNAMELEN+1] ;
	if ((rs = sncpy5(cbuf,clen,"dbmkXXXXXX",".",suf,end,"n")) >= 0) {
	    char	tbuf[MAXPATHLEN+1] ;
	    if ((rs = mkpath2(tbuf,dname,cbuf)) >= 0) {
	        int	of = (O_WRONLY | O_CREAT | O_EXCL) ;
	        int	fd = -1 ;
	        char	nfname[MAXPATHLEN+1] ;
	        if ((rs = opentmpfile(tbuf,of,0664,nfname)) >= 0) {
	            fd = rs ;
	        } else if (rs == SR_EXIST) {
	            USTAT	sb ;
	            int		i ;
	            for (i = 0 ; i < TO_FILECOME ; i += 1) {
	                sleep(1) ;
	                rs1 = u_stat(op->dbfname,&sb) ;
	                if ((rs1 >= 0) && (sb.st_size >= size)) break ;
	            } /* end for */
	            rs = (i < TO_FILECOME) ? SR_OK : SR_TIMEDOUT ;
	            if (rs == SR_TIMEDOUT) {
	                of = (O_WRONLY | O_CREAT) ;
	                rs = u_open(nfname,of,0664) ;
	                if (rs == SR_ACCESS) {
	                    u_unlink(nfname) ;
	                    rs = u_open(nfname,of,0664) ;
	                }
	                fd = rs ;
	            } /* end if (attempt to break the lock) */
	        } /* end if (waiting for file to come in) */
	        if ((rs >= 0) && (fd >= 0)) {
	            if ((rs = mailalias_dbmaking(op,fd,dt,n)) >= 0) {
	                u_close(fd) ;
	                fd = -1 ;
	                if ((rs = u_rename(nfname,op->dbfname)) >= 0) {
	                    nfname[0] = '\0' ;
	                }
	            }
	            if (rs < 0) {
	                if (fd >= 0) {
	                    u_close(fd) ;
	                    fd = -1 ;
	                }
	                nfname[0] = '\0' ;
	            }
	        } else {
	            nfname[0] = '\0' ;
	        }
	        if (nfname[0] != '\0') {
	            u_unlink(nfname) ;
	        }
	    } /* end if (mkpath2) */
	} /* end if (sncpy5) */
	return rs ;
}
/* end subroutine (mailalias_dbmaker) */


static int mailalias_dbmaking(MAILALIAS *op,int fd,time_t dt,int n)
{
	const int	size = sizeof(RECORD) ;
	int		rs ;
	int		rs1 ;
	VECOBJ		recs ;
	if ((rs = vecobj_start(&recs,size,n,0)) >= 0) {
	    STRTAB	skeys ;
	    if ((rs = strtab_start(&skeys,n)) >= 0) {
	        STRTAB	svals ;
	        if ((rs = strtab_start(&svals,(n * 2))) >= 0) {
	            if ((rs = mailalias_aprofile(op,dt)) >= 0) {
	                DBMAKE	data(&recs,&skeys,&svals,fd) ;
	                int	i ;
	                cchar	*cp ;
	                char	tmpfname[MAXPATHLEN+1] ;
	                for (i = 0 ; op->aprofile[i] != NULL ; i += 1) {
	                    cp = (cchar *) op->aprofile[i] ;
	                    if (*cp != '/') {
	                        cchar	*ap = op->aprofile[i] ;
	                        cp = tmpfname ;
	                        rs = mkpath2(tmpfname,op->pr,ap) ;
	                    }
#if	CF_DEBUGS
	                    debugprintf("mailalias_dbmake: "
	                        "proc file=%s\n",cp) ;
#endif
	                    if (rs >= 0) {
	                        rs = mailalias_procfile(op,&data,cp) ;
	                    }
	                    if (rs < 0) break ;
	                } /* end for */
/* where are we? */
#if	CF_DEBUGS
	                debugprintf("mailalias_dbmake: "
	                    "record count=%d\n",n) ;
#endif
/* OK, write out the file */
	                if (rs >= 0) {
	                    if ((rs = mailalias_wrfile(op,&data,dt)) >= 0) {
	                        u_close(fd) ;
	                        fd = -1 ;
	                    } /* end if */
	                } /* end if (ok) */
	            } /* end if (mailalias_aprofile) */
	            rs1 = strtab_finish(&svals) ;
	            if (rs >= 0) rs = rs1 ;
	        } /* end if (strtab_start) */
	        rs1 = strtab_finish(&skeys) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (strtab_start) */
	    rs1 = vecobj_finish(&recs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (vecobj-recs) */
	return rs ;
}
/* end subroutine (mailalias_dbmaking) */


static int mailalias_procfile(MAILALIAS *op,DBMAKE *dp,cchar *fname)
{
	bfile		afile ;
	int		rs ;
	int		rs1 ;
	int		c_rec = 0 ; /* ¥ GCC false complaint */

	if (op == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mailalias_procfile: ent fname=%s\n",fname) ;
#endif

	if ((rs = bopen(&afile,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    dp->f_havekey = FALSE ;
	    c_rec = 0 ;
	    dp->c = 0 ;
	    while ((rs = breadline(&afile,lbuf,llen)) > 0) {
	        int	len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;
	        if (f_eol) len -= 1 ;

	        if ((len > 0) && f_bol && f_eol) {
	            if (lbuf[0] != '#') {
	                if (! CHAR_ISWHITE(lbuf[0])) {
	                    dp->c = 0 ;
	                    dp->f_havekey = FALSE ;
	                }
	                rs = mailalias_procfileline(op,dp,lbuf,len) ;
	                c_rec += rs ;
	            }
	        } /* end if (BOL and EOL) */

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading extended lines) */

#if	CF_DEBUGS
	    debugprintf("mailalias_procfile: while-out rs=%d\n",rs) ;
#endif

#if	CF_DEBUGS && CF_DEBUGSFILE
	    {
	        RECORD	*rep ;
	        int	i ;
	        debugprintf("mailalias_procfile: record dump:\n") ;
	        for (i = 0 ; vecobj_get(dp->rlp,i,&rep) >= 0 ; i += 1) {
	            if (rep != NULL) {
	                debugprintf("mailalias_procfile: k=%u v=%u\n",
	                    rep->key,rep->val) ;
#ifdef	COMMENT
	                rs1 = strtab_get(dp->klp,rep->key,&cp) ;
	                debugprintf("mailalias_procfile: key=%s \n",cp) ;
	                rs1 = strtab_get(dp->vlp,rep->val,&cp) ;
	                debugprintf("mailalias_procfile: val=%s \n",cp) ;
#endif /* COMMENT */
	            }
	        } /* end for */
	    }
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	    debugprintf("mailalias_procfile: mid-bclose rs=%d\n",rs) ;
#endif

	    rs1 = bclose(&afile) ;
	    if (rs >= 0) rs = rs1 ;
	} else if (isNotPresent(rs)) {
#if	CF_DEBUGS
	    debugprintf("mailalias_procfile: not-found\n") ;
#endif
	    rs = SR_OK ;
	} /* end if (afile) */

#if	CF_DEBUGS
	debugprintf("mailalias_procfile: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c_rec : rs ;
}
/* end subroutine (mailalias_procfile) */


static int mailalias_procfileline(MAILALIAS *op,DBMAKE *dp,
		cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		c_rec = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    STRTAB	*skp = dp->klp ;
	    STRTAB	*svp = dp->vlp ;
	    vecobj	*rlp = dp->rlp ;
	    const int	rsn = SR_NOTFOUND ;
	    char	kbuf[ALIASNAMELEN+1] = { 0 } ;
	    dp->c = 0 ;
	    if (! dp->f_havekey) {
	        int	kl ;
	        cchar	*kp ;
	        if ((kl = field_get(&fsb,kterms,&kp)) >= 0) {
	            if (kl > 0) {
	                int	f ;
	                cchar	*pm = "Postmaster" ;
	                dp->f_havekey = TRUE ;
	                dp->c = 0 ;
	                f = (kl == 10) && (strncasecmp(pm,kp,kl) == 0) ;
	                if (f) {
	                    strwcpylc(kbuf,kp,MIN(kl,ALIASNAMELEN)) ;
	                } else {
	                    strwcpy(kbuf,kp,MIN(kl,ALIASNAMELEN)) ;
	                }
	            } /* end if (positive) */
	        } /* end if (field_get) */
	    } /* end if (didn't have a key already) */
	    if (dp->f_havekey && (fsb.term != '#')) {
	        int	vl ;
	        cchar	*vp ;
	        while ((vl = field_get(&fsb,vterms,&vp)) >= 0) {
	            if (vl > 0) {
	                int	ival = 0 ;
	                if (dp->c == 0) { /* enter into key-string table */
	                    if ((rs = strtab_already(skp,kbuf,-1)) == rsn) {
	                        rs = strtab_add(skp,kbuf,-1) ;
	                        dp->ikey = rs ;
	                    } else {
	                        dp->ikey = rs ;
	                    }
	                } /* end if (entering key) */
/* enter value into string table */
	                if (rs >= 0) { /* enter into val-string table */
	                    if ((rs = strtab_already(svp,vp,vl)) == rsn) {
	                        rs = strtab_add(svp,vp,vl) ;
	                        ival = rs ;
	                    } else if (rs >= 0) {
	                        ival = rs ;
	                    }
/* enter record */
	                    if ((rs >= 0) && (ival > 0)) {
	                        RECORD	re ;
	                        re.key = dp->ikey ;
	                        re.val = ival ;
	                        if ((rs = vecobj_find(rlp,&re)) == rsn) {
	                            rs = vecobj_add(rlp,&re) ;
	                        }
	                        if (rs >= 0) {
	                            dp->c += 1 ;
	                            dp->nrecs += 1 ;
	                            c_rec += 1 ;
	                        }
	                    } /* end if (new entry) */
	                } /* end if (ok) */
	                if (fsb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while (fields) */
	        } /* end if (retrieved key) */
	    } /* end if (have key) */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? c_rec : rs ;
}
/* end subroutine (mailalias_procfileline) */


/* write out the cache file */
static int mailalias_wrfile(MAILALIAS *op,DBMAKE *dp,time_t dt)
{
	int		rs ;

	if ((rs = strtab_count(dp->klp)) >= 0) {
	    dp->ktlen = (rs+1) ;
	    dp->ktsize = (dp->ktlen + 1) * sizeof(int) ;
	    if ((rs = vecobj_count(dp->rlp)) >= 0) {
		dp->reclen = (rs + 1) ;
		dp->recsize = ((dp->reclen+1) * (2 * sizeof(int))) ;
		dp->rilen = nextpowtwo(dp->reclen) ;
		if (dp->rilen < 4) dp->rilen = 4 ;
		dp->risize = (dp->rilen * 2 * sizeof(int)) ;
		if ((rs = strtab_strsize(dp->klp)) >= 0) {
		    dp->sksize = rs ;
		    if ((rs = strtab_strsize(dp->vlp)) >= 0) {
			dp->svsize = rs ;
		        rs = mailalias_wrfiler(op,dp,dt) ;
		    }
		}
	    }
	}

#if	CF_DEBUGS
	debugprintf("mailalias_wrfile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailalias_wrfile) */


/* OK, we're ready to start writing stuff out! */
static int mailalias_wrfiler(MAILALIAS *op,DBMAKE *dp,time_t dt)
{
	int		rs ;
	int		header[header_overlast] ;
	int		fto = 0 ;
	int		ml ;
	char		fidbuf[MAILALIAS_IDLEN + 1] ;
	char		*bp ;

#if	CF_DEBUGS
	debugprintf("mailalias_wrfiler: ent\n") ;
#endif

/* prepare the file magic */

	bp = fidbuf ;
	ml = mkmagic(bp,MAILALIAS_FILEMAGICSIZE,MAILALIAS_FILEMAGIC) ;
	bp += ml ;

/* prepare the version and encoding (VETU) */

	fidbuf[16] = MAILALIAS_FILEVERSION ;
	fidbuf[17] = ENDIAN ;
	fidbuf[18] = op->ropts ;
	fidbuf[19] = 0 ;

/* write magic along with version encoding */

	if ((rs = u_write(dp->fd,fidbuf,MAILALIAS_IDLEN)) >= 0) {
	    const int	hsize = (header_overlast * sizeof(int)) ;
	    fto += MAILALIAS_IDLEN ;

/* make the header itself (skip over it for FTO) */

	    fto += (header_overlast * sizeof(int)) ;

/* everything else */

	    header[header_wtime] = (uint) dt ;
	    header[header_wcount] = 1 ;

	    header[header_key] = fto ;
	    header[header_keylen] = dp->ktlen ;
	    fto += dp->ktsize ;

	    header[header_rec] = fto ;
	    header[header_reclen] = dp->reclen ;
	    fto += dp->recsize ;

	    header[header_ind] = fto ;
	    header[header_indlen] = dp->rilen ;
	    fto += dp->risize ;

	    header[header_skey] = fto ;
	    header[header_skeysize] = dp->sksize ;
	    fto += dp->sksize ;

	    header[header_sval] = fto ;
	    header[header_svalsize] = dp->svsize ;
	    fto += dp->svsize ;

/* write out the header */

#if	CF_DEBUGS
	    {
	    int	i ;
	    for (i = 0 ; i < header_overlast ; i += 1)
	        debugprintf("mailalis_wrfiler: header[%2u]=%08x\n",
			i,header[i]) ;
	    }
#endif /* CF_DEBUGS */

	    if ((rs = u_write(dp->fd,header,hsize)) >= 0) {
		if ((rs = mailalias_wrfilekeytab(op,dp)) >= 0) {
	            if ((rs = mailalias_wrfilerec(op,dp)) >= 0) {
	        	if ((rs = mailalias_wrfilekeys(op,dp)) >= 0) {
	        	    rs = mailalias_wrfilevals(op,dp) ;
			}
		    }
		}
	    } /* end if (ok) */

	} /* end if (write of id-section) */

#if	CF_DEBUGS
	debugprintf("mailalias_wrfiler: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailalias_wrfiler) */


/* write the key-table */
static int mailalias_wrfilekeytab(MAILALIAS *op,DBMAKE *dp)
{
	const int	ktsize = dp->ktsize ;
	int		rs ;
	int		*keytab ;
	if ((rs = uc_malloc(ktsize,&keytab)) >= 0) {
	    if ((rs = strtab_recmk(dp->klp,keytab,ktsize)) >= 0) {
		rs = u_write(dp->fd,keytab,ktsize) ;
	    }
	    uc_free(keytab) ;
	} /* end if */
	return rs ;
}
/* end subroutine (mailalias_wrfilekeytab) */


/* write the record table */
static int mailalias_wrfilerec(MAILALIAS *op,DBMAKE *dp)
{
	const int	recsize = dp->recsize ;
	int		rs ;
	int		(*rectab)[2] ;
	if ((rs = uc_malloc(recsize,&rectab)) >= 0) {
	    RECORD	*rep ;
	    int		ri = 0 ;
	    int		i ;
	    rectab[ri][0] = 0 ;
	    rectab[ri][1] = 0 ;
	    ri += 1 ;
	    for (i = 0 ; vecobj_get(dp->rlp,i,&rep) >= 0 ; i += 1) {
	        rectab[ri][0] = rep->key ;
	        rectab[ri][1] = rep->val ;
	        ri += 1 ;
	    } /* end for */
	    rectab[ri][0] = -1 ;
	    rectab[ri][1] = 0 ;
	    rs = u_write(dp->fd,rectab,recsize) ;
	    uc_free(rectab) ;
	} /* end if (record table) */
	return rs ;
}
/* end subroutine (mailalias_wrfilerec) */


/* make the index table and the key-string table */
static int mailalias_wrfilekeys(MAILALIAS *op,DBMAKE *dp)
{
	const int	risize = dp->risize ;
	int		rs ;
	int		(*indtab)[2] ;
	if ((rs = uc_malloc(risize,&indtab)) >= 0) {
	    const int	sksize = dp->sksize ;
	    char	*kstab ;
	    if ((rs = uc_malloc(sksize,&kstab)) >= 0) {
	        if ((rs = strtab_strmk(dp->klp,kstab,sksize)) >= 0) {
	            VECOBJ	*rlp = dp->rlp ;
	            int		(*it)[2] = indtab ;
	            const int	ris = risize ;
	            if ((rs = mailalias_mkind(op,rlp,kstab,it,ris)) >= 0) {
	                if ((rs = u_write(dp->fd,indtab,risize)) >= 0) {
	                    rs = u_write(dp->fd,kstab,sksize) ;
	                }
	            }
	        }
	        uc_free(kstab) ;
	    } /* end if (memory allocation) */
	    uc_free(indtab) ;
	} /* end if (memory allocation) */
	return rs ;
}
/* end subroutine (mailalias_wrfilekeys) */


/* make the value-string table */
static int mailalias_wrfilevals(MAILALIAS *op,DBMAKE *dp)
{
	const int	svsize = dp->svsize ;
	int		rs ;
	char		*vstab ;
	if ((rs = uc_malloc(svsize,&vstab)) >= 0) {
	    if ((rs = strtab_strmk(dp->vlp,vstab,svsize)) >= 0) {
	        rs = u_write(dp->fd,vstab,svsize) ;
	    }
	    uc_free(vstab) ;
	} /* end if (value string-table) */
	return rs ;
}
/* end subroutine (mailalias_wrfilevals) */


/* make the (only) index for this file */
static int mailalias_mkind(MAILALIAS *op,VECOBJ *rp,cchar skey[],
		int it[][2],int itsize)
{
#if	CF_DEBUGSSHIFT
	int		ns = (nshift > 0) ? nshift : NSHIFT ;
#else
	const int	ns = NSHIFT ;
#endif
	int		rs ;
	int		size ;
	int		n = 0 ; /* ¥ GCC false complaint */

	if (it == NULL) return SR_FAULT ;

	if ((rs = vecobj_count(rp)) >= 0) {
	    n = nextpowtwo(rs) ;
	    if (n < 4) n = 4 ;

	    size = (n * 2 * sizeof(uint)) ;

#if	CF_DEBUGS
	    debugprintf("mailalias_mkind: size calc=%u given=%u\n",
	        size,itsize) ;
#endif

	    if (size <= itsize) {
	        RECORD		*rep ;
	        uint		khash ;
	        int		i, ri, hi, ki ;
	        int		v, c ;
	        int		kl ;
	        int		f ;
	        const char	*kp ;

#if	CF_DEBUGS
	        {
	            debugprintf("mailalias_mkind: key string dump¬\n") ;
	            kp = (skey + 1) ;
	            while (kp[0] != '\0') {
	                kl = strlen(kp) ;
	                debugprintf("mailalias_mkind: sl=%u skey=%s\n",kl,kp) ;
	                kp += (kl + 1) ;
	            }
	            debugprintf("mailalias_mkind: end dump\n") ;
	        }
#endif /* CF_DEBUGS */

	        memset(it,0,size) ;
	        ri = 1 ;
	        for (i = 0 ; vecobj_get(rp,i,&rep) >= 0 ; i += 1) {

#if	CF_DEBUGS
	            debugprintf("mailalias_mkind: rec ri=%u k=%u\n",
	                ri,rep->key) ;
#endif

	            kp = (const char *) (skey + rep->key) ;
	            kl = strlen(kp) ;

#if	CF_DEBUGS
	            debugprintf("mailalias_mkind: kl=%d key=%s\n",kl,kp) ;
#endif

	            khash = hashelf(kp,kl) ;

	            hi = hashindex(khash,n) ;

#if	CF_DEBUGS
	            debugprintf("mailalias_mkind: khash=%08x hi=%u\n",
	                khash,hi) ;
#endif

	            c = 0 ;
	            if ((op->ropts & MAILALIAS_OSEC) && (it[hi][0] != 0)) {

#if	CF_DEBUGS
	                debugprintf("mailalias_mkind: collision ri=%d\n",ri) ;
#endif

	                while ((v = it[hi][0]) != 0) {

	                    ki = op->rectab[v][0] ;
	                    f = (strcmp(kp,(op->skey + ki)) == 0) ;

	                    if (! f) break ;

	                    if (op->ropts & MAILALIAS_ORANDLC) {
	                        khash = randlc(khash + c) ;
	                    } else {
	                        khash = ((khash<<(32-ns))|(khash>>ns))+c ;
	                    }

	                    hi = hashindex(khash,n) ;

#if	CF_DEBUGS
	                    debugprintf("mailalias_mkind: "
	                        "new khash=%08x hi=%u\n", khash,hi) ;
#endif

	                    c += 1 ;

	                } /* end while */

	            } /* end if (secondary hash on collision) */

	            if (it[hi][0] != 0) {
	                int	lhi ;

#if	CF_DEBUGS
	                debugprintf("mailalias_mkind: collision same key\n") ;
#endif

	                c += 1 ;
	                while (it[hi][1] != 0) {
	                    c += 1 ;
	                    hi = it[hi][1] ;
	                }

	                lhi = hi ;		/* save last hash-index value */
	                hi = hashindex((hi + 1),n) ;

	                while (it[hi][0] != 0) {
	                    c += 1 ;
	                    hi = hashindex((hi + 1),n) ;
	                } /* end while */

	                it[lhi][1] = hi ;	/* update the previous slot */

	            } /* end if (got a hash collision) */

	            it[hi][0] = ri ;
	            it[hi][1] = 0 ;

#if	CF_DEBUGS
	            debugprintf("mailalias_mkind: ri=%u hi=%u\n",ri,hi) ;
#endif

#ifdef	COMMENT
	            op->s.c_l1 += c ;
	            recorder_cden(asp,wi,c) ;
#endif

	            ri += 1 ;

	        } /* end for (looping through records) */

	    } else {
	        rs = SR_OVERFLOW ;
	    }

	} /* end if (vecobj_count) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mailalias_mkind) */


static int mailalias_filechanged(MAILALIAS *op,USTAT *sbp)
{
	int		f = FALSE ;
	f = f || (op->fi.size != sbp->st_size) ;
	f = f || (op->fi.mtime != sbp->st_mtime) ;
	f = f || (op->fi.ino != sbp->st_ino) ;
	f = f || (op->fi.dev != sbp->st_dev) ;
	return f ;
}
/* end subroutine (mailalias_filechanged) */


/* return TRUE if the underlying file(s) are "old" */
static int mailalias_fileold(MAILALIAS *op,time_t dt)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = mailalias_aprofile(op,dt)) >= 0) {
	    USTAT	sb ;
	    int		i ;
	    cchar	*cp ;
	    char	tmpfname[MAXPATHLEN + 1] ;
	    for (i = 0 ; op->aprofile[i] != NULL ; i += 1) {
	        cp = (const char *) op->aprofile[i] ;
	        if (*cp != '/') {
	            cp = tmpfname ;
	            mkpath2(tmpfname,op->pr,op->aprofile[i]) ;
	        }
	        rs1 = u_stat(cp,&sb) ;
	        if ((rs1 >= 0) && (sb.st_mtime > op->fi.mtime)) break ;
	    } /* end for */
	    f = (op->aprofile[i] != NULL) ? 1 : 0 ;
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_fileold) */


/* get apfiles */
static int mailalias_aprofile(MAILALIAS *op,time_t dt)
{
	struct ustat	sb ;
	kvsfile		aptab ;
	const int	tlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	const char	*cp ;
	char		tbuf[MAXPATHLEN + 1] ;

	if ((dt - op->ti_aprofile) <= TO_APROFILE)
	    return SR_OK ;

	op->ti_aprofile = dt ;

/* get the mailalias profile (AP) table */

	rs1 = SR_NOENT ;
	for (i = 0 ; aptabsched[i] != NULL ; i += 1) {

	    cp = (const char *) aptabsched[i] ;
	    if (*cp != '/') {
	        cp = tbuf ;
	        mkpath2(tbuf,op->pr,aptabsched[i]) ;
	    }

#if	CF_DEBUGS
	    debugprintf("mailalias_aprofile: aptab=%s\n",cp) ;
#endif

	    rs1 = u_stat(cp,&sb) ;

	    if ((rs1 >= 0) && S_ISDIR(sb.st_mode))
	        rs1 = SR_ISDIR ;

	    if (rs1 >= 0)
	        rs1 = sperm(&op->id,&sb,R_OK) ;

	    if (rs1 >= 0)
	        break ;

	} /* end for */

	if (aptabsched[i] != NULL) {

	    if ((rs = kvsfile_open(&aptab,0,cp)) >= 0) {
	        kvsfile_cur	cur ;

	        for (i = 0 ; vecstr_get(&op->apfiles,i,&cp) >= 0 ; i += 1) {
	            if (cp != NULL) {
	                vecstr_del(&op->apfiles,i--) ;
	            }
	        } /* end for */

	        if ((rs = kvsfile_curbegin(&aptab,&cur)) >= 0) {

	            while (rs >= 0) {

	                rs1 = kvsfile_fetch(&aptab,op->apname,&cur,tbuf,tlen) ;
	                if (rs1 < 0) break ;

#if	CF_DEBUGS
	                debugprintf("mailalias_aprofile: "
			    "kvsfile_fetch() v=%s\n",tbuf) ;
#endif

	                rs = vecstr_add(&op->apfiles,tbuf,rs1) ;

	            } /* end while */

	            kvsfile_curend(&aptab,&cur) ;
		} /* end if (kvsfile-cur) */

	        if (rs >= 0) {
	            rs = vecstr_getvec(&op->apfiles,&op->aprofile) ;
	        }
	        kvsfile_close(&aptab) ;
	    } /* end if (opened key-values table) */

#if	CF_DEFPROFILE
	    if (op->aprofile[0] == NULL)
	        op->aprofile = defprofile ;
#endif

	} else {
	    op->aprofile = defprofile ;
	}

#if	CF_DEBUGS
	debugprintf("mailalias_aprofile: ret rs=%d enum\n",rs) ;
	if (rs >= 0) {
	    for (i = 0 ; op->aprofile[i] != NULL ; i += 1) {
	        debugprintf("mailalias_aprofile: aprofile file=%s\n",
	            op->aprofile[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (mailalias_aprofile) */


/* acquire access to the file (mapped memory) */
static int mailalias_enterbegin(MAILALIAS *op,time_t dt)
{
	int		rs ;
	int		f = FALSE ;

#if	CF_DEBUGS
	debugprintf("mailalias_enterbegin: ent\n") ;
#endif

	if ((rs = mailalias_mapcheck(op,dt)) > 0) {
	    f = TRUE ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_enterbegin) */


/* release our hold on the filemap */
/* ARGSUSED */
static int mailalias_enterend(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;

	if (op->f.held) {
	    op->f.held = FALSE ;
	}

	return rs ;
}
/* end subroutine (mailalias_enterend) */


static int mailalias_mapcheck(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		f = FALSE ;
	if (op->mapdata == NULL) {
	    if ((rs = mailalias_fileopen(op,dt)) >= 0) {
	        if ((rs = mailalias_mapbegin(op,dt)) > 0) {
	            f = TRUE ;
	            rs = mailalias_hdrload(op) ;
	            if (rs < 0) {
	                mailalias_mapend(op) ;
	            }
	        } /* end if (mailalias_mapbegin) */
		rs1 = mailalias_fileclose(op) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (mailalias_fileopen) */
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_mapcheck) */


/* calculate the next hash table index from a given one */
static int hashindex(uint i,int n)
{
	int		hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end if (hashindex) */


