/* tse (Time-Stamp Entry) */

/* time-stamp entry marshalling */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-06-25, David A­D­ Morano
        This is being writen to support shared (and permanent) timestamps for
        key-names.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        These little module implements the messages (reads and writes) to the TS
        (Time-Stamp) database file. These subroutines actually do the argument
        marshalling for the file reads and writes.


******************************************************************************/


#define	TSE_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<inttypes.h>

#include	<vsystem.h>
#include	<serialbuf.h>
#include	<localmisc.h>

#include	"tse.h"


/* local defines */


/* exported subroutines */


int tse_all(ep,f_read,abuf,alen)
char		abuf[] ;
int		alen ;
int		f_read ;
TSE_ALL		*ep ;
{
	SERIALBUF	m, *mp = &m ;
	int		rs ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;
	if (abuf == NULL) return SR_FAULT ;

	if (alen < 0) alen = INT_MAX ;

	if ((rs = serialbuf_start(mp,abuf,alen)) >= 0) {
	    if (f_read) {
	        serialbuf_ruint(mp,&ep->count) ;
	        serialbuf_ruint(mp,&ep->utime) ;
	        serialbuf_ruint(mp,&ep->ctime) ;
	        serialbuf_ruint(mp,&ep->hash) ;
	        serialbuf_rstrn(mp,ep->keyname,TSE_LKEYNAME) ;
	    } else {
	        serialbuf_wuint(mp,ep->count) ;
	        serialbuf_wuint(mp,ep->utime) ;
	        serialbuf_wuint(mp,ep->ctime) ;
	        serialbuf_wuint(mp,ep->hash) ;
	        serialbuf_wstrn(mp,ep->keyname,TSE_LKEYNAME) ;
	    } /* end if */
	    rs1 = serialbuf_finish(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	return rs ;
}
/* end subroutine (entry_all) */


int tse_update(ep,f_read,abuf,alen)
char		abuf[] ;
int		alen ;
int		f_read ;
TSE_UPDATE	*ep ;
{
	SERIALBUF	m, *mp = &m ;
	int		rs ;
	int		rs1 ;

	if (ep == NULL) return SR_FAULT ;
	if (abuf == NULL) return SR_FAULT ;

/* go to where this data is in the message buffer */

	abuf += TSE_OCOUNT ;
	alen -= TSE_OCOUNT ;

/* proceed as normal (?) :-) */

	if ((rs = serialbuf_start(mp,abuf,alen)) >= 0) {
	    if (f_read) {
	        serialbuf_ruint(mp,&ep->count) ;
	        serialbuf_ruint(mp,&ep->utime) ;
	    } else {
	        serialbuf_wuint(mp,ep->count) ;
	        serialbuf_wuint(mp,ep->utime) ;
	    } /* end if */
	    rs1 = serialbuf_finish(mp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (serialbuf) */

	if (rs >= 0) {
	    rs += TSE_OCOUNT ;
	}

	return rs ;
}
/* end subroutine (tse_update) */


