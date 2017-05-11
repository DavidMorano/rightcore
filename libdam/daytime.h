/* daytime */

/* daytime manipulation object */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DAYTIME_INCLUDE
#define	DAYTIME_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vsystem.h>


/* object defines */

#define	DAYTIME		time_t


typedef time_t		daytime ;


#if	(! defined(DAYTIME_MASTER)) || (DAYTIME_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int daytime_start(DAYTIME *) ;
extern int daytime_loadelapsed(DAYTIME *,const char *,int) ;
extern int daytime_mkelapsed(DAYTIME *,char *,int) ;
extern int daytime_add(DAYTIME *,char *,int) ;
extern int daytime_get(DAYTIME *,int,const char **) ;
extern int daytime_finish(DAYTIME *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DAYTIME_MASTER */

#endif /* DAYTIME_INCLUDE */


