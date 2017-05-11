/* mkcmds */


/* revision history:

	= 1999-03-01, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	MKCMDS_INCLUDE
#define	MKCMDS_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>


enum mkcmds {
	mkcmd_minwordlen,
	mkcmd_maxwordlen,
	mkcmd_eigenwords,
	mkcmd_nkeys,
	mkcmd_tablen,
	mkcmd_sdn,
	mkcmd_sfn,
	mkcmd_lang,
	mkcmd_overlast
} ;


extern const char	*mkcmds[] ;


#endif /* MKCMDS_INCLUDE */


