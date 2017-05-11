/* strtabfind */

/* find a string in tables created by a STRTAB object */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2008-10-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine finds a specified string in tables that were (assumed)
	created by a STRTAB object.

	Synopsis:

	int strtabfind(tab,it,itlen,nskip,sp,sl)
	const char	tab[] ;
	int		(*it)[3] ;
	int		itlen ;
	const char	*sp ;
	int		sl ;

	Arguments:

	tab		the string table
	it		the index table
	itlen		the length (number of entries) of index table
	nskip		the "skip" factor
	sp		pointer to string to lookup
	sl		length of string to lookup

	Returns:

	>=0		collisions
	<0		error code (NOTFOUND)


	Note: This subroutine uses -> hash-linking <- to track down key matches
	in the index table.


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#define	MODP2(v,n)	((v) & ((n) - 1))


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	hasuc(const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* exported variables */


/* local structures */


/* forward references */

static int	hashindex(uint,int) ;
static int	ismatkey(const char *,const char *,int) ;


/* local variables */


/* exported subroutines */


int strtabfind(cchar tab[],int (*it)[3],int itlen,int nskip,cchar *sp,int sl)
{
	uint		khash, nhash ;
	uint		chash ;
	int		nhi, hi ;
	int		nmax ;
	int		j ;
	int		si = -1 ;
	int		sc = 0 ; /* Skip-Count */
	int		f_mathash = FALSE ;
	int		f_mat = FALSE ;
	const char	*cp ;

	nmax = itlen + nskip ;
	khash = hashelf(sp,sl) ;

	chash = (khash & INT_MAX) ;
	nhash = khash ;
	hi = hashindex(nhash,itlen) ;

#if	CF_DEBUGS
	debugprintf("strtabfind: il=%u ns=%u ss>%t< khash=%08x hi=%u\n",
	    itlen,nskip,sp,sl,khash,hi) ;
#endif

	for (j = 0 ; (j < nmax) && ((si = it[hi][0]) > 0) ; j += 1) {

#if	CF_DEBUGS
	        cp = (tab + si) ;
	debugprintf("strtabfind: j=%u hi=%u si=%u ts>%s< thash=%08x\n",
		j,hi,si,cp,it[hi][1]) ;
#endif

	    f_mathash = ((it[hi][1] & INT_MAX) == chash) ;
	    if (f_mathash) break ;

	    if ((it[hi][1] & (~ INT_MAX)) == 0) {

#if	CF_DEBUGS
	debugprintf("strtabfind: no continuation\n") ;
#endif

		break ;
	    }

	    nhash = hashagain(nhash,j,nskip) ;

	    hi = hashindex(nhash,itlen) ;

	} /* end for */

	sc += j ;

#if	CF_DEBUGS
	debugprintf("strtabfind: mid si=%d f_mathash=%u\n",
		si,f_mathash) ;
#endif

	if ((si > 0) && f_mathash) {

	    while ((si = it[hi][0]) > 0) {

	        cp = (tab + si) ;
	        f_mat = (cp[0] == sp[0]) && ismatkey(cp,sp,sl) ;
	        if (f_mat)
	            break ;

		nhi = it[hi][2] ;
		if (nhi == 0)
		    break ;

		hi = nhi ;
		sc += 1 ;

	    } /* end while */

	} /* end if */

#if	CF_DEBUGS
	debugprintf("strtabfind: ret f=%u sc=%u\n",f_mat,sc) ;
#endif

	return (f_mat) ? sc : SR_NOTFOUND ;
}
/* end subroutine (strtabfind) */


/* local subroutines */


/* calculate the next hash from a given one */
static int hashindex(uint i,int n)
{
	int	hi = MODP2(i,n) ;
	if (hi == 0) hi = 1 ;
	return hi ;
}
/* end subroutine (hashindex) */


static int ismatkey(cchar key[],cchar kp[],int kl)
{
	int	f = (key[0] == kp[0]) ;
	if (f) {
	    int	m = nleadstr(key,kp,kl) ;
	    f = (m == kl) && (key[m] == '\0') ;
	}
	return f ;
}
/* end subroutine (ismatkey) */


