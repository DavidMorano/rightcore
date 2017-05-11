/* tmpx */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	TMPX_INCLUDE
#define	TMPX_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<utmpx.h>

#include	<localmisc.h>	/* for special types */


#undef	TMPX_DARWIN
#define	TMPX_DARWIN	\
	(defined(OSNAME_Darwin) && (OSNAME_Darwin > 0))

#undef	TMPX_SUNOS
#define	TMPX_SUNOS	\
	(defined(OSNAME_SunOS) && (OSNAME_SunOS > 0))

/* object defines */

#if	TMPX_SUNOS
#define	TMPX_ENT	struct futmpx
#else
#define	TMPX_ENT	struct utmpx
#endif

#define	TMPX		struct tmpx_head
#define	TMPX_CUR	struct tmpx_c
#define	TMPX_FL		struct tmpx_flags
#define	TMPX_MAGIC	1092387456
#define	TMPX_ENTSIZE	sizeof(TMPX_ENT)

/* other defines */

#if	defined(TMPX_SUNOS) && TMPX_SUNOS
#define	TMPX_DEFUTMP	"/var/adm/utmpx"
#elif	defined(TMPX_DARWIN) && TMPX_DARWIN
#define	TMPX_DEFUTMP	"/var/run/utmpx"
#else
#define	TMPX_DEFUTMP	"/var/run/utmpx"
#endif

/* entry type values */

#define	TMPX_TEMPTY		0	/* entry is unused */
#define	TMPX_TRUNLEVEL		1
#define	TMPX_TBOOTTIME		2
#define	TMPX_TOLDTIME		3
#define	TMPX_TNEWTIME		4
#define	TMPX_TINITPROC		5	/* process spawned by "init" */
#define	TMPX_TLOGINPROC		6	/* a "getty" waiting for login */
#define	TMPX_TUSERPROC		7	/* a regular user process */
#define	TMPX_TDEADPROC		8	/* dead process (moved to WTMPX) */
#define	TMPX_TACCOUNT		9	/* used in WTMPX only? */
#define	TMPX_TSIGNATURE		10	/* used in WTMPX only? */

/* entry lengths */

#define	TMPX_LID		4
#define	TMPX_LUSER		32
#define	TMPX_LLINE		32
#define	TMPX_LHOST		256

/* UTMPX stuff (in theory could be different from above) */

#ifndef	UTMPX_TEMPTY
#define	UTMPX_TEMPTY		0	/* entry is unused */
#define	UTMPX_TRUNLEVEL		1
#define	UTMPX_TBOOTTIME		2
#define	UTMPX_TOLDTIME		3
#define	UTMPX_TNEWTIME		4
#define	UTMPX_TINITPROC		5	/* process spawned by "init" */
#define	UTMPX_TLOGINPROC	6	/* a "getty" waiting for login */
#define	UTMPX_TUSERPROC		7	/* a regular user process */
#define	UTMPX_TDEADPROC		8	/* used in WTMPX only? */
#define	UTMPX_TACCOUNT		9	/* used in WTMPX only? */
#define	UTMPX_TSIGNATURE	10	/* used in WTMPX only? */
#endif /* UTMPX_TEMPTY */

#ifndef	UTMPX_LID
#define	UTMPX_LID		4
#define	UTMPX_LUSER		32
#define	UTMPX_LLINE		32
#define	UTMPX_LHOST		256
#endif

/* UTMP stuff (in theory could be different from above) */

#ifndef	UTMP_TEMPTY
#define	UTMP_TEMPTY		0	/* entry is unused */
#define	UTMP_TRUNLEVEL		1
#define	UTMP_TBOOTTIME		2
#define	UTMP_TOLDTIME		3
#define	UTMP_TNEWTIME		4
#define	UTMP_TINITPROC		5	/* process spawned by "init" */
#define	UTMP_TLOGINPROC		6	/* a "getty" waiting for login */
#define	UTMP_TUSERPROC		7	/* a regular user process */
#define	UTMP_TDEADPROC		8	/* used in WTMP only? */
#define	UTMP_TACCOUNT		9	/* used in WTMP only? */
#define	UTMP_TSIGNATURE		10	/* used in WTMP only? */
#endif /* UTMP_TEMPTY */

#ifndef	UTMP_LID
#define	UTMP_LID		4
#define	UTMP_LUSER		8
#define	UTMP_LLINE		12
#endif


struct tmpx_c {
	int		i ;
} ;

struct tmpx_flags {
	uint		writable:1 ;
} ;

struct tmpx_head {
	uint		magic ;
	const char	*fname ;	/* stored file name */
	caddr_t		mapdata ;	/* file mapping buffer */
	TMPX_FL		f ;
	time_t		ti_open ;	/* open time (for FD caching) */
	time_t		ti_mod ;	/* last modification time */
	time_t		ti_check ;	/* last check time */
	size_t		mapsize ;
	size_t		fsize ;		/* file total size */
	uint		mapoff ;	/* file mapping starting offset */
	int		pagesize ;
	int		oflags ;	/* open flags */
	int		operms ;	/* open permissions */
	int		fd ;		/* file descriptor */
	int		ncursors ;
	int		mapei ;		/* index of top mapped entry */
	int		mapen ;		/* number of mapped entries */
} ;


#if	(! defined(TMPX_MASTER)) || (TMPX_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int tmpx_open(TMPX *,const char *,int) ;
extern int tmpx_read(TMPX *,int,TMPX_ENT *) ;
extern int tmpx_write(TMPX *,int,TMPX_ENT *) ;
extern int tmpx_check(TMPX *,time_t) ;
extern int tmpx_curbegin(TMPX *,TMPX_CUR *) ;
extern int tmpx_curend(TMPX *,TMPX_CUR *) ;
extern int tmpx_enum(TMPX *,TMPX_CUR *,TMPX_ENT *) ;
extern int tmpx_fetchuser(TMPX *,TMPX_CUR *,TMPX_ENT *,cchar *) ;
extern int tmpx_fetchpid(TMPX *,TMPX_ENT *,pid_t) ;
extern int tmpx_nusers(TMPX *) ;
extern int tmpx_close(TMPX *) ;

#ifdef	__cplusplus
}
#endif

#endif /* TMPX_MASTER */

#endif /* TMPX_INCLUDE */


