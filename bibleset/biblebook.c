/* biblebook */

/* BIBLEBOOK object-load management */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This module implements an interface (a trivial one) that allows access
	to the BIBLEBOOK datbase.


*******************************************************************************/


#define	BIBLEBOOK_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<char.h>
#include	<localmisc.h>

#include	"biblebook.h"
#include	"biblebooks.h"


/* local defines */

#define	BIBLEBOOK_MODBNAME	"biblebooks"
#define	BIBLEBOOK_OBJNAME	"biblebooks"

#ifndef	SYMNAMELEN
#define	SYMNAMELEN	60
#endif

#define	tolc(c)		CHAR_TOLC(c)
#define	touc(c)		CHAR_TOUC(c)
#define	tofc(c)		CHAR_TOFC(c)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath4(char *,cchar *,cchar *,cchar *,cchar *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	biblebook_openload(BIBLEBOOK *,const char *,const char *) ;
static int	biblebook_openlocal(BIBLEBOOK *) ;

static int	biblebook_objloadbegin(BIBLEBOOK *,const char *,const char *) ;
static int	biblebook_objloadend(BIBLEBOOK *) ;
static int	biblebook_loadcalls(BIBLEBOOK *,const char *) ;

static int	biblebook_matcher(BIBLEBOOK *,const char *,int) ;
static int	biblebook_loadnames(BIBLEBOOK *) ;
static int	biblebook_loadnameslocal(BIBLEBOOK *) ;
static int	biblebook_loadnamesremote(BIBLEBOOK *) ;

static int	isrequired(int) ;

static char	*strwcpyspecial(char *,cchar *,int) ;


/* global variables */


/* local variables */

static const char	*subs[] = {
	"open",
	"count",
	"max",
	"lookup",
	"get",
	"match",
	"size",
	"audit",
	"close",
	NULL
} ;

enum subs {
	sub_open,
	sub_count,
	sub_max,
	sub_lookup,
	sub_get,
	sub_match,
	sub_size,
	sub_audit,
	sub_close,
	sub_overlast
} ;

static const char	*booknames[] = {
	"Bible",
	"Genesis",
	"Exodus",
	"Leviticus",
	"Numbers",
	"Deuteronomy",
	"Joshua",
	"Judges",
	"Ruth",
	"I Samuel",
	"II Samuel",
	"I Kings",
	"II Kings",
	"I Chronicles",
	"II Chronicles",
	"Ezra",
	"Nehemiah",
	"Esther",
	"Job",
	"Psalms",
	"Proverbs",
	"Ecclesiastes",
	"Song of Solomon",
	"Isaiah",
	"Jeremiah",
	"Lamentations",
	"Ezekiel",
	"Daniel",
	"Hosea",
	"Joel",
	"Amos",
	"Obadiah",
	"Jonah",
	"Micah",
	"Nahum",
	"Habakkuk",
	"Zephaniah",
	"Haggai",
	"Zechariah",
	"Malachi",
	"Matthew",
	"Mark",
	"Luke",
	"John",
	"Acts",
	"Romans",
	"I Corinthians",
	"II Corinthians",
	"Galatians",
	"Ephesians",
	"Philippians",
	"Colossians",
	"I Thessalonians",
	"II Thessalonians",
	"I Timothy",
	"II Timothy",
	"Titus",
	"Philemon",
	"Hebrews",
	"James",
	"I Peter",
	"II Peter",
	"I John",
	"II John",
	"III John",
	"Jude",
	"Revelation",
	NULL
} ;


/* exported subroutines */


int biblebook_open(BIBLEBOOK *op,cchar pr[],cchar dbname[])
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(BIBLEBOOK)) ;

	if ((rs = biblebook_openload(op,pr,dbname)) == SR_NOENT) {
	    rs = biblebook_openlocal(op) ;
	}
	if (rs >= 0) {
	    op->magic = BIBLEBOOK_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("biblebook_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblebook_open) */


/* free up the entire vector string data structure object */
int biblebook_close(BIBLEBOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOK_MAGIC) return SR_NOTOPEN ;

	if (op->names != NULL) {
	    rs1 = uc_free(op->names) ;
	    if (rs >= 0) rs = rs1 ;
	    op->names = NULL ;
	}

	if (! op->f.localdb) {

	    rs1 = (*op->call.close)(op->obj) ;
	    if (rs >= 0) rs = rs1 ;

	    rs1 = biblebook_objloadend(op) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end if */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (biblebook_close) */


int biblebook_count(BIBLEBOOK *op)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOK_MAGIC) return SR_NOTOPEN ;

	if (op->f.localdb) {
	    rs = nelem(booknames) - 1 ;
	} else {
	    if (op->call.count != NULL) {
	        rs = (*op->call.count)(op->obj) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (biblebook_count) */


int biblebook_max(BIBLEBOOK *op)
{
	int		rs ;
	int		max = 0 ;

	if ((rs = biblebook_count(op)) >= 0) {
	    max = (rs - 1) ;
	}

	return (rs >= 0) ? max : rs ;
}
/* end subroutine (biblebook_max) */


int biblebook_audit(BIBLEBOOK *op)
{
	int		rs = SR_OK ;

	if (op == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOK_MAGIC) return SR_NOTOPEN ;

	if (! op->f.localdb) {
	    rs = SR_NOSYS ;
	    if (op->call.audit != NULL) {
	        rs = (*op->call.audit)(op->obj) ;
	    }
	} /* end if */

	return rs ;
}
/* end subroutine (biblebook_audit) */


int biblebook_lookup(BIBLEBOOK *op,char *rbuf,int rlen,int bi)
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOK_MAGIC) return SR_NOTOPEN ;

	if (bi < 0) return SR_INVALID ;

#if	CF_DEBUGS
	debugprintf("biblebook_lookup: bi=%d\n",bi) ;
#endif

	if (op->f.localdb) {
	    const int	n = (nelem(booknames) - 1) ;
	    if (bi < n) {
	        rs = sncpy1(rbuf,rlen,booknames[bi]) ;
	    } else {
	        rs = SR_NOTFOUND ;
	    }
	} else {
	    rs = (*op->call.lookup)(op->obj,rbuf,rlen,bi) ;
	}

#if	CF_DEBUGS
	debugprintf("biblebook_lookup: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblebook_lookup) */


int biblebook_read(BIBLEBOOK *op,char *rbuf,int rlen,int bi)
{
	return biblebook_lookup(op,rbuf,rlen,bi) ;
}
/* end subroutine (biblebook_read) */


int biblebook_get(BIBLEBOOK *op,int bi,char *rbuf,int rlen)
{
	return biblebook_lookup(op,rbuf,rlen,bi) ;
}
/* end subroutine (biblebook_get) */


int biblebook_match(BIBLEBOOK *op,cchar mbuf[],int mlen)
{
	int		rs = SR_NOSYS ;

	if (op == NULL) return SR_FAULT ;
	if (mbuf == NULL) return SR_FAULT ;

	if (op->magic != BIBLEBOOK_MAGIC) return SR_NOTOPEN ;

	if (! op->f.localdb) {
	    rs = SR_NOSYS ;
	    if (op->call.match != NULL) {
	        rs = (*op->call.match)(op->obj,mbuf,mlen) ;
	    }
	} 

#if	CF_DEBUGS
	debugprintf("biblebook_match: mid rs=%d\n",rs) ;
#endif

	if (op->f.localdb || (rs == SR_NOSYS)) {
	    rs = biblebook_matcher(op,mbuf,mlen) ;
	}

	return rs ;
}
/* end subroutine (biblebook_match) */


int biblebook_size(BIBLEBOOK *op)
{
	if (op == NULL) return SR_FAULT ;
	if (op->magic != BIBLEBOOK_MAGIC) return SR_NOTOPEN ;
	return biblebook_loadnames(op) ;
}
/* end subroutine (biblebook_size) */


/* private subroutines */


static int biblebook_openload(BIBLEBOOK *op,cchar pr[],cchar dbname[])
{
	int		rs ;
	const char	*objname = BIBLEBOOK_OBJNAME ;

	if ((rs = biblebook_objloadbegin(op,pr,objname)) >= 0) {
	    rs = (*op->call.open)(op->obj,pr,dbname) ;
	    if (rs < 0)
	        biblebook_objloadend(op) ;
	}

	return rs ;
}
/* end subroutine (biblebook_openload) */


static int biblebook_openlocal(BIBLEBOOK *op)
{

	op->f.localdb = TRUE ;
	return SR_OK ;
}
/* end subroutine (biblebook_openlocal) */


/* find and load the DB-access object */
static int biblebook_objloadbegin(BIBLEBOOK *op,cchar *pr,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	VECSTR		syms ;
	const int	n = nelem(subs) ;
	int		rs ;
	int		rs1 ;
	int		opts ;

#if	CF_DEBUGS
	debugprintf("biblebook_objloadbegin: pr=%s\n",pr) ;
	debugprintf("biblebook_objloadbegin: objname=%s\n",objname) ;
#endif

	opts = VECSTR_OCOMPACT ;
	if ((rs = vecstr_start(&syms,n,opts)) >= 0) {
	    const int	nlen = SYMNAMELEN ;
	    int		i ;
	    int		f_modload = FALSE ;
	    char	nbuf[SYMNAMELEN + 1] ;

	    for (i = 0 ; (i < n) && (subs[i] != NULL) ; i += 1) {
	        if (isrequired(i)) {
	            if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
			rs = vecstr_add(&syms,nbuf,rs) ;
		    }
		}
		if (rs < 0) break ;
	    } /* end for */

	    if (rs >= 0) {
		cchar	**sv ;
	        if ((rs = vecstr_getvec(&syms,&sv)) >= 0) {
	            cchar	*modbname = BIBLEBOOK_MODBNAME ;
	            opts = (MODLOAD_OLIBVAR | MODLOAD_OSDIRS) ;
	            rs = modload_open(lp,pr,modbname,objname,opts,sv) ;
		    f_modload = (rs >= 0)  ;
		}
	    }

	    rs1 = vecstr_finish(&syms) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && f_modload) {
		modload_close(lp) ;
	    }
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("biblebook_objloadbegin: modload_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {
	    if ((rs = modload_getmv(lp,0)) >= 0) {
		void	*p ;

#if	CF_DEBUGS
		debugprintf("biblebook_objloadbegin: getmv rs=%d\n",rs) ;
#endif

		op->objsize = rs ;
		if ((rs = uc_malloc(op->objsize,&p)) >= 0) {
		    op->obj = p ;
		    rs = biblebook_loadcalls(op,objname) ;
#if	CF_DEBUGS
		    debugprintf("biblebook_objloadbegin: calls rs=%d\n",rs) ;
#endif
		    if (rs < 0) {
			uc_free(op->obj) ;
			op->obj = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } /* end if (getmva) */
	    if (rs < 0)
		modload_close(lp) ;
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("biblebook_objloadbegin: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (biblebook_objloadbegin) */


static int biblebook_objloadend(BIBLEBOOK *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->obj != NULL) {
	    rs1 = uc_free(op->obj) ;
	    if (rs >= 0) rs = rs1 ;
	    op->obj = NULL ;
	}

	rs1 = modload_close(&op->loader) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (biblebook_objloadend) */


static int biblebook_loadcalls(BIBLEBOOK *op,cchar *objname)
{
	MODLOAD		*lp = &op->loader ;
	const int	nlen = SYMNAMELEN ;
	int		rs = SR_OK ;
	int		i ;
	int		c = 0 ;
	char		nbuf[SYMNAMELEN + 1] ;
	const void	*snp ;

	for (i = 0 ; subs[i] != NULL ; i += 1) {

	    if ((rs = sncpy3(nbuf,nlen,objname,"_",subs[i])) >= 0) {
	         if ((rs = modload_getsym(lp,nbuf,&snp)) == SR_NOTFOUND) {
		     snp = NULL ;
		     if (! isrequired(i)) rs = SR_OK ;
		}
	    }

	    if (rs < 0) break ;

	    if (snp != NULL) {
	        c += 1 ;
	        switch (i) {
	        case sub_open:
	            op->call.open = 
	                (int (*)(void *,const char *,const char *)) snp ;
	            break ;
	        case sub_count:
	            op->call.count = (int (*)(void *)) snp ;
	            break ;
	        case sub_max:
	            op->call.max = (int (*)(void *)) snp ;
	            break ;
	        case sub_lookup:
	            op->call.lookup = (int (*)(void *,char *,int,int)) snp ;
	            break ;
	        case sub_get:
	            op->call.get = (int (*)(void *,int,char *,int)) snp ;
	            break ;
	        case sub_match:
	            op->call.match = (int (*)(void *,const char *,int)) snp ;
	            break ;
	        case sub_size:
	            op->call.size = (int (*)(void *)) snp ;
	            break ;
	        case sub_audit:
	            op->call.audit = (int (*)(void *)) snp ;
	            break ;
	        case sub_close:
	            op->call.close = (int (*)(void *)) snp ;
	            break ;
	        } /* end switch */
	    } /* end if (it had the call) */

	} /* end for (subs) */

#if	CF_DEBUGS
	debugprintf("biblebook_loadcalls: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (biblebook_loadcalls) */


static int biblebook_matcher(BIBLEBOOK *op,cchar mbuf[],int mlen)
{
	int		rs = SR_OK ;
	int		i = 0 ;

	if (op->names == NULL) {
	    rs = biblebook_loadnames(op) ;
	}

	if (rs >= 0) {
	    int		ml ;
	    int		bl ;
	    char	bbuf[BIBLEBOOK_LEN + 1] ;

#if	CF_DEBUGS
	    debugprintf("biblebook_matcher: m=>%t<\n",mbuf,mlen) ;
#endif

	    ml = MIN(mlen,BIBLEBOOK_LEN) ;
	    bl = strwcpyspecial(bbuf,mbuf,ml) - bbuf ;

#if	CF_DEBUGS
	    debugprintf("biblebook_matcher: mm=>%t<\n",bbuf,bl) ;
	    for (i = 0 ; op->names[i] != NULL ; i += 1) 
	        debugprintf("biblebook_matcher: b=>%s<\n",op->names[i]) ;
#endif

	    rs = matostr(op->names,1,bbuf,bl) ;
	    i = rs ;
	    if (rs < 0) rs = SR_NOTFOUND ;

	} /* end if (ok) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (biblebook_matcher) */


static int biblebook_loadnames(BIBLEBOOK *op)
{
	int		rs = SR_OK ;
	if (op->names == NULL) {
	    if (op->f.localdb) {
	        rs = biblebook_loadnameslocal(op) ;
	    } else {
	        rs = biblebook_loadnamesremote(op) ;
	    }
	} else {
	    rs = op->namesize ;
	}
	return rs ;
}
/* end subroutine (biblebook_loadnames) */


static int biblebook_loadnameslocal(BIBLEBOOK *op)
{
	int		rs ;
	int		sizetab ;
	int		size = 0 ;
	int		namesize = 0 ;
	int		n ;
	char		*mp ;

	for (n = 0 ; booknames[n] != NULL ; n += 1) {
	    namesize += (strlen(booknames[n]) + 1) ;
	}

	sizetab = (n + 1) * sizeof(const char *) ;
	size += sizetab ;
	size += namesize ;
	if ((rs = uc_malloc(size,&mp)) >= 0) {
	    int		i ;
	    const char	**names = (const char **) mp ;
	    char	*bp = mp ;
	    bp = (mp + sizetab) ;
	    op->namestrs = bp ;
	    for (i = 0 ; i < n ; i += 1) {
	        names[i] = bp ;
	        bp = strwcpyspecial(bp,booknames[i],-1) + 1 ;
	    } /* end for */
	    names[i] = NULL ;
	    op->names = names ;
	    op->namesize = namesize ;
	} /* end if (memory-allocation) */

	return (rs >= 0) ? namesize : rs ;
}
/* end subroutine (biblebook_loadnameslocal) */


static int biblebook_loadnamesremote(BIBLEBOOK *op)
{
	const int	blen = BIBLEBOOK_LEN ;
	int		rs = SR_OK ;
	int		n = 0 ;
	int		namesize = 0 ;
	char		bbuf[BIBLEBOOK_LEN + 1] ;

	if (op->call.size != NULL) {
	    if ((rs = (*op->call.size)(op->obj)) >= 0) {
		namesize = rs ;
		rs = (*op->call.count)(op->obj) ;
		n = rs ;
	    }
	} else {
	    for (n = 0 ; rs >= 0 ; n += 1) {
	        if ((rs = (*op->call.get)(op->obj,n,bbuf,blen)) >= 0) {
		    namesize += (rs+1) ;
	        } else {
		    if (rs == SR_NOTFOUND) rs = SR_OK ;
		    break ;
	        }
	    } /* end for */
	} /* end if */

#if	CF_DEBUGS
	debugprintf("biblebook_loadnames: mid rs=%d n=%d\n",rs,n) ;
#endif

	if (rs >= 0) {
	    const int	sizetab = (n + 1) * sizeof(const char *) ;
	    int		size = namesize ;
	    int		bl ;
	    char	*mp ;
	    size += sizetab ;
	    if ((rs = uc_malloc(size,&mp)) >= 0) {
		int		i ;
		const char	**names = (const char **) mp ;
		char		*bp = (mp + sizetab) ;
	        op->namestrs = bp ;
	        for (i = 0 ; (rs >= 0) && (i < n) ; i += 1) {
	            if ((rs = (*op->call.get)(op->obj,i,bbuf,blen)) >= 0) {
	                bl = rs ;
	                names[i] = bp ;
	                bp = strwcpyspecial(bp,bbuf,bl) + 1 ;
	            }
	        } /* end for */
	        names[i] = NULL ;
	        op->namesize = namesize ;
	        if (rs < 0) { /* should not or cannot fail! */
	            uc_free(mp) ;
	        } else
	            op->names = names ;
	    } /* end if (m-a) */
	} /* end if (ok) */

#if	CF_DEBUGS
	debugprintf("biblebook_loadnames: ret rs=%d ns=%d\n",
		rs,namesize) ;
#endif

	return (rs >= 0) ? namesize : rs ;
}
/* end subroutine (biblebook_loadnamesremote) */


static int isrequired(int i)
{
	int		f = FALSE ;
	switch (i) {
	case sub_open:
	case sub_count:
	case sub_lookup:
	case sub_close:
	    f = TRUE ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isrequired) */


static char *strwcpyspecial(char *dp,cchar *sp,int sl)
{

	if (sl >= 0) {
	    while (sl && (*sp != '\0')) {
		if (! CHAR_ISWHITE(*sp)) *dp++ = tolc(*sp) ;
		sp += 1 ;
		sl -= 1 ;
	    }
	} else {
	    while (*sp != '\0') {
		if (! CHAR_ISWHITE(*sp)) *dp++ = tolc(*sp) ;
		sp += 1 ;
	    }
	} /* end if */

	*dp = '\0' ;
	return dp ;
}
/* end subroutine (strwcpyspecial) */


