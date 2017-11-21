/* svcentsub */

/* expanded server entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	SVCENTSUB_INCLUDE
#define	SVCENTSUB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<strpack.h>

#include	"mfslocinfo.h"		/* for |locinfo()| */
#include	"svcfile.h"


#define	SVCENTSUB	struct svcentsub_head
#define	SVCENTSUB_CSIZE	100		/* default string-chunk size */


enum svckeys {
	svckey_so,
	svckey_p,
	svckey_pass,
	svckey_a,
	svckey_u,
	svckey_g,
	svckey_interval,
	svckey_acc,
	svckey_opts,
	svckey_failcont,
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

extern int svcentsub_start(SVCENTSUB *,LOCINFO *,SVCFILE_ENT *) ;
extern int svcentsub_finish(SVCENTSUB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCENTSUB_MASTER */

#endif /* SVCENTSUB_INCLUDE */


