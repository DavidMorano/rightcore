/* mfsbuilt */
/* Service-Request */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	MFSBUILT_INCLUDE
#define	MFSBUILT_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	MFSBUILT	struct mfsbuilt
#define	MSGBUILT_NENTS	5
#define	MSGBUILT_MAGIC	0x75428311


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
extern int mfsbuilt_load(MFSBUILT *,cchar *,int,void *) ;
extern int mfsbuilt_release(MFSBUILT *,void *) ;
extern int mfsbuilt_finish(MFSBUILT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSBUILT_MASTER */


#endif /* MFSBUILT_INCLUDE */


