/* sbuf_addquoted */

/* storage buffer (SBuf) object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Add a shell-quoted string to the SBUF object.

	Synopsis:

	int sbuf_addquoted(sbp,ap,al)
	SBUF		*sbp ;
	const char	*ap ;
	int		al ;

	Arguments:

	sbp		pointer to the buffer object
	ap		string to add
	al		length of string to add

	Returns:

	>=0		amount of new space used by the newly stored item
			(not including any possible trailing NUL characters)
	<0		error


*******************************************************************************/


#define	SBUF_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"sbuf.h"


/* local defines */


/* external subroutines */

extern int	mkquoted(char *,int,const char *,int) ;


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int sbuf_addquoted(SBUF *sbp,cchar *ap,int al)
{
	int		rs ;
	int		qlen ;
	int		len = 0 ;
	char		*qbuf ;

#if	CF_DEBUGS
	debugprintf("sbuf_addquoted: ent >%t<\n",ap,al) ;
#endif

	if (ap == NULL) return SR_FAULT ;

	if (al < 0) al = strlen(ap) ;

	qlen = ((al * 2) + 3) ;
	if ((rs = uc_malloc((qlen+1),&qbuf)) >= 0) {

	    if ((rs = mkquoted(qbuf,qlen,ap,al)) >= 0) {
	        len = rs ;
	        rs = sbuf_strw(sbp,qbuf,len) ;
	    }

	    uc_free(qbuf) ;
	} /* end if (allocation) */

#if	CF_DEBUGS
	debugprintf("sbuf_addquoted: ret rs=%d len=%u\n",rs,len) ;
#endif

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sbuf_addquoted) */


