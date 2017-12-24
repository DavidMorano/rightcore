/* svcentsub */

/* expanded server entry */


/* revision history:

	= 2017-10-13, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2017 David A­D­ Morano.  All rights reserved. */


#ifndef	SVCENTSUB_INCLUDE
#define	SVCENTSUB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<strpack.h>

#include	"mfslocinfo.h"		/* for |locinfo()| */
#include	"svcfile.h"


#define	SVCENTSUB	struct svcentsub_head
#define	SVCENTSUB_CSIZE	100		/* default string-chunk size */

#ifndef	SVCENT
#define	SVCENT		SVCFILE_ENT
#endif


enum svckeys { /* KEEP IN SYNC W/ CODE-FILE */
	svckey_file,
	svckey_pass,
	svckey_so,
	svckey_p,
	svckey_a,
	svckey_u,
	svckey_g,
	svckey_interval,
	svckey_acc,
	svckey_opts,
	svckey_failcont,
	svckey_include,
	svckey_overlast
} ;

struct svcentsub_head {
	STRPACK		strs ;
	cchar		*var[svckey_overlast] ;
} ;


#if	(! defined(SVCENTSUB_MASTER)) || (SVCENTSUB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int svcentsub_start(SVCENTSUB *,LOCINFO *,SVCENT *) ;
extern int svcentsub_finish(SVCENTSUB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCENTSUB_MASTER */

#endif /* SVCENTSUB_INCLUDE */


