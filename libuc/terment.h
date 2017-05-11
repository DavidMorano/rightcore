/* terment */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	TERMENT_INCLUDE
#define	TERMENT_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>	/* for special types */


/* object defines */

#define	TERMENT		struct terment

/* entry type values */

#define	TERMENT_TEMPTY		0	/* entry is unused */
#define	TERMENT_TLOGINPROC	1	/* a "getty" waiting for login */
#define	TERMENT_TUSERPROC	2	/* a regular user process */
#define	TERMENT_TDEADPROC	3	/* dead process (moved to WTERMENT) */

/* entry lengths */

#define	TERMENT_LID		4
#define	TERMENT_LLINE		32
#define	TERMENT_LTERMTYPE	32
#define	TERMENT_LANSWER		116


struct terment {
	pid_t		sid ;		/* session ID */
	uchar		type ;		/* type of entry (see above) */
	uchar		termcode ;	/* ANSI terminal code */
	uchar		lines ;
	uchar		cols ;
	char		id[TERMENT_LID] ;	/* UTMP ID */
	char		line[TERMENT_LLINE] ;
	char		termtype[TERMENT_LTERMTYPE] ;
	char		answer[TERMENT_LANSWER] ;
} ;

#if	(! defined(TERMENT_MASTER)) || (TERMENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int terment_start(TERMENT *) ;
extern int terment_settype(TERMENT *,int) ;
extern int terment_setsid(TERMENT *,pid_t) ;
extern int terment_setcode(TERMENT *,int) ;
extern int terment_setlines(TERMENT *,int) ;
extern int terment_setcols(TERMENT *,int) ;
extern int terment_setid(TERMENT *,cchar *,int) ;
extern int terment_setline(TERMENT *,cchar *,int) ;
extern int terment_settermtype(TERMENT *,cchar *,int) ;
extern int terment_setanswer(TERMENT *,cchar *,int) ;

extern int terment_gettype(TERMENT *) ;
extern int terment_getsid(TERMENT *) ;
extern int terment_getcode(TERMENT *) ;
extern int terment_getlines(TERMENT *) ;
extern int terment_getcols(TERMENT *) ;
extern int terment_getid(TERMENT *,cchar **) ;
extern int terment_getline(TERMENT *,cchar **) ;
extern int terment_gettermtype(TERMENT *,cchar **) ;
extern int terment_getanswer(TERMENT *,cchar **) ;
extern int terment_finish(TERMENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TERMENT_MASTER */

#endif /* TERMENT_INCLUDE */


