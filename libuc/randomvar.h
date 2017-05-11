/* randomvar */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	RANDOMVAR_INCLUDE
#define	RANDOMVAR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<time.h>

#include	<localmisc.h>


/* object defines */

#define	RANDOMVAR_MAGIC		0x97857322
#define	RANDOMVAR_DEGREE	128
#define	RANDOMVAR_STATELEN	(RANDOMVAR_DEGREE * sizeof(LONG))
#define	RANDOMVAR_STIRTIME	(5 * 60)

#define	RANDOMVAR		struct randomvar_head
#define	RANDOMVAR_FL		struct randomvar_flags
#define	RANDOMVAR_ST		union randomvar_state

#ifndef	UINT
#define	UINT	unsigned int
#endif


struct randomvar_flags {
	UINT		pseudo:1 ;
	UINT		flipper:1 ;
} ;

union randomvar_state {
	LONG		ls[RANDOMVAR_DEGREE] ;
	int		is[RANDOMVAR_DEGREE * 2] ;
} ;
	
struct randomvar_head {
	uint		magic ;
	RANDOMVAR_FL	f ;
	RANDOMVAR_ST	state ;
	time_t		laststir ;
	int		maintcount ;
	int		a, b, c ;
} ;


typedef RANDOMVAR		randomvar ;


#if	(! defined(RANDOMVAR_MASTER)) || (RANDOMVAR_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int randomvar_start(RANDOMVAR *,int,UINT) ;
extern int randomvar_stateload(RANDOMVAR *,cchar *,int) ;
extern int randomvar_statesave(RANDOMVAR *,char *,int) ;
extern int randomvar_setpoly(RANDOMVAR *,int,int) ;
extern int randomvar_addnoise(RANDOMVAR *,const void *,int) ;
extern int randomvar_getlong(RANDOMVAR *,LONG *) ;
extern int randomvar_getulong(RANDOMVAR *,ULONG *) ;
extern int randomvar_getint(RANDOMVAR *,int *) ;
extern int randomvar_getuint(RANDOMVAR *,UINT *) ;
extern int randomvar_finish(RANDOMVAR *) ;

#ifdef	__cplusplus
}
#endif

#endif /* RANDOMVAR_MASTER */

#endif /* RANDOMVAR_INCLUDE */


