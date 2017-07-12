/* mailalias */

/* manage a MAILALIAS object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGSFILE	0		/* file parsing */
#define	CF_DEBUGSSHIFT	0
#define	CF_SAFE		1		/* safe mode */
#define	CF_HOLDING	1		/* use file-map holding */
#define	CF_FASTKEYMATCH	1		/* faster key matching */
#define	CF_CLOSEONEXEC	0		/* don't need it */
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
#include	<strings.h>

#include	<vsystem.h>
#include	<endian.h>
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

#define	MAILALIAS_IDLEN		(16 + 4)
#define	MAILALIAS_HEADLEN	(12 * 4)	/* check w/ 'header_' above */
#define	MAILALIAS_TOPLEN	(MAILALIAS_IDLEN + MAILALIAS_HEADLEN)

#define	MAILALIAS_IDOFF		0
#define	MAILALIAS_HEADOFF	MAILALIAS_IDLEN
#define	MAILALIAS_BUFOFF	(MAILALIAS_HEADOFF + MAILALIAS_HEADLEN)

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	TO_APROFILE	(1 * 60)
#define	TO_FILECOME	15		/* timeout for file to "come in" */
#define	TO_LOCK		(5 * 60)
#define	TO_OPEN		(60 * 60)
#define	TO_ACCESS	(1 * 60)
#define	TO_CHECK	5		/* minimum check interval */
#define	TO_FILECHANGED	5		/* DB file check */
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

extern uint	nextpowtwo(uint) ;
extern uint	uceil(uint,int) ;
extern uint	ulceil(ulong,int) ;
extern uint	hashelf(const char *,int) ;

extern int	snopenflags(char *,int,int) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	mkfnamesuf2(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	randlc(int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	isfsremote(int) ;


#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* local structures */

struct dbmake {
	VECOBJ		recs ;
	STRTAB		skeys ;
	STRTAB		svals ;
	uint		nrecs ;
	uint		nkeys ;
	int		fd ;
} ;

struct record {
	uint		key ;
	uint		val ;
} ;


/* forward references */

static int	mailalias_fileheader(MAILALIAS *) ;
static int	mailalias_holdget(MAILALIAS *,time_t) ;
static int	mailalias_holdrelease(MAILALIAS *,time_t) ;

static int	mailalias_fileopen(MAILALIAS *,time_t) ;
static int	mailalias_fileclose(MAILALIAS *) ;
static int	mailalias_filechanged(MAILALIAS *,struct ustat *) ;
static int	mailalias_mapcreate(MAILALIAS *,time_t dt) ;
static int	mailalias_mapdestroy(MAILALIAS *) ;

#if	(! CF_FASTKEYMATCH)
static int	mailalias_keymatch(MAILALIAS *,int,int,const char *) ;
#endif

static int	mailalias_dbopen(MAILALIAS *,time_t) ;
static int	mailalias_dbclose(MAILALIAS *) ;
static int	mailalias_dbmake(MAILALIAS *,time_t) ;
static int	mailalias_procfile(MAILALIAS *,struct dbmake *,const char *) ;
static int	mailalias_writefile(MAILALIAS *,struct dbmake *, time_t) ;
static int	mailalias_mkind(MAILALIAS *,VECOBJ *,
			const char *,int (*)[2],int) ;
static int	mailalias_fileold(MAILALIAS *,time_t) ;
static int	mailalias_aprofile(MAILALIAS *,time_t) ;

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
	            const char	*fe = MAILALIAS_FE ;
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

	op->cursors -= 1 ;

#if	CF_HOLDING
	if (op->f.held)
	    rs = mailalias_holdrelease(op,dt) ;
#endif

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
	int		rs = SR_OK ;
	int		ri = 0 ;
	int		cl ;
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

#if	CF_HOLDING
	if (! op->f.held) {
	    dt = time(NULL) ;
	    rs = mailalias_holdget(op,dt) ;
	    if (rs < 0)
	        goto ret0 ;
	}
#endif /* CF_HOLDING */

/* ok, we're good to go */

	if (ri >= op->rtlen) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

/* go */

	if (rs >= 0) {
	    uint	ai, vi ;

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
	            if (vlen >= 0)
	                cl = MIN(vlen,MAILADDRLEN) ;

	            strwcpy(vbuf,(op->sval + vi),cl) ;

	        } /* end if (value buffer present) */

/* update the cursor */

	        curp->i = ri ;

	    } else {
	        rs = SR_BADFMT ;
	    }

	} /* end if */

ret0:
	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (mailalias_enum) */


/* fetch an entry by key lookup */
/* ARGSUSED */
int mailalias_fetch(MAILALIAS *op,int opts,cchar aname[],MAILALIAS_CUR *curp,
		char *vbuf,int vlen)
{
	MAILALIAS_CUR	cur ;
	time_t		dt = 0 ;
	uint		khash ;
	const int	ns = NSHIFT ;
	int		rs = SR_OK ;
	int		vi, hi, ri, ki ;
	int		cl, hl, c, n ;
	int		vl = 0 ; /* ¥ GCC false complaint */
	int		f_cur = FALSE ;
	int		f ;
	const char	*hp ;
	const char	*cp ;

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
	        goto ret0 ;
	    }
	} /* end if */

/* do we have a hold on the file? */

#if	CF_HOLDING
	if (! op->f.held) {
	    dt = time(NULL) ;
	    rs = mailalias_holdget(op,dt) ;
	    if (rs < 0)
	        goto ret0 ;

	}
#endif /* CF_HOLDING */

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
	    debugprintf("mailalias_fetch: khash=%08x hi=%d\n",khash,hi) ;
#endif

/* start searching! */

	    if (op->ropts & MAILALIAS_OSEC) {
	        int	f ;

#if	CF_DEBUGS
	        debugprintf("mailalias_fetch: secondary initial ri=%d\n",
	            (op->indtab)[hi][0]) ;
#endif

	        c = 0 ;
	        while ((ri = (op->indtab)[hi][0]) != 0) {

#if	CF_DEBUGS
	            debugprintf("mailalias_fetch: trymatch ri=%d\n",ri) ;
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
	                khash = ((khash << (32 - ns)) | (khash >> ns)) + c ;
		    }

	            hi = hashindex(khash,n) ;

#if	CF_DEBUGS
	            debugprintf("mailalias_fetch: new khash=%08x hi=%d\n",
	                khash,hi) ;
#endif

	            c += 1 ;

	        } /* end while */

#if	CF_DEBUGS
	        debugprintf("mailalias_fetch: index-key-match ri=%d\n",ri) ;
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
	debugprintf("mailalias_fetch: match search rs=%d hi=%d\n",rs,hi) ;
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
	debugprintf("mailalias_fetch: searched rs=%d ri=%u hi=%u\n",rs,ri,hi) ;
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
	        debugprintf("mailalias_fetch: cursor update hi=%u\n",hi) ;
#endif
	        curp->i = hi ;
	    }

	} /* end if (got one) */

#if	CF_HOLDING
	if (op->cursors == 0) {
	    if (dt == 0) dt = time(NULL) ;
	    op->ti_access = dt ;
	    rs = mailalias_holdrelease(op,dt) ;

#if	CF_DEBUGS
	    debugprintf("mailalias_fetch: mailalias_holdrelease() rs=%d\n",rs) ;
#endif

	}
#endif /* CF_HOLDING */

ret0:

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
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_changed = FALSE ;

#if	CF_DEBUGS
	char		timebuf[TIMEBUFLEN + 1] ;
#endif

	if (op == NULL) return SR_FAULT ;

	if (op->magic != MAILALIAS_MAGIC) return SR_NOTOPEN ;

	if (op->f.held || (op->mapdata == NULL))
	    return SR_OK ;

#if	CF_DEBUGS
	debugprintf("mailalias_check: %s\n",
	    timestr_log(dt,timebuf)) ;
#endif

	if ((dt - op->ti_access) > TO_ACCESS)
	    goto closeit ;

	if ((dt - op->ti_open) > TO_OPEN)
	    goto closeit ;

	if (dt == 0) dt = time(NULL) ;

/* do not check more rapidly than TO_CHECK time */

	if ((dt - op->ti_check) > TO_CHECK) {

	    op->ti_check = dt ;

/* has the file changed? */

	    if ((! f_changed) &&
	        ((dt - op->ti_filechanged) > TO_FILECHANGED)) {

	        op->ti_filechanged = dt ;
	        if ((rs1 = u_stat(op->dbfname,&sb)) >= 0) {
	            rs1 = mailalias_filechanged(op,&sb) ;
	            f_changed = (rs1 > 0) ;
	        }

	    }

/* is the file old? */

	    if ((! f_changed) &&
	        op->f.ocreate && op->f.owrite &&
	        ((dt - op->ti_fileold) > TO_FILEOLD)) {

	        op->ti_fileold = dt ;
	        rs1 = mailalias_fileold(op,dt) ;
	        f_changed = (rs1 > 0) ;

	    }

	    if (f_changed)
	        goto closeit ;

	} /* end if (within check interval) */

ret0:
	return (rs >= 0) ? f_changed : rs ;

/* handle a close out */
closeit:
	rs = mailalias_dbclose(op) ;
	goto ret0 ;
}
/* end subroutine (mailalias_check) */


/* private subroutines */


/* read the file header and check it out */
static int mailalias_fileheader(MAILALIAS *op)
{
	uint		*table ;
	int		rs = SR_OK ;
	int		f ;
	const char	*cp ;

	cp = (const char *) op->mapdata ;
	f = (strncmp(cp,MAILALIAS_FILEMAGIC,MAILALIAS_FILEMAGICLEN) == 0) ;

	f = f && (*(cp + MAILALIAS_FILEMAGICLEN) == '\n') ;

	if (! f) {

#if	CF_DEBUGS
	    debugprintf("mailalias_fileheader: bad magic=>%t<\n",
	        cp,strnlen(cp,14)) ;
#endif

	    rs = SR_BADFMT ;
	    goto ret0 ;
	}

	cp += 16 ;
	if (cp[0] > MAILALIAS_FILEVERSION) {
	    rs = SR_NOTSUP ;
	    goto ret0 ;
	}

	if (cp[1] != ENDIAN) {
	    rs = SR_NOTSUP ;
	    goto ret0 ;
	}

/* the recorder options */

	op->ropts = cp[2] ;

#if	CF_DEBUGS
	debugprintf("mailalias_fileheader: ropts=%02x\n",op->ropts) ;
#endif

/* if looks good, read the header stuff */

	table = (uint *) (op->mapdata + MAILALIAS_IDLEN) ;

#if	CF_DEBUGS
	{
	    int	i ;
	    for (i = 0 ; i < header_overlast ; i += 1)
	        debugprintf("mailalias_fileheader: header[%2u]=%08x\n",
	            i,table[i]) ;
	}
#endif /* CF_DEBUGS */

#ifdef	COMMENT
	op->i.wtime = table[header_wtime] ;
	op->i.wcount = table[header_wcount] ;
#endif

	op->keytab = (int *) (op->mapdata + table[header_key]) ;
	op->rectab = (int (*)[2]) (op->mapdata + table[header_rec]) ;
	op->indtab = (int (*)[2]) (op->mapdata + table[header_ind]) ;
	op->skey = (const char *) (op->mapdata + table[header_skey]) ;
	op->sval = (const char *) (op->mapdata + table[header_sval]) ;

	op->ktlen = table[header_keylen] ;
	op->rtlen = table[header_reclen] ;
	op->rilen = table[header_indlen] ;
	op->sklen = table[header_skeysize] ;
	op->svlen = table[header_svalsize] ;

ret0:
	return rs ;
}
/* end subroutine (mailalias_fileheader) */


/* acquire access to the file (mapped memory) */
static int mailalias_holdget(MAILALIAS *op,time_t dt)
{
	struct ustat	sb ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		f_changed = FALSE ;
	int		f_create = FALSE ;

#if	CF_DEBUGS
	debugprintf("mailalias_holdget: ent\n") ;
#endif

	if (op->f.held)
	    return SR_OK ;

/* if we have it mapped, check some stuff */

	if (op->mapdata != NULL) {

/* has the file changed at all? */

	    if ((dt - op->ti_filechanged) > TO_FILECHANGED) {

	        op->ti_filechanged = dt ;
	        rs1 = u_stat(op->dbfname,&sb) ;

	        if (rs1 >= 0)
	            rs1 = mailalias_filechanged(op,&sb) ;

	        f_changed = (rs1 > 0) ;
	        if (f_changed && (sb.st_size == 0))
	            f_create = TRUE ;

	    }

	    if ((! f_changed) &&
	        op->f.ocreate && op->f.owrite &&
	        ((dt - op->ti_fileold) > TO_FILEOLD)) {

	        op->ti_fileold = dt ;
	        rs1 = mailalias_fileold(op,dt) ;

	        f_changed = (rs1 > 0) ;
	        f_create = f_changed ;
	    }

/* close as necessart */

	    if (f_changed) {
	        mailalias_mapdestroy(op) ;
	    } /* end if (file size changed) */

	} /* end if (checking existing map) */

/* do a map or a re-map as necessary */

	if ((rs >= 0) && (op->mapdata == NULL)) {

	    if (f_create && op->f.ocreate && op->f.owrite) {
	        rs = mailalias_dbmake(op,dt) ;
	    }

	    f_changed = TRUE ;
	    if (rs >= 0) {
	        if ((rs = mailalias_fileopen(op,dt)) >= 0) {
	            if ((rs = u_fstat(op->fd,&sb)) >= 0) {
	                op->fi.mtime = sb.st_mtime ;
	                op->fi.size = sb.st_size ;
	                op->fi.ino = sb.st_ino ;
	                op->fi.dev = sb.st_dev ;
	                if ((rs = mailalias_mapcreate(op,dt)) >= 0) {
	                    rs = mailalias_fileheader(op) ;
	                    if (rs < 0) {
	                        mailalias_mapdestroy(op) ;
	                    }
	                } /* end if (mmap) */
	            } /* end if (fstat) */
	            if (rs < 0) {
	                mailalias_fileclose(op) ;
	            }
	        } /* end if (file) */
	    } /* end if (DB made) */

	} /* end if (mapping file) */

	if (rs >= 0)
	    op->f.held = TRUE ;

#if	CF_DEBUGS
	debugprintf("mailalias_holdget: ret rs=%d f_changed=%u\n",
	    rs,f_changed) ;
#endif

	return (rs >= 0) ? f_changed : rs ;
}
/* end subroutine (mailalias_holdget) */


/* release our hold on the filemap */
/* ARGSUSED */
static int mailalias_holdrelease(MAILALIAS *op,time_t dt)
{
	int		rs = SR_OK ;

	if (! op->f.held) return SR_OK ;
	op->f.held = FALSE ;

	return rs ;
}
/* end subroutine (mailalias_holdrelease) */


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
#if	CF_CLOSEONEXEC
	        uc_closeonexec(op->fd,TRUE) ;
#endif
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


static int mailalias_mapcreate(MAILALIAS *op,time_t dt)
{
	int		rs ;
	int		f = FALSE ;

	if (op->mapdata == NULL) {
	    size_t	ms = ulceil(op->fi.size,op->pagesize) ;
	    int		mp = PROT_READ ;
	    int		mf = MAP_SHARED ;
	    int		fd = op->fd ;
	    void	*md ;
	    if (ms == 0) ms = op->pagesize ;
	    if ((rs = u_mmap(NULL,ms,mp,mf,fd,0L,&md)) >= 0) {
	        op->mapdata = md ;
	        op->mapsize = ms ;
	        op->ti_map = dt ;
		f = TRUE ;
	    }
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (mailalias_mapcreate) */


static int mailalias_mapdestroy(MAILALIAS *op)
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
/* end subroutine (mailalias_mapdestroy) */


#if	(! CF_FASTKEYMATCH)

static int mailalias_keymatch(MAILALIAS *op,int opts,int ri,cchar aname[])
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
	struct ustat	sb ;
	int		rs, rs1 ;
	int		i ;
	int		of = op->oflags ;
	int		size ;
	int		f_create = FALSE ;

#if	CF_DEBUGS
	{
	    char	obuf[100+1] ;
	    snopenflags(obuf,100,of) ;
	    debugprintf("mailalias_dbopen: ent of=%s\n",obuf) ;
	}
#endif /* CF_DEBUGS */

	of &= (~ O_CREAT) ;
	of &= (~ O_RDWR) ;
	of &= (~ O_WRONLY) ;
	of |= O_RDONLY ;
	if ((rs = u_open(op->dbfname,of,op->operm)) >= 0) {
	    op->fd = rs ;

	    if ((rs = u_fstat(op->fd,&sb)) >= 0) {

	        op->fi.mtime = sb.st_mtime ;
	        op->fi.size = sb.st_size ;
	        op->fi.ino = sb.st_ino ;
	        op->fi.dev = sb.st_dev ;

	        op->ti_open = dt ;

	        if (op->f.ocreate && op->f.owrite) {

	            if ((! f_create) && (sb.st_size == 0))
	                f_create = TRUE ;

	            if (! f_create) {

	                rs1 = mailalias_fileold(op,dt) ;
	                f_create = (rs1 > 0) ;

#if	CF_DEBUGS
	                debugprintf("mailalias_dbopen: fileold f_create=%u\n",
	                    f_create) ;
#endif

	            } /* end if */

	            if (f_create)
	                mailalias_fileclose(op) ;

	        } /* end if (we can possibly create the DB) */

	    } /* end if (stat) */

	    if (rs < 0) {
	        u_close(op->fd) ;
	        op->fd = -1 ;
	    }
	} /* end if (successful file open) */

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: open-out rs=%d\n",rs) ;
	debugprintf("mailalias_dbopen: f_ocreate=%u\n",op->f.ocreate) ;
#endif

	if (op->f.ocreate && op->f.owrite &&
	    ((rs == SR_NOENT) || f_create)) {

#if	CF_DEBUGS
	    debugprintf("mailalias_dbopen: creating new DB rs=%d\n",rs) ;
#endif

	    f_create = TRUE ;
	    rs = mailalias_dbmake(op,dt) ;

#if	CF_DEBUGS
	    debugprintf("mailalias_dbopen: mailalias_dbmake() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        if ((rs = u_open(op->dbfname,of,op->operm)) >= 0) {
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

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: open sequence rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad0 ;

/* local or remote */

	rs = isfsremote(op->fd) ;
	op->f.remote = (rs > 0) ;
	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: f_remote=%u\n",op->f.remote) ;
#endif

	size = MAILALIAS_IDLEN + (header_overlast * sizeof(int)) ;

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: rs=%d filesize=%d headersize=%d\n",
	    rs,sb.st_size,size) ;
#endif

/* wait for the file to come in if it is not yet available */

	for (i = 0 ; (i < TO_FILECOME) && (rs >= 0) && (sb.st_size < size) ;
	    i += 1) {

	    sleep(1) ;

	    rs = u_fstat(op->fd,&sb) ;
	    op->fi.size = sb.st_size ;

	} /* end while */

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: filecome rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto bad2 ;

	if (i >= TO_FILECOME) {
	    rs = SR_TIMEDOUT ;
	    goto bad2 ;
	}

	op->fi.size = sb.st_size ;
	op->fi.mtime = sb.st_mtime ;

/* OK, continue on */

	if (op->fi.size >= size) {
	    if ((rs = mailalias_mapcreate(op,dt)) >= 0) {
	        if ((rs = mailalias_fileheader(op)) >= 0) {
	            op->ti_access = dt ;
	            op->ti_map = dt ;
	            op->f.fileinit = TRUE ;
	        }
	        if (rs < 0)
	            mailalias_mapdestroy(op) ;
	    }
	} /* end if (file had some data) */

	if ((rs >= 0) && op->f.fileinit)
	    mailalias_fileclose(op) ;

ret0:

#if	CF_DEBUGS
	debugprintf("mailalias_dbopen: ret rs=%d f_create=%u\n",
	    rs,f_create) ;
#endif

	return (rs >= 0) ? f_create : rs ;

/* bad things comes here */
bad2:
bad1:
	mailalias_fileclose(op) ;

bad0:
	goto ret0 ;
}
/* end subroutine (mailalias_dbopen) */


static int mailalias_dbclose(MAILALIAS *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = mailalias_mapdestroy(op) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = mailalias_fileclose(op) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mailalias_dbclose) */


/* make the index file */
static int mailalias_dbmake(MAILALIAS *op,time_t dt)
{
	struct ustat	sb ;
	struct dbmake	data ;
	int		rs, rs1 ;
	int		i, cl ;
	int		size ;
	int		oflags ;
	int		n = NREC_GUESS ;
	int		f_dbmake ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;
	char		newfname[MAXPATHLEN + 1] ;

	newfname[0] = '\0' ;

/* get the directory of the DB file and see if it is writable to us */

	cl = sfdirname(op->dbfname,-1,&cp) ;

	if ((rs = mkpath1w(tmpfname,cp,cl)) >= 0) {
	    if ((rs = u_stat(tmpfname,&sb)) >= 0) {
	        if (! S_ISDIR(sb.st_mode)) rs = SR_NOTDIR ;
	        if (rs >= 0) {
		    rs = sperm(&op->id,&sb,W_OK) ;
		}
	    } else {
	        rs = mkdirs(tmpfname,MAILALIAS_DIRMODE) ;
	    }
	}

	if (rs < 0)
	    goto ret0 ;

	memset(&data,0,sizeof(struct dbmake)) ;

/* make the new file to replace any existing one */

	data.fd = -1 ;
	rs = sncpy2(newfname,MAXPATHLEN,op->dbfname,"n") ;
	if (rs < 0)
	    goto ret0 ;

	f_dbmake = TRUE ;
	oflags = (O_WRONLY | O_CREAT | O_EXCL) ;
	rs = u_open(newfname,oflags,0664) ;
	data.fd = rs ;
	if (rs == SR_EXIST) {

	    size = MAILALIAS_IDLEN + (header_overlast * sizeof(int)) ;
	    for (i = 0 ; i < TO_FILECOME ; i += 1) {

	        sleep(1) ;

	        rs1 = u_stat(op->dbfname,&sb) ;

	        if ((rs1 >= 0) && (sb.st_size >= size))
	            break ;

	    } /* end for */

	    rs = (i < TO_FILECOME) ? SR_OK : SR_TIMEDOUT ;

/* should we "break" the lock? */

	    f_dbmake = FALSE ;
	    if (rs == SR_TIMEDOUT) {

	        oflags = O_WRONLY | O_CREAT ;
	        rs = u_open(newfname,oflags,0664) ;

	        if (rs == SR_ACCESS) {
	            u_unlink(newfname) ;
	            rs = u_open(newfname,oflags,0664) ;
	        }
	        data.fd = rs ;
	        f_dbmake = TRUE ;

	    } /* end if (attempt to break the lock) */

	} /* end if (waiting for file to come in) */

	if ((rs < 0) || (! f_dbmake))
	    goto ret0 ;

/* process the source files */

#if	CF_DEBUGS
	debugprintf("mailalias_dbmake: ropts=%02x\n",op->ropts) ;
#endif

/* initialize the objects to track everything */

	rs = strtab_start(&data.skeys,n) ;
	if (rs < 0)
	    goto ret1 ;

	rs = strtab_start(&data.svals,(n * 2)) ;
	if (rs < 0)
	    goto ret2 ;

	size = sizeof(struct record) ;
	rs = vecobj_start(&data.recs,size,n,0) ;
	if (rs < 0)
	    goto ret3 ;

/* process all of the files in this profile */

	rs = mailalias_aprofile(op,dt) ;
	if (rs < 0)
	    goto ret4 ;

	rs1 = SR_NOENT ;
	for (i = 0 ; op->aprofile[i] != NULL ; i += 1) {

	    cp = (const char *) op->aprofile[i] ;
	    if (*cp != '/') {
	        cp = tmpfname ;
	        mkpath2(tmpfname,op->pr,op->aprofile[i]) ;
	    }

#if	CF_DEBUGS
	    debugprintf("mailalias_dbmake: proc file=%s\n",cp) ;
#endif

	    rs1 = mailalias_procfile(op,&data,cp) ;

	    if (rs1 < 0) break ;
	} /* end for */

/* where are we? */

#if	CF_DEBUGS
	debugprintf("mailalias_dbmake: nrecs=%u\n",data.nrecs) ;
#endif /* CF_DEBUGS */

	rs = vecobj_count(&data.recs) ;
	n = (rs + 1) ;

#if	CF_DEBUGS
	debugprintf("mailalias_dbmake: record count=%d\n",n) ;
#endif

/* OK, write out the file */

	dt = time(NULL) ;

	if ((rs = mailalias_writefile(op,&data,dt)) >= 0) {
	    u_close(data.fd) ;
	    data.fd = -1 ;
	    rs = u_rename(newfname,op->dbfname) ;
	} /* end if */

/* we're out of here */
ret4:
	vecobj_finish(&data.recs) ;

ret3:
	strtab_finish(&data.svals) ;

ret2:
	strtab_finish(&data.skeys) ;

ret1:
	if (data.fd >= 0) {
	    u_close(data.fd) ;
	    data.fd = -1 ;
	}

ret0:
	if ((rs < 0) && (newfname[0] != '\0'))
	    u_unlink(newfname) ;

	return (rs >= 0) ? data.nrecs : rs ;
}
/* end subroutine (mailalias_dbmake) */


static int mailalias_procfile(MAILALIAS *op,struct dbmake *dp,cchar fname[])
{
	struct record	re ;
	FIELD		fsb ;
	bfile		afile ;
	int		rs, rs1 ;
	int		len ;
	int		cl, kl, vl ;
	int		c ;
	int		ival ;
	int		c_rec = 0 ; /* ¥ GCC false complaint */
	int		ikey = 0 ; /* ¥ GCC false complaint */
	int		f_bol, f_eol ;
	int		f_havekey ;
	int		f ;
	const char	*kp, *vp ;
	const char	*cp ;
	char		keybuf[ALIASNAMELEN + 1] ;

	if (op == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("mailalias_procfile: fname=%s\n",fname) ;
#endif

	if ((rs = bopen(&afile,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    f_havekey = FALSE ;
	    c_rec = 0 ;
	    c = 0 ;
	    f_bol = TRUE ;
	    while ((rs = breadline(&afile,lbuf,llen)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;
	        if (f_eol) len -= 1 ;

	        if ((! f_bol) || (! f_eol)) continue ;

	        lbuf[len] = '\0' ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	        debugprintf("mailalias_procfile: line>%t<\n",lbuf,len) ;
#endif

	        cp = lbuf ;
	        cl = len ;
	        if (*cp == '\0')
	            continue ;

	        if (*cp == '#')
	            continue ;

	        if (! CHAR_ISWHITE(*cp)) {
	            c = 0 ;
	            f_havekey = FALSE ;
	        }

	        if ((rs = field_start(&fsb,cp,cl)) >= 0) {

	            if (! f_havekey) {

	                kl = field_get(&fsb,kterms,&kp) ;

	                if (kl > 0) {

	                    f_havekey = TRUE ;
	                    c = 0 ;
	                    f = (kl == 10) && 
	                        (strncasecmp("Postmaster",kp,kl) == 0) ;

	                    if (f) {
	                        strwcpylc(keybuf,kp,MIN(kl,ALIASNAMELEN)) ;
	                    } else {
	                        strwcpy(keybuf,kp,MIN(kl,ALIASNAMELEN)) ;
			    }

#if	CF_DEBUGS && CF_DEBUGSFILE
	                    debugprintf("mailalias_procfile: key=%s\n",keybuf) ;
#endif

	                }

	            } /* end if (didn't have a key already) */

	            if (f_havekey) {

	                while ((vl = field_get(&fsb,vterms,&vp)) >= 0) {
	                    if (vl == 0) continue ;

/* enter key into string table (if first time) */

	                    if (c == 0) {

	                        ikey = strtab_already(&dp->skeys,keybuf,-1) ;

	                        if (ikey == SR_NOENT)
	                            ikey = strtab_add(&dp->skeys,keybuf,-1) ;

	                        rs = ikey ;

	                    } /* end if (entering key) */

/* enter value into string table */

	                    ival = strtab_already(&dp->svals,vp,vl) ;

	                    if (ival == SR_NOENT)
	                        ival = strtab_add(&dp->svals,vp,vl) ;

	                    rs = ival ;

#if	CF_DEBUGS && CF_DEBUGSFILE
	                    debugprintf("mailalias_procfile: "
				"add rs=%d val=%t\n",rs,vp,vl) ;
#endif

/* enter record */

	                    if ((rs >= 0) && (ival >= 0)) {

	                        re.key = ikey ;
	                        re.val = ival ;
	                        rs1 = vecobj_find(&dp->recs,&re) ;

	                        if (rs1 == SR_NOTFOUND) {

	                            rs = vecobj_add(&dp->recs,&re) ;

	                            if (rs >= 0) {
	                                c += 1 ;
	                                c_rec += 1 ;
	                                dp->nrecs += 1 ;
	                            }

	                        } /* end if (new entry) */

	                    } /* end if (entered into string tables) */

	                    if (fsb.term == '#') break ;
	                    if (rs < 0) break ;
	                } /* end while (fields) */

	            } /* end if (got a key) */

	            field_finish(&fsb) ;
	        } /* end if (field) */

	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while (reading extended lines) */

#if	CF_DEBUGS && CF_DEBUGSFILE
	    {
	        struct record	*rep ;
	        debugprintf("mailalias_procfile: record dump:\n") ;
	        for (i = 0 ; vecobj_get(&dp->recs,i,&rep) >= 0 ; i += 1) {
	            if (rep == NULL) continue ;
	            debugprintf("mailalias_procfile: k=%u v=%u\n",
	                rep->key,rep->val) ;
	            rs1 = strtab_get(&dp->skeys,rep->key,&cp) ;
	            debugprintf("mailalias_procfile: key=%s \n",cp) ;
	            rs1 = strtab_get(&dp->svals,rep->val,&cp) ;
	            debugprintf("mailalias_procfile: val=%s \n",cp) ;
	        }
	    }
#endif /* CF_DEBUGS */

	    bclose(&afile) ;
	} /* end if (afile) */

#if	CF_DEBUGS
	debugprintf("mailalias_procfile: ret rs=%d\n",rs) ;
#endif

	return (rs >= 0) ? c_rec : rs ;
}
/* end subroutine (mailalias_procfile) */


/* write out the cache file */
static int mailalias_writefile(MAILALIAS *op,struct dbmake *dp,time_t dt)
{
	struct record	*rep ;
	int		*keytab ;
	int		(*rectab)[2] ;
	int		(*indtab)[2] ;
	int		header[header_overlast] ;
	int		fto ;
	int		rs ;
	int		i, size, ri ;
	int		rklen, rlen, rilen ;
	int		rksize, rsize, risize ;
	int		sksize, svsize ;
#if	CF_DEBUGS
	int		off = 0 ;
#endif
	char		fidbuf[MAILALIAS_IDLEN + 1] ;
	char		*kstab, *vstab ;
	char		*cp ;

/* get length and size of key record table */

	rs = strtab_count(&dp->skeys) ;
	if (rs < 0)
	    goto ret0 ;

	rklen = (rs + 1) ;
	rksize = (rklen + 1) * sizeof(int) ;

/* get length and size of main record table */

	rs = vecobj_count(&dp->recs) ;
	if (rs < 0)
	    goto ret0 ;

	rlen = (rs + 1) ;
	rsize = rlen * (2 * sizeof(int)) ;

/* get the index length and size */

	rilen = nextpowtwo(rlen) ;

	risize = rilen * 2 * sizeof(int) ;

/* get length and size of key string table */

	rs = strtab_strsize(&dp->skeys) ;
	sksize = rs ;
	if (rs < 0)
	    goto ret0 ;

/* get length and size of value string table */

	rs = strtab_strsize(&dp->svals) ;
	svsize = rs ;
	if (rs < 0)
	    goto ret0 ;

/* OK, we're ready to start writing stuff out! */

#if	CF_DEBUGS
	debugprintf("mailalias_writefile: about to write stuff out\n") ;
#endif

	fto = 0 ;

/* prepare and write the file magic */

	cp = strwcpy(fidbuf,MAILALIAS_FILEMAGIC,15) ;

	*cp++ = '\n' ;
	memset(cp,0,(fidbuf + 16) - cp) ;

/* prepare and write the version and encoding (VETU) */

	fidbuf[16] = MAILALIAS_FILEVERSION ;
	fidbuf[17] = ENDIAN ;
	fidbuf[18] = op->ropts ;
	fidbuf[19] = 0 ;

	rs = u_write(dp->fd,fidbuf,MAILALIAS_IDLEN) ;
	if (rs < 0)
	    goto ret0 ;

	fto += MAILALIAS_IDLEN ;

/* make the header itself */

	fto += (header_overlast * sizeof(int)) ;

/* everything else */

	header[header_wtime] = (uint) dt ;
	header[header_wcount] = 1 ;

	header[header_key] = fto ;
	header[header_keylen] = rklen ;
	fto += rksize ;

	header[header_rec] = fto ;
	header[header_reclen] = rlen ;
	fto += rsize ;

	header[header_ind] = fto ;
	header[header_indlen] = rilen ;
	fto += risize ;

	header[header_skey] = fto ;
	header[header_skeysize] = sksize ;
	fto += sksize ;

	header[header_sval] = fto ;
	header[header_svalsize] = svsize ;
	fto += svsize ;

/* write out the header */

#if	CF_DEBUGS
	for (i = 0 ; i < header_overlast ; i += 1)
	    debugprintf("writecache: header[%2u]=%08x\n",i,header[i]) ;
#endif /* CF_DEBUGS */

	size = header_overlast * sizeof(int) ;
	rs = u_write(dp->fd,header,size) ;

#if	CF_DEBUGS
	{
	    off += rs ;
	    debugprintf("writecache: write() header rs=%d off=%08x\n",
	        rs,off) ;
	}
#endif

	if (rs < 0)
	    goto ret1 ;

/* make the key record table */

#if	CF_DEBUGS
	debugprintf("mailalias_writefile: making key table\n") ;
#endif

	if ((rs = uc_malloc(rksize,&keytab)) >= 0) {

	    rs = strtab_recmk(&dp->skeys,(int *) keytab,rksize) ;

	    if (rs >= 0)
	        rs = u_write(dp->fd,keytab,rksize) ;

	    uc_free(keytab) ;
	} /* end if */

	if (rs < 0)
	    goto ret2 ;

/* make the main record table */

#if	CF_DEBUGS
	debugprintf("mailalias_writefile: making main record table\n") ;
#endif

	if ((rs = uc_malloc(rsize,&rectab)) >= 0) {

	    ri = 0 ;
	    rectab[ri][0] = 0 ;
	    rectab[ri][1] = 0 ;
	    ri += 1 ;
	    for (i = 0 ; vecobj_get(&dp->recs,i,&rep) >= 0 ; i += 1) {
	        rectab[ri][0] = rep->key ;
	        rectab[ri][1] = rep->val ;
	        ri += 1 ;
	    } /* end for */

	    rs = u_write(dp->fd,rectab,rsize) ;

	    uc_free(rectab) ;
	} /* end if (record table) */

	if (rs < 0)
	    goto ret3 ;

/* make the main index (there is only one index in this file) */

#if	CF_DEBUGS
	debugprintf("mailalias_writefile: making record index table\n") ;
#endif

	if ((rs = uc_malloc(risize,&indtab)) >= 0) {

	    if ((rs = uc_malloc(sksize,&kstab)) >= 0) {

	        rs = strtab_strmk(&dp->skeys,kstab,sksize) ;
	        if (rs >= 0)
	            rs = mailalias_mkind(op,&dp->recs,kstab,indtab,risize) ;

#if	CF_DEBUGS
	        debugprintf("writecache: mailalias_mkind() rs=%d\n",rs) ;
#endif

	        if (rs >= 0)
	            rs = u_write(dp->fd,indtab,risize) ;

/* write out the key string table */

#if	CF_DEBUGS
	        debugprintf("mailalias_writefile: writing key-str table\n") ;
#endif

	        if (rs >= 0)
	            rs = u_write(dp->fd,kstab,sksize) ;

	        uc_free(kstab) ;
	    } /* end if (memory allocation) */

	    uc_free(indtab) ;
	} /* end if (memory allocation) */

	if (rs < 0)
	    goto ret4 ;

/* make the value string table */

#if	CF_DEBUGS
	debugprintf("mailalias_writefile: handing value string table\n") ;
#endif

	if ((rs = uc_malloc(svsize,&vstab)) >= 0) {

	    rs = strtab_strmk(&dp->svals,vstab,svsize) ;

	    if (rs >= 0)
	        rs = u_write(dp->fd,vstab,svsize) ;

	    uc_free(vstab) ;
	} /* end if (value string-table) */

/* we're out of here */
ret4:
ret3:
ret2:
ret1:
ret0:

#if	CF_DEBUGS
	debugprintf("mailalias_writefile: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (mailalias_writefile) */


/* make the (only) index for this file */
static int mailalias_mkind(MAILALIAS *op,VECOBJ *rp,cchar skey[],int it[][2],
		int itsize)
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

	rs = vecobj_count(rp) ;
	if (rs < 0)
	    goto ret0 ;

	n = nextpowtwo(rs) ;

	size = n * 2 * sizeof(uint) ;

#if	CF_DEBUGS
	debugprintf("mailalias_mkind: size calc=%u given=%u\n",size,itsize) ;
#endif

	if (size <= itsize) {
	    struct record	*rep ;
	    uint		khash ;
	    int			i, ri, hi, ki ;
	    int			v, c ;
	    int			kl ;
	    int			f ;
	    const char		*kp ;

#if	CF_DEBUGS
	{
	    debugprintf("mailalias_mkind: key string dump:\n") ;
	    kp = skey + 1 ;
	    while (kp[0] != '\0') {
	        kl = strlen(kp) ;
	        debugprintf("mailalias_mkind: sl=%u skey=%s\n",kl,kp) ;
	        kp += (kl + 1) ;
	    }
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

	            if (! f)
	                break ;

	            if (op->ropts & MAILALIAS_ORANDLC) {
	                khash = randlc(khash + c) ;
	            } else
	                khash = ((khash << (32 - ns)) | (khash >> ns)) + c ;

	            hi = hashindex(khash,n) ;

#if	CF_DEBUGS
	            debugprintf("mailalias_mkind: new khash=%08x hi=%u\n",
	                khash,hi) ;
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

	} else
	    rs = SR_OVERFLOW ;

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (mailalias_mkind) */


static int mailalias_filechanged(MAILALIAS *op,struct ustat *sbp)
{

	if (op->fi.size != sbp->st_size)
	    return 1 ;

	if (op->fi.mtime != sbp->st_mtime)
	    return 1 ;

	if (op->fi.ino != sbp->st_ino)
	    return 1 ;

	if (op->fi.dev != sbp->st_dev)
	    return 1 ;

	return 0 ;
}
/* end subroutine (mailalias_filechanged) */


static int mailalias_fileold(MAILALIAS *op,time_t dt)
{
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if ((rs = mailalias_aprofile(op,dt)) >= 0) {
	    struct ustat	sb ;
	    int			i ;
	    const char		*cp ;
	    char		tmpfname[MAXPATHLEN + 1] ;
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
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;
	const char	*cp ;
	char		tmpfname[MAXPATHLEN + 1] ;

	if ((dt - op->ti_aprofile) <= TO_APROFILE)
	    return SR_OK ;

	op->ti_aprofile = dt ;

/* get the mailalias profile (AP) table */

	rs1 = SR_NOENT ;
	for (i = 0 ; aptabsched[i] != NULL ; i += 1) {

	    cp = (const char *) aptabsched[i] ;
	    if (*cp != '/') {
	        cp = tmpfname ;
	        mkpath2(tmpfname,op->pr,aptabsched[i]) ;
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

	    rs = kvsfile_open(&aptab,0,cp) ;

#if	CF_DEBUGS
	    debugprintf("mailalias_aprofile: kvsfile_open() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) {
	        kvsfile_cur	cur ;

	        for (i = 0 ; vecstr_get(&op->apfiles,i,&cp) >= 0 ; i += 1) {
	            if (cp == NULL) continue ;

	            vecstr_del(&op->apfiles,i--) ;

	        } /* end for */

	        kvsfile_curbegin(&aptab,&cur) ;

	        while (rs >= 0) {

	            rs1 = kvsfile_fetch(&aptab,op->apname,&cur,
	                tmpfname,MAXPATHLEN) ;

	            if (rs1 < 0)
	                break ;

#if	CF_DEBUGS
	            debugprintf("mailalias_aprofile: kvsfile_fetch() v=%s\n",
	                tmpfname) ;
#endif

	            rs = vecstr_add(&op->apfiles,tmpfname,rs1) ;

	        } /* end while */

	        kvsfile_curend(&aptab,&cur) ;

	        kvsfile_close(&aptab) ;

	        if (rs >= 0)
	            rs = vecstr_getvec(&op->apfiles,&op->aprofile) ;

	    } /* end if (opened key-values table) */

#if	CF_DEFPROFILE
	    if (op->aprofile[0] == NULL)
	        op->aprofile = defprofile ;
#endif

	} else
	    op->aprofile = defprofile ;

#if	CF_DEBUGS
	debugprintf("mailalias_aprofile: ret rs=%d enum\n",rs) ;
	if (rs >= 0) {
	    for (i = 0 ; op->aprofile[i] != NULL ; i += 1)
	        debugprintf("mailalias_aprofile: aprofile file=%s\n",
	            op->aprofile[i]) ;
	}
#endif

	return rs ;
}
/* end subroutine (mailalias_aprofile) */


/* calculate the next hash table index from a given one */
static int hashindex(uint i,int n)
{
	int		hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end if (hashindex) */


