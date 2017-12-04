/* badback */

/* write a bad status back to indicate a connection failure */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1986-07-01, David A­D­ Morano
	This program was originally written.

	= 1998-07-01, David A­D­ Morano
        This subroutine has been enhanced with some modifications to the user
        and group handling of the spawned server program.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Handle a request for which we have a matching server entry.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<varsub.h>
#include	<vecstr.h>
#include	<srvtab.h>
#include	<acctab.h>
#include	<localmisc.h>

#include	"srventry.h"
#include	"builtin.h"
#include	"connection.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	BACKBUFLEN	100


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */

/* Connection Failures */

#ifdef	COMMENT
#define	TCPMUXD_CFNOSVC		1	/* specified service not found */
#define	TCPMUXD_CFACCESS	2	/* access denied to host for service */
#define	TCPMUXD_CFNOSRV		3	/* server was not configured */
#define	TCPMUXD_CFNOEXIST	4	/* server does not exist */
#define	TCPMUXD_CFOVERLAST	5
#endif

static char	*const badmsg[] = {
	"connection established",
	"service not available",
	"access not allowed",
	"server not available",
	"server not available",
	NULL
} ;

enum badmsg {
	badmsg_ok,
	badmsg_nosvc,
	badmsg_noaccess,
	badmsg_nosrv,
	badmsg_noexist,
	badmsg_overlast
} ;


/* exported subroutines */


int badback(ofd,code,msg)
int	ofd ;
int	code ;
char	*msg ;
{
	char		backbuf[BACKBUFLEN + 1] ;
	int		blen ;

	if (msg == NULL) {
	    if ((code >= 0) && (code < badmsg_overlast)) {
		msg = badmsg[code] ;
	    }
	}

	if (msg != NULL) {
	    blen = bufprintf(backbuf,100,"-%d %s\r\n", code,msg) ;
	} else {
	    blen = bufprintf(backbuf,100,"-%d\r\n", code) ;
	}

	return uc_writen(ofd,backbuf,blen) ;
} 
/* end subroutine (badback) */


