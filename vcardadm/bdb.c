/* bdb */

/* Bibliographical DataBase */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1987-09-10, David A­D­ Morano

	This code module was originally written.


	= 1998-09-10, David A­D­ Morano

	This module was changed to serve in the REFERM program.


	= 2005-10-01, David A­D­ Morano

	This was changed to work in the MMCITE program.  The old REFERM
	program is really obsolete.  It used a database lookup strategy
	to remote databases.  The high-level problem is: what to do
	if the cited BIB entry isn't found?  How does a maintainer of
	the present (local) document know what that BIB entry was?
	The new strategy (implemented by the MMCITE program) is more
	like what is done with BibTeX in the TeX (or LaTeX) world.
	All BIB databases are really expected to be maintained by the
	document creator -- not some centralized entiry.  The older
	centralized model reflected more the use in the corporate world
	(where different people create BIB entries) than in the more
	"modern" personal-responsibility type of world! :-)  Anyway,
	this is the way the gods seem to now want to do things.  Deal
	with it!


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************

	This code object module implements a little DB query facility.
	The database is a set of files that contain bibliographical
	entries in the "REFER" format.

	Queries to the database that succeed return a structure with
	the elements of the bibliographic entry.

	All queries are to database files that were referenced last
	(most previously) in the processing of the document.  The idea
	is that a more recent reference to a database file means that
	it is the preferred one to use since it is closest to the user
	(or the most preferred by the user).

	A small nice thing about this database is that database files
	(files containing "REFER"-formatted entries) are not indexed
	unless they are needed (due to a failure to find the query in
	existing indexed files).  Only after all database files have
	been indexed and scanned to try to satisfy the query (and the
	request os not found), does the query fail.


*********************************************************************/


#define	BDB_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<char.h>
#include	<hdb.h>
#include	<sbuf.h>
#include	<storeitem.h>
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

#define	BDB_QUERYKEY	"Q"

#define	TWOCHARS(a,b)	(((a) << 8) + (b))


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	nextfield(const char *,int,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct keyval {
	const char	*kp, *vp ;
	int		kl, vl ;
} ;

struct bibentry {
	const char	*kp ;
	VECOBJ	keyvals ;
	BUFFER	vb ;
	int	size ;
	int	kl ;
	int	fi ;
} ;


/* forward references */

static int bdb_scan(BDB *,HDB_DATUM,struct bdb_key *) ;
static int bdb_fileindex(BDB *,int) ;
static int bdb_keyinsert(BDB *,int,offset_t,offset_t,const char *) ;
static int bdb_readentry(BDB *,struct bdb_key *,BDB_ENT *,char *,int) ;

static int bdbfile_init(struct bdb_file *,const char *) ;
static int bdbfile_free(struct bdb_file *) ;
static int bdbfile_indexed(struct bdb_file *) ;

static int bdbkey_init(struct bdb_key *,int,const char *,offset_t,int) ;
static int bdbkey_free(struct bdb_key *) ;

static int bibentry_init(struct bibentry *,int) ;
static int bibentry_start(struct bibentry *,const char *,int,
		const char *,int) ;
static int bibentry_continue(struct bibentry *,const char *,int) ;
static int bibentry_end(struct bibentry *) ;
static int bibentry_free(struct bibentry *) ;

static int	entry_load(BDB_ENT *,char *,int,struct bibentry *) ;

static int	keyval_init(struct keyval *) ;
static int	keyval_size(struct keyval *) ;
static int	keyval_free(struct keyval *) ;

static int	iskey(const char *,int) ;


/* local variables */

enum states {
	state_search,
	state_have,
	state_overlast
} ;


/* exported subroutines */


int bdb_init(op,qkey,opts)
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

	if ((qkey == NULL) || (qkey[0] == '\0'))
	    qkey = BDB_QUERYKEY ;

/* store away stuff */

	op->opts = opts ;
	rs = uc_mallocstrw(qkey,-1,&op->qkey) ;
	op->qkeylen = rs ;
	if (rs < 0)
	    goto bad0 ;

#if	CF_DEBUGS
	debugprintf("bdb_init: qkey=%s opts=\\x%02x\n",
	    op->qkey,op->opts) ;
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
	if (op->qkey != NULL)
	    uc_free((void *) op->qkey) ;

	op->qkey = NULL ;

bad0:
	goto ret0 ;
}
/* end subroutine (bdb_init) */


int bdb_free(op)
BDB		*op ;
{
	struct bdb_file	*bfp ;
	struct bdb_key	*bkp ;
	HDB_CUR		cur ;
	HDB_DATUM	key, value ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if (op->qkey != NULL) {
	    uc_free(op->qkey) ;
	    op->qkey = NULL ;
	}

/* free up all key entries */

	hdb_curbegin(&op->keys,&cur) ;

	while (hdb_enum(&op->keys,&cur,&key,&value) >= 0) {

	    bkp = (struct bdb_key *) value.buf ;
	    bdbkey_free(bkp) ;

	    uc_free(bkp) ;

	} /* end while */

	hdb_curend(&op->keys,&cur) ;

/* free up the key container */

	hdb_finish(&op->keys) ;

/* free up all file entries */

	for (i = 0 ; vecobj_get(&op->files,i,&bfp) >= 0 ; i += 1) {
	    if (bfp == NULL) continue ;

	    bdbfile_free(bfp) ;

	} /* end for */

/* free up the file entry container */

	rs1 = vecobj_finish(&op->files) ;
	if (rs >= 0) rs = rs1 ;

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (bdb_free) */


int bdb_add(op,fname)
BDB		*op ;
const char	fname[] ;
{
	struct bdb_file	bfe ;

	struct ustat	sb ;

	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

/* check if the file actually exists */

	rs = u_stat(fname,&sb) ;
	if (rs < 0)
	    goto ret0 ;

	if (S_ISDIR(sb.st_mode)) {
	    rs = SR_ISDIR ;
	    goto ret0 ;
	}

	rs = bdbfile_init(&bfe,fname) ;
	if (rs < 0)
	    goto bad0 ;

	rs = vecobj_add(&op->files,&bfe) ;
	if (rs < 0)
	    goto bad1 ;

	op->unindexed += 1 ;

ret0:
bad0:
	return rs ;

/* bad stuff */
bad1:
	bdbfile_free(&bfe) ;

	goto ret0 ;
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

	if ((rs = hdb_fetch(&op->keys,key,NULL,&value)) < 0)
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
BDB_CUR	*cp ;
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
BDB_CUR	*curp ;
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
int bdb_query(op,citekey,bep,buf,buflen)
BDB		*op ;
const char	citekey[] ;
BDB_ENT	*bep ;
char		buf[] ;
int		buflen ;
{
	struct bdb_key	bke ;

	HDB_DATUM	key ;

	int	rs, rs1 ;
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

	if (buf == NULL)
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
	HDB_DATUM	value ;
	HDB_CUR	cur ;


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

	    rs = bdb_readentry(op,&bke,bep,buf,buflen) ;
	    n = rs ;

	} /* end if */

ret0:

#if	CF_DEBUGS
	debugprintf("bdb_query: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bdb_query) */


int bdb_curbegin(op,cp)
BDB		*op ;
BDB_CUR	*cp ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if (cp == NULL)
	    return SR_FAULT ;

	rs = hdb_curbegin(&op->keys,(HDB_CUR *) cp) ;

	return rs ;
}
/* end subroutine (bdb_curbegin) */


int bdb_curend(op,cp)
BDB		*op ;
BDB_CUR	*cp ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != BDB_MAGIC)
	    return SR_NOTOPEN ;

	if (cp == NULL)
	    return SR_FAULT ;

	rs = hdb_curend(&op->keys,(HDB_CUR *) cp) ;

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

	HDB_CUR	cur ;

	HDB_DATUM	value ;

	int	fi ;
	int	c ;


	c = 0 ;
	fi = -1 ;
	hdb_curbegin(&op->keys,&cur) ;

	while (hdb_fetch(&op->keys,key,&cur,&value) >= 0) {

	    c += 1 ;
	    bkp = (struct bdb_key *) value.buf ;
	    if (bkp->fi > fi) {
	        *ubkp = *bkp ;
	        fi = bkp->fi ;
	    }

	} /* end while (looping through keys) */

	hdb_curend(&op->keys,&cur) ;

#if	CF_DEBUGS
	debugprintf("bdb_scan: ret fi=%d c=%u\n",fi,c) ;
#endif

	return (fi >= 0) ? c : SR_NOTFOUND ;
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
	    return SR_NOENT ;

/* find the youngest file that has not been indexed */

	rs = SR_NOENT ;
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

	    SBUF	citekey ;

	    bfile	bibfile ;

	    offset_t	offset, citeoff ;

	    int		len ;
	    int		state ;
	    int		ll, cl, kl ;
	    int		f_bol, f_eol ;
	    int		f_inkey = FALSE ;

	    char	linebuf[LINEBUFLEN + 1] ;
	    char	citebuf[CITEBUFLEN + 1] ;
	    char	*lp, *cp, *kp ;


	    op->unindexed -= 1 ;
	    rs = bopen(&bibfile,bfp->fname,"r",0666) ;

	    if (rs >= 0) {

	        offset = 0 ;
	        state = state_search ;
	        f_bol = TRUE ;
	        while ((rs = breadline(&bibfile,linebuf,LINEBUFLEN)) > 0) {

	            len = rs ;
	            f_eol = (linebuf[len - 1] == '\n') ;
	            lp = linebuf ;
	            ll = (f_eol) ? (len - 1) : len ;

#if	CF_DEBUGS
	            debugprintf("bdb_fileindex: line=>%t<\n",
	                linebuf,
	                ((f_eol) ? (len - 1) : len)) ;
	            debugprintf("bdb_fileindex: f_bol=%u state=%u\n",
			f_bol,state) ;
#endif

	            switch (state) {

	            case state_search:
	                if (! f_bol)
	                    break ;

	                if (! iskey(linebuf,len))
	                    break ;

	                state = state_have ;
	                citeoff = offset ;

/* fall-through from previous case */
/*FALLTHROUGH*/
	            case state_have:
	                if (f_bol && iskey(linebuf,len)) {

#if	CF_DEBUGS
	                    debugprintf("bdb_fileindex: key=%c\n",linebuf[1]) ;
#endif

	                    if (f_inkey) {

	                        f_inkey = FALSE ;
	                        sbuf_finish(&citekey) ;

	                    }

	                    lp += 1 ;
	                    ll -= 1 ;
	                    cl = nextfield((linebuf + 1),(len - 1),&cp) ;

	                    if ((strncmp(cp,op->qkey,op->qkeylen) == 0) &&
	                        (cl == op->qkeylen)) {

	                        ll -= ((cp + cl) - lp) ;
	                        lp = (cp + cl) ;
	                        kl = nextfield(lp,ll,&kp) ;

	                        rs = sbuf_start(&citekey,citebuf,CITEBUFLEN) ;
	                        f_inkey = (rs >= 0) ;

	                        if ((rs >= 0) && (kl > 0))
	                            sbuf_strw(&citekey,kp,kl) ;

	                    } /* end if (found citekey) */

	                } else if (f_inkey) {

	                    if (sbuf_getlen(&citekey) > 0)
	                        sbuf_char(&citekey,' ') ;

	                    sbuf_strw(&citekey,linebuf,len) ;

	                }

/* are we leaving an entry? */

	                if ((len == 0) || (linebuf[0] == '\n')) {

#if	CF_DEBUGS
	                    debugprintf("bdb_fileindex: end-of-entry \n") ;
#endif

	                    state = state_search ;
	                    if (f_inkey) {

	                        f_inkey = FALSE ;
	                        sbuf_finish(&citekey) ;

	                    }

	                    if (citebuf[0] != '\0') {

#if	CF_DEBUGS
	                        debugprintf("bdb_fileindex: inserting key=%s\n",
	                            citebuf) ;
	                        debugprintf("bdb_fileindex: fi=%u offset=%lu "
	                            "citeoff=%lu\n",
	                            fi,offset,citeoff) ;
#endif

	                        rs = bdb_keyinsert(op,fi,
	                            citeoff, offset, citebuf) ;

#if	CF_DEBUGS
	                        debugprintf("bdb_fileindex: bdb_keyinsert() "
	                            "rs=%d\n",rs) ;
#endif

	                        citebuf[0] = '\0' ;

	                    }

	                } /* end if (state transition) */

	                break ;

	            } /* end switch */

	            if (rs < 0)
	                break ;

	            offset += len ;
	            f_bol = f_eol ;

	        } /* end while */

	        bclose(&bibfile) ;

	    } /* end if (opened file) */

	    if (f_inkey)
	        sbuf_finish(&citekey) ;

/* mark this file as being indexed */

	   bdbfile_indexed(bfp) ;

	} /* end if (indexing file) */

#if	CF_DEBUGS
	debugprintf("bdb_fileindex: ret rs=%d fi=%u\n",rs,fi) ;
#endif

	return (rs >= 0) ? fi : rs ;
}
/* end subroutine (bdb_fileindex) */


/* insert a citation-key into the BIB key DB */
static int bdb_keyinsert(op,fi, citeoff, offset, citebuf)
BDB		*op ;
int		fi ;
offset_t		offset, citeoff ;
const char	citebuf[] ;
{
	struct bdb_key		*bkp ;

	int	rs ;
	int	size ;
	int	citelen = 0 ;


	size = sizeof(struct bdb_key) ;
	rs = uc_malloc(size,&bkp) ;

	if (rs >= 0) {

	    citelen = offset - citeoff ;
	    rs = bdbkey_init(bkp,fi,citebuf,citeoff,citelen) ;

	    if (rs >= 0) {

	        HDB_DATUM	key, value ;


	        key.buf = bkp->citekey ;
	        key.len = strlen(bkp->citekey) ;

	        value.buf = bkp ;
	        value.len = size ;

	        rs = hdb_store(&op->keys,key,value) ;
	        if (rs < 0)
	            bdbkey_free(bkp) ;

	    }

	    if (rs < 0)
	        uc_free(bkp) ;

	} /* end if */

	return (rs >= 0) ? citelen : rs ;
}
/* end subroutine (bdb_keyinsert) */


/* read a BIB entry */
static int bdb_readentry(op,bkp,bep,buf,buflen)
BDB		*op ;
struct bdb_key	*bkp ;
BDB_ENT	*bep ;
char		buf[] ;
int		buflen ;
{
	struct bdb_file	*bfp ;

	struct bibentry	ie ;

	bfile	bibfile ;

	int	rs ;
	int	rlen, len ;
	int	ll, cl ;
	int	n = 0 ;
	int	f_bol, f_eol ;
	int	f_inkey ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*lp ;
	char	*cp ;


	memset(bep,0,sizeof(BDB_ENT)) ;

	rs = vecobj_get(&op->files,bkp->fi,&bfp) ;
	if (rs < 0)
	    goto ret0 ;

	rs = bibentry_init(&ie,bkp->fi) ;
	if (rs < 0)
	    goto ret0 ;

	rs = bopen(&bibfile,bfp->fname,"r",0666) ;
	if (rs < 0)
	    goto ret1 ;

	bseek(&bibfile,bkp->offset,SEEK_SET) ;

	f_inkey = FALSE ;
	f_bol = TRUE ;
	rlen = 0 ;
	while ((rlen < bkp->len) &&
	    ((rs = breadline(&bibfile,linebuf,LINEBUFLEN)) > 0)) {

	    len = rs ;
	    f_eol = (linebuf[len - 1] == '\n') ;
	    lp = linebuf ;
	    ll = (f_eol) ? (len - 1) : len ;

	    if (f_bol && ((ll == 0) || (linebuf[0] == '\n')))
	        break ;

	    if (f_bol && (linebuf[0] == '%')) {

	        if (f_inkey)
	            rs = bibentry_end(&ie) ;

	        f_inkey = TRUE ;
	        cl = nextfield((lp + 1),(ll - 1),&cp) ;

	        if (rs >= 0) {

	            ll -= ((cp + cl) - lp) ;
	            lp = (cp + cl) ;
	            rs = bibentry_start(&ie,cp,cl,lp,ll) ;

	        }

	    } else if (f_inkey) {

	        rs = bibentry_continue(&ie,lp,ll) ;

	    }

	    if (rs < 0)
	        break ;

	    f_bol = f_eol ;
	    rlen += len ;

	} /* end while */

	if ((rs >= 0) && f_inkey)
	    rs = bibentry_end(&ie) ;

/* OK, load this internal entry up into the interface entry */

#if	CF_DEBUGS
	{
	    n = vecobj_count(&ie.keyvals) ;
	    debugprintf("bdb_readentry: total n=%u size=%u\n",n,ie.size) ;
	}
#endif

	if (rs >= 0) {
	    rs = entry_load(bep,buf,buflen,&ie) ;
	    n = rs ;
	}

ret2:
	bclose(&bibfile) ;

ret1:
	bibentry_free(&ie) ;

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (bdb_readentry) */


/* operate on the BDBFILE object */
static int bdbfile_init(fp,fname)
struct bdb_file	*fp ;
const char	fname[] ;
{
	int	rs ;


	memset(fp,0,sizeof(struct bdb_file)) ;

	fp->f_indexed = 0 ;
	rs = uc_mallocstrw(fname,-1,&fp->fname) ;

	return rs ;
}

static int bdbfile_free(fp)
struct bdb_file	*fp ;
{


	if (fp == NULL)
	    return SR_FAULT ;

	if (fp->fname != NULL) {
	    uc_free(fp->fname) ;
	    fp->fname = NULL ;
	}

	return SR_OK ;
}

static int bdbfile_indexed(fp)
struct bdb_file	*fp ;
{


	if (fp == NULL)
	    return SR_FAULT ;

	fp->f_indexed = TRUE ;
	return SR_OK ;
}


/* operate on the BDBKEY object */
static int bdbkey_init(bkp,fi,citekey,offset,len)
struct bdb_key	*bkp ;
int		fi ;
const char	citekey[] ;
offset_t		offset ;
int		len ;
{
	int	rs ;


#if	CF_DEBUGS
	debugprintf("bdb/bdbkey_init: fi=%u\n",fi) ;
#endif

	memset(bkp,0,sizeof(struct bdb_key)) ;

	bkp->fi = fi ;
	bkp->offset = offset ;
	bkp->len = len ;
	rs = uc_mallocstrw(citekey,-1,&bkp->citekey) ;

	return rs ;
}

static int bdbkey_free(bkp)
struct bdb_key	*bkp ;
{


	if (bkp == NULL)
	    return SR_FAULT ;

	if (bkp->citekey != NULL) {
	    uc_free(bkp->citekey) ;
	    bkp->citekey = NULL ;
	}

	bkp->fi = -1 ;
	return SR_OK ;
}


/* initialize a BIB accumulation object */
static int bibentry_init(iep,fi)
struct bibentry	*iep ;
int		fi ;
{
	int	rs ;
	int	size = sizeof(struct keyval) ;


	memset(iep,0,sizeof(struct bibentry)) ;

	rs = vecobj_start(&iep->keyvals,size,10,0) ;
	if (rs < 0)
	    goto bad0 ;

	iep->size = 0 ;
	iep->fi = fi ;

ret0:
	return rs ;

/* bad stuff */
bad0:
	goto ret0 ;
}
/* end subroutine (bibentry_init) */


static int bibentry_start(iep,kp,kl,vp,vl)
struct bibentry	*iep ;
const char	*kp, *vp ;
int		kl, vl ;
{
	int	rs ;


	if (kl < 0)
	    kl = strlen(kp) ;

/* key */

	rs = uc_malloc((kl + 1),&iep->kp) ;
	if (rs < 0)
	    goto bad0 ;

/* enter the key */

	iep->kl = kl ;
	strwcpy(iep->kp,kp,kl) ;

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

	char	*bp ;


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
	int	vl ;
	int	size ;

	char	*vp ;


	keyval_init(&kv) ;

	kv.kl = iep->kl ;
	kv.kp = iep->kp ;
	iep->kp = NULL ;		/* good!  zapped! */

	rs = buffer_get(&iep->vb,&vp) ;
	vl = rs ;
	if (rs >= 0)
	    rs = uc_malloc((vl + 1),&kv.vp) ;

	if (rs >= 0) {
	        kv.vl = vl ;
	        strwcpy(kv.vp,vp,vl) ;
	}

	if (rs >= 0) {

	    rs = vecobj_add(&iep->keyvals,&kv) ;
	    if (rs < 0)
		keyval_free(&kv) ;

	    if (rs >= 0) {
	        size = keyval_size(&kv) ;
	        iep->size += size ;
	    }

	} /* end if (inserting into DB) */

	buffer_finish(&iep->vb) ;

	return rs ;
}
/* end subroutine (bibentry_end) */


static int bibentry_free(iep)
struct bibentry	*iep ;
{
	struct keyval	*kvp ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	if (iep->kp != NULL) {
	    uc_free(iep->kp) ;
	    iep->kp = NULL ;
	}

	for (i = 0 ; vecobj_get(&iep->keyvals,i,&kvp) >= 0 ; i += 1) {
	    if (kvp == NULL) continue ;

	    keyval_free(kvp) ;

	} /* end for */

	rs1 = vecobj_finish(&iep->keyvals) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (bibentry_free) */


/* load up the user-interface entry from the internal structure */
static int entry_load(ep,ebuf,ebuflen,iep)
BDB_ENT		*ep ;
char			ebuf[] ;
int			ebuflen ;
struct bibentry		*iep ;
{
	struct keyval	*kvp ;

	int	rs = SR_OK ;
	int	tabsize, size ;
	int	n ;
	int	bo, i, j, kal ;
#ifdef	OPTIONAL
	int	cl ;
#endif

	char	*(*keyvals)[2] ;
	char	*cp ;


	rs = vecobj_count(&iep->keyvals) ;
	n = rs ;
	if (rs < 0)
	    goto ret0 ;

	tabsize = ((n + 1) * 2 * sizeof(char *)) ;
	size = tabsize + iep->size ;

	bo = BDB_BO((ulong) ebuf) ;
	if (size <= (ebuflen - bo)) {

	    keyvals = (char *(*)[2]) (ebuf + bo) ;
	    kal = tabsize ;
	    cp = (char *) (ebuf + bo + kal) ;
#ifdef	OPTIONAL
	    cl = ebuflen - bo - kal ;
#endif

	    j = 0 ;
	    for (i = 0 ; vecobj_get(&iep->keyvals,i,&kvp) >= 0 ; i += 1) {

	        if (kvp == NULL) continue ;

	        keyvals[j][0] = cp ;
	        cp = strwcpy(cp,kvp->kp,kvp->kl) + 1 ;

#if	CF_DEBUGS
	        debugprintf("entry_load: k=%s\n",keyvals[j][0]) ;
#endif

	        if (kvp->vp != NULL) {

	            keyvals[j][1] = cp ;
	            cp = strwcpy(cp,kvp->vp,kvp->vl) + 1 ;

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


static int keyval_init(kvp)
struct keyval	*kvp ;
{


	memset(kvp,0,sizeof(struct keyval)) ;
	return SR_OK ;
}
/* end subroutine (keyval_init) */


static int keyval_size(kvp)
struct keyval	*kvp ;
{
	int	rs = 0 ;
	int	size = 2 ;


	if (kvp->kp != NULL)
	    size += kvp->kl ;

	if (kvp->vp != NULL)
	    size += kvp->vl ;

	return (rs >= 0) ? size : rs ;
}
/* end subroutine (keyval_size) */


static int keyval_free(kvp)
struct keyval	*kvp ;
{


	if (kvp->kp != NULL) {
	    uc_free(kvp->kp) ;
	    kvp->kp = NULL ;
	}

	if (kvp->vp != NULL) {
	    uc_free(kvp->vp) ;
	    kvp->vp = NULL ;
	}

	return SR_OK ;
}
/* end subroutine (keyval_free) */


/* is there a key? */
static int iskey(linebuf,ll)
const char	linebuf[] ;
int		ll ;
{
	int	f = FALSE ;


	if (ll >= 2)
	    f = ((linebuf[0] == '%') &&
	        isalpha(linebuf[1])) ;

	return f ;
}
/* end subroutine (iskey) */



