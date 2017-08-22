/* recorder */

/* string recorder object */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_DEBUGBOUNDS	0		/* debug bounds */
#define	CF_SAFE		1		/* safety */
#define	CF_FASTGROW	1		/* grow exponetially? */
#define	CF_EXCLUDEKEY	0		/* exclude small keys from indices */
#define	CF_DEBUGSHIFT	0		/* debug shift amount */


/* revision history:

	= 2002-04-29, David A­D­ Morano
	This object module was created for the MKPWI program.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This object module creates records that represent system password
        (PASSWD DB) username and realname information. The recrods can be
        indexed by various numbers of characters from the last name and the
        first name of each record.


*******************************************************************************/


#define	RECORDER_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"recorder.h"


/* local defines */

#define	MODP2(v,n)	((v) & ((n) - 1))

#define	NSHIFT		6

#if	CF_DEBUGS
#ifndef	HEXBUFLEN
#define	HEXBUFLEN	100
#endif
#endif


/* external subroutines */

extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;

extern int	randlc(int) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
extern int	mkhexstr(char *,int,const void *,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

#if	CF_DEBUGSHIFT
extern int	nshift ;
#endif


/* local structures */


/* forward references */

static int	recorder_extend(RECORDER *) ;
static int	recorder_matfl3(RECORDER *,cchar *,uint [][2],int,cchar *) ;
static int	recorder_matun(RECORDER *,cchar *,uint [][2],int,cchar *) ;
static int	recorder_cden(RECORDER *,int,int) ;

static int	hashindex(uint,int) ;

#if	CF_DEBUGS && CF_DEBUGBOUNDS
static int inbounds(const char *,int,const char *) ;
#endif


/* local variables */

enum indices {
	index_l1,
	index_l3,
	index_f,
	index_fl3,
	index_un,
	index_overlast
} ;


/* exported subroutines */


int recorder_start(RECORDER *asp,int n,int opts)
{
	int		rs ;
	int		size ;
	void		*p ;

	if (asp == NULL) return SR_FAULT ;

	memset(asp,0,sizeof(RECORDER)) ;

	if (n < RECORDER_STARTNUM)
	    n = RECORDER_STARTNUM ;

#if	CF_DEBUGS
	debugprintf("recorder_start: allocating RECTAB n=%d\n",n) ;
#endif

	size = (n * sizeof(RECORDER_ENT)) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    asp->rectab = p ;
	    asp->e = n ;
	    asp->c = 0 ;
	    asp->opts = opts ;
	    asp->i = 0 ;
	    memset(&asp->rectab[asp->i],0,sizeof(RECORDER_ENT)) ;
	    asp->i += 1 ;
	    asp->magic = RECORDER_MAGIC ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (recorder_start) */


/* free up this recorder object */
int recorder_finish(RECORDER *asp)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (asp->rectab != NULL) {
	    rs1 = uc_free(asp->rectab) ;
	    if (rs >= 0) rs = rs1 ;
	    asp->rectab = NULL ;
	}

	rs = asp->i ;
	asp->e = 0 ;
	asp->i = 0 ;
	asp->magic = 0 ;
	return rs ;
}
/* end subroutine (recorder_finish) */


/* add a record to this object */
int recorder_add(RECORDER *asp,RECORDER_ENT *ep)
{
	int		rs = SR_OK ;
	int		i = 0 ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (asp->i < 0) return asp->i ;

/* do we need to extend the table? */

	if ((asp->i + 1) > asp->e) {
	    rs = recorder_extend(asp) ;
	} /* end if */

/* copy the new one in */

	if (rs >= 0) {

	    i = asp->i ;
#if	CF_DEBUGS
	    debugprintf("recorder_add: i=%d username=%d\n",i,ep->username) ;
#endif
	    asp->rectab[i] = *ep ;
	    asp->i = (i+1) ;

#if	CF_DEBUGS
	    debugprintf("recorder_add: rectab[%d].last=%u username=%d\n",
	        i, asp->rectab[i].last,asp->rectab[i].username) ;
#endif

	    asp->c += 1 ;

	} /* end if (ok) */

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (recorder_add) */


/* is a given entry already present? */
int recorder_already(RECORDER *asp,RECORDER_ENT *ep)
{
	const int	esize = sizeof(RECORDER_ENT) ;
	int		i ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (asp->i < 0)
	    return asp->i ;

	for (i = 0 ; i < asp->i ; i += 1) {
	    if (memcmp(ep,(asp->rectab+i),esize)) break ;
	} /* end for */

	return (i < asp->i) ? i : SR_NOTFOUND ;
}
/* end subroutine (recorder_already) */


/* get the length (total number of entries) of the table */
int recorder_rtlen(RECORDER *asp)
{

	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;

	return asp->i ;
}
/* end subroutine (recorder_rtlen) */


/* get the count of valid entries in the table */
int recorder_count(RECORDER *asp)
{

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	return asp->c ;
}
/* end subroutine (recorder_count) */


/* calculate the index table length (number of entries) at this point */
int recorder_indlen(RECORDER *asp)
{
	int		n ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	n = nextpowtwo(asp->i) ;

	return n ;
}
/* end subroutine (recorder_indlen) */


/* calculate the index table size */
int recorder_indsize(RECORDER *asp)
{
	int		n ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	n = nextpowtwo(asp->i) ;

	return (n * 2 * sizeof(int)) ;
}
/* end subroutine (recorder_indsize) */


/* get the address of the rectab array */
int recorder_gettab(RECORDER *asp,RECORDER_ENT **rpp)
{
	int		size ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (rpp != NULL) {
	    *rpp = asp->rectab ;
	}

	size = (asp->i * sizeof(RECORDER_ENT)) ;
	return size ;
}
/* end subroutine (recorder_gettab) */


/* create a record index for the caller */
int recorder_mkindl1(RECORDER *asp,cchar *s,uint (*it)[2],int itsize)
{
	uint		rhash ;
#if	CF_DEBUGSHIFT
	int		ns = (nshift > 0) ? nshift : NSHIFT ;
#else
	const int	ns = NSHIFT ;
#endif
	int		rs = SR_OK ;
	int		hi, ri, c, nc = 1 ;
	int		sl, hl, n, size ;
	int		wi = index_l1 ;
	cchar		*sp ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

#if	CF_DEBUGS
	debugprintf("recorder_mkindl1: entered itsize=%u\n",itsize) ;
#endif

	if (it == NULL) return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;

#if	CF_DEBUGS
	debugprintf("recorder_mkindl1: size calc=%u given=%u\n",
	    size,itsize) ;
#endif

	if (size > itsize)
	    return SR_OVERFLOW ;

	memset(it,0,size) ;

#if	CF_DEBUGS
	debugprintf("recorder_mkindl1: nrecs=%u\n",asp->i) ;
#endif

	for (ri = 1 ; ri < asp->i ; ri += 1) {

#if	CF_DEBUGS
	    debugprintf("recorder_mkindl1: stab_off=%u\n",
	        asp->rectab[ri].last) ;
#endif

	    sp = s + asp->rectab[ri].last ;

#if	CF_DEBUGS
	    if (sp != NULL) {
#ifdef	COMMENT
	        {
	            int	hbl ;
	            char	hexbuf[HEXBUFLEN + 1] ;
	            hbl = mkhexstr(hexbuf,HEXBUFLEN,sp,20) ;
	            debugprintf("recorder_mkindl1: hexbuf=%t \n",hexbuf,hbl) ;
	        }
#endif /* COMMENT */
	        debugprintf("recorder_mkindl1: sp=%p\n",sp) ;
	        debugprintf("recorder_mkindl1: key=%t\n",sp,strnlen(sp,20)) ;
	    } else
	        debugprintf("recorder_mkindl1: key=NULL\n") ;
#endif

	    sl = strlen(sp) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindl1: key=%s\n",sp) ;
#endif

#if	CF_EXCLUDEKEY
	    if (sl < nc)
	        continue ;
#endif

	    hl = MIN(sl,nc) ;
	    rhash = hashelf(sp,hl) ;

	    hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindl1: rhash=%08x hi=%u\n",rhash,hi) ;
#endif

	    c = 0 ;
	    if ((asp->opts & RECORDER_OSEC) && (it[hi][0] != 0)) {

#if	CF_DEBUGS
	        debugprintf("recorder_mkindl1: collision ri=%d\n",ri) ;
#endif

	        while ((it[hi][0] != 0) &&
	            (strncmp(sp,(s + asp->rectab[it[hi][0]].last),hl) != 0)) {

#if	CF_DEBUGS
	            debugprintf("recorder_mkindl1: collision c=%d "
	                "not same key\n",c) ;
#endif

	            if (asp->opts & RECORDER_ORANDLC) {
	                rhash = randlc(rhash + c) ;
	            } else {
	                rhash = ((rhash << (32 - ns)) | (rhash >> ns)) + c ;
	            }

	            hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	            debugprintf("recorder_mkindl1: new rhash=%08x hi=%u\n",
	                rhash,hi) ;
#endif

	            c += 1 ;

	        } /* end while */

	    } /* end if (secondary hash on collision) */

	    if (it[hi][0] != 0) {
	        int	lhi ;

#if	CF_DEBUGS
	        debugprintf("recorder_mkindl1: collision same key\n") ;
#endif

	        c += 1 ;
	        while (it[hi][1] != 0) {
	            c += 1 ;
	            hi = it[hi][1] ;
	        }

	        lhi = hi ;			/* save last hash-index value */
	        hi = hashindex((hi + 1),n) ;

	        while (it[hi][0] != 0) {
	            c += 1 ;
	            hi = hashindex((hi + 1),n) ;
	        } /* end while */

	        it[lhi][1] = hi ;		/* update the previous slot */

	    } /* end if (got a hash collision) */

	    it[hi][0] = ri ;
	    it[hi][1] = 0 ;
	    asp->s.c_l1 += c ;
	    recorder_cden(asp,wi,c) ;

	} /* end for (looping through records) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (recorder_mkindl1) */


int recorder_mkindl3(RECORDER *asp,cchar s[],uint it[][2],int itsize)
{
	uint		rhash ;
#if	CF_DEBUGSHIFT
	int		ns = (nshift > 0) ? nshift : NSHIFT ;
#else
	const int	ns = NSHIFT ;
#endif
	int		rs = SR_OK ;
	int		hi, ri, c ;
	int		nc = 3 ;
	int		sl, hl, n, size ;
	int		wi = index_l3 ;
	const char	*sp ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (it == NULL)
	    return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;

	if (size > itsize)
	    return SR_OVERFLOW ;

	memset(it,0,size) ;

	for (ri = 1 ; ri < asp->i ; ri += 1) {

	    sp = s + asp->rectab[ri].last ;
	    sl = strlen(sp) ;

#if	CF_EXCLUDEKEY
	    if (sl < nc)
	        continue ;
#endif

	    hl = MIN(sl,nc) ;
	    rhash = hashelf(sp,hl) ;

	    hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindl3: rhash=%08x hi=%d\n",rhash,hi) ;
#endif

	    c = 0 ;
	    if ((asp->opts & RECORDER_OSEC) && (it[hi][0] != 0)) {

#if	CF_DEBUGS
	        debugprintf("recorder_mkindl3: collision ri=%d\n",ri) ;
#endif

	        while ((it[hi][0] != 0) &&
	            (strncmp(sp,(s + asp->rectab[it[hi][0]].last),hl) != 0)) {

#if	CF_DEBUGS
	            debugprintf("recorder_mkindl3: collision c=%d "
	                "not same key\n",c) ;
#endif

	            if (asp->opts & RECORDER_ORANDLC) {
	                rhash = randlc(rhash + c) ;
	            } else {
	                rhash = ((rhash << (32 - ns)) | (rhash >> ns)) + c ;
	            }

	            hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	            debugprintf("recorder_mkindl3: new rhash=%08x hi=%u\n",
	                rhash,hi) ;
#endif

	            c += 1 ;

	        } /* end while */

	    } /* end if (secondary hash on collision) */

	    if (it[hi][0] != 0) {
	        int	lhi ;

	        c += 1 ;
	        while (it[hi][1] != 0) {
	            c += 1 ;
	            hi = it[hi][1] ;
	        }

	        lhi = hi ;			/* save last hash-index value */
	        hi = hashindex((hi + 1),n) ;

	        while (it[hi][0] != 0) {
	            c += 1 ;
	            hi = hashindex((hi + 1),n) ;
	        } /* end while */

	        it[lhi][1] = hi ;		/* update the previous slot */

	    } /* end if (got a hash collision) */

	    it[hi][0] = ri ;
	    it[hi][1] = 0 ;
	    asp->s.c_l3 += c ;
	    recorder_cden(asp,wi,c) ;

	} /* end for (looping through records) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (recorder_mkindl3) */


int recorder_mkindf(RECORDER *asp,cchar s[],uint it[][2],int itsize)
{
	uint		rhash ;
#if	CF_DEBUGSHIFT
	int		ns = (nshift > 0) ? nshift : NSHIFT ;
#else
	const int	ns = NSHIFT ;
#endif
	int		rs = SR_OK ;
	int		hi, ri, c, nc = 1 ;
	int		sl, hl, n, size ;
	int		wi = index_f ;
	const char	*sp ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (it == NULL) return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;

	if (size > itsize)
	    return SR_OVERFLOW ;

	memset(it,0,size) ;

	for (ri = 1 ; ri < asp->i ; ri += 1) {

	    if (asp->rectab[ri].first == 0)
	        continue ;

	    sp = s + asp->rectab[ri].first ;
	    sl = strlen(sp) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindf: key=%s\n",sp) ;
#endif

#if	CF_EXCLUDEKEY
	    if (sl < nc)
	        continue ;
#endif

	    hl = MIN(sl,nc) ;
	    rhash = hashelf(sp,hl) ;

	    hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindf: rhash=%08x hi=%u\n",rhash,hi) ;
#endif

	    c = 0 ;
	    if ((asp->opts & RECORDER_OSEC) && (it[hi][0] != 0)) {

#if	CF_DEBUGS
	        debugprintf("recorder_mkindf: collision ri=%d\n",ri) ;
#endif

	        while ((it[hi][0] != 0) &&
	            (strncmp(sp,(s + asp->rectab[it[hi][0]].first),hl) != 0)) {

#if	CF_DEBUGS
	            debugprintf("recorder_mkindf: collision c=%d "
	                "not same key\n",c) ;
#endif

	            if (asp->opts & RECORDER_ORANDLC) {
	                rhash = randlc(rhash + c) ;
	            } else {
	                rhash = ((rhash << (32 - ns)) | (rhash >> ns)) + c ;
	            }

	            hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	            debugprintf("recorder_mkindf: new rhash=%08x hi=%u\n",
	                rhash,hi) ;
#endif

	            c += 1 ;

	        } /* end while */

	    } /* end if (secondary hash on collision) */

	    if (it[hi][0] != 0) {
	        int	lhi ;

	        c += 1 ;
	        while (it[hi][1] != 0) {
	            c += 1 ;
	            hi = it[hi][1] ;
	        }

	        lhi = hi ;			/* save last hash-index value */
	        hi = hashindex((hi + 1),n) ;

	        while (it[hi][0] != 0) {
	            c += 1 ;
	            hi = hashindex((hi + 1),n) ;
	        } /* end while */

	        it[lhi][1] = hi ;		/* update the previous slot */

	    } /* end if (got a hash collision) */

	    it[hi][0] = ri ;
	    it[hi][1] = 0 ;
	    asp->s.c_f += c ;
	    recorder_cden(asp,wi,c) ;

	} /* end for (looping through records) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (recorder_mkindf) */


int recorder_mkindfl3(RECORDER *asp,cchar s[],uint it[][2],int itsize)
{
	uint		rhash ;
#if	CF_DEBUGSHIFT
	int		ns = (nshift > 0) ? nshift : NSHIFT ;
#else
	const int	ns = NSHIFT ;
#endif
	int		rs = SR_OK ;
	int		hi, ri, c, maxlast ;
	int		sl, hl, n, size ;
	int		wi = index_fl3 ;
	const char	*sp ;
	char		hbuf[4 + 1] ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (it == NULL) return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;

	if (size > itsize)
	    return SR_OVERFLOW ;

	memset(it,0,size) ;

	for (ri = 1 ; ri < asp->i ; ri += 1) {

	    if ((asp->rectab[ri].first == 0) ||
	        (asp->rectab[ri].last == 0))
	        continue ;

	    sp = s + asp->rectab[ri].last ;
	    sl = strlen(sp) ;

	    if (sl < 3)
	        continue ;

	    maxlast = MIN(sl,3) ;
	    strncpy((hbuf + 1),sp,maxlast) ;

	    sp = s + asp->rectab[ri].first ;
	    hbuf[0] = sp[0] ;

#if	CF_DEBUGS
	    hbuf[4] = '\0' ;
	    debugprintf("recorder_mkindfl3: ri=%u key=%s\n",ri,hbuf) ;
#endif

#if	CF_EXCLUDEKEY
	    if (sl < nc)
	        continue ;
#endif

	    hl = 1 + maxlast ;
	    rhash = hashelf(hbuf,hl) ;

	    hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindfl3: rhash=%08x hi=%u\n",rhash,hi) ;
#endif

	    c = 0 ;
	    if ((asp->opts & RECORDER_OSEC) && (it[hi][0] != 0)) {

#if	CF_DEBUGS
	        debugprintf("recorder_mkindfl3: sec collision ri=%d\n",ri) ;
#endif

	        while ((it[hi][0] != 0) &&
	            (recorder_matfl3(asp,s,it,hi,hbuf) <= 0)) {

#if	CF_DEBUGS
	            debugprintf("recorder_mkindfl3: sec collision c=%u "
	                "not same key\n",c) ;
#endif

	            if (asp->opts & RECORDER_ORANDLC) {
	                rhash = randlc(rhash + c) ;
	            } else {
	                rhash = ((rhash << (32 - ns)) | (rhash >> ns)) + c ;
	            }

	            hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	            debugprintf("recorder_mkindfl3: sec new rhash=%08x hi=%u\n",
	                rhash,hi) ;
#endif

	            c += 1 ;

	        } /* end while */

	    } /* end if (secondary hash on collision) */

	    if (it[hi][0] != 0) {
	        int	lhi ;

	        c += 1 ;
	        while (it[hi][1] != 0) {
	            c += 1 ;
	            hi = it[hi][1] ;
	        }

	        lhi = hi ;			/* save last hash-index value */
	        hi = hashindex((hi + 1),n) ;

	        while (it[hi][0] != 0) {
	            c += 1 ;
	            hi = hashindex((hi + 1),n) ;
	        } /* end while */

	        it[lhi][1] = hi ;		/* update the previous slot */

	    } /* end if (got a hash collision) */

#if	CF_DEBUGS
	    debugprintf("recorder_mkindfl3: setting ri=%u hi=%u\n",ri,hi) ;
#endif

	    it[hi][0] = ri ;
	    it[hi][1] = 0 ;
	    asp->s.c_fl3 += c ;
	    recorder_cden(asp,wi,c) ;

	} /* end for (looping through records) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (recorder_mkindfl3) */


int recorder_mkindun(RECORDER *asp,cchar s[],uint it[][2],int itsize)
{
	uint		rhash ;
#if	CF_DEBUGSHIFT
	int		ns = (nshift > 0) ? nshift : NSHIFT ;
#else
	const int	ns = NSHIFT ;
#endif
	int		rs = SR_OK ;
	int		hi, ri, c ;
	int		hl, n, size ;
	int		wi = index_un ;
	const char	*hp ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (it == NULL) return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;

	if (size > itsize)
	    return SR_OVERFLOW ;

	memset(it,0,size) ;

	for (ri = 1 ; ri < asp->i ; ri += 1) {

	    hp = s + asp->rectab[ri].username ;
	    hl = strlen(hp) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindun: ri=%u key=%s\n",ri,hp) ;
#endif

#if	CF_EXCLUDEKEY
	    if (sl < nc) continue ;
#endif

	    rhash = hashelf(hp,hl) ;

	    hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	    debugprintf("recorder_mkindun: rhash=%08x hi=%u\n",rhash,hi) ;
#endif

	    c = 0 ;
	    if ((asp->opts & RECORDER_OSEC) && (it[hi][0] != 0)) {

#if	CF_DEBUGS
	        debugprintf("recorder_mkindun: sec collision ri=%d\n",ri) ;
#endif

	        while ((it[hi][0] != 0) &&
	            (recorder_matun(asp,s,it,hi,hp) <= 0)) {

#if	CF_DEBUGS
	            debugprintf("recorder_mkindun: sec collision c=%u "
	                "not same key\n",c) ;
#endif

	            if (asp->opts & RECORDER_ORANDLC) {
	                rhash = randlc(rhash + c) ;
	            } else {
	                rhash = ((rhash << (32 - ns)) | (rhash >> ns)) + c ;
	            }

	            hi = hashindex(rhash,n) ;

#if	CF_DEBUGS
	            debugprintf("recorder_mkindun: sec new rhash=%08x hi=%u\n",
	                rhash,hi) ;
#endif

	            c += 1 ;

	        } /* end while */

	    } /* end if (secondary hash on collision) */

	    if (it[hi][0] != 0) {
	        int	lhi ;

	        c += 1 ;
	        while (it[hi][1] != 0) {
	            c += 1 ;
	            hi = it[hi][1] ;
	        }

	        lhi = hi ;			/* save last hash-index value */
	        hi = hashindex((hi + 1),n) ;

	        while (it[hi][0] != 0) {
	            c += 1 ;
	            hi = hashindex((hi + 1),n) ;
	        } /* end while */

	        it[lhi][1] = hi ;		/* update the previous slot */

	    } /* end if (got a hash collision) */

#if	CF_DEBUGS
	    debugprintf("recorder_mkindun: setting ri=%u hi=%u\n",ri,hi) ;
#endif

	    it[hi][0] = ri ;
	    it[hi][1] = 0 ;
	    asp->s.c_un += c ;
	    recorder_cden(asp,wi,c) ;

	} /* end for (looping through records) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (recorder_mkindun) */


/* get statistics */
int recorder_info(RECORDER *asp,RECORDER_INFO *ip)
{
	int		rs = SR_OK ;
	int		n ;

#if	CF_SAFE
	if (asp == NULL) return SR_FAULT ;

	if (asp->magic != RECORDER_MAGIC) return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (ip == NULL)
	    return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	*ip = asp->s ;
	ip->ilen = n ;
	rs = asp->i ;

	return rs ;
}
/* end subroutine (recorder_info) */


/* private subroutines */


/* extend the object recorder */
static int recorder_extend(RECORDER *asp)
{
	int		rs = SR_OK ;
	int		ne, size ;
	uint		*nrt = NULL ;

#if	CF_DEBUGS
	debugprintf("recorder_extend: entered e=%u\n",asp->e) ;
#endif

#if	CF_FASTGROW
	ne = ((asp->e + 1) * 2) ;
#else
	ne = (asp->e + RECORDER_STARTNUM) ;
#endif /* CF_FASTGROW */

	size = ne * sizeof(RECORDER_ENT) ;
	if ((rs = uc_realloc(asp->rectab,size,&nrt)) >= 0) {
	    asp->e = ne ;
	    asp->rectab = (RECORDER_ENT *) nrt ;
	} else {
	    asp->i = rs ;
	}

#if	CF_DEBUGS
	debugprintf("recorder_extend: returning e=%d\n",asp->e) ;
#endif

	return (rs >= 0) ? asp->e : rs ;
}
/* end subroutine (recorder_extend) */


static int recorder_matfl3(RECORDER *asp,cchar s[],uint it[][2],int hi,
		cchar *hbuf)
{
	int		si, f ;

	si = asp->rectab[it[hi][0]].first ;
	f = ((s + si)[0] == hbuf[0]) ;

	if (f) {
	    si = asp->rectab[it[hi][0]].last ;
	    f = (strncmp((s + si),(hbuf + 1),3) == 0) ;
	}

	return f ;
}
/* end subroutine (recorder_matfl3) */


static int recorder_matun(asp,s,it,hi,hbuf)
RECORDER	*asp ;
const char	s[] ;
uint		it[][2] ;
int		hi ;
const char	hbuf[] ;
{
	int		si, f ;

	si = asp->rectab[it[hi][0]].username ;
	f = (strcmp((s + si),hbuf) == 0) ;

	return f ;
}
/* end subroutine (recorder_matun) */


static int recorder_cden(RECORDER *asp,int wi,int c)
{

	if (c >= RECORDER_NCOLLISIONS) c = (RECORDER_NCOLLISIONS - 1) ;

	asp->s.cden[wi][c] += 1 ;
	return 0 ;
}
/* end subroutine (recorder_cden) */


/* calculate the next hash from a given one */
static int hashindex(uint i,int n)
{
	int	hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end if (hashindex) */


#if	CF_DEBUGS && CF_DEBUGBOUNDS
static int inbounds(cchar *buf,int buflen,cchar *tp)
{
	return ((tp >= (buf + buflen)) && (tp < buf)) ? TRUE : FALSE ;
}
/* end if (inbounds) */
#endif /* CF_DEBUGS */


