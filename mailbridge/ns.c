/* ns */

/* name server */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUGSFIELD	0


/* revision history:

	- 1996-02-01, David A­D­ Morano
	This subroutine was adopted for use from the DWD program.

	- 2003-11-04, David A­D­ Morano
        I don't know where all this has been (apparently "around") but I grabbed
        it from the CM object!

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is a name server object.


*******************************************************************************/


#define	NS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"systems.h"
#include	"ns.h"


/* local defines */

#define	NS_MAGIC	31815926

#define	TI_FILECHECK	3

#define	ARGBUFLEN	(MAXPATHLEN + 35)


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;


/* external variables */


/* forward references */


/* local variables */

static const unsigned char 	fterms[32] = {
	    0x00, 0x00, 0x00, 0x00,
	    0x08, 0x10, 0x00, 0x24,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
} ;


/* exported subroutines */


int ns_open(op,ap,n1,n2)
NS		*op ;
NS_ARGS		*ap ;
const char	n1[], n2[] ;
{


	if (op == NULL)
		return SR_OK ;

	memset(op,0,sizeof(NS)) ;

	return 0 ;
}
/* end subroutine (ns_open) */


int ns_close(op)
NS		*op ;
{


	if (op == NULL)
		return SR_OK ;

	if (op->magic != NS_MAGIC)
		return SR_NOTOPEN ;

	return 0 ;
}
/* end subroutine (ns_close) */



/* PRIVATE SUBROUTINES */



