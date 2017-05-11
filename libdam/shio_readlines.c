/* shio_readlines */

/* get a line with possible continuation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Written a new out of frustration of previous hacks.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a SHIO subroutine that reads line that may be continued onto the
        next real line using the back-slash character.

	Synopsis:

	int shio_readlines(fp,lbuf,llen,lcp)
	bfile		*fp ;
	char		lbuf[] ;
	int		llen ;
	int		*lcp ;

	Arguments:

	fp		pointer to object
	lbuf		user buffer to receive data
	llen		length of user supplied buffer
	lcp		pointer to integer to receive number of lines read

	Returns:

	>=0		number of bytes read
	<0		error


*******************************************************************************/


#define	SHIO_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>

#include	"shio.h"


/* local defines */

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int shio_readlines(fp,lbuf,llen,lcp)
SHIO		*fp ;
char		lbuf[] ;
int		llen ;
int		*lcp ;
{
	int	rs = SR_OK ;
	int	alen ;			/* "add" length */
	int	i = 0 ;
	int	f_first = TRUE ;
	int	f_cont = FALSE ;


#if	CF_DEBUGS
	debugprintf("shio_readlines: llen=%d\n",llen) ;
#endif

	lbuf[0] = '\0' ;
	while (f_first || (f_cont = ISCONT(lbuf,i))) {

	    f_first = FALSE ;
	    if (f_cont)
	        i -= 2 ;

	    alen = (llen - i) ;

#if	CF_DEBUGS
	    debugprintf("shio_readlines: i=%d alen=%d\n",i,alen) ;
#endif

	    rs = shio_readline(fp,(lbuf + i),alen) ;
	    if (rs <= 0) break ;
	    i += rs ;

	    if (lcp != NULL)
	        *lcp += 1 ;

#if	CF_DEBUGS
	    debugprintf("shio_readlines: i=%d next-f_cont=%d\n",
	        i, ISCONT(lbuf,i)) ;
#endif

	} /* end while */

ret0:

#if	CF_DEBUGS
	debugprintf("shio_readlines: ret i=%d\n",i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (shio_readlines) */


