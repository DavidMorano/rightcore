/* bdb */

/* Bibliographical DataBase */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1987-09-10, David A­D­ Morano
	This code module was originally written.

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
        document creator -- not some centralized entiry. The older centralized
        model reflected more the use in the corporate world (where different
        people create BIB entries) than in the more "modern"
        personal-responsibility type of world! :-) Anyway, this is the way the
        gods seem to now want to do things. Deal with it!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

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

#define	BDB_MAGIC	27
#define	BDB_DEFFILES	10		/* default files */
#define	BDB_DEFENTRIES	40		/* default entries */

#define	BDB_KA		sizeof(char *(*)[2])
#define	BDB_BO(v)		\
	((BDB_KA - ((v) % BDB_KA)) % BDB_KA)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	BUFLEN		(20 * LINEBUFLEN)
#define	CITEBUFLEN	LINEBUFLEN

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	TWOCHARS(a,b)	(((a) << 8) + (b))


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	sibreak(const char *,int,char *,const char **) ;
extern int	sisub(const char *,int,const char *,const char **) ;
extern int	isalphalatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

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
static int bdb_keyinsert(BDB *,int,offset_t,offset_t,const char *) ;
static int bdb_readentry(BDB *,struct bdb_key *,BDB_ENT *,char *,int) ;

static int bdbfile_start(struct bdb_file *,const char *) ;
static int bdbfile_finish(struct bdb_file *) ;
static int bdbfile_indexed(struct bdb_file *) ;

static int bdbkey_start(struct bdb_key *,int,const char *,offset_t,int) ;
static int bdbkey_finish(struct bdb_key *) ;

static int bibentry_start(struct bibentry *,int) ;
static int bibentry_begin(struct bibentry *,const char *,int,
const char *,int) ;
static int bibentry_continue(struct bibentry *,const char *,int) ;
static int bibentry_end(struct bibentry *) ;
static int bibentry_finish(struct bibentry *) ;

static int	entry_load(BDB_ENT *,char *,int,struct bibentry *) ;

static int	keyval_start(struct keyval *) ;
static int	keyval_size(struct keyval *) ;
static int	keyval_finish(struct keyval *) ;

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


int bdb_start(op,qkey,opts)
BDB		*op ;
const char	qkey[] ;
int		opts ;
{
	int	rs ;
	int	size ;
	int	vecopts ;


	if (op == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(BDB)) ;

#if	CF_DEBUGS
	debugprintf("bdb_start: qkey=%s\n",qkey) ;
#endif

	if ((qkey == NULL) || (qkey[0] == '\0'))
	    qkey = BDB_QUERYKEY ;

/* store away stuff */

	op->opts = opts ;

	{
	    int		cl = strlen(qkey) ;
	    const char	*cp ;
	    if ((rs = uc_mallocstrw(qkey,cl,&cp)) >= 0) {
	        op->qkbuf = cp ;
	        op->qklen = cl ;
	    }
	}

#if	CF_DEBUGS
	debugprintf("bdb_start: uc_mallocstrw() rs=%d\n",rs) ;
#endif
	if (rs < 0) goto bad0 ;

#if	CF_DEBUGS
	debugprintf("bdb_start: qkey=%s opts=\\x%02x\n",
	    op->qkbuf,op->opts) ;
#endif

/* set up the other structures needed */

	size = sizeof(struct bdb_file) ;
	vecopts = VECOBJ_OORDERED | VECOBJ_OSTATIONARY ;
	rs = vecobj_start(&op->files,size,BDB_DEFFILES,vecopts) ;
	if (rs < 0)
	    goto bad1 ;

	rs = hdb_start(&op->keys,BDB_DEFENTRIES,1,NULL,NULL) ;
	if (rs < 0)
	    goto bad2 ;

/* finish up */

	op->magic = BDB_MAGIC ;

ret0:
	return rs ;

/* bad stuff */
bad2:
	vecobj_finish(&op->files) ;

bad1:
	if (op->qkbuf != NULL) {
	    uc_free(op->qkbuf) ;
	    op->qkbuf = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (bdb_start) */


int bdb_finish(op)
BDB		*op ;
{
	struct bdb_file	*bfp ;
	struct bdb_key	*bkp ;

	HDB_CUR		cur ;
	HDB_DATUM	key, value ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

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

	for (i = 0 ; vecobj_get(&op->files,i,&bfp) >= 0 ; i += 1) {
	    if (bfp == NULL) continue ;

	    rs1 = bdbfile_finish(bfp) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

/* free up the file entry container */

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bdb_finish) */


int bdb_add(op,fname)
BDB		*op ;
const char	fname[] ;
{
	struct ustat	sb ;

	int	rs ;


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
	            if (rs < 0)
	                bdbfile_finish(&bfe) ;
	        } /* end if (bdbfile) */
	    } else
	        rs = SR_ISDIR ;
	} /* end if (stat) */

	return rs ;
}
/* end subroutine (bdb_add) */


#ifdef	COMMENT

/* find a paramter by name (path of the bibliography) */
int bdb_find(op,name,len,rpp)
BDB		*op ;
char		name[] ;
char		len ;
BDB_VALUE	**rpp ;
{
	HDB_DATUM	key, value ;

	int	rs ;
	int	len = 0 ;


#if	CF_DEBUGS
	debugprintf("bdbfind: entered, n=%s\n",name) ;
#endif

	key.buf = name ;
	key.len = (len >= 0) ? len : strlen(name) ;

	rs = hdb_fetch(&op->keys,key,NULL,&value) ;
	if (rs < 0)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("bdbfind: got it\n") ;
#endif

	if (rpp != NULL)
	    *rpp = (struct bdb__value *) value.buf ;

	len = value.len ;

ret0:
	return (rs >= 0) ? len : rs ;
}
/* end subroutine (bdb_find) */

#endif /* COMMENT */


int bdb_count(op)
BDB		*op ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	rs = hdb_count(&op->keys) ;

	return rs ;
}
/* end subroutine (bdb_count) */


#ifdef	COMMENT

/* get the entry under the current cursor */
int bdb_getname(op,cp,rpp)
BDB		*op ;
BDB_CUR		*cp ;
char		**rpp ;
{
	HDB_DATUM	key, value ;

	int	rs ;


#if	CF_DEBUGS
	debugprintf("bdbgetkey: entered\n") ;
#endif

	if ((rs = hdb_enum(&op->keys,(HDB_CUR *) cp,&key,&value)) >= 0) {

#if	CF_DEBUGS
	    debugprintf("bdbgetkey: loop\n") ;
#endif

	    rs = -1 ;
	    if ((key.buf != NULL) && (key.len > 0)) {

	        if (rpp != NULL)
	            *rpp = key.buf ;

	        rs = key.len ;
	    }

	} /* end if */

#if	CF_DEBUGS
	debugprintf("bdbgetkey: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdb_getname) */

#endif /* COMMENT */


/* delete the entry under the current cursor */
int bdb_delcur(op,curp,f_adv)
BDB		*op ;
BDB_CUR		*curp ;
int		f_adv ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_INVALID ;

	rs = hdb_delcur(&op->keys,&curp->cur,f_adv) ;

#if	CF_DEBUGS
	debugprintf("bdbdelcursor: hdb_delcur() rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bdb_delcur) */


/* make a query */
int bdb_query(op,citekey,bep,rbuf,rlen)
BDB		*op ;
const char	citekey[] ;
BDB_ENT	*bep ;
char		rbuf[] ;
int		rlen ;
{
	struct bdb_key	bke ;

	HDB_DATUM	key ;

	int	rs ;
	int	rs1 ;
	int	n ;


#if	CF_DEBUGS
	debugprintf("bdb_query: entered citekey=%s\n",citekey) ;
#endif

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if ((citekey == NULL) || (citekey[0] == '\0'))
	    return SR_INVALID ;

	if (bep == NULL)
	    return SR_FAULT ;

	if (rbuf == NULL)
	    return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("bdb_query: continuing\n") ;
#endif

	rs = vecobj_count(&op->files) ;
	n = rs ;
	if (rs < 0)
	    goto ret0 ;

	if (n == 0) {
	    rs = SR_NOTFOUND ;
	    goto ret0 ;
	}

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
	        debugprintf("bdb_query: UNIQ bdb_fileindex() rs=%d\n",rs1) ;
#endif

	        if (rs1 < 0)
	            break ;

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

	    if (rs > 1)
	        rs = SR_NOTUNIQ ;

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

	    if (rs >= 0)
	        break ;

	} /* end while */

#if	CF_DEBUGS && 0
	{
	    struct bdb_key	*bkp ;
	    HDB_CUR	cur ;
	    HDB_DATUM	value ;
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

	    rs = bdb_readentry(op,&bke,bep,rbuf,rlen) ;
	    n = rs ;

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("bdb_query: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bdb_query) */


int bdb_curbegin(op,curp)
BDB		*op ;
BDB_CUR		*curp ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	rs = hdb_curbegin(&op->keys,(HDB_CUR *) curp) ;

	return rs ;
}
/* end subroutine (bdb_curbegin) */


int bdb_curend(op,curp)
BDB		*op ;
BDB_CUR		*curp ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if (curp == NULL)
	    return SR_FAULT ;

	rs = hdb_curend(&op->keys,(HDB_CUR *) curp) ;

	return rs ;
}
/* end subroutine (bdb_curend) */


/* private subroutines */


static int bdb_scan(op,key,ubkp)
BDB		*op ;
HDB_DATUM	key ;
struct bdb_key	*ubkp ;
{
	struct bdb_key	*bkp ;

	HDB_CUR		cur ;
	HDB_DATUM	value ;

	int	rs ;
	int	fi = -1 ;
	int	c = 0 ;


	if ((rs = hdb_curbegin(&op->keys,&cur)) >= 0) {

	    while (hdb_fetch(&op->keys,key,&cur,&value) >= 0) {

	        c += 1 ;
	        bkp = (struct bdb_key *) value.buf ;
	        if (bkp->fi > fi) {
	            *ubkp = *bkp ;
	            fi = bkp->fi ;
	        }

	    } /* end while (looping through keys) */

	    hdb_curend(&op->keys,&cur) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("bdb_scan: ret fi=%d c=%u\n",fi,c) ;
#endif

	if ((rs >= 0) && (fi < 0)) rs = SR_NOTFOUND ;

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (bdb_scan) */


/* index one BDB database file (the most recent unindexed file) */
static int bdb_fileindex(op,n)
BDB		*op ;
int		n ;
{
	struct bdb_file	*bfp = NULL ;

	int	rs = SR_OK ;
	int	fi = 0 ;


#if	CF_DEBUGS
	debugprintf("bdb_fileindex: entered n=%u\n",n) ;
#endif

	if (n == 0)
	    return SR_NOTFOUND ;

/* find the youngest file that has not been indexed */

	rs = SR_NOTFOUND ;
	for (fi = (n - 1) ; 
	    (fi >= 0) && ((rs = vecobj_get(&op->files,fi,&bfp)) >= 0) ; 
	    fi -= 1) {

	    if (bfp == NULL) continue ;

	    if (! bfp->f_indexed)
	        break ;

	} /* end for */

#if	CF_DEBUGS
	debugprintf("bdb_fileindex: file to index fi=%u\n",fi) ;
#endif

/* something to index? */

	if ((rs >= 0) && (fi >= 0) && (bfp != NULL)) {
	    rs = bdb_fileproc(op,fi,bfp) ;
	}

#if	CF_DEBUGS
	debugprintf("bdb_fileindex: ret rs=%d fi=%u\n",rs,fi) ;
#endif

	return (rs >= 0) ? fi : rs ;
}
/* end subroutine (bdb_fileindex) */


static int bdb_fileproc(op,fi,bfp)
BDB		*op ;
int		fi ;
struct bdb_file	*bfp ;
{
	SBUF	citekey ;

	bfile	bibfile ;

	offset_t	citeoff ;
	offset_t	offset = 0 ;

	const int	llen = LINEBUFLEN ;
	const int	clen = CITEBUFLEN  ;

	int	rs ;
	int	len ;
	int	state ;
	int	ll, cl, kl ;
	int	f_bol, f_eol ;
	int	f_inkey = FALSE ;

	const char	*lp, *cp, *kp ;

	char	lbuf[LINEBUFLEN + 1] ;
	char	cbuf[CITEBUFLEN + 1] ;


	op->unindexed -= 1 ;
	if ((rs = bopen(&bibfile,bfp->fname,"r",0666)) >= 0) {

	    state = state_search ;
	    f_bol = TRUE ;
	    while ((rs = breadline(&bibfile,lbuf,llen)) > 0) {
	        len = rs ;

	        f_eol = (lbuf[len - 1] == '\n') ;

	        lp = lbuf ;
	        ll = (f_eol) ? (len - 1) : len ;

#if	CF_DEBUGS
	        debugprintf("bdb_fileindex: line=>%t<\n",
	            lbuf,
	            ((f_eol) ? (len - 1) : len)) ;
	        debugprintf("bdb_fileindex: f_bol=%u state=%s(%u)\n",
	            f_bol,states[state],state) ;
#endif

	        switch (state) {

	        case state_search:
	            if (! f_bol)
	                break ;

	            if (! iskey(lbuf,len))
	                break ;

	            state = state_have ;
	            citeoff = offset ;

/* fall-through from previous case */
/*FALLTHROUGH*/
	        case state_have:
	            if (f_bol && iskey(lbuf,len)) {

#if	CF_DEBUGS
	                debugprintf("bdb_fileindex: key=%c\n",lbuf[1]) ;
	                debugprintf("bdb_fileindex: f_inkey=%u\n",f_inkey) ;
#endif

	                if (f_inkey) {
	                    f_inkey = FALSE ;
	                    sbuf_finish(&citekey) ;
	                }

	                lp += 1 ;
	                ll -= 1 ;

	                cl = nextfield(lp,ll,&cp) ;

#if	CF_DEBUGS
	                debugprintf("bdb_fileindex: cl=%u c=%t\n",cl,cp,cl) ;
	                debugprintf("bdb_fileindex: ql=%u q=%t\n",
	                    op->qklen,op->qkbuf, op->qklen) ;
#endif

	                if ((strncmp(cp,op->qkbuf,op->qklen) == 0) &&
	                    (cl == op->qklen)) {

	                    ll -= ((cp + cl) - lp) ;
	                    lp = (cp + cl) ;

#if	CF_DEBUGS
	                    debugprintf("bdb_fileindex: rline=%t\n",lp,ll) ;
#endif

	                    kl = nextfield(lp,ll,&kp) ;

#if	CF_DEBUGS
	                    debugprintf("bdb_fileindex: citekey start\n") ;
	                    debugprintf("bdb_fileindex: k=%t\n",kp,kl) ;
#endif

	                    rs = sbuf_start(&citekey,cbuf,clen) ;
	                    f_inkey = (rs >= 0) ;

	                    if ((rs >= 0) && (kl > 0))
	                        sbuf_strw(&citekey,kp,kl) ;

	                } /* end if (found citekey) */

	            } else if (f_inkey) {

	                if (sbuf_getlen(&citekey) > 0)
	                    sbuf_char(&citekey,' ') ;

	                sbuf_strw(&citekey,lbuf,len) ;

	            } /* end if */

/* are we leaving an entry? */

#if	CF_DEBUGS
	            debugprintf("bdb_fileindex: leaving?\n") ;
	            debugprintf("bdb_fileindex: len=%d\n",len) ;
	            debugprintf("bdb_fileindex: lbuf=%s\n",lbuf) ;
#endif

	            if ((rs >= 0) && ((len == 0) || (lbuf[0] == '\n'))) {

#if	CF_DEBUGS
	                debugprintf("bdb_fileindex: end-of-entry \n") ;
	                debugprintf("bdb_fileindex: f_inkey=%u\n",f_inkey) ;
#endif

	                state = state_search ;
	                if (f_inkey) {
	                    f_inkey = FALSE ;
	                    sbuf_finish(&citekey) ;
	                }

#if	CF_DEBUGS
	                debugprintf("bdb_fileindex: cbuf=%s\n",cbuf) ;
#endif

	                if (cbuf[0] != '\0') {

#if	CF_DEBUGS
	                    debugprintf("bdb_fileindex: inserting key=%s\n",
	                        cbuf) ;
	                    debugprintf("bdb_fileindex: fi=%u offset=%lu "
	                        "citeoff=%lu\n",
	                        fi,offset,citeoff) ;
#endif

	                    rs = bdb_keyinsert(op,fi,
	                        citeoff, offset, cbuf) ;

#if	CF_DEBUGS
	                    debugprintf("bdb_fileindex: bdb_keyinsert() "
	                        "rs=%d\n",rs) ;
#endif

	                    cbuf[0] = '\0' ;

	                }

	            } /* end if (state transition) */

	            break ;

	        } /* end switch */

	        offset += len ;
	        f_bol = f_eol ;
	        if (rs < 0) break ;
	    } /* end while */

	    bclose(&bibfile) ;
	} /* end if (opened file) */

	if (f_inkey)
	    sbuf_finish(&citekey) ;

/* mark this file as being indexed */

	if (rs >= 0)
	    bdbfile_indexed(bfp) ;

	return rs ;
}
/* end subroutine if (bdb_fileproc) */


/* insert a citation-key into the BIB key DB */
static int bdb_keyinsert(op,fi, citeoff, offset, cbuf)
BDB		*op ;
int		fi ;
offset_t	offset, citeoff ;
const char	cbuf[] ;
{
	struct bdb_key		*bkp ;

	int	rs ;
	int	size ;
	int	citelen = 0 ;


#if	CF_DEBUGS
	debugprintf("bdb_keyinsert: cbuf=%s\n",cbuf) ;
#endif

	size = sizeof(struct bdb_key) ;
	if ((rs = uc_malloc(size,&bkp)) >= 0) {

	    citelen = offset - citeoff ;
	    if ((rs = bdbkey_start(bkp,fi,cbuf,citeoff,citelen)) >= 0) {
	        HDB_DATUM	key, value ;

	        key.buf = bkp->citekey ;
	        key.len = strlen(bkp->citekey) ;

	        value.buf = bkp ;
	        value.len = size ;

	        rs = hdb_store(&op->keys,key,value) ;
	        if (rs < 0)
	            bdbkey_finish(bkp) ;

	    } /* end if */

	    if (rs < 0)
	        uc_free(bkp) ;

	} /* end if (memory-allocation) */

#if	CF_DEBUGS
	debugprintf("bdb_keyinsert: ret rs=%s clen=%u\n",rs,citelen) ;
#endif

	return (rs >= 0) ? citelen : rs ;
}
/* end subroutine (bdb_keyinsert) */


/* read a BIB entry */
static int bdb_readentry(op,bkp,bep,bebuf,belen)
BDB		*op ;
struct bdb_key	*bkp ;
BDB_ENT	*bep ;
char		bebuf[] ;
int		belen ;
{
	struct bdb_file	*bfp ;

	struct bibentry	ie ;

	bfile		bibfile ;

	const int	llen = 0 ;

	int	rs ;
	int	rlen, len ;
	int	ll, cl ;
	int	n = 0 ;
	int	f_bol, f_eol ;
	int	f_inkey ;

	const char	*cp ;
	const char	*lp ;

	char	lbuf[LINEBUFLEN + 1] ;


#ifdef	COMMENT
	if (op == NULL) return SR_FAULT ;
	if (bkp == NULL) return SR_FAULT ;
	if (bep == NULL) return SR_FAULT ;
	if (bebuf == NULL) return SR_FAULT ;
#endif /* COMMENT */

	memset(bep,0,sizeof(BDB_ENT)) ;

	rs = vecobj_get(&op->files,bkp->fi,&bfp) ;
	if (rs < 0)
	    goto ret0 ;

	if ((rs = bibentry_start(&ie,bkp->fi)) >= 0) {

	    if ((rs = bopen(&bibfile,bfp->fname,"r",0666)) >= 0) {

	        bseek(&bibfile,bkp->offset,SEEK_SET) ;

	        f_inkey = FALSE ;
	        f_bol = TRUE ;
	        rlen = 0 ;
	        while ((rlen < bkp->len) &&
	            ((rs = breadline(&bibfile,lbuf,llen)) > 0)) {
	            len = rs ;

	            f_eol = (lbuf[len - 1] == '\n') ;
	            lp = lbuf ;
	            ll = (f_eol) ? (len - 1) : len ;

	            if (f_bol && ((ll == 0) || (lbuf[0] == '\n')))
	                break ;

	            if (f_bol && (lbuf[0] == '%')) {

	                if (f_inkey)
	                    rs = bibentry_end(&ie) ;

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

	        if ((rs >= 0) && f_inkey)
	            rs = bibentry_end(&ie) ;

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

	        bclose(&bibfile) ;
	    } /* end if (file-open) */

	    bibentry_finish(&ie) ;
	} /* end if (bib-entry) */

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bdb_readentry) */


/* operate on the BDBFILE object */
static int bdbfile_start(fp,fname)
struct bdb_file	*fp ;
const char	fname[] ;
{
	int	rs ;

	const char	*cp ;


	memset(fp,0,sizeof(struct bdb_file)) ;

	fp->f_indexed = 0 ;
	rs = uc_mallocstrw(fname,-1,&cp) ;
	if (rs >= 0) fp->fname = cp ;

	return rs ;
}
/* end subroutine bdbfile_start) */


static int bdbfile_finish(fp)
struct bdb_file	*fp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (fp == NULL)
	    return SR_FAULT ;

	if (fp->fname != NULL) {
	    rs1 = uc_free(fp->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    fp->fname = NULL ;
	}

	return rs ;
}
/* end subroutine bdbfile_finish) */


static int bdbfile_indexed(fp)
struct bdb_file	*fp ;
{


	if (fp == NULL)
	    return SR_FAULT ;

	fp->f_indexed = TRUE ;
	return SR_OK ;
}
/* end subroutine bdbfile_indexed) */


/* operate on the BDBKEY object */
static int bdbkey_start(bkp,fi,citekey,offset,len)
struct bdb_key	*bkp ;
int		fi ;
const char	citekey[] ;
offset_t	offset ;
int		len ;
{
	int	rs ;

	const char	*cp ;


#if	CF_DEBUGS
	debugprintf("bdb/bdbkey_start: fi=%u\n",fi) ;
#endif

	memset(bkp,0,sizeof(struct bdb_key)) ;

	bkp->fi = fi ;
	bkp->offset = offset ;
	bkp->len = len ;
	rs = uc_mallocstrw(citekey,-1,&cp) ;
	if (rs >= 0) bkp->citekey = cp ;

	return rs ;
}

static int bdbkey_finish(bkp)
struct bdb_key	*bkp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (bkp == NULL)
	    return SR_FAULT ;

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
static int bibentry_start(iep,fi)
struct bibentry	*iep ;
int		fi ;
{
	const int	size = sizeof(struct keyval) ;

	int	rs ;


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
	int	rs ;

	const char	*cp ;


	if (kl < 0)
	    kl = strlen(kp) ;

/* key */

	iep->kp = NULL ;
	rs = uc_mallocstrw(kp,kl,&cp) ;
	if (rs < 0) goto bad0 ;

/* enter the key */

	iep->kp = cp ;
	iep->kl = kl ;

/* value */

	rs = buffer_start(&iep->vb,80) ;
	if (rs < 0)
	    goto bad1 ;

/* enter the value */

	if (vl < 0)
	    vl = strlen(vp) ;

	while (vl && CHAR_ISWHITE(*vp)) {
	    vp += 1 ;
	    vl -= 1 ;
	}

	rs = buffer_strw(&iep->vb,vp,vl) ;
	if (rs < 0)
	    goto bad2 ;

ret0:
	return rs ;

/* bad stuff */
bad2:
	buffer_finish(&iep->vb) ;

bad1:
	if (iep->kp != NULL) {
	    uc_free(iep->kp) ;
	    iep->kp = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (bibentry_start) */


static int bibentry_continue(iep,vp,vl)
struct bibentry	*iep ;
const char	*vp ;
int		vl ;
{
	int	rs ;
	int	bl ;

	const char	*bp ;


	if (vl < 0)
	    vl = strlen(vp) ;

	while (vl && CHAR_ISWHITE(*vp)) {
	    vp += 1 ;
	    vl -= 1 ;
	}

	rs = buffer_get(&iep->vb,&bp) ;
	bl = rs ;
	if (rs > 0) {

	    if (! CHAR_ISWHITE(bp[bl - 1]))
	        rs = buffer_char(&iep->vb,' ') ;

	} /* end if */

	if (rs >= 0)
	    rs = buffer_strw(&iep->vb,vp,vl) ;

	return rs ;
}
/* end subroutine (bibentry_continue) */


static int bibentry_end(iep)
struct bibentry	*iep ;
{
	struct keyval	kv ;

	int	rs ;
	int	rs1 ;
	int	vl ;
	int	size ;

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
	struct keyval	*kvp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	if (iep->kp != NULL) {
	    rs1 = uc_free(iep->kp) ;
	    if (rs >= 0) rs = rs1 ;
	    iep->kp = NULL ;
	}

	for (i = 0 ; vecobj_get(&iep->keyvals,i,&kvp) >= 0 ; i += 1) {
	    if (kvp == NULL) continue ;

	    rs1 = keyval_finish(kvp) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	rs1 = vecobj_finish(&iep->keyvals) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bibentry_finish) */


/* load up the user-interface entry from the internal structure */
static int entry_load(ep,ebuf,ebuflen,iep)
BDB_ENT		*ep ;
char			ebuf[] ;
int			ebuflen ;
struct bibentry		*iep ;
{
	struct keyval	*kvp ;

	int	rs ;
	int	tabsize, size ;
	int	n ;
	int	bo, i, j, kal ;
#ifdef	OPTIONAL
	int	bl ;
#endif

	const char	*(*keyvals)[2] ;

	char	*bp ;


	rs = vecobj_count(&iep->keyvals) ;
	n = rs ;
	if (rs < 0)
	    goto ret0 ;

	tabsize = ((n + 1) * 2 * sizeof(char *)) ;
	size = tabsize + iep->size ;

	bo = BDB_BO((ulong) ebuf) ;
	if (size <= (ebuflen - bo)) {

	    keyvals = (const char *(*)[2]) (ebuf + bo) ;
	    kal = tabsize ;
	    bp = (char *) (ebuf + bo + kal) ;
#ifdef	OPTIONAL
	    bl = ebuflen - bo - kal ;
#endif

	    j = 0 ;
	    for (i = 0 ; vecobj_get(&iep->keyvals,i,&kvp) >= 0 ; i += 1) {
	        if (kvp == NULL) continue ;

	        keyvals[j][0] = bp ;
	        bp = strwcpy(bp,kvp->kp,kvp->kl) + 1 ;

#if	CF_DEBUGS
	        debugprintf("entry_load: k=%s\n",keyvals[j][0]) ;
#endif

	        if (kvp->vp != NULL) {

	            keyvals[j][1] = bp ;
	            bp = strwcpy(bp,kvp->vp,kvp->vl) + 1 ;

	        } else
	            keyvals[j][1] = NULL ;

	        j += 1 ;

	    } /* end for */

	    keyvals[j][0] = NULL ;
	    keyvals[j][1] = NULL ;

	    ep->size = size ;
	    ep->nkeys = n ;
	    ep->keyvals = keyvals ;
	    ep->fi = iep->fi ;

	} else
	    rs = SR_OVERFLOW ;

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (entry_load) */


static int keyval_start(kvp)
struct keyval	*kvp ;
{


	memset(kvp,0,sizeof(struct keyval)) ;
	return SR_OK ;
}
/* end subroutine (keyval_start) */


static int keyval_size(kvp)
struct keyval	*kvp ;
{
	int	rs = SR_OK ;
	int	size = 2 ;


	if (kvp->kp != NULL)
	    size += kvp->kl ;

	if (kvp->vp != NULL)
	    size += kvp->vl ;

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (keyval_size) */


static int keyval_finish(kvp)
struct keyval	*kvp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


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
static int iskey(lp,ll)
const char	lp[] ;
int		ll ;
{
	int	ch ;
	int	f = FALSE ;


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



