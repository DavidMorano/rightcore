/* util */

/* utilities */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These are support utility subroutines for the REXD daemon program.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	PROTONAME	"tcp"


/* external subroutines */

extern int	cfdeci(const char *,int,int *) ;


/* externals variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */





