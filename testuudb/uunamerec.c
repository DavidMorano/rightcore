/* uunamerec */

/* string uunamerec object */


#define	F_FASTGROW	1
#define	F_UCMALLOC	1
#define	F_WEIRD		0


/* revision history:

	= 94/03/24, David A­D­ Morano

	This object module was morphed from some previous one.
	I do not remember what the previous one was.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module creates and manages a string table object.
	This string table can later be written to a file or otherwise
	stored some way so that it can be used in-place later.  This
	is often useful for cache files or ELF code object files.


*******************************************************************************/


#define	UUNAMEREC_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"uunamerec.h"


/* local defines */

#define	UUNAMEREC_STARTLEN	100	/* starting number records */

#define	MODP2(v,n)	((v) & ((n) - 1))


/* external subroutines */

extern uint	nextpowtwo(uint) ;
extern uint	hashelf(const void *,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* forward references */

static int	uunamerec_extend(UUNAMEREC *) ;

static int	hashindex(uint,uint) ;







int uunamerec_init(asp,startlen)
UUNAMEREC	*asp ;
int		startlen ;
{
	int	rs ;


	if (asp == NULL)
	    return SR_FAULT ;

	if (startlen < UUNAMEREC_STARTLEN)
	    startlen = UUNAMEREC_STARTLEN ;

	rs = uc_malloc(startlen,&asp->rectab) ;

	if (rs < 0)
	    goto bad1 ;

	asp->i = 1 ;
	asp->rectab[0] = 0 ;

	asp->e = startlen ;
	asp->c = 0 ;
	return SR_OK ;

/* bad things */
bad1:
	asp->rectab = NULL ;
	asp->e = 0 ;

bad0:
	return rs ;
}
/* end subroutine (uunamerec_init) */


/* add a record to this object */
int uunamerec_add(asp,val)
UUNAMEREC	*asp ;
int		val ;
{
	int	rs, i ;


	if (asp->i < 0)
	    return asp->i ;

/* do we need to extend the table ? */

	if ((asp->i + 1) > asp->e) {

	    if ((rs = uunamerec_extend(asp)) < 0)
	        goto bad0 ;

	} /* end if */

/* copy the new one in */

	i = asp->i ;
	asp->rectab[i] = val ;
	asp->i = i + 1 ;

	asp->c += 1 ;
	return i ;

/* bad things */
bad0:
	return rs ;
}
/* end subroutine (uunamerec_add) */


/* is a given string already represented ? */
int uunamerec_already(asp,val)
UUNAMEREC	*asp ;
int		val ;
{
	int	i ;


	if (asp->i < 0)
	    return asp->i ;

	for (i = 1 ; i < asp->i ; i += 1) {

	    if (val == asp->rectab[i])
	        break ;

	} /* end for */

	return (i < asp->i) ? i : SR_NOTFOUND ;
}
/* end subroutine (uunamerec_already) */


/* get the string count in the table */
int uunamerec_count(asp)
UUNAMEREC	*asp ;
{


	if (asp == NULL)
	    return SR_FAULT ;

	return asp->c ;
}
/* end subroutine (uunamerec_count) */


/* get the address of the rectab array */
int uunamerec_gettab(asp,rpp)
UUNAMEREC	*asp ;
uint		**rpp ;
{
	int	size ;


	if (asp == NULL)
	    return SR_FAULT ;

	if (rpp != NULL)
	    *rpp = asp->rectab ;

	size = asp->i * sizeof(uint) ;
	return size ;
}
/* end subroutine (uunamerec_gettab) */


/* calculate the index table length (entries) */
int uunamerec_indexlen(asp)
UUNAMEREC	*asp ;
{
	int	n ;


	if (asp == NULL)
	    return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	return n ;
}
/* end subroutine (uunamerec_indexlen) */


/* calculate the index table size at this point */
int uunamerec_indexsize(asp)
UUNAMEREC	*asp ;
{
	int	n ;
	int	size ;


	if (asp == NULL)
	    return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;
	return size ;
}
/* end subroutine (uunamerec_indexlen) */


/* create a record index for the caller */
int uunamerec_mkindex(asp,s,it,itsize)
UUNAMEREC	*asp ;
char		s[] ;
uint		it[][2] ;
int		itsize ;
{
	uint	rhash, hi, si ;

	int	rs, i ;
	int	n, size ;

	char	*sp ;


	if (asp == NULL)
	    return SR_FAULT ;

	if (s == NULL)
	    return SR_FAULT ;

	n = nextpowtwo(asp->i) ;

	size = n * 2 * sizeof(uint) ;
	if (size > itsize)
	    return SR_TOOBIG ;

	(void) memset(it,0,size) ;

	for (i = 1 ; i < asp->i ; i += 1) {

	    sp = s + asp->rectab[i] ;
	    rhash = hashelf(sp,-1) ;

	    hi = hashindex(rhash,n) ;

	    if (it[hi][0] != 0) {
	        int	lhi ;

	        while (it[hi][1] != 0)
	            hi = it[hi][1] ;

	        lhi = hi ;
	        hi = hashindex((hi + 1),n) ;

	        while (it[hi][0] != 0)
	            hi = hashindex((hi + 1),n) ;

	        it[lhi][1] = hi ;

	    } /* end if (had a hash collison) */

	    it[hi][0] = i ;
	    it[hi][1] = 0 ;

	} /* end while (looping through records) */

	rs = SR_OK ;

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (uunamerec_mkindex) */


/* free up this object */
int uunamerec_free(asp)
UUNAMEREC	*asp ;
{
	int	rs ;


	if (asp == NULL)
	    return SR_FAULT ;

	if (asp->rectab != NULL)
	    free(asp->rectab) ;

	rs = asp->i ;
	asp->e = 0 ;
	asp->i = 0 ;
	return rs ;
}
/* end subroutine (uunamerec_free) */


/* private subroutines */


/* extend the => record table <= */
static int uunamerec_extend(asp)
UUNAMEREC	*asp ;
{
	int	rs ;
	int	size ;

	uint	*nrt ;


#if	F_FASTGROW
	asp->e = (asp->e * 2) ;
#else
	asp->e = (asp->e + UUNAMEREC_STARTLEN) ;
#endif /* F_FASTGROW */

	size = asp->e * sizeof(uint) ;

#if	F_UCMALLOC
	if ((rs = uc_realloc(asp->rectab,size,&nrt)) < 0) {

	    asp->i = SR_NOMEM ;
	    return rs ;
	}
#else
	if ((nrt = (uint *) realloc(asp->buf,size)) == NULL) {

	    asp->i = SR_NOMEM ;
	    return SR_NOMEM ;
	}
#endif /* F_UCMALLOC */

	asp->rectab = nrt ;
	return (asp->e) ;
}
/* end subroutine (uunamerec_extend) */


/* calculate the next hash from a given one */
static int hashindex(i,n)
uint	i, n ;
{
	int	hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end if (hashindex) */


