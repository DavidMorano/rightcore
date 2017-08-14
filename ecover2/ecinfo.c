/* ecinfo */

/* create and parse the internal messages */


#define	CF_DEBUGS	0


/* revision history:

	= 1998-04-19, David A­D­ Morano
	This module was originally written.

	= 1999-07-21, David A­D­ Morano
	I changed something.

*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This module contains the code to make and parse the internal messages
        that are used in this whole server facility.


******************************************************************************/


#define	ECINFO_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>

#include	<vsystem.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"ecinfo.h"


/* local defines */


/* external subroutines */


/* local structures */


/* local variables */


/* exported subroutines */


int ecinfo_data(buf,buflen,f,sp)
char			buf[] ;
int			buflen ;
int			f ;
ECINFO_DATA		*sp ;
{
	SERIALBUF	msgbuf ;
	int		rs ;
	int		rs1 ;

	if ((rs = serialbuf_start(&msgbuf,(char *) buf,buflen)) >= 0) {

	if (f) { /* read */

	    serialbuf_ruint(&msgbuf,&sp->type) ;

	    serialbuf_ruint(&msgbuf,&sp->tag) ;

	    serialbuf_ruint(&msgbuf,&sp->filelen) ;

	    serialbuf_ruint(&msgbuf,&sp->filetime) ;

	    serialbuf_ruint(&msgbuf,&sp->filesum) ;

	    serialbuf_ruint(&msgbuf,&sp->msglen) ;

	    serialbuf_ruint(&msgbuf,&sp->msgtime) ;

	    serialbuf_ruint(&msgbuf,&sp->msgsum) ;

	} else { /* write */

	    sp->type = ecinfotype_data ;

	    serialbuf_wuint(&msgbuf,sp->type) ;

	    serialbuf_wuint(&msgbuf,sp->tag) ;

	    serialbuf_wuint(&msgbuf,sp->filelen) ;

	    serialbuf_wuint(&msgbuf,sp->filetime) ;

	    serialbuf_wuint(&msgbuf,sp->filesum) ;

	    serialbuf_wuint(&msgbuf,sp->msglen) ;

	    serialbuf_wuint(&msgbuf,sp->msgtime) ;

	    serialbuf_wuint(&msgbuf,sp->msgsum) ;

	} /* end if */

	    rs1 = serialbuf_finish(&msgbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (ecinfo_data) */


