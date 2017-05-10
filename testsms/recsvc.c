/* recsvc */

/* SMS service entry table object */


#define	CF_DEBUGS	0		/* non-switchable print-outs */
#define	CF_FASTGROW	1		/* grow exponetially ? */
#define	CF_UCMALLOC	1
#define	CF_SAFE		1


/* revision history:

	= 2002-04-29, David A­D­ Morano

	This object module was created for building a SMS service
	entry table.


*/


/******************************************************************************

	This object module creates a table of SMS service entries.


*****************************************************************************/


#define	RECSVC_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"recsvc.h"




/* local defines */

#define	RECSVC_MAGIC	0x12356734

#define	MODP2(v,n)	((v) & ((n) - 1))

#ifndef	ENDIAN
#if	defined(SOLARIS) && defined(__sparc)
#define	ENDIAN		1
#else
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifndef	ENDIAN
#error	"could not determine endianness of this machine"
#endif
#endif
#endif


/* external subroutines */

extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	recsvc_extend(RECSVC *) ;
static int	recsvc_already(RECSVC *,uint,int,int) ;

static int	hashindex(uint,uint) ;






int recsvc_init(asp,n)
RECSVC	*asp ;
int	n ;
{
	int	rs ;
	int	size ;


	if (asp == NULL)
	    return SR_FAULT ;

	(void) memset(asp,0,sizeof(RECSVC)) ;

	if (n < RECSVC_STARTNUM)
	    n = RECSVC_STARTNUM ;

#if	CF_DEBUGS
	debugprintf("recsvc_init: allocating RECTAB n=%d\n",n) ;
#endif

	size = n * sizeof(struct recsvc_ent) ;
	rs = uc_malloc(size,&asp->rectab) ;

	if (rs < 0)
	    goto bad1 ;

#if	CF_DEBUGS
	debugprintf("recsvc_init: rectab=%p\n",asp->rectab) ;
#endif

	asp->e = n ;
	asp->c = 0 ;

/* create a dummy first record for implmentation reasons */

	asp->i = 1 ;
	asp->rectab[asp->i].ia = 0xFFFFFFFF ;


	asp->magic = RECSVC_MAGIC ;
	return rs ;

/* bad things */
bad1:
	asp->rectab = NULL ;
	asp->e = 0 ;

bad0:
	return rs ;
}
/* end subroutine (recsvc_init) */


/* add a record to this object */
int recsvc_add(asp,svc,canon,name)
RECSVC	*asp ;
uint	svc, canon, name ;
{
	int	rs, i ;


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (asp->i < 0)
	    return asp->i ;

/* do we already have this record ? */

	rs = recsvc_already(asp,svc,canon,name) ;

	if (rs != SR_NOTFOUND)
		return rs ;

/* do we need to extend the table ? */

	if ((asp->i + 1) > asp->e) {

	    if ((rs = recsvc_extend(asp)) < 0)
	        goto bad0 ;

	} /* end if */

/* copy the new one in */

	i = asp->i ;

#if	CF_DEBUGS
	debugprintf("recsvc_add: i=%d svc=%d\n",svc) ;
#endif

	asp->rectab[i].svc = svc ;
	asp->rectab[i].canon = canon ;
	asp->rectab[i].name = name ;
	asp->i = i + 1 ;

#if	CF_DEBUGS
	debugprintf("recsvc_add: rectab[%d].svc=%d\n",
		i, asp->rectab[i].svc) ;
#endif

	asp->c += 1 ;
	return i ;

/* bad things */
bad0:
	return rs ;
}
/* end subroutine (recsvc_add) */


/* is a given string IA represented ? */
int recsvc_already(asp,svc,canon,name)
RECSVC	*asp ;
uint	svc, canon, name ;
{
	int	i ;


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (asp->i < 0)
	    return asp->i ;

	for (i = 0 ; i < asp->i ; i += 1) {

	    if (asp->rectab[i].svc != svc)
		continue ;

	    if (asp->rectab[i].canon != canon )
		continue ;

	    if (asp->rectab[i].name == name)
		break ;
		
	} /* end for */

	return (i < asp->i) ? i : SR_NOTFOUND ;
}
/* end subroutine (recsvc_already) */


/* get the length (total number of entries) of the table */
int recsvc_rtlen(asp)
RECSVC	*asp ;
{


	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;

	return asp->i ;
}
/* end subroutine (recsvc_rtlen) */


/* get the count of valid entries in the table */
int recsvc_count(asp)
RECSVC	*asp ;
{


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	return asp->c ;
}
/* end subroutine (recsvc_count) */


/* calculate the index table length (number of entries) at this point */
int recsvc_countindex(asp)
RECSVC	*asp ;
{
	int	n ;


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	n = nextpowtwo(asp->i) ;

	return n ;
}
/* end subroutine (recsvc_countindex) */


/* calculate the index table size */
int recsvc_sizeindex(asp)
RECSVC	*asp ;
{
	int	n ;


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	n = nextpowtwo(asp->i) ;

	return (n * 2 * sizeof(int)) ;
}
/* end subroutine (recsvc_countindex) */


/* get the address of the rectab array */
int recsvc_gettab(asp,rpp)
RECSVC	*asp ;
uint	**rpp ;
{
	int	size ;


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (rpp != NULL)
	    *rpp = (uint *) asp->rectab ;

	size = asp->i * sizeof(struct recsvc_ent) ;
	return size ;
}
/* end subroutine (recsvc_gettab) */


/* create a record index for the caller */
int recsvc_mkindex(asp,it,itsize)
RECSVC	*asp ;
uint	it[][2] ;
int	itsize ;
{
	uint	key, rhash ;
	uint	ri, hi ;

	int	rs, i ;
	int	n, size ;

	char	*sp ;


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (it == NULL)
	    return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;

#if	CF_DEBUGS
	debugprintf("recsvc_mkindex: n=%d itsize=%d size=%d\n",
		n,itsize,size) ;
#endif

	if (size > itsize)
	    return SR_OVERFLOW ;

	(void) memset(it,0,size) ;

	for (i = 1 ; i < asp->i ; i += 1) {

	    key = asp->rectab[i].ia ;
	    rhash = hashelf(&key,sizeof(uint)) ;

	    hi = hashindex(rhash,n) ;

	    if (it[hi][0] != 0) {

	        int	lhi ;


	        while (it[hi][1] != 0)
	            hi = it[hi][1] ;

	        lhi = hi ;			/* save last hash-index value */
	        hi = hashindex((hi + 1),n) ;

	        while (it[hi][0] != 0)
	            hi = hashindex((hi + 1),n) ;

	        it[lhi][1] = hi ;		/* update the previous slot */

	    } /* end if (got a hash collision) */

	    it[hi][0] = i ;
	    it[hi][1] = 0 ;

	} /* end while (looping through records) */

	rs = SR_OK ;

ret0:
	return ((rs >= 0) ? n : rs) ;
}
/* end subroutine (recsvc_mkindex) */


/* free up this recsvc object */
int recsvc_free(asp)
RECSVC	*asp ;
{
	int	rs ;


#if	CF_SAFE
	if (asp == NULL)
		return SR_FAULT ;

	if (asp->magic != RECSVC_MAGIC)
		return SR_NOTOPEN ;
#endif /* CF_SAFE */

	if (asp->rectab != NULL)
	    free(asp->rectab) ;

	rs = asp->i ;
	asp->e = 0 ;
	asp->i = 0 ;
	asp->magic = 0 ;
	return rs ;
}
/* end subroutine (recsvc_free) */



/* PRIVATE SUBROUTINES */



/* extend the object recsvc */
static int recsvc_extend(asp)
RECSVC	*asp ;
{
	int	rs ;
	int	n, size ;

	uint	*nrt ;


#if	CF_DEBUGS
	debugprintf("recsvc_extend: entered e=%d\n",asp->e) ;
#endif

#if	CF_FASTGROW
	asp->e = (asp->e * 2) ;
#else
	asp->e = (asp->e + RECSVC_STARTNUM) ;
#endif /* CF_FASTGROW */

	n = asp->e ;
	size = n * sizeof(struct recsvc_ent) ;

#if	CF_UCMALLOC
	if ((rs = uc_realloc(asp->rectab,size,&nrt)) < 0) {

	    asp->i = SR_NOMEM ;
	    return rs ;
	}
#else
	if ((nrt = (uint *) realloc(asp->rectab,size)) == NULL) {

	    asp->i = SR_NOMEM ;
	    return SR_NOMEM ;
	}
#endif /* CF_UCMALLOC */

	asp->rectab = (struct recsvc_ent *) nrt ;

#if	CF_DEBUGS
	debugprintf("recsvc_extend: returning e=%d\n",asp->e) ;
#endif

	return (asp->e) ;
}
/* end subroutine (recsvc_extend) */


/* calculate the next hash from a given one */
static int hashindex(i,n)
uint	i, n ;
{
	int	hi ;


	hi = MODP2(i,n) ;

	if (hi == 0)
		hi = 1 ;

	return hi ;
}
/* end if (hashindex) */



