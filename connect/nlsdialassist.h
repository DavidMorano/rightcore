/* nlsdialassist */

/* assistance for NLS dialing */
/* last modified %G% version %I% */


/* revision history:

	= 1998-02-17, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	We provide NLS dialing assistance.


******************************************************************************/


#ifndef	NLSDIALASSIST_INCLUDE
#define	NLSDIALASSIST_INCLUDE	1

#include	<envstandards.h>
#include	<localmisc.h>

#define	NLPS_REQ0	"NLPS:000:001:"	/* version 0 */
#define	NLPS_REQ2	"NLPS:002:002:"	/* version 2 (we use this one) */
#define	NLPS_REQVERSION	2		/* NLPS protocol version */
#define	NLPS_TO		30		/* time-out in seconds */


#ifdef	__cplusplus
extern "C" {
#endif

extern int	mknlsreq(char *,int,const char *,int) ;
extern int	readnlsresp(int,char *,int,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* NLSDIALASSIST_INCLUDE */


