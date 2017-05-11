/* shio_writeblanks */

/* write blacks to a SHIO object */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Write a specified number of blanks to a SHIO object.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>

#include	"shio.h"


/* local defines */

#undef	NBLANKS
#define	NBLANKS		8


/* external subroutines */

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnset(char *,int,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int shio_writeblanks(SHIO *fp,int n)
{
	int		rs = SR_OK ;
	int		ml ;
	int		wlen = 0 ;
	char		blanks[NBLANKS] ;

	strnset(blanks,' ',NBLANKS) ;
	while ((rs >= 0) && (wlen < n)) {
	    ml = MIN((n-wlen),NBLANKS) ;
	    rs = shio_write(fp,blanks,ml) ;
	    wlen += rs ;
	} /* end while */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (shio_writeblanks) */


