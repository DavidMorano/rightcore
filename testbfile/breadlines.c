/* breadlines */

/* get a line with possible continuation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	Written a new out of frustration of previous hacks.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a BFILE subroutine that reads line that may be continued onto
        the next real line using the back-slash character.

	Synopsis:

	int breadlines(fp,lbuf,llen,lcp)
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

#define	BFILE_MASTER	0

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#define	TO_READ		(5*60)

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int breadlines(bfile *fp,char *lbuf,int llen,int *lcp)
{
	const int	to = TO_READ ;
	int		rs = SR_OK ;
	int		i = 0 ;
	int		lines = 0 ;
	int		f_cont = FALSE ;

#if	CF_DEBUGS
	debugprintf("breadlines: llen=%d\n",llen) ;
#endif

	if (lbuf == NULL) return SR_FAULT ;

	lbuf[0] = '\0' ;
	while ((lines == 0) || (f_cont = ISCONT(lbuf,i))) {

	    if (f_cont) i -= 2 ;

	    rs = breadlinetimed(fp,(lbuf + i),(llen - i),to) ;
	    if (rs <= 0) break ;
	    i += rs ;
	    lines += 1 ;

	} /* end while */

	if (lcp != NULL) *lcp  = lines ;

#if	CF_DEBUGS
	debugprintf("breadlines: ret rs=%d i=%d\n",rs,i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (breadlines) */


