/* bfile_fnames */

/* "Basic I/O" package */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0	/* compile-time debug print-outs */


/* revision history:

	= 1998-07-01, David A­D­ Morano

	This subroutine was originally written.


	= 1999-01-10, David A­D­ Morano

	I added the little extra code to allow for memory mapped I/O.
	It is all a waste because it is way slower than without it!
	This should teach me to leave old programs alone!!!


*/

/* Copyright © 1998,1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	BFILE file-names.


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/resource.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */


/* external subroutines */


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported variables */

const char	*bfile_fnames[] = {
	"*STDIN*",
	"*STDOUT*",
	"*STDERR*",
	"*STDNULL*",
	NULL
} ;


/* exported subroutines */



