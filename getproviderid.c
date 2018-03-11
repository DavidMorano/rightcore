/* getproviderid */
/* Get Provider (manufacturer) ID */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a small hack to get a number associated with a manufacturing
	provider.  Prividers are identified (from getting this information from
	the kernel) by a string.  This subroutine looks this string up and
	returns the corresponding number.

	Synopsis:

	int getproviderid(np,nl)
	const char	np[] ;
	int		nl ;

	Arguments:

	np		pointer to provider string (to lookup)
	nl		length of given provider string

	Returns:

	0		provider ID was not found
	>0		provider ID was found and this is it
	

*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<sys/param.h>
#include	<string.h>
#include	<vsystem.h>
#include	<estrings.h>
#include	<localmisc.h>


/* external subroutines */

extern int	getprovider(char *,int) ;
extern int	nleadstr(const char *,const char *,int) ;


/* local structures */

struct provider {
	uint		providerid ;
	char		*codename ;
	char		*realname ;
} ;


/* local variables */

const struct provider	providers[] = {
	{ 0, "unknown", "Unknown" }, 
	{ 1, "Sun_Microsystems", "Sun Microsystems" },
	{ 2, "Compaq Computer Corporation", NULL },
	{ 3, "sgi", "Silicon Graphics" },
	{ 4, NULL, NULL }
} ;


/* exported subroutines */


int getproviderid(cchar *np,int nl)
{
	int		i ;
	int		m ;
	int		id = 0 ;
	int		f = FALSE ;
	cchar		*bs ;

	if (nl < 0) nl = strlen(np) ;

	for (i = 0 ; providers[i].codename != NULL ; i += 1) {
	    bs = providers[i].codename ;
	    m = nleadstr(bs,np,nl) ;
	    f = ((m == nl) && (bs[m] == '\0')) ;
	    if (f) break ;
	} /* end for */

	if (f) id = providers[i].providerid ;

	return id ;
}
/* end subroutine (getproviderid) */


int getvendor(char *rbuf,int rlen)
{
	const int	plen = MAXNAMELEN ;
	int		rs ;
	char		pbuf[MAXNAMELEN+1] ;
	rbuf[0] = '\0' ;
	if ((rs = getprovider(pbuf,plen)) >= 0) {
	    const int	pl = rs ;
	    int		m, i ;
	    int		f = FALSE ;
	    rs = SR_NOTFOUND ;
	    for (i = 0 ; providers[i].codename != NULL ; i += 1) {
	        cchar	*pn = providers[i].codename ;
	        m = nleadstr(pn,pbuf,pl) ;
	        f = ((m == pl) && (pn[m] == '\0')) ;
	        if (f) break ;
	    } /* end for */
	    if (f) {
		rs = sncpy1(rbuf,rlen,providers[i].realname) ;
	    }
	}
	return rs ;
}
/* end subroutine (getvendor) */


