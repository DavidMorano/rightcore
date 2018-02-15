/* bdb */

/* Bibliographical DataBase */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This module was changed to serve in the REFERM program.

	= 2005-10-01, David A­D­ Morano
        This was changed to work in the MMCITE program. The old REFERM program
        is really obsolete. It used a database lookup strategy to remote
        databases. The high-level problem is: what to do if the cited BIB entry
        isn't found? How does a maintainer of the present (local) document know
        what that BIB entry was? The new strategy (implemented by the MMCITE
        program) is more like what is done with BibTeX in the TeX (or LaTeX)
        world. All BIB databases are really expected to be maintained by the
        document creator -- not some centralized entity. The older centralized
        model reflected more the use in the corporate world (where different
        people create BIB entries) than in the more "modern"
        personal-responsibility type of world! :-) Anyway, this is the way the
        gods seem to now want to do things. Deal with it!

*/

/* Copyright © 1998,2005 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code object module implements a little DB query facility. The
        database is a set of files that contain bibliographical entries in the
        "REFER" format.

        Queries to the database that succeed return a structure with the
        elements of the bibliographic entry.

        All queries are to database files that were referenced last (most
        previously) in the processing of the document. The idea is that a more
        recent reference to a database file means that it is the preferred one
        to use since it is closest to the user (or the most preferred by the
        user).

        A small nice thing about this database is that database files (files
        containing "REFER"-formatted entries) are not indexed unless they are
        needed (due to a failure to find the query in existing indexed files).
        Only after all database files have been indexed and scanned to try to
        satisfy the query (and the request os not found), does the query fail.


*******************************************************************************/


#define	BDB_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<hdb.h>
#include	<sbuf.h>
#include	<vecobj.h>
#include	<buffer.h>
#include	<localmisc.h>

#include	"bdb.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		(20 * LINEBUFLEN)
#define	CITEBUFLEN	LINEBUFLEN

#define	BDB_MAGIC	27
#define	BDB_DEFFILES	10		/* default files */
#define	BDB_DEFENTRIES	40		/* default entries */

#define	BDB_KA		sizeof(char *(*)[2])
#define	BDB_BO(v)		\
	((BDB_KA - ((v) % BDB_KA)) % BDB_KA)

#define	KEYVAL		struct keyval

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	TWOCHARS(a,b)	(((a) << 8) + (b))


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	isalphalatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct bdb_key {
	const char	*citekey ;
	uint		coff ;
	int		clen ;
	int		fi ;		/* file index */
} ;

struct bdb_file {
	const char	*fname ;
	int		f_indexed ;
} ;

struct keyval {
	const char	*kp, *vp ;
	int		kl, vl ;
} ;

struct bibentry {
	const char	*kp ;
	VECOBJ		keyvals ;
	BUFFER		vb ;
	int		size ;
	int		kl ;
	int		fi ;
} ;


/* forward references */

static int bdb_scan(BDB *,HDB_DATUM,struct bdb_key *) ;
static int bdb_fileindex(BDB *,int) ;
static int bdb_fileproc(BDB *,int,struct bdb_file *) ;
static int bdb_keyinsert(BDB *,int,uint,const char *,int) ;
static int bdb_readentry(BDB *,BDB_ENT *,char *,int,struct bdb_key *) ;

static int bdbfile_start(struct bdb_file *,const char *) ;
static int bdbfile_finish(struct bdb_file *) ;
static int bdbfile_indexed(struct bdb_file *) ;

static int bdbkey_start(struct bdb_key *,int,uint,const char *,int) ;
static int bdbkey_finish(struct bdb_key *) ;

static int bibentry_start(struct bibentry *,int) ;
static int bibentry_begin(struct bibentry *,cchar *,int,cchar *,int) ;
static int bibentry_continue(struct bibentry *,const char *,int) ;
static int bibentry_end(struct bibentry *) ;
static int bibentry_finish(struct bibentry *) ;

static int	entry_load(BDB_ENT *,char *,int,struct bibentry *) ;

static int	keyval_start(KEYVAL *) ;
static int	keyval_size(KEYVAL *) ;
static int	keyval_finish(KEYVAL *) ;

static int	iskey(const char *,int) ;


/* local variables */

enum states {
	state_search,
	state_have,
	state_overlast
} ;

#if	CF_DEBUGS
static const char	*states[] = {
	"search",
	"have",
	NULL
} ;
#endif /* CF_DEBUGS */


/* exported subroutines */


int bdb_start(BDB *op,cchar *qkey,int opts)
{
	int		rs ;
	int		cl ;
	const char	*cp ;

	if (op == NULL) return SR_FAULT ;

	memset(op,0,sizeof(BDB)) ;

#if	CF_DEBUGS
	debugprintf("bdb_start: qkey=%s\n",qkey) ;
#endif

	if ((qkey == NULL) || (qkey[0] == '\0'))
	    qkey = BDB_QUERYKEY ;

/* store away stuff */

	cl = strlen(qkey) ;
	op->opts = opts ;

	if ((rs = uc_mallocstrw(qkey,cl,&cp)) >= 0) {
	    const int	esize = sizeof(struct bdb_file) ;
	    const int	nf = BDB_DEFFILES ;
	    const int	vo = (VECOBJ_OORDERED | VECOBJ_OSTATIONARY) ;
	    op->qkbuf = cp ;
	    op->qklen = cl ;
	    if ((rs = vecobj_start(&op->files,esize,nf,vo)) >= 0) {
		const int	ne = BDB_DEFENTRIES ;
		if ((rs = hdb_start(&op->keys,ne,1,NULL,NULL)) >= 0) {
		    op->magic = BDB_MAGIC ;
		}
		if (rs < 0)
		    vecobj_finish(&op->files) ;
	    } /* end if (vecobj_start) */
	    if (rs < 0) {
	        uc_free(op->qkbuf) ;
	        op->qkbuf = NULL ;
	    }
	} /* end if (m-a) */
	
	return rs ;
}
/* end subroutine (bdb_start) */


int bdb_finish(BDB *op)
{
	struct bdb_file	*bfep ;
	struct bdb_key	*bkp ;
	HDB_CUR		cur ;
	HDB_DATUM	key, value ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BDB_MAGIC) return SR_NOTOPEN ;

	if (op->qkbuf != NULL) {
	    rs1 = uc_free(op->qkbuf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->qkbuf = NULL ;
	}

/* free up all key entries */

	if (hdb_curbegin(&op->keys,&cur) >= 0) {
	    while (hdb_enum(&op->keys,&cur,&key,&value) >= 0) {
	        bkp = (struct bdb_key *) value.buf ;
	        rs1 = bdbkey_finish(bkp) ;
	        if (rs >= 0) rs = rs1 ;
	        rs1 = uc_free(bkp) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end while */
	    rs1 = hdb_curend(&op->keys,&cur) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (cursor) */

/* free up the key container */

	rs1 = hdb_finish(&op->keys) ;
	if (rs >= 0) rs = rs1 ;

/* free up all file entries */

	for (i = 0 ; vecobj_get(&op->files,i,&bfep) >= 0 ; i += 1) {
	    if (bfep != NULL) {
	        rs1 = bdbfile_finish(bfep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

/* free up the file entry container */

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bdb_finish) */


int bdb_add(BDB *op,cchar *fname)
{
	struct ustat	sb ;
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (op->magic != BDB_MAGIC) return SR_NOTOPEN ;

	if (fname[0] == '\0') return SR_INVALID ;

/* check if the file actually exists */

	if ((rs = u_stat(fname,&sb)) >= 0) {
	    if (! S_ISDIR(sb.st_mode)) {
	        struct bdb_file	bfe ;
	        if ((rs = bdbfile_start(&bfe,fname)) >= 0) {
	            if ((rs = vecobj_add(&op->files,&bfe)) >= 0) {
	                op->unindexed += 1 ;
	            }
	            if (rs < 0) {
	                bdbfile_finish(&bfe) ;
		    }
	        } /* end if (bdbfile) */
	    } else {
	        rs = SR_ISDIR ;
	    }
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (bdb_add) */


int bdb_count(BDB *op)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BDB_MAGIC) return SR_NOTOPEN ;

	rs = hdb_count(&op->keys) ;

	return rs ;
}
/* end subroutine (bdb_count) */


/* delete the entry under the current cursor */
int bdb_delcur(BDB *op,BDB_CUR *curp,int f_adv)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BDB_MAGIC) return SR_NOTOPEN ;

	rs = hdb_delcur(&op->keys,&curp->cur,f_adv) ;

#if	CF_DEBUGS
	debugprintf("bdbdelcursor: hdb_delcur() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdb_delcur) */


/* make a query */
int bdb_query(op,citekey,bep,bebuf,belen)
BDB		*op ;
const char	citekey[] ;
BDB_ENT		*bep ;
char		bebuf[] ;
int		belen ;
{
	struct bdb_key	bke ;
	HDB_DATUM	key ;
	int		rs ;
	int		rs1 ;
	int		n = 0 ;

#if	CF_DEBUGS
	debugprintf("bdb_query: ent citekey=%s\n",citekey) ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (citekey == NULL) return SR_FAULT ;
	if (bep == NULL) return SR_FAULT ;
	if (bebuf == NULL) return SR_FAULT ;

	if (op->magic != BDB_MAGIC) return SR_NOTOPEN ;

	if (citekey[0] == '\0') return SR_INVALID ;
	if (belen <= 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("bdb_query: continuing\n") ;
#endif

	if ((rs = vecobj_count(&op->files)) > 0) {
	    n = rs ;

/* search the keys and find the highest file-index */

	key.buf = (void *) citekey ;
	key.len = strlen(citekey) ;

/* perform the search */

	if (op->opts & BDB_OUNIQ) {

#if	CF_DEBUGS
	    debugprintf("bdb_query: UNIQ mode n=%u\n",n) ;
#endif

	    while (op->unindexed > 0) {

	        rs1 = bdb_fileindex(op,n) ;

#if	CF_DEBUGS
	        debugprintf("bdb_query: "
			"UNIQ bdb_fileindex() rs=%d\n",rs1) ;
#endif

	        if (rs1 < 0) break ;
	    } /* end while */

	} /* end if (uniqueness of query required) */

#if	CF_DEBUGS
	debugprintf("bdb_query: bdb_scan() \n") ;
#endif

	rs = bdb_scan(op,key,&bke) ;

#if	CF_DEBUGS
	debugprintf("bdb_query: bdb_scan() rs=%d\n",rs) ;
#endif

	if (op->opts & BDB_OUNIQ) {
	    if (rs > 1) rs = SR_NOTUNIQ ;
	} /* end if (uniqueness) */

	while ((rs == SR_NOTFOUND) && (op->unindexed > 0)) {

#if	CF_DEBUGS
	    debugprintf("bdb_query: bdb_fileindex()\n") ;
#endif

	    rs1 = bdb_fileindex(op,n) ;

#if	CF_DEBUGS
	    debugprintf("bdb_query: bdb_fileindex() rs=%d\n",rs1) ;
#endif

	    if (rs1 < 0)
	        break ;

#if	CF_DEBUGS
	    debugprintf("bdb_query: bdb_scan()\n") ;
#endif

	    rs = bdb_scan(op,key,&bke) ;

#if	CF_DEBUGS
	    debugprintf("bdb_query: bdb_scan() rs=%d\n",rs) ;
#endif

	    if (rs >= 0) break ;
	} /* end while */

#if	CF_DEBUGS && 0
	{
	    struct bdb_key	*bkp ;
	    HDB_CUR		cur ;
	    HDB_DATUM		value ;
	    debugprintf("bdb_query: search loop rs=%d\n",rs) ;
	    hdb_curbegin(&op->keys,&cur) ;
	    while (hdb_enum(&op->keys,&cur,&key,&value) >= 0) {
	        bkp = (struct bdb_key *) value.buf ;
	        debugprintf("bdb_query: citekey=%t fi=%u\n",
	            key.buf,key.len,bkp->fi) ;
	    }
	    hdb_curend(&op->keys,&cur) ;
	}
#endif /* CF_DEBUGS */

	if (rs >= 0) {
	    rs = bdb_readentry(op,bep,bebuf,belen,&bke) ;
	    n = rs ;
	} /* end if */

	} else if (rs == 0) {
	    rs = SR_NOTFOUND ;
	}

#if	CF_DEBUGS
	debugprintf("bdb_query: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bdb_query) */


int bdb_curbegin(BDB *op,BDB_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BDB_MAGIC) return SR_NOTOPEN ;

	rs = hdb_curbegin(&op->keys,(HDB_CUR *) curp) ;

	return rs ;
}
/* end subroutine (bdb_curbegin) */


int bdb_curend(BDB *op,BDB_CUR *curp)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (curp == NULL) return SR_FAULT ;

	if (op->magic != BDB_MAGIC) return SR_NOTOPEN ;

	rs = hdb_curend(&op->keys,(HDB_CUR *) curp) ;

	return rs ;
}
/* end subroutine (bdb_curend) */


/* private subroutines */


static int bdb_scan(BDB *op,HDB_DATUM key,struct bdb_key *ubkp)
{
	struct bdb_key	*bkp ;
	HDB_CUR		cur ;
	HDB_DATUM	value ;
	int		rs ;
	int		c = 0 ;

	if ((rs = hdb_curbegin(&op->keys,&cur)) >= 0) {
	    int	fi = -1 ;

	    while (hdb_fetch(&op->keys,key,&cur,&value) >= 0) {

	        c += 1 ;
	        bkp = (struct bdb_key *) value.buf ;
	        if (bkp->fi > fi) {
	            *ubkp = *bkp ; /* copy */
	            fi = bkp->fi ;
	        }

	    } /* end while (looping through keys) */

	    hdb_curend(&op->keys,&cur) ;
	    if ((rs >= 0) && (fi < 0)) rs = SR_NOTFOUND ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("bdb_scan: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bdb_scan) */


/* index one BDB database file (the most recent unindexed file) */
static int bdb_fileindex(BDB *op,int n)
{
	struct bdb_file	*bfep = NULL ;
	vecobj		*flp = &op->files ;
	int		rs = SR_NOTFOUND ;
	int		fi = 0 ;

#if	CF_DEBUGS
	debugprintf("bdb_fileindex: ent n=%u\n",n) ;
#endif

	if (n == 0) return SR_NOTFOUND ;

/* find the youngest file that has not been indexed */

	for (fi = (n - 1) ; 
	    (fi >= 0) && ((rs = vecobj_get(flp,fi,&bfep)) >= 0) ; 
	    fi -= 1) {

	    if (bfep != NULL) {
	        if (! bfep->f_indexed) break ;
	    }

	} /* end for */

#if	CF_DEBUGS
	debugprintf("bdb_fileindex: file to index fi=%u\n",fi) ;
#endif

/* something to index? */

	if ((rs >= 0) && (fi >= 0) && (bfep != NULL)) {
	    rs = bdb_fileproc(op,fi,bfep) ;
	}

#if	CF_DEBUGS
	debugprintf("bdb_fileindex: ret rs=%d fi=%u\n",rs,fi) ;
#endif

	return (rs >= 0) ? fi : rs ;
}
/* end subroutine (bdb_fileindex) */


static int bdb_fileproc(BDB *op,int fi,struct bdb_file *bfep)
{
	SBUF		citekey ;
	bfile		bibfile, *bfp = &bibfile ;
	uint		foff = 0 ;
	uint		coff ;
	int		rs ;
	int		rs1 ;
	int		len ;
	int		f_inkey = FALSE ;

#if	CF_DEBUGS
	debugprintf("bdb_fileproc: fi=%u\n",fi) ;
#endif

	op->unindexed -= 1 ;
	if ((rs = bopen(bfp,bfep->fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    const int	clen = CITEBUFLEN  ;
	    int		state = state_search ;
	    int		ll, cl, kl ;
	    int		f_bol = TRUE ;
	    int		f_eol ;
	    cchar	*lp, *cp, *kp ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    char	cbuf[CITEBUFLEN + 1] ;

	    while ((rs = breadline(bfp,lbuf,llen)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;

	        lp = lbuf ;
	        ll = (f_eol) ? (len - 1) : len ;

#if	CF_DEBUGS
	        debugprintf("bdb_fileproc: line=>%t<\n",
	            lbuf,
	            ((f_eol) ? (len - 1) : len)) ;
	        debugprintf("bdb_fileproc: f_bol=%u state=%s(%u)\n",
	            f_bol,states[state],state) ;
#endif

	        switch (state) {

	        case state_search:
	            if (! f_bol)
	                break ;

	            if (! iskey(lbuf,len))
	                break ;

	            state = state_have ;
	            coff = foff ;

/* fall-through from previous case */
/* FALLTHROUGH */
	        case state_have:
	            if (f_bol && iskey(lbuf,len)) {

#if	CF_DEBUGS
	                debugprintf("bdb_fileproc: key=%c\n",lbuf[1]) ;
	                debugprintf("bdb_fileproc: f_inkey=%u\n",f_inkey) ;
#endif

	                if (f_inkey) {
	                    f_inkey = FALSE ;
	                    sbuf_finish(&citekey) ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	                cl = nextfield(lp,ll,&cp) ;

#if	CF_DEBUGS
	                debugprintf("bdb_fileproc: cl=%u c=%t\n",cl,cp,cl) ;
	                debugprintf("bdb_fileproc: ql=%u q=%t\n",
	                    op->qklen,op->qkbuf,op->qklen) ;
#endif

	                if ((strncmp(cp,op->qkbuf,op->qklen) == 0) &&
	                    (cl == op->qklen)) {

	                    ll -= ((cp + cl) - lp) ;
	                    lp = (cp + cl) ;

#if	CF_DEBUGS
	                    debugprintf("bdb_fileproc: rline=%t\n",lp,ll) ;
#endif

	                    kl = nextfield(lp,ll,&kp) ;

#if	CF_DEBUGS
	                    debugprintf("bdb_fileproc: citekey start\n") ;
	                    debugprintf("bdb_fileproc: k=%t\n",kp,kl) ;
#endif

	                    rs = sbuf_start(&citekey,cbuf,clen) ;
	                    f_inkey = (rs >= 0) ;

	                    if ((rs >= 0) && (kl > 0)) {
	                        sbuf_strw(&citekey,kp,kl) ;
			    }

	                } /* end if (found citekey) */

	            } else if (f_inkey) {

	                if (sbuf_getlen(&citekey) > 0) {
	                    sbuf_char(&citekey,' ') ;
			}

	                sbuf_strw(&citekey,lbuf,len) ;

	            } /* end if */

/* are we leaving an entry? */

#if	CF_DEBUGS
	            debugprintf("bdb_fileproc: leaving?\n") ;
	            debugprintf("bdb_fileproc: len=%d\n",len) ;
	            debugprintf("bdb_fileproc: lbuf=%s\n",lbuf) ;
#endif

	            if ((rs >= 0) && ((len == 0) || (lbuf[0] == '\n'))) {

#if	CF_DEBUGS
	                debugprintf("bdb_fileproc: end-of-entry \n") ;
	                debugprintf("bdb_fileproc: f_inkey=%u\n",f_inkey) ;
#endif

	                state = state_search ;
	                if (f_inkey) {
	                    f_inkey = FALSE ;
	                    sbuf_finish(&citekey) ;
	                }

#if	CF_DEBUGS
	                debugprintf("bdb_fileproc: cbuf=%s\n",cbuf) ;
#endif

	                if (cbuf[0] != '\0') {
			    const int	colen = (foff-coff) ;

#if	CF_DEBUGS
	                    debugprintf("bdb_fileproc: inserting key=%s\n",
	                        cbuf) ;
	                    debugprintf("bdb_fileproc: fi=%u foff=%u "
	                        "coff=%u\n",
	                        fi,foff,coff) ;
#endif

	                    rs = bdb_keyinsert(op,fi,coff,cbuf,colen) ;

#if	CF_DEBUGS
	                    debugprintf("bdb_fileproc: bdb_keyinsert() "
	                        "rs=%d\n",rs) ;
#endif

	                    cbuf[0] = '\0' ;

	                } /* end if */

	            } /* end if (state transition) */
	            break ;

	        } /* end switch */

	        foff += len ;
	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while */

	    rs1 = bclose(bfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bfile) */

	if (f_inkey) {
	    f_inkey = FALSE ;
	    sbuf_finish(&citekey) ;
	}

/* mark this file as being indexed */

	if (rs >= 0) {
	    bdbfile_indexed(bfep) ;
	}

#if	CF_DEBUGS
	debugprintf("bdb_fileproc: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine if (bdb_fileproc) */


/* insert a citation-key into the BIB key DB */
static int bdb_keyinsert(op,fi,coff,cbuf,clen)
BDB		*op ;
int		fi ;
uint		coff ;
const char	cbuf[] ;
int		clen ;
{
	struct bdb_key	*bkp ;
	const int	esize = sizeof(struct bdb_key) ;
	int		rs ;

#if	CF_DEBUGS
	debugprintf("bdb_keyinsert: coff=%u\n",coff) ;
	debugprintf("bdb_keyinsert: cbuf=%s\n",cbuf) ;
	debugprintf("bdb_keyinsert: clen=%d\n",clen) ;
#endif

	if ((rs = uc_malloc(esize,&bkp)) >= 0) {

	    if ((rs = bdbkey_start(bkp,fi,coff,cbuf,clen)) >= 0) {
	        HDB_DATUM	key, value ;

	        key.buf = bkp->citekey ;
	        key.len = strlen(bkp->citekey) ;

	        value.buf = bkp ;
	        value.len = esize ;

	        rs = hdb_store(&op->keys,key,value) ;
	        if (rs < 0)
	            bdbkey_finish(bkp) ;

	    } /* end if (bdbkey_start) */

	    if (rs < 0)
	        uc_free(bkp) ;

	} /* end if (m-a) */

#if	CF_DEBUGS
	debugprintf("bdb_keyinsert: ret rs=%d clen=%u\n",rs,clen) ;
#endif

	return (rs >= 0) ? clen : rs ;
}
/* end subroutine (bdb_keyinsert) */


/* read a BIB entry */
static int bdb_readentry(op,bep,bebuf,belen,bkp)
BDB		*op ;
BDB_ENT		*bep ;
char		bebuf[] ;
int		belen ;
struct bdb_key	*bkp ;
{
	struct bibentry	ie ;
	struct bdb_file	*bfep ;
	int		rs ;
	int		rs1 ;
	int		rlen ;
	int		n = 0 ;
	int		f_inkey ;

#if	CF_DEBUGS
	debugprintf("bdb_readentry: ent\n") ;
#endif

#ifdef	COMMENT
	if (op == NULL) return SR_FAULT ;
	if (bkp == NULL) return SR_FAULT ;
	if (bep == NULL) return SR_FAULT ;
	if (bebuf == NULL) return SR_FAULT ;
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("bdb_readentry: bk fi=%u\n",bkp->fi) ;
	debugprintf("bdb_readentry: bk ckey=%s\n",bkp->citekey) ;
	debugprintf("bdb_readentry: bk coff=%u\n",bkp->coff) ;
	debugprintf("bdb_readentry: bk clen=%d\n",bkp->clen) ;
#endif

	memset(bep,0,sizeof(BDB_ENT)) ;

	if ((rs = vecobj_get(&op->files,bkp->fi,&bfep)) >= 0) {

	if ((rs = bibentry_start(&ie,bkp->fi)) >= 0) {
	    bfile	bibfile, *bfp = &bibfile ;

#if	CF_DEBUGS
	debugprintf("bdb_readentry: bfname=%s\n",bfep->fname) ;
#endif

	    if ((rs = bopen(bfp,bfep->fname,"r",0666)) >= 0) {
		offset_t	boff = (offset_t) bkp->coff ;
	        const int	llen = LINEBUFLEN ;
		int		ll, cl ;
		int		len ;
		int		f_bol, f_eol ;
	        const char	*lp, *cp ;
	        char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
		debugprintf("bdb_readentry: coff=%u\n",bkp->coff) ;
		debugprintf("bdb_readentry: boff=%lld\n",boff) ;
#endif

	        bseek(bfp,boff,SEEK_SET) ;

	        f_inkey = FALSE ;
	        f_bol = TRUE ;
	        rlen = 0 ;
	        while ((rlen < bkp->clen) &&
	            ((rs = breadline(bfp,lbuf,llen)) > 0)) {
	            len = rs ;

	            f_eol = (lbuf[len - 1] == '\n') ;

	            lp = lbuf ;
	            ll = (f_eol) ? (len - 1) : len ;

#if	CF_DEBUGS
		debugprintf("bdb_readentry: l=%t\n",
			lp,strlinelen(lp,ll,40)) ;
#endif

	            if (f_bol && ((ll == 0) || (lbuf[0] == '\n')))
	                break ;

	            if (f_bol && (lbuf[0] == '%')) {

	                if (f_inkey) {
	                    rs = bibentry_end(&ie) ;
			}

	                f_inkey = TRUE ;
	                cl = nextfield((lp + 1),(ll - 1),&cp) ;

	                if (rs >= 0) {
	                    ll -= ((cp + cl) - lp) ;
	                    lp = (cp + cl) ;
	                    rs = bibentry_begin(&ie,cp,cl,lp,ll) ;
	                }

	            } else if (f_inkey) {

	                rs = bibentry_continue(&ie,lp,ll) ;

	            }

	            rlen += len ;
	            f_bol = f_eol ;
	            if (rs < 0) break ;
	        } /* end while */

	        if ((rs >= 0) && f_inkey) {
	            rs = bibentry_end(&ie) ;
		}

/* OK, load this internal entry up into the interface entry */

#if	CF_DEBUGS
	        {
	            n = vecobj_count(&ie.keyvals) ;
	            debugprintf("bdb_readentry: total n=%u size=%u\n",
			n,ie.size) ;
	        }
#endif

	        if (rs >= 0) {
	            rs = entry_load(bep,bebuf,belen,&ie) ;
	            n = rs ;
	        }

	        rs1 = bclose(bfp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (file-open) */

	    rs1 = bibentry_finish(&ie) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (bib-entry) */

	} /* end if (vecobj_get) */

#if	CF_DEBUGS
	debugprintf("bdb_readentry: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bdb_readentry) */


/* operate on the BDBFILE object */
static int bdbfile_start(struct bdb_file *fp,cchar *fname)
{
	int		rs ;
	const char	*cp ;

	memset(fp,0,sizeof(struct bdb_file)) ;

	fp->f_indexed = 0 ;
	rs = uc_mallocstrw(fname,-1,&cp) ;
	if (rs >= 0) fp->fname = cp ;

	return rs ;
}
/* end subroutine bdbfile_start) */


static int bdbfile_finish(struct bdb_file *fp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (fp == NULL) return SR_FAULT ;

	if (fp->fname != NULL) {
	    rs1 = uc_free(fp->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    fp->fname = NULL ;
	}

	return rs ;
}
/* end subroutine bdbfile_finish) */


static int bdbfile_indexed(struct bdb_file *fp)
{

	if (fp == NULL) return SR_FAULT ;

	fp->f_indexed = TRUE ;
	return SR_OK ;
}
/* end subroutine bdbfile_indexed) */


/* operate on the BDBKEY object */
static int bdbkey_start(bkp,fi,coff,citekey,clen)
struct bdb_key	*bkp ;
int		fi ;
uint		coff ;
const char	citekey[] ;
int		clen ;
{
	int		rs ;
	const char	*cp ;

#if	CF_DEBUGS
	debugprintf("bdb/bdbkey_start: fi=%u\n",fi) ;
	debugprintf("bdb/bdbkey_start: coff=%u\n",coff) ;
	debugprintf("bdb/bdbkey_start: cbuf=%s\n",citekey) ;
	debugprintf("bdb/bdbkey_start: clen=%d\n",clen) ;
#endif

	memset(bkp,0,sizeof(struct bdb_key)) ;

	bkp->fi = fi ;
	bkp->coff = coff ;
	bkp->clen = clen ;
	if ((rs = uc_mallocstrw(citekey,-1,&cp)) >= 0) {
	    bkp->citekey = cp ;
	}

	return rs ;
}
/* end subroutine (bdbkey_start) */


static int bdbkey_finish(struct bdb_key *bkp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (bkp == NULL) return SR_FAULT ;

	if (bkp->citekey != NULL) {
	    rs1 = uc_free(bkp->citekey) ;
	    if (rs >= 0) rs = rs1 ;
	    bkp->citekey = NULL ;
	}

	bkp->fi = -1 ;
	return rs ;
}
/* end subroutine (bdbkey_finish) */


/* initialize a BIB accumulation object */
static int bibentry_start(struct bibentry *iep,int fi)
{
	const int	size = sizeof(KEYVAL) ;
	int		rs ;

	memset(iep,0,sizeof(struct bibentry)) ;

	if ((rs = vecobj_start(&iep->keyvals,size,10,0)) >= 0) {
	    iep->size = 0 ;
	    iep->fi = fi ;
	}

	return rs ;
}
/* end subroutine (bibentry_start) */


static int bibentry_begin(iep,kp,kl,vp,vl)
struct bibentry	*iep ;
const char	*kp, *vp ;
int		kl, vl ;
{
	int		rs ;
	const char	*cp ;

	if (kl < 0) kl = strlen(kp) ;

/* key */

	iep->kp = NULL ;
	if ((rs = uc_mallocstrw(kp,kl,&cp)) >= 0) {
	    iep->kp = cp ;
	    iep->kl = kl ;
	    if ((rs = buffer_start(&iep->vb,80)) >= 0) {
	        if (vl < 0) vl = strlen(vp) ;
	        while (vl && CHAR_ISWHITE(*vp)) {
	            vp += 1 ;
	            vl -= 1 ;
	        }
	        rs = buffer_strw(&iep->vb,vp,vl) ;
	        if (rs < 0)
	            buffer_finish(&iep->vb) ;
	    }
	    if (rs < 0) {
	        uc_free(iep->kp) ;
	        iep->kp = NULL ;
	    }
	} /* end if (m-a) */

	return rs ;
}
/* end subroutine (bibentry_begin) */


static int bibentry_continue(struct bibentry *iep,cchar *vp,int vl)
{
	int		rs ;
	int		bl ;
	const char	*bp ;

	if (vl < 0) vl = strlen(vp) ;

	while (vl && CHAR_ISWHITE(*vp)) {
	    vp += 1 ;
	    vl -= 1 ;
	}

	rs = buffer_get(&iep->vb,&bp) ;
	bl = rs ;
	if (rs > 0) {

	    if (! CHAR_ISWHITE(bp[bl - 1])) {
	        rs = buffer_char(&iep->vb,' ') ;
	    }

	} /* end if */

	if (rs >= 0) {
	    rs = buffer_strw(&iep->vb,vp,vl) ;
	}

	return rs ;
}
/* end subroutine (bibentry_continue) */


static int bibentry_end(struct bibentry *iep)
{
	KEYVAL	kv ;
	int		rs ;
	int		rs1 ;
	int		vl ;
	int		size ;
	const char	*vp ;
	const char	*cp ;

	keyval_start(&kv) ;

	kv.kl = iep->kl ;
	kv.kp = iep->kp ;
	iep->kp = NULL ;		/* good!  zapped! */

	rs = buffer_get(&iep->vb,&vp) ;
	vl = rs ;
	if (rs >= 0) {
	    if ((rs = uc_mallocstrw(vp,vl,&cp)) >= 0) {
	        kv.vp = cp ;
	        kv.vl = vl ;
	    }
	}

	if (rs >= 0) {

	    rs = vecobj_add(&iep->keyvals,&kv) ;
	    if (rs < 0)
	        keyval_finish(&kv) ;

	    if (rs >= 0) {
	        size = keyval_size(&kv) ;
	        iep->size += size ;
	    }

	} /* end if (inserting into DB) */

	rs1 = buffer_finish(&iep->vb) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bibentry_end) */


static int bibentry_finish(iep)
struct bibentry	*iep ;
{
	KEYVAL		*kvp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (iep->kp != NULL) {
	    rs1 = uc_free(iep->kp) ;
	    if (rs >= 0) rs = rs1 ;
	    iep->kp = NULL ;
	}

	for (i = 0 ; vecobj_get(&iep->keyvals,i,&kvp) >= 0 ; i += 1) {
	    if (kvp != NULL) {
	        rs1 = keyval_finish(kvp) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end for */

	rs1 = vecobj_finish(&iep->keyvals) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bibentry_finish) */


/* load up the user-interface entry from the internal structure */
static int entry_load(ep,ebuf,ebuflen,iep)
BDB_ENT		*ep ;
char		*ebuf ;
int		ebuflen ;
struct bibentry *iep ;
{
	KEYVAL		*kvp ;
	int		rs ;
	int		tabsize, size ;
	int		bo, i, kal ;
	int		n = 0 ;
#ifdef	OPTIONAL
	int		bl ;
#endif
	const char	*(*keyvals)[2] ;
	char		*bp ;

	if ((rs = vecobj_count(&iep->keyvals)) > 0) {
	    n = rs ;

	tabsize = ((n + 1) * 2 * sizeof(const char *)) ;
	size = (tabsize + iep->size) ;

	bo = BDB_BO((ulong) ebuf) ;
	if (size <= (ebuflen - bo)) {
	    vecobj	*klp = &iep->keyvals ;
	    int		j = 0 ;

	    keyvals = (const char *(*)[2]) (ebuf + bo) ;
	    kal = tabsize ;
	    bp = (char *) (ebuf + bo + kal) ;
#ifdef	OPTIONAL
	    bl = ebuflen - bo - kal ;
#endif

	    for (i = 0 ; vecobj_get(klp,i,&kvp) >= 0 ; i += 1) {
	        if (kvp != NULL) {

	        keyvals[j][0] = bp ;
	        bp = strwcpy(bp,kvp->kp,kvp->kl) + 1 ;

#if	CF_DEBUGS
	        debugprintf("entry_load: k=%s\n",keyvals[j][0]) ;
#endif

	        if (kvp->vp != NULL) {
	            keyvals[j][1] = bp ;
	            bp = strwcpy(bp,kvp->vp,kvp->vl) + 1 ;
	        } else {
	            keyvals[j][1] = NULL ;
		}

	        j += 1 ;

	        }
	    } /* end for */

	    keyvals[j][0] = NULL ;
	    keyvals[j][1] = NULL ;

	    ep->size = size ;
	    ep->nkeys = n ;
	    ep->keyvals = keyvals ;
	    ep->fi = iep->fi ;

	} else {
	    rs = SR_OVERFLOW ;
	}

	} /* end if (vecobj_count) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (entry_load) */


static int keyval_start(KEYVAL *kvp)
{

	memset(kvp,0,sizeof(KEYVAL)) ;
	return SR_OK ;
}
/* end subroutine (keyval_start) */


static int keyval_size(KEYVAL *kvp)
{
	int		rs = SR_OK ;
	int		size = 2 ;

	if (kvp->kp != NULL)
	    size += kvp->kl ;

	if (kvp->vp != NULL)
	    size += kvp->vl ;

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (keyval_size) */


static int keyval_finish(KEYVAL *kvp)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (kvp->kp != NULL) {
	    rs1 = uc_free(kvp->kp) ;
	    if (rs >= 0) rs = rs1 ;
	    kvp->kp = NULL ;
	}

	if (kvp->vp != NULL) {
	    rs1 = uc_free(kvp->vp) ;
	    if (rs >= 0) rs = rs1 ;
	    kvp->vp = NULL ;
	}

	return rs ;
}
/* end subroutine (keyval_finish) */


/* is there a key? */
static int iskey(cchar *lp,int ll)
{
	int		ch ;
	int		f = FALSE ;

	if (ll >= 2) {
	    f = (lp[0] == '%') ;
	    if (f) {
	        ch = lp[1] & 0xff ;
	        f = isalphalatin(ch) ;
	    }
	}

	return f ;
}
/* end subroutine (iskey) */


