/* hname */

/* header-names */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This code piece supplies the mail message header names that are used for
        some (important?) purposes in the rest of the code. :-)


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"hname.h"


const char	*hname[] = {
	"errorsto",
	"replyto",
	"sender",
	"from",
	"to",
	"cc",
	"bcc",
	"org",
	"msgid",
	"subject",
	NULL
} ;




