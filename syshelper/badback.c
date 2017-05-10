/* badback */

/* write a bad status back to indicate a connection failure */
/* last modified %G% version %I% */


#define	CF_DEBUG	1
#define	CF_SRVSHELLARG	0


/* revision history:

	= 1996-07-01, David A­D­ Morano

	This program was originally written.


	= 1998-07-01, David A­D­ Morano

	This subroutine has been enhanced with some modifications to
	the user and group handling of the spawned server program.


*/

/* Copyright © 1996,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Handle a request for which we have a matching server entry.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<grp.h>

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

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	LOGBUFLEN	(LOGNAMELEN + 20)
#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)


/* external subroutines */

extern int	quoteshellarg() ;
extern int	field_svcargs(FIELD *,vecstr *) ;
extern int	processargs(char *,vecstr *) ;
extern int	execute(struct proginfo *,int,char *,char *,vecstr *,vecstr *) ;

extern char	*strbasename(char *) ;
extern char	*timestr_log(), *timestr_edate(), *timestr_elapsed() ;


/* external variables */


/* forward references */


/* local data */

/* Connection Failures */

#ifdef	COMMENT
#define	TCPMUXD_OK		0	/* OK */
#define	TCPMUXD_CFNOSVC		1	/* specified service not found */
#define	TCPMUXD_CFACCESS	2	/* access denied to host for service */
#define	TCPMUXD_CFNOSRV		3	/* server was not found */
#endif

static char	*badmsg[] = {
	"connection established",
	"service not available",
	"access not allowed",
	"server not available",
	NULL
} ;


/* exported subroutines */


int badback(ofd,code,msg)
int	ofd ;
int	code ;
char	*msg ;
{
		char	backbuf[100 + 1] ;

		int	blen ;

	if (msg == NULL)
		msg = badmsg[code] ;

	blen = bufprintf(backbuf,100,"-%d %s\n",
		code,msg) ;

	return uc_writen(ofd,backbuf,blen) ;
} 
/* end subroutine (badback) */


