/* mkcmds */

/* names string storiage: purpose? */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	MKCMDS_INCLUDE
#define	MKCMDS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<localmisc.h>


const char *mkcmds[] = {
	"minwordlen",
	"maxwordlen",
	"eigenwords",
	"nkeys",
	"tablen",
	"sdn",
	"sfn",
	"lang",
	NULL
} ;

#endif /* MKCMDS_INCLUDE */


