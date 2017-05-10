/* utmpaccent */

/* UTMPACCENT management */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	UTMPACCENT_INCLUDE
#define	UTMPACCENT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<utmpx.h>
#include	<localmisc.h>		/* extra types */


#ifndef	UTMPFENT
#define	UTMPFENT		struct futmpx
#endif

#define	UTMPACCENT		struct utmpaccent

#define	UTMPACCENT_BUFLEN	332

#ifndef	UTMPACCENT_TEMPTY
#define	UTMPACCENT_TEMPTY	0	/* entry is unused */
#define	UTMPACCENT_TRUNLEVEL	1
#define	UTMPACCENT_TBOOTTIME	2
#define	UTMPACCENT_TOLDTIME	3
#define	UTMPACCENT_TNEWTIME	4
#define	UTMPACCENT_TINITPROC	5	/* process spawned by "init" */
#define	UTMPACCENT_TLOGINPROC	6	/* a "getty" waiting for login */
#define	UTMPACCENT_TUSERPROC	7	/* a regular user process */
#define	UTMPACCENT_TDEADPROC	8	/* used in WTMPX only? */
#define	UTMPACCENT_TACCOUNT	9	/* used in WTMPX only? */
#define	UTMPACCENT_TSIGNATURE	10	/* used in WTMPX only? */
#endif /* UTMPACCENT_TEMPTY */

#ifndef	UTMPACCENT_LID
#define	UTMPACCENT_LID		4
#define	UTMPACCENT_LUSER	32
#define	UTMPACCENT_LLINE	32
#define	UTMPACCENT_LHOST	256
#endif


struct utmpaccent {
	const char	*user ;
	const char	*line ;
	const char	*host ;
	time_t		ctime ;
	pid_t		sid ;
	uint		session ;
	ushort		type ;
	ushort		syslen ;
	short		e_exit ;
	short		e_term ;
	char		id[4+1] ;
} ;


#if	(! defined(UTMPACCENT_MASTER)) || (UTMPACCENT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int utmpaccent_load(UTMPACCENT *,char *,int,const UTMPFENT *) ;
extern int utmpaccent_size(const UTMPACCENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* UTMPACCENT_MASTER */

#endif /* UTMPACCENT_INCLUDE */


