/* breadlines */

/* get a line with possible continuation */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	Written a new out of frustration of previous hacks.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a BFILE subroutine that reads line that may be continued
	onto the next real line using the back-slash character.

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

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int breadlines(fp,lbuf,llen,lcp)
bfile		*fp ;
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
	debugprintf("breadlines: llen=%d\n",llen) ;
#endif

	if (lbuf == NULL) return SR_FAULT ;

	if (lcp != NULL) *lcp = 0 ;

	lbuf[0] = '\0' ;
	while (f_first || (f_cont = ISCONT(lbuf,i))) {

#if	CF_DEBUGS
	    debugprintf("breadlines: f_first=%d f_cont=%d\n",f_first,f_cont) ;
	    debugprintf("breadlines: i=%d\n",i) ;
#endif

	    f_first = FALSE ;
	    if (f_cont)
	        i -= 2 ;

	    alen = llen - i ;

#if	CF_DEBUGS
	    debugprintf("breadlines: i=%d alen=%d\n",i,alen) ;
#endif

	    rs = breadline(fp,lbuf + i,alen) ;

#if	CF_DEBUGS
	    debugprintf("breadlines: breadline() rs=%d\n",rs) ;
#endif

	    if (rs <= 0)
	        break ;

	    i += rs ;
	    if (lcp != NULL)
	        *lcp += 1 ;

#if	CF_DEBUGS
	    debugprintf("breadlines: i=%d next-f_cont=%d\n",
	        i, ISCONT(lbuf,i)) ;
#endif

	} /* end while */

#ifdef	COMMENT
	if ((rs >= 0) && (lbuf[i - 1] == '\\'))
	    i -= 1 ;
#endif

ret0:

#if	CF_DEBUGS
	debugprintf("breadlines: ret i=%d\n",i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (breadlines) */



