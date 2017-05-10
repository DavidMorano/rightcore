/* ha */

/* header-addresses - part of the MKMSG program */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This code piece supplies the mail message header key-names that are used
        to specified addresses (header-Addresses).


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>

#include	<localmisc.h>

#include	"ha.h"


const char	*ha[] = {
	"errorsto",
	"replyto",
	"sender",
	"from",
	"to",
	"cc",
	"bcc",
	NULL
} ;


