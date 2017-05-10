/* keys */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	KEYS_INCLUDE
#define	KEYS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<bfile.h>
#include	<hdb.h>
#include	<ptm.h>
#include	<localmisc.h>
#include	"defs.h"


#ifdef	__cplusplus
extern "C" {
#endif

extern int keysstart(PROGINFO *,HDB *,int) ;
extern int keysadd(PROGINFO *,HDB *,const char *,int) ;
extern int keysfinish(PROGINFO *,HDB *,bfile *,PTM *,cchar *,offset_t,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* KEYS_INCLUDE */


