/* pcsgetorg */

/* get the PCS organization */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine retrieves the PCS-specific organization string.

	Synopsis:

	int pcsgetorg(pcsroot,rbuf,rlen,un)
	const char	pcsroot[] ;
	char		rbuf[] ;
	char		rlen ;
	const char	*un ;

	Arguments:

	pcsroot		PCS program root path
	rbuf		caller supplied buffer to place result in
	rlen		length of caller supplied buffer
	un		username

	Returns:

	>=0		length of returned ID
	<0		error in process of creating ID


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	localgetorg(const char *,char *,int,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int pcsgetorg(cchar *pr,char *rbuf,int rlen,cchar *un)
{
	int		rs ;

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	rs = localgetorg(pr,rbuf,rlen,un) ;

#if	CF_DEBUGS
	debugprintf("pcsgetorg: ret rs=%d org=>%s<\n",rs,rbuf) ;
#endif

	return rs ;
}
/* end subroutine (pcsgetorg) */


