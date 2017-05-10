/* moduleclean */

/* clean up a software module name */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1986-03-01, David A­D­ Morano

	This subroutine was originally written for use for LPPI firmware
	development.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Here we do some processing on the (?) module name.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern char	*strnpbrk(const char *,int,const char *) ;


/* exported subroutines */


int moduleclean(pip,rbuf,rlen,inname)
PROGINFO	*pip ;
char		rbuf[] ;
int		rlen ;
const char	inname[] ;
{
	SBUF		ubuf ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;

	if (pip == NULL) return SR_FAULT ;
	if (inname == NULL) return SR_FAULT ;

	if (inname[0] == '\0')
	    return SR_OK ;

	if (rbuf == NULL) return SR_FAULT ;

	if ((rs = sbuf_start(&ubuf,rbuf,rlen)) >= 0) {
	     const char	*np = inname ;
	     const char	*tp ;

	    while ((tp = strpbrk(np,"+-/")) != NULL) {
	        sbuf_buf(&ubuf,np,(tp - np)) ;
	        sbuf_char(&ubuf,'_') ;
	        np = (tp + 1) ;
	        f = TRUE ;
	    } /* end while */
	    sbuf_buf(&ubuf,np,-1) ;

	    rs1 = sbuf_finish(&ubuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (moduleclean) */


