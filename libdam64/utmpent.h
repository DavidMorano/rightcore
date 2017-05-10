/* utmpent */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	UTMPENT_INCLUDE
#define	UTMPENT_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<utmpx.h>
#include	<localmisc.h>	/* for special types */


/* object defines */

#ifndef	UTMPFENT
#define	UTMPFENT		struct futmpx
#endif

/* entry type values */

#define	UTMPENT_TEMPTY		0	/* entry is unused */
#define	UTMPENT_TRUNLEVEL	1
#define	UTMPENT_TBOOTTIME	2
#define	UTMPENT_TOLDTIME	3
#define	UTMPENT_TNEWTIME	4
#define	UTMPENT_TINITPROC	5	/* process spawned by "init" */
#define	UTMPENT_TLOGINPROC	6	/* a "getty" waiting for login */
#define	UTMPENT_TUSERPROC	7	/* a regular user process */
#define	UTMPENT_TDEADPROC	8	/* dead process (moved to WUTMPENT) */
#define	UTMPENT_TACCOUNT	9	/* used in WUTMPENT only? */
#define	UTMPENT_TSIGNATURE	10	/* used in WUTMPENT only? */

/* entry lengths */

#define	UTMPENT_LID		4
#define	UTMPENT_LUSER		32
#define	UTMPENT_LLINE		32
#define	UTMPENT_LHOST		256


#if	(! defined(UTMPENT_MASTER)) || (UTMPENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int utmpent_start(UTMPENT *) ;
extern int utmpent_settype(UTMPENT *,int) ;
extern int utmpent_setpid(UTMPENT *,pid_t) ;
extern int utmpent_setsession(UTMPENT *,int) ;
extern int utmpent_setlines(UTMPENT *,int) ;
extern int utmpent_setcols(UTMPENT *,int) ;
extern int utmpent_setid(UTMPENT *,cchar *,int) ;
extern int utmpent_setline(UTMPENT *,cchar *,int) ;
extern int utmpent_setuser(UTMPENT *,cchar *,int) ;
extern int utmpent_sethost(UTMPENT *,cchar *,int) ;
extern int utmpent_gettype(UTMPENT *) ;
extern int utmpent_getpid(UTMPENT *) ;
extern int utmpent_getsession(UTMPENT *) ;
extern int utmpent_getlines(UTMPENT *) ;
extern int utmpent_getcols(UTMPENT *) ;
extern int utmpent_getid(UTMPENT *,cchar **) ;
extern int utmpent_getline(UTMPENT *,cchar **) ;
extern int utmpent_getuser(UTMPENT *,cchar **) ;
extern int utmpent_gethost(UTMPENT *,cchar **) ;
extern int utmpent_finish(UTMPENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UTMPENT_MASTER */

#endif /* UTMPENT_INCLUDE */


