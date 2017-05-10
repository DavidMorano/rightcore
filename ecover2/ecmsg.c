/* ecmsg */

/* email-cover subchannel message */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little dittie provides a little way to help manage the subchannel
        message for the email-cover algorithm.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"ecmsg.h"


/* local defines */


/* exported subroutines */


int ecmsg_start(mp)
ECMSG		*mp ;
{


	if (mp == NULL)
	    return SR_FAULT ;

	memset(mp,0,sizeof(ECMSG)) ;

	return SR_OK ;
}
/* end subroutine (ecmsg_start) */


int ecmsg_finish(mp)
ECMSG		*mp ;
{


	if (mp == NULL)
	    return SR_FAULT ;

	if (mp->buf != NULL)
	    uc_free(mp->buf) ;

	memset(mp,0,sizeof(ECMSG)) ;

	return SR_OK ;
}
/* end subroutine (ecmsg_finish) */


int ecmsg_loadbuf(mp,buf,buflen)
ECMSG		*mp ;
const char	buf[] ;
int		buflen ;
{
	int	rs = SR_OK ;
	int	bl ;


	if (mp == NULL)
	    return SR_FAULT ;

	if (mp->buf != NULL)
	    uc_free(mp->buf) ;

	mp->buf = NULL ;
	mp->buflen = 0 ;
	bl = (buflen >= 0) ? buflen : strlen(buf) ;

	if (bl > ECMSG_MAXBUFLEN)
	    bl = ECMSG_MAXBUFLEN ;

	if (bl >= 0) {

	    rs = uc_malloc((bl + 1),&mp->buf) ;

	    if (rs >= 0) {

	        memcpy(mp->buf,buf,bl) ;

	        mp->buf[bl] = '\0' ;
	        mp->buflen = bl ;

	    }
	}

	return rs ;
}
/* end subroutine (ecmsg_loadbuf) */


int ecmsg_already(mp)
ECMSG		*mp ;
{


	if (mp == NULL)
	    return SR_FAULT ;

	return (mp->buf != NULL) ;
}
/* end subroutine (ecmsg_initbuf) */



