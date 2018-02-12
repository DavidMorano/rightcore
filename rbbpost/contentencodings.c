/* content encodings */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code piece supplies the constants needed for content encoding of a
        message body.


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>


/* exported variables */

const char	*contentencodings[] = {
	"none",
	"7bit",
	"8bit",
	"binary",
	"base64",
	"quoted-printable",
	NULL
} ;



