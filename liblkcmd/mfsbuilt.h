/* mfsbuilt */

/* Service-Request */
/* last modified %G% version %I% */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This subroutine was originally written.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to insert into MFSERVE.

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */


#ifndef	MFSBUILT_INCLUDE
#define	MFSBUILT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<hdb.h>
#include	<localmisc.h>

#include	"mfserve.h"


#define	MFSBUILT		struct mfsbuilt
#define	MFSBUILT_CUR		struct mfsbuilt_c
#define	MFSBUILT_INFO		struct mfsbuilt_info
#define	MFSBUILT_MAGIC		0x75428311
#define	MFSBUILT_NENTS		5
#define	MFSBUILT_INTCHECK	(3*60)


struct mfsbuilt_c {
	uint		magic ;
	HDB_CUR		hcur ;
} ;

struct mfsbuilt {
	uint		magic ;
	HDB		db ;
	const char	*dname ;	/* directory of object modules */
	time_t		ti_check ;	/* last check time */
} ;


#if	(! defined(MFSBUILT_MASTER)) || (MFSBUILT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mfsbuilt_start(MFSBUILT *,cchar *) ;
extern int mfsbuilt_have(MFSBUILT *,cchar *,int) ;
extern int mfsbuilt_loadbegin(MFSBUILT *,MFSERVE_INFO *,cchar *,int) ;
extern int mfsbuilt_loadend(MFSBUILT *,cchar *,int) ;
extern int mfsbuilt_count(MFSBUILT *) ;
extern int mfsbuilt_curbegin(MFSBUILT *,MFSBUILT_CUR *) ;
extern int mfsbuilt_curend(MFSBUILT *,MFSBUILT_CUR *) ;
extern int mfsbuilt_enum(MFSBUILT *,MFSBUILT_CUR *,char *,int) ;
extern int mfsbuilt_strsize(MFSBUILT *) ;
extern int mfsbuilt_strvec(MFSBUILT *,cchar **,char *,int) ;
extern int mfsbuilt_check(MFSBUILT *,time_t) ;
extern int mfsbuilt_finish(MFSBUILT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSBUILT_MASTER */


#endif /* MFSBUILT_INCLUDE */


