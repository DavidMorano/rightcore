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
#include	<string.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* external subroutines */

extern int	nleadstr(const char *,const char *,int) ;


/* local structures */

struct provider {
	uint		providerid ;
	char		*name ;
	char		*fullname ;
} ;


/* local variables */

const struct provider	providers[] = {
	{ 0, "unknown", "Unknown Provider" }, 
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
	const char	*bs ;

	if (nl < 0) nl = strlen(np) ;

	for (i = 0 ; providers[i].name != NULL ; i += 1) {

	    bs = providers[i].name ;
	    m = nleadstr(bs,np,nl) ;

	    f = ((m == nl) && (bs[m] == '\0')) ;
	    if (f) break ;

	} /* end for */

	if (f) id = providers[i].providerid ;

	return id ;
}
/* end subroutine (getproviderid) */


