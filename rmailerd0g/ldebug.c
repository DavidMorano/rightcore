/* ldebug */

/* local debugging stubs */


#define	CF_DEBUGS	0


/* revision history:

	- 1997-11-21, David A­D­ Morano

	This program was started by copying from the RSLOW program.


*/

/* Copyright © 1997 David A­D­ Morano.  All rights reserved. */

/**************************************************************************

	This modeule provides debugging support for the REXEC program.


**************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stropts.h>
#include	<poll.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>

#include	<vsystem.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */



/* external subroutines */


/* forward subroutines */


/* external variables */


/* local variables */


/* exported subroutines */


/* convert file mode bits to ASCII string */
char *d_procmode(mode,buf,buflen)
int	mode ;
char	buf[] ;
int	buflen ;
{


	switch (mode) {

	case IM_FILE:
	    return "FILE" ;

	case IM_PIPE:
	    return "PIPE" ;

	case IM_SEEK:
	    return "SEEK" ;

	} /* end switch */

	return NULL ;
}
/* end subroutine (d_procmode) */




